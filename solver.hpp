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

    std::set<int> conflictClause;

    std::vector<std::set<int> > clauses;

    // Decision trail: Each pair denotes the satisfied literal and whether it is a decision literal.
    std::vector<std::pair<int, bool> > trail;

    // Reason.
    std::pair<int, std::set<int> > reason;

    // Set the value of a literal.
    void setLiteral(int, bool);

    // Set reason clause (literal, clause).
    void setReason(int, std::set<int>);

    // Get reason clause.
    std::set<int> getReasonClause();

    // Get reason literal.
    int getReasonLiteral();

    void applyExplainUIP();

    bool isUIP();

    void applyExplainEmpty();

    void applyExplain(int);

    void applyLearn();

    void applyBackjump();

    int getBackjumpLevel();

    void prefixToLevel(int);

    std::set<int> negatedClause(std::set<int>);

    int findLevel(int);

    // Select the first literal that is not already in the trail.
    // TODO: Needs a better/faster implementation, see reference in the paper.
    int selectLiteral() const;

    // Select the next decision literal.
    void decideLiteral();

    // Find the last decision literal and return an iterator to it.
    std::vector<std::pair<int, bool> >::iterator findLastDecision();

    int findLastAssertedLiteral(std::set<int>);

    // Check if the trail satisfies the negated formula.
    // TODO: Needs a better/faster implementation, see chapter 4.8 of the paper.
    bool checkConflict();

    // Flip the value of the last decision literal and remove any following literals from the trail.
    void backtrack();

    // Check whether a literal is a unit literal.
    // TODO: Needs a better/faster implementation, will have to see if there is any in the paper.
    //       If there is no better/faster implementation we will have to come up with something.
    bool isUnit(int, const std::set<int> &);

    // Assert unit literals.
    // TODO: Needs a better/faster implementation, will have to see if there is any in the paper.
    //       If there is no better/faster implementation we will have to come up with something.
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
