#include <vector>
#include <set>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

#include "solver.hpp"

using uint = unsigned int;

//TODO: will add exception handling later.

/**
 * Initializes clauses from input file.
 */
Solver::Solver(const std::string &dimacs)
{
    std::set<int> variables;
    char c;
    int n;
    std::stringstream buf(dimacs);

    // Skip whitespace and 'p cnf'.
    buf >> std::skipws;
    for (int i = 0; i < 4; i++)
    {
        buf >> c;
    }
    buf >> this->numberVariables;
    buf >> this->numberClauses;

    for (int i = 0; i < this->numberClauses; i++)
    {
        while (buf >> n)
        {
            if (n == 0)
            {
                this->clauses.push_back(variables);
                variables.clear();
                break;
            }
            else
            {
                variables.insert(n);
            }
        }
    }
}

/**
 * operator<<(out, solver)
 * Print the processed DIMACS file.
 */
std::ostream& operator<<(std::ostream& out, const Solver& solver)
{
    out << "number of clauses: " << solver.numberClauses << std::endl;
    out << "number of variables: " << solver.numberVariables << std::endl;

    for (std::set<int> const &variables : solver.clauses) {
        for(const int i : variables) {
            std::cout << i << ' ';
        }
        std::cout << '\n';
    }

    return out;
}
