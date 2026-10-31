#include <TTint/TextStyle.hpp>

#include <string>
#include <utility>

int main() {
    std::string label = "Dynamic title";
    std::string value = "42";

    // Moving a std::string into a Segment makes the Segment own the text. String literals and
    // plain(std::string_view) only borrow, so their source must outlive the Segment.
    const ttint::Segment segments[] = {
            std::move(label) | ttint::bold(),
            ttint::plain(" = "),
            std::move(value) | ttint::fg(0x66ccff),
            ttint::plain("\n"),
    };

    ttint::render_ansi(segments);
}
