
#include "automaton/WeightedAutomaton.h"

#include <stdexcept>

WeightedAutomaton::WeightedAutomaton( const std::vector<Symbol>& alphabet)
{
    for (Symbol symbol : alphabet) 
    {
        auto [it, inserted] = alphabet_.insert(symbol);

        if (!inserted) {
            throw std::invalid_argument(
                "Alphabet contains duplicate symbols"
            );
        }
    }
}

void WeightedAutomaton::validateState(StateId state) const 
{
    if (state >= states_.size()) {
        throw std::out_of_range(
            "Invalid state identifier"
        );
    }
}

void WeightedAutomaton::validateSymbol(Symbol symbol) const 
{
    if (!alphabet_.contains(symbol)) {
        throw std::invalid_argument(
            "Symbol does not belong to the alphabet"
        );
    }
}

StateId WeightedAutomaton::addState( Weight finalWeight)
{
    StateId id = states_.size();

    states_.push_back(State{finalWeight, {}});

    return id;
}

void WeightedAutomaton::setInitialState(StateId state)
{
    validateState(state);
    initialState_ = state;
}

void WeightedAutomaton::addTransition(StateId source, Symbol symbol, StateId target, Weight weight)
{
    validateState(source);
    validateState(target);
    validateSymbol(symbol);

    auto [it, inserted] =
        states_[source].transitions.emplace(
            symbol,
            Transition{target, weight}
        );

    if (!inserted) {
        throw std::invalid_argument(
            "Transition for this symbol already exists"
        );
    }
}

bool WeightedAutomaton::hasTransition(StateId state, Symbol symbol) const 
{
    validateState(state);
    validateSymbol(symbol);

    return states_[state].transitions.contains(symbol);
}

StateId WeightedAutomaton::nextState(StateId state, Symbol symbol) const
{
    validateState(state);
    validateSymbol(symbol);

    auto it = states_[state].transitions.find(symbol);

    if (it == states_[state].transitions.end()) {
        throw std::runtime_error(
            "Transition does not exist"
        );
    }

    return it->second.target;
}

Weight WeightedAutomaton::transitionWeight(StateId state, Symbol symbol) const 
{
    validateState(state);
    validateSymbol(symbol);

    auto it = states_[state].transitions.find(symbol);

    if (it == states_[state].transitions.end()) {
        throw std::runtime_error(
            "Transition does not exist"
        );
    }

    return it->second.weight;
}

Weight WeightedAutomaton::finalWeight(StateId state) const 
{
    validateState(state);
    return states_[state].finalWeight;
}

StateId WeightedAutomaton::initialState() const 
{
    if (!initialState_.has_value()) {
        throw std::runtime_error(
            "Initial state has not been set"
        );
    }

    return initialState_.value();
}

std::size_t WeightedAutomaton::numberOfStates() const {
    return states_.size();
}

Weight WeightedAutomaton::evaluate(const Word& word) const 
{
    if (!initialState_.has_value()) {
        throw std::runtime_error(
            "Initial state has not been set"
        );
    }

    StateId currentState = initialState_.value();

    Weight accumulatedWeight = algebra_.identity();

    for (Symbol symbol : word) {
        validateSymbol(symbol);

        auto it = states_[currentState].transitions.find(symbol);

        if (it == states_[currentState].transitions.end()) {
            throw std::runtime_error(
                "Cannot evaluate word: missing transition"
            );
        }

        accumulatedWeight = algebra_.combine(
            accumulatedWeight,
            it->second.weight
        );

        currentState = it->second.target;
    }

    return algebra_.combine(
        accumulatedWeight,
        states_[currentState].finalWeight
    );
}
