#pragma once
#include "graph.h"
#include <vector>
#include <unordered_map>

struct DijkstraResult {
    std::unordered_map<int, double> dist;
    std::unordered_map<int, int>    prev;
    std::vector<int>                path;
    bool found = false;
};

class Dijkstra {
public:
    static DijkstraResult run(const Graph& graph, int source, int target);
};