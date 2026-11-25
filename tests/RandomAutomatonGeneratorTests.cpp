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

void testGeneratorRejectsUnimplementedAdviceTypes() {
    SumAlgebra algebra;
    std::mt19937 random(5);
    const WordPairAdvice geqAdvice{
        Word{'a'},
        Word{'b'},
        AdviceType::Geq
    };

    bool unsupportedTypeRejected = false;
    try {
        generateRandomAutomatonWithAdvice(
            2,
            geqAdvice,
            algebra,
            random
        );
    } catch (const std::invalid_argument&) {
        unsupportedTypeRejected = true;
    }
    assert(unsupportedTypeRejected);
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

void testGeneratedAutomatonSatisfiesWordPairAdvice() {
    SumAlgebra algebra;
    std::mt19937 random(73);
    const WordPairAdvice advice{
        Word{'a', 'b'},
        Word{'c', 'd', 'd'}
    };
    WeightedAutomaton automaton = generateRandomAutomatonWithAdvice(
        5,
        advice,
        algebra,
        random
    );

    for (StateId source = 0; source < automaton.numberOfStates(); ++source) {
        StateId afterU = source;
        StateId afterV = source;
        Weight weightU = 0;
        Weight weightV = 0;

        for (Symbol symbol : advice.u) {
            weightU += automaton.transitionWeight(afterU, symbol);
            afterU = automaton.nextState(afterU, symbol);
        }
        for (Symbol symbol : advice.v) {
            weightV += automaton.transitionWeight(afterV, symbol);
            afterV = automaton.nextState(afterV, symbol);
        }

        assert(afterU == afterV);
        assert(weightU == weightV);
    }

    const Word prefix{'b', 'a'};
    const Word suffix{'d', 'c'};
    Word prefixU = prefix;
    prefixU.insert(prefixU.end(), advice.u.begin(), advice.u.end());
    prefixU.insert(prefixU.end(), suffix.begin(), suffix.end());
    Word prefixV = prefix;
    prefixV.insert(prefixV.end(), advice.v.begin(), advice.v.end());
    prefixV.insert(prefixV.end(), suffix.begin(), suffix.end());
    assert(automaton.evaluate(prefixU) == automaton.evaluate(prefixV));
}

void testGeneratorSupportsEqualAndEmptyAdviceWords() {
    SumAlgebra algebra;
    std::mt19937 random(17);
    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );
    const std::vector<WordPairAdvice> advices{
        {Word{'a', 'b'}, Word{'b', 'a'}},
        {Word{}, Word{'a', 'b', 'c', 'd'}},
        {Word{}, Word{}}
    };

    for (const WordPairAdvice& advice : advices) {
        WeightedAutomaton automaton = generateRandomAutomatonWithAdvice(
            4,
            advice,
            algebra,
            random
        );

        for (StateId source = 0;
             source < automaton.numberOfStates();
             ++source) {
            StateId afterU = source;
            StateId afterV = source;
            Weight weightU = 0;
            Weight weightV = 0;
            for (Symbol symbol : advice.u) {
                weightU += automaton.transitionWeight(afterU, symbol);
                afterU = automaton.nextState(afterU, symbol);
            }
            for (Symbol symbol : advice.v) {
                weightV += automaton.transitionWeight(afterV, symbol);
                afterV = automaton.nextState(afterV, symbol);
            }
            assert(afterU == afterV);
            assert(weightU == weightV);
        }

        assert(automaton.evaluate(advice.u) ==
               automaton.evaluate(advice.v));
    }

    bool invalidSymbolRejected = false;
    try {
        generateRandomAutomatonWithAdvice(
            2,
            WordPairAdvice{Word{'x'}, Word{'a'}},
            algebra,
            random
        );
    } catch (const std::invalid_argument&) {
        invalidSymbolRejected = true;
    }
    assert(invalidSymbolRejected);
}

void testLearnsAutomatonWithAdvice() {
    SumAlgebra algebra;
    std::mt19937 random(123);
    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );
    const WordPairAdvice advice{
        Word{'a', 'b'},
        Word{'c', 'd', 'd'}
    };
    WeightedAutomaton target = generateRandomAutomatonWithAdvice(
        5,
        advice,
        algebra,
        random
    );
    AutomatonOracle membershipOracle(target);
    SumEquivalenceChecker equivalenceOracle(target, alphabet);
    WeightedTTT learner(
        membershipOracle,
        equivalenceOracle,
        algebra,
        alphabet,
        advice
    );

    WeightedAutomaton hypothesis = learner.learn();

    assert(!equivalenceOracle.findCounterexample(hypothesis).has_value());
}

void testLearnsAdvisedTargetWithoutAdvice() {
    SumAlgebra algebra;
    std::mt19937 random(123);
    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );
    const WordPairAdvice advice{
        Word{'a', 'b'},
        Word{'c', 'd', 'd'}
    };
    WeightedAutomaton target = generateRandomAutomatonWithAdvice(
        5,
        advice,
        algebra,
        random
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

    assert(!equivalenceOracle.findCounterexample(hypothesis).has_value());
}

void testNonEqAdviceSkipsEqSpecificHypothesisHandling() {
    SumAlgebra algebra;
    std::mt19937 random(31);
    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );
    WeightedAutomaton target = generateRandomAutomaton(3, algebra, random);
    AutomatonOracle membershipOracle(target);
    SumEquivalenceChecker equivalenceOracle(target, alphabet);
    const WordPairAdvice leqAdvice{
        Word{'a'},
        Word{'b'},
        AdviceType::Leq
    };
    WeightedTTT learner(
        membershipOracle,
        equivalenceOracle,
        algebra,
        alphabet,
        leqAdvice
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
    testGeneratorRejectsUnimplementedAdviceTypes();
    testLearnsGeneratedAutomaton();
    testGeneratedAutomatonSatisfiesWordPairAdvice();
    testGeneratorSupportsEqualAndEmptyAdviceWords();
    testLearnsAutomatonWithAdvice();
    testLearnsAdvisedTargetWithoutAdvice();
    testNonEqAdviceSkipsEqSpecificHypothesisHandling();
    testEmptyWordHasZeroValue();
    std::cout << "All random automaton generator tests passed!\n";
    return 0;
}
