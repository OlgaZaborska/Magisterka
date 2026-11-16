#pragma once

#include "oracle/MembershipOracle.h"
#include "utils/QueryCache.h"

class CachedMembershipOracle : public MembershipOracle {
public:
    explicit CachedMembershipOracle(MembershipOracle& oracle)
        : oracle_(oracle) {
    }

    Weight query(const Word& word) override {
        auto cachedResult = cache_.find(word);

        if (cachedResult.has_value()) {
            return cachedResult.value();
        }

        Weight result = oracle_.query(word);

        cache_.insert(word, result);

        return result;
    }

    void clearCache() {
        cache_.clear();
    }

    std::size_t cachedQueries() const {
        return cache_.size();
    }

private:
    MembershipOracle& oracle_;
    QueryCache cache_;
};