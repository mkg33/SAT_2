Instructions:

Use the 'makefile' to compile the programme.
We also include a script that we have used to test our solver.
To use the runTests.py script, please change the output of the solver (as described in the comments in maph.cpp).

Usage instructions:

Usage: ./maph.out <DIMACS file> selection heuristic:
 <FIRST=0 | RANDOM=1 | DLIS=2 | RDLIS=3 | DLCS=4 | RDLCS=5 | JW=6 | RJW=7 | MOMS=8 | RMOMS=9>

Available selection heuristics:
- FIRST: select the first available literal
- RANDOM: select a random literal
- DLIS: Dynamic Largest Individual Sum
- RDLIS: randomized Dynamic Largest Individual Sum
- DLCS: Dynamic Largest Combined Sum
- RDLCS: randomized Dynamic Largest Combined Sum
- JW: Jeroslow-Wang heuristic
- RJW: randomized Jeroslow-Wang heuristic
- MOMS: Maximum [number of] Occurrences in Minimum [length] Clauses
- RMOMS: randomized Maximum [number of] Occurrences in Minimum [length] Clauses

Note: Although we have implemented pure literal elimination, we quickly found that
it slowed out solver down. So we haven't measured the CPU time with pure literal elimination.
