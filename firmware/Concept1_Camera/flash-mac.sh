#!/usr/bin/env bash
set -euo pipefail
if [[ $# -ne 1 || "$1" != /dev/cu.* ]]; then
  echo "Usage: bash flash-mac.sh /dev/cu.usbmodemYOUR_PORT" >&2
  echo "Find your port with: arduino-cli board list" >&2
  exit 2
fi
command -v arduino-cli >/dev/null || { echo "Install Arduino CLI first; see README.md." >&2; exit 1; }
sketch_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$sketch_dir/build/mac-production"
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3:CDCOnBoot=default \
  --build-property compiler.cpp.extra_flags=-DTEAM2_SIMULATION=0 \
  --build-path "$build_dir" "$sketch_dir"
arduino-cli upload --fqbn esp32:esp32:XIAO_ESP32C3:CDCOnBoot=default \
  --port "$1" --input-dir "$build_dir" "$sketch_dir"
