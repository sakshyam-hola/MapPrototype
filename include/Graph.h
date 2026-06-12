#pragma once
#include "Node.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <limits>

// ────────────────────────────────────────────────────────────────────────────
//  Edge — weighted directed half-edge stored in an adjacency list.
// ────────────────────────────────────────────────────────────────────────────
struct Edge {
    int    from   = -1;
    int    to     = -1;
    double weight = 1.0;

    Edge(int f, int t, double w = 1.0) : from(f), to(t), weight(w) {}
};

// ────────────────────────────────────────────────────────────────────────────
//  Graph — supports directed/undirected, dynamic node/edge management,
//           built-in map presets, and file-based save / load.
// ────────────────────────────────────────────────────────────────────────────
class Graph {
public:
    // Built-in map presets (add more as needed)
    enum class Preset { CITY_MAP, GRID_4x4, SPARSE_GRAPH };

    explicit Graph(bool directed = false);

    // ── Node operations ──────────────────────────────────────────────────────
    int         addNode(const std::string& name, double x = 0.0, double y = 0.0);
    bool        removeNode(int id);
    bool        updateNode(int id, const std::string& name, double x, double y);
    const Node& getNode(int id) const;
    bool        hasNode(int id) const;
    std::vector<int> nodeIds() const;  // sorted
    int         nodeCount() const;

    // ── Edge operations ──────────────────────────────────────────────────────
    bool   addEdge(int from, int to, double weight = 1.0);
    bool   removeEdge(int from, int to);
    bool   updateWeight(int from, int to, double weight);
    bool   hasEdge(int from, int to) const;
    double getWeight(int from, int to) const;  // ∞ if missing
    const std::vector<Edge>& neighbors(int id) const;
    int    edgeCount() const;

    // ── Map management ───────────────────────────────────────────────────────
    void  loadPreset(Preset preset);
    void  clear();
    bool  loadFromFile(const std::string& path);
    bool  saveToFile(const std::string& path) const;

    // ── Misc ───────────────────────────────────────────────────────────────
    void        print() const;
    bool        isDirected() const { return directed_; }
    std::string presetName(Preset p) const;

private:
    std::unordered_map<int, Node>              nodes_;
    std::unordered_map<int, std::vector<Edge>> adj_;
    bool directed_;
    int  nextId_;

    static const std::vector<Edge> kEmpty_;   // returned for unknown node ids

    void buildCityMap();
    void buildGrid4x4();
    void buildSparseGraph();
};
