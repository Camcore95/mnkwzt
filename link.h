#ifndef LINK_H
#define LINK_H

#include "node.h"
#include <cmath>
#include <iostream>

class Link
{
public:
    Link(Node p_out, Node p_in, double p_throughputCost, double p_turningOn, double p_maxThroughput)
        :out(p_out), in(p_in), throughputCost(p_throughputCost), turnOnCost(p_turningOn), moduleMaxThroughput(p_maxThroughput)
    {}

    Node getSourceNode() const { return out; }
    Node getTargetNode() const { return in; }
    double getThroughputCost() const { return throughputCost; }
    double getTurnOnCost() const { return turnOnCost; }
    double getModuleMaxThroughput() const { return moduleMaxThroughput; }
    unsigned int getOnlineModulesNumber() const { return onlineModules; }
    void setOnlineModulesNumber(unsigned int p_num) { onlineModules = p_num; }
    long double getSummarizedThroughput() const { return summarizedThroughput; }
    void setSummarizedThroughput(long double p_val) { summarizedThroughput = p_val; }
    void addToThroughput(double p_val) { summarizedThroughput += p_val;
                                         onlineModules = ceil(summarizedThroughput / moduleMaxThroughput); }

private:
    const Node out;
    const Node in;
    const double throughputCost;
    const double turnOnCost;
    const double moduleMaxThroughput;
    unsigned int onlineModules = 0;
    long double summarizedThroughput = 0;
};

#endif // LINK_H
