
#include "learning/HypothesisBuilder.h"
#include "oracle/MembershipOracle.h"
#include "algebra/SumAlgebra.h"

#include <cassert>
#include <iostream>

class TestOracle : public MembershipOracle {
public:
    Weight query(const Word& word) override {
        return static_cast<Weight>(word.size() % 2);
    }
};

void testSingleStateHypothesis() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    HypothesisBuilder builder(residual, tree);

    WeightedAutomaton hypothesis = builder.build({'a'});

    assert(hypothesis.numberOfStates() == 1);
    assert(hypothesis.initialState() == 0);

    assert(hypothesis.hasTransition(0, 'a'));
    assert(hypothesis.nextState(0, 'a') == 0);
    assert(hypothesis.transitionWeight(0, 'a') == 1);
    assert(hypothesis.finalWeight(0) == 0);
}

void testTwoStateHypothesis() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    AccessSequence empty;
    AccessSequence a(Word{'a'});

    tree.split(empty, a, Word{'a'});

    HypothesisBuilder builder(residual, tree);

    WeightedAutomaton hypothesis = builder.build({'a'});

    assert(hypothesis.numberOfStates() == 2);
    assert(hypothesis.initialState() == 0);

    assert(hypothesis.hasTransition(0, 'a'));
    assert(hypothesis.hasTransition(1, 'a'));

    assert(hypothesis.nextState(0, 'a') == 1);
    assert(hypothesis.nextState(1, 'a') == 0);

    assert(hypothesis.transitionWeight(0, 'a') == 1);
    assert(hypothesis.transitionWeight(1, 'a') == -1);

    assert(hypothesis.finalWeight(0) == 0);
    assert(hypothesis.finalWeight(1) == 0);
}

int main() {
    testSingleStateHypothesis();
    testTwoStateHypothesis();

    std::cout << "All HypothesisBuilder tests passed!\n";
    return 0;
}
