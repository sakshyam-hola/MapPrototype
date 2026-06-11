#pragma once
#include "graph.h"
#include <QString>
#include <QPointF>
#include <unordered_map>

class OsmParser {
public:
    bool parse(const QString& filePath, Graph& graph,
               int canvasW = 1200, int canvasH = 800);

    // bounding box of parsed data (for display)
    double minLat, maxLat, minLon, maxLon;

private:
    struct RawNode { double lat, lon; };
    std::unordered_map<long long, RawNode> rawNodes;

    QPointF toPixel(double lat, double lon,
                    double latRange, double lonRange,
                    int W, int H) const;

    double haversine(double lat1, double lon1,
                     double lat2, double lon2) const;
};