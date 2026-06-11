#pragma once
#include <QMainWindow>
#include <QLabel>
#include "graph.h"
#include "mapwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    MapWidget* mapWidget;
    Graph      graph;
    QLabel*    statusLabel;

    void loadOsmData();
    void onPathRequested(int startId, int endId);
};