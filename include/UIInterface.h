#pragma once
// =============================================================================
//  UIInterface.h  ──  UI DESIGNER INTEGRATION CONTRACT
// =============================================================================
//
//  This header is the ONLY coupling point between the pathfinding backend
//  and any UI layer (Qt, Dear ImGui, wxWidgets, web / Emscripten, ncurses…).
//
//  ┌─────────────────────────────────────────────────────────────────────────┐
//  │  HOW TO PLUG IN YOUR UI                                                 │
//  │                                                                          │
//  │  1. Subclass UIInterface                                                │
//  │  2. Implement all pure-virtual methods (marked = 0)                     │
//  │  3. Override the optional animation hooks if you want live step-by-step │
//  │     node/edge highlighting as the algorithm runs                        │
//  │  4. Hand your UI to PathFinder:                                         │
//  │       PathFinder pf(std::make_unique<YourUI>());                        │
//  │       pf.run();                                                          │
//  └─────────────────────────────────────────────────────────────────────────┘
//
//  ── ANIMATION HOOKS (step-by-step visualisation) ────────────────────────────
//  While an algorithm runs it fires:
//    onAlgorithmStart()   – before the first iteration
//    onNodeVisited()      – each time a node is popped from the frontier
//    onEdgeRelaxed()      – each time a shorter path is found via an edge
//    onAlgorithmComplete()– when the search exits (path found or not)
//
//  Override these to drive your renderer. They are no-ops by default so a
//  simple console UI does not need to care about them.
//
//  ── TRIGGERING SEARCHES FROM YOUR UI ────────────────────────────────────────
//  After setBackend() is called you can reach the engine from any UI event:
//    PathResult r = backend_->findPath(startId, endId, algoIdx);
//    ComparisonResult c = backend_->compareAll(startId, endId);
// =============================================================================

#include "Graph.h"
#include "AlgorithmBase.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>

// ────────────────────────────────────────────────────────────────────────────
//  ComparisonResult — results from all algorithms run head-to-head
// ────────────────────────────────────────────────────────────────────────────
struct ComparisonResult {
    std::vector<PathResult> results;
    int startNode = -1;
    int endNode   = -1;

    void print(const Graph& graph) const;

    // Convenience accessors (return nullptr when no algorithm found a path)
    const PathResult* cheapest()      const;  // lowest totalCost
    const PathResult* fastest()       const;  // lowest timeMs
    const PathResult* leastExplored() const;  // fewest nodesExplored
};

// ────────────────────────────────────────────────────────────────────────────
//  UIInterface — abstract base.  Implement this for your target UI.
// ────────────────────────────────────────────────────────────────────────────
class UIInterface {
public:
    virtual ~UIInterface() = default;

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    virtual void initialize() = 0;   ///< Called once before run()
    virtual void run()        = 0;   ///< Main render / event loop
    virtual void shutdown()   = 0;   ///< Clean-up after run() returns

    // ── Graph display ──────────────────────────────────────────────────────────
    virtual void displayGraph(const Graph& graph) = 0;
    virtual void displayPath(const PathResult& result, const Graph& graph) = 0;
    virtual void displayComparison(const ComparisonResult& cmp,
                                   const Graph& graph) = 0;

    // ── User input ─────────────────────────────────────────────────────────────
    /// Prompt the user to select a node; returns its id.
    virtual int  promptNodeId(const std::string& prompt,
                              const Graph& graph) = 0;
    /// 0 = Dijkstra, 1 = A*, 2 = BFS, 3 = Compare all
    virtual int  promptAlgorithmChoice() = 0;
    /// 0 = City, 1 = Grid, 2 = Sparse, 3 = Load file, 4 = Edit current
    virtual int  promptMapChoice()       = 0;
    virtual bool promptRunAgain()        = 0;

    // ── Map editor ─────────────────────────────────────────────────────────────
    /// Opens an interactive map-editing session; modifies `graph` in-place.
    virtual void openMapEditor(Graph& graph) = 0;

    // ── Status / feedback ──────────────────────────────────────────────────────
    virtual void showMessage(const std::string& msg)            = 0;
    virtual void showError(const std::string& err)              = 0;
    virtual void showProgress(int pct, const std::string& label)= 0;

    // ── Animation hooks (optional — override for live visualisation) ──────────
    virtual void onAlgorithmStart(const std::string& name,
                                  int start, int end)              {}
    virtual void onNodeVisited(int nodeId, double cost)            {}
    virtual void onEdgeRelaxed(int from, int to, double cost)      {}
    virtual void onAlgorithmComplete(const PathResult& result)     {}

    // ── Backend linkage ────────────────────────────────────────────────────────
    /**
     * PathFinder calls this after construction so the UI can drive searches
     * directly (e.g. from a button-click callback in a GUI).
     */
    virtual void setBackend(class PathFinder* pf) { backend_ = pf; }
    class PathFinder* getBackend() const { return backend_; }

protected:
    class PathFinder* backend_ = nullptr;
};
