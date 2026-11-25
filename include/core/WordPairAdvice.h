#pragma once

#include "core/Types.h"

enum class AdviceType {
    Eq,
    Leq,
    Geq
};

struct WordPairAdvice {
    Word u;
    Word v;
    AdviceType type = AdviceType::Eq;
};
