
#pragma once

#include "oracle/EquivalenceOracle.h"

class SumEquivalenceChecker : public EquivalenceOracle {
private:
    const WeightedAutomaton& target_;
    std::vector<Symbol> alphabet_;

public:
    SumEquivalenceChecker(
        const WeightedAutomaton& target,
        const std::vector<Symbol>& alphabet
    );

    std::optional<Word> findCounterexample(
        const WeightedAutomaton& hypothesis
    ) override;
};
