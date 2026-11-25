#include "automaton/RandomAutomatonGenerator.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace {

std::size_t symbolIndex(Symbol symbol) {
    for (std::size_t i = 0;
         i < std::size(RandomAutomatonAlphabet);
         ++i) {
        if (RandomAutomatonAlphabet[i] == symbol) {
            return i;
        }
    }

    throw std::invalid_argument(
        "Advice words must use symbols from the automaton alphabet"
    );
}

StateId positiveModulo(Weight value, StateId modulus) {
    const Weight signedModulus = static_cast<Weight>(modulus);
    Weight remainder = value % signedModulus;
    if (remainder < 0) {
        remainder += signedModulus;
    }
    return static_cast<StateId>(remainder);
}

Weight checkedDifference(Weight left, Weight right) {
    if ((right > 0 &&
         left < std::numeric_limits<Weight>::min() + right) ||
        (right < 0 &&
         left > std::numeric_limits<Weight>::max() + right)) {
        throw std::overflow_error(
            "Potential difference exceeds the supported weight range"
        );
    }
    return left - right;
}

}

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

WeightedAutomaton generateRandomAutomatonWithAdvice(
    std::size_t numberOfStates,
    const WordPairAdvice& advice,
    const WeightAlgebra& algebra,
    std::mt19937& random,
    Weight minimumPotential,
    Weight maximumPotential
) {
    if (numberOfStates == 0) {
        throw std::invalid_argument(
            "A random automaton must have at least one state"
        );
    }
    if (advice.type != AdviceType::Eq) {
        throw std::invalid_argument(
            "The advice-aware generator currently supports only Eq advice"
        );
    }
    if (minimumPotential > maximumPotential) {
        throw std::invalid_argument(
            "Minimum potential cannot exceed maximum potential"
        );
    }

    std::array<Weight, 4> countDifference{};
    for (Symbol symbol : advice.u) {
        ++countDifference[symbolIndex(symbol)];
    }
    for (Symbol symbol : advice.v) {
        --countDifference[symbolIndex(symbol)];
    }

    std::array<StateId, 4> stateOffsets{};
    std::vector<std::size_t> zeroDifferenceIndices;
    std::vector<std::size_t> nonzeroDifferenceIndices;
    for (std::size_t i = 0; i < countDifference.size(); ++i) {
        if (countDifference[i] == 0) {
            zeroDifferenceIndices.push_back(i);
        } else {
            nonzeroDifferenceIndices.push_back(i);
        }
    }

    if (numberOfStates > 1) {
        std::uniform_int_distribution<StateId> unitDistribution(
            1,
            numberOfStates - 1
        );
        const auto randomUnit = [&]() {
            StateId candidate;
            do {
                candidate = unitDistribution(random);
            } while (std::gcd(candidate, numberOfStates) != 1);
            return candidate;
        };

        if (nonzeroDifferenceIndices.empty()) {
            std::uniform_int_distribution<std::size_t> symbolDistribution(
                0,
                stateOffsets.size() - 1
            );
            stateOffsets[symbolDistribution(random)] = randomUnit();
        } else if (!zeroDifferenceIndices.empty()) {
            std::uniform_int_distribution<std::size_t> zeroIndexDistribution(
                0,
                zeroDifferenceIndices.size() - 1
            );
            stateOffsets[zeroDifferenceIndices[zeroIndexDistribution(random)]] =
                randomUnit();
        } else {
            std::uniform_int_distribution<std::size_t> firstIndexDistribution(
                0,
                nonzeroDifferenceIndices.size() - 1
            );
            const std::size_t firstPosition =
                firstIndexDistribution(random);
            std::uniform_int_distribution<std::size_t> secondPositionDistribution(
                0,
                nonzeroDifferenceIndices.size() - 2
            );
            std::size_t secondPosition =
                secondPositionDistribution(random);
            if (secondPosition >= firstPosition) {
                ++secondPosition;
            }

            const std::size_t firstIndex =
                nonzeroDifferenceIndices[firstPosition];
            const std::size_t secondIndex =
                nonzeroDifferenceIndices[secondPosition];
            const Weight divisor = std::gcd(
                countDifference[firstIndex],
                countDifference[secondIndex]
            );
            stateOffsets[firstIndex] = positiveModulo(
                countDifference[secondIndex] / divisor,
                numberOfStates
            );
            stateOffsets[secondIndex] = positiveModulo(
                -countDifference[firstIndex] / divisor,
                numberOfStates
            );
        }
    }

    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );
    WeightedAutomaton automaton(alphabet, algebra);
    std::uniform_int_distribution<Weight> potentialDistribution(
        minimumPotential,
        maximumPotential
    );

    std::vector<Weight> potentials(numberOfStates);
    for (Weight& potential : potentials) {
        potential = potentialDistribution(random);
        automaton.addState(0);
    }
    automaton.setInitialState(0);

    for (StateId source = 0; source < numberOfStates; ++source) {
        for (std::size_t i = 0; i < alphabet.size(); ++i) {
            const StateId offset = stateOffsets[i];
            const StateId target =
                source >= numberOfStates - offset
                    ? source - (numberOfStates - offset)
                    : source + offset;
            const Weight transitionWeight = checkedDifference(
                potentials[target],
                potentials[source]
            );
            automaton.addTransition(
                source,
                alphabet[i],
                target,
                transitionWeight
            );
        }
    }

    return automaton;
}
