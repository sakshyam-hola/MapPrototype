#pragma once
#include <QWidget>
#include <QPointF>
#include <vector>
#include "graph.h"

class MapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MapWidget(QWidget* parent = nullptr);

    void setGraph(Graph* g);
    void showPath(const std::vector<int>& path, double distanceMeters);
    void clearPath();
    void resetSelection();

signals:
    void pathRequested(int startId, int endId);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    Graph*           graph      = nullptr;
    std::vector<int> currentPath;
    double           pathDist   = 0.0;
    int              startNode  = -1;
    int              endNode    = -1;

    // Pan & zoom state
    double   scale     = 1.0;
    QPointF  panOffset = {0, 0};
    QPointF  lastMouse;
    bool     panning   = false;

    int     nodeAtPos(QPointF screenPos) const;
    QPointF worldToScreen(QPointF world) const;
    QPointF screenToWorld(QPointF screen) const;

    void drawEdges(QPainter& p);
    void drawPath(QPainter& p);
    void drawNodes(QPainter& p);
    void drawInfoOverlay(QPainter& p);
};