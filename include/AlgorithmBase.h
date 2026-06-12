#pragma once
#include "Graph.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <limits>

// ────────────────────────────────────────────────────────────────────────────
//  PathResult — everything a UI needs to display after a search.
// ────────────────────────────────────────────────────────────────────────────
struct PathResult {
    std::string      algorithmName;
    std::vector<int> path;          // node IDs, start→end
    double  totalCost     = std::numeric_limits<double>::infinity();
    int     nodesExplored = 0;
    double  timeMs        = 0.0;    // wall-clock ms
    bool    found         = false;

    void print(const Graph& graph) const;
};

// ────────────────────────────────────────────────────────────────────────────
//  AlgorithmBase — abstract base class for every pathfinding algorithm.
//
//  Implement solve() in a derived class.  The optional Callbacks struct lets
//  a UI animate the search step by step (hover a node, highlight an edge …).
// ────────────────────────────────────────────────────────────────────────────
class AlgorithmBase {
public:
    // ── Step-by-step animation hooks ─────────────────────────────────────────
    // Populate these before calling solve(); leave null if not needed.
    struct Callbacks {
        std::function<void(int nodeId, double cost)>       onNodeVisited;  // node popped
        std::function<void(int from, int to, double cost)> onEdgeRelaxed;  // edge improved
        std::function<void(const PathResult&)>             onComplete;     // search done
    };

    explicit AlgorithmBase(std::string name) : name_(std::move(name)) {}
    virtual ~AlgorithmBase() = default;

    // Pure virtual — implement in Dijkstra / AStar / BFS
    virtual PathResult solve(const Graph& graph,
                             int start, int end,
                             const Callbacks* cb = nullptr) = 0;

    const std::string& getName() const { return name_; }

protected:
    std::string name_;

    // Shared utility: walks `parent` map backwards to rebuild path.
    std::vector<int> reconstructPath(
        const std::unordered_map<int,int>& parent,
        int start, int end) const;
};
