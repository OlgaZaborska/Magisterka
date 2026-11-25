#pragma once

#include "algebra/WeightAlgebra.h"
#include "automaton/WeightedAutomaton.h"
#include "core/WordPairAdvice.h"

#include <random>

inline constexpr Symbol RandomAutomatonAlphabet[] = {'a', 'b', 'c', 'd'};

WeightedAutomaton generateRandomAutomaton(
    std::size_t numberOfStates,
    const WeightAlgebra& algebra,
    std::mt19937& random,
    Weight minimumWeight = 0,
    Weight maximumWeight = 5
);

WeightedAutomaton generateRandomAutomatonWithAdvice(
    std::size_t numberOfStates,
    const WordPairAdvice& advice,
    const WeightAlgebra& algebra,
    std::mt19937& random,
    Weight minimumPotential = 0,
    Weight maximumPotential = 5
);
