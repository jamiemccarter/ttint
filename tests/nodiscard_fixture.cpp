// Build-only fixture (see tests/CMakeLists.txt). Case 0 uses every result and
// must compile; each other case discards one result and must not.
#include <TTint/OStreamStyle.hpp>
#include <TTint/Palettes/AppleHig.hpp>

#include <iostream>

int main() {
    using namespace ttint;
    const Color c{1, 2, 3};
    const Style s = bold();

#if TTINT_NODISCARD_CASE == 0
    // Control: results are consumed, so this must build.
    const Segment segment = "x" | s;
    (void) segment;
    (void) (c == c);
    (void) (fg(1.0f, 1.0f, 1.0f) | bg(0x000000) | italic() | underline());
    (void) supports_color(stdout);
    (void) supports_color(std::cout);
    (void) to_ansi_string(s);
    (void) reset_string();
    (void) reset();
    (void) with_policy(s, ColorPolicy::Never);
    (void) palette::apple_hig::red(palette::apple_hig::Theme::Dark);
#elif TTINT_NODISCARD_CASE == 1
    bold();
#elif TTINT_NODISCARD_CASE == 2
    fg(0xffffff);
#elif TTINT_NODISCARD_CASE == 3
    s | s;
#elif TTINT_NODISCARD_CASE == 4
    // Explicit call: a bare `c == c;` would be reported as an unused comparison instead.
    c.operator==(c);
#elif TTINT_NODISCARD_CASE == 5
    plain("x");
#elif TTINT_NODISCARD_CASE == 6
    supports_color(stdout);
#elif TTINT_NODISCARD_CASE == 7
    to_ansi_string(s);
#elif TTINT_NODISCARD_CASE == 8
    reset_string();
#elif TTINT_NODISCARD_CASE == 9
    with_policy(s, ColorPolicy::Never);
#elif TTINT_NODISCARD_CASE == 10
    palette::apple_hig::red(palette::apple_hig::Theme::Dark);
#endif
    return 0;
}
