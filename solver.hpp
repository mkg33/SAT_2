#ifndef __SOLVER_H__
#define __SOLVER_H__

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

    std::vector<std::set<int> > clauses;

    // Decision trail: Each pair denotes the literal that is set to true and
    // whether it is a decision literal or not.
    std::vector<std::pair<int, bool> > trail;

    // Sets the value of a literal.
    void setLiteral(int, bool);

    // Select the first literal that is not already in the trail.
    int selectLiteral() const;

    // Set the value of a decision literal.
    void decideLiteral();

    // Find the last decision literal and return an iterator to it.
    std::vector<std::pair<int, bool> >::iterator findLastDecision();

    // Check if the trail falsifies the clauses.
    bool checkContradiction();

    // Flip the value of the last decision literal and remove any following literals.
    void backtrack();

    // Set the literals of unit clauses to true.
    void eliminateUnitClauses();

public:

    // Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
    Solver(std::istream &);

    // Return the number of clauses.
    std::size_t getNumberClauses() const;

    // Solve the SAT problem.
    bool solve();

    // Print the SAT problem.
    friend std::ostream & operator<<(std::ostream &, const Solver &);
};

#endif
