#include "learning/HypothesisBuilder.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <stdexcept>

namespace {

class DisjointSet {
public:
    explicit DisjointSet(std::size_t size)
        : parents_(size),
          ranks_(size, 0) {
        std::iota(parents_.begin(), parents_.end(), 0);
    }

    StateId find(StateId state) {
        if (parents_[state] != state) {
            parents_[state] = find(parents_[state]);
        }
        return parents_[state];
    }

    bool unite(StateId left, StateId right) {
        left = find(left);
        right = find(right);
        if (left == right) {
            return false;
        }

        if (ranks_[left] < ranks_[right]) {
            std::swap(left, right);
        }
        parents_[right] = left;
        if (ranks_[left] == ranks_[right]) {
            ++ranks_[left];
        }
        return true;
    }

private:
    std::vector<StateId> parents_;
    std::vector<std::size_t> ranks_;
};

struct WordEffect {
    StateId state;
    Weight weight;
};

WordEffect followAdviceWord(
    const WeightedAutomaton& automaton,
    DisjointSet& states,
    StateId source,
    const Word& word
) {
    StateId current = states.find(source);
    Weight accumulatedWeight = 0;
    for (Symbol symbol : word) {
        accumulatedWeight +=
            automaton.transitionWeight(current, symbol);
        current = states.find(
            automaton.nextState(current, symbol)
        );
    }
    return {current, accumulatedWeight};
}

WeightedAutomaton applyWordPairAdvice(
    const WeightedAutomaton& automaton,
    const std::vector<Symbol>& alphabet,
    const WeightAlgebra& algebra,
    const WordPairAdvice& advice
) {
    const std::size_t numberOfStates = automaton.numberOfStates();
    DisjointSet states(numberOfStates);

    for (StateId source = 0; source < numberOfStates; ++source) {
        const WordEffect afterU =
            followAdviceWord(automaton, states, source, advice.u);
        const WordEffect afterV =
            followAdviceWord(automaton, states, source, advice.v);
        if (afterU.weight != afterV.weight) {
            return automaton;
        }
    }

    bool changed = true;

    while (changed) {
        changed = false;

        for (StateId source = 0; source < numberOfStates; ++source) {
            if (states.find(source) != source) {
                continue;
            }

            const WordEffect afterU = followAdviceWord(
                automaton,
                states,
                source,
                advice.u
            );
            const WordEffect afterV = followAdviceWord(
                automaton,
                states,
                source,
                advice.v
            );
            changed = states.unite(afterU.state, afterV.state) || changed;
        }

        std::vector<StateId> representatives(
            numberOfStates,
            numberOfStates
        );
        for (StateId state = 0; state < numberOfStates; ++state) {
            const StateId representative = states.find(state);
            if (representatives[representative] == numberOfStates) {
                representatives[representative] = state;
                continue;
            }

            const StateId other = representatives[representative];
            for (Symbol symbol : alphabet) {
                changed = states.unite(
                    automaton.nextState(state, symbol),
                    automaton.nextState(other, symbol)
                ) || changed;
            }
        }
    }

    std::vector<StateId> representatives(
        numberOfStates,
        numberOfStates
    );
    for (StateId state = 0; state < numberOfStates; ++state) {
        const StateId representative = states.find(state);
        if (representatives[representative] == numberOfStates) {
            representatives[representative] = state;
            continue;
        }

        const StateId other = representatives[representative];
        if (automaton.finalWeight(state) !=
            automaton.finalWeight(other)) {
            return automaton;
        }
        for (Symbol symbol : alphabet) {
            if (automaton.transitionWeight(state, symbol) !=
                automaton.transitionWeight(other, symbol)) {
                return automaton;
            }
        }
    }

    WeightedAutomaton quotient(alphabet, algebra);
    std::vector<StateId> quotientIds(
        numberOfStates,
        numberOfStates
    );
    for (StateId state = 0; state < numberOfStates; ++state) {
        if (states.find(state) != state) {
            continue;
        }
        quotientIds[state] = quotient.addState(
            automaton.finalWeight(state)
        );
    }

    quotient.setInitialState(
        quotientIds[states.find(automaton.initialState())]
    );

    for (StateId state = 0; state < numberOfStates; ++state) {
        if (states.find(state) != state) {
            continue;
        }
        for (Symbol symbol : alphabet) {
            const StateId target = states.find(
                automaton.nextState(state, symbol)
            );
            quotient.addTransition(
                quotientIds[state],
                symbol,
                quotientIds[target],
                automaton.transitionWeight(state, symbol)
            );
        }
    }

    return quotient;
}

}

HypothesisBuilder::HypothesisBuilder(
    ResidualFunction& residualFunction,
    DiscriminationTree& discriminationTree
)
    : residualFunction_(residualFunction),
      discriminationTree_(discriminationTree) {}

WeightedAutomaton HypothesisBuilder::build(
    const std::vector<Symbol>& alphabet,
    const WordPairAdvice* advice
) {
    std::vector<AccessSequence> accessSequences =
        discriminationTree_.accessSequences();

    if (accessSequences.empty()) {
        throw std::runtime_error(
            "Cannot build hypothesis: discrimination tree is empty"
        );
    }

    WeightedAutomaton hypothesis(
        alphabet,
        residualFunction_.algebra()
    );

    std::map<Word, StateId> stateIds;

    auto initialSequence = std::find_if(
        accessSequences.begin(),
        accessSequences.end(),
        [](const AccessSequence& sequence) {
            return sequence.empty();
        }
    );

    if (initialSequence == accessSequences.end()) {
        throw std::runtime_error(
            "Initial access sequence epsilon not found"
        );
    }

    StateId initialStateId = hypothesis.addState(0);
    stateIds.emplace(initialSequence->word(), initialStateId);

    for (const AccessSequence& sequence : accessSequences) {
        if (sequence.empty()) {
            continue;
        }

        StateId state = hypothesis.addState(0);
        stateIds.emplace(sequence.word(), state);
    }

    hypothesis.setInitialState(initialStateId);

    for (std::size_t sequenceIndex = 0;
         sequenceIndex < accessSequences.size();
         ++sequenceIndex) {
        const AccessSequence sequence = accessSequences[sequenceIndex];
        StateId sourceState = stateIds.at(sequence.word());
        for (Symbol symbol : alphabet) {
            AccessSequence extendedSequence =
                sequence.extend(symbol);

            AccessSequence targetSequence =
                discriminationTree_.sift(extendedSequence.word());

            for (const AccessSequence& discoveredSequence :
                 discriminationTree_.accessSequences()) {
                if (stateIds.contains(discoveredSequence.word())) {
                    continue;
                }

                StateId discoveredState =
                    hypothesis.addState(0);
                stateIds.emplace(
                    discoveredSequence.word(),
                    discoveredState
                );
                accessSequences.push_back(discoveredSequence);
            }

            auto targetState = stateIds.find(targetSequence.word());

            if (targetState == stateIds.end()) {
                throw std::runtime_error(
                    "Target access sequence not found among tree leaves"
                );
            }

            Weight transitionWeight =
                residualFunction_.compute(
                    sequence.word(),
                    Word{symbol}
                );

            hypothesis.addTransition(
                sourceState,
                symbol,
                targetState->second,
                transitionWeight
            );
        }
    }

    if (advice == nullptr || advice->type != AdviceType::Eq) {
        return hypothesis;
    }

    return applyWordPairAdvice(
        hypothesis,
        alphabet,
        residualFunction_.algebra(),
        *advice
    );
}
