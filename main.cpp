#include <fstream>
#include <iostream>

#include "solver.hpp"

int main(int argc, char **argv)
{
    std::string option = "without"; // default: no selection heuristic
    if (argc < 2) { // 'ANSI shadow' font from https://www.patorjk.com/software/taag/
        std::cout << R"(
███╗   ███╗ █████╗ ██████╗ ██╗  ██╗███████╗ ██████╗ ██╗     ██╗   ██╗███████╗██████╗
████╗ ████║██╔══██╗██╔══██╗██║  ██║██╔════╝██╔═══██╗██║     ██║   ██║██╔════╝██╔══██╗
██╔████╔██║███████║██████╔╝███████║███████╗██║   ██║██║     ██║   ██║█████╗  ██████╔╝
██║╚██╔╝██║██╔══██║██╔═══╝ ██╔══██║╚════██║██║   ██║██║     ╚██╗ ██╔╝██╔══╝  ██╔══██╗
██║ ╚═╝ ██║██║  ██║██║     ██║  ██║███████║╚██████╔╝███████╗ ╚████╔╝ ███████╗██║  ██║
╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚══════╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝
        )";
        std::cerr <<"\nUsage: " << argv[0] << " <DIMACS file>" << " optional selection heuristic [case insensitive]:\n <YESNO | RANDOM | DLIS | RDLIS | DLCS | RDLCS | JW | RJW | MOMS | RMOMS | lucky>\n\n"
        << "Available selection heuristics: \n" << "- YES/NO: select the first available literal and use a random yes/no decision during the selection\n"
        << "- RANDOM: select a random literal\n" << "- DLIS: Dynamic Largest Individual Sum\n" << "- RDLIS: randomized Dynamic Largest Individual Sum\n"
        << "- DLCS: Dynamic Largest Combined Sum\n" << "- RDLCS: randomized Dynamic Largest Combined Sum\n" << "- JW: Jeroslow-Wang heuristic\n"
        << "- RJW: randomized Jeroslow-Wang heuristic\n" << "- MOMS: Maximum [number of] Occurrences in Minimum [length] Clauses\n"
        << "- RMOMS: randomized Maximum [number of] Occurrences in Minimum [length] Clauses\n" << "- lucky: select random heuristic\n\n"
        << "Note: If no selection heuristic is specified, the first available literal is selected.\n\n";

        return 1;
    }
    else if (argc == 3) {
        option = argv[2];
    }

    std::ifstream stream(argv[1]);

    Solver solver(stream, option);

    solver.solve();
    std::cout << solver;

    if (std::cout.bad()) {
        std::cerr << "Error while printing.\n";
        return 1;
    }
}
