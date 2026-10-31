#!/usr/bin/env python3
"""Self-test for scripts/check_style.py.

Runs the real command line against fixtures: every check must fail on a
deliberate violation and pass on a clean file, so a check that silently stops
matching anything is caught here. Fixtures are stored as `*.fixture` so the
repository's own format/token scans never see them.

Set TTINT_REQUIRE_TOOLS=1 to fail, instead of skip, when clang-format or
clang-tidy is missing.
"""

import contextlib
import io
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
SCRIPT = ROOT / "scripts" / "check_style.py"
FIXTURES = HERE / "fixtures"

sys.path.insert(0, str(ROOT / "scripts"))
import check_style  # noqa: E402

REQUIRE_TOOLS = os.environ.get("TTINT_REQUIRE_TOOLS") == "1"


# CI sets TTINT_REQUIRE_TOOLS=1; the cases below choose their own policy.
CLEAN_ENV = {k: v for k, v in os.environ.items() if k != "TTINT_REQUIRE_TOOLS"}


def run_check(check, *paths, extra=()):
    result = subprocess.run(
        [sys.executable, str(SCRIPT), check, "--paths", *map(str, paths), *extra],
        capture_output=True,
        text=True,
        cwd=ROOT,
        env=CLEAN_ENV,
    )
    return result.returncode, result.stdout + result.stderr


class FixtureCase(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="ttint-style-"))
        self.addCleanup(shutil.rmtree, self.tmp, ignore_errors=True)

    def fixture(self, name):
        """Copies `<name>.fixture` into the temp dir under its real extension."""
        destination = self.tmp / name
        shutil.copy(FIXTURES / (name + ".fixture"), destination)
        return destination

    def require_tool(self, finder):
        if finder() is None:
            if REQUIRE_TOOLS:
                self.fail("required tool is missing")
            self.skipTest("tool not installed")


class TokenChecks(FixtureCase):
    def test_typedef_is_reported(self):
        code, out = run_check("tokens", self.fixture("tokens_bad_typedef.hpp"))
        self.assertEqual(code, 1, out)
        self.assertIn("typedef", out)
        self.assertIn(":3:", out)  # the line, not just the file

    def test_null_is_reported(self):
        code, out = run_check("tokens", self.fixture("tokens_bad_null.cpp"))
        self.assertEqual(code, 1, out)
        self.assertIn("NULL", out)
        self.assertIn(":2:", out)

    def test_leading_underscore_identifiers_are_reported(self):
        code, out = run_check("tokens", self.fixture("tokens_bad_underscore.hpp"))
        self.assertEqual(code, 1, out)
        self.assertIn("_leading", out)
        self.assertIn("__double_leading", out)

    def test_code_after_a_prefixed_char_literal_is_still_scanned(self):
        code, out = run_check("tokens", self.fixture("tokens_bad_after_prefixed_char.cpp"))
        self.assertEqual(code, 1, out)
        self.assertIn(":2:", out)
        self.assertIn("NULL", out)

    def test_unreadable_file_is_a_finding_not_a_crash(self):
        code, out = run_check("tokens", self.tmp / "missing.cpp")
        self.assertEqual(code, 1, out)
        self.assertIn("cannot read", out)
        self.assertNotIn("Traceback", out)

    def test_empty_selection_fails(self):
        # A glob or listing that matches nothing must not read as "no violations".
        empty = (lambda files, args: check_style.Result([]), lambda: [], True, "")
        with mock.patch.dict(check_style.CHECKS, {"tokens": empty}), contextlib.redirect_stdout(io.StringIO()) as out:
            code = check_style.main(["tokens"])
        self.assertEqual(code, check_style.EXIT_FAILED)
        self.assertIn("no files to check", out.getvalue())

    def test_comments_strings_and_platform_names_are_ignored(self):
        code, out = run_check("tokens", self.fixture("tokens_clean.cpp"))
        self.assertEqual(code, 0, out)

    def test_repository_sources_are_clean(self):
        code, out = run_check_default("tokens")
        self.assertEqual(code, 0, out)


class CaseNameChecks(FixtureCase):
    def test_free_form_name_is_reported(self):
        code, out = run_check("cases", self.fixture("cases_bad.cpp"))
        self.assertEqual(code, 1, out)
        self.assertIn("render_ansi with simple segments", out)
        self.assertIn(":3:", out)

    def test_structured_name_passes(self):
        code, out = run_check("cases", self.fixture("cases_clean.cpp"))
        self.assertEqual(code, 0, out)

    def test_split_declaration_and_digits(self):
        self.assertEqual(list(check_style.scan_case_names('TEST_CASE(\n    "bad name") {}')), [(1, 'test case "bad name" is not named Domain.Feature.Behavior')])
        self.assertEqual(list(check_style.scan_case_names('TEST_CASE("Color.Rgb8.Clamps") {}')), [])
        self.assertEqual(len(list(check_style.scan_case_names('TEST_CASE("Only.Two") {}'))), 1)

    def test_repository_cases_are_clean(self):
        code, out = run_check_default("cases")
        self.assertEqual(code, 0, out)


class FormatChecks(FixtureCase):
    def test_misformatted_file_fails(self):
        self.require_tool(check_style.find_clang_format)
        code, out = run_check("format", self.fixture("format_bad.cpp"))
        self.assertEqual(code, 1, out)

    def test_formatted_file_passes(self):
        self.require_tool(check_style.find_clang_format)
        code, out = run_check("format", self.fixture("format_clean.cpp"))
        self.assertEqual(code, 0, out)


class NamingChecks(FixtureCase):
    # One identifier per rule in naming_bad.hpp.fixture; each must be named in
    # the output, which proves the rule fires and not merely that something did.
    EXPECTED = [
        "bad_macro",  # macro
        "BadNamespace",  # namespace
        "bad_type",  # struct
        "bad_union",  # union
        "bad_alias",  # using alias
        "bad_typedef",  # typedef alias
        "bad_enum",  # enum type
        "bad_enumerator",  # enumerator
        "bad_template_parameter",  # template parameter
        "bad_concept",  # concept
        "BadFunction",  # free function
        "kBadConstant",  # k-prefixed constant
        "BadConstexpr",  # constexpr variable
        "BadConstGlobal",  # const global
        "BadLocalConstant",  # function-local constexpr
        "BadPublicMember",  # public data member
        "bad_method_BadName",  # method
        "protected_without_suffix",  # protected member missing trailing underscore
        "Bad_protected_case_",  # protected member with the suffix but the wrong case
        "hidden_without_suffix",  # private member missing trailing underscore
        "Bad_private_case_",  # private member with the suffix but the wrong case
        "BadParameter",  # parameter
        "BadLocal",  # local variable
    ]

    def test_every_rule_fires(self):
        self.require_tool(check_style.find_clang_tidy)
        code, out = run_check("naming", self.fixture("naming_bad.hpp"))
        self.assertEqual(code, 1, out)
        for identifier in self.EXPECTED:
            self.assertIn(f"'{identifier}'", out, f"{identifier} was not reported")

    def test_conforming_file_passes(self):
        self.require_tool(check_style.find_clang_tidy)
        code, out = run_check("naming", self.fixture("naming_clean.hpp"))
        self.assertEqual(code, 0, out)

    def test_library_headers_conform(self):
        # Covers the detail namespace too, where helpers are not public API.
        self.require_tool(check_style.find_clang_tidy)
        headers = sorted((ROOT / "include" / "TTint").glob("*.hpp"))
        self.assertTrue(headers)
        code, out = run_check("naming", *headers)
        self.assertEqual(code, 0, out)

    def test_examples_and_test_support_headers_conform(self):
        self.require_tool(check_style.find_clang_tidy)
        scoped = {path.relative_to(ROOT).as_posix() for path in check_style.naming_sources(ROOT)}
        self.assertIn("tests/test_environment.hpp", scoped)
        self.assertTrue(any(name.startswith("examples/") for name in scoped))
        code, out = run_check_default("naming")
        self.assertEqual(code, 0, out)

    def test_compile_errors_are_failures(self):
        # A header that does not parse must not look like "no naming problems".
        self.require_tool(check_style.find_clang_tidy)
        broken = self.tmp / "broken.hpp"
        broken.write_text("#pragma once\nint broken( {\n")
        code, out = run_check("naming", broken)
        self.assertEqual(code, 1, out)

    def test_tool_failure_without_diagnostics_is_a_failure(self):
        # An unreadable input produces no diagnostic lines at all, only a non-zero exit.
        self.require_tool(check_style.find_clang_tidy)
        code, out = run_check("naming", self.tmp / "does_not_exist.hpp")
        self.assertEqual(code, 1, out)
        self.assertIn("clang-tidy failed", out)


class ToolDiscovery(FixtureCase):
    def test_missing_tool_is_a_skip_by_default(self):
        code, out = run_check(
            "format", self.fixture("format_clean.cpp"), extra=["--clang-format", "/nonexistent/clang-format"]
        )
        self.assertEqual(code, check_style.EXIT_SKIPPED, out)
        self.assertIn("SKIP", out)

    def test_unresolvable_override_warns(self):
        code, out = run_check(
            "format", self.fixture("format_clean.cpp"), extra=["--clang-format", "/nonexistent/clang-format"]
        )
        self.assertIn("warning: /nonexistent/clang-format is not an executable", out)

    def test_missing_tool_fails_when_required(self):
        code, out = run_check(
            "format",
            self.fixture("format_clean.cpp"),
            extra=["--clang-format", "/nonexistent/clang-format", "--require-tools"],
        )
        self.assertEqual(code, 1, out)

    def test_a_real_failure_wins_over_a_skip(self):
        # tokens fails, format is skipped (tool missing): the run must report failure.
        bad = self.fixture("tokens_bad_null.cpp")
        result = subprocess.run(
            [sys.executable, str(SCRIPT), "tokens", "format", "--paths", str(bad), "--clang-format", "/nonexistent"],
            capture_output=True,
            text=True,
            cwd=ROOT,
            env=CLEAN_ENV,
        )
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("SKIP format", result.stdout)

    def test_environment_can_require_tools(self):
        env = dict(CLEAN_ENV, TTINT_REQUIRE_TOOLS="1")
        result = subprocess.run(
            [sys.executable, str(SCRIPT), "format", "--paths", str(self.fixture("format_clean.cpp")),
             "--clang-format", "/nonexistent"],
            capture_output=True,
            text=True,
            cwd=ROOT,
            env=env,
        )
        self.assertEqual(result.returncode, 1, result.stdout)

    def test_all_includes_the_opt_in_checks(self):
        self.require_tool(check_style.find_clang_tidy)
        code, out = run_check("all", self.fixture("naming_bad.hpp"))
        self.assertEqual(code, 1, out)
        self.assertIn("FAIL naming", out)

    def test_all_cannot_be_mixed_with_other_names(self):
        result = subprocess.run(
            [sys.executable, str(SCRIPT), "all", "format"], capture_output=True, text=True, cwd=ROOT, env=CLEAN_ENV
        )
        self.assertEqual(result.returncode, check_style.EXIT_USAGE)
        self.assertNotIn("Traceback", result.stderr)

    def test_unknown_check_is_a_usage_error(self):
        result = subprocess.run([sys.executable, str(SCRIPT), "bogus"], capture_output=True, text=True, cwd=ROOT)
        self.assertEqual(result.returncode, check_style.EXIT_USAGE)


class Wiring(unittest.TestCase):
    def test_cmake_targets_match_the_checks(self):
        # The per-check CMake targets are a hand-kept copy of CHECKS; fail on drift.
        cmake = (ROOT / "CMakeLists.txt").read_text()
        listed = re.search(r"foreach\(check IN ITEMS ([^)]*)\)", cmake)
        self.assertIsNotNone(listed, "per-check target loop not found in CMakeLists.txt")
        self.assertEqual(set(listed.group(1).split()), set(check_style.CHECKS))


def run_check_default(check):
    """Runs one check over the repository's own sources (no --paths)."""
    result = subprocess.run(
        [sys.executable, str(SCRIPT), check], capture_output=True, text=True, cwd=ROOT
    )
    return result.returncode, result.stdout + result.stderr


if __name__ == "__main__":
    outcome = unittest.main(exit=False).result
    if not outcome.wasSuccessful():
        sys.exit(1)
    # CTest's SKIP_RETURN_CODE: a tool was missing, so some cases did not run.
    sys.exit(check_style.EXIT_SKIPPED if outcome.skipped else 0)
