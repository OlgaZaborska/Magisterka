#include "algebra/SumAlgebra.h"
#include "automaton/RandomAutomatonGenerator.h"
#include "learning/WeightedTTT.h"
#include "oracle/AutomatonOracle.h"
#include "oracle/SumEquivalenceChecker.h"

#include <cassert>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <vector>

void testCreatesCompleteFourSymbolAutomaton() {
    SumAlgebra algebra;
    std::mt19937 random(17);
    WeightedAutomaton automaton =
        generateRandomAutomaton(3, algebra, random, -2, 4);

    assert(automaton.numberOfStates() == 3);
    assert(automaton.initialState() == 0);

    for (StateId state = 0; state < automaton.numberOfStates(); ++state) {
        assert(automaton.finalWeight(state) == 0);

        for (Symbol symbol : RandomAutomatonAlphabet) {
            assert(automaton.hasTransition(state, symbol));
            assert(automaton.nextState(state, symbol) < 3);
            assert(automaton.transitionWeight(state, symbol) >= -2);
            assert(automaton.transitionWeight(state, symbol) <= 4);
        }
    }
}

void testGenerationIsReproducibleWithSameSeed() {
    SumAlgebra algebra;
    std::mt19937 firstRandom(123);
    std::mt19937 secondRandom(123);
    WeightedAutomaton first =
        generateRandomAutomaton(4, algebra, firstRandom);
    WeightedAutomaton second =
        generateRandomAutomaton(4, algebra, secondRandom);

    for (StateId state = 0; state < 4; ++state) {
        assert(first.finalWeight(state) == second.finalWeight(state));
        for (Symbol symbol : RandomAutomatonAlphabet) {
            assert(first.nextState(state, symbol) ==
                   second.nextState(state, symbol));
            assert(first.transitionWeight(state, symbol) ==
                   second.transitionWeight(state, symbol));
        }
    }
}

void testRejectsInvalidConfiguration() {
    SumAlgebra algebra;
    std::mt19937 random(1);

    bool zeroStatesRejected = false;
    try {
        generateRandomAutomaton(0, algebra, random);
    } catch (const std::invalid_argument&) {
        zeroStatesRejected = true;
    }
    assert(zeroStatesRejected);

    bool invalidRangeRejected = false;
    try {
        generateRandomAutomaton(1, algebra, random, 2, 1);
    } catch (const std::invalid_argument&) {
        invalidRangeRejected = true;
    }
    assert(invalidRangeRejected);
}

void testLearnsGeneratedAutomaton() {
    SumAlgebra algebra;
    std::mt19937 random(31);
    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );
    WeightedAutomaton target = generateRandomAutomaton(3, algebra, random);
    AutomatonOracle membershipOracle(target);
    SumEquivalenceChecker equivalenceOracle(target, alphabet);
    WeightedTTT learner(
        membershipOracle,
        equivalenceOracle,
        algebra,
        alphabet
    );

    WeightedAutomaton hypothesis = learner.learn();

    assert(!equivalenceOracle.findCounterexample(hypothesis).has_value());
}

void testEmptyWordHasZeroValue() {
    SumAlgebra algebra;
    std::mt19937 random(7);
    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );
    WeightedAutomaton target = generateRandomAutomaton(
        1,
        algebra,
        random,
        3,
        3
    );
    AutomatonOracle membershipOracle(target);
    SumEquivalenceChecker equivalenceOracle(target, alphabet);
    WeightedTTT learner(
        membershipOracle,
        equivalenceOracle,
        algebra,
        alphabet
    );

    WeightedAutomaton hypothesis = learner.learn();

    assert(target.evaluate(Word{}) == 0);
    assert(hypothesis.evaluate(Word{}) == 0);
    assert(target.evaluate(Word{'a'}) == 3);
    assert(hypothesis.evaluate(Word{'a'}) == 3);
    assert(!equivalenceOracle.findCounterexample(hypothesis).has_value());
}

int main() {
    testCreatesCompleteFourSymbolAutomaton();
    testGenerationIsReproducibleWithSameSeed();
    testRejectsInvalidConfiguration();
    testLearnsGeneratedAutomaton();
    testEmptyWordHasZeroValue();
    std::cout << "All random automaton generator tests passed!\n";
    return 0;
}
