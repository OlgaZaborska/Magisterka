#pragma once

#include "core/Types.h"

#include <optional>
#include <unordered_map>
#include <cstddef>
#include <functional>

class QueryCache {
public:
    std::optional<Weight> find(const Word& word) const {
        auto it = cache_.find(word);

        if (it == cache_.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    void insert(const Word& word, Weight result) {
        cache_[word] = result;
    }

    void clear() {
        cache_.clear();
    }

    std::size_t size() const {
        return cache_.size();
    }

    bool empty() const {
        return cache_.empty();
    }

private:
    struct WordHash {
        std::size_t operator()(const Word& word) const {
            std::size_t seed = 0;

            for (Symbol symbol : word) {
                seed ^= std::hash<Symbol>{}(symbol)
                    + 0x9e3779b9
                    + (seed << 6)
                    + (seed >> 2);
            }

            return seed;
        }
    };

    std::unordered_map<Word, Weight, WordHash> cache_;
};