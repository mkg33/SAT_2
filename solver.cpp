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
// TODO: Needs better implementation, see reference in paper.
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

// Select the next decision literal.
void Solver::decideLiteral() {
    const int literal = selectLiteral();
    if (literal == 0)
        return;
    setLiteral(literal, true);
    ++numberDecisions;
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

// Check if the trail satisfies the negated formula.
// TODO: Needs a better/faster implementation, see chapter 4.8 of the paper.
bool Solver::checkContradiction() {
    for (const auto & clause : clauses) {
        bool contradiction = true;
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == -literal;
            }) == trail.end()) {
                contradiction = false;
                break;
            }
        }
        if (contradiction)
            return true;
    }
    return false;
}

// Flip the value of the last decision literal and remove any following literals from the trail.
void Solver::backtrack() {
    const auto it = findLastDecision();
    if (it == trail.end())
        return;

    const int literal = it->first;

    // Remove the decision literal and all following literals.
    trail.erase(it, trail.end());
    // Add the flipped literal as a non-decision literal back to the trail.
    setLiteral(-literal, false);
    --numberDecisions;
}

// Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
Solver::Solver(std::istream & stream) : state(Solver::State::UNDEF), numberVariables(0),
    numberClauses(0), numberDecisions(0) {
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

    // Reserve space for the clauses and the trail.
    clauses.reserve(numberClauses);
    trail.reserve(numberVariables);

    // Read clauses.
    int var;
    std::set<int> clause;
    for (std::size_t i = 0; i < numberClauses; ++i) {
        while (stream >> var) {
            if (var == 0) {
                clauses.push_back(clause);
                clause.clear();
                break;
            } else
                clause.insert(var);
        }
        if (stream.fail())
            throw std::invalid_argument("Error reading DIMACS.");
    }
}

// Solve the SAT problem.
bool Solver::solve() {
    while (state == Solver::State::UNDEF) {
        if (checkContradiction()) {
            if (numberDecisions == 0)
                state = Solver::State::UNSAT;
            else
                backtrack();
        } else {
            if (trail.size() == numberVariables)
                state = Solver::State::SAT;
            else
                decideLiteral();
        }
    }

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

    #ifdef DEBUG
    if (solver.state == Solver::State::SAT) {
        out << "Satisfying assignment:\n";
        for (const auto & literal : solver.trail)
            out << literal.first << ' ';
    }
    #endif

    return out;
}
