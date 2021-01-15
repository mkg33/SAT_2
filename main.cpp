#include <fstream>
#include <iostream>

#include "solver.hpp"

int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <DIMACS file>" << " <selection heuristics: WITHOUT, YESNO, RANDOM, DLIS, RDLIS, DLCS, RDLCS, JW>\n";
        return 1;
    }

    std::ifstream stream(argv[1]);
    std::string option(argv[2]);
    Solver solver(stream, option);

    solver.solve();
    std::cout << solver;

    if (std::cout.bad()) {
        std::cerr << "Error while printing.\n";
        return 1;
    }
}
