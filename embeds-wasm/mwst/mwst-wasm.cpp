// mwst-wasm.cpp
//
// Emscripten shim around the unmodified CSE 102 MWST (Minimum Weight
// Spanning Tree) solver — graph.cpp + graph.h from the original
// programming project.
//
// JS interface:
//   mwst_compute(input_text)   — feed it the same line-format the
//                                CLI expected (vert count / edge count
//                                / edges); returns text containing
//                                MST + total cost (matches CLI output).
//   mwst_node_count()          — how many distinct nodes were parsed
//   mwst_edge_count()          — how many edges in the input
//   mwst_get_edge(i, ...)      — read parsed input edge i
//   mwst_mst_count()           — how many edges ended up in the MST
//   mwst_mst_get(i, ...)       — read MST edge i
//   mwst_total_weight()        — sum of MST edge weights
//
// The shim reuses the same input parsing as main.cpp so that input
// text from the original test/ directory works unchanged.

#include "graph.h"

#include <emscripten/emscripten.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
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

// Comparator matches the lambda in main.cpp: descending by weight, so
// std::sort(...) yields ascending after reverse.
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
}

}  // namespace

extern "C" {

/**
 * Run MWST on `input`, which is the same line-format the CLI took:
 *   <numVerts>\n<numEdges>\n<u> <v> <w>\n... (numEdges lines)
 *
 * Returns 1 on success, 0 on parse failure (call mwst_last_error()).
 *
 * After success:
 *   - mwst_compute_text() returns the formatted MST report
 *   - mwst_mst_count() / mwst_mst_get() expose the MST edges
 *   - mwst_total_weight() is the sum
 */
EMSCRIPTEN_KEEPALIVE
int mwst_compute(const char* input) {
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
        // Validate the weight parses as a number.
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

    // Run Prim's. The original prims() returns parent edges as a flat
    // [parent, child, parent, child, ...] vector of strings.
    std::vector<std::string> primsOut = g_graph->prims();

    // Walk the input edges in the order they appeared, collecting
    // edges that are present in the MST result. Match the CLI's
    // sort (ascending weight).
    using Tup = std::tuple<std::string, std::string, std::string, int>;
    std::vector<Tup> mst;
    int idx = 0;
    for (const auto& ie : g_inputEdges) {
        ++idx;  // 1-based input index, like the CLI's "edge index" column
        // primsOut is pairs (parent, child) — match either direction.
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

    float total = 0.0f;
    for (const auto& e : mst) total += std::stof(std::get<2>(e));
    g_totalWeight = (int)total;

    // Save results for the per-edge accessors.
    for (const auto& e : mst) {
        ParsedEdge pe{std::get<0>(e), std::get<1>(e), std::get<2>(e)};
        g_mstEdges.push_back(pe);
    }

    // Build the formatted report (same format as main.cpp's writer).
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
