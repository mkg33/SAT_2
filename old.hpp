#ifndef __MAPHSAT_HPP__
#define __MAPHSAT_HPP__

#include <cstddef>
#include <deque>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

class MaphSAT {

public:

enum class Heuristic {
    FIRST,
    RANDOM,
    DLIS,
    RDLIS,
    DLCS,
    RDLCS,
    JW,
    RJW,
    MOMS,
    RMOMS
};
Heuristic heuristic;

private:

    enum class State {
        UNDEF,  // The formula has not yet been found to be satisfiable or unsatisfiable.
        SAT,    // The formula is satisfiable.
        UNSAT   // The formula is unsatisfiable.
    };
    State state;

    std::size_t numberVariables;
    std::size_t numberClauses;
    std::size_t numberDecisions;

    bool conflict;

    // The formula in CNF format. Each inner vector represents a clause.
    std::vector<std::vector<int> > formula;

    // The trail represents the current partial evaluation, with each pair being
    // a literal and a boolean denoting whether it is a decision literal or not.
    std::vector<std::pair<int, bool> > trail;

    // Maps a literal to the clauses that are watching the literal.
    std::unordered_map<int, std::vector<std::size_t> > watchList;

    // Literals than can be unit propagated and the clause that forced the propagation.
    std::deque<int> unitQueue;

    // List for an initial pure literal elimination.
    std::vector<std::pair<bool, bool> > pureLiterals;

    // Selection heuristics.
    void combinedSum(std::vector<std::pair<int, int> > &, std::vector<std::pair<int, int> > &, bool, std::size_t) const;
    int selectFirst() const;
    int selectRandom() const;
    int selectDLIS(bool) const;
    int selectDLCS(bool) const;
    int selectJW(bool) const;
    int selectMOMS(bool) const;

    // Assert a literal as a decision literal or as a non-decision literal.
    void assertLiteral(int, bool);

    // Select a literal that is not yet asserted and assert it as a decision literal.
    void applyDecide();

    // Find the last decision literal and return an iterator to it.
    std::vector<std::pair<int, bool> >::iterator findLastDecision();

    // Flip the value of the last decision literal and remove any following literals from the trail.
    void applyBacktrack();

    // If there are any unit literals due to the current partial evaluation, assert
    // them as non-decision literals. Repeat until there are no more unit literals.
    void applyUnitPropagate();

    // Notify clauses that a literal has been asserted.
    void notifyWatches(int);

public:

    // Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
    MaphSAT(std::istream &, Heuristic);

    // Solve the SAT problem.
    bool solve();

    // Print the SAT problem.
    friend std::ostream & operator<<(std::ostream &, const MaphSAT &);
};

#endif
