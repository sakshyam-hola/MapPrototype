#pragma once
#include "Graph.h"
#include "AlgorithmBase.h"
#include "UIInterface.h"
#include <memory>
#include <vector>
#include <string>

// ────────────────────────────────────────────────────────────────────────────
//  PathFinder — top-level coordinator.
//
//  Owns:
//    • Graph         — the map being searched
//    • Algorithm registry — any number of AlgorithmBase derivations
//    • UIInterface   — pluggable UI layer
//
//  The UI calls findPath() / compareAll() whenever the user requests a search.
//  Results are passed back through PathResult / ComparisonResult and the
//  optional animation callbacks in AlgorithmBase::Callbacks.
// ────────────────────────────────────────────────────────────────────────────
class PathFinder {
public:
    PathFinder();
    explicit PathFinder(std::unique_ptr<UIInterface> ui);

    // ── Graph management ───────────────────────────────────────────────────────
    void         loadPreset(Graph::Preset preset);
    void         setGraph(Graph graph);
    Graph&       graph()       { return graph_; }
    const Graph& graph() const { return graph_; }

    // ── Pathfinding ────────────────────────────────────────────────────────────
    PathResult       findPath(int start, int end, int algoIndex);
    ComparisonResult compareAll(int start, int end);

    // ── Algorithm registry ─────────────────────────────────────────────────────
    void addAlgorithm(std::unique_ptr<AlgorithmBase> algo);
    std::vector<std::string> algorithmNames() const;
    int  algorithmCount() const;

    // ── UI ─────────────────────────────────────────────────────────────────────
    void setUI(std::unique_ptr<UIInterface> ui);
    UIInterface* ui() const { return ui_.get(); }

    /// Calls ui->initialize(), ui->run(), ui->shutdown().
    void run();

private:
    Graph  graph_;
    std::vector<std::unique_ptr<AlgorithmBase>> algos_;
    std::unique_ptr<UIInterface>                ui_;

    void registerDefaultAlgorithms();

    // Builds an AlgorithmBase::Callbacks that forwards events to the UI.
    AlgorithmBase::Callbacks makeCallbacks();
};
