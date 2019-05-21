#ifndef DEMAND_H
#define DEMAND_H

#include "node.h"

class Demand
{
public:
    Demand(Node p_src, Node p_dst, double p_data)
        :source(p_src), target(p_dst), data(p_data) {}

    Demand& operator=(const Demand& p_other)
    {
        const_cast<Node&>(source) = p_other.getSourceNode();
        const_cast<Node&>(target) = p_other.getTargetNode();
        const_cast<double&>(data) = p_other.getData();
        return *this;
    }

    Node getSourceNode() const { return source; }
    Node getTargetNode() const { return target; }
    double getData() const { return data; }

public:
    const Node source;
    const Node target;
    const double data;
};

#endif // DEMAND_H
