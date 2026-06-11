#include "graph.h"

void Graph::addNode(int id, const QString& name,
                    QPointF pos, double lat, double lon) {
    nodes[id] = {id, name, pos, lat, lon, {}};
}

void Graph::addEdgeDirected(int from, int to, double weight) {
    if (nodes.count(from))
        nodes[from].neighbors.push_back({to, weight});
}

void Graph::addEdge(int from, int to, double weight) {
    addEdgeDirected(from, to, weight);
    addEdgeDirected(to, from, weight);
}

void Graph::clear() { nodes.clear(); }

Node* Graph::getNode(int id) {
    auto it = nodes.find(id);
    return it != nodes.end() ? &it->second : nullptr;
}

const Node* Graph::getNode(int id) const {
    auto it = nodes.find(id);
    return it != nodes.end() ? &it->second : nullptr;
}

const std::unordered_map<int, Node>& Graph::getNodes() const {
    return nodes;
}

int Graph::nodeCount() const { return (int)nodes.size(); }