#include <TTint/OStreamStyle.hpp>

#include <type_traits>

static_assert(
        std::is_same_v<decltype(ttint::with_policy(ttint::bold(), ttint::ColorPolicy::Never)), ttint::StyleWithPolicy>);
