#pragma once

#include <cstdlib>
#include <string>

namespace test_support {

// A `class` rather than a `struct`: it owns the saved environment and restores it
// on destruction, so the saved state is hidden to keep that invariant intact.
class NoColorGuard {
public:
    NoColorGuard() : previous_(std::getenv("NO_COLOR")) {
        if (previous_) previous_value_ = previous_;
#if defined(_WIN32)
        _putenv_s("NO_COLOR", "");
#else
        unsetenv("NO_COLOR");
#endif
    }

    void disable_color() {
#if defined(_WIN32)
        _putenv_s("NO_COLOR", "1");
#else
        setenv("NO_COLOR", "1", 1);
#endif
    }

    ~NoColorGuard() {
#if defined(_WIN32)
        _putenv_s("NO_COLOR", previous_ ? previous_value_.c_str() : "");
#else
        if (previous_) {
            setenv("NO_COLOR", previous_value_.c_str(), 1);
        } else {
            unsetenv("NO_COLOR");
        }
#endif
    }

private:
    const char* previous_;
    std::string previous_value_;
};

} // namespace test_support
