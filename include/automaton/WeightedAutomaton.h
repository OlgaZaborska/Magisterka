
#pragma once

#include "core/Types.h"
#include "algebra/SumAlgebra.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Transition {
    StateId target;
    Weight weight;
};

struct State {
    Weight finalWeight;

    std::unordered_map<Symbol, Transition> transitions;
};

class WeightedAutomaton {
private:
    SumAlgebra algebra_;

    std::unordered_set<Symbol> alphabet_;
    std::vector<State> states_;

    std::optional<StateId> initialState_;

    void validateState(StateId state) const;
    void validateSymbol(Symbol symbol) const;

public:
    explicit WeightedAutomaton(
        const std::vector<Symbol>& alphabet
    );

    StateId addState(Weight finalWeight);

    void setInitialState(StateId state);

    void addTransition(
        StateId source,
        Symbol symbol,
        StateId target,
        Weight weight
    );

    Weight evaluate(const Word& word) const;

    StateId nextState(
        StateId state,
        Symbol symbol
    ) const;

    Weight transitionWeight(
        StateId state,
        Symbol symbol
    ) const;

    Weight finalWeight(StateId state) const;

    StateId initialState() const;
    std::size_t numberOfStates() const;

    bool hasTransition(
        StateId state,
        Symbol symbol
    ) const;
};
