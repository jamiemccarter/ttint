#include <TTint/TextStyle.hpp>
#include <doctest/doctest.h>

TEST_CASE("Style.Composition.ColorRightPrecedence") {
    using namespace ttint;

    Style s = fg(0xFF0000) | fg(0x0000FF);
    CHECK(s.fg->r == 0);
    CHECK(s.fg->g == 0);
    CHECK(s.fg->b == 255);
}

TEST_CASE("Style.Composition.BooleansAreOred") {
    using namespace ttint;

    Style s = bold() | italic();
    CHECK(s.bold);
    CHECK(s.italic);

    Style s2 = bold() | bold();
    CHECK(s2.bold); // Idempotent
}

TEST_CASE("Style.Composition.ComplexMixing") {
    using namespace ttint;

    Style s = bold() | fg(0xFF0000) | italic();
    CHECK(s.bold);
    CHECK(s.italic);
    CHECK(!s.underline);
    CHECK(s.fg->r == 255);
    CHECK(s.fg->g == 0);
    CHECK(s.fg->b == 0);
}

TEST_CASE("Style.Composition.BackgroundRightPrecedence") {
    using namespace ttint;

    Style s = bg(0xFF0000) | bg(0x0000FF);
    CHECK(s.bg->r == 0);
    CHECK(s.bg->g == 0);
    CHECK(s.bg->b == 255);
}
