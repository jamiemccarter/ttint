#include <TTint/TextStyle.hpp>
#include <doctest/doctest.h>

TEST_CASE("Rendering.Segments.StyledSmoke") {
    using namespace ttint;
    // Basic smoke test - renders without crashing
    Segment segments[] = {"Hello" | bold(), " World" | italic()};
    render_ansi(segments);
    // If we get here without crashing, the test passes
    CHECK(true);
}

TEST_CASE("Rendering.Segments.Colors") {
    using namespace ttint;
    Segment segments[] = {
            "Red" | fg(0xFF0000),
            " Blue" | fg(0x0000FF),
            " Green" | fg(0x00FF00),
    };
    render_ansi(segments);
    CHECK(true);
}

TEST_CASE("Rendering.Segments.MixedStyles") {
    using namespace ttint;
    Segment segments[] = {
            "Bold and Red" | (bold() | fg(0xFF0000)),
            " Italic and Blue" | (italic() | bg(0x0000FF)),
            " Underline" | underline(),
    };
    render_ansi(segments);
    CHECK(true);
}
