#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "solver.hpp"


int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <DIMACS file>" << std::endl;
        return 0;
    }

    std::ifstream infile(argv[1]);
    if (infile.fail()) {
        std::cout << "Invalid file.\n";
    }

    std::string line;
    std::string dimacs;

    while (std::getline(infile, line))
        if (!(line.find("c", 0) == 0)) //ignore comments
        {
            dimacs += line + '\n';
        }

    //for debugging: std::cout << dimacs << std::endl;

    infile.close();

    Solver *solver = new Solver(dimacs);

    std::cout << *solver << std::endl; //print processed input file
}
