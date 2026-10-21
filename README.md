# sort-benchmark

A small C++23 benchmark suite that measures the performance of classic sorting algorithms
using [Google Benchmark](https://github.com/google/benchmark).

The project was created for the **"Algorithm efficiency evaluation"** practical assignment
of the course **"Processing and Analysis of Research Results Using IT"**. Its goal is to
produce reproducible timing data (console, JSON or CSV) that can be analysed further and
compared against the theoretical complexity of each algorithm.

## What is benchmarked

Every algorithm is run on three kinds of input — **random**, **already sorted** and
**reverse-sorted** — starting at 128 elements and doubling each step. Bubble sort,
selection sort, and the random/reverse-sorted variants of gnome and insertion sort
stop at 4 096; most other series reach 65 536, and radix sort reaches 262 144.
Google Benchmark's complexity estimator reports the best-fitting Big-O and RMS error for each series.

| Benchmark prefix   | Algorithm                                   |
|--------------------|---------------------------------------------|
| `bubble_sort`      | Bubble sort                                 |
| `gnome_sort`       | Gnome sort                                  |
| `heap_sort`        | Heap sort                                   |
| `insertion_sort`   | Insertion sort                              |
| `intro_sort`       | Introsort (quick + heap + insertion)        |
| `merge_sort`       | Merge sort                                  |
| `quick_sort`       | Quick sort (iterative, middle pivot)        |
| `radix_sort`       | Radix sort                                  |
| `selection_sort`   | Selection sort                              |
| `shell_sort`       | Shell sort                                  |
| `std_sort_vector`  | `std::sort` on `std::vector`                |
| `std_sort_list`    | `std::list::sort`                           |
| `tree_sort`        | Tree sort via `std::multiset`               |

Each prefix has three variants: `<prefix>_random`, `<prefix>_sorted`, `<prefix>_reverse_sorted`.

## Prerequisites

The CMake presets are written for **Linux only**.

- CMake **4.2** or newer
- [Ninja](https://ninja-build.org/)
- Clang / `clang++` with **libc++** and the **lld** linker (the presets use `-stdlib=libc++ -fuse-ld=lld`)
- libpfm development headers (`libpfm4-dev` on Debian/Ubuntu, `libpfm-devel` on Fedora) — Google Benchmark
  is built with `BENCHMARK_ENABLE_LIBPFM=ON`
- Internet access on the first configure: Google Benchmark v1.9.5 is downloaded via `FetchContent`

## Project structure

```
.
├── CMakeLists.txt             # top-level project (C++23, install rules)
├── CMakePresets.json       # release-linux / debug-linux presets
├── build.sh                         # one-shot configure + build + install
├── cmake/
│   └── google_benchmark.cmake
├── benchmark/
│   ├── CMakeLists.txt     # benchmark_tool executable
│   └── src/
│       ├── main.cpp           # Google Benchmark entry point
│       └── tests/                 # one .cpp per sorting algorithm
├── .build/<preset>/          # build directory (generated)
└── out/                             # installed benchmark_tool + results (generated)
```

## Building

### Quick way

```sh
./build.sh                         # release-linux (default)
./build.sh debug-linux    # debug build
```

The script configures, builds and installs the executable into `./out/`.

### Manual steps

```sh
cmake --preset "release-linux"
cmake --build --preset "release-linux"
cmake --install ./.build/release-linux
```

Replace `release-linux` with `debug-linux` for a debug build.
The resulting binary is `./out/benchmark_tool`.

## Running the benchmark

Run every benchmark (this takes a while — the O(n²) sorts go up to 65 536 elements):

```sh
./out/benchmark_tool
```

List all available benchmarks without running them:

```sh
./out/benchmark_tool --benchmark_list_tests
```

Run only a subset using a regular expression filter:

```sh
./out/benchmark_tool --benchmark_filter="quick_sort*|merge_sort*"
```

### Exporting results

JSON:

```sh
./out/benchmark_tool --benchmark_out=./out/result.json --benchmark_out_format=json
```

CSV:

```sh
./out/benchmark_tool --benchmark_out=./out/result.csv --benchmark_out_format=csv
```

Filters and export can be combined:

```sh
./out/benchmark_tool --benchmark_filter="heap_sort*" --benchmark_out=./out/heap.csv --benchmark_out_format=csv
```

Console output is still printed when exporting to a file.

### Hardware performance counters (CYCLES, INSTRUCTIONS)

Google Benchmark is built with libpfm support, so hardware counters can be collected alongside timings.
Pass a comma-separated list of counter names:

```sh
./out/benchmark_tool --benchmark_filter="quick_sort_random" --benchmark_perf_counters=CYCLES,INSTRUCTIONS
```

The counters appear in the `UserCounters...` column (values are per iteration):

```
----------------------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------
quick_sort_random/128         3011 ns         2994 ns       267748 CYCLES=4.09416k INSTRUCTIONS=13.696k
quick_sort_random/1024       18906 ns        18734 ns        37531 CYCLES=51.4977k INSTRUCTIONS=157.511k
quick_sort_random/8192      334403 ns       332363 ns         1531 CYCLES=1.24303M INSTRUCTIONS=1.61534M
quick_sort_random/65536    4231411 ns      4206368 ns          200 CYCLES=13.0506M INSTRUCTIONS=15.8085M
quick_sort_random_BigO        4.03 NlgN       4.00 NlgN
quick_sort_random_RMS            4 %             4 %
```

Counters are also written to the JSON/CSV output when combined with `--benchmark_out`.
Other libpfm event names (e.g. `BRANCHES`, `BRANCH-MISSES`, `CACHE-MISSES`) can be used as well,
depending on what your CPU exposes.

If you get an error like `Unable to open perf event` or no counters appear, your kernel may restrict
access to perf events for non-root users. Either run the tool with `sudo` or lower the restriction:

```sh
sudo sysctl kernel.perf_event_paranoid=1
```

## Generating report images

The Python report generator builds the suite, runs **all** benchmarks with
`CYCLES,INSTRUCTIONS`, validates the native CSV, and generates PNG tables and
plots for use in a LaTeX report. It is run with [uv](https://docs.astral.sh/uv/):
Python 3.10+ and Matplotlib are declared inline in the script (PEP 723), so uv
creates and caches an isolated environment automatically — no manual
`pip install` or virtual environment is needed:

```sh
uv run benchmark_report.py
# or, using the uv shebang:
./benchmark_report.py
```

No display server is needed (the script uses Agg).
The script resolves paths from its own location, so it can also run from another
working directory. It does not change the sorting implementations or edit LaTeX.
The full measurement run can take a while.

Outputs:

| Path | Purpose |
|------|---------|
| `docs/result/benchmark_result.csv` | Native Google Benchmark output, including timings, counters, Big-O and RMS |
| `docs/result/benchmark_complexity.png` | Algorithms versus Random, Sorted and Reverse-sorted input, with color-coded empirical Big-O |
| `docs/result/benchmark_rms.png` | Normalized CPU model fitting error in percent |
| `docs/result/benchmark_overview_<input>_<scale>.png` | All algorithms, separately for each input distribution |
| `docs/result/benchmark_group_<input>_<class>_<scale>.png` | Comparisons by fitted complexity, or singleton versus nearest fitted classes |

`<input>` is `random`, `sorted`, or `reverse_sorted`; `<scale>` is `linear` or
`loglog`. Class names are `constant`, `log_n`, `n`, `n_log_n`, `n_squared`, and
`n_cubed`. The set of group plots depends on the measured fits. For a singleton
class, comparators are all algorithms in the nearest available class by position
in that order; equally close classes are both included. If there are no
comparators at all, the script warns and retains only the overview.

To regenerate images from the existing CSV without building or measuring:

```sh
uv run benchmark_report.py --plot-only
```

### Reading the figures

- Every time plot uses **CPU time per iteration**, in nanoseconds. This reduces
  waiting-related noise compared with wall-clock time, but does not remove cache,
  CPU frequency, or scheduling effects.
- Both **linear** and **log-log** views are produced: the former shows absolute
  differences, the latter makes disparate magnitudes and growth easier to see.
- Lines with circles connect actual measurements only; there is **no
  extrapolation**. Each plot is cropped to the N range measured by **all**
  algorithms on it (largest minimum N to smallest maximum N). For example, an
  overview that includes quadratic algorithms stops at N = 4 096, while a group
  of fast algorithms can reach 65 536.
- On linear axes, small sizes are squashed into the bottom-left corner and may
  look like points at (0, 0); there is no N = 0 benchmark.
- Images carry only a short title, axis labels and a legend; explanations go
  into the LaTeX captions.
- Big-O is an **empirical fit over tested sizes**, not a proven asymptotic bound.
  Reverse-sorted input is **not necessarily the worst case** for an implementation.
  RMS is root-mean-square residual divided by mean measured time; it describes
  fit quality.
- Complexity colors are a chosen rubric: Random and Reverse-sorted are green
  through O(N log N); Sorted is green only through O(N). Remaining subquadratic
  fits are yellow, and O(N²) or higher is red.

The module docstring and function documentation in `benchmark_report.py` preserve
these methodological explanations for future report writing and AI-assisted
analysis. The parser handles the context preamble and the unusual RMS encoding
in this repository's **Google Benchmark v1.9.5** CSV. Plot-only is intended for
CSV produced by this workflow, with the default Big-O coefficient unit fixed to
nanoseconds. Repeated samples and statistical aggregate rows are explicitly
rejected rather than silently averaged.

Example insertion from `docs/main.tex` (with `graphicx` loaded):

```tex
\begin{figure}[htbp]
  \centering
  \includegraphics[width=\linewidth]{result/benchmark_overview_random_loglog.png}
  \caption{CPU time on random input over the N range measured by all algorithms.}
\end{figure}
```

### Failure handling

Both hardware counters must be present in every measurement row. If perf
permissions prevent collection, the script stops with an explanation (see the
hardware-counter section above). On any failure the previous CSV and images are
kept unchanged.

## Example output

```
$ ./out/benchmark_tool --benchmark_filter="quick_sort_random"
Run on (16 X 5263.06 MHz CPU s)
...
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
quick_sort_random/128         1581 ns         1566 ns       377805
quick_sort_random/1024       15977 ns        15916 ns        59742
quick_sort_random/8192      404518 ns        403096 ns         1763
quick_sort_random/65536    4137962 ns      4124679 ns          170
quick_sort_random_BigO        3.94 NlgN       3.93 NlgN
quick_sort_random_RMS            1 %             1 %
```

- **Time** — wall-clock time per iteration, **CPU** — CPU time per iteration.
- **Iterations** — how many times the benchmark was repeated to get a stable measurement.
- **`_BigO`** — the complexity class that best fits the measured series (here *N·log N*);
  **`_RMS`** — the fitting error.

## Tips for stable measurements

- Google Benchmark prints `***WARNING*** CPU scaling is enabled` if the CPU frequency governor is
  dynamic. For less noisy numbers switch to the `performance` governor, e.g.
  `sudo cpupower frequency-set --governor performance`.
- Use `--benchmark_repetitions=N` to repeat each benchmark and get mean / median / stddev rows.
- Use `--benchmark_min_time=<n>s` to lengthen individual runs if iteration counts look low.
- Run `./out/benchmark_tool --help` for the full list of Google Benchmark options.
