#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.10"
# dependencies = ["matplotlib>=3.9,<4"]
# ///
"""Build, measure, and illustrate the sorting benchmark for a LaTeX report.

Usage (paths are relative to this script, not the invoking working directory):
    uv run benchmark_report.py
    uv run benchmark_report.py --plot-only
    ./benchmark_report.py            # same thing via the uv shebang

Dependencies are declared inline (PEP 723 block above); uv creates and caches
an isolated environment automatically, so no manual pip install or venv is
needed. Outputs are the unmodified native Google Benchmark CSV at
docs/result/benchmark_result.csv and benchmark_*.png images in the same
docs/result directory. No TeX is generated. From docs/main.tex, for example, use
\\includegraphics[width=\\linewidth]{result/benchmark_overview_random_loglog.png}
with graphicx loaded.

The images deliberately contain almost no explanatory text (only axis labels,
a short title and the legend). Everything below is meant to be moved into the
LaTeX captions / report text instead.

Why these images exist
----------------------
benchmark_complexity.png introduces empirical growth before absolute timings.
Rows are algorithms; columns are input distributions. Colors are a chosen rubric:
Random and Reverse-sorted are green through O(N log N), whereas Sorted is green
only through O(N); remaining subquadratic classes are yellow, quadratic/cubic red.
This is NOT a proof of asymptotic bounds or a universal assessment of algorithms.
Reverse-sorted is a distribution, not necessarily an implementation's worst case.

benchmark_rms.png reports CPU fit RMS as percentages. Google Benchmark normalizes
root-mean-square residuals by the mean measured time, not by each sample's time.
Large RMS warns that a one-term model poorly describes the measurements. There is
no arbitrary pass/fail RMS threshold and no confidence interval implied.

benchmark_overview_<distribution>_<scale>.png compares all algorithms, with one
line per algorithm. CPU time per iteration avoids charging most waiting time, but
frequency scaling, caches, scheduling effects and measurement noise still matter.
Linear axes show absolute differences; log-log axes reveal growth across orders
of magnitude without slow algorithms hiding the faster ones. Slopes are not
proofs of Big-O, particularly for N log N over short ranges.
On linear axes the smallest sizes (N=128, 256, ...) are visually squashed into
the bottom-left corner and can look like points at (0, 0). There is no N=0
benchmark; it is purely a scale effect, which is the main reason the log-log
variant exists. Times are CPU time in nanoseconds.

benchmark_group_<distribution>_<class>_<scale>.png compares algorithms with the
same measured Big-O. A singleton instead includes all algorithms in the nearest
available class(es), using ordinal distance in 1, log N, N, N log N, N^2, N^3.
Both equidistant neighbors are included. These are explicitly cross-class plots,
not evidence that the added algorithms belong to the singleton's class.

Lines join measured samples, marked with circles; the connecting segments are
visual guides, not new measurements. There is NO extrapolation: measured ranges
differ (most quadratic series stop at N=4096, most others at 65536, radix sort
at 262144), so every plot is cropped to the common range - the N values that
ALL algorithms in that plot actually measured, i.e. from the largest minimum N
to the smallest maximum N. Consequences to mention in the report:
- an overview that contains quadratic algorithms only goes up to N=4096;
- a group plot of fast algorithms only can reach 65536 (or more);
- measurements beyond the common range exist in the CSV but are not drawn.

CSV compatibility details (Google Benchmark v1.9.5)
-------------------------------------------------
The native CSV begins with a context preamble, so locate its header first.
BigO rows store a class token in time_unit; their coefficients are in nanoseconds
because the BigO Run uses the default time unit, independent of sample units.
RMS rows in this version's CSV store raw normalized RMS divided by the sample
unit multiplier: for ns, multiply the CSV value by 1e9, then by 100 for percent.
Do NOT interpret a CSV RMS value as already a percentage. This is pinned to the
repository's Benchmark version, not a universal CSV convention across versions.
The default command explicitly sets ns and a single repetition. Plot-only accepts
ns/us/ms/s measurement units, but rejects repeated samples/statistical aggregates
rather than silently mixing observations. Counter values must be positive and
finite for every measurement; BigO/RMS rows legitimately have blank counters.

Build, validation and rendering failures do not publish new results. All images
are rendered before replacing previous outputs; publication rolls back on I/O
errors. Individual renames are atomic, but a power loss/process kill during a
multi-file publication is not transactional. Only the exact generated filenames
owned by this script are replaced or removed; unrelated files are preserved.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
import math
from pathlib import Path
import re
import subprocess
import sys
import tempfile
from typing import TextIO

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parent
CSV_PATH = Path("docs/result/benchmark_result.csv")
IMAGE_PATH = Path("docs/result")
DISTRIBUTIONS = ("random", "sorted", "reverse_sorted")
TITLES = dict(zip(DISTRIBUTIONS, ("Random input", "Sorted input", "Reverse-sorted input")))
CLASSES = ("(1)", "lgN", "N", "NlgN", "N^2", "N^3")
CLASS_NAMES = ("constant", "log_n", "n", "n_log_n", "n_squared", "n_cubed")
NOTATION = ("O(1)", "O(log N)", "O(N)", "O(N log N)", "O(N²)", "O(N³)")
UNITS = {"ns": 1e-9, "us": 1e-6, "ms": 1e-3, "s": 1.0}
COUNTERS = ("CYCLES", "INSTRUCTIONS")
PERF_HELP = (
    "CYCLES and INSTRUCTIONS must both be available and positive. Check libpfm "
    "event support and perf permissions (kernel.perf_event_paranoid); this script "
    "will not change system settings or rerun without counters."
)
COLORS = {"green": "#b9dfb3", "yellow": "#ffe59a", "red": "#f1b1ab"}
NAME_PATTERN = re.compile(
    r"(?P<algorithm>[A-Za-z][A-Za-z0-9_]*?)_"
    r"(?P<distribution>reverse_sorted|random|sorted)"
    r"(?:(?:/(?P<n>[1-9][0-9]*))|_(?P<aggregate>BigO|RMS))"
)


class ReportError(ValueError):
    """An actionable data or workflow error, displayed by the CLI."""


@dataclass(frozen=True)
class Point:
    n: int
    cpu_seconds: float


@dataclass(frozen=True)
class Series:
    algorithm: str
    distribution: str
    points: tuple[Point, ...]
    complexity: str
    coefficient_seconds: float
    rms: float


def number(value: str | float | None, context: str, *, zero: bool = False) -> float:
    try:
        result = float(value) if value is not None else math.nan
    except ValueError as error:
        raise ReportError(f"{context}: expected a number, got {value!r}") from error
    if not math.isfinite(result) or result < 0 or (not zero and result == 0):
        raise ReportError(f"{context}: expected a finite {'nonnegative' if zero else 'positive'} number")
    return result


def parse_csv(stream: TextIO) -> tuple[Series, ...]:
    """Read this repository's native CSV; never average or discard bad samples."""
    for line in stream:
        if line.startswith("name,iterations,real_time,cpu_time,time_unit,"):
            header = next(csv.reader([line]))
            break
    else:
        raise ReportError("Google Benchmark CSV header not found.")
    required = {"name", "iterations", "real_time", "cpu_time", "time_unit",
                "error_occurred", "error_message", *COUNTERS}
    if len(header) != len(set(header)) or not required.issubset(header):
        raise ReportError(f"Invalid CSV header or missing required columns. {PERF_HELP}")

    observations: dict[tuple[str, str], dict[int, Point]] = {}
    units: dict[tuple[str, str], set[str]] = {}
    models: dict[tuple[str, str], tuple[str, float]] = {}
    residuals: dict[tuple[str, str], float] = {}
    reader = csv.DictReader(stream, fieldnames=header)
    for row in reader:
        name = row.get("name") or "<unnamed>"
        if None in row or any(value is None for value in row.values()):
            raise ReportError(f"{name}: malformed CSV row (wrong number of columns).")
        if row["error_occurred"] not in ("", "false", "0") or row["error_message"]:
            raise ReportError(f"{name}: benchmark error: {row['error_message'] or row['error_occurred']}")
        match = NAME_PATTERN.fullmatch(name)
        if match is None:
            raise ReportError(f"Unsupported benchmark row {name!r}; use one repetition and no statistical aggregates.")
        key = (match["algorithm"], match["distribution"])
        cpu = number(row["cpu_time"], f"{name}: CPU time", zero=match["aggregate"] == "RMS")
        number(row["real_time"], f"{name}: real time", zero=match["aggregate"] == "RMS")
        if match["aggregate"] == "BigO":
            if key in models or row["time_unit"] not in CLASSES or row["iterations"]:
                raise ReportError(f"{name}: duplicate or unsupported Big-O model.")
            coefficient = number(cpu * UNITS["ns"], f"{name}: CPU coefficient in seconds")
            models[key] = (row["time_unit"], coefficient)
        elif match["aggregate"] == "RMS":
            if key in residuals or row["time_unit"] or row["iterations"]:
                raise ReportError(f"{name}: duplicate or malformed RMS row.")
            residuals[key] = cpu
        else:
            unit = row["time_unit"]
            if unit not in UNITS:
                raise ReportError(f"{name}: unsupported timing unit {unit!r}.")
            if not row["iterations"].isdigit() or int(row["iterations"]) < 1:
                raise ReportError(f"{name}: invalid iteration count.")
            for counter in COUNTERS:
                number(row[counter], f"{name}: {counter}. {PERF_HELP}")
            n = int(match["n"])
            if n > 2**63 - 1:
                raise ReportError(f"{name}: input size exceeds Google Benchmark's signed 64-bit range.")
            points = observations.setdefault(key, {})
            if n in points:
                raise ReportError(f"{name}: duplicate size/repetition; one measurement per N is required.")
            seconds = number(cpu * UNITS[unit], f"{name}: CPU time in seconds")
            points[n] = Point(n, seconds)
            units.setdefault(key, set()).add(unit)
    keys = set(observations) | set(models) | set(residuals)
    if not keys:
        raise ReportError("CSV contains no benchmark series.")
    result = []
    for algorithm in sorted({key[0] for key in keys}):
        for distribution in DISTRIBUTIONS:
            key = (algorithm, distribution)
            if key not in observations or key not in models or key not in residuals:
                raise ReportError(f"{algorithm}/{distribution}: missing measurements, BigO, or RMS.")
            if len(observations[key]) < 2 or len(units[key]) != 1:
                raise ReportError(f"{algorithm}/{distribution}: need at least two sizes and one consistent time unit.")
            complexity, coefficient = models[key]
            rms = residuals[key] / UNITS[next(iter(units[key]))]
            if not math.isfinite(rms):
                raise ReportError(f"{algorithm}/{distribution}: RMS conversion overflow.")
            result.append(Series(algorithm, distribution,
                                 tuple(sorted(observations[key].values(), key=lambda p: p.n)),
                                 complexity, coefficient, rms))
    return tuple(result)


def load_csv(path: Path) -> tuple[Series, ...]:
    with path.open(encoding="utf-8", newline="") as stream:
        return parse_csv(stream)


def cell_color(distribution: str, complexity: str) -> str:
    rank = CLASSES.index(complexity)
    green_limit = CLASSES.index("N" if distribution == "sorted" else "NlgN")
    return COLORS["green" if rank <= green_limit else "yellow" if rank < 4 else "red"]


def comparison_groups(series: tuple[Series, ...]) -> list[tuple[str, tuple[Series, ...], bool]]:
    """Return focal class, members and whether nearest-class comparators were added."""
    groups = {complexity: tuple(s for s in series if s.complexity == complexity)
              for complexity in CLASSES}
    result = []
    for complexity, members in groups.items():
        if not members:
            continue
        cross_class = len(members) == 1
        if cross_class:
            other = [c for c in CLASSES if c != complexity and groups[c]]
            if not other:
                print(f"Warning: {members[0].distribution}/{complexity}: no other class to compare.",
                      file=sys.stderr)
                continue
            distance = min(abs(CLASSES.index(c) - CLASSES.index(complexity)) for c in other)
            members += tuple(s for c in other
                             if abs(CLASSES.index(c) - CLASSES.index(complexity)) == distance
                             for s in groups[c])
        result.append((complexity, tuple(sorted(members, key=lambda s: s.algorithm)), cross_class))
    return result


def common_range(series: tuple[Series, ...]) -> tuple[int, int]:
    """N interval measured by every algorithm in the plot (no extrapolation)."""
    lower = max(s.points[0].n for s in series)
    upper = min(s.points[-1].n for s in series)
    if lower >= upper:
        raise ReportError("Algorithms in one plot share fewer than two measured sizes.")
    return lower, upper


def algorithm_label(algorithm: str) -> str:
    standard_algorithms = {
        "std_sort_list": "std::list::sort",
        "std_sort_vector": "std::sort (vector)",
    }
    return standard_algorithms.get(algorithm, algorithm.replace("_", " "))


def save_figure(fig: plt.Figure, path: Path) -> None:
    try:
        fig.savefig(path, dpi=220, bbox_inches="tight", facecolor="white")
    finally:
        plt.close(fig)


def render_table(series: tuple[Series, ...], destination: Path, *, rms: bool) -> None:
    algorithms = sorted({s.algorithm for s in series})
    lookup = {(s.algorithm, s.distribution): s for s in series}
    rows, colors = [], []
    for algorithm in algorithms:
        row = [algorithm_label(algorithm)]
        row_colors = ["#f3f3f3"]
        for distribution in DISTRIBUTIONS:
            item = lookup[algorithm, distribution]
            row.append(f"{item.rms * 100:.2f}%" if rms else NOTATION[CLASSES.index(item.complexity)])
            row_colors.append("white" if rms else cell_color(distribution, item.complexity))
        rows.append(row)
        colors.append(row_colors)
    fig, ax = plt.subplots(figsize=(12, 1.6 + len(rows) * 0.42))
    ax.axis("off")
    table = ax.table(cellText=rows, colLabels=["Algorithm", *TITLES.values()],
                     cellColours=colors, colColours=["#dce6f1"] * 4,
                     cellLoc="center", bbox=[0, 0, 1, 1], colWidths=[0.28, 0.24, 0.24, 0.24])
    table.auto_set_font_size(False)
    table.set_fontsize(11)
    # No title/caption on purpose: the color rubric and RMS meaning (see module
    # docstring) go into the LaTeX caption.
    fig.subplots_adjust(left=0.01, right=0.99, bottom=0.01, top=0.99)
    save_figure(fig, destination)


def render_plot(series: tuple[Series, ...], colors: dict[str, tuple],
                title: str, destination: Path, *, logarithmic: bool) -> None:
    """One line of measured points per algorithm, cropped to the common N range.

    Only a short title, axis labels and the legend are drawn; methodology notes
    (common range, RMS) belong in the LaTeX text. Time is CPU time in ns.
    """
    lower, upper = common_range(series)
    fig, ax = plt.subplots(figsize=(11, 6.5))
    for item in series:
        color = colors[item.algorithm]
        label = f"{algorithm_label(item.algorithm)} [{NOTATION[CLASSES.index(item.complexity)]}]"
        points = [p for p in item.points if lower <= p.n <= upper]
        ax.plot([p.n for p in points], [p.cpu_seconds * 1e9 for p in points],
                color=color, marker="o", markersize=3.5, linewidth=1.5, label=label)
    if logarithmic:
        ax.set_xscale("log", base=2)
        ax.set_yscale("log")
    ax.set_xlabel("Number of elements, N")
    ax.set_ylabel("CPU Time (ns)")
    ax.set_title(title, fontsize=13)
    ax.grid(True, which="both", alpha=0.25)
    ax.legend(loc="upper left", bbox_to_anchor=(1.02, 1), fontsize=9, frameon=False)
    save_figure(fig, destination)


def owned_image_names() -> set[str]:
    names = {"benchmark_complexity.png", "benchmark_rms.png"}
    for distribution in DISTRIBUTIONS:
        for scale in ("linear", "loglog"):
            names.add(f"benchmark_overview_{distribution}_{scale}.png")
            names.update(f"benchmark_group_{distribution}_{name}_{scale}.png" for name in CLASS_NAMES)
    return names


def render_report(series: tuple[Series, ...], directory: Path) -> None:
    directory.mkdir(parents=True, exist_ok=True)
    algorithms = sorted({s.algorithm for s in series})
    palette = plt.get_cmap("tab20")
    colors = {name: palette(i % 20) for i, name in enumerate(algorithms)}
    render_table(series, directory / "benchmark_complexity.png", rms=False)
    render_table(series, directory / "benchmark_rms.png", rms=True)
    for distribution in DISTRIBUTIONS:
        items = tuple(s for s in series if s.distribution == distribution)
        comparisons = [("overview", items, TITLES[distribution] + " - all algorithms")]
        for complexity, members, cross_class in comparison_groups(items):
            name = CLASS_NAMES[CLASSES.index(complexity)]
            title = TITLES[distribution] + " - " + NOTATION[CLASSES.index(complexity)]
            title += " vs nearest fitted classes" if cross_class else " - same fitted class"
            comparisons.append((f"group_{name}", members, title))
        for name, members, title in comparisons:
            for scale in ("linear", "loglog"):
                stem = (f"overview_{distribution}" if name == "overview"
                        else f"group_{distribution}_{name.removeprefix('group_')}")
                render_plot(members, colors, title, directory / f"benchmark_{stem}_{scale}.png",
                            logarithmic=scale == "loglog")


def run_benchmark(root: Path, output: Path) -> None:
    """Explicit flags prevent environment defaults from changing the report shape."""
    subprocess.run(["./build.sh"], cwd=root, check=True)
    subprocess.run([
        "./out/benchmark_tool", "--benchmark_perf_counters=CYCLES,INSTRUCTIONS",
        f"--benchmark_out={output}", "--benchmark_out_format=csv",
        "--benchmark_time_unit=ns", "--benchmark_repetitions=1", "--benchmark_filter=.",
    ], cwd=root, check=True)


def publish(replacements: list[tuple[Path | None, Path]], backup: Path) -> None:
    """Rollback handled I/O failures, including removal of obsolete owned plots."""
    for _, target in replacements:
        if target.exists() and not target.is_file():
            raise ReportError(f"Cannot replace a non-file output: {target}")
    backup.mkdir()
    saved: list[tuple[Path, Path]] = []
    installed: list[Path] = []
    try:
        for index, (source, target) in enumerate(replacements):
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.exists():
                previous = backup / str(index)
                target.replace(previous)
                saved.append((previous, target))
            if source is not None:
                source.replace(target)
                installed.append(target)
    except OSError:
        for target in reversed(installed):
            target.unlink()
        for previous, target in reversed(saved):
            previous.replace(target)
        raise


def generate(*, plot_only: bool, root: Path = ROOT) -> int:
    """Stage validated data and complete figures before touching existing output."""
    out = root / "out"
    out.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix=".benchmark-report-", dir=out) as temporary:
        stage = Path(temporary)
        source_csv = root / CSV_PATH if plot_only else stage / "benchmark_result.csv"
        if not plot_only:
            run_benchmark(root, source_csv)
        series = load_csv(source_csv)
        images = stage / "images"
        render_report(series, images)
        replacements: list[tuple[Path | None, Path]] = []
        if not plot_only:
            replacements.append((source_csv, root / CSV_PATH))
        generated = {path.name: path for path in images.iterdir()}
        for name in sorted(owned_image_names()):
            target = root / IMAGE_PATH / name
            if name in generated or target.exists():
                replacements.append((generated.get(name), target))
        publish(replacements, stage / "backup")
        return len(generated)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--plot-only", action="store_true",
                        help="validate saved CSV and regenerate PNGs without building or benchmarking")
    args = parser.parse_args(argv)
    try:
        count = generate(plot_only=args.plot_only)
    except (ReportError, OSError, subprocess.CalledProcessError, csv.Error) as error:
        print(f"Report failed: {error}\nNo new complete report was published. "
              "Previous outputs are retained unless an I/O error prevented publication rollback.",
              file=sys.stderr)
        return 1
    print(f"Report ready: {ROOT / CSV_PATH}\nGenerated {count} PNG images in {ROOT / IMAGE_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
