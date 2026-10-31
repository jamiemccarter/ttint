#include <TTint/TextStyle.hpp>

int main() {
    const ttint::Segment segments[] = {"status\n" | ttint::bold()};

    // Never prints plain text, regardless of the destination.
    ttint::render_ansi(stdout, segments, ttint::ColorPolicy::Never);

    // Always emits ANSI even when redirected, unless NO_COLOR is set (it overrides every policy).
    ttint::render_ansi(stdout, segments, ttint::ColorPolicy::Always);

    // Omitting the policy means Automatic: ANSI only for a TTY, and never when NO_COLOR is set.
}
