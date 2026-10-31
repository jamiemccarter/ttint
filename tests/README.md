# Tests

## Building and Running

```bash
cmake -B build -DTTINT_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
scripts/check_style.py all
```

Run the test executable directly with `./build/ttint_tests`.

Style checks run under ctest as `ttint_style` (the repository) and `ttint_style_selftest`
(`tests/style/`, which proves each check fires on a deliberate violation). See the style
guide's Enforcement section for prerequisites and the skip/require-tools behavior.

## Case names

Name each `TEST_CASE` `Domain.Feature.Behavior`, for example `Style.Composition.ColorRightPrecedence`; `scripts/check_style.py cases` enforces it. `SUBCASE` names are free-form.

## Test Framework

Uses [doctest](https://github.com/doctest/doctest) - a lightweight, header-only C++ testing framework.

## Test Files

- `test_main.cpp` - Test runner entry point
- `test_style.cpp` - Style composition tests
- `test_colors.cpp` - Color conversion tests
- `test_segments.cpp` - Segment creation and composition tests
- `test_render.cpp` - ANSI rendering tests
- `test_platform.cpp` - Platform-specific behavior tests
