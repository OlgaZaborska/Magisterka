#include "automaton/RandomAutomatonGenerator.h"

#include <iterator>
#include <stdexcept>
#include <vector>

WeightedAutomaton generateRandomAutomaton(
    std::size_t numberOfStates,
    const WeightAlgebra& algebra,
    std::mt19937& random,
    Weight minimumWeight,
    Weight maximumWeight
) {
    if (numberOfStates == 0) {
        throw std::invalid_argument(
            "A random automaton must have at least one state"
        );
    }

    if (minimumWeight > maximumWeight) {
        throw std::invalid_argument(
            "Minimum weight cannot exceed maximum weight"
        );
    }

    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );
    WeightedAutomaton automaton(alphabet, algebra);

    std::uniform_int_distribution<Weight> weightDistribution(
        minimumWeight,
        maximumWeight
    );
    std::uniform_int_distribution<StateId> stateDistribution(
        0,
        numberOfStates - 1
    );

    for (std::size_t i = 0; i < numberOfStates; ++i) {
        automaton.addState(0);
    }

    automaton.setInitialState(0);

    for (StateId source = 0; source < numberOfStates; ++source) {
        for (Symbol symbol : alphabet) {
            automaton.addTransition(
                source,
                symbol,
                stateDistribution(random),
                weightDistribution(random)
            );
        }
    }

    return automaton;
}
