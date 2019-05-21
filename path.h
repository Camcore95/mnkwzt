#ifndef PATH_H
#define PATH_H
#include <vector>

using LinkIndicator = unsigned int;

class Path
{
public:
    Path() = default;
    Path(double p_initCost) : cost(p_initCost) {}

    double getCost() const { return cost; }
    void setCost(double p_cost) { cost = p_cost; }
    void addCost(double p_val) { cost += p_val; }
    void addToPath(LinkIndicator p_indicator) { path.push_back(p_indicator); }
    unsigned int getPathLength() const { return path.size(); }
    std::vector<LinkIndicator>& getPath() { return path; }
private:
    std::vector<LinkIndicator> path;
    double cost = 0;
};

#endif // PATH_H
