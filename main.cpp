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
    int counter = 0; //check if the number of clauses matches the number of lines

    while (std::getline(infile, line))
        if (!(line.find("c", 0) == 0)) //ignore comments
        {
            dimacs += line + '\n';
            counter ++;
        }
    //for debugging: std::cout << dimacs << std::endl;
    infile.close();

    try
    {
        Solver *solver = new Solver(dimacs);
        if (counter - 1 != solver->getNumberClauses()) //subtract 1 because the first read line is 'p cnf'
        {
            std::cout << "Invalid input. The number of clauses does not match the number of lines." << std::endl;
        }
        else
        {
            std::cout << *solver << std::endl; //print processed input file
        }
    }
    catch(std::invalid_argument &e)
    {
        std::cerr << e.what() << std::endl;
    }
}
