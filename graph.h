#pragma once
#include <QString>
#include <QPointF>
#include <vector>
#include <unordered_map>

struct Edge {
    int    to;
    double weight; // real-world meters
};

struct Node {
    int     id;
    QString name;
    QPointF pos;      // pixel position on canvas
    double  lat, lon; // real-world coordinates
    std::vector<Edge> neighbors;
};

class Graph {
public:
    void addNode(int id, const QString& name,
                 QPointF pos, double lat = 0, double lon = 0);
    void addEdge(int from, int to, double weight);
    void addEdgeDirected(int from, int to, double weight);
    void clear();

    Node*       getNode(int id);
    const Node* getNode(int id) const;
    const std::unordered_map<int, Node>& getNodes() const;
    int         nodeCount() const;

private:
    std::unordered_map<int, Node> nodes;
};