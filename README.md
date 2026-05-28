# UCSC CSE 102 — Algorithm Analysis & Design (Fall 2020 + Fall 2021)

UC Santa Cruz CSE 102 (Algorithm Analysis & Design) coursework. The headline artifact is the **programming project** — Minimum Weight Spanning Tree (MWST) over a weighted undirected graph — implemented **twice**:

- **C++ + Prim's algorithm** (`src/`) — the Fall 2021 submission
- **Go + Kruskal's algorithm with union-find** (`kruskal-go/`) — also from Fall 2021, written for the same project but using a different language and a different MWST algorithm

The interactive demo on codeseys.io runs both. You can flip between Prim's and Kruskal's in the algorithm selector and confirm they always produce the same total MST weight (modulo edge-tie-breaking) on connected graphs.

## Live demo

[https://codeseys.io/projects/cse-102-mwst](https://codeseys.io/projects/cse-102-mwst)

The C++ Prim's runs as WebAssembly via Emscripten. The Kruskal's algorithm is implemented in the same wasm shim as a sibling, since `graph.h` is unmodified — the shim builds union-find on top of the public `edges` vector. This means **the same wasm bundle exposes both algorithms**.

## Layout

```
src/                        — Fall 2021 programming project (C++)
  graph.{cpp,h}             — Graph class with insert() and prims()
  main.cpp                  — original CLI driver, kept as documentation

kruskal-go/                 — Fall 2021 programming project (Go), Kruskal's
  go.mod                    — Go 1.21 module
  main.go                   — CLI: go run . -f input.txt
  graph/                    — generic graph types (edge list, adjacency map)
  graph/mwst/kruskal.go     — Kruskal's algorithm with union-find
  utils/set.go              — set helper
  README.md                 — original README

embeds-wasm/mwst/           — Emscripten shim layer + HTML harness
  mwst-wasm.cpp             — exposes BOTH algorithms via mwst_compute(input, algo)
  mwst.html                 — algorithm selector + circular-layout viz

test/                       — original test fixtures (in1/out1, in2/out2, in3/out3)
writeups/                   — LaTeX writeups for HW1-4 (Fall 2020) + HW1-2 (Fall 2021)
                              + Lab 1-3 (Fall 2020). Documents only — not deployable
                              demos. Not exposed as project tabs on codeseys.io.

scripts/build-embeds.sh     — CI build script (Emscripten compile + asset prep)
web.codeseys.json           — Manifest for codeseys.io
.github/workflows/          — Calls baladithyab/web-embed-workflows@main
```

## Quick verify (CLI, both implementations)

### C++ Prim's
```bash
g++ -std=gnu++2a -O2 -o ydc src/graph.cpp src/main.cpp
./ydc test/in1 /tmp/out1
diff /tmp/out1 test/out1   # → no diff, byte-for-byte match
```

### Go Kruskal's
```bash
cd kruskal-go
go run . -f ../test/in1
# Edges in MST: (3,5,1) (6,7,2) (1,4,3) (3,7,4) (2,3,5) (1,2,6)
# Total weight: 1+2+3+4+5+6 = 21 (matches Prim's output)
```

## Build pipeline

1. Push to `master` triggers `.github/workflows/build-web-asset.yml`, which calls the reusable workflow at [`baladithyab/web-embed-workflows@main`](https://github.com/baladithyab/web-embed-workflows).
2. Manifest declares `build.tools: ["emsdk"]` so the workflow installs Emscripten (cached after first run).
3. `scripts/build-embeds.sh` runs `emcc` over `src/graph.cpp` + the wasm shim — the Go code is preserved as readable source but does not compile in CI (it would need TinyGo and a separate output bundle; not worth the complexity).
4. The workflow uploads everything in `embeds/` to Cloudflare R2 at `cse-102-mwst/<git-sha>/`, authenticated via OIDC (no static API keys).

## Algorithms

### Prim's (C++, `src/graph.cpp::prims()`)

1. Initialize `key[v] = ∞` for all vertices, pick any start vertex with `key[start] = 0`.
2. Min-heap of `(vertex, weight)` pairs ordered by weight.
3. Pop smallest; if not yet visited, add `weight` to `total_cost`.
4. For each unvisited neighbor `(u, weight)`, if `weight < key[u]`, update `key[u] = weight` and push `(u, weight)`.
5. Repeat until heap is empty.

### Kruskal's (Go, ported into `embeds-wasm/mwst/mwst-wasm.cpp` for the demo)

1. Sort all edges ascending by weight.
2. Walk edges in order; for each, use union-find to check if it would form a cycle.
3. If not, add it to the MST and union the two endpoints' components.
4. Stop when the MST has `numNodes - 1` edges.

The Go version uses path-compression in `find()` and union-by-rank in the disjoint-set; the C++ port mirrors both.

## Writeups

The `writeups/` directory contains LaTeX-rendered PDFs for HW1-4 (Fall 2020) and HW1-2 (Fall 2021), plus Lab 1-3 (Fall 2020). These are **course writeups**, not interactive demos — algorithm analysis problems, recurrence solutions, induction proofs. They're preserved for documentation but **deliberately not exposed as project tabs on the live site** (they "lead to nothing" interactive).

## Running the WASM build locally

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
