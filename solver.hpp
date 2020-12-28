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

    // Decision trail: Each pair denotes the satisfied literal and whether it is a decision literal.
    std::vector<std::pair<int, bool> > trail;

    // Set the value of a literal.
    void setLiteral(int, bool);

    // Select the first literal that is not already in the trail.
    // TODO: Needs a better/faster implementation, see reference in the paper.
    int selectLiteral() const;

    // Select the next decision literal.
    void decideLiteral();

    // Find the last decision literal and return an iterator to it.
    std::vector<std::pair<int, bool> >::iterator findLastDecision();

    // Check if the trail satisfies the negated formula.
    // TODO: Needs a better/faster implementation, see chapter 4.8 of the paper.
    bool checkContradiction();

    // Flip the value of the last decision literal and remove any following literals from the trail.
    void backtrack();

public:

    // Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
    Solver(std::istream &);

    // Solve the SAT problem.
    bool solve();

    // Print the SAT problem.
    friend std::ostream & operator<<(std::ostream &, const Solver &);
};

#endif
