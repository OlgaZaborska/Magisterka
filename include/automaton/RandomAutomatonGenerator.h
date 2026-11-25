#pragma once

#include "algebra/WeightAlgebra.h"
#include "automaton/WeightedAutomaton.h"

#include <random>

inline constexpr Symbol RandomAutomatonAlphabet[] = {'a', 'b', 'c', 'd'};

WeightedAutomaton generateRandomAutomaton(
    std::size_t numberOfStates,
    const WeightAlgebra& algebra,
    std::mt19937& random,
    Weight minimumWeight = 0,
    Weight maximumWeight = 5
);
