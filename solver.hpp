#ifndef __SOLVER_H__
#define __SOLVER_H__

#include <set>
#include <vector>
#include <utility>
#include <string>

using uint = unsigned int;

class Solver {

private:

    int numberClauses;
    int numberVariables;
    std::vector<std::set<int> > clauses; //the inner sets are single clauses
    std::vector<std::pair<int, int> > assignments; //first element is a variable number, second element is the assignment

public:

    /**
     * Solver(str):
     * Initializes clauses from input file.
     * Throws std::invalid_argument() exception if unsuccessful.
     */
    Solver(const std::string &dimacs);

    /**
     * getNumberClauses():
     * Returns the number of clauses.
     */
    int getNumberClauses();

    /**
     * unitClauses():
     * Identifies unit clauses
     * and assigns 'true' to the respective variable(s).
     */
    void unitClauses();

    /**
     * solve():
     * Solve the SAT problem.
     * Returns true if satisfiable, false otherwise.
     */
    bool solve();

    /**
     * operator<<(out, solver):
     * Print the output.
     * For now: print the processed DIMACS file.
     */
    friend std::ostream& operator<<(std::ostream&, const Solver&);
};

#endif
