#include <TTint/TextStyle.hpp>
#include <doctest/doctest.h>

// Basic platform smoke tests
TEST_CASE("Platform.Headers.Compile") {
    // Just verifies platform-specific code compiles
    CHECK(true);
}

TEST_CASE("Style.Creation.CombinesAttributes") {
    using namespace ttint;
    Style s = bold() | italic() | underline();
    CHECK(s.bold);
    CHECK(s.italic);
    CHECK(s.underline);
}

TEST_CASE("Color.Creation.HoldsChannels") {
    using namespace ttint;
    Color c{255, 128, 64};
    CHECK(c.r == 255);
    CHECK(c.g == 128);
    CHECK(c.b == 64);
}
