#include "solver.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cassert>
#include "print.h"
#include <queue>
#include <iomanip>

Solver::Solver(unsigned int p_numOfNodes)
{
    setupNodes(p_numOfNodes);
    setupLinks(p_numOfNodes);
    logger.open("logs.txt", std::ios::in | std::ios::trunc);
    if(not logger.is_open())
    {
        std::cout << "Unable to create log file" << std::endl;
    }
}

void Solver::addDemands(Demands&& p_demands)
{
    std::move(std::begin(p_demands), std::end(p_demands), std::back_inserter(demands));
}

void Solver::addDemand(Demand&& p_demand)
{
    demands.push_back(std::move(p_demand));
}

void Solver::solve()
{
    std::vector<LinkIndicator> l_links = createLinkIndicators();

    std::sort(std::begin(demands), std::end(demands), [](auto& p_lhs, auto& p_phs)
                                    { return p_lhs.getData() > p_phs.getData(); });

    for (const auto& demand: demands)
    {
        auto l_path = findShortestPath(l_links, demand);
        assert(validatePathConsistency(l_path, demand));
        saveResolvedPath(l_path, demand);
    }
}

void Solver::setupNodes(unsigned int p_numOfNodes)
{
    nodes.reserve(p_numOfNodes);
    for (unsigned int i = 0 ;i < p_numOfNodes ; ++i)
    {
        nodes.push_back(i);
    }
}

void Solver::setupLinks(unsigned int p_numOfNodes)
{
    const long unsigned int l_numOfLinks = p_numOfNodes * (p_numOfNodes - 1);
    links.reserve(l_numOfLinks);
    for (long unsigned int i = 0; i < l_numOfLinks; ++i)
    {
        const unsigned int l_nodeOut = static_cast<unsigned int>(i / (p_numOfNodes - 1));
        unsigned int l_mod = i % (p_numOfNodes - 1);
        unsigned int l_nodeIn = l_mod;
        if(l_nodeOut <= l_mod)   ++l_nodeIn;
        links.emplace_back(nodes[l_nodeOut],
                           nodes[l_nodeIn],
                           LINK_THROUGHPUT_COST,
                           MODULE_TURN_ON_COST,
                           LINK_MAX_THROUGHPUT);
    }
}

Path Solver::findShortestPath(std::vector<LinkIndicator> p_links, Demand p_demand)
{
    using CostToNode = std::pair<double, Node>;

    std::vector<Path> l_paths(nodes.size(), std::numeric_limits<double>::max());

    std::priority_queue<CostToNode, std::vector<CostToNode>, std::greater<CostToNode>> l_queue;

    const auto l_src = p_demand.getSourceNode();
    const auto l_dst = p_demand.getTargetNode();

    l_queue.push(std::make_pair(0, l_src));
    l_paths[l_src].setCost(0);

    while (!l_queue.empty())
    {
        const auto l_visitedNode = l_queue.top().second;
        if(l_visitedNode == l_dst or l_queue.top().first >= l_paths[l_dst].getCost()) break;
        l_queue.pop();
        auto l_linksFromSource = findLinks(p_links, l_visitedNode, NodeRole::source);

        removeLinksRelatedWithNode(p_links, l_visitedNode, NodeRole::target);

        for (auto link : l_linksFromSource)
        {
            const auto l_linkDstNode = links[link].getTargetNode();
            const auto l_cost = calculateSendingDataCost(links[link], p_demand.getData());

            if (l_paths[l_linkDstNode].getCost() > l_paths[l_visitedNode].getCost() + l_cost)
            {
                l_paths[l_linkDstNode] = l_paths[l_visitedNode];
                l_paths[l_linkDstNode].addToPath(link);
                l_paths[l_linkDstNode].addCost(l_cost);
                l_queue.push(std::make_pair(l_paths[l_linkDstNode].getCost(), l_linkDstNode));
            }
        }
    }
    return l_paths[l_dst];
}


std::vector<LinkIndicator> Solver::findLinks(const std::vector<LinkIndicator>& p_links, Node p_node, NodeRole p_role) const
{
    std::vector<LinkIndicator> l_links;
    for(auto it = std::begin(p_links); it != std::end(p_links); ++it)
    {
        if((p_role == NodeRole::source and links[*it].getSourceNode() == p_node)
            or (p_role == NodeRole::target and links[*it].getTargetNode() == p_node))
        {
            l_links.push_back(*it);
        }
    }
    return l_links;
}

double Solver::calculateSendingDataCost(Link& p_link, double p_data)
{
    auto l_actModulesIfSent = ceil((p_link.getSummarizedThroughput() + p_data) / p_link.getModuleMaxThroughput());
    return (l_actModulesIfSent - p_link.getOnlineModulesNumber()) * p_link.getTurnOnCost() + p_link.getThroughputCost() * p_data;
}

void Solver::removeLinksRelatedWithNode(std::vector<LinkIndicator>& p_links, Node p_node, NodeRole p_role)
{
    for(auto it = std::begin(p_links); it != std::end(p_links); ++it)
    {
        if((p_role == NodeRole::source and links[*it].getSourceNode() == p_node) or
           (p_role == NodeRole::target and links[*it].getTargetNode() == p_node))
        {
            p_links.erase(it);
            --it;
        }
    }
}

long double Solver::calculateGoalFunction() const
{
    long double l_result = 0;
    auto l_calcForLink = [](const auto& p_link)
        { return (p_link.getOnlineModulesNumber() * p_link.getTurnOnCost()) +
                 (p_link.getSummarizedThroughput() * p_link.getThroughputCost()); };

    for(const auto& link :links)
    {
        l_result += l_calcForLink(link);
    }

    return l_result;
}

bool Solver::validateLimitations(const std::vector<LinkIndicator>& p_links) const
{
    for(auto node : nodes)
    {
        const auto l_linksOut = findLinks(p_links, node, NodeRole::source);
        const auto l_linksIn = findLinks(p_links, node, NodeRole::target);

        long double summarizedTrafficOut = 0;
        long double summarizedTrafficIn = 0;

        for(const auto& link : l_linksOut)
        {
            summarizedTrafficOut += links[link].getSummarizedThroughput();
        }

        for(const auto& link : l_linksIn)
        {
            summarizedTrafficIn += links[link].getSummarizedThroughput();
        }
        if(static_cast<bool>(summarizedTrafficIn - summarizedTrafficOut +
                    calculateSummarizedVertexDemandedDataSize(node, NodeRole::source) -
                    calculateSummarizedVertexDemandedDataSize(node, NodeRole::target)))
        {
            logger << "Node " << node << "traffic is not valid!" << std::endl;
            return false;
        }

    }
    return true;
}

long double Solver::calculateSummarizedVertexDemandedDataSize(Node p_node, NodeRole p_role) const
{
    long double l_dataSum = 0;

    for(const auto& demand: demands)
    {
        if((p_role == NodeRole::source and p_node == demand.getSourceNode()) or
           (p_role == NodeRole::target and p_node == demand.getTargetNode()))
        {
            l_dataSum += demand.getData();
        }
    }
    return l_dataSum;
}

bool Solver::validatePathConsistency(Path& l_path, const Demand& l_demand) const
{
    auto& l_links = l_path.getPath();
    for(auto it = std::begin(l_links); it != std::end(l_links); ++it)
    {
        if((it != std::begin(l_links) and links[*(it - 1)].getTargetNode() != links[*it].getSourceNode()) or
           (it == std::begin(l_links) and links[*it].getSourceNode() != l_demand.getSourceNode()) or
           (it == (std::end(l_links) - 1 ) and links[*it].getTargetNode() != l_demand.getTargetNode()))
        {
            return false;
        }
    }
    return true;
}

std::vector<LinkIndicator> Solver::createLinkIndicators() const
{
    std::vector<LinkIndicator> l_links;
    l_links.reserve(links.size());
    for(unsigned int i = 0; i < links.size(); ++i)
    {
        l_links.push_back(i);
    }
    return l_links;
}

void Solver::logResult(const std::vector<LinkIndicator>& p_links) const
{
    for (const auto& link : links)
    {
        logger << link << "  " << link.getOnlineModulesNumber() << std::endl;
    }
    assert(validateLimitations(p_links));
    logger << "Goal function result: " << std::setprecision(10)
           << calculateGoalFunction() << std::endl;
}

void Solver::saveResolvedPath(Path& p_path, const Demand& p_demand)
{
    static unsigned int l_numOfDemand = 1;
    logger << "############## " << l_numOfDemand++  << ":  "<< p_demand <<", cost: "
           << p_path.getCost() << ", path length: " << p_path.getPathLength() << std::endl << "\t";
    for(auto link: p_path.getPath())
    {
        links[link].addToThroughput(p_demand.getData());
        logger << links[link] << std::endl << "\t";
    }
    logger << std::endl;
}

