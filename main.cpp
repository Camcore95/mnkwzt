
#include <iostream>
#include "solver.h"
#include "demand.h"
#include <cstdlib>
#include <ctime>

using namespace std;

int main()
{
    auto l_start = clock();
    const unsigned int l_numOfNodes = 100;
    Solver solver(l_numOfNodes);
    for(auto i = 0; i < l_numOfNodes; ++i)
    {
        for(auto j = 0; j < l_numOfNodes; ++j)
        {
            if(i != j)
                solver.addDemand(Demand(i, j, rand() % 20 + 1));
        }
    }
    solver.solve();
    std::cout << "Computing time: " << (clock() - l_start) / CLOCKS_PER_SEC << std::endl;
    return 0;
}
