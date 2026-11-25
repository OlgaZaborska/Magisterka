#pragma once

#include "learning/DiscriminationTree.h"
#include "learning/ResidualFunction.h"
#include "automaton/WeightedAutomaton.h"
#include "core/WordPairAdvice.h"

#include <vector>

class HypothesisBuilder {
private:
    ResidualFunction& residualFunction_;
    DiscriminationTree& discriminationTree_;

public:
    HypothesisBuilder(
        ResidualFunction& residualFunction,
        DiscriminationTree& discriminationTree
    );

    WeightedAutomaton build(
        const std::vector<Symbol>& alphabet,
        const WordPairAdvice* advice = nullptr
    );
};