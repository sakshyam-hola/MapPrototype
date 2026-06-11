#include "osmparser.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <QtMath>
#include <cmath>
#include <limits>

double OsmParser::haversine(double lat1, double lon1,
                            double lat2, double lon2) const {
    const double R = 6371000.0;
    double dLat = qDegreesToRadians(lat2 - lat1);
    double dLon = qDegreesToRadians(lon2 - lon1);
    double a = std::sin(dLat/2)*std::sin(dLat/2) +
               std::cos(qDegreesToRadians(lat1)) *
                   std::cos(qDegreesToRadians(lat2)) *
                   std::sin(dLon/2)*std::sin(dLon/2);
    return R * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0-a));
}

QPointF OsmParser::toPixel(double lat, double lon,
                           double latRange, double lonRange,
                           int W, int H) const {
    const int PAD = 60;
    double x = (lon - minLon) / lonRange * (W - 2*PAD) + PAD;
    double y = (1.0 - (lat - minLat) / latRange) * (H - 2*PAD) + PAD;
    return {x, y};
}

bool OsmParser::parse(const QString& filePath, Graph& graph,
                      int canvasW, int canvasH) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "OsmParser: cannot open" << filePath;
        return false;
    }

    rawNodes.clear();
    graph.clear();
    minLat=90; maxLat=-90; minLon=180; maxLon=-180;

    std::unordered_map<long long, QString> nodeNames;   // name or name:en
    std::unordered_map<long long, QString> nodeTypes;   // amenity/historic/tourism

    // ── PASS 1: all nodes + tags ────────────────────────────────
    {
        QXmlStreamReader xml(&file);
        long long curId = -1;
        bool inNode = false;

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement()) {
                if (xml.name() == u"node") {
                    auto attr  = xml.attributes();
                    curId      = attr.value("id").toLongLong();
                    double lat = attr.value("lat").toDouble();
                    double lon = attr.value("lon").toDouble();
                    rawNodes[curId] = {lat, lon};
                    minLat = std::min(minLat, lat);
                    maxLat = std::max(maxLat, lat);
                    minLon = std::min(minLon, lon);
                    maxLon = std::max(maxLon, lon);
                    inNode = true;
                }
                else if (xml.name() == u"tag" && inNode) {
                    QString k = xml.attributes().value("k").toString();
                    QString v = xml.attributes().value("v").toString();

                    // Prefer English name; fall back to any name
                    if (k == "name:en") {
                        nodeNames[curId] = v; // English name takes priority
                    }
                    else if (k == "name" && !nodeNames.count(curId)) {
                        nodeNames[curId] = v; // only if no English name yet
                    }

                    if (k == "amenity" || k == "historic" ||
                        k == "tourism" || k == "religion")
                        nodeTypes[curId] = v;
                }
            }
            if (xml.isEndElement() && xml.name() == u"node")
                inNode = false;
        }
    }

    if (rawNodes.empty()) {
        qWarning() << "OsmParser: no nodes found in" << filePath;
        return false;
    }

    double latRange = maxLat - minLat;
    double lonRange = maxLon - minLon;
    if (latRange < 1e-9) latRange = 1e-9;
    if (lonRange < 1e-9) lonRange = 1e-9;

    qDebug() << "Bounding box:"
             << minLat << minLon << "→" << maxLat << maxLon;
    qDebug() << "Named nodes found:" << nodeNames.size();

    int graphId = 0;
    std::unordered_map<long long, int> osmToGraph;

    auto getOrCreate = [&](long long osmId,
                           const QString& forceName = "") -> int {
        auto it = osmToGraph.find(osmId);
        if (it != osmToGraph.end()) return it->second;
        if (!rawNodes.count(osmId)) return -1;

        auto&   rn   = rawNodes[osmId];
        QPointF px   = toPixel(rn.lat, rn.lon,
                             latRange, lonRange, canvasW, canvasH);
        QString name = !forceName.isEmpty()    ? forceName
                       : nodeNames.count(osmId)  ? nodeNames[osmId]
                                                : QString("Node%1").arg(graphId);

        int gid = graphId++;
        graph.addNode(gid, name, px, rn.lat, rn.lon);
        osmToGraph[osmId] = gid;
        return gid;
    };

    // ── PASS 2: ways → landmark centroids + road edges ──────────
    file.seek(0);
    {
        QXmlStreamReader xml(&file);
        bool    inWay=false, isHighway=false;
        bool    isLandmark=false, isOneWay=false;
        QString wayName, wayType;
        QList<long long> refs;

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement()) {
                if (xml.name() == u"way") {
                    inWay=true; isHighway=false;
                    isLandmark=false; isOneWay=false;
                    wayName.clear(); wayType.clear();
                    refs.clear();
                }
                else if (xml.name() == u"nd" && inWay) {
                    refs.append(
                        xml.attributes().value("ref").toLongLong());
                }
                else if (xml.name() == u"tag" && inWay) {
                    QString k = xml.attributes().value("k").toString();
                    QString v = xml.attributes().value("v").toString();
                    if (k == "highway")               isHighway  = true;
                    if (k == "amenity" || k == "historic"
                        || k == "tourism")            isLandmark = true;
                    if (k == "oneway" && v == "yes")  isOneWay   = true;
                    if (k == "name:en")               wayName    = v;
                    else if (k == "name" && wayName.isEmpty())
                        wayName    = v;
                    if (k == "amenity" || k == "historic"
                        || k == "tourism")            wayType    = v;
                }
            }

            if (xml.isEndElement() && xml.name() == u"way" && inWay) {
                inWay = false;

                // Landmark way → centroid node
                if (isLandmark && !refs.isEmpty()) {
                    double sLat=0, sLon=0; int cnt=0;
                    for (auto r : refs) {
                        if (rawNodes.count(r)) {
                            sLat += rawNodes[r].lat;
                            sLon += rawNodes[r].lon;
                            ++cnt;
                        }
                    }
                    if (cnt > 0) {
                        double cLat = sLat/cnt, cLon = sLon/cnt;
                        QPointF px  = toPixel(cLat, cLon,
                                             latRange, lonRange,
                                             canvasW, canvasH);
                        QString name = !wayName.isEmpty()
                                           ? wayName : wayType;
                        int gid = graphId++;
                        graph.addNode(gid, name, px, cLat, cLon);
                    }
                }

                // Highway way → road edges
                if (isHighway && refs.size() >= 2) {
                    for (int i = 0; i+1 < refs.size(); ++i) {
                        long long a = refs[i], b = refs[i+1];
                        if (!rawNodes.count(a) || !rawNodes.count(b))
                            continue;
                        int gA = getOrCreate(a);
                        int gB = getOrCreate(b);
                        if (gA < 0 || gB < 0) continue;
                        double d = haversine(rawNodes[a].lat, rawNodes[a].lon,
                                             rawNodes[b].lat, rawNodes[b].lon);
                        graph.addEdgeDirected(gA, gB, d);
                        if (!isOneWay)
                            graph.addEdgeDirected(gB, gA, d);
                    }
                }
            }
        }
    }

    // ── PASS 3: add all named landmark nodes ────────────────────
    for (auto& [osmId, name] : nodeNames) {
        if (!nodeTypes.count(osmId)) continue;
        if (osmToGraph.count(osmId)) continue;
        getOrCreate(osmId, name);
    }

    // ── Connect isolated nodes to nearest road node ──────────────
    const auto& allNodes = graph.getNodes();
    for (auto& [gid, node] : allNodes) {
        if (!node.neighbors.empty()) continue;
        double best  = std::numeric_limits<double>::infinity();
        int    bestId = -1;
        for (auto& [oid, other] : allNodes) {
            if (oid == gid || other.neighbors.empty()) continue;
            double d = haversine(node.lat, node.lon,
                                 other.lat, other.lon);
            if (d < best) { best = d; bestId = oid; }
        }
        if (bestId >= 0 && best < 800)
            graph.addEdge(gid, bestId, best);
    }

    qDebug() << "OsmParser: loaded" << graph.nodeCount()
             << "total nodes from" << filePath;
    return true;
}