#pragma once
#include "WeightAlgebra.h"

class SumAlgebra : public WeightAlgebra {
public:
    Weight identity() const override {
        return 0;
    }

    Weight combine(
        Weight a,
        Weight b
    ) const override {
        return a + b;
    }

    bool equal(
        Weight a,
        Weight b
    ) const override {
        return a == b;
    }
};