#ifndef __SOLVER_H__
#define __SOLVER_H__

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

    // Number of clauses and variables.
    std::vector<std::set<int> >::size_type numberClauses;
    std::set<int>::size_type numberVariables;

    // Clauses and assignments.
    std::vector<std::set<int> > clauses;
    std::vector<bool> assignments;

    // Decision trail: Each pair denotes the literal that is set to true and
    // whether it is a decision literal or not.
    std::vector<std::pair<int, bool> > trail;

    // Sets the value of a literal.
    void setLiteral(int, bool, bool);

    // Select a literal. For now we choose the first literal that is not already
    // in the trail.
    int selectLiteral() const;

    // Find the last decision literal.
    std::vector<std::pair<int, bool> >::iterator findLastDecision();

    // Set the literals of unit clauses to true.
    void eliminateUnitClauses();

public:

    // Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
    Solver(std::istream &);

    // Return the number of clauses.
    std::vector<std::set<int> >::size_type getNumberClauses() const;

    // Solve the SAT problem.
    bool solve();

    // Print the SAT problem.
    friend std::ostream & operator<<(std::ostream &, const Solver &);
};

#endif
