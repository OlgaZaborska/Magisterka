
#pragma once

#include "automaton/WeightedAutomaton.h"

#include <optional>

class EquivalenceOracle {
public:
    virtual ~EquivalenceOracle() = default;

    virtual std::optional<Word> findCounterexample(
        const WeightedAutomaton& hypothesis
    ) = 0;
};
