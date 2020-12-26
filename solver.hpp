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
    std::vector<std::pair<int, bool>> assignments;

    /**
     * unitClauses():
     * Identifies unit clauses
     * and assigns 'true' to the respective variable(s).
     */
    void satisfyUnitClauses();

public:

    /**
     * Solver(str):
     * Initializes clauses from input file.
     * Throws std::invalid_argument() exception if unsuccessful.
     */
    //Solver(const std::string &dimacs);
    Solver(std::istream &);

    /**
     * getNumberClauses():
     * Returns the number of clauses.
     */
    std::vector<std::set<int> >::size_type getNumberClauses();

    /**
     * solve():
     * Solve the SAT problem. Returns true if satisfiable, false otherwise.
     */
    bool solve();

    /**
     * operator<<(out, solver):
     * Print the output.
     * For now: print the processed DIMACS file.
     */
    friend std::ostream & operator<<(std::ostream &, const Solver &);
};

#endif
