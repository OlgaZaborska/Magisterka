
#include "oracle/SumEquivalenceChecker.h"
#include "algebra/SumAlgebra.h"

#include <cassert>
#include <iostream>

void testEquivalentAutomata() {
    SumAlgebra algebra;

    WeightedAutomaton target({'a'}, algebra);
    WeightedAutomaton hypothesis({'a'}, algebra);

    StateId targetState = target.addState(0);
    StateId hypothesisState = hypothesis.addState(0);

    target.setInitialState(targetState);
    hypothesis.setInitialState(hypothesisState);

    target.addTransition(targetState, 'a', targetState, 1);
    hypothesis.addTransition(hypothesisState, 'a', hypothesisState, 1);

    SumEquivalenceChecker checker(target, {'a'});

    auto result = checker.findCounterexample(hypothesis);

    assert(!result.has_value());
}

void testDifferentTransitionWeights() {
    SumAlgebra algebra;

    WeightedAutomaton target({'a'}, algebra);
    WeightedAutomaton hypothesis({'a'}, algebra);

    StateId targetState = target.addState(0);
    StateId hypothesisState = hypothesis.addState(0);

    target.setInitialState(targetState);
    hypothesis.setInitialState(hypothesisState);

    target.addTransition(targetState, 'a', targetState, 1);
    hypothesis.addTransition(hypothesisState, 'a', hypothesisState, 2);

    SumEquivalenceChecker checker(target, {'a'});

    auto result = checker.findCounterexample(hypothesis);

    assert(result.has_value());
    assert(result.value() == Word{'a'});
}

int main() {
    testEquivalentAutomata();
    testDifferentTransitionWeights();

    std::cout << "All SumEquivalenceChecker tests passed!\n";
    return 0;
}
