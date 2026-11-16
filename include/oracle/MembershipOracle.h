#pragma once

#include "core/Types.h"

class MembershipOracle {
public:
    virtual ~MembershipOracle() = default;

    virtual Weight query(
        const Word& word
    ) = 0;
};