#include <TTint/TextStyle.hpp>
#include <doctest/doctest.h>
#include "test_environment.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

std::string read_file(FILE* file) {
    fflush(file);
    fseek(file, 0, SEEK_END);
    const long size = ftell(file);
    rewind(file);

    std::string contents(static_cast<size_t>(size), '\0');
    fread(contents.data(), 1, contents.size(), file);
    return contents;
}

} // namespace

TEST_CASE("Rendering.Policy.NoColorOverridesStdio") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;
    no_color_guard.disable_color();

    CHECK_FALSE(supports_color(stdout));
    CHECK_FALSE(supports_color(stderr));
}

TEST_CASE("Rendering.Policy.StringsEmptyWithoutColor") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;
    no_color_guard.disable_color();

    const Style styled = bold() | fg(0xFF0000);
    CHECK(to_ansi_string(styled, stdout).empty());
    CHECK(reset_string(stdout).empty());
}

TEST_CASE("Rendering.Policy.StringsAgreeWithSupportsColor") {
    using namespace ttint;

    const Style styled = bold() | fg(0xFF0000);
    const bool supports = supports_color(stdout);
    CHECK(to_ansi_string(styled, stdout).empty() == !supports);
    CHECK(reset_string(stdout).empty() == !supports);
}

TEST_CASE("Rendering.Policy.RedirectedFile") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;

    const Segment segments[] = {"bold" | bold(), plain(" plain")};
    const auto render = [&](ColorPolicy policy) {
        FILE* file = tmpfile();
        REQUIRE(file != nullptr);
        render_ansi(file, segments, policy);
        const std::string contents = read_file(file);
        fclose(file);
        return contents;
    };

    CHECK(render(ColorPolicy::Automatic) == "bold plain");
    CHECK(render(ColorPolicy::Always) == "\x1b[1mbold\x1b[22m plain\x1b[0m");
    CHECK(render(ColorPolicy::Never) == "bold plain");

    no_color_guard.disable_color();
    CHECK(render(ColorPolicy::Automatic) == "bold plain");
    CHECK(render(ColorPolicy::Always) == "bold plain");
    CHECK(render(ColorPolicy::Never) == "bold plain");
}

TEST_CASE("Rendering.Transitions.UnchangedColor") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;

    FILE* file = tmpfile();
    REQUIRE(file != nullptr);
    const Segment segments[] = {
            "one" | fg(0xFF0000),
            " two" | fg(0xFF0000),
            " three" | bg(0x0000FF),
            " four" | bg(0x0000FF),
    };
    render_ansi(file, segments, ColorPolicy::Always);

    CHECK(read_file(file) == "\x1b[38;2;255;0;0mone two\x1b[39m\x1b[48;2;0;0;255m three four\x1b[0m");
    fclose(file);
}
