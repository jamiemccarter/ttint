#include <TTint/TextStyle.hpp>
#include <doctest/doctest.h>

#include <cmath>

TEST_CASE("Color.FloatConversion.RoundsToNearest") {
    using namespace ttint;

    SUBCASE("0.5 rounds to 128") {
        Style s = fg(0.5f, 0.5f, 0.5f);
        CHECK(s.fg->r == 128);
        CHECK(s.fg->g == 128);
        CHECK(s.fg->b == 128);
    }

    SUBCASE("Edge cases") {
        CHECK(fg(0.0f, 0.0f, 0.0f).fg->r == 0);
        CHECK(fg(1.0f, 1.0f, 1.0f).fg->r == 255);
        CHECK(fg(0.999f, 0.999f, 0.999f).fg->r == 255);
    }
}

TEST_CASE("Color.FloatConversion.ClampsOutOfRange") {
    using namespace ttint;

    CHECK(fg(-0.2f, 1.5f, 0.0f).fg == Color{0, 255, 0});
    CHECK(bg(std::nanf(""), 2.0f, -1.0f).bg == Color{0, 255, 0});
}

TEST_CASE("Color.FloatConversion.Constexpr") {
    using namespace ttint;

    constexpr Style s = fg(0.5f, 0.0f, 1.0f) | bg(0.25f, 0.5f, 0.75f);
    static_assert(s.fg == Color{128, 0, 255});
    static_assert(s.bg == Color{64, 128, 191});
    CHECK(s.fg == Color{128, 0, 255});
}

TEST_CASE("Color.HexConversion.SplitsChannels") {
    using namespace ttint;
    Style s = fg(0xAABBCC);
    CHECK(s.fg->r == 0xAA);
    CHECK(s.fg->g == 0xBB);
    CHECK(s.fg->b == 0xCC);
}

TEST_CASE("Color.BackgroundConversion.SetsBackgroundChannels") {
    using namespace ttint;

    SUBCASE("Hex background color") {
        Style s = bg(0x112233);
        CHECK(s.bg->r == 0x11);
        CHECK(s.bg->g == 0x22);
        CHECK(s.bg->b == 0x33);
    }

    SUBCASE("Float background color") {
        Style s = bg(0.25f, 0.5f, 0.75f);
        CHECK(s.bg->r == 64);  // 0.25 * 255 ≈ 64
        CHECK(s.bg->g == 128); // 0.5 * 255 = 128
        CHECK(s.bg->b == 191); // 0.75 * 255 ≈ 191
    }
}
