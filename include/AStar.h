#pragma once
#include "AlgorithmBase.h"

// ────────────────────────────────────────────────────────────────────────────
//  AStar — A* search using node coordinates for the heuristic.
//
//  With EUCLIDEAN or MANHATTAN heuristics it explores fewer nodes than
//  Dijkstra while still guaranteeing optimal cost (admissible heuristics).
//  Setting ZERO degrades it to Dijkstra behaviour.
// ────────────────────────────────────────────────────────────────────────────
class AStar : public AlgorithmBase {
public:
    enum class Heuristic { EUCLIDEAN, MANHATTAN, ZERO };

    explicit AStar(Heuristic h = Heuristic::EUCLIDEAN)
        : AlgorithmBase("A* Search"), heuristic_(h) {}

    PathResult solve(const Graph& graph,
                     int start, int end,
                     const Callbacks* cb = nullptr) override;

    void      setHeuristic(Heuristic h) { heuristic_ = h; }
    Heuristic getHeuristic() const      { return heuristic_; }

private:
    Heuristic heuristic_;
    double h(const Node& a, const Node& goal) const;
};
