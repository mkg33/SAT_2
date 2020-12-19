#include <vector>
#include <set>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>
#include "solver.hpp"

using uint = unsigned int;

/**
 * Initializes clauses from input file.
 * Throws std::invalid_argument() exception if unsuccessful.
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
        if(!(buf >> c))
            throw std::invalid_argument("Cannot read string.");
    }
    if (!(buf >> this->numberVariables))
        throw std::invalid_argument("Cannot read the number of variables.");
    if (!(buf >> this->numberClauses))
        throw std::invalid_argument("Cannot read the number of clauses.");
    if (this->numberVariables < 0)
        throw std::invalid_argument("The number of variables cannot be negative.");
    if (this->numberClauses < 0)
        throw std::invalid_argument("The number of clauses cannot be negative.");

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
            else if (n > this->numberVariables)
            {
                throw std::invalid_argument("Invalid number of variables.");
            }
            else
            {
                variables.insert(n);
            }
        }
    }
    if ((int)clauses.size() != this->numberClauses)
        throw std::invalid_argument("Cannot read clauses.");
}

/**
 * getNumberClauses()
 * Returns the number of clauses.
 */
int
Solver::getNumberClauses()
{
    return this->numberClauses;
}

/**
 * operator<<(out, solver)
 * Print the processed DIMACS file.
 */
std::ostream& operator<<(std::ostream& out, const Solver& solver)
{
    out << "\nnumber of clauses: " << solver.numberClauses << std::endl;
    out << "number of variables: " << solver.numberVariables << std::endl;

    for (std::set<int> const &variables : solver.clauses)
    {
        for(const int i : variables)
        {
            std::cout << i << ' ';
        }
        std::cout << '\n';
    }
    return out;
}
