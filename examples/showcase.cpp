#include <TTint/TextStyle.hpp>

using ttint::Segment;
using ttint::Style;

// Reusable styles, named once and applied to several segments below.
constexpr Style header = ttint::bold();
constexpr Style command = ttint::fg(0x66ccff);

constexpr Segment demo[] = {
        "TTint\n" | ttint::bold(),
        ttint::plain("  direct call: "),
        "\"this text is bold\" | ttint::bold()\n" | ttint::bold(),
        ttint::plain("  chained:\n"),
        ttint::plain("    "),
        "\"bold and italic\" | (ttint::bold() | ttint::italic())\n" | (ttint::bold() | ttint::italic()),
        ttint::plain("    "),
        "\"underlined and blue\" | (ttint::underline() | ttint::fg(0x66ccff))\n" |
                (ttint::underline() | ttint::fg(0x66ccff)),
        ttint::plain("  reusable styles:\n"),
        ttint::plain("    "),
        "header = ttint::bold()\n" | header,
        ttint::plain("    "),
        "command = ttint::fg(0x66ccff)\n" | command,
};

// Each line shows the expression that produced it, rendered in its own style.
int main() {
    ttint::render_ansi(demo);
}
