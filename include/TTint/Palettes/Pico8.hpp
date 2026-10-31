#pragma once

#include "../TextStyle.hpp"

namespace ttint::palette::pico8 {

// The 16 default colors of the PICO-8 fantasy console by Lexaloffle Games
// (https://www.lexaloffle.com/pico-8.php), in console index order 0-15. Color
// values are factual RGB data; PICO-8 is a trademark of its owner and this
// project is not endorsed by it. Names follow the common community naming.
inline constexpr Color black = {0x00, 0x00, 0x00};
inline constexpr Color dark_blue = {0x1D, 0x2B, 0x53};
inline constexpr Color dark_purple = {0x7E, 0x25, 0x53};
inline constexpr Color dark_green = {0x00, 0x87, 0x51};
inline constexpr Color brown = {0xAB, 0x52, 0x36};
inline constexpr Color dark_gray = {0x5F, 0x57, 0x4F};
inline constexpr Color light_gray = {0xC2, 0xC3, 0xC7};
inline constexpr Color white = {0xFF, 0xF1, 0xE8};
inline constexpr Color red = {0xFF, 0x00, 0x4D};
inline constexpr Color orange = {0xFF, 0xA3, 0x00};
inline constexpr Color yellow = {0xFF, 0xEC, 0x27};
inline constexpr Color green = {0x00, 0xE4, 0x36};
inline constexpr Color blue = {0x29, 0xAD, 0xFF};
inline constexpr Color indigo = {0x83, 0x76, 0x9C};
inline constexpr Color pink = {0xFF, 0x77, 0xA8};
inline constexpr Color peach = {0xFF, 0xCC, 0xAA};

} // namespace ttint::palette::pico8
