#include <TTint/TextStyle.hpp>
#include <doctest/doctest.h>

TEST_CASE("Segment.Text.BorrowsLiteral") {
    using namespace ttint;
    const char* literal = "test";
    Segment s = literal | ttint::bold();
    CHECK(s.view() == "test");
}

TEST_CASE("Segment.Text.OwnsString") {
    using namespace ttint;
    std::string owned = "dynamic";
    Segment s = std::move(owned) | bold();
    CHECK(s.view() == "dynamic");
}

TEST_CASE("Segment.Text.OwnsTemporaryString") {
    using namespace ttint;
    Segment s = std::string("temp") | fg(0xFF0000);
    CHECK(s.view() == "temp");
}

TEST_CASE("Segment.Composition.AttachesStyle") {
    using namespace ttint;

    Segment s1 = "hello" | bold();
    CHECK(s1.style.bold);
    CHECK(s1.view() == "hello");

    Segment s2 = "world" | (italic() | fg(0xFF0000));
    CHECK(s2.style.italic);
    CHECK(s2.style.fg->r == 255);
}
