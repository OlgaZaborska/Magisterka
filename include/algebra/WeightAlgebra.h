#pragma once
#include <cstdint>
#include "core/Types.h"

class WeightAlgebra {
public:
    virtual ~WeightAlgebra() = default;

    virtual Weight identity() const = 0;

    virtual Weight combine(
        Weight a,
        Weight b
    ) const = 0;

    virtual bool equal(
        Weight a,
        Weight b
    ) const = 0;
};