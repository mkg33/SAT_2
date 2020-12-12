#include <iostream>
#include <ffstream>
#include <sstream>
#include <string>

#include <solver.hpp>


int main(int argc, char *argv)
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <DIMACS file>" << std::endl;
        return 0;
    }

    std::ifstream infile(argv[1]);
    if (infile.fail()) {
        std::cout << "Invalid file.\n";
    }
}
