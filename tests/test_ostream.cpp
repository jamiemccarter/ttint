#include <TTint/OStreamStyle.hpp>
#include <doctest/doctest.h>
#include "test_environment.hpp"

#include <iostream>
#include <sstream>

TEST_CASE("Rendering.Ostream.SingleSegmentOpensTextResets") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;

    std::ostringstream out;
    const Segment segment[] = {"hello" | (bold() | fg(0xFF0000))};
    render_ansi(out, segment, ColorPolicy::Always);
    CHECK(out.str() == "\x1b[1m\x1b[38;2;255;0;0mhello\x1b[0m");
}

TEST_CASE("Rendering.Ostream.KeepsTransitionsInOrder") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;

    std::ostringstream out;
    Segment segments[] = {
            "bold" | bold(),
            plain(" plain"),
            " red" | fg(0xFF0000),
    };

    render_ansi(out, segments);

    CHECK(out.str() == "\x1b[1mbold\x1b[22m plain\x1b[38;2;255;0;0m red\x1b[0m");
}

TEST_CASE("Rendering.Ostream.OmitsCodesWhenNoColor") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;
    no_color_guard.disable_color();

    std::ostringstream out;
    Segment segments[] = {"bold" | bold(), plain(" plain")};
    render_ansi(out, segments);

    CHECK(out.str() == "bold plain");
}

TEST_CASE("Rendering.Ostream.CoutStaysInCoutWithoutColor") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;
    no_color_guard.disable_color();

    std::ostringstream captured;
    auto* original_buffer = std::cout.rdbuf(captured.rdbuf());
    Segment segments[] = {"bold" | bold(), plain(" plain")};
    render_ansi(std::cout, segments);
    std::cout.rdbuf(original_buffer);

    CHECK(captured.str() == "bold plain");
}

TEST_CASE("Rendering.Ostream.StyleThenResetRoundTrips") {
    using namespace ttint;

    std::ostringstream out;
    out << bold() << "styled" << reset(out);
    CHECK(out.str().find("styled") != std::string::npos);
}

TEST_CASE("Rendering.Policy.ArbitraryOstreamDefaultsToColor") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;

    std::ostringstream out;
    CHECK(supports_color(out));
    CHECK(supports_color(out, ColorPolicy::Always));
    CHECK_FALSE(supports_color(out, ColorPolicy::Never));
}

TEST_CASE("Rendering.Policy.ClogUsesStderr") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;

    CHECK(supports_color(std::clog) == supports_color(stderr));
    CHECK(supports_color(std::clog, ColorPolicy::Always));
    CHECK_FALSE(supports_color(std::clog, ColorPolicy::Never));
}

TEST_CASE("Rendering.Policy.NoColorOverridesArbitraryOstream") {
    using namespace ttint;

    test_support::NoColorGuard no_color_guard;
    no_color_guard.disable_color();

    std::ostringstream out;
    CHECK_FALSE(supports_color(out));
    CHECK(to_ansi_string(bold(), out).empty());
    CHECK(reset_string(out).empty());
}

TEST_CASE("Rendering.Policy.NoColorOstream") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;
    no_color_guard.disable_color();

    const Segment segments[] = {"bold" | bold(), plain(" plain")};
    const auto render = [&](ColorPolicy policy) {
        std::ostringstream out;
        render_ansi(out, segments, policy);
        return out.str();
    };

    CHECK(render(ColorPolicy::Automatic) == "bold plain");
    CHECK(render(ColorPolicy::Always) == "bold plain");
    CHECK(render(ColorPolicy::Never) == "bold plain");
}

TEST_CASE("Rendering.Policy.OstreamConvenience") {
    using namespace ttint;
    test_support::NoColorGuard no_color_guard;

    std::ostringstream out;
    out << with_policy(bold(), ColorPolicy::Never) << "plain" << reset(out, ColorPolicy::Never);
    CHECK(out.str() == "plain");

    std::ostringstream styled_out;
    const Segment segment[] = {"plain" | bold()};
    render_ansi(styled_out, segment, ColorPolicy::Never);
    CHECK(styled_out.str() == "plain");
}

TEST_CASE("Rendering.Policy.ResetDefaultsToCerr") {
    using namespace ttint;
    CHECK(reset().empty() == !supports_color(std::cerr));
}
