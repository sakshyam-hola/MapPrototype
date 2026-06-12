#pragma once
#include <string>

// ────────────────────────────────────────────────────────────────────────────
//  Node — a vertex in the graph.
//  (x, y) are 2-D coordinates used by A*'s heuristic and by any visualiser.
// ────────────────────────────────────────────────────────────────────────────
struct Node {
    int         id   = -1;
    std::string name;
    double      x    = 0.0;
    double      y    = 0.0;

    Node() = default;
    Node(int id, const std::string& name, double x = 0.0, double y = 0.0)
        : id(id), name(name), x(x), y(y) {}

    bool operator==(const Node& o) const { return id == o.id; }
    bool operator!=(const Node& o) const { return id != o.id; }
};
