#include "learning/WeightedTTT.h"

#include <optional>
#include <stdexcept>

WeightedTTT::WeightedTTT(
    MembershipOracle& membershipOracle,
    EquivalenceOracle& equivalenceOracle,
    const WeightAlgebra& algebra,
    const std::vector<Symbol>& alphabet,
    std::optional<WordPairAdvice> advice
)
    : membershipOracle_(membershipOracle),
      equivalenceOracle_(equivalenceOracle),
      algebra_(algebra),
      alphabet_(alphabet),
      advice_(advice),
      residualFunction_(membershipOracle_, algebra_),
      discriminationTree_(AccessSequence{}, residualFunction_) {
}

bool WeightedTTT::refine(const Word& counterexample) {
    for (std::size_t prefixLength = 1;
         prefixLength <= counterexample.size();
         ++prefixLength) {
        Word prefix(
            counterexample.begin(),
            counterexample.begin() + prefixLength
        );
        const std::size_t leavesBeforeSift =
            discriminationTree_.accessSequences().size();
        AccessSequence oldAccessSequence =
            discriminationTree_.sift(prefix);

        if (oldAccessSequence.word() == prefix) {
            if (discriminationTree_.accessSequences().size() >
                leavesBeforeSift) {
                return true;
            }
            continue;
        }

        for (std::size_t suffixStart = prefixLength;
             suffixStart <= counterexample.size();
             ++suffixStart) {
            Word suffix(
                counterexample.begin() + suffixStart,
                counterexample.end()
            );

            Weight targetResidual =
                residualFunction_.compute(prefix, suffix);
            Weight representativeResidual =
                residualFunction_.compute(
                    oldAccessSequence.word(),
                    suffix
                );

            if (!algebra_.equal(targetResidual, representativeResidual)) {
                discriminationTree_.split(
                    oldAccessSequence,
                    AccessSequence(prefix),
                    suffix
                );
                return true;
            }
        }
    }

    return false;
}

WeightedAutomaton WeightedTTT::learn() {
    while (true) {
        HypothesisBuilder builder(
            residualFunction_,
            discriminationTree_
        );
        const WordPairAdvice* advice =
            advice_.has_value() ? &advice_.value() : nullptr;
        WeightedAutomaton hypothesis = builder.build(alphabet_, advice);

        std::optional<Word> counterexample =
            equivalenceOracle_.findCounterexample(hypothesis);

        if (!counterexample.has_value()) {
            return hypothesis;
        }

        if (!refine(counterexample.value())) {
            throw std::runtime_error(
                "Counterexample does not provide a distinguishable residual"
            );
        }
    }
}
