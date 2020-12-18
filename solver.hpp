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

public:

    /**
     * Initializes clauses from input file.
     */
    Solver(const std::string &dimacs);

    /**
     * operator<<(out, solver)
     * Print the processed DIMACS file.
     */
    friend std::ostream& operator<<(std::ostream&, const Solver&);

};

#endif
