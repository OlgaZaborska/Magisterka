#pragma once

#include "automaton/WeightedAutomaton.h"
#include "core/WordPairAdvice.h"
#include "learning/DiscriminationTree.h"
#include "learning/HypothesisBuilder.h"
#include "learning/ResidualFunction.h"
#include "oracle/EquivalenceOracle.h"
#include "oracle/MembershipOracle.h"

#include <optional>
#include <vector>

class WeightedTTT {
private:
    MembershipOracle& membershipOracle_;
    EquivalenceOracle& equivalenceOracle_;
    const WeightAlgebra& algebra_;
    std::vector<Symbol> alphabet_;
    std::optional<WordPairAdvice> advice_;
    ResidualFunction residualFunction_;
    DiscriminationTree discriminationTree_;

    bool refine(const Word& counterexample);

public:
    WeightedTTT(
        MembershipOracle& membershipOracle,
        EquivalenceOracle& equivalenceOracle,
        const WeightAlgebra& algebra,
        const std::vector<Symbol>& alphabet,
        std::optional<WordPairAdvice> advice = std::nullopt
    );

    WeightedAutomaton learn();
};
