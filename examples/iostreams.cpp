// OStreamStyle.hpp includes TextStyle.hpp and adds the std::ostream overloads.
#include <TTint/OStreamStyle.hpp>

#include <iostream>

int main() {
    const ttint::Segment segments[] = {
            "status: " | ttint::bold(),
            "ready\n" | ttint::fg(0x66ccff),
    };

    // Writes through std::cout's own buffer, so it is safe to mix with other cout output.
    // The stdio overload render_ansi(segments) writes to stdout directly and should not be mixed with it.
    ttint::render_ansi(std::cout, segments);
}
