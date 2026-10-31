#include <TTint/OStreamStyle.hpp>
#include <TTint/Palettes/Ansi16.hpp>
#include <TTint/Palettes/AppleHig.hpp>
#include <TTint/Palettes/Curiosities.hpp>
#include <TTint/Palettes/Pico8.hpp>
#include <doctest/doctest.h>

#include <sstream>

TEST_CASE("Palette.Pico8.AllSixteenValues") {
    using namespace ttint;
    namespace p = palette::pico8;

    const Color expected[] = {
            {0x00, 0x00, 0x00}, {0x1D, 0x2B, 0x53}, {0x7E, 0x25, 0x53}, {0x00, 0x87, 0x51},
            {0xAB, 0x52, 0x36}, {0x5F, 0x57, 0x4F}, {0xC2, 0xC3, 0xC7}, {0xFF, 0xF1, 0xE8},
            {0xFF, 0x00, 0x4D}, {0xFF, 0xA3, 0x00}, {0xFF, 0xEC, 0x27}, {0x00, 0xE4, 0x36},
            {0x29, 0xAD, 0xFF}, {0x83, 0x76, 0x9C}, {0xFF, 0x77, 0xA8}, {0xFF, 0xCC, 0xAA},
    };
    const Color actual[] = {p::black,      p::dark_blue, p::dark_purple, p::dark_green, p::brown,  p::dark_gray,
                            p::light_gray, p::white,     p::red,         p::orange,     p::yellow, p::green,
                            p::blue,       p::indigo,    p::pink,        p::peach};

    for (std::size_t i = 0; i < 16; ++i) {
        CAPTURE(i);
        CHECK(actual[i] == expected[i]);
    }
}

TEST_CASE("Palette.Curiosities.AllSixValues") {
    using namespace ttint;
    namespace p = palette::curiosities;

    CHECK(p::slate == Color{0x46, 0x42, 0x5E});
    CHECK(p::teal == Color{0x15, 0x78, 0x8C});
    CHECK(p::aqua == Color{0x00, 0xB9, 0xBE});
    CHECK(p::cream == Color{0xFF, 0xEE, 0xCC});
    CHECK(p::peach == Color{0xFF, 0xB0, 0xA3});
    CHECK(p::pink == Color{0xFF, 0x69, 0x73});
}

TEST_CASE("Palette.Ansi16.AllSixteenValues") {
    using namespace ttint;
    namespace p = palette::ansi16;

    CHECK(p::black == Color{0x00, 0x00, 0x00});
    CHECK(p::red == Color{0xAA, 0x00, 0x00});
    CHECK(p::green == Color{0x00, 0xAA, 0x00});
    CHECK(p::yellow == Color{0xAA, 0x55, 0x00});
    CHECK(p::blue == Color{0x00, 0x00, 0xAA});
    CHECK(p::magenta == Color{0xAA, 0x00, 0xAA});
    CHECK(p::cyan == Color{0x00, 0xAA, 0xAA});
    CHECK(p::white == Color{0xAA, 0xAA, 0xAA});
    CHECK(p::bright_black == Color{0x55, 0x55, 0x55});
    CHECK(p::bright_red == Color{0xFF, 0x55, 0x55});
    CHECK(p::bright_green == Color{0x55, 0xFF, 0x55});
    CHECK(p::bright_yellow == Color{0xFF, 0xFF, 0x55});
    CHECK(p::bright_blue == Color{0x55, 0x55, 0xFF});
    CHECK(p::bright_magenta == Color{0xFF, 0x55, 0xFF});
    CHECK(p::bright_cyan == Color{0x55, 0xFF, 0xFF});
    CHECK(p::bright_white == Color{0xFF, 0xFF, 0xFF});
}

TEST_CASE("Palette.AppleHig.AllValuesBothThemes") {
    using namespace ttint;
    namespace p = palette::apple_hig;

    struct Row {
        const char* name;
        p::ColorPair pair;
        Color light;
        Color dark;
    };
    const Row rows[] = {
            {"red", p::red, {0xFF, 0x3B, 0x30}, {0xFF, 0x45, 0x3A}},
            {"orange", p::orange, {0xFF, 0x95, 0x00}, {0xFF, 0x9F, 0x0A}},
            {"yellow", p::yellow, {0xFF, 0xCC, 0x00}, {0xFF, 0xD6, 0x0A}},
            {"green", p::green, {0x34, 0xC7, 0x59}, {0x30, 0xD1, 0x58}},
            {"mint", p::mint, {0x00, 0xC7, 0xBE}, {0x63, 0xE6, 0xE2}},
            {"teal", p::teal, {0x30, 0xB0, 0xC7}, {0x40, 0xCB, 0xE0}},
            {"cyan", p::cyan, {0x32, 0xAD, 0xE6}, {0x64, 0xD2, 0xFF}},
            {"blue", p::blue, {0x00, 0x7A, 0xFF}, {0x0A, 0x84, 0xFF}},
            {"indigo", p::indigo, {0x58, 0x56, 0xD6}, {0x5E, 0x5C, 0xE6}},
            {"purple", p::purple, {0xAF, 0x52, 0xDE}, {0xBF, 0x5A, 0xF2}},
            {"pink", p::pink, {0xFF, 0x2D, 0x55}, {0xFF, 0x37, 0x5F}},
            {"brown", p::brown, {0xA2, 0x84, 0x5E}, {0xAC, 0x8E, 0x68}},
            {"gray", p::gray, {0x8E, 0x8E, 0x93}, {0x8E, 0x8E, 0x93}},
            {"gray2", p::gray2, {0xAE, 0xAE, 0xB2}, {0x63, 0x63, 0x66}},
            {"gray3", p::gray3, {0xC7, 0xC7, 0xCC}, {0x48, 0x48, 0x4A}},
            {"gray4", p::gray4, {0xD1, 0xD1, 0xD6}, {0x3A, 0x3A, 0x3C}},
            {"gray5", p::gray5, {0xE5, 0xE5, 0xEA}, {0x2C, 0x2C, 0x2E}},
            {"gray6", p::gray6, {0xF2, 0xF2, 0xF7}, {0x1C, 0x1C, 0x1E}},
    };

    for (const auto& row : rows) {
        CAPTURE(row.name);
        CHECK(row.pair(p::Theme::Light) == row.light);
        CHECK(row.pair(p::Theme::Dark) == row.dark);
    }
}

TEST_CASE("Palette.Usage.ConstexprInDsl") {
    using namespace ttint;

    // The documented consumer spellings must be usable at compile time.
    constexpr Segment hello = "Hello" | fg(palette::curiosities::pink);
    constexpr Segment error = "Error" | fg(palette::pico8::red);
    static_assert(hello.style.fg.has_value());

    const Segment segments[] = {hello, error};
    std::ostringstream out;
    render_ansi(out, segments, ColorPolicy::Always);
    CHECK(out.str() == "\x1b[38;2;255;105;115mHello\x1b[38;2;255;0;77mError\x1b[0m");
}

namespace {
// A consumer-defined palette: no registration or base type beyond ttint::Color.
namespace brand {
inline constexpr ttint::Color accent = {0x66, 0xCC, 0xFF};
} // namespace brand
} // namespace

TEST_CASE("Palette.Usage.UserDefinedPalette") {
    using namespace ttint;

    constexpr Segment hi = "Hi" | fg(brand::accent);
    const Segment segments[] = {hi};
    std::ostringstream out;
    render_ansi(out, segments, ColorPolicy::Always);
    CHECK(out.str() == "\x1b[38;2;102;204;255mHi\x1b[0m");
}
