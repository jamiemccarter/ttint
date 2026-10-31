#pragma once

#include "../TextStyle.hpp"

namespace ttint::palette::ansi16 {

// The classic 16-color terminal vocabulary (SGR 30-37 and 90-97) with the
// default IBM VGA text-mode RGB values. These are fixed RGB data, not the
// user's terminal theme: real terminals remap these slots, so fg(ansi16::red)
// emits a 24-bit color and will not follow the terminal's own "red".
// VGA's "yellow" is the dark yellow (brown) 0xAA5500.
inline constexpr Color black = {0x00, 0x00, 0x00};
inline constexpr Color red = {0xAA, 0x00, 0x00};
inline constexpr Color green = {0x00, 0xAA, 0x00};
inline constexpr Color yellow = {0xAA, 0x55, 0x00};
inline constexpr Color blue = {0x00, 0x00, 0xAA};
inline constexpr Color magenta = {0xAA, 0x00, 0xAA};
inline constexpr Color cyan = {0x00, 0xAA, 0xAA};
inline constexpr Color white = {0xAA, 0xAA, 0xAA};

inline constexpr Color bright_black = {0x55, 0x55, 0x55};
inline constexpr Color bright_red = {0xFF, 0x55, 0x55};
inline constexpr Color bright_green = {0x55, 0xFF, 0x55};
inline constexpr Color bright_yellow = {0xFF, 0xFF, 0x55};
inline constexpr Color bright_blue = {0x55, 0x55, 0xFF};
inline constexpr Color bright_magenta = {0xFF, 0x55, 0xFF};
inline constexpr Color bright_cyan = {0x55, 0xFF, 0xFF};
inline constexpr Color bright_white = {0xFF, 0xFF, 0xFF};

} // namespace ttint::palette::ansi16
