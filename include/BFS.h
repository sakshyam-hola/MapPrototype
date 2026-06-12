#pragma once
#include "AlgorithmBase.h"

// ────────────────────────────────────────────────────────────────────────────
//  BFS — Breadth-First Search.
//  Finds the minimum-hop path. On unweighted / unit-weight graphs this is also
//  the minimum-cost path. On variable-weight graphs it may not be optimal by
//  cost, but it serves as a useful reference and explores the fewest hops.
//  Time: O(V + E)
// ────────────────────────────────────────────────────────────────────────────
class BFS : public AlgorithmBase {
public:
    BFS() : AlgorithmBase("Breadth-First Search") {}

    PathResult solve(const Graph& graph,
                     int start, int end,
                     const Callbacks* cb = nullptr) override;
};
