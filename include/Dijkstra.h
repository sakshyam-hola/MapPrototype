#pragma once
#include "AlgorithmBase.h"

// ────────────────────────────────────────────────────────────────────────────
//  Dijkstra — classic single-source shortest path using a min-heap.
//  Guarantees optimal cost on non-negative weighted graphs.
//  Time: O((V + E) log V)
// ────────────────────────────────────────────────────────────────────────────
class Dijkstra : public AlgorithmBase {
public:
    Dijkstra() : AlgorithmBase("Dijkstra's Algorithm") {}

    PathResult solve(const Graph& graph,
                     int start, int end,
                     const Callbacks* cb = nullptr) override;
};
