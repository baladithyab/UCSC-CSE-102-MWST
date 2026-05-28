// mwst-wasm.cpp
//
// Emscripten shim around the unmodified CSE 102 MWST (Minimum Weight
// Spanning Tree) solver — graph.cpp + graph.h from the original
// programming project.
//
// Two algorithms are exposed:
//
//   prims    — the C++ implementation in src/graph.cpp (Prim's algorithm
//              with a priority-queue), unmodified.
//   kruskal  — Kruskal's algorithm with union-find. Implemented in this
//              shim file using the Graph's public `edges` vector;
//              graph.h / graph.cpp are NOT modified to add this.
//              Mirrors the semantics of the GO/graph/mwst/kruskal.go
//              implementation that lived in the original 2021
//              `programming_project/bbalamur/GO` directory.
//
// JS interface adds an algorithm selector — `mwst_compute(input, algo)`
// where algo is 0 for Prim's, 1 for Kruskal's. Both algorithms produce
// the same total weight on connected graphs (modulo ties between
// equal-weight edges, which can pick different concrete edges but
// always sum to the same MWST weight).

#include "graph.h"

#include <emscripten/emscripten.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>

namespace {

// One global graph + computed MST per page.
Graph* g_graph = nullptr;

struct ParsedEdge {
    std::string a, b, weight;
};
std::vector<ParsedEdge> g_inputEdges;
std::vector<ParsedEdge> g_mstEdges;
int g_totalWeight = 0;
int g_nodeCount = 0;

std::string g_outBuf;     // for returning strings to JS
std::string g_resultText; // captured text result of mwst_compute()
std::string g_lastError;
std::string g_algoName;   // "prims" or "kruskal"

// Comparator: ascending by numerical weight.
bool cmpyAscending(const std::tuple<std::string, std::string, std::string, int>& a,
                   const std::tuple<std::string, std::string, std::string, int>& b) {
    return std::stof(std::get<2>(a)) < std::stof(std::get<2>(b));
}

void resetState() {
    delete g_graph;
    g_graph = new Graph();
    g_inputEdges.clear();
    g_mstEdges.clear();
    g_totalWeight = 0;
    g_nodeCount = 0;
    g_resultText.clear();
    g_lastError.clear();
    g_algoName.clear();
}

// ---------------------- Kruskal's algorithm -----------------------------
//
// Union-find / disjoint-set with path compression and union-by-rank.
// Mirrors the Go implementation in the original programming_project/
// bbalamur/GO/graph/mwst/kruskal.go.

struct UFNode { int parent; int rank; };

int uf_find(std::vector<UFNode>& uf, int i) {
    if (uf[i].parent != i) uf[i].parent = uf_find(uf, uf[i].parent);
    return uf[i].parent;
}

bool uf_union(std::vector<UFNode>& uf, int a, int b) {
    int ra = uf_find(uf, a);
    int rb = uf_find(uf, b);
    if (ra == rb) return false;  // would form a cycle
    if (uf[ra].rank < uf[rb].rank) {
        uf[ra].parent = rb;
    } else if (uf[ra].rank > uf[rb].rank) {
        uf[rb].parent = ra;
    } else {
        uf[rb].parent = ra;
        uf[ra].rank++;
    }
    return true;
}

// Run Kruskal's: sort edges ascending by weight, walk them in order,
// add to MST if they don't form a cycle (per union-find).
std::vector<std::tuple<std::string, std::string, std::string, int>>
kruskal(const std::vector<ParsedEdge>& edges) {
    using Tup = std::tuple<std::string, std::string, std::string, int>;
    std::vector<Tup> indexed;
    int idx = 0;
    for (const auto& e : edges) {
        ++idx;
        indexed.emplace_back(e.a, e.b, e.weight, idx);
    }
    std::sort(indexed.begin(), indexed.end(), cmpyAscending);

    // Build node-to-index map for union-find.
    std::unordered_map<std::string, int> nodeIdx;
    for (const auto& e : edges) {
        if (nodeIdx.find(e.a) == nodeIdx.end()) nodeIdx[e.a] = (int)nodeIdx.size();
        if (nodeIdx.find(e.b) == nodeIdx.end()) nodeIdx[e.b] = (int)nodeIdx.size();
    }
    std::vector<UFNode> uf(nodeIdx.size());
    for (size_t i = 0; i < uf.size(); ++i) { uf[i].parent = (int)i; uf[i].rank = 0; }

    std::vector<Tup> mst;
    for (const auto& t : indexed) {
        int ai = nodeIdx[std::get<0>(t)];
        int bi = nodeIdx[std::get<1>(t)];
        if (uf_union(uf, ai, bi)) {
            mst.push_back(t);
            if ((int)mst.size() == (int)nodeIdx.size() - 1) break;  // MST complete
        }
    }
    return mst;
}

// ---------------------- Prim's wrapper ---------------------------------

std::vector<std::tuple<std::string, std::string, std::string, int>>
prims_against_input(const std::vector<ParsedEdge>& edges, const std::vector<std::string>& primsOut) {
    using Tup = std::tuple<std::string, std::string, std::string, int>;
    std::vector<Tup> mst;
    int idx = 0;
    for (const auto& ie : edges) {
        ++idx;
        for (size_t k = 0; k + 1 < primsOut.size(); k += 2) {
            const std::string& p = primsOut[k];
            const std::string& c = primsOut[k + 1];
            if (p.empty() || c.empty()) continue;
            if ((p == ie.a && c == ie.b) || (p == ie.b && c == ie.a)) {
                mst.emplace_back(ie.a, ie.b, ie.weight, idx);
                break;
            }
        }
    }
    std::sort(mst.begin(), mst.end(), cmpyAscending);
    return mst;
}

}  // namespace

extern "C" {

/**
 * Run MWST on `input`, which is the same line-format the CLI took:
 *   <numVerts>\n<numEdges>\n<u> <v> <w>\n... (numEdges lines)
 *
 * `algo`: 0 = Prim's (the original), 1 = Kruskal's (sibling algorithm
 * implemented in this shim file).
 *
 * Returns 1 on success, 0 on parse failure (call mwst_last_error()).
 */
EMSCRIPTEN_KEEPALIVE
int mwst_compute(const char* input, int algo) {
    resetState();

    std::istringstream in(input);
    std::string line;

    if (!std::getline(in, line)) {
        g_lastError = "input is empty";
        return 0;
    }
    int numVerts;
    try {
        numVerts = std::stoi(line);
    } catch (...) {
        g_lastError = "first line must be vertex count (integer)";
        return 0;
    }
    if (!std::getline(in, line)) {
        g_lastError = "missing edge count line";
        return 0;
    }
    int numEdges;
    try {
        numEdges = std::stoi(line);
    } catch (...) {
        g_lastError = "second line must be edge count (integer)";
        return 0;
    }
    if (numVerts < 1 || numVerts > 1024 || numEdges < 0 || numEdges > 8192) {
        g_lastError = "input size out of bounds (verts 1..1024, edges 0..8192)";
        return 0;
    }

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        std::vector<std::string> vals;
        std::string v;
        while (ls >> v) vals.push_back(v);
        if (vals.size() < 3) {
            g_lastError = "edge line needs 3 tokens (a b weight): '" + line + "'";
            return 0;
        }
        try {
            (void)std::stof(vals[2]);
        } catch (...) {
            g_lastError = "edge weight not a number in: '" + line + "'";
            return 0;
        }
        g_graph->insert(vals);
        ParsedEdge pe{vals[0], vals[1], vals[2]};
        g_inputEdges.push_back(pe);
    }

    g_nodeCount = numVerts;

    using Tup = std::tuple<std::string, std::string, std::string, int>;
    std::vector<Tup> mst;
    if (algo == 1) {
        // Kruskal's
        g_algoName = "kruskal";
        mst = kruskal(g_inputEdges);
    } else {
        // Prim's (default)
        g_algoName = "prims";
        std::vector<std::string> primsOut = g_graph->prims();
        mst = prims_against_input(g_inputEdges, primsOut);
    }

    float total = 0.0f;
    for (const auto& e : mst) total += std::stof(std::get<2>(e));
    g_totalWeight = (int)total;

    for (const auto& e : mst) {
        ParsedEdge pe{std::get<0>(e), std::get<1>(e), std::get<2>(e)};
        g_mstEdges.push_back(pe);
    }

    // Build the formatted report.
    std::ostringstream oss;
    oss << std::fixed;
    for (const auto& e : mst) {
        oss << std::setw(4) << std::get<3>(e) << ": ("
            << std::get<0>(e) << ", " << std::get<1>(e) << ") "
            << std::setprecision(1) << std::stof(std::get<2>(e)) << "\n";
    }
    oss << "Total Weight = " << std::setprecision(2) << total << "\n";
    g_resultText = oss.str();

    return 1;
}

EMSCRIPTEN_KEEPALIVE
const char* mwst_compute_text(void) { return g_resultText.c_str(); }

EMSCRIPTEN_KEEPALIVE
const char* mwst_algo_name(void) { return g_algoName.c_str(); }

EMSCRIPTEN_KEEPALIVE
int mwst_node_count(void) { return g_nodeCount; }

EMSCRIPTEN_KEEPALIVE
int mwst_edge_count(void) { return (int)g_inputEdges.size(); }

EMSCRIPTEN_KEEPALIVE
const char* mwst_get_edge_a(int i) {
    if (i < 0 || i >= (int)g_inputEdges.size()) { g_outBuf.clear(); return g_outBuf.c_str(); }
    g_outBuf = g_inputEdges[i].a; return g_outBuf.c_str();
}

EMSCRIPTEN_KEEPALIVE
const char* mwst_get_edge_b(int i) {
    if (i < 0 || i >= (int)g_inputEdges.size()) { g_outBuf.clear(); return g_outBuf.c_str(); }
    g_outBuf = g_inputEdges[i].b; return g_outBuf.c_str();
}

EMSCRIPTEN_KEEPALIVE
const char* mwst_get_edge_weight(int i) {
    if (i < 0 || i >= (int)g_inputEdges.size()) { g_outBuf.clear(); return g_outBuf.c_str(); }
    g_outBuf = g_inputEdges[i].weight; return g_outBuf.c_str();
}

EMSCRIPTEN_KEEPALIVE
int mwst_mst_count(void) { return (int)g_mstEdges.size(); }

EMSCRIPTEN_KEEPALIVE
const char* mwst_mst_get_a(int i) {
    if (i < 0 || i >= (int)g_mstEdges.size()) { g_outBuf.clear(); return g_outBuf.c_str(); }
    g_outBuf = g_mstEdges[i].a; return g_outBuf.c_str();
}

EMSCRIPTEN_KEEPALIVE
const char* mwst_mst_get_b(int i) {
    if (i < 0 || i >= (int)g_mstEdges.size()) { g_outBuf.clear(); return g_outBuf.c_str(); }
    g_outBuf = g_mstEdges[i].b; return g_outBuf.c_str();
}

EMSCRIPTEN_KEEPALIVE
const char* mwst_mst_get_weight(int i) {
    if (i < 0 || i >= (int)g_mstEdges.size()) { g_outBuf.clear(); return g_outBuf.c_str(); }
    g_outBuf = g_mstEdges[i].weight; return g_outBuf.c_str();
}

EMSCRIPTEN_KEEPALIVE
int mwst_total_weight(void) { return g_totalWeight; }

EMSCRIPTEN_KEEPALIVE
const char* mwst_last_error(void) { return g_lastError.c_str(); }

}  // extern "C"

