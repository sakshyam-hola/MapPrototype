#include "mainwindow.h"
#include "osmparser.h"
#include "dijkstra.h"
#include <QStatusBar>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Kathmandu Temple Map — Dijkstra Routing");
    resize(1280, 800);

    mapWidget = new MapWidget(this);
    setCentralWidget(mapWidget);

    statusLabel = new QLabel("Loading map data...");
    statusBar()->addWidget(statusLabel);

    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Open OSM File...", this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, "Open OSM File", "", "OSM Files (*.osm)");
        if (!path.isEmpty()) {
            graph.clear();
            OsmParser parser;
            statusLabel->setText("Parsing OSM data...");
            QApplication::processEvents();
            if (parser.parse(path, graph, 1200, 750)) {
                mapWidget->setGraph(&graph);
                statusLabel->setText(
                    QString("Loaded %1 nodes — Left-click START then END.")
                        .arg(graph.nodeCount()));
            } else {
                QMessageBox::warning(this, "Error",
                                     "Failed to parse OSM file.");
            }
        }
    });
    fileMenu->addSeparator();
    fileMenu->addAction("&Reset Selection",
                        mapWidget, &MapWidget::resetSelection);
    fileMenu->addAction("E&xit", qApp, &QApplication::quit);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&How to Use", this, [this]() {
        QMessageBox::information(this, "How to Use",
                                 "1. Left-click any temple node  →  sets START (green)\n"
                                 "2. Left-click another node     →  sets END (orange)\n"
                                 "3. Shortest path shown in red\n"
                                 "4. Right-drag to pan the map\n"
                                 "5. Scroll wheel to zoom in/out\n"
                                 "6. Left-click again to reset selection");
    });

    connect(mapWidget, &MapWidget::pathRequested,
            this,      &MainWindow::onPathRequested);

    loadOsmData();
}

void MainWindow::loadOsmData() {
    OsmParser parser;

    QFile testFile(":/temple.osm");
    if (!testFile.exists()) {
        qDebug() << "ERROR: :/temple.osm not found in QRC!";
        statusLabel->setText(
            "No map loaded — use File → Open OSM File.");
        return;
    }
    qDebug() << "Found :/temple.osm, size:" << testFile.size() << "bytes";

    if (parser.parse(":/temple.osm", graph, 1200, 750)) {
        qDebug() << "Parse SUCCESS — nodes:" << graph.nodeCount();
        mapWidget->setGraph(&graph);
        statusLabel->setText(
            QString("Loaded %1 temple nodes  |  "
                    "Left-click a node to set START, then END.")
                .arg(graph.nodeCount()));
    } else {
        qDebug() << "Parse FAILED";
        statusLabel->setText(
            "Failed to parse temple.osm — check Application Output.");
    }
}

void MainWindow::onPathRequested(int startId, int endId) {
    statusLabel->setText("Finding shortest path...");
    QApplication::processEvents();

    auto result = Dijkstra::run(graph, startId, endId);

    if (!result.found) {
        statusLabel->setText("No path found between selected nodes.");
        QMessageBox::information(this, "No Path",
                                 "No path found between these two temples.\n"
                                 "Try selecting nodes that are closer together.");
        return;
    }

    double dist = result.dist[endId];
    mapWidget->showPath(result.path, dist);
}