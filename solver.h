#ifndef SOLVER_H
#define SOLVER_H

#include "node.h"
#include "link.h"
#include "demand.h"
#include "path.h"
#include <vector>
#include <utility>
#include <memory>
#include <limits>
#include <fstream>


constexpr double MODULE_TURN_ON_COST = 10;
constexpr double LINK_THROUGHPUT_COST = 0.1;
constexpr double LINK_MAX_THROUGHPUT = 10;

using Nodes = std::vector<Node>;
using Links = std::vector<Link>;
using Demands = std::vector<Demand>;

class Solver
{
public:
    Solver(unsigned int p_numOfNodes);
    ~Solver() { logger.close(); }

    void addDemands(std::vector<Demand>&&);
    void addDemand(Demand&&);
    void solve();

private:
    Nodes nodes;
    Links links;
    Demands demands;
    mutable std::ofstream logger;

    void setupNodes(unsigned int);
    void setupLinks(unsigned int);

    Path findShortestPath(std::vector<LinkIndicator>, Demand);

    enum class NodeRole
    {
        source,
        target
    };

    std::vector<LinkIndicator> createLinkIndicators() const;
    std::vector<LinkIndicator> findLinks(const std::vector<LinkIndicator>&, Node, NodeRole) const;
    void removeLinksRelatedWithNode(std::vector<LinkIndicator>&, Node, NodeRole);
    double calculateSendingDataCost(Link&, double);
    long double calculateGoalFunction() const;
    long double calculateSummarizedVertexDemandedDataSize(Node, NodeRole) const;
    bool validateLimitations(const std::vector<LinkIndicator>&) const;
    bool validatePathConsistency(Path &, const Demand&) const;
    void logResult(const std::vector<LinkIndicator>&) const;
    void saveResolvedPath(Path&, const Demand&);
};

#endif // SOLVER_H
