#include "algebra/SumAlgebra.h"
#include "learning/WeightedTTT.h"
#include "oracle/AutomatonOracle.h"
#include "oracle/SumEquivalenceChecker.h"

#include <cassert>
#include <iostream>

WeightedAutomaton makeTarget(const SumAlgebra& algebra) {
    WeightedAutomaton target({'a'}, algebra);
    StateId q0 = target.addState(0);
    StateId q1 = target.addState(0);

    target.setInitialState(q0);
    target.addTransition(q0, 'a', q1, 1);
    target.addTransition(q1, 'a', q0, 2);

    return target;
}

void testLearnsTargetAutomaton() {
    SumAlgebra algebra;
    WeightedAutomaton target = makeTarget(algebra);
    AutomatonOracle membershipOracle(target);
    SumEquivalenceChecker equivalenceOracle(target, {'a'});
    WeightedTTT learner(
        membershipOracle,
        equivalenceOracle,
        algebra,
        {'a'}
    );

    WeightedAutomaton hypothesis = learner.learn();

    assert(!equivalenceOracle.findCounterexample(hypothesis).has_value());
    assert(hypothesis.evaluate(Word{}) == target.evaluate(Word{}));
    assert(hypothesis.evaluate(Word{'a'}) == target.evaluate(Word{'a'}));
    assert(hypothesis.evaluate(Word{'a', 'a'}) ==
           target.evaluate(Word{'a', 'a'}));
}

int main() {
    testLearnsTargetAutomaton();
    std::cout << "All WeightedTTT tests passed!\n";
    return 0;
}
