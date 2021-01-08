#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <utility>
#include <vector>

#include "solver.hpp"

// Depending on 'decision', assert a decision literal or a non-decision literal.
void Solver::assertLiteral(int literal, bool decision) {
    trail.emplace_back(literal, decision);
    #ifdef DEBUG
    std::cout << "assertLiteral(l = " << literal << ", d = " << decision << ")\n";
    #endif
}

// Declare 'clause' to be the reason that forced the propagation of 'literal'.
void Solver::setReason(int literal, const std::set<int> & clause) {
    auto it = std::find_if(reason.begin(), reason.end(), [&](const auto & lit) {
        return lit.first == literal;
    });
    if (it == reason.end())
        reason.emplace_back(literal, clause);
    else
        it->second = clause;
}

// Returns the reason 'clause' that forced the propagation of 'literal'.
std::vector<std::pair<int, std::set<int> > >::iterator Solver::getReason(int literal) {
    return std::find_if(reason.begin(), reason.end(), [&](const auto & lit) {
        return lit.first == literal;
    });
}

// Returns a literal from 'clause' that is in the trail, such that no other
// literal from 'clause' comes after it in the trail.
int Solver::lastAssertedLiteral(const std::set<int> & clause) {
    auto last = std::find_first_of(trail.rbegin(), trail.rend(), clause.begin(), clause.end(),
        [](const auto & p, const auto & lit) {
            return p.first == lit;
        });
    if (last == trail.rend())
        return 0;
    else
        return last->first;
}

// Returns the number of decision literals in the trail that precede the first
// occurrence of 'literal', including 'literal' if it is a decision literal.
int Solver::level(int literal) {
    int decisions = 0;
    for (const auto & lit : trail) {
        if (lit.second == true)
            ++decisions;
        if (lit.first == literal)
            break;
    }
    return decisions;
}

bool Solver::isUIP() {
    const int literal = lastAssertedLiteral(negConflictClause);
    for (int lit : negConflictClause) {
        if (lit != literal && level(lit) == level(literal))
            return false;
    }
    return true;
}

void Solver::applyExplain(int literal) {
    const auto it = getReason(literal);
    if (it == reason.end()) {
        return;
    }

    posConflictClause.erase(-literal);
    negConflictClause.erase(literal);

    for (int lit : it->second) {
        if (lit != literal) {
            posConflictClause.insert(lit);
            negConflictClause.insert(-lit);
        }
    }
}

void Solver::applyExplainUIP() {
    while (!isUIP())
        applyExplain(lastAssertedLiteral(negConflictClause));
}

void Solver::applyExplainEmpty() {
    while (!posConflictClause.empty())
        applyExplain(lastAssertedLiteral(negConflictClause));
}

void Solver::applyLearn() {
    clauses.push_back(posConflictClause);
}

// Returns an iterator to the first literal of the trail whose level is greater than 'level'.
std::vector<std::pair<int, bool> >::iterator Solver::firstLiteralPast(int level) {
    int decisions = 0;
    for (auto it = trail.begin(); it != trail.end(); ++it) {
        if (it->second)
            ++decisions;
        if (decisions > level)
            return it;
    }
    return trail.end();
}

// Remove any literals in the trail whose level > 'level'.
void Solver::removePast(int level) {
    auto first = firstLiteralPast(level);

    // Count decisions after 'first' (inclusive).
    int decisions = 0;
    for (auto it = first; it < trail.end(); ++it)
        decisions += it->second;

    trail.erase(first, trail.end());
    numberDecisions -= decisions;
}

int Solver::getBackjumpLevel() {
    const int literal = lastAssertedLiteral(negConflictClause);
    if ((literal != 0 && negConflictClause.size() > 1) || (literal == 0 && negConflictClause.size() > 0)) {
        int maxLvl = 0;
        for (int lit : negConflictClause) {
            if (lit != literal) {
                const int lvl = level(lit);
                if (lvl > maxLvl)
                    maxLvl = lvl;
            }
        }
        return maxLvl;
    } else
        return 0;
}

void Solver::applyBackjump() {
    const int literal = lastAssertedLiteral(negConflictClause);
    const int level = getBackjumpLevel();
    removePast(level);
    assertLiteral(-literal, false);
    setReason(-literal, posConflictClause);
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

    assertLiteral(literal, true);
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
        if (conflict) {
            posConflictClause.clear();
            negConflictClause.clear();
            for (int literal : clause) {
                posConflictClause.insert(literal);
                negConflictClause.insert(-literal);
            }
            return true;
        }
    }
    return false;
}

// Check whether a literal is a unit literal.
// TODO: Check if there is a better/faster implementation.
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
// TODO: Check if there is a better/faster implementation.
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
                    assertLiteral(literal, false);
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
            } else {
                applyExplainUIP();
                applyLearn();
                applyBackjump();
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
