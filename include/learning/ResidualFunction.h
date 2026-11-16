#pragma once

#include "core/Types.h"
#include "oracle/MembershipOracle.h"
#include "algebra/WeightAlgebra.h"

class ResidualFunction {
private:
    MembershipOracle& oracle_;
    const WeightAlgebra& algebra_;

public:
    ResidualFunction(
        MembershipOracle& oracle,
        const WeightAlgebra& algebra
    )
        : oracle_(oracle),
          algebra_(algebra) {
    }

    Weight compute(
        const Word& prefix,
        const Word& suffix
    ) {
        // Construct prefix + suffix
        Word concatenated = prefix;

        concatenated.insert(
            concatenated.end(),
            suffix.begin(),
            suffix.end()
        );

        // Query the membership oracle
        Weight extendedValue = oracle_.query(concatenated);
        Weight prefixValue = oracle_.query(prefix);

        // D(prefix, suffix) = f(prefix + suffix) - f(prefix)
        return algebra_.residual(
            extendedValue,
            prefixValue
        );
    }

    const WeightAlgebra& algebra() const {
        return algebra_;
    }
};