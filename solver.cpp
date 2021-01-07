#include <algorithm>
#include <cstddef>
#include <cstdlib>
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

// Set reason clause.
// Update: Things like vectors, sets (big objects) should be passed as const reference or
//         just as reference if we modify it. This avoids copying the whole container each
//         time the function is called.
void Solver::setReason(int literal, const std::set<int> & clause) {
    reason.first = literal;
    reason.second = clause;
    // Why do we insert all literals into the clause again?
    // 'reason.second = clause' should already copy the whole clause.
    for (auto const & literal : clause)
        reason.second.insert(literal);
}

// We don't really need these two functions because reason is accessible by
// all other functions anyways. We can just use reason.first instead of
// getReasonLiteral() when we need it and it is just as short. An exception would
// be if these two functions will do more in the future...
// Get reason literal.
int Solver::getReasonLiteral() {
    return reason.first;
}
// Get reason clause.
std::set<int> Solver::getReasonClause() {
    return reason.second;
}

void Solver::applyExplainUIP() {
    while (!isUIP())
        applyExplain(findLastAssertedLiteral(negatedClause(conflictClause)));
}

// Update: We don't modify 'literal' so we can use const.
bool Solver::isUIP() {
    const int literal = findLastAssertedLiteral(negatedClause(conflictClause));
    for (int lit : negatedClause(conflictClause)) {
        if (lit != literal && (findLevel(lit) == findLevel(literal)))
            return false;
    }
    return true;
}

void Solver::applyExplainEmpty() {
    std::set<int> negatedConflictClause = negatedClause(conflictClause);
    while (!conflictClause.empty()) {
        applyExplain(findLastAssertedLiteral(negatedConflictClause));
        negatedConflictClause = negatedClause(conflictClause);
    }
}

void Solver::applyExplain(int literal) {
    std::cout << "trail: \n";
    for (auto const & lit : trail)
        std::cout << lit.first << " ";
    std::cout << "\n";

    std::set<int> reasonClause = getReasonClause();
    std::set<int>::iterator it;

    it = conflictClause.find(-literal);
    if (it != conflictClause.end()) {
        conflictClause.erase(it);
    }

    it = reasonClause.find(literal);
    if (it != reasonClause.end()) {
        reasonClause.erase(it);
    }
    std::set<int> unionClause;
    std::set_union(conflictClause.begin(), conflictClause.end(),
                   reasonClause.begin(), reasonClause.end(),
                   std::inserter(unionClause, unionClause.begin()));

    conflictClause = unionClause;
    for (auto const & lit : unionClause)
        conflictClause.insert(lit);
}

void Solver::applyLearn() {
    clauses.push_back(conflictClause);
}

// Update: Here we can use const again.
void Solver::applyBackjump() {
    std::set<int> negatedConflictClause = negatedClause(conflictClause);
    const int literal = findLastAssertedLiteral(negatedConflictClause);
    const int level = getBackjumpLevel();
    prefixToLevel(level);
    setLiteral(-literal, false);
    setReason(-literal, conflictClause);
    --numberDecisions;
}

// Update: Here we can use const again.
int Solver::getBackjumpLevel() {
    std::set<int> negatedConflictClause = negatedClause(conflictClause);
    const int literal = findLastAssertedLiteral(negatedConflictClause);
    std::set<int>::iterator it;
    it = negatedConflictClause.find(literal);
    if (it != negatedConflictClause.end())
        negatedConflictClause.erase(it);
    if (!negatedConflictClause.empty()) {
        int maxLevel = findLevel(findLastAssertedLiteral(negatedConflictClause));
        return maxLevel;
    }
    return 0;
}

void Solver::prefixToLevel(int level) {
    auto it = trail.begin();
    for (auto & lit : trail) {
        if (findLevel(lit.first) > level && it != trail.end())
            trail.erase(it);
        ++it;
    }
}

// Update: Pass by const reference again.
std::set<int> Solver::negatedClause(const std::set<int> & clause) {
    std::set<int> negatedClause;
    for (int lit : clause)
        negatedClause.insert(-lit);
    return negatedClause;
}

int Solver::findLevel(int literal) {
    int decisions = 0; // default
    for (auto & lit : trail) {
        if (lit.first == literal) {
            if (lit.second == true)
                ++decisions;
            break;
        } else if (lit.second == true)
            ++decisions;
    }
    return decisions;
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

int Solver::findLastAssertedLiteral(std::set<int> clause) {
    int assertedLiteral = 0; // default fail scenario
    int lastIndex = 0;

    for (const auto & literal : clause) {
        auto index = std::distance(trail.begin(), std::find_if(trail.begin(),
                                   trail.end(), [&](const auto & lit) {
                                       return (lit.first == literal);
                                   }));
        if (index >= lastIndex) {
            lastIndex = index;
            assertedLiteral = literal;
        }
    }
    std::cout << "last literal " << assertedLiteral << "\n";
    return assertedLiteral;
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
        if (conflict) {
            conflictClause = clause;
            for (int literal : clause) {
                conflictClause.insert(literal);
            }
            return true;
        }
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
                    setReason(literal, clause);
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
            if (numberDecisions == 0) {
                applyExplainEmpty();
                applyLearn();
                state = Solver::State::UNSAT;
            }
            else {
                #ifdef DEBUG
                std::cout << "backtrack()";
                #endif
                applyExplainUIP();
                applyLearn();
                applyBackjump();
                //backtrack();
            }
        } else {
            if (trail.size() == numberVariables)
                state = Solver::State::SAT;
            else
                decideLiteral();
        }
    }

    if (state == State::SAT) {
        std::sort(trail.begin(), trail.end(), [](const auto & l1, const auto & l2) {
            const int abs1 = std::abs(l1.first);
            const int abs2 = std::abs(l2.first);
            return abs1 < abs2;
        });
        return true;
    }
    return false;
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
        for (const auto & literal : solver.trail)
            out << literal.first << ' ';
    }
    #endif

    return out;
}
