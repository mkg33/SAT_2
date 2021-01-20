#include <algorithm>
#include <cstdlib>
#include <limits>

#include "old.hpp"
#include "selectionHeuristics.hpp"

// Assert a literal as a decision literal or as a non-decision literal.
void MaphSAT::assertLiteral(int literal, bool decision) {
    trail.emplace_back(literal, decision);
    notifyWatches(-literal);
}

// Select a literal that is not yet asserted and assert it as a decision literal.
void MaphSAT::applyDecide() {
    int literal = 0;

    switch (heuristic) {
    case MaphSAT::Heuristic::FIRST:
        literal = selectFirst();
        break;
    case MaphSAT::Heuristic::RANDOM:
        literal = selectRandom();
        break;
    case MaphSAT::Heuristic::DLIS:
        literal = selectDLIS(false);
        break;
    case MaphSAT::Heuristic::RDLIS:
        literal = selectDLIS(true);
        break;
    case MaphSAT::Heuristic::DLCS:
        literal = selectDLCS(false);
        break;
    case MaphSAT::Heuristic::RDLCS:
        literal = selectDLCS(true);
        break;
    case MaphSAT::Heuristic::JW:
        literal = selectJW(false);
        break;
    case MaphSAT::Heuristic::RJW:
        literal = selectJW(true);
        break;
    case MaphSAT::Heuristic::MOMS:
        literal = selectMOMS(false);
        break;
    case MaphSAT::Heuristic::RMOMS:
        literal = selectMOMS(true);
        break;
    }

    if (literal == 0)
        return;

    assertLiteral(literal, true);
    ++numberDecisions;
}

// Find the last decision literal and return an iterator to it.
std::vector<std::pair<int, bool> >::iterator MaphSAT::findLastDecision() {
    for (auto it = trail.rbegin(); it != trail.rend(); ++it) {
        if (it->second)
            return --it.base();
    }
    return trail.end();
}

// Flip the value of the last decision literal and remove any following literals from the trail.
void MaphSAT::applyBacktrack() {
    const auto it = findLastDecision();
    if (it == trail.end())
        return;

    const int literal = it->first;

    // Remove the decision literal and all following literals.
    trail.erase(it, trail.end());
    unitQueue.clear();
    unitQueue.push_front(-literal);
    --numberDecisions;
    conflict = false;
}

// If there are any unit literals due to the current partial evaluation, assert
// them as non-decision literals. Repeat until there are no more unit literals.
void MaphSAT::applyUnitPropagate() {
    while (!unitQueue.empty() && !conflict) {
        const auto literal = unitQueue.back();
        unitQueue.pop_back();
        assertLiteral(literal, false);
    }
}

// Notify clauses that a literal has been asserted.
void MaphSAT::notifyWatches(int literal) {
    // Are there any clauses that watch 'literal'?
    if (watchList.find(literal) == watchList.end())
        return;

    std::vector<std::size_t> newWL;
    newWL.reserve(watchList[literal].size());

    const auto list = watchList[literal];

    for (std::size_t clauseIndex : list) {

        auto & clause = formula[clauseIndex];
        // Swap the watched literals if the first watched literal was falsified.
        if (clause[0] == literal)
            std::swap(clause[0], clause[1]);

        // Is the clause already satisfied? Only check the first watched literal.
        if (std::find_if(trail.begin(), trail.end(), [&clause](const auto & p) { return p.first == clause[0]; }) != trail.end()) {
            newWL.push_back(clauseIndex);
            continue;
        }

        // Are there any other unfalsified literals in the clause?
        std::vector<int>::iterator other = clause.end();
        for (auto it = clause.begin() + 2; it != clause.end(); ++it) {
            if (std::find_if(trail.begin(), trail.end(), [it](const auto & p) { return p.first == -*it; }) == trail.end()) {
                other = it;
                break;
            }
        }
        // If there is, swap the unfalsified literal with the second watched literal.
        if (other != clause.end()) {
            std::iter_swap(clause.begin() + 1, other);
            watchList[clause[1]].push_back(clauseIndex);
            continue;
        }

        // If there is no other unfalsified literal and the first watched literal is
        // also falsified, then there is a conflict.
        // If the first watched literal is not falsified, it is a unit literal.
        newWL.push_back(clauseIndex);
        if (std::find_if(trail.begin(), trail.end(), [&clause](const auto & p) { return p.first == -clause[0]; }) != trail.end())
            conflict = true;
        else if (std::find(unitQueue.begin(), unitQueue.end(), clause[0]) == unitQueue.end())
            unitQueue.push_front(clause[0]);
    }

    watchList[literal] = newWL;
}

// Parse a CNF formula and throw invalid_argument() if unsuccessful.
MaphSAT::MaphSAT(std::istream & stream, MaphSAT::Heuristic heuristic):
    heuristic(heuristic), state(MaphSAT::State::UNDEF), numberVariables(0),
    numberClauses(0), numberDecisions(0), conflict(false) {
    // Skip optional comments and the mandatory 'p cnf' appearing at the top of the CNF formula.
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
    // Parse the number of variables and the number of clauses.
    if (!(stream >> numberVariables))
        throw std::invalid_argument("Error reading DIMACS.");
    if (!(stream >> numberClauses))
        throw std::invalid_argument("Error reading DIMACS.");

    // Reserve space for the clauses and the trail.
    formula.reserve(numberClauses);
    trail.reserve(numberVariables);

    // Parse all clauses.
    int literal;
    std::vector<int> clause;
    for (std::size_t i = 0; i < numberClauses; ++i) {
        while (stream >> literal) {
            if (literal == 0 && clause.size() > 1) {
                // Add the clause to the formula and watch lists.
                formula.push_back(clause);
                watchList[clause[0]].push_back(formula.size() - 1);
                watchList[clause[1]].push_back(formula.size() - 1);
                clause.clear();
                break;
            } else if (literal == 0 && clause.size() == 1) {
                // If the clause is just one literal add the literal to the unit queue.
                unitQueue.push_front(clause[0]);
                clause.clear();
                break;
            } else if (literal != 0 && std::find(clause.begin(), clause.end(), literal) == clause.end()) {
                clause.push_back(literal);
            }
        }
        if (stream.fail())
            throw std::invalid_argument("Error parsing DIMACS.");
    }
}

// Solve the SAT problem.
bool MaphSAT::solve() {
    // Check if there are any pure literals.
    pureLiterals.reserve(numberVariables);
    for (const auto & clause : formula) {
        for (int literal : clause) {
            if (literal > 0)
                pureLiterals[literal].first = true;
            else
                pureLiterals[-literal].second = true;
        }
    }
    // Add the pure literals to the unit queue.
    for (std::size_t i = 0; i < pureLiterals.size(); ++i) {
        const auto p = pureLiterals[i];
        if (p.first && !p.second)
            unitQueue.push_front(i + 1);
        else if (!p.first && p.second)
            unitQueue.push_front(- i - 1);
    }

    // Are there any conflicts with the unit literals?
    for (std::size_t i = 0; i < unitQueue.size(); ++i) {
        const int unitLiteral = unitQueue[i];
        if (std::any_of(unitQueue.begin() + i, unitQueue.end(), [unitLiteral](int literal) { return literal == -unitLiteral; })) {
            state = MaphSAT::State::UNSAT;
            return false;
        }
    }

    // Until the formula is satisfiable or unsatisfiable, the state of the solver is undefined.
    while (state == MaphSAT::State::UNDEF) {

        /*std::cout << "trail: ";
        for (const auto & p : trail)
            std::cout << p.first << ":" << p.second << " ";
        std::cout << std::endl;

        std::cout << "unit queue: ";
        for (int literal : unitQueue)
            std::cout << literal << " ";
        std::cout << std::endl;*/

        // Assert any unit literals.
        applyUnitPropagate();
        if (conflict) {
            // Can we backtrack to resolve the conflict?
            if (numberDecisions == 0)
                state = MaphSAT::State::UNSAT;
            else
                applyBacktrack();
        } else {
            // If every variable has an assignment we are done.
            if (trail.size() == numberVariables)
                state = MaphSAT::State::SAT;
            else
                applyDecide();
        }
    }

    // If the formula is satisfiable, the trail represents the satisfying assignment.
    // Sort the trail before it gets printed.
    if (state == MaphSAT::State::SAT) {
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
std::ostream & operator<<(std::ostream & out, const MaphSAT & solver) {
    switch (solver.state) {
    case MaphSAT::State::UNDEF:
        out << "UNDEF\n";
        break;
    case MaphSAT::State::SAT:
        out << "SAT\n";
        break;
    case MaphSAT::State::UNSAT:
        out << "UNSAT\n";
        break;
    }

    #ifdef DEBUG
    if (solver.state == MaphSAT::State::SAT) {
        for (const auto & literal : solver.trail)
            out << literal.first << ' ';
    }
    #endif

    return out;
}
