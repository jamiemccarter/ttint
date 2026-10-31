#include <TTint/TextStyle.hpp>

int main() {
    const ttint::Segment segments[] = {ttint::plain("installed package\n")};
    ttint::render_ansi(segments);
}
