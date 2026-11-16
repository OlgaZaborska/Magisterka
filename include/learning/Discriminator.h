
#pragma once

#include "core/Types.h"
#include "learning/ResidualFunction.h"

class Discriminator {
private:
    Word suffix_;
    ResidualFunction& residualFunction_;

public:
    Discriminator(
        const Word& suffix,
        ResidualFunction& residualFunction
    )
        : suffix_(suffix),
          residualFunction_(residualFunction) {
    }

    Weight evaluate(const Word& prefix) {
        return residualFunction_.compute(prefix, suffix_);
    }

    const Word& suffix() const {
        return suffix_;
    }
};
