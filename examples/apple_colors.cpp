#include <TTint/Palettes/AppleHig.hpp>
#include <TTint/TextStyle.hpp>

using ttint::Segment;
namespace hig = ttint::palette::apple_hig;

// Prints every HIG hue and gray in one theme. Switch to hig::Theme::Light to compare.
constexpr auto theme = hig::Theme::Dark;

constexpr Segment demo[] = {
        "Apple HIG Colors (Dark Mode)\n" | ttint::bold(),
        ttint::plain("  "),
        "Red   " | ttint::fg(hig::red(theme)),
        ttint::plain("  "),
        "Orange" | ttint::fg(hig::orange(theme)),
        ttint::plain("  "),
        "Yellow" | ttint::fg(hig::yellow(theme)),
        ttint::plain("  "),
        "Green " | ttint::fg(hig::green(theme)),
        ttint::plain("\n"),
        ttint::plain("  "),
        "Mint  " | ttint::fg(hig::mint(theme)),
        ttint::plain("  "),
        "Teal  " | ttint::fg(hig::teal(theme)),
        ttint::plain("  "),
        "Cyan  " | ttint::fg(hig::cyan(theme)),
        ttint::plain("  "),
        "Blue  " | ttint::fg(hig::blue(theme)),
        ttint::plain("\n"),
        ttint::plain("  "),
        "Indigo" | ttint::fg(hig::indigo(theme)),
        ttint::plain("  "),
        "Purple" | ttint::fg(hig::purple(theme)),
        ttint::plain("  "),
        "Pink  " | ttint::fg(hig::pink(theme)),
        ttint::plain("  "),
        "Brown" | ttint::fg(hig::brown(theme)),
        ttint::plain("\n"),
        ttint::plain("  "),
        "Gray  " | ttint::fg(hig::gray(theme)),
        ttint::plain("  "),
        "Gray2 " | ttint::fg(hig::gray2(theme)),
        ttint::plain("  "),
        "Gray3 " | ttint::fg(hig::gray3(theme)),
        ttint::plain("  "),
        "Gray4 " | ttint::fg(hig::gray4(theme)),
        ttint::plain("\n"),
};

int main() {
    ttint::render_ansi(demo);
}
