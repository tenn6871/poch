#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"
MODTOPIA_INCLUDE="${MODTOPIA_INCLUDE:-$(pwd)/sdk/include}"
if [[ ! -f "${MODTOPIA_INCLUDE}/modtopia/modtopia.h" ]]; then
    echo "Set MODTOPIA_INCLUDE to the SDK include directory containing modtopia/modtopia.h." >&2
    exit 1
fi

case "$(uname -m)" in
    x86_64|amd64) platform=linux-x64 ;;
    aarch64|arm64) platform=linux-arm64 ;;
    *) echo "Unsupported architecture: $(uname -m)" >&2; exit 1 ;;
esac

mkdir -p "build/bin/$platform"
"${CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -fPIC -fvisibility=hidden \
    -shared -I "$MODTOPIA_INCLUDE" mod.c -o "build/bin/$platform/mod.so"
"${PYTHON:-python3}" tools/pack.py --platform "$platform"
