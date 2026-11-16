
#include "learning/AccessSequence.h"

#include <cassert>
#include <iostream>

void testEmptySequence() {
    AccessSequence sequence;

    assert(sequence.empty());
    assert(sequence.length() == 0);
    assert(sequence.word().empty());
}

void testConstruction() {
    AccessSequence sequence(Word{'a', 'b'});

    assert(sequence.length() == 2);
    assert((sequence.word() == Word{'a', 'b'}));
}

void testExtension() {
    AccessSequence original(Word{'a', 'b'});
    AccessSequence extended = original.extend('c');

    assert((original.word() == Word{'a', 'b'}));
    assert((extended.word() == Word{'a', 'b', 'c'}));
}

void testEquality() {
    AccessSequence first(Word{'a', 'b'});
    AccessSequence second(Word{'a', 'b'});
    AccessSequence third(Word{'a', 'c'});

    assert(first == second);
    assert(first != third);
}

int main() {
    testEmptySequence();
    testConstruction();
    testExtension();
    testEquality();

    std::cout << "All AccessSequence tests passed!\n";
    return 0;
}
