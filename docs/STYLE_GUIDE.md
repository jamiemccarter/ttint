# C++ Style Guide

TTint’s C++ conventions for public headers under `include/TTint/`, tests under `tests/`, and examples under `examples/`. This guide is **normative**: it defines how TTint code is written and reviewed. Code that disagrees with it should change, not the guide. [`.clang-format`](../.clang-format) is authoritative for layout.

## Repository layout

| Path | Role |
|---|---|
| `include/TTint/` | Public, header-only library. Users copy this tree into their projects. |
| `tests/` | doctest suite (FetchContent); not shipped in the library artifact. |
| `examples/` | Runnable samples linked against the interface target. Every README code block is one of these files. |
| `scripts/` | Contributor tooling (README/example sync check, screenshot cropping). |
| `docs/` | Contributor references (this guide). |
| `.github/images/` | Screenshots embedded in the README. |

There is no private `src/` tree. Implementation lives in headers, with internal helpers in `ttint::detail`.

## Headers and namespaces

- The library is **header-only**. Prefer `inline` / `constexpr` functions and templates; do not add `.cpp` files for library code.
- Use **`#pragma once`**. Do not write `#ifndef` include guards.
- Header files are `PascalCase.hpp` (`TextStyle.hpp`). Optional headers depend on the core header, never the reverse, and never in a cycle.
- Split a new major surface into its own header instead of growing an existing one.

Namespaces are `snake_case`, with one naming dialect per namespace:

- **`ttint::`** — all public API: core types, style builders, render and color-policy functions, and ostream adapters.
- **`ttint::palette::<name>`** — one nested namespace per named palette (for example `palette::pico8`). Each palette is independent RGB data built on `ttint::Color`; palettes do not share an abstraction beyond that.
- **`ttint::detail`** — internal helpers. Not public API; may change without notice. Do not use from examples or README snippets.

## Naming

| Element | Convention | Example |
|---|---|---|
| Types (struct, class, enum, `using` alias) | `PascalCase` | `Style`, `Color`, `ColorPolicy` |
| Enumerators | `PascalCase` | `ColorPolicy::Automatic` |
| Functions, free and member | `snake_case` | `bold()`, `render_ansi()`, `supports_color()` |
| Variables and parameters | `snake_case` | `frame_count` |
| Constants (`constexpr`/`const`, any scope) | `snake_case` | `palette::pico8::red` |
| Public data members | `snake_case` | `Style::bold`, `Color::r` |
| Private/protected data members | `snake_case_` | `buffer_` |
| Namespaces | `snake_case` | `ttint::detail` |
| Test cases | `Domain.Feature.Behavior` | `Style.Composition.ColorRightPrecedence` |

Rules that apply across the table:

- **Acronyms are words.** Write `render_ansi`, `to_ansi_string`, and `AnsiWriter`.
- **Constants look like values.** TTint constants are `snake_case`, with no `k` prefix, so a palette token reads naturally in the DSL: `"Error" | fg(palette::pico8::red)`. `SCREAMING_SNAKE` is only for macros, and macros are avoided (`TTINT_` prefix if one is unavoidable).
- **Booleans read as assertions:** `is_enabled`, `has_color`, `should_emit`. The exception is the DSL style flags listed under [Accepted naming exceptions](#accepted-naming-exceptions).
- **Public struct fields** have no trailing underscore. Only non-public storage does.
- **Overrides** (rare here) match the base declaration.

## Fluent DSL (`operator|`)

Styles and text compose with `operator|`:

```cpp
"text" | (bold() | fg(0x66ccff))
```

- **Builders** are `constexpr` free functions that return a `Style`. They are named for the attribute (`bold()`, `fg()`), not with a `make_` or `get_` prefix.
- **`operator|`** between two styles merges them. Boolean flags OR together, and for colors the right-hand side wins. `operator|` between text and a style builds a text-plus-style value. Keep operator syntax; do not rename or replace it.
- **Render functions** are named for what they do (`render_ansi`). Functions that return text instead of writing it are `to_*` or `*_string`.
- A policy-taking overload adds a trailing `ColorPolicy` parameter that defaults to `Automatic`; do not create parallel function names for each policy.

## Errors and invariants

TTint has no result-wrapper type. Expected failures use these patterns:

- **`bool`** predicates, for example `supports_color()` encodes TTY and `NO_COLOR` policy.
- **Empty strings**, for example `to_ansi_string` and `reset_string` return `{}` when color is disabled.
- **Early return**, for example render paths write plain text and return when color is off.

Do not introduce `std::expected`, exceptions, or a result type for control flow unless the public error model is deliberately changed.

- **`assert()`** only for internal invariants inside `detail`, where a violation means a bug in TTint. Never for validating user input.
- **Exceptions** are not part of the public API. Render paths stay no-throw-friendly.

## Language features

- `nullptr`, never `NULL`. `using`, never `typedef`.
- `enum class`, never unscoped `enum`. Keep invalid states unrepresentable where cheap (an optional color instead of a sentinel RGB value).
- `override` on every overriding virtual function.
- `explicit` on single-argument constructors and conversion operators.
- `[[nodiscard]]` on every function whose only effect is its return value: style builders, `operator|`, predicates such as `supports_color`, string-returning helpers (`to_ansi_string`, `reset_string`), and accessors. Not on functions that write to a stream.
- `constexpr` by default where possible. Use `consteval` only when every call must be compile-time.
- `auto` when it clarifies intent. Trailing return types for long or templated returns; simple `bool` / `void` functions may use leading return types.
- `struct` for plain data with behavior; `class` only when invariants require hidden state. Member order in a `class`: public API, then private helpers, then private data.
- Prefer the standard library. TTint stays tiny and header-only.
- Headers include what they use, with no reliance on transitive includes. Standard headers are in alphabetical order. Tests and examples build with `-Wall -Wextra -Wpedantic` (`/W4` on MSVC) by default, and must stay clean under `-DTTINT_WERROR=ON`.

## Ownership and lifetime

- Types that own text hold a `std::string` (RAII). Types that borrow hold a `std::string_view`, and the header documents that the source must outlive the value.
- Take read-only parameters by `const&`, non-owning text by `std::string_view`, and move into a type when it should take ownership.
- Use `std::optional` for optional values such as a style’s colors, and `std::span` for ranges passed to render functions.
- Never construct a borrowing type from a temporary `std::string`.

## Conversions

- Make narrowing visible at boundaries (RGB from `uint32_t`, float channels to bytes) and use `static_cast`; no C-style casts.
- Avoid implicit conversions across API boundaries. A bare string literal is intentionally not a text-plus-style value: callers write `plain("text")` or `"text" | style`.

## Templates

- Prefer overloads for the small set of color and text types TTint supports.
- Constrain any generic code with concepts; keep `detail` helpers non-template where possible.
- Prefer ordinary code over template metaprogramming.

## Comments

- **Explain why, or what is not obvious, not what the code says.** Skip comments that restate a line (`// bold`). Prefer a short note on a rule, lifetime, or hidden behavior the reader cannot infer from the code.
- **Public headers:** `///` on non-obvious public behavior. Plain `//` for implementation notes.
- **Examples:** comment the API rules a newcomer would trip over (for instance `plain()` for unstyled text, string ownership, `NO_COLOR` precedence). Aim for 2–4 one-line comments per file so the README stays skimmable.
- **README parity:** every README `cpp` block is byte-identical to a file in `examples/`, so comments in an example appear in the README. Write them for the README reader.
- **Placement:** put comments on the line above the code they describe. `.clang-format` aligns trailing comments, which gets awkward inside array initializers.
- Keep comments within the 120-column limit and update them with the code; a stale comment is worse than none.

## Formatting

- [`.clang-format`](../.clang-format) is authoritative (LLVM-based, 120 columns, 4-space indent). If `clang-format` produced it, it is correctly formatted. Do not debate layout in review, and do not hand-align code.

- Format touched C++ before commit. `clang-format -i` does not accept directories, so pass files:

  ```bash
  git ls-files '*.cpp' '*.hpp' | xargs clang-format -i -style=file
  ```

- Keep formatting-only changes separate from behavior changes.

## Tests and documentation

- **Framework:** [doctest](https://github.com/doctest/doctest) via CMake `FetchContent` (see `tests/`).
- **Case names:** `Domain.Feature.Behavior`, for example `Style.Composition.ColorRightPrecedence`. Spaced, free-form names are not used; `scripts/check_style.py cases` enforces this. `SUBCASE` names are free-form.
- Test at public boundaries: composition, render and ostream output, `supports_color` / `NO_COLOR`, color conversion and rounding.
- **Docs:** `///` on non-obvious public behavior in headers; user-facing usage lives in `README.md`. Update this guide when policy changes.

Build, test and check a change:

```bash
cmake -B build -DTTINT_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
scripts/check_style.py all
```

## Performance

TTint is stdio and terminal bound, not CPU bound. Still:

- Prefer `constexpr` builders and compile-time text segments where possible.
- Rendering already minimizes output by diffing styles. Avoid extra allocations in hot paths: reuse `Style` values and prefer `std::string_view` for fixed text.
- No caching and no global mutable state for appearance or theme. Callers pass a theme explicitly.

## Enforcement

Every check below must pass before merge. `scripts/check_style.py all` runs every script check; the test suite runs through `ctest`. With no arguments the script runs only the checks enabled by default, and `--list` shows which those are; name checks explicitly (`check_style.py format naming`) to run a subset.

| Check | Command | Scope |
|---|---|---|
| Format | `scripts/check_style.py format` | `include/`, `tests/`, `examples/` |
| Cases: doctest `TEST_CASE` names are `Domain.Feature.Behavior` | `scripts/check_style.py cases` | `tests/*.cpp` |
| Tokens: no `typedef`, no `NULL`, no leading-underscore identifiers | `scripts/check_style.py tokens` | `include/`, `tests/`, `examples/` |
| Naming: clang-tidy `readability-identifier-naming` ([`.clang-tidy`](../.clang-tidy)) | `scripts/check_style.py naming` | `include/TTint/`, `examples/` and test support headers (`tests/*.hpp`) |
| README ↔ examples ↔ screenshots | `scripts/check_style.py readme` | `README.md`, `examples/`, `.github/images/` |
| Tests | `ctest --test-dir build` | |

- **Prerequisites:** Python 3, `clang-format`, and `clang-tidy`. The script finds LLVM tools on `PATH` or in Homebrew's keg-only LLVM; `CLANG_FORMAT` and `CLANG_TIDY` override the lookup. A missing tool is reported as a skip (exit status 77), and `--require-tools` or `TTINT_REQUIRE_TOOLS=1` makes it a failure. CI should set the latter so a skipped check cannot pass a merge. A tool override that does not resolve prints a warning.
- **Naming scope:** test sources (`tests/*.cpp`) are excluded from the naming check; test support headers are checked. doctest macros generate identifiers the project cannot rename, so test-case names are reviewed against `Domain.Feature.Behavior` instead. Headers are checked directly, with `-std=c++20 -Iinclude`, not through the translation units that include them, so no `compile_commands.json` is needed.
- **Underscore rule:** the token check allows only the platform names TTint uses but does not declare (`_WIN32`, `_isatty`, `_fileno`, `_putenv_s`, and similar). Anything else that starts with an underscore is reported.
- **CMake:** `ttint_style_all` (every check) and `ttint_style_format`, `_tokens`, `_cases`, `_naming` and `_readme` run the same checks; With `TTINT_BUILD_TESTS=ON`, `ctest` runs `ttint_style` (the default set) and `ttint_style_selftest`. The `ttint_style_*` build targets fail on a missing tool (exit status 77); only `ctest` reports it as skipped. They are on by default for a top-level build and controlled by `TTINT_STYLE_CHECKS`.
- **Self-test:** `tests/style/` holds a deliberately violating fixture for every rule and a conforming one for each check. A check that stops detecting its rule fails `ttint_style_selftest`.
- **Warnings:** `-DTTINT_WERROR=ON` turns the warning level from [Language features](#language-features) into errors for tests and examples.

Identifier acronym handling cannot be mechanized and is checked in review with the checklist below.

## Accepted naming exceptions

Every `NOLINT(readability-identifier-naming)` (or equivalent) must be listed here before merge. Current list:

| Name | Rule it bends | Why |
|---|---|---|
| `Style::bold`, `Style::italic`, `Style::underline` | Boolean fields read as assertions | They are the attribute names the DSL is built on (`bold()` sets `bold`). |
| `Color::r`, `Color::g`, `Color::b` | Descriptive names | Universal RGB component names. |

## Review checklist

- [ ] Plain data types are `struct`s; `class` only where invariants need hidden state.
- [ ] All new callables are `snake_case`; acronyms are words; no `camelCase` or `PascalCase` functions.
- [ ] New constants are `snake_case`, no `k` prefix.
- [ ] Public data members are `snake_case`; private/protected members end in `_`.
- [ ] `enum class`, `#pragma once`, `using` (not `typedef`).
- [ ] Expected failures use `bool` / empty string / early return — no result type, no exceptions.
- [ ] Optional headers depend only on the core header.
- [ ] Output of `clang-format`; formatting-only changes are separate commits.
- [ ] New tests use `Domain.Feature.Behavior` names.
- [ ] README code blocks still match `examples/`, and each has a screenshot (`scripts/check_readme_examples.py`).
- [ ] Any naming exception is listed under [Accepted naming exceptions](#accepted-naming-exceptions).
