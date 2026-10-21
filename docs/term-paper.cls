% Reusable term-paper class for consistent formatting across projects.
% Author: Oleksii Chernykh (o.f.chernykh@khai.edu)
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{term-paper}[2026/10/13 Reusable term-paper class]

% Select the main document language. Both languages remain available through babel.
% Ukrainian is the default; use \documentclass[english]{term-paper} for English.
% This option controls automatic labels, hyphenation, and language conventions.
\newcommand{\term@language}{ukrainian}
\DeclareOption{ukrainian}{\renewcommand{\term@language}{ukrainian}}
\DeclareOption{english}{\renewcommand{\term@language}{english}}
\ProcessOptions*\relax

\LoadClass[a4paper,14pt]{extreport}

% This class holds shared term-paper formatting. Add graphics/table packages here
% only when a project needs them; keep project-specific settings in setup.sty.
% fontspec is required for system fonts and means this class must be compiled
% with XeLaTeX or LuaLaTeX, not pdfLaTeX.
\RequirePackage{fontspec}
% Load both languages so Ukrainian and English text can coexist; the class
% option selects the default language for captions and hyphenation.
\RequirePackage[english,ukrainian]{babel}
\AtBeginDocument{\selectlanguage{\term@language}}
% Use the thesis template's requested fonts; replace these only if the
% institution's approved template specifies different typefaces.
\setmainfont{Times New Roman}
\setsansfont{Courier New}
\setmonofont{Courier New}

% extreport's 14pt option sets 14.4pt; enforce exactly 14pt while retaining
% the class's 17pt baseline and the one-and-a-half line spacing below.
% The after-hook reapplies 14pt whenever LaTeX restores \normalsize; the
% begin-document hook fixes the initial size after packages set their defaults.
\AddToHook{cmd/normalsize/after}{\fontsize{14}{17}\selectfont}
\AddToHook{begindocument/end}{\normalsize}
\normalsize

\RequirePackage{geometry}
% Shared page layout based on the current thesis requirements: A4, left margin
% 25 mm, right margin 15 mm, and top/bottom margins 20 mm. includehead keeps
% the top-right page number inside the measured page layout.
\geometry{
  a4paper,
  left=25mm,
  right=15mm,
  top=20mm,
  bottom=20mm,
  includehead
}

\RequirePackage{fancyhdr}
\setlength{\headheight}{17pt} % Reserve enough room for a 14pt page number in the header.
\fancyhf{}                    % Clear the class/package default header and footer fields.
\fancyhead[R]{\thepage}       % Put the page number at the top-right, as required by the template.
\renewcommand{\headrulewidth}{0pt} % Do not draw a decorative rule below the header.
% LaTeX uses "plain" on some heading/opening pages; keep numbering position
% consistent there rather than letting it fall back to a centered footer.
\fancypagestyle{plain}{%
  \fancyhf{}%
  \fancyhead[R]{\thepage}%
  \renewcommand{\headrulewidth}{0pt}%
}
\pagestyle{fancy} % Use the configured top-right page-number style throughout the body.

\RequirePackage{setspace}
\onehalfspacing % Required 1.5 line spacing; with 14pt text the baseline is 21.25pt.
\newlength{\thesisparindent}
\setlength{\thesisparindent}{1.25cm} % Shared 1.25 cm paragraph and numbered-heading indent.
\setlength{\parindent}{\thesisparindent}
\clubpenalty=10000 % Prevent a paragraph's first line being left alone at a page bottom.
\widowpenalty=10000 % Prevent a paragraph's last line being left alone at a page top.

% csquotes provides language-aware quotation commands such as \enquote.
\RequirePackage{csquotes}
% graphicx is the shared foundation for including figures; specialized figure
% packages should be added here only when a project actually needs them.
\RequirePackage{graphicx}
% tocloft and titlesec provide the customized contents and heading formats below.
\RequirePackage{tocloft}
\RequirePackage{titlesec}
% The first body paragraph after a heading must use the same paragraph indent.
\RequirePackage{indentfirst}
% Hyperlinks are useful in the contents; hide link borders/colours for print.
\RequirePackage[hidelinks,linktoc=all]{hyperref}

% Use simple Arabic section numbers (1, 2, ...) instead of chapter.section numbers.
\renewcommand{\thesection}{\arabic{section}}

% Unnumbered sections are included in the TOC automatically. An optional short
% title is used in the TOC; numbered sections retain the standard behavior.
% Example: \section*[Introduction]{INTRODUCTION} prints INTRODUCTION but lists
% Introduction in the contents, without a separate \addcontentsline call.
\makeatletter
\let\term@originalsection\section
\RenewDocumentCommand{\section}{s o m}{%
  \IfBooleanTF{#1}{%
    \IfNoValueTF{#2}{%
      \term@originalsection*{#3}%
      \phantomsection\addcontentsline{toc}{section}{#3}%
    }{%
      \term@originalsection*{#3}%
      \phantomsection\addcontentsline{toc}{section}{#2}%
    }%
  }{%
    \IfNoValueTF{#2}{\term@originalsection{#3}}{\term@originalsection[#2]{#3}}%
  }%
}
\makeatother

% Table of contents title and compact number-to-title spacing.
\setlength{\cftbeforetoctitleskip}{0pt} % Align the contents title with the top-of-page layout.
\renewcommand{\cfttoctitlefont}{\normalfont\fontsize{14}{17}\selectfont\bfseries} % Match body size and make the TOC title bold.
\renewcommand{\cftaftertoctitle}{\centering\par} % Center the contents title.
\addto\captionsukrainian{\renewcommand{\contentsname}{ЗМІСТ}} % Keep Ukrainian TOC heading uppercase.
\addto\captionsenglish{\renewcommand{\contentsname}{CONTENTS}} % Use the matching uppercase English heading.
\renewcommand{\cftsecfont}{\bfseries} % Make level-1 section entries (number and title) bold in the TOC.
\renewcommand{\cftsecpagefont}{\bfseries} % Keep the page number bold with its level-1 section entry.
\setlength{\cftsecnumwidth}{0pt} % Avoid a wide reserved number column before section titles.
\setlength{\cftsubsecnumwidth}{0pt} % Avoid a wide reserved number column before subsection titles.
\tocloftpagestyle{empty} % Hide the page number on the first contents page.

% Keep title page and contents in the page count without displaying their numbers.
% The first body page continues numbering at 3; TOC pages use empty style throughout.
% Redefine titlepage so extreport does not reset the page counter after the cover.
\RenewDocumentEnvironment{titlepage}{}{%
  \clearpage
  \thispagestyle{empty}%
}{%
  \clearpage
}
% Clear all TOC page numbers, including continuation pages, then restore the
% normal body style. Scope the compact \numberline spacing to the contents only
% so lists of figures/tables keep their standard number formatting.
\AddToHook{cmd/tableofcontents/before}{%
  \clearpage
  \begingroup
  \pagestyle{empty}%
  \renewcommand{\numberline}[1]{#1\nobreakspace}%
}
\AddToHook{cmd/tableofcontents/after}{%
  \clearpage
  \endgroup
  \pagestyle{fancy}%
}

% Metadata accessors used by the reusable title-page file.
% These expose LaTeX's \title/\author/\date values to title.tex. \supervisor
% is separate because the standard document classes do not define that field.
\makeatletter
\providecommand{\thetitle}{\@title}
\providecommand{\theauthor}{\@author}
\providecommand{\thedate}{\@date}
\newcommand{\@supervisor}{TBD}
\newcommand{\supervisor}[1]{\renewcommand{\@supervisor}{#1}}
\newcommand{\thesupervisor}{\@supervisor}
\makeatother

% Center and bold unnumbered section headings (e.g. INTRODUCTION).
\titleformat{name=\section,numberless}[block]
{\normalfont\fontsize{14}{17}\selectfont\bfseries\centering}
{}{0pt}{}
% Numbered section and subsection labels start at the same 1.25 cm indent as
% paragraphs; this is a template layout rule, separate from TOC indentation.
\titleformat{\section}[block]
{\normalfont\fontsize{14}{17}\selectfont\bfseries}
{\hspace*{\thesisparindent}\thesection}{0.5em}{}
\titleformat{\subsection}[block]
{\normalfont\fontsize{14}{17}\selectfont\bfseries}
{\hspace*{\thesisparindent}\thesubsection}{0.5em}{}
