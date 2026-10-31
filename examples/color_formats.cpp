#include <TTint/TextStyle.hpp>

int main() {
    // Every color form is constexpr, so the whole array can be too.
    constexpr ttint::Segment segments[] = {
            // Packed 0xRRGGBB hex.
            "hex " | ttint::fg(0xff9f0a),
            // Explicit 8-bit channels (0-255).
            "rgb " | ttint::fg(ttint::Color{0x30, 0xd1, 0x58}),
            // Normalized floats (0.0-1.0), rounded to 8 bits.
            "float " | ttint::fg(0.4f, 0.8f, 1.0f),
            // bg() takes the same three forms and composes with fg().
            "highlighted" | (ttint::fg(0.0f, 0.0f, 0.0f) | ttint::bg(0xffd60a)),
            ttint::plain("\n"),
    };

    ttint::render_ansi(segments);
}
