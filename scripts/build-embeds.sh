#!/usr/bin/env bash
# scripts/build-embeds.sh — CSE 102 MWST
#
# Compiles the original src/graph.cpp + the wasm shim into
# embeds/mwst.{js,wasm,html}.
set -euo pipefail

DEST="${EMBED_SOURCE_DIR:-embeds}"
mkdir -p "$DEST"

echo "Build script: cse-102-mwst → $DEST/ (slug=$EMBED_SLUG, version=$EMBED_VERSION)"

if ! command -v emcc >/dev/null 2>&1; then
  echo "::error::emcc not on PATH; the workflow's emsdk setup step must run first"
  exit 1
fi
echo "emcc: $(emcc --version | head -1)"

emcc \
  src/graph.cpp \
  embeds-wasm/mwst/mwst-wasm.cpp \
  -I src \
  -I embeds-wasm/mwst \
  -O3 \
  -std=gnu++2a \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='createMwstModule' \
  -s ENVIRONMENT='web' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s INITIAL_MEMORY=4194304 \
  -s STACK_SIZE=1048576 \
  -s EXPORTED_FUNCTIONS='[
    "_mwst_compute",
    "_mwst_compute_text",
    "_mwst_node_count",
    "_mwst_edge_count",
    "_mwst_get_edge_a",
    "_mwst_get_edge_b",
    "_mwst_get_edge_weight",
    "_mwst_mst_count",
    "_mwst_mst_get_a",
    "_mwst_mst_get_b",
    "_mwst_mst_get_weight",
    "_mwst_total_weight",
    "_mwst_last_error",
    "_malloc",
    "_free"
  ]' \
  -s EXPORTED_RUNTIME_METHODS='["UTF8ToString","stringToUTF8","HEAPU8","ccall","cwrap"]' \
  -s SINGLE_FILE=0 \
  --closure 0 \
  -o "$DEST/mwst.js"

cp embeds-wasm/mwst/mwst.html "$DEST/mwst.html"

echo "  ✓ $DEST/mwst.js   ($(stat -c%s "$DEST/mwst.js" 2>/dev/null || stat -f%z "$DEST/mwst.js") bytes)"
echo "  ✓ $DEST/mwst.wasm ($(stat -c%s "$DEST/mwst.wasm" 2>/dev/null || stat -f%z "$DEST/mwst.wasm") bytes)"
echo "  ✓ $DEST/mwst.html"

echo
echo "Build script complete"
ls -la "$DEST"
