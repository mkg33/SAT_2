#include <algorithm>
#include <cstddef>
#include <iostream>
#include <limits>
#include <set>
#include <utility>
#include <vector>

#include "solver.hpp"

// Set the value of a literal.
void Solver::setLiteral(int literal, bool decision) {
    trail.emplace_back(literal, decision);
}

// Select the first literal that is not already in the trail.
int Solver::selectLiteral() const {
    for (const auto & clause : clauses) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == literal || lit.first == -literal;
            }) == trail.end())
                return literal;
        }
    }
    return 0;
}

// Set the value of a decision literal.
void Solver::decideLiteral() {
    const int literal = selectLiteral();
    if (literal == 0)
        return;
    setLiteral(literal, true);
}

// Find the last decision literal and return an iterator to it.
std::vector<std::pair<int, bool> >::iterator Solver::findLastDecision() {
    std::vector<std::pair<int, bool> >::reverse_iterator it;
    for (it = trail.rbegin(); it != trail.rend(); ++it) {
        if (it->second)
            return --it.base();
    }
    return trail.end();
}

// Flip the value of the last decision literal and remove any following literals.
void Solver::backtrack() {
    const auto it = findLastDecision();
    if (it == trail.end())
        return;

    const int literal = it->first;

    // Remove the decision literal and all following literals.
    trail.erase(it, trail.end());
    // Add the flipped literal as a non-decision literal back to the trail.
    setLiteral(-literal, false);
}

// Set the literals of unit clauses to true.
void Solver::eliminateUnitClauses() {
    for (const auto & clause : clauses) {
        if (clause.size() == 1)
            setLiteral(*clause.begin(), false);
    }
}

// Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
Solver::Solver(std::istream & stream) {
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
    int numberVariables;
    std::size_t numberClauses;
    if (!(stream >> numberVariables))
        throw std::invalid_argument("Error reading DIMACS.");
    if (!(stream >> numberClauses))
        throw std::invalid_argument("Error reading DIMACS.");

    // Reserve space for the clauses.
    clauses.reserve(numberClauses);

    // Read clauses.
    int var;
    std::set<int> clause;
    for (std::size_t i = 0; i < numberClauses; ++i) {
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
std::size_t Solver::getNumberClauses() const {
    return clauses.size();
}

// Solve the SAT problem.
bool Solver::solve() {
    state = Solver::State::UNDEF;
    //eliminateUnitClauses();
    return state == State::SAT ? true : false;
}

// Print the SAT problem.
std::ostream & operator<<(std::ostream & out, const Solver & solver) {
    switch (solver.state) {
    case Solver::State::UNDEF:
        out << "UNDEF\n";
        break;
    case Solver::State::SAT:
        out << "SAT\n";
        break;
    case Solver::State::UNSAT:
        out << "UNSAT\n";
        break;
    }
    return out;
}
