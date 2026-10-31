#include <TTint/TextStyle.hpp>

int main() {
    // A Segment is text plus a Style. A bare string literal is not a Segment, so use plain() for unstyled text.
    constexpr ttint::Segment status[] = {
            ttint::plain("build: "),
            // `|` between styles merges them; `|` between text and a style builds a Segment.
            "passing" | (ttint::bold() | ttint::fg(0x30d158)),
            ttint::plain("\n"),
    };

    // Writes to stdout and emits only the style changes between segments, then a reset.
    ttint::render_ansi(status);
}
