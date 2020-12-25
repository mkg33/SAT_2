# SAT_2

**Projekt 2**

Start: 10.12.2020

Deadline: 21.01.2021

*Update 25.12.2020*:
In the previous project we used large /* */ comments above class methods.
On second thought, I think it might be overkill. So I suggest we just use
a single line (or two lines for complicated methods) of normal // comments
to describe the method.

**TODO**:

- open DIMACS file :white_check_mark:
- read DIMACS input :white_check_mark:
- store clauses as `<vector<set>>` :white_check_mark:
- create a data structure to store variables and the numbers of clauses they appear in (?) -> to remove clauses faster etc.
- count the frequency of respective variables (?)
- check input validity :white_check_mark:
- identify unit clauses :white_check_mark:
- remove all clauses containing variables from the unit clauses
- prepare a basic strategy [this part will be split into finer categories]
- identify pure literals
- eliminate pure literals
- format output :white_check_mark:
