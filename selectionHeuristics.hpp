#ifndef __SELECTIONHEURISTICS_HPP__
#define __SELECTIONHEURISTICS_HPP__

#include <cmath>
#include <limits>
#include <random>

#include "old.hpp"

// Helper for random selection heuristics.
// Takes a lower and upper bound and returns a random index within the bounds.
// It's mainly used for vectors, therefore the upper bound is decremented.
int getRandomIndex(int lowerBound, int upperBound) {
    std::random_device randDevice;
    std::mt19937 rng(randDevice());
    std::uniform_int_distribution<int> result(lowerBound,upperBound-1);
    int randIndex = result(rng);
    return randIndex;
}

// Helper for computing the combined sum of occurrences (both polarities).
// If the first parameter is true, it computes the combined sum for the MOMS heuristic
// and uses the int to determine the 'cutoffLength' of a clause.
// If false, it computes the usual combined sum for DLCS.
void MaphSAT::combinedSum(std::vector<std::pair<int, int> > & pos, std::vector<std::pair<int, int> > & neg, bool constraint, std::size_t cutoffLength) const {
    int counterPos = 0;
    int counterNeg = 0;
    for (const auto & clause : formula){
        for (int literal : clause) {
            if (constraint && clause.size() > cutoffLength)
                break;
            if (std::find_if(trail.begin(), trail.end(), [literal](const auto & p) { return p.first == literal || p.first == -literal; }) == trail.end()) {
                auto itPos = std::find_if(pos.begin(), pos.end(), [&counterPos, literal](const auto & p) {
                    counterPos = p.second;
                    return p.first == literal || p.first == -literal;
                });
                auto itNeg = std::find_if(neg.begin(), neg.end(), [&counterNeg, literal](const auto & p) {
                    counterNeg = p.second;
                    return p.first == literal || p.first == -literal;
                });

                if (literal > 0) {
                    if (itPos == pos.end() && literal > 0)
                        pos.emplace_back(std::make_pair(literal, 1));
                    else {
                        pos.erase(itPos);
                        const int newPosCounter = ++counterPos;
                        pos.emplace_back(literal, newPosCounter);
                    }
                } else if (literal < 0) {
                    if (itNeg == neg.end())
                        neg.emplace_back(literal, 1);
                    else {
                        neg.erase(itNeg);
                        const int newNegCounter = ++counterNeg;
                        neg.emplace_back(literal, newNegCounter);
                    }
                }
            }
        }
    }
}

// Select the first literal that is not yet asserted.
int MaphSAT::selectFirst() const {
    for (const auto & clause : formula) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [literal](const auto & p) { return p.first == literal || p.first == -literal; }) == trail.end())
                return literal;
        }
    }
    return 0;
}

// Selection heuristic: Pick a random literal.
int MaphSAT::selectRandom() const {
    int maxLit = 0;
    std::vector<int> randCandidates;

    for (const auto & clause : formula) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [literal](const auto & p) { return p.first == literal || p.first == -literal; }) == trail.end())
                randCandidates.push_back(literal);
        }
    }
    if (!randCandidates.empty()) {
        const int randIndex = getRandomIndex(0, randCandidates.size());
        maxLit = randCandidates[randIndex];
    }
    if (maxLit > 0)
        return maxLit;
    else
        return -maxLit;
}

// Selection heuristic: Dynamic Largest Individual Sum.
// Picks the literal with the highest number of occurrences in the unsatisfied clauses.
// Sets value to true if the literal is positive.
// If the literal is negative, sets the value of its negation to true.
// If randomized true, it runs the randomized DLIS variant.
int MaphSAT::selectDLIS(bool random) const {
    int counter = 0;
    int maxNumber = 0;
    int maxLit = 0;
    std::vector<int> randCandidates;
    std::vector<std::pair<int, int> > vCount;

    for (const auto & clause : formula) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(),
                [literal](const auto & p) { return p.first == literal || p.first == -literal; }) == trail.end()) {
                auto it = std::find_if(vCount.begin(), vCount.end(), [&counter, literal](const auto & p) {
                    counter = p.second;
                    return p.first == literal || p.first == -literal;
                });
                if (it == vCount.end()) {
                    vCount.emplace_back(literal, 1);
                    if (maxNumber > 1) {
                        maxNumber = 1;
                        maxLit = literal;
                    } else if (random && maxNumber == 1)
                        randCandidates.push_back(literal);
                } else {
                    vCount.erase(it);
                    const int newValue = ++counter;
                    vCount.emplace_back(literal, newValue);
                    if (newValue > maxNumber) {
                        maxNumber = newValue;
                        maxLit = literal;
                    } else if (random && newValue == maxNumber)
                        randCandidates.push_back(literal);
                }
            }
        }
    }

    if (random && !randCandidates.empty()) {
        const int randIndex = getRandomIndex(0, randCandidates.size());
        maxLit = randCandidates[randIndex];
    }
    if (maxLit > 0)
        return maxLit;
    else
        return -maxLit;
}

// Selection heuristic: Dynamic Largest Combined Sum.
// Picks the variable with the highest number of occurrences of its positive and negative literals (combined).
// If randomized true, it runs the randomized DLCS variant.
int MaphSAT::selectDLCS(bool random) const {
    int counterNeg = 0;
    int maxScore = 0;
    int maxLit = 0;
    std::vector<int> randCandidates;
    std::vector<std::pair<int, int> > posVariableCount;
    std::vector<std::pair<int, int> > negVariableCount;

    combinedSum(posVariableCount, negVariableCount, false, 0);

    for (auto & posLit : posVariableCount) {
        auto itNeg = std::find_if(negVariableCount.begin(), negVariableCount.end(), [&counterNeg, posLit](const auto & negLit) {
            counterNeg = negLit.second;
            return negLit.first == -posLit.first;
        });
        if (itNeg != negVariableCount.end()) {
            if (counterNeg + posLit.second > maxScore) {
                maxScore = counterNeg + posLit.second;
                maxLit = posLit.first;
                negVariableCount.erase(itNeg);
            } else if (random && (counterNeg + posLit.second) == maxScore) {
                randCandidates.push_back(posLit.first);
                negVariableCount.erase(itNeg);
            }
        }
    }

    if (maxLit == 0) {
        if (!posVariableCount.empty())
            maxLit = posVariableCount.front().first;
        else if (!negVariableCount.empty()) {
            maxLit = -negVariableCount.front().first;
        }
    }

    if (random && !randCandidates.empty()) {
        const int randIndex = getRandomIndex(0, randCandidates.size());
        maxLit = randCandidates[randIndex];
    }

    return maxLit;
}

// Selection heuristic: the Jeroslow-Wang method.
// If randomized true, it runs the randomized J-W variant.
int MaphSAT::selectJW(bool random) const {
    double score = 0;
    double maxScore = std::numeric_limits<double>::min();
    int maxLit = 0;
    std::vector<int> randCandidates;
    std::vector<std::pair<int, double> > JWcount;

    for (const auto & clause : formula) {
        for (int literal : clause) {
            if (std::find_if(trail.begin(), trail.end(), [literal](const auto & p) { return p.first == literal || p.first == -literal; }) == trail.end()) {
                auto it = std::find_if(JWcount.begin(), JWcount.end(), [&score, literal](const auto & p) {
                    score = p.second;
                    return p.first == literal || p.first == -literal;
                });
                if (it == JWcount.end()) {
                    double initialScore = std::pow(2.0, -clause.size());
                    JWcount.emplace_back(literal, initialScore);
                    if (initialScore > maxScore) {
                        maxScore = initialScore;
                        maxLit = literal;
                    } else if (random && initialScore == maxScore)
                        randCandidates.push_back(literal);
                } else {
                    JWcount.erase(it);
                    score = score + pow(2.0, -clause.size());
                    JWcount.emplace_back(literal, score);
                    if (score > maxScore) {
                        maxScore = score;
                        maxLit = literal;
                    } else if (random && score == maxScore)
                        randCandidates.push_back(literal);
                }
            }
        }
    }

    if (maxLit != 0) {
        if (random && !randCandidates.empty()) {
            const int randIndex = getRandomIndex(0, randCandidates.size());
            maxLit = randCandidates[randIndex];
        }
        if (maxLit > 0)
            return maxLit;
        else
            return -maxLit;
    }

    return 0;
}

// Selection heuristic: Maximum [number of] Occurrences in Minimum [length] Clauses.
// If randomized true, it runs the randomized MOMS variant.
int MaphSAT::selectMOMS(bool random) const {
    int maxLit = 0;
    int counterNeg = 0;
    int maxScore = 0;
    int parameter = 10; // as suggested in: J. Freeman, “Improvements to propositional satisfiability search algorithms” , PhD thesis, The University of Pennsylvania, 1995.
    int cutoffLength = 0;
    std::size_t totalClauseLength = 0;
    std::vector<int> randCandidates;
    std::vector<std::pair<int, int> > posVariableCount;
    std::vector<std::pair<int, int> > negVariableCount;

    for (const auto & clause : formula)
        totalClauseLength += clause.size();

    cutoffLength = totalClauseLength / formula.size();
    if (cutoffLength >= 2)
        --cutoffLength;

    combinedSum(posVariableCount, negVariableCount, true, cutoffLength);

    for (auto & posLit : posVariableCount) {
        auto itNeg = std::find_if(negVariableCount.begin(), negVariableCount.end(), [&counterNeg, posLit](const auto & negLit) {
            counterNeg = negLit.second;
            return negLit.first == -posLit.first;
        });
        if (itNeg != negVariableCount.end()) {
            const int tempScore = (counterNeg + posLit.second) * std::pow(2, parameter) + (counterNeg * posLit.second);
            if (tempScore > maxScore) {
                maxScore = tempScore;
                maxLit = posLit.first;
                negVariableCount.erase(itNeg);
            } else if (random && tempScore == maxScore) {
                randCandidates.push_back(posLit.first);
            }
        }
    }

    if (maxLit == 0)
        maxLit = selectFirst();
    if (random && !randCandidates.empty()) {
        const int randIndex = getRandomIndex(0, randCandidates.size());
        maxLit = randCandidates[randIndex];
    }

    return maxLit;
}

#endif