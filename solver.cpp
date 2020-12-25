#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
#include <utility>
#include <vector>

#include "solver.hpp"

/**
 * Solver(stream):
 * Read DIMACS CNF. Throws std::invalid_argument() if unsuccessful.
 */
Solver::Solver(std::istream & stream)
{
    // Skip comments and 'p cnf' appearing at the top of the file.
    char c;
    while (stream >> c) {
        if (c == 'c')
            stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        else if (c == 'p') {
            stream.ignore(4, '\0');
            break;
        }
    }

    stream >> std::skipws;

    // Read number of variables and number of clauses.
    if (!(stream >> numberVariables))
        throw std::invalid_argument("Error reading DIMACS.");
    if (!(stream >> numberClauses))
        throw std::invalid_argument("Error reading DIMACS.");

    // Reserve space for the clauses.
    clauses.reserve(numberClauses);

    // Read clauses.
    int var;
    std::set<int> clause;
    std::vector<std::set<int> >::size_type i;

    for (i = 0; i < numberClauses; ++i) {
        while (stream >> var) {
            if (stream.fail())
                throw std::invalid_argument("Error reading DIMACS.");
            if (var == 0) {
                clauses.push_back(clause);
                clause.clear();
                break;
            } else
                clause.insert(var);
        }
    }
}

/**
 * getNumberClauses():
 * Returns the number of clauses.
 */
std::vector<std::set<int> >::size_type
Solver::getNumberClauses()
{
    return numberClauses;
}

/**
 * satisfyUnitClauses():
 * Set variable of unit clause to true.
 */
void
Solver::satisfyUnitClauses()
{
    for (const auto & clause : clauses) {
        if (clause.size() == 1)
            assignments.emplace_back(*clause.begin(), true);
    }
}

/**
 * solve():
 * Solve the SAT problem. Returns true if satisfiable, false otherwise.
 */
bool
Solver::solve()
{
    satisfyUnitClauses();
    std::sort(this->assignments.begin(), this->assignments.end(), [](const auto & p1, const auto & p2) {
        return p1.first < p2.first;
    });

    return false; // Temporary return
}

/**
 * operator<<(out, solver):
 * Print the output.
 * For now: print the processed DIMACS file.
 */
std::ostream & operator<<(std::ostream & out, const Solver & solver)
{
    out << "p cnf " << solver.numberVariables << ' ' << solver.numberClauses << '\n';
    for (const auto & clause : solver.clauses) {
        for (auto var : clause)
            out << var << ' ';
        out << '\n';
    }

    out << "Result: " << '\n';     // SAT or UNSAT
    out << "Assignment: " << '\n'; // Variable assignment

    for (const auto & assignment : solver.assignments)
        out << assignment.first << ": " << assignment.second << '\n';

    return out;
}
