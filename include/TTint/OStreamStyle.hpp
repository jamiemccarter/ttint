#pragma once

#include "TextStyle.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

namespace ttint {

/// Returns whether ANSI color should be emitted for `stream` under `policy`.
/// std::cout maps to stdout, while std::cerr and std::clog map to stderr. An
/// arbitrary ostream has unknown terminal status and remains color-enabled in
/// Automatic mode for compatibility with text assembled for a real terminal.
[[nodiscard]] inline bool supports_color(const std::ostream& stream, ColorPolicy policy = ColorPolicy::Automatic) {
    if (policy == ColorPolicy::Never) return false;
    if (std::getenv("NO_COLOR") != nullptr) return false;
    if (policy == ColorPolicy::Always) return true;
    if (&stream == &std::cerr || &stream == &std::clog) return supports_color(stderr);
    if (&stream == &std::cout) return supports_color(stdout);
    return true;
}

/// Returns the ANSI SGR sequence to transition from a reset state into
/// `style`, or an empty string if `stream` doesn't support color. Does not
/// write anywhere — hands back a plain string for embedding in text
/// assembled now and written to `stream` later.
[[nodiscard]] inline std::string
to_ansi_string(const Style& style, const std::ostream& stream, ColorPolicy policy = ColorPolicy::Automatic) {
    if (!supports_color(stream, policy)) return {};
    return detail::diff_string(Style{}, style);
}

/// Returns the ANSI reset sequence, or an empty string if `stream` doesn't
/// support color.
[[nodiscard]] inline std::string reset_string(const std::ostream& stream, ColorPolicy policy = ColorPolicy::Automatic) {
    if (!supports_color(stream, policy)) return {};
    return "\x1b[0m";
}

/// A Style paired with the ColorPolicy to apply when it is inserted into an
/// ostream. Created by with_policy().
struct StyleWithPolicy {
    Style style;
    ColorPolicy policy = ColorPolicy::Automatic;
};

/// Associates an explicit color policy with a Style for ostream insertion.
[[nodiscard]] constexpr StyleWithPolicy with_policy(Style style, ColorPolicy policy) {
    return StyleWithPolicy{style, policy};
}

/// Renders segments directly to `out`, keeping styled output in the ostream's
/// buffer. When styling is enabled, the output ends with a full ANSI reset;
/// disabled output contains only segment text. Include this header when an
/// application uses iostreams for the surrounding output and wants to avoid
/// assembling ANSI strings manually.
inline void
render_ansi(std::ostream& out, std::span<const Segment> segments, ColorPolicy policy = ColorPolicy::Automatic) {
    if (!supports_color(out, policy)) {
        for (const auto& segment : segments) out << segment.view();
        return;
    }

    Style current{};
    for (const auto& segment : segments) {
        out << detail::diff_string(current, segment.style) << segment.view();
        current = segment.style;
    }
    out << "\x1b[0m";
}

/// Streams the SGR open sequence for `style` only; caller is responsible for
/// a matching reset() (or use render_ansi, which resets automatically).
inline std::ostream& operator<<(std::ostream& out, const Style& style) {
    return out << to_ansi_string(style, out);
}

inline std::ostream& operator<<(std::ostream& out, const StyleWithPolicy& styled) {
    return out << to_ansi_string(styled.style, out, styled.policy);
}

/// Returns the ANSI reset sequence for `stream`, defaulting to std::cerr to
/// match common diagnostic-output conventions. This returns text and does not
/// write to the stream.
[[nodiscard]] inline std::string reset(const std::ostream& stream = std::cerr,
                                       ColorPolicy policy = ColorPolicy::Automatic) {
    return reset_string(stream, policy);
}

} // namespace ttint
