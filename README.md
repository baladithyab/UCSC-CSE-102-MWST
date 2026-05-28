# UCSC CSE 102 — Algorithm Analysis & Design (Fall 2021)

Programming-project assignment for CSE 102 at UC Santa Cruz: compute the **Minimum Weight Spanning Tree** (MWST) of a weighted undirected graph using Prim's algorithm with a priority-queue.

## Live demo

[https://codeseys.io/projects/cse-102-mwst](https://codeseys.io/projects/cse-102-mwst)

The C++ implementation from the original assignment runs unchanged — `src/graph.cpp` + `src/graph.h` (~250 LOC) — compiled to WebAssembly via Emscripten. The JavaScript layer just renders the resulting MST.

## Layout

```
src/                     — original assignment sources, unmodified
  graph.{cpp,h}          — Graph class with insert() and prims()
  main.cpp               — original CLI driver (kept as documentation)
embeds-wasm/mwst/        — Emscripten shim layer + HTML harness
test/                    — original test fixtures (in1/out1, in2/out2, in3/out3)
scripts/build-embeds.sh  — CI build script
web.codeseys.json        — Manifest for codeseys.io
.github/workflows/       — Calls baladithyab/web-embed-workflows@main
```

## Original CLI

```bash
g++ -std=gnu++2a -O2 -o ydc src/graph.cpp src/main.cpp
./ydc test/in1 /tmp/out1
diff /tmp/out1 test/out1   # → no diff, byte-for-byte match
```

The CLI takes 2 args (`input.txt output.txt`) and reads a graph in the line-format:

```
<numVerts>
<numEdges>
<u> <v> <w>
... <numEdges> lines ...
```

It writes the MST in ascending weight order plus the total weight, e.g. for `test/in1`:

```
   5: (3, 5) 1.0
  11: (6, 7) 2.0
   2: (1, 4) 3.0
   6: (3, 7) 4.0
   3: (2, 3) 5.0
   1: (1, 2) 6.0
Total Weight = 21.00
```

## Build pipeline

1. Push to `master` triggers `.github/workflows/build-web-asset.yml`, which calls the reusable workflow at [`baladithyab/web-embed-workflows@main`](https://github.com/baladithyab/web-embed-workflows).
2. Manifest declares `build.tools: ["emsdk"]` so the workflow installs Emscripten (cached).
3. `scripts/build-embeds.sh` runs `emcc` over `src/graph.cpp` + the wasm shim, producing `embeds/mwst.{js,wasm,html}`.
4. The workflow uploads everything in `embeds/` to Cloudflare R2 at `cse-102-mwst/<git-sha>/`, authenticated via OIDC.

## Algorithm

`Graph::prims()` (`src/graph.cpp:121-176`):

1. Initialize `key[v] = ∞` for all vertices, pick any start vertex with `key[start] = 0`.
2. Maintain a min-heap of (vertex, weight) pairs ordered by weight.
3. Pop the smallest; if not yet visited, visit it and add `weight` to `total_cost`.
4. For each unvisited neighbor `(u, weight)`, if `weight < key[u]`, update `key[u] = weight` and push `(u, weight)`.
5. Repeat until the heap is empty.
6. Return parent-edges as a flat `[parent, child, parent, child, ...]` vector.

The wasm shim (`embeds-wasm/mwst/mwst-wasm.cpp`) takes input text, runs Prim's via the original Graph class, walks the input edges to identify which ones are in the MST, sorts those by weight ascending, and exposes the result via `mwst_compute_text()` plus per-edge accessors.

## Running locally

```bash
# Install emsdk (one-time)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source emsdk_env.sh

# Build
cd ../UCSC-CSE-102-MWST
EMBED_SOURCE_DIR=embeds ./scripts/build-embeds.sh

# Serve
python3 -m http.server -d embeds 8080
# Open http://localhost:8080/mwst.html
```
