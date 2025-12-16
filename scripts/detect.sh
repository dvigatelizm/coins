#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

IMG="${1:-}"
OUT="${2:-}"

if [[ -z "$IMG" ]]; then
  echo "Usage: scripts/detect.sh <image_path> [out_labels.txt]"
  exit 2
fi

# ѕуть под твой preset (как у теб€: --preset x64-release)
EXE="$ROOT/out/build/x64-release/tools/detect_cli/coin_detect_cli.exe"

if [[ ! -f "$EXE" ]]; then
  echo "Executable not found: $EXE"
  echo "Build it:"
  echo "  cmake --build --preset x64-release --target coin_detect_cli"
  exit 3
fi

if [[ -n "$OUT" ]]; then
  "$EXE" --image "$IMG" --out "$OUT"
else
  "$EXE" --image "$IMG"
fi
