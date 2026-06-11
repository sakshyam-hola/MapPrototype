#include "dijkstra.h"
#include <queue>
#include <limits>

DijkstraResult Dijkstra::run(const Graph& graph, int source, int target) {
    DijkstraResult result;
    const auto& nodes = graph.getNodes();

    for (auto& [id, _] : nodes) {
        result.dist[id] = std::numeric_limits<double>::infinity();
        result.prev[id] = -1;
    }
    result.dist[source] = 0.0;

    // min-heap: (distance, nodeId)
    using P = std::pair<double, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > result.dist[u]) continue;
        if (u == target) break;

        const Node* node = graph.getNode(u);
        if (!node) continue;
        for (auto& edge : node->neighbors) {
            double nd = result.dist[u] + edge.weight;
            if (nd < result.dist[edge.to]) {
                result.dist[edge.to] = nd;
                result.prev[edge.to] = u;
                pq.push({nd, edge.to});
            }
        }
    }

    // reconstruct path
    if (result.dist.count(target) &&
        result.dist[target] != std::numeric_limits<double>::infinity()) {
        for (int at = target; at != -1; at = result.prev.at(at))
            result.path.push_back(at);
        std::reverse(result.path.begin(), result.path.end());
        result.found = true;
    }
    return result;
}