#include <TTint/TextStyle.hpp>

#include <type_traits>

static_assert(std::is_same_v<decltype(ttint::bold()), ttint::Style>);
static_assert(std::is_same_v<decltype(ttint::plain("text")), ttint::Segment>);
