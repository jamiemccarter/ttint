#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace ttint {

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    [[nodiscard]] constexpr bool operator==(const Color&) const = default;
};

struct Style {
    bool bold = false;
    bool italic = false;
    bool underline = false;

    std::optional<Color> fg;
    std::optional<Color> bg;
};

enum class ColorPolicy {
    Automatic, //< Color terminal streams, while respecting TTY detection and NO_COLOR.
    Always,    //< Color even when the destination is not a terminal, unless NO_COLOR is set.
    Never,     //< Never emit ANSI styling.
};

struct Segment {
    /// `std::string` alternatives own their text. `std::string_view`
    /// alternatives borrow it, so the source must outlive this Segment.
    std::variant<std::string, std::string_view> text;
    Style style;

    /// Returns a view of the text regardless of ownership
    [[nodiscard]] std::string_view view() const {
        return std::visit([](const auto& arg) -> std::string_view { return std::string_view(arg); }, text);
    }
};

[[nodiscard]] constexpr Segment plain(std::string_view text) {
    return Segment{text, Style{}};
}

[[nodiscard]] constexpr Style bold() {
    Style s{};
    s.bold = true;
    return s;
}

[[nodiscard]] constexpr Style italic() {
    Style s{};
    s.italic = true;
    return s;
}

[[nodiscard]] constexpr Style underline() {
    Style s{};
    s.underline = true;
    return s;
}

namespace detail {

/// Converts a normalized channel to 8 bits, rounding half up and clamping to [0, 1] (NaN becomes 0).
/// Hand-rolled because `std::round` is not constexpr before C++23.
[[nodiscard]] constexpr uint8_t to_byte(float v) {
    if (!(v > 0.0f)) return 0;
    if (v >= 1.0f) return 255;
    return static_cast<uint8_t>(static_cast<double>(v * 255.0f) + 0.5);
}

} // namespace detail

[[nodiscard]] constexpr Style fg(Color c) {
    Style s{};
    s.fg = c;
    return s;
}

[[nodiscard]] constexpr Style bg(Color c) {
    Style s{};
    s.bg = c;
    return s;
}

[[nodiscard]] constexpr Style fg(uint32_t rgb) {
    return fg(Color{
            static_cast<uint8_t>((rgb >> 16) & 0xff),
            static_cast<uint8_t>((rgb >> 8) & 0xff),
            static_cast<uint8_t>(rgb & 0xff),
    });
}

[[nodiscard]] constexpr Style fg(float r, float g, float b) {
    return fg(Color{
            detail::to_byte(r),
            detail::to_byte(g),
            detail::to_byte(b),
    });
}

[[nodiscard]] constexpr Style bg(uint32_t rgb) {
    return bg(Color{
            static_cast<uint8_t>((rgb >> 16) & 0xff),
            static_cast<uint8_t>((rgb >> 8) & 0xff),
            static_cast<uint8_t>(rgb & 0xff),
    });
}

[[nodiscard]] constexpr Style bg(float r, float g, float b) {
    return bg(Color{
            detail::to_byte(r),
            detail::to_byte(g),
            detail::to_byte(b),
    });
}

/// Composes two styles with right-precedence semantics.
///
/// Boolean flags (bold, italic, underline): OR semantics - enabled if either is true.
/// Color attributes (fg, bg): Right-side wins if present, otherwise left-side.
///
/// @example
///   bold() | italic()            // Both bold and italic
///   fg(red) | fg(blue)           // Blue foreground (right wins)
///   bold() | fg(blue) | italic() // Bold, italic, blue
[[nodiscard]] constexpr Style operator|(Style a, Style b) {
    Style out{};
    out.bold = a.bold || b.bold;
    out.italic = a.italic || b.italic;
    out.underline = a.underline || b.underline;
    out.fg = b.fg ? b.fg : a.fg;
    out.bg = b.bg ? b.bg : a.bg;
    return out;
}

[[nodiscard]] constexpr Segment operator|(std::string_view text, Style style) {
    return Segment{text, style};
}

[[nodiscard]] constexpr Segment operator|(const char* text, Style style) {
    return Segment{std::string_view(text), style};
}

[[nodiscard]] inline Segment operator|(std::string text, Style style) {
    return Segment{std::move(text), style};
}

/// Returns whether ANSI color output should be emitted for `stream` under
/// `policy`. NO_COLOR is a user opt-out and therefore suppresses output even
/// when policy is Always. Automatic mode requires a TTY.
namespace detail {

#if defined(_WIN32)
[[nodiscard]] inline int file_descriptor(FILE* stream) {
    return _fileno(stream);
}
[[nodiscard]] inline int is_terminal(int descriptor) {
    return _isatty(descriptor);
}
#else
[[nodiscard]] inline int file_descriptor(FILE* stream) {
    return fileno(stream);
}
[[nodiscard]] inline int is_terminal(int descriptor) {
    return isatty(descriptor);
}
#endif

} // namespace detail

[[nodiscard]] inline bool supports_color(FILE* stream, ColorPolicy policy = ColorPolicy::Automatic) {
    if (stream == nullptr || policy == ColorPolicy::Never) return false;
    if (std::getenv("NO_COLOR") != nullptr) return false;
    if (policy == ColorPolicy::Always) return true;
    return detail::is_terminal(detail::file_descriptor(stream)) != 0;
}

namespace detail {

inline void emit(FILE* stream, const char* s) {
    std::fputs(s, stream);
}

inline void emit_text(FILE* stream, std::string_view text) {
    std::fwrite(text.data(), 1, text.size(), stream);
}

[[nodiscard]] inline std::string color_fg_string(Color c) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "\x1b[38;2;%u;%u;%um", c.r, c.g, c.b);
    return buf;
}

[[nodiscard]] inline std::string color_bg_string(Color c) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "\x1b[48;2;%u;%u;%um", c.r, c.g, c.b);
    return buf;
}

inline void emit_color_fg(FILE* stream, Color c) {
    emit(stream, color_fg_string(c).c_str());
}

inline void emit_color_bg(FILE* stream, Color c) {
    emit(stream, color_bg_string(c).c_str());
}

[[nodiscard]] inline std::string diff_string(const Style& from, const Style& to) {
    std::string out;
    if (from.bold && !to.bold) out += "\x1b[22m";
    if (from.italic && !to.italic) out += "\x1b[23m";
    if (from.underline && !to.underline) out += "\x1b[24m";

    if (from.fg && !to.fg) out += "\x1b[39m";
    if (from.bg && !to.bg) out += "\x1b[49m";

    if (!from.bold && to.bold) out += "\x1b[1m";
    if (!from.italic && to.italic) out += "\x1b[3m";
    if (!from.underline && to.underline) out += "\x1b[4m";

    if (from.fg != to.fg && to.fg) out += color_fg_string(*to.fg);
    if (from.bg != to.bg && to.bg) out += color_bg_string(*to.bg);
    return out;
}

inline void emit_diff(FILE* stream, const Style& from, const Style& to) {
    emit(stream, diff_string(from, to).c_str());
}

} // namespace detail

/// Renders segments to a stdio stream according to `policy`. When styling is
/// enabled, the output ends with a full ANSI reset; disabled output contains
/// only the segment text.
inline void render_ansi(FILE* stream, std::span<const Segment> segments, ColorPolicy policy = ColorPolicy::Automatic) {
    if (stream == nullptr) return;
    if (!supports_color(stream, policy)) {
        for (const auto& s : segments) detail::emit_text(stream, s.view());
        return;
    }

    Style current{};

    for (const auto& segment : segments) {
        detail::emit_diff(stream, current, segment.style);
        detail::emit_text(stream, segment.view());
        current = segment.style;
    }

    detail::emit(stream, "\x1b[0m");
}

/// Renders segments to stdout using stdio. This preserves the original API and
/// Automatic policy; use the FILE* overload to choose another destination or
/// policy. Mixing this stdio overload with buffered iostream output can reorder
/// text, so use one I/O layer for a complete line.
inline void render_ansi(std::span<const Segment> segments, ColorPolicy policy = ColorPolicy::Automatic) {
    render_ansi(stdout, segments, policy);
}

/// Returns the ANSI SGR sequence to transition from a reset state into
/// `style`, or an empty string when the destination and `policy` disable
/// styling. Unlike render_ansi, this does not write anywhere — it hands back a
/// plain string, so it can be embedded in text assembled now and written to
/// `stream` later.
[[nodiscard]] inline std::string
to_ansi_string(const Style& style, FILE* stream = stdout, ColorPolicy policy = ColorPolicy::Automatic) {
    if (!supports_color(stream, policy)) return {};
    return detail::diff_string(Style{}, style);
}

/// Returns the ANSI reset sequence, or an empty string when the destination
/// and `policy` disable styling. This returns text; it does not write to the
/// stream. Pairs with to_ansi_string() for building a styled substring.
[[nodiscard]] inline std::string reset_string(FILE* stream = stdout, ColorPolicy policy = ColorPolicy::Automatic) {
    if (!supports_color(stream, policy)) return {};
    return "\x1b[0m";
}

} // namespace ttint
