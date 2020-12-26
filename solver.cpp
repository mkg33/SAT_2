#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
#include <utility>
#include <vector>

#include "solver.hpp"

// Set the value of a literal.
void Solver::setLiteral(int literal, bool assignment, bool decision) {
    assignments[literal - 1] = assignment;
    trail.emplace_back(literal, decision);
}

// Set the literals of unit clauses to true.
void Solver::eliminateUnitClauses() {
    for (const auto & clause : clauses) {
        if (clause.size() == 1)
            setLiteral(*clause.begin(), true, false);
    }
}

// Select a literal. For now we choose the first literal that is not already
// in the trail.
int Solver::selectLiteral() const {
    for (const auto & clause : clauses) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == literal;
            }) == trail.end())
                return literal;
        }
    }
    return 0;
}

// Find the last decision literal and return an iterator to that literal.
std::vector<std::pair<int, bool> >::iterator Solver::findLastDecision() {
    std::vector<std::pair<int, bool> >::reverse_iterator it;
    for (it = trail.rbegin(); it != trail.rend(); ++it) {
        if (it->second == true)
            return --it.base();
    }
    return trail.end();
}

// Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
Solver::Solver(std::istream & stream) : state(Solver::State::UNDEF) {
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
    // Reserve space for the assignments.
    assignments.assign(numberVariables, false);

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

// Return the number of clauses.
std::vector<std::set<int> >::size_type Solver::getNumberClauses() const {
    return numberClauses;
}

// Solve the SAT problem.
bool Solver::solve() {
    eliminateUnitClauses();
    return state == State::SAT ? true : false;
}

// Print the SAT problem.
std::ostream & operator<<(std::ostream & out, const Solver & solver) {
    out << "p cnf " << solver.numberVariables << ' ' << solver.numberClauses << '\n';
    for (const auto & clause : solver.clauses) {
        for (auto var : clause)
            out << var << ' ';
        out << '\n';
    }

    return out;
}
