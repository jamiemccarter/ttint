#include <TTint/TextStyle.hpp>

using ttint::Segment;
using ttint::Style;

// Style builders are constexpr, so a composed Style can be named once and reused.
constexpr Style heading = ttint::bold() | ttint::fg(0x66ccff);
constexpr Style emphasis = ttint::bold() | ttint::italic();

constexpr Segment message[] = {
        "TTint\n" | heading,
        ttint::plain("  reusable styles\n"),
        "  bold and italic\n" | emphasis,
        // Styles can also be composed inline. Boolean flags are ORed; a later color replaces an earlier one.
        "  underlined\n" | (ttint::underline() | ttint::fg(0x888888)),
};

int main() {
    ttint::render_ansi(message);
}
