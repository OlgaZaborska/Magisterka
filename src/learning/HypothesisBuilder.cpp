#include "learning/HypothesisBuilder.h"

#include <algorithm>
#include <map>
#include <stdexcept>

HypothesisBuilder::HypothesisBuilder(
    ResidualFunction& residualFunction,
    DiscriminationTree& discriminationTree
)
    : residualFunction_(residualFunction),
      discriminationTree_(discriminationTree) {}

WeightedAutomaton HypothesisBuilder::build(
    const std::vector<Symbol>& alphabet
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

    for (const AccessSequence& sequence : accessSequences) {
        StateId sourceState = stateIds.at(sequence.word());

        for (Symbol symbol : alphabet) {
            AccessSequence extendedSequence =
                sequence.extend(symbol);

            AccessSequence targetSequence =
                discriminationTree_.sift(extendedSequence.word());

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

    return hypothesis;
}
