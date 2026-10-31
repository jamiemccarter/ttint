#!/usr/bin/env python3
"""Mechanical style checks for TTint (see docs/STYLE_GUIDE.md, "Enforcement").

    scripts/check_style.py                  # every enabled check
    scripts/check_style.py all              # every check, including opt-in ones
    scripts/check_style.py format tokens    # selected checks
    scripts/check_style.py --list

Checks:
    format  clang-format --dry-run over include/, tests/ and examples/
    tokens  no `typedef`, no `NULL`, no leading-underscore identifiers
    cases   doctest TEST_CASE names are Domain.Feature.Behavior
    naming  clang-tidy readability-identifier-naming (.clang-tidy)
    readme  README code blocks match examples/ and have screenshots

Exit status:
    0   every selected check passed
    1   a check found violations, or a required tool is missing (--require-tools)
    2   usage error
    77  nothing failed, but a check was skipped because its tool is not installed
        (CTest's SKIP_RETURN_CODE; CI should pass --require-tools instead)
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

EXIT_OK = 0
EXIT_FAILED = 1
EXIT_USAGE = 2
EXIT_SKIPPED = 77

SOURCE_SUFFIXES = {".cpp", ".hpp"}
SOURCE_DIRS = ("include", "tests", "examples")

# Naming covers the public headers, the examples and the test support headers.
# Test sources (.cpp) are excluded: doctest macros generate identifiers that the
# project cannot rename, and the test names themselves follow the
# Domain.Feature.Behavior convention instead.
NAMING_FILES = ("include/TTint/**/*.hpp", "examples/**/*.cpp", "tests/*.hpp")

# Identifiers that start with an underscore because a platform or the language
# dictates it. The rule is about names TTint declares, so uses of these are fine.
RESERVED_NAMES_IN_USE = frozenset(
    {
        "_WIN32",
        "_WIN64",
        "_MSC_VER",
        "_isatty",
        "_fileno",
        "_putenv_s",
        "__cplusplus",
        "__has_include",
        "__has_cpp_attribute",
    }
)


@dataclass
class Result:
    """Outcome of one check: violations to print, or a reason it did not run."""

    problems: list
    skipped: str = ""


# --------------------------------------------------------------------------- files


def repo_sources(root=ROOT):
    """Tracked and untracked-but-not-ignored C++ files under the source dirs."""
    try:
        listing = subprocess.run(
            ["git", "ls-files", "--cached", "--others", "--exclude-standard", "--", *SOURCE_DIRS],
            cwd=root,
            capture_output=True,
            text=True,
            check=True,
        ).stdout.split("\n")
        candidates = [root / name for name in listing if name]
    except (OSError, subprocess.CalledProcessError):
        candidates = [path for directory in SOURCE_DIRS for path in (root / directory).rglob("*")]
    # A path that git still lists but that was deleted from the working tree is
    # not a violation.
    return sorted(p for p in candidates if p.suffix in SOURCE_SUFFIXES and p.is_file())


def naming_sources(root=ROOT):
    return sorted(path for pattern in NAMING_FILES for path in root.glob(pattern))


def case_sources(root=ROOT):
    return sorted((root / "tests").glob("*.cpp"))


# -------------------------------------------------------------------------- tools


def find_tool(env_var, names, extra_dirs):
    """Resolves an executable: $ENV, then PATH, then well-known LLVM locations."""
    override = os.environ.get(env_var)
    if override:
        found = shutil.which(override)
        if found is None:
            print(f"warning: ${env_var}={override} is not an executable", file=sys.stderr)
        return found
    for name in names:
        found = shutil.which(name)
        if found:
            return found
    for directory in extra_dirs:
        for name in names:
            candidate = Path(directory) / name
            if candidate.is_file() and os.access(candidate, os.X_OK):
                return str(candidate)
    return None


# Homebrew's LLVM is keg-only, so its tools are installed but not on PATH.
LLVM_DIRS = ("/opt/homebrew/opt/llvm/bin", "/usr/local/opt/llvm/bin", "/usr/lib/llvm/bin")


def find_clang_format():
    return find_tool("CLANG_FORMAT", ["clang-format"], LLVM_DIRS)


def find_clang_tidy():
    return find_tool("CLANG_TIDY", ["clang-tidy"], LLVM_DIRS)


def resolve_override(path):
    found = shutil.which(path)
    if found is None:
        print(f"warning: {path} is not an executable", file=sys.stderr)
    return found


# ------------------------------------------------------------------------- format


def check_format(files, args):
    tool = resolve_override(args.clang_format) if args.clang_format else find_clang_format()
    if tool is None:
        return Result([], skipped="clang-format not found (set CLANG_FORMAT or pass --clang-format)")
    problems = []
    # Batches keep the command line short on platforms with small limits.
    for start in range(0, len(files), 50):
        batch = [str(f) for f in files[start : start + 50]]
        proc = subprocess.run(
            [tool, "--dry-run", "--Werror", f"-style=file:{ROOT / '.clang-format'}", *batch],
            capture_output=True,
            text=True,
        )
        if proc.returncode != 0:
            problems.extend(line for line in proc.stderr.splitlines() if "error:" in line or "Error" in line)
            if not problems:
                problems.append(proc.stderr.strip() or "clang-format failed")
    return Result(problems)


# ------------------------------------------------------------------------- tokens

# Alternatives in match order: comments, literals, then angle-bracket includes.
# Everything matched is blanked (newlines kept) so line numbers stay correct and
# the later scan only sees code.
_BLANKED = re.compile(
    r"""
      //(?:\\\n|[^\n])*                         # line comment (a backslash continues it)
    | /\*.*?\*/                                 # block comment
    | R"(?P<delim>[^()\\\s]{0,16})\(.*?\)(?P=delim)"   # raw string literal
    | "(?:\\.|[^"\\\n])*"                       # string literal
    | (?<![A-Za-z0-9_])(?:u8|u|U|L)?'(?:\\.|[^'\\\n])+'  # char literal; not the ' in 1'000
    | (?m:^[ \t]*\#[ \t]*include[ \t]*<[^>\n]*>)  # #include <...>
    """,
    re.VERBOSE | re.DOTALL,
)

_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")


def blank_non_code(text):
    return _BLANKED.sub(lambda m: re.sub(r"[^\n]", " ", m.group(0)), text)


def scan_tokens(text):
    """Yields (line_number, message) for each banned token in code."""
    code = blank_non_code(text)
    for number, line in enumerate(code.split("\n"), start=1):
        for match in _IDENTIFIER.finditer(line):
            word = match.group(0)
            if word == "typedef":
                yield number, "`typedef` is not allowed; use `using`"
            elif word == "NULL":
                yield number, "`NULL` is not allowed; use `nullptr`"
            elif word.startswith("_") and word not in RESERVED_NAMES_IN_USE:
                yield number, f"identifier `{word}` starts with an underscore (reserved); non-public storage ends with `_`"


def check_tokens(files, args):
    problems = []
    for path in files:
        try:
            text = Path(path).read_text(encoding="utf-8", errors="replace")
        except OSError as error:
            problems.append(f"{display(path)}: cannot read: {error.strerror}")
            continue
        for number, message in scan_tokens(text):
            problems.append(f"{display(path)}:{number}: {message}")
    return Result(problems)


# ------------------------------------------------------------------------- cases

_TEST_CASE = re.compile(r'\bTEST_CASE\(\s*"(?P<name>[^"]*)"')
_CASE_NAME = re.compile(r"[A-Z][A-Za-z0-9]*(\.[A-Z][A-Za-z0-9]*){2}")


def scan_case_names(text):
    """Yields (line_number, message) for each TEST_CASE not named Domain.Feature.Behavior."""
    for match in _TEST_CASE.finditer(text):
        name = match.group("name")
        if not _CASE_NAME.fullmatch(name):
            number = text.count("\n", 0, match.start()) + 1
            yield number, f'test case "{name}" is not named Domain.Feature.Behavior'


def check_cases(files, args):
    problems = []
    for path in files:
        try:
            text = Path(path).read_text(encoding="utf-8", errors="replace")
        except OSError as error:
            problems.append(f"{display(path)}: cannot read: {error.strerror}")
            continue
        for number, message in scan_case_names(text):
            problems.append(f"{display(path)}:{number}: {message}")
    return Result(problems)


# ------------------------------------------------------------------------- naming

_TIDY_DIAGNOSTIC = re.compile(r"^(?P<where>.+?:\d+:\d+): (?P<severity>error|warning): (?P<message>.*)$")


def tidy_one(tool, path):
    proc = subprocess.run(
        [
            tool,
            f"--config-file={ROOT / '.clang-tidy'}",
            "--quiet",
            str(path),
            "--",
            "-x",
            "c++",
            "-std=c++20",
            f"-I{ROOT / 'include'}",
        ],
        capture_output=True,
        text=True,
    )
    diagnostics = []
    for line in proc.stdout.splitlines():
        match = _TIDY_DIAGNOSTIC.match(line)
        if match:
            diagnostics.append(f"{match['where']}: {match['message']}")
    # A translation unit that fails to parse is a failure, not "no naming problems".
    if proc.returncode != 0 and not diagnostics:
        diagnostics.append(f"{display(path)}: clang-tidy failed: {(proc.stderr or proc.stdout).strip()}")
    return diagnostics


def check_naming(files, args):
    tool = resolve_override(args.clang_tidy) if args.clang_tidy else find_clang_tidy()
    if tool is None:
        return Result([], skipped="clang-tidy not found (set CLANG_TIDY or pass --clang-tidy)")
    # Headers are analyzed as main files and also through every file that
    # includes them, so the same diagnostic can appear several times.
    with ThreadPoolExecutor() as pool:
        results = pool.map(lambda f: tidy_one(tool, f), files)
    return Result(sorted({d for diagnostics in results for d in diagnostics}))


# ------------------------------------------------------------------------- readme


def check_readme(files, args):
    proc = subprocess.run(
        [sys.executable, str(ROOT / "scripts" / "check_readme_examples.py")],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    if proc.returncode == 0:
        return Result([])
    return Result([line for line in (proc.stdout + proc.stderr).splitlines() if line.strip()])


# ---------------------------------------------------------------------- driver

# name -> (function, default file selection, enabled by default, description)
# `naming` stays opt-in until the public headers and examples conform to the
# guide: it reports real violations today, and the default command must pass on
# a clean tree. Flip it here when the tree conforms.
CHECKS = {
    "format": (check_format, repo_sources, True, "clang-format --dry-run"),
    "tokens": (check_tokens, repo_sources, True, "typedef / NULL / leading underscore"),
    "cases": (check_cases, case_sources, True, "doctest case names are Domain.Feature.Behavior"),
    "naming": (check_naming, naming_sources, False, "clang-tidy readability-identifier-naming"),
    "readme": (check_readme, lambda: [], True, "README <-> examples <-> screenshots"),
}


def display(path):
    try:
        return str(Path(path).resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def parse_args(argv):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("checks", nargs="*", help=f"checks to run ({', '.join(CHECKS)}); default: all enabled")
    parser.add_argument("--list", action="store_true", help="list checks and whether they are enabled")
    parser.add_argument("--paths", nargs="+", type=Path, help="check these files instead of the repository's")
    parser.add_argument(
        "--require-tools",
        action="store_true",
        default=os.environ.get("TTINT_REQUIRE_TOOLS") == "1",
        help="a missing tool is a failure, not a skip (also: TTINT_REQUIRE_TOOLS=1)",
    )
    parser.add_argument("--clang-format", help="clang-format executable")
    parser.add_argument("--clang-tidy", help="clang-tidy executable")
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(sys.argv[1:] if argv is None else argv)
    if "all" in args.checks and len(args.checks) > 1:
        print("`all` cannot be combined with other check names", file=sys.stderr)
        return EXIT_USAGE
    unknown = [name for name in args.checks if name not in CHECKS and name != "all"]
    if unknown:
        print(f"unknown check: {', '.join(unknown)} (choose from {', '.join(CHECKS)})", file=sys.stderr)
        return EXIT_USAGE
    if args.list:
        for name, (_, _, enabled, description) in CHECKS.items():
            print(f"{name:7} {'enabled ' if enabled else 'opt-in  '} {description}")
        return EXIT_OK

    if args.checks == ["all"]:
        selected = list(CHECKS)
    else:
        selected = args.checks or [name for name, spec in CHECKS.items() if spec[2]]
    failed = skipped = False
    for name in selected:
        function, default_files, _, _ = CHECKS[name]
        files = args.paths if args.paths is not None else default_files()
        if name != "readme" and not files:
            # An empty selection would otherwise read as "no violations".
            print(f"FAIL {name}: no files to check")
            failed = True
            continue
        result = function(files, args)
        if result.skipped:
            if args.require_tools:
                print(f"FAIL {name}: {result.skipped}")
                failed = True
            else:
                print(f"SKIP {name}: {result.skipped}")
                skipped = True
        elif result.problems:
            print(f"FAIL {name}")
            for problem in result.problems:
                print(f"  {problem}")
            failed = True
        else:
            print(f"ok   {name}")

    if failed:
        return EXIT_FAILED
    return EXIT_SKIPPED if skipped else EXIT_OK


if __name__ == "__main__":
    sys.exit(main())
