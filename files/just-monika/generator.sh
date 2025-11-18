#!/bin/bash

set -euo pipefail

if [[ $# -lt 5 || "$1" == "-h" || "$1" == "--help" ]]; then
    echo "Usage: $0 background.png head.png left.png right.png audio.xxx [output.mp4]"
    exit 1
fi

BG="$1"
SPRITE1="$2"
SPRITE2="$3"
SPRITE3="$4"
AUDIO="$5"

WIDTH="1920"
HEIGHT="960"

if [[ -n "${6:-}" ]]; then
    OUT="$6"
else
    OUT="${AUDIO%.*}.mp4"
fi

for f in "$BG" "$SPRITE1" "$SPRITE2" "$SPRITE3" "$AUDIO"; do
    [[ -f "$f" ]] || { echo "File not found: $f"; exit 1; }
done

echo "Background : $BG"
echo "Head       : $SPRITE1"
echo "Left       : $SPRITE2"
echo "Right      : $SPRITE3"
echo "Audio      : $AUDIO"
echo "Output     : $OUT"

ffmpeg -loop 1 -i "$BG" \
       -i "$SPRITE1" \
       -i "$SPRITE2" \
       -i "$SPRITE3" \
       -i "$AUDIO" \
       -filter_complex "
         [0:v]scale=$WIDTH:$HEIGHT:force_original_aspect_ratio=decrease,
              pad=$WIDTH:$HEIGHT:(ow-iw)/2:(oh-ih)/2:black[bg];
         [1:v]scale=-2:$HEIGHT[head];
         [2:v]scale=-2:$HEIGHT[left];
         [3:v]scale=-2:$HEIGHT[right];
         [bg][head]overlay=(main_w-overlay_w)/2:(main_h-overlay_h)/2[tmp1];
         [tmp1][left]overlay=(main_w-overlay_w)/2:(main_h-overlay_h)/2[tmp2];
         [tmp2][right]overlay=(main_w-overlay_w)/2:(main_h-overlay_h)/2[v]
       " \
       -map "[v]" -map 4:a \
       -c:v libx264 -tune stillimage -pix_fmt yuv420p -r 10 \
       -c:a aac -b:a 192k \
       -shortest \
       -movflags +faststart \
       -y "$OUT"

