#include <fstream>
#include <iostream>

#include "solver.hpp"

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <DIMACS file>\n";
        return 1;
    }

    std::ifstream stream(argv[1]);
    Solver solver(stream);

    const bool sat = solver.solve();
    std::cout << solver;

    if (sat)
        std::cout << "SAT\n";
    else
        std::cout << "UNSAT\n";

    if (std::cout.bad()) {
        std::cerr << "Error while printing.\n";
        return 1;
    }
}
