#include <TTint/Palettes/AppleHig.hpp>
#include <TTint/TextStyle.hpp>

namespace hig = ttint::palette::apple_hig;

int main() {
    // TTint does not detect the OS appearance, so the theme is always chosen explicitly.
    constexpr auto theme = hig::Theme::Dark;
    constexpr ttint::Segment message[] = {
            // hig::red(theme) returns a Color, which fg() accepts like any other.
            "Error: " | ttint::fg(hig::red(theme)),
            "something went wrong\n" | ttint::bold(),
            "Info: " | ttint::fg(hig::blue(theme)),
            ttint::plain("operation completed\n"),
    };

    ttint::render_ansi(message);
}
