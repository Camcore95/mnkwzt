#ifndef PRINT_H
#define PRINT_H

#include <iostream>
#include "link.h"
#include "demand.h"

std::ostream& operator<<(std::ostream& p_out, const Link& p_link)
{
    p_out << "link(" << p_link.getSourceNode() << ", " << p_link.getTargetNode()
          << "), throughput: " << p_link.getSummarizedThroughput();
    return p_out;
}

std::ostream& operator<<(std::ostream& p_out, const Demand& p_link)
{
    p_out << "demand(" << p_link.getSourceNode() << ", " << p_link.getTargetNode()
          << "), data size: " << p_link.getData();
    return p_out;
}

#endif // PRINT_H
