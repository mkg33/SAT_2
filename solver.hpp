// TODO: See TODOs
// TODO: Declare functions const that do not change members.

#ifndef __SOLVER_HPP__
#define __SOLVER_HPP__

#include <cstddef>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

class Solver {

private:

    enum class State {
        UNDEF,
        SAT,
        UNSAT
    };

    State state;

    std::size_t numberVariables;
    std::size_t numberClauses;
    std::size_t numberDecisions;

    std::vector<std::set<int> > clauses;

    // A decision trail whose pairs denote an asserted literal and whether it is a decision literal.
    std::vector<std::pair<int, bool> > trail;

    // The variable assignment that lead to a conflict. 'negConflictClause' is the negated
    // version of 'posConflictClause'.
    std::set<int> posConflictClause;
    std::set<int> negConflictClause;

    // Propagated literals and the clause that forced their propagation.
    // TODO: Maybe a std::unordered_map would be faster? Need to profile.
    std::vector<std::pair<int, std::set<int> > > reason;

    // Depending on 'decision', assert a decision literal or a non-decision literal.
    void assertLiteral(int, bool);

    // Declare 'clause' to be the reason that forced the propagation of 'literal'.
    void setReason(int, const std::set<int> &);

    // Returns the reason 'clause' that forced the propagation of 'literal'.
    std::vector<std::pair<int, std::set<int> > >::iterator getReason(int);

    // Returns a literal from 'clause' that is in the trail, such that no other
    // literal from 'clause' comes after it in the trail.
    int lastAssertedLiteral(const std::set<int> &);

    // Returns the number of decision literals in the trail that precede the first
    // occurrence of 'literal', including 'literal' if it is a decision literal.
    int level(int);

    // TODO: The following five functions need good comments.
    bool isUIP();
    void applyExplain(int);
    void applyExplainUIP();
    void applyExplainEmpty();
    void applyLearn();

    // Returns an iterator to the first literal of the trail whose level is greater than 'level'.
    std::vector<std::pair<int, bool> >::iterator firstLiteralPast(int);

    // Remove any literals in the trail whose level > 'level'.
    void removePast(int);

    // TODO: The following two functions need good comments.
    int getBackjumpLevel();
    void applyBackjump();

    // Select the first literal that is not already in the trail.
    // TODO: Needs a better/faster implementation, see reference in the paper.
    int selectLiteral() const;

    // Select the next decision literal.
    void decideLiteral();

    // Find the last decision literal and return an iterator to it.
    std::vector<std::pair<int, bool> >::iterator findLastDecision();

    // Check if the trail satisfies the negated formula.
    // TODO: Needs a better/faster implementation, see chapter 4.8 of the paper.
    bool checkConflict();

    // Check whether a literal is a unit literal.
   // TODO: Check if there is a better/faster implementation.
    bool isUnit(int, const std::set<int> &);

    // Assert unit literals.
    // TODO: Check if there is a better/faster implementation.
    void unitPropagate();

public:

    // Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
    Solver(std::istream &);

    // Solve the SAT problem.
    bool solve();

    // Print the SAT problem.
    friend std::ostream & operator<<(std::ostream &, const Solver &);
};

#endif
