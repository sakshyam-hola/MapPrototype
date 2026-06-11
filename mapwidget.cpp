#include "mapwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>

MapWidget::MapWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(800, 600);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setStyleSheet("background-color:#1a1a2e;");
}

void MapWidget::setGraph(Graph* g) {
    graph = g; resetSelection();
}
void MapWidget::showPath(const std::vector<int>& path, double dist) {
    currentPath = path; pathDist = dist; update();
}
void MapWidget::clearPath()      { currentPath.clear(); pathDist=0; update(); }
void MapWidget::resetSelection() { startNode=endNode=-1; clearPath(); }

QPointF MapWidget::worldToScreen(QPointF w) const {
    return w * scale + panOffset;
}
QPointF MapWidget::screenToWorld(QPointF s) const {
    return (s - panOffset) / scale;
}

int MapWidget::nodeAtPos(QPointF sp) const {
    if (!graph) return -1;
    double R = std::max(12.0, 12.0 / scale);
    QPointF w = screenToWorld(sp);
    int best = -1;
    double bd = R * R;
    for (auto& [id, node] : graph->getNodes()) {
        double dx = node.pos.x() - w.x();
        double dy = node.pos.y() - w.y();
        double d2 = dx*dx + dy*dy;
        if (d2 < bd) { bd = d2; best = id; }
    }
    return best;
}

void MapWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::RightButton) {
        panning = true;
        lastMouse = e->position();
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    if (e->button() == Qt::LeftButton) {
        int clicked = nodeAtPos(e->position());
        if (clicked == -1) return;
        if (startNode == -1) {
            startNode = clicked; clearPath();
        } else if (endNode == -1 && clicked != startNode) {
            endNode = clicked;
            emit pathRequested(startNode, endNode);
        } else {
            startNode = clicked; endNode = -1; clearPath();
        }
        update();
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent* e) {
    if (panning) {
        panOffset += e->position() - lastMouse;
        lastMouse  = e->position();
        update();
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::RightButton) {
        panning = false;
        setCursor(Qt::CrossCursor);
    }
}

void MapWidget::wheelEvent(QWheelEvent* e) {
    QPointF mw = screenToWorld(e->position());
    double f   = e->angleDelta().y() > 0 ? 1.15 : 1.0/1.15;
    scale       = std::clamp(scale * f, 0.05, 100.0);
    panOffset   = e->position() - mw * scale;
    update();
}

void MapWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (!graph || graph->nodeCount() == 0) {
        p.setPen(QColor("#888888"));
        QFont f; f.setPointSize(14); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter,
                   "No map loaded.\nUse File → Open OSM File");
        return;
    }

    drawEdges(p);
    drawPath(p);
    drawNodes(p);
    drawInfoOverlay(p);
}

void MapWidget::drawEdges(QPainter& p) {
    double lw = std::clamp(1.5 * scale, 0.4, 4.0);
    p.setPen(QPen(QColor("#2d4a6b"), lw, Qt::SolidLine,
                  Qt::RoundCap, Qt::RoundJoin));

    for (auto& [id, node] : graph->getNodes()) {
        QPointF a = worldToScreen(node.pos);
        for (auto& edge : node.neighbors) {
            if (edge.to <= id) continue; // draw each edge once
            const Node* nb = graph->getNode(edge.to);
            if (!nb) continue;
            p.drawLine(a, worldToScreen(nb->pos));
        }
    }
}

void MapWidget::drawPath(QPainter& p) {
    if (currentPath.size() < 2) return;
    double lw = std::clamp(5.0 * scale, 2.5, 12.0);
    p.setPen(QPen(QColor("#ff6b6b"), lw, Qt::SolidLine,
                  Qt::RoundCap, Qt::RoundJoin));
    for (size_t i = 0; i+1 < currentPath.size(); ++i) {
        const Node* a = graph->getNode(currentPath[i]);
        const Node* b = graph->getNode(currentPath[i+1]);
        if (a && b)
            p.drawLine(worldToScreen(a->pos), worldToScreen(b->pos));
    }
}

void MapWidget::drawNodes(QPainter& p) {
    double r = std::clamp(7.0 * scale, 5.0, 16.0);

    for (auto& [id, node] : graph->getNodes()) {
        QPointF sc = worldToScreen(node.pos);

        // Skip off-screen nodes
        if (sc.x() < -20 || sc.y() < -20 ||
            sc.x() > width()+20 || sc.y() > height()+20)
            continue;

        // Determine color
        QColor color("#c77dff"); // default purple = temple
        if (id == startNode)     color = QColor("#00f5d4"); // cyan-green
        else if (id == endNode)  color = QColor("#ff9e00"); // orange
        else {
            for (int pid : currentPath)
                if (pid == id) { color = QColor("#ff6b6b"); break; }
        }

        // Glow ring
        p.setBrush(QColor(color.red(), color.green(), color.blue(), 40));
        p.setPen(Qt::NoPen);
        p.drawEllipse(sc, r * 2.0, r * 2.0);

        // Main circle
        p.setBrush(color);
        p.setPen(QPen(QColor(20, 20, 50),
                      std::max(1.0, 1.5 * scale)));
        p.drawEllipse(sc, r, r);

        // Temple symbol inside
        p.setPen(QColor(20, 20, 50));
        QFont sym;
        sym.setPointSize(std::max(5.0, r * 0.75));
        sym.setBold(true);
        p.setFont(sym);
        p.drawText(QRectF(sc.x()-r, sc.y()-r, r*2, r*2),
                   Qt::AlignCenter, "✦");

        // Name label — show when zoomed in, skip generic node names
        bool hasRealName = !node.name.isEmpty()
                           && !node.name.startsWith("Node");
        if (scale > 1.0 && hasRealName) {
            QString label = node.name.length() > 22
                                ? node.name.left(20) + "…"
                                : node.name;
            QFont lf; lf.setPointSize(7); lf.setBold(true);
            p.setFont(lf);
            QFontMetrics fm(lf);
            int tw = fm.horizontalAdvance(label) + 10;

            QRectF tr(sc.x() - tw/2.0, sc.y() + r + 4, tw, 16);
            p.setBrush(QColor(10, 10, 30, 210));
            p.setPen(color);
            p.drawRoundedRect(tr, 4, 4);
            p.setPen(Qt::white);
            p.drawText(tr, Qt::AlignCenter, label);
        }
    }
}

void MapWidget::drawInfoOverlay(QPainter& p) {
    p.resetTransform();

    // ── Legend box ───────────────────────────────────────────────
    QRect box(12, 12, 230, 126);
    p.setBrush(QColor(10, 10, 30, 220));
    p.setPen(QColor("#333366"));
    p.drawRoundedRect(box, 10, 10);

    struct L { QColor c; QString t; };
    QList<L> legend = {
                       {QColor("#c77dff"), "Temple / Landmark"},
                       {QColor("#00f5d4"), "Start  (1st click)"},
                       {QColor("#ff9e00"), "End    (2nd click)"},
                       {QColor("#ff6b6b"), "Shortest Path"},
                       };
    int y = 26;
    for (auto& l : legend) {
        p.setBrush(l.c); p.setPen(Qt::NoPen);
        p.drawEllipse(22, y, 11, 11);
        p.setPen(Qt::white);
        QFont f; f.setPointSize(8); p.setFont(f);
        p.drawText(40, y + 9, l.t);
        y += 24;
    }

    p.setPen(QColor("#666699"));
    QFont hf; hf.setPointSize(7); p.setFont(hf);
    p.drawText(14, box.bottom() - 5,
               "R-drag: pan   Scroll: zoom   L-click: select");

    // ── Path result bar ──────────────────────────────────────────
    if (!currentPath.empty()) {
        QString sName = graph->getNode(startNode)
        ? graph->getNode(startNode)->name : "?";
        QString eName = graph->getNode(endNode)
                            ? graph->getNode(endNode)->name   : "?";
        QString dStr  = pathDist >= 1000
                           ? QString::number(pathDist/1000.0, 'f', 2) + " km"
                           : QString::number((int)pathDist) + " m";

        QString msg = QString("  ✅  %1  →  %2   |   %3   |   %4 stops")
                          .arg(sName).arg(eName)
                          .arg(dStr).arg(currentPath.size());

        QRect bar(0, height()-44, width(), 44);
        p.setBrush(QColor("#6a0dad"));
        p.setPen(Qt::NoPen);
        p.drawRect(bar);
        p.setPen(Qt::white);
        QFont bf; bf.setPointSize(9); bf.setBold(true); p.setFont(bf);
        p.drawText(bar.adjusted(8,0,-8,0),
                   Qt::AlignVCenter | Qt::AlignLeft, msg);
    }

    // ── Zoom level ────────────────────────────────────────────────
    p.setPen(QColor("#555577"));
    QFont zf; zf.setPointSize(7); p.setFont(zf);
    p.drawText(width()-90, height()-6,
               QString("zoom: %1x").arg(scale, 0, 'f', 1));
}