
#include "learning/DiscriminationTree.h"
#include "learning/ResidualFunction.h"
#include "oracle/MembershipOracle.h"
#include "algebra/SumAlgebra.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

// Oracle with residuum, that depends on the prefix and suffix.
// For a word w, it returns |w|^2.
class TestOracle : public MembershipOracle {
public:
    Weight query(const Word& word) override {
        Weight length = static_cast<Weight>(word.size());
        Weight suffixContribution =
            word.size() >= 2 &&
            word[word.size() - 2] == 'a' &&
            word[word.size() - 1] == 'a'
                ? 1
                : 0;

        return length * length + suffixContribution;
    }
};


void testTreeInitialization() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    assert(tree.sift(Word{}) == AccessSequence{});

    std::cout << "[OK] Tree initialization\n";
}



void testSiftSingleLeaf() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    assert(tree.sift(Word{'a'}) == AccessSequence{});

    std::cout << "[OK] Sift single leaf\n";
}


void testSplitLeaf() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    // D(epsilon, "a") = f("a") - f(epsilon) = 1
    // D("aa", "a") = f("aaa") - f("aa") = 5
    tree.split(
        AccessSequence{},
        AccessSequence(Word{'a', 'a'}),
        Word{'a'}
    );


    assert((tree.sift(Word{}) == AccessSequence{}));
    assert((
        tree.sift(Word{'a', 'a'}) ==
        AccessSequence(Word{'a', 'a'})
    ));

    std::cout << "[OK] Split leaf\n";
}


void testMultipleSplits() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    tree.split(
        AccessSequence{},
        AccessSequence(Word{'a', 'a'}),
        Word{'a'}
    );

    // Both access sequences follow the root branch with result 5.
    // The second discriminator distinguishes "aa" from "ab".
    tree.split(
        AccessSequence(Word{'a', 'a'}),
        AccessSequence(Word{'a', 'b'}),
        Word{'a', 'a'}
    );

    assert((tree.sift(Word{}) == AccessSequence{}));
    assert((
        tree.sift(Word{'a', 'a'}) ==
        AccessSequence(Word{'a', 'a'})
    ));
    assert((
        tree.sift(Word{'a', 'b'}) ==
        AccessSequence(Word{'a', 'b'})
    ));

    std::cout << "[OK] Multiple splits\n";
}


void testSplitWithoutDistinction() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    bool exceptionThrown = false;

    try {
        // D(epsilon, "a") = 1
        // D("a", "a") = 3
        tree.split(
            AccessSequence{},
            AccessSequence(Word{'a'}),
            Word{'a'}
        );

    } catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(!exceptionThrown);

    exceptionThrown = false;

    try {
        tree.split(
            AccessSequence{},
            AccessSequence(Word{'a'}),
            Word{}
        );

    } catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "[OK] Split without distinction\n";
}


void testSplitNonExistingLeaf() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    bool exceptionThrown = false;

    try {
        tree.split(
            AccessSequence(Word{'x'}),
            AccessSequence(Word{'a'}),
            Word{'b'}
        );

    } catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    std::cout << "[OK] Split non-existing leaf\n";
}


void testAccessSequencesSingleLeaf() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    auto sequences = tree.accessSequences();

    assert(sequences.size() == 1);
    assert(sequences[0] == AccessSequence{});
}


void testAccessSequencesAfterSplit() {
    TestOracle oracle;
    SumAlgebra algebra;
    ResidualFunction residual(oracle, algebra);

    DiscriminationTree tree(AccessSequence{}, residual);

    AccessSequence empty;
    AccessSequence aa(Word{'a', 'a'});

    tree.split(empty, aa, Word{'a'});

    auto sequences = tree.accessSequences();

    assert(sequences.size() == 2);

    bool foundEmpty = false;
    bool foundAA = false;

    for (const auto& sequence : sequences) {
        if (sequence == empty) foundEmpty = true;
        if (sequence == aa) foundAA = true;
    }

    assert(foundEmpty);
    assert(foundAA);
}


int main() {
    testTreeInitialization();
    testSiftSingleLeaf();
    testSplitLeaf();
    testMultipleSplits();
    testSplitWithoutDistinction();
    testSplitNonExistingLeaf();

    std::cout << "\nAll DiscriminationTree tests passed!\n";

    return 0;
}
