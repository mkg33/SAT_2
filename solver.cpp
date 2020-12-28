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
    #ifdef DEBUG
    std::cout << "setLiteral(l = " << literal << ", d = " << decision << ")\n";
    #endif
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
bool Solver::checkConflict() {
    for (const auto & clause : clauses) {
        bool conflict = true;
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == -literal;
            }) == trail.end()) {
                conflict = false;
                break;
            }
        }
        if (conflict)
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

// Check whether a literal is a unit literal.
// TODO: Needs a better/faster implementation, will have to see if there is any in the paper.
//       If there is no better/faster implementation we will have to come up with something.
bool Solver::isUnit(int literal, const std::set<int> & clause) {
    // Is the literal an element of the clause?
    if (std::find(clause.begin(), clause.end(), literal) == clause.end())
        return false;

    // Does the literal already have an assignment?
    if (std::find_if(trail.begin(), trail.end(), [&](const auto & l) {
        return l.first == literal || l.first == -literal;
    }) != trail.end())
        return false;

    // Is every other literal in the clause already not satisfied?
    for (int lit : clause) {
        if (lit != literal && std::find_if(trail.begin(), trail.end(), [&](const auto & l) {
            return l.first == -lit;
        }) == trail.end())
            return false;
    }

    // It is a unit literal.
    return true;
}

// Assert unit literals.
// TODO: Needs a better/faster implementation, will have to see if there is any in the paper.
//       If there is no better/faster implementation we will have to come up with something.
void Solver::unitPropagate() {
    bool finished;
    do {
        finished = true;
        for (const auto & clause : clauses) {
            for (int literal : clause) {
                if (isUnit(literal, clause)) {
                    #ifdef DEBUG
                    std::cout << "isUnit(l = " << literal << ", c = [";
                    for (int l : clause)
                        std::cout << l << ", ";
                    std::cout << "\b\b])\n";
                    #endif
                    setLiteral(literal, false);
                    finished = false;
                }
            }
        }
    } while (!checkConflict() && !finished);
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
        unitPropagate();
        if (checkConflict()) {
            if (numberDecisions == 0)
                state = Solver::State::UNSAT;
            else {
                #ifdef DEBUG
                std::cout << "backtrack()";
                #endif
                backtrack();
            }
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
