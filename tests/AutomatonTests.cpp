
#include "automaton/WeightedAutomaton.h"
#include "algebra/SumAlgebra.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

void testEmptyWord() {
    WeightedAutomaton automaton({'a', 'b'});

    StateId q0 = automaton.addState(5);
    automaton.setInitialState(q0);

    assert(automaton.evaluate({}) == 5);

    std::cout << "testEmptyWord: OK\n";
}

void testSingleSymbol() {
    WeightedAutomaton automaton({'a', 'b'});

    StateId q0 = automaton.addState(5);
    StateId q1 = automaton.addState(7);

    automaton.setInitialState(q0);

    automaton.addTransition(q0, 'a', q1, 3);

    // f(a) = 3 + 7 = 10
    assert(automaton.evaluate({'a'}) == 10);

    std::cout << "testSingleSymbol: OK\n";
}

void testMultipleSymbols() {
    WeightedAutomaton automaton({'a', 'b'});

    StateId q0 = automaton.addState(5);
    StateId q1 = automaton.addState(7);
    StateId q2 = automaton.addState(2);

    automaton.setInitialState(q0);

    automaton.addTransition(q0, 'a', q1, 3);
    automaton.addTransition(q1, 'b', q2, 4);
    automaton.addTransition(q2, 'b', q0, 2);
    automaton.addTransition(q0, 'b', q0, 1);

    // f(ab) = 3 + 4 + 2 = 9
    assert(automaton.evaluate({'a', 'b'}) == 9);

    // f(abb) = 3 + 4 + 2 + 5 = 14
    assert(automaton.evaluate({'a', 'b', 'b'}) == 14);

    // f(b) = 1 + 5 = 6
    assert(automaton.evaluate({'b'}) == 6);

    std::cout << "testMultipleSymbols: OK\n";
}

void testNegativeWeights() {
    WeightedAutomaton automaton({'a', 'b'});

    StateId q0 = automaton.addState(-5);
    StateId q1 = automaton.addState(2);

    automaton.setInitialState(q0);

    automaton.addTransition(q0, 'a', q1, -3);

    // f(a) = -3 + 2 = -1
    assert(automaton.evaluate({'a'}) == -1);

    std::cout << "testNegativeWeights: OK\n";
}

void testMissingTransition() {
    WeightedAutomaton automaton({'a', 'b'});

    StateId q0 = automaton.addState(0);
    automaton.setInitialState(q0);

    bool exceptionThrown = false;

    try {
        automaton.evaluate({'a'});
    }
    catch (const std::runtime_error&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "testMissingTransition: OK\n";
}

void testInvalidSymbol() {
    WeightedAutomaton automaton({'a', 'b'});

    StateId q0 = automaton.addState(0);
    automaton.setInitialState(q0);

    bool exceptionThrown = false;

    try {
        automaton.evaluate({'x'});
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "testInvalidSymbol: OK\n";
}

void testDuplicateTransition() {
    WeightedAutomaton automaton({'a'});

    StateId q0 = automaton.addState(0);
    StateId q1 = automaton.addState(0);

    automaton.setInitialState(q0);

    automaton.addTransition(q0, 'a', q1, 1);

    bool exceptionThrown = false;

    try {
        automaton.addTransition(q0, 'a', q0, 2);
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "testDuplicateTransition: OK\n";
}

void testAlgebra() {
    SumAlgebra algebra;

    assert(algebra.identity() == 0);
    assert(algebra.combine(3, 4) == 7);
    assert(algebra.combine(-3, 4) == 1);
    assert(algebra.equal(5, 5));
    assert(!algebra.equal(5, 6));

    std::cout << "testAlgebra: OK\n";
}

int main() {
    testAlgebra();
    testEmptyWord();
    testSingleSymbol();
    testMultipleSymbols();
    testNegativeWeights();
    testMissingTransition();
    testInvalidSymbol();
    testDuplicateTransition();

    std::cout << "\nAll tests passed!\n";

    return 0;
}
