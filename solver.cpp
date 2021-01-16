#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <utility>
#include <vector>
#include <ctime>
#include <random>
#include <boost/algorithm/string/predicate.hpp>

#include "solver.hpp"

// Depending on 'decision', assert a decision literal or a non-decision literal.
void Solver::assertLiteral(int literal, bool decision) {
    trail.emplace_back(literal, decision);
    #ifdef DEBUG
    std::cout << "assertLiteral(l = " << literal << ", d = " << decision << ")\n";
    #endif
}

// Declare 'clause' to be the reason that forced the propagation of 'literal'.
void Solver::setReason(int literal, const std::set<int> & clause) {
    auto it = std::find_if(reason.begin(), reason.end(), [&](const auto & lit) {
        return lit.first == literal;
    });
    if (it == reason.end())
        reason.emplace_back(literal, clause);
    else
        it->second = clause;
}

// Returns the reason 'clause' that forced the propagation of 'literal'.
std::vector<std::pair<int, std::set<int> > >::iterator Solver::getReason(int literal) {
    return std::find_if(reason.begin(), reason.end(), [&](const auto & lit) {
        return lit.first == literal;
    });
}

// Returns a literal from 'clause' that is in the trail, such that no other
// literal from 'clause' comes after it in the trail.
int Solver::lastAssertedLiteral(const std::set<int> & clause) {
    auto last = std::find_first_of(trail.rbegin(), trail.rend(), clause.begin(), clause.end(),
        [](const auto & p, const auto & lit) {
            return p.first == lit;
        });
    if (last == trail.rend())
        return 0;
    else
        return last->first;
}

// Returns the number of decision literals in the trail that precede the first
// occurrence of 'literal', including 'literal' if it is a decision literal.
int Solver::level(int literal) {
    int decisions = 0;
    for (const auto & lit : trail) {
        if (lit.second == true)
            ++decisions;
        if (lit.first == literal)
            break;
    }
    return decisions;
}

bool Solver::isUIP() {
    const int literal = lastAssertedLiteral(negConflictClause);
    for (int lit : negConflictClause) {
        if (lit != literal && level(lit) == level(literal))
            return false;
    }
    return true;
}

void Solver::applyExplain(int literal) {
    const auto it = getReason(literal);
    if (it == reason.end()) {
        return;
    }

    posConflictClause.erase(-literal);
    negConflictClause.erase(literal);

    for (int lit : it->second) {
        if (lit != literal) {
            posConflictClause.insert(lit);
            negConflictClause.insert(-lit);
        }
    }
}

void Solver::applyExplainUIP() {
    while (!isUIP())
        applyExplain(lastAssertedLiteral(negConflictClause));
}

void Solver::applyExplainEmpty() {
    while (!posConflictClause.empty())
        applyExplain(lastAssertedLiteral(negConflictClause));
}

void Solver::applyLearn() {
    clauses.push_back(posConflictClause);
}

// Returns an iterator to the first literal of the trail whose level is greater than 'level'.
std::vector<std::pair<int, bool> >::iterator Solver::firstLiteralPast(int level) {
    int decisions = 0;
    for (auto it = trail.begin(); it != trail.end(); ++it) {
        if (it->second)
            ++decisions;
        if (decisions > level)
            return it;
    }
    return trail.end();
}

// Remove any literals in the trail whose level > 'level'.
void Solver::removePast(int level) {
    auto first = firstLiteralPast(level);

    // Count decisions after 'first' (inclusive).
    int decisions = 0;
    for (auto it = first; it < trail.end(); ++it)
        decisions += it->second;

    trail.erase(first, trail.end());
    numberDecisions -= decisions;
}

int Solver::getBackjumpLevel() {
    const int literal = lastAssertedLiteral(negConflictClause);
    if ((literal != 0 && negConflictClause.size() > 1) || (literal == 0 && negConflictClause.size() > 0)) {
        int maxLvl = 0;
        for (int lit : negConflictClause) {
            if (lit != literal) {
                const int lvl = level(lit);
                if (lvl > maxLvl)
                    maxLvl = lvl;
            }
        }
        return maxLvl;
    } else
        return 0;
}

void Solver::applyBackjump() {
    const int literal = lastAssertedLiteral(negConflictClause);
    const int level = getBackjumpLevel();
    removePast(level);
    assertLiteral(-literal, false);
    setReason(-literal, posConflictClause);
}

// Select the first literal that is not already in the trail.
int Solver::selectLiteral() const {
    for (const auto & clause : clauses) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == literal || lit.first == -literal;
            }) == trail.end()){
                //std::cout << literal << "\n";
                return literal;
            }
        }
    }
    return 0;
}

// Select the first literal that is not already in the trail
// and use a random yes/no decision during the selection.
int Solver::selectLiteralBool() const {
    auto generateBool = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());
    for (const auto & clause : clauses) {
        for (int literal : clause) {
            bool decision = generateBool();
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == literal || lit.first == -literal;
            }) == trail.end()) {
                if(decision) {
                    return literal;
                }
            }
        }
    }
    return 0;
}

// Helper for random selection heuristics.
// Takes a lower and upper bound and returns a random index within the bounds.
// It's mainly used for vectors, therefore the upper bound is decremented.
int Solver::getRandomIndex(int lowerBound, int upperBound) {
    std::random_device randDevice;
    std::mt19937 rng(randDevice());
    std::uniform_int_distribution<int> result(lowerBound,upperBound-1);
    int randIndex = result(rng);
    return randIndex;
}

// Selection heuristic: pick random literal.
int Solver::selectLiteralRand() {
    int maxLit = 0;
    std::vector<int> randCandidates;
    for (const auto & clause : clauses) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == literal || lit.first == -literal;
            }) == trail.end()) {
                randCandidates.push_back(literal);
            }
        }
    }
    if (!randCandidates.empty()) {
        #ifdef DEBUG
        for (auto const & lit : randCandidates) {
            std::cout << "Candidate: " << lit << '\n';
        }
        #endif

        int randIndex = getRandomIndex(0, randCandidates.size());
        maxLit = randCandidates.at(randIndex);

        #ifdef DEBUG
        std::cout << "Chosen maxLit: " << maxLit << '\n';
        #endif
    }
    if (maxLit > 0) {
        return maxLit;
    }
    else {
        return -maxLit;
    }
}
// Selection heuristic: Dynamic Largest Individual Sum.
// Picks the literal with the highest number of occurrences in the unsatisfied clauses.
// Sets value to true if the literal is positive.
// If the literal is negative, sets the value of its negation to true.
// If randomized true, it runs the randomized DLIS variant.
int Solver::selectLiteralDLIS(bool randomized) {
    int counter = 0;
    int maxNumber = 0;
    int maxLit = 0;
    std::vector<int> randCandidates; // used in RDLIS

    for (const auto & clause : clauses) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == literal || lit.first == -literal;
            }) == trail.end()) {
                auto it = std::find_if(variableCount.begin(), variableCount.end(), [&](const auto & lit) {
                    counter = lit.second;
                    return lit.first == literal || lit.first == -literal;
                });

                if (it == variableCount.end()) { // if the literal is not in the variableCount
                    variableCount.emplace_back(std::make_pair(literal, 1));
                    if (1 > maxNumber) {
                        maxNumber = 1;
                        maxLit = literal;
                    }
                    else if (randomized && 1 == maxNumber) {
                        randCandidates.push_back(literal);
                    }
                }
                else {
                    variableCount.erase(it);
                    int newValue = ++counter;
                    variableCount.emplace_back(literal, newValue); // update the counter associated with the literal

                    if (newValue > maxNumber) {
                        maxNumber = newValue;
                        maxLit= literal;
                    }
                    else if (randomized && newValue == maxNumber) {
                        randCandidates.push_back(literal);
                    }
                }
            }
        }
    }

    if (randomized && !randCandidates.empty()) {
        #ifdef DEBUG
        for (auto const & lit : randCandidates) {
            std::cout << "Candidate: " << lit << '\n';
        }
        #endif

        int randIndex = getRandomIndex(0, randCandidates.size());
        maxLit = randCandidates.at(randIndex);

        #ifdef DEBUG
        std::cout << "Chosen maxLit: " << maxLit << '\n';
        #endif
    }

    #ifdef DEBUG
    for (const auto & lit : variableCount) {
        std::cout << "Literal: " << lit.first;
        std::cout << " Count: " << lit.second << "\n";
    }
    std::cout << "maxLit is: " << maxLit << "\n";
    #endif

    if (maxLit > 0) {
        variableCount.clear();
        return maxLit;
        return 1;
    }
    else {
        variableCount.clear();
        return -maxLit;
    }
}

// Helper for computing the combined sum of occurrences (both polarities).
// If the first parameter is true, it computes the combined sum for the MOMS heuristic
// and uses the int to determine the 'cutoffLength' of a clause.
// If false, it computes the usual combined sum for DLCS.
void Solver::combinedSum(bool constraint, int cutoffLength) {
    int counterPos = 0;
    int counterNeg = 0;

    for (const auto & clause : clauses){
        for (int literal : clause) {
            if (constraint) {
                if (int(clause.size()) > cutoffLength) {
                    break;
                }
            }
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == literal || lit.first == -literal;
            }) == trail.end()) {
                auto itPos = std::find_if(posVariableCount.begin(), posVariableCount.end(), [&](const auto & posLit) {
                    counterPos = posLit.second;
                    return posLit.first == literal || posLit.first == -literal;
                });
                auto itNeg = std::find_if(negVariableCount.begin(), negVariableCount.end(), [&](const auto & negLit) {
                    counterNeg = negLit.second;
                    return negLit.first == literal || negLit.first == -literal;
                });

                if (literal > 0) {
                    if (itPos == posVariableCount.end()) {
                        if (literal > 0) {
                            posVariableCount.emplace_back(std::make_pair(literal, 1));
                        }
                    }
                    else {
                        posVariableCount.erase(itPos);
                        int newPosCounter = ++counterPos;
                        posVariableCount.emplace_back(literal, newPosCounter);
                    }
                }
                else if (literal < 0) {
                    if (itNeg == negVariableCount.end()) {
                        negVariableCount.emplace_back(std::make_pair(literal, 1));
                    }
                    else {
                        negVariableCount.erase(itNeg);
                        int newNegCounter = ++counterNeg;
                        negVariableCount.emplace_back(literal, newNegCounter);
                    }
                }
            }
        }
    }
    #ifdef DEBUG
    for (const auto & lit : posVariableCount) {
        std::cout << "Literal: " << lit.first;
        std::cout << " Count: " << lit.second << "\n";
    }
    for (const auto & lit : negVariableCount) {
        std::cout << "Literal: " << lit.first;
        std::cout << " Count: " << lit.second << "\n";
    }
    #endif
}

// Selection heuristic: Dynamic Largest Combined Sum.
// Picks the variable with the highest number of occurrences of its positive and negative literals (combined).
// If randomized true, it runs the randomized DLCS variant.
int Solver::selectLiteralDLCS(bool randomized) {
    int counterNeg = 0;
    int maxScore = 0;
    int maxLit = 0;
    std::vector<int> randCandidates; // used in RDLCS

    combinedSum(false, 0); // compute the combined sum

    for (auto & lit : posVariableCount) { // match corresponding literals (i.e., x and -x) in the two vectors
        auto itNeg = std::find_if(negVariableCount.begin(), negVariableCount.end(), [&](const auto & negLit) {
            counterNeg = negLit.second;
            return negLit.first == -lit.first;
        });
        if (itNeg != negVariableCount.end()) {
            if ((counterNeg + lit.second) > maxScore) { // update the combined variable score
                maxScore = counterNeg + lit.second;
                maxLit = lit.first;
                negVariableCount.erase(itNeg);
            }
            else if (randomized && (counterNeg + lit.second) == maxScore) {
                randCandidates.push_back(lit.first);
                negVariableCount.erase(itNeg);
            }
        }
    }

    if (maxLit == 0) { // If there are no matching literals, pick the first literal available.
        if (!posVariableCount.empty()) {
            maxLit = posVariableCount.front().first;
        }
        else if (!negVariableCount.empty()) {
            maxLit = negVariableCount.front().first;
            maxLit = -maxLit; // we need to negate the literal because of decideLiteral()
        }
    }
    posVariableCount.clear();
    negVariableCount.clear();

    if (randomized && !randCandidates.empty()) {
        #ifdef DEBUG
        for (auto const & lit : randCandidates) {
            std::cout << "Candidate: " << lit << '\n';
        }
        #endif

        int randIndex = getRandomIndex(0, randCandidates.size());
        maxLit = randCandidates.at(randIndex);

        #ifdef DEBUG
        std::cout << "Chosen maxLit: " << maxLit << '\n';
        #endif
    }

    #ifdef DEBUG
    std::cout << "maxLit: " << maxLit << '\n';
    #endif

    return maxLit;
}

// Selection heuristic: the Jeroslow-Wang method.
// If randomized true, it runs the randomized J-W variant.
int Solver::selectLiteralJW(bool randomized) {

    double score = 0;
    double maxScore = -std::numeric_limits<double>::max();
    int maxLit = 0;
    std::vector<int> randCandidates; // used in randomized J-W

    for (auto const & clause : clauses) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == literal || lit.first == -literal;
            }) == trail.end()) {
            auto it = std::find_if(JWcount.begin(), JWcount.end(), [&](const auto & lit) {
                score = lit.second;
                return lit.first == literal || lit.first == -literal;
            });

            double clauseSize = clause.size();

            if (it == JWcount.end()) { // if the literal is not in the JWcount
                double initialScore = pow(2.0, -clauseSize);
                JWcount.emplace_back(std::make_pair(literal, initialScore));
                if (initialScore > maxScore) { // select the literal that maximizes the score
                    maxScore = initialScore;
                    maxLit = literal;
                }
                else if (randomized && initialScore == maxScore) {
                    randCandidates.push_back(literal);
                }
            }
            else {
                JWcount.erase(it);
                score = score + pow(2.0, -clauseSize);
                JWcount.emplace_back(literal, score); // update the score associated with the literal
                if (score > maxScore) { // select the literal that maximizes the score
                    maxScore = score;
                    maxLit = literal;
                }
                else if (randomized && score == maxScore) {
                    randCandidates.push_back(literal);
                }
             }
          }
       }
   }
  if (maxLit != 0) {

      #ifdef DEBUG
      for (auto const & lit : JWcount) {
          std::cout << "Literal: " << lit.first << " Score: " << lit.second << "\n";
      }
      std::cout << "Selected literal: " << maxLit << "\n";
      #endif

      JWcount.clear();

      if (randomized && !randCandidates.empty()) {
          int randIndex = getRandomIndex(0, randCandidates.size());
          maxLit = randCandidates.at(randIndex);

          #ifdef DEBUG
          std::cout << "Chosen maxLit: " << maxLit << '\n';
          #endif
      }

      if (maxLit > 0) {
          return maxLit;
      }
      else {
          return -maxLit;
      }
  }
  return 0;
}

// Selection heuristic: Maximum [number of] Occurrences in Minimum [length] Clauses.
// If randomized true, it runs the randomized MOMS variant.
int Solver::selectLiteralMOMS(bool randomized) {
    int maxLit = 0;
    int counterNeg = 0;
    int maxScore = 0;
    int parameter = 3; // arbitrarily chosen
    int totalClauseLength = 0;
    int cutoffLength = 0; // defines the 'minimum length' of a clause
    std::vector<int> randCandidates; // used in randomized MOMS

    for (const auto & clause : clauses) {
        totalClauseLength += int(clause.size());
    }
    cutoffLength = totalClauseLength / int(clauses.size()); // an arbitrary definition of a short clause
    if (cutoffLength >= 2) {
        --cutoffLength;
    }

    #ifdef DEBUG
    std::cout << "cuffoffLength: " << cutoffLength << '\n';
    #endif

    combinedSum(true, cutoffLength); // compute the combined sum using the 'cutoffLength' parameter

    for (auto & lit : posVariableCount) { // match corresponding literals (i.e., x and -x) in the two vectors
        auto itNeg = std::find_if(negVariableCount.begin(), negVariableCount.end(), [&](const auto & negLit) {
            counterNeg = negLit.second;
            return negLit.first == -lit.first;
        });
        if (itNeg != negVariableCount.end()) {
            int tempScore = (counterNeg + lit.second) * pow(2, parameter) + (counterNeg * lit.second);
            if (tempScore > maxScore) { // update the combined variable score
                maxScore = tempScore;
                maxLit = lit.first;
                negVariableCount.erase(itNeg);
            }
            else if (randomized && tempScore == maxScore) {
                randCandidates.push_back(lit.first);
            }
        }
    }

    if (maxLit == 0) { // If there are no matching literals, pick the first literal available.
        maxLit = selectLiteral();
    }
    posVariableCount.clear();
    negVariableCount.clear();

    if (randomized && !randCandidates.empty()) {
        int randIndex = getRandomIndex(0, randCandidates.size());
        maxLit = randCandidates.at(randIndex);

        #ifdef DEBUG
        std::cout << "Chosen maxLit: " << maxLit << '\n';
        #endif
    }
    return maxLit;
}

// Select the next decision literal.
void Solver::decideLiteral() {
    int literal = 0;
    // Choose the selection heuristic based on user input.
    switch(heuristics) {
        case 1:
            literal = selectLiteral();
            break;
        case 2:
            literal = selectLiteralBool();
            break;
        case 3:
            literal = selectLiteralDLIS(false);
            break;
        case 4:
            literal = selectLiteralDLCS(false);
            break;
        case 5:
            literal = selectLiteralJW(false);
            break;
        case 6:
            literal = selectLiteralDLCS(true);
            break;
        case 7:
            literal = selectLiteralDLIS(true);
            break;
        case 8:
            literal = selectLiteralRand();
            break;
        case 9:
            literal = selectLiteralMOMS(false);
            break;
        case 10:
            literal = selectLiteralMOMS(true);
            break;
        case 11:
            literal = selectLiteralJW(true);
            break;
    }

    if (literal == 0)
        return;

    assertLiteral(literal, true);
    ++numberDecisions;
}

// Find the last decision literal and return an iterator to it.
std::vector<std::pair<int, bool> >::iterator Solver::findLastDecision() {
    std::vector<std::pair<int, bool> >::reverse_iterator it;
    for (it = trail.rbegin(); it != trail.rend(); ++it) {
        if (it->second)
            return --it.base();
    }
    return trail.end();
}

// Check if the trail satisfies the negated formula.
// TODO: Needs a better/faster implementation, see chapter 4.8 of the paper.
bool Solver::checkConflict() {
    for (const auto & clause : clauses) {
        bool conflict = true;
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [&](const auto & lit) {
                return lit.first == -literal;
            }) == trail.end()) {
                conflict = false;
                break;
            }
        }
        if (conflict) {
            posConflictClause.clear();
            negConflictClause.clear();
            for (int literal : clause) {
                posConflictClause.insert(literal);
                negConflictClause.insert(-literal);
            }
            return true;
        }
    }
    return false;
}

// Check whether a literal is a unit literal.
// TODO: Check if there is a better/faster implementation.
bool Solver::isUnit(int literal, const std::set<int> & clause) {
    // Is the literal an element of the clause?
    if (std::find(clause.begin(), clause.end(), literal) == clause.end())
        return false;

    // Does the literal already have an assignment?
    if (std::find_if(trail.begin(), trail.end(), [&](const auto & l) {
        return l.first == literal || l.first == -literal;
    }) != trail.end())
        return false;

    // Is every other literal in the clause already not satisfied?
    for (int lit : clause) {
        if (lit != literal && std::find_if(trail.begin(), trail.end(), [&](const auto & l) {
            return l.first == -lit;
        }) == trail.end())
            return false;
    }

    // It is a unit literal.
    return true;
}

// Assert unit literals.
// TODO: Check if there is a better/faster implementation.
void Solver::unitPropagate() {
    bool finished;
    do {
        finished = true;
        for (const auto & clause : clauses) {
            for (int literal : clause) {
                if (isUnit(literal, clause)) {
                    #ifdef DEBUG
                    std::cout << "isUnit(l = " << literal << ", c = [";
                    for (int l : clause)
                        std::cout << l << ", ";
                    std::cout << "\b\b])\n";
                    #endif
                    assertLiteral(literal, false);
                    setReason(literal, clause);
                    finished = false;
                }
            }
        }
    } while (!checkConflict() && !finished);
}

// Elimiate pure literals.
void Solver::pureLiteral() {
    std::vector<int> trackLiterals; // keep track of literals occurring with unique polarity
    std::vector<int> erasedLiterals; // keep track of the erased literals
    for (const auto & clause : clauses) {
        for (int literal : clause) {
            auto it = std::find_if(trackLiterals.begin(), trackLiterals.end(), [&](const auto & lit) {
                return lit == -literal;
            });
            auto duplicate = std::find_if(trackLiterals.begin(), trackLiterals.end(), [&](const auto & lit) {
                return lit == literal;
            });
            auto erased = std::find_if(erasedLiterals.begin(), erasedLiterals.end(), [&](const auto & lit) {
                return lit == literal || lit == -literal;
            });
            if (it == trackLiterals.end() && duplicate == trackLiterals.end() && erased == erasedLiterals.end()) {
                trackLiterals.push_back(literal);
            }
            else if (it != trackLiterals.end()) {
                #ifdef DEBUG
                std::cout << "\nErasing: " << literal << '\n';
                #endif
                erasedLiterals.push_back(literal);
                trackLiterals.erase(it);
            }
        }
    }
    if (!trackLiterals.empty()) {
        for (const auto & lit : trackLiterals) {
            #ifdef DEBUG
            std::cout << "Pure literal: " << lit << '\n';
            #endif
            assertLiteral(lit, true);
            ++numberDecisions;
        }
        trackLiterals.clear();
    }
}

// Read a DIMACS CNF SAT problem. Throws invalid_argument() if unsuccessful.
Solver::Solver(std::istream & stream, std::string & option) : state(Solver::State::UNDEF), numberVariables(0),
    numberClauses(0), numberDecisions(0) {
    // Skip comments and 'p cnf' appearing at the top of the file.
    char c;
    while (stream >> c) {
        if (c == 'c')
            stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        else if (c == 'p') {
            stream.ignore(4, '\0');
            break;
        }
    }

    stream >> std::skipws;

    // Read number of variables and number of clauses.
    if (!(stream >> numberVariables))
        throw std::invalid_argument("Error reading DIMACS.");
    if (!(stream >> numberClauses))
        throw std::invalid_argument("Error reading DIMACS.");

    // Reserve space for the clauses and the trail.
    clauses.reserve(numberClauses);
    trail.reserve(numberVariables);

    // Read clauses.
    int var;
    std::set<int> clause;
    for (std::size_t i = 0; i < numberClauses; ++i) {
        while (stream >> var) {
            if (var == 0) {
                clauses.push_back(clause);
                clause.clear();
                break;
            } else
                clause.insert(var);
        }
        if (stream.fail())
            throw std::invalid_argument("Error reading DIMACS.");
    }
    // Set selection heuristics.
    // Using boost to ignore case.
    std::string without = "without";
    std::string yesNo = "yesno";
    std::string random = "random";
    std::string dlis = "dlis";
    std::string rdlis = "rdlis";
    std::string dlcs = "dlcs";
    std::string rdlcs = "rdlcs";
    std::string jw = "jw";
    std::string rjw = "rjw";
    std::string moms = "moms";
    std::string rmoms = "rmoms";
    std::string lucky = "lucky";

    if (boost::iequals(option, without)) {
        heuristics = 1;
    }
    else if (boost::iequals(option, yesNo)) {
        heuristics = 2;
    }
    else if (boost::iequals(option, dlis)) {
        heuristics = 3;
    }
    else if (boost::iequals(option, dlcs)) {
        heuristics = 4;
    }
    else if (boost::iequals(option, jw)) {
        heuristics = 5;
    }
    else if (boost::iequals(option, rdlcs)) {
        heuristics = 6;
    }
    else if (boost::iequals(option, rdlis)) {
        heuristics = 7;
    }
    else if (boost::iequals(option, random)) {
        heuristics = 8;
    }
    else if (boost::iequals(option, moms)) {
        heuristics = 9;
    }
    else if (boost::iequals(option, rmoms)) {
        heuristics = 10;
    }
    else if (boost::iequals(option, rjw)) {
        heuristics = 11;
    }
    else if (boost::iequals(option, lucky)) {
        heuristics = getRandomIndex(2, 9); // the upper bound is decremented so the max is option 8
        #ifdef DEBUG
        std::cout << "Picked: " << heuristics << '\n';
        #endif
    }
    else {
        throw std::invalid_argument("Error reading heuristics option.");
    }
}

// Solve the SAT problem.
bool Solver::solve() {
    //std::clock_t start = std::clock();
    pureLiteral();
    while (state == Solver::State::UNDEF) {
        unitPropagate();
        if (checkConflict()) {
            if (numberDecisions == 0) {
                applyExplainEmpty();
                applyLearn();
                state = Solver::State::UNSAT;
            } else {
                applyExplainUIP();
                applyLearn();
                applyBackjump();
            }
        } else {
            if (trail.size() == numberVariables)
                state = Solver::State::SAT;
            else
                decideLiteral();
        }
    }
    //std::clock_t end = std::clock();
    //double testTime = double(end-start);
    //std::cout << testTime << '\n';
    //double totalCpuTime = double(end-start)/CLOCKS_PER_SEC;
    //std::cout << totalCpuTime << '\n';

    if (state == State::SAT) {
        std::sort(trail.begin(), trail.end(), [](const auto & l1, const auto & l2) {
            const int abs1 = std::abs(l1.first);
            const int abs2 = std::abs(l2.first);
            return abs1 < abs2;
        });

        return true;
    }

    return false;
}

// Print the SAT problem.
std::ostream & operator<<(std::ostream & out, const Solver & solver) {
    switch (solver.state) {
    case Solver::State::UNDEF:
        out << "UNDEF\n";
        break;
    case Solver::State::SAT:
        out << "s SATISFIABLE\n";
        // for runTests.py:
        //out << "S";
        break;
    case Solver::State::UNSAT:
        out << "s UNSATISFIABLE\n";
        // for runTests.py:
        //out << "U";
        break;
    }
    // uncomment for runTests.py
    if (solver.state == Solver::State::SAT) {
        std::cout << "v ";
        for (const auto & literal : solver.trail)
            out << literal.first << ' ';
    }

    return out;
}
