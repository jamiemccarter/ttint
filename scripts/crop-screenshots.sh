#!/bin/sh
# Crops full-window terminal screenshots (3024x1896, Retina) down to the command and its output.
# Usage: scripts/crop-screenshots.sh SRC_DIR [DEST_DIR]   (needs ImageMagick 7)
#
# SRC_DIR holds NAME.png per example. Each is cropped to the same size, with everything below
# the last output line painted over so the trailing prompt and cursor do not show.
# The geometry is tuned to one terminal layout and font size; retake screenshots the same way
# (or adjust the constants below).
set -eu

src=${1:?usage: crop-screenshots.sh SRC_DIR [DEST_DIR]}
dest=${2:-"$(dirname "$0")/../.github/images"}
mkdir -p "$dest"

# NAME:ROWS, where ROWS is the command line plus its output lines.
examples="status:2 owned_strings:2 reusable_styles:5 iostreams:2 color_policy:3 color_formats:2 named_colors:3"

x=1535 y=1080     # top-left of the command line in the zsh pane
w=640 h=164       # fixed crop: fits the tallest example (5 rows)
pitch=32          # px per terminal row
pad_x=14 pad_y=12

for spec in $examples; do
    name=${spec%%:*} rows=${spec##*:}
    bg=$(magick "$src/$name.png" -crop 1x1+$((x + 5))+$((y + 5)) +repage -format '%[hex:u.p{0,0}]' info:)
    magick "$src/$name.png" -crop "${w}x${h}+${x}+${y}" +repage \
        -fill "#$bg" -draw "rectangle 0,$((pitch * rows + 2)) $w,$h" \
        -bordercolor "#$bg" -border "${pad_x}x${pad_y}" "$dest/$name.png"
done
