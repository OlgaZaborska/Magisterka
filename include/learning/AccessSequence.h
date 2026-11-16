
#pragma once

#include "core/Types.h"

class AccessSequence {
private:
    Word word_;

public:
    AccessSequence() = default;

    explicit AccessSequence(const Word& word)
        : word_(word) {}

    const Word& word() const {
        return word_;
    }

    AccessSequence extend(Symbol symbol) const {
        Word extended = word_;
        extended.push_back(symbol);
        return AccessSequence(extended);
    }

    bool operator==(const AccessSequence& other) const {
        return word_ == other.word_;
    }

    bool operator!=(const AccessSequence& other) const {
        return !(*this == other);
    }

    std::size_t length() const {
        return word_.size();
    }

    bool empty() const {
        return word_.empty();
    }
};
