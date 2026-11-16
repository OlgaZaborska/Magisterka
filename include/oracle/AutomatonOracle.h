#pragma once

#include "oracle/MembershipOracle.h"
#include "automaton/WeightedAutomaton.h"

class AutomatonOracle : public MembershipOracle {
public:
    explicit AutomatonOracle(const WeightedAutomaton& automaton);

    Weight query(const Word& word) override;

private:
    const WeightedAutomaton& automaton_;
};