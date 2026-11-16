
#include "oracle/SumEquivalenceChecker.h"

#include <queue>
#include <unordered_map>
#include <stdexcept>

namespace {

struct StatePair {
    StateId target;
    StateId hypothesis;

    bool operator==(const StatePair& other) const {
        return target == other.target &&
               hypothesis == other.hypothesis;
    }
};

struct StatePairHash {
    std::size_t operator()(const StatePair& pair) const {
        std::size_t h1 = std::hash<StateId>{}(pair.target);
        std::size_t h2 = std::hash<StateId>{}(pair.hypothesis);

        return h1 ^ (h2 << 1);
    }
};

struct SearchNode {
    StatePair states;
    Weight weightDifference;
    Word word;
};

struct PreviousPath {
    Weight weightDifference;
    Word word;
};

}

SumEquivalenceChecker::SumEquivalenceChecker(
    const WeightedAutomaton& target,
    const std::vector<Symbol>& alphabet
)
    : target_(target),
      alphabet_(alphabet) {}

std::optional<Word> SumEquivalenceChecker::findCounterexample(
    const WeightedAutomaton& hypothesis
) {
    std::queue<SearchNode> queue;

    std::unordered_map<StatePair, PreviousPath, StatePairHash> visited;

    StatePair initialPair{
        target_.initialState(),
        hypothesis.initialState()
    };

    queue.push({initialPair, 0, Word{}});

    visited.emplace(
        initialPair,
        PreviousPath{0, Word{}}
    );

    while (!queue.empty()) {
        SearchNode current = queue.front();
        queue.pop();

        Weight finalDifference =
            current.weightDifference +
            target_.finalWeight(current.states.target) -
            hypothesis.finalWeight(current.states.hypothesis);

        if (finalDifference != 0) {
            return current.word;
        }

        for (Symbol symbol : alphabet_) {
            bool targetHasTransition =
                target_.hasTransition(current.states.target, symbol);

            bool hypothesisHasTransition =
                hypothesis.hasTransition(current.states.hypothesis, symbol);

            if (targetHasTransition != hypothesisHasTransition) {
                throw std::runtime_error(
                    "Automata have different transition structures"
                );
            }

            if (!targetHasTransition) {
                continue;
            }

            StateId nextTarget =
                target_.nextState(current.states.target, symbol);

            StateId nextHypothesis =
                hypothesis.nextState(current.states.hypothesis, symbol);

            Weight nextDifference =
                current.weightDifference +
                target_.transitionWeight(current.states.target, symbol) -
                hypothesis.transitionWeight(current.states.hypothesis, symbol);

            Word nextWord = current.word;
            nextWord.push_back(symbol);

            StatePair nextPair{nextTarget, nextHypothesis};

            auto it = visited.find(nextPair);

            if (it != visited.end()) {
                if (it->second.weightDifference != nextDifference) {
                    Weight finalDifference =
                        target_.finalWeight(nextTarget) -
                        hypothesis.finalWeight(nextHypothesis);

                    if (nextDifference + finalDifference != 0) {
                        return nextWord;
                    }
                }

                continue;
            }


            visited.emplace(
                nextPair,
                PreviousPath{nextDifference, nextWord}
            );

            queue.push({
                nextPair,
                nextDifference,
                nextWord
            });
        }
    }

    return std::nullopt;
}
