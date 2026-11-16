#include "oracle/AutomatonOracle.h"
#include "oracle/CachedMembershipOracle.h"
#include "automaton/WeightedAutomaton.h"
#include "learning/ResidualFunction.h"
#include "algebra/SumAlgebra.h"

#include <cassert>
#include <iostream>

class CountingOracle : public MembershipOracle {
public:
    Weight query(const Word& word) override {
        ++queryCount;

        Weight result = 0;

        for (Symbol symbol : word) {
            result += static_cast<Weight>(symbol);
        }

        return result;
    }

    std::size_t getQueryCount() const {
        return queryCount;
    }

private:
    std::size_t queryCount = 0;
};


void testAutomatonOracle() {
    SumAlgebra algebra;
    WeightedAutomaton automaton({'a', 'b'}, algebra);

    StateId q0 = automaton.addState(5);
    StateId q1 = automaton.addState(7);
    StateId q2 = automaton.addState(2);

    automaton.setInitialState(q0);

    automaton.addTransition(q0, 'a', q1, 3);
    automaton.addTransition(q1, 'b', q2, 4);

    AutomatonOracle oracle(automaton);

    assert(oracle.query(Word{}) == 5);
    assert(oracle.query(Word{'a'}) == 10);
    assert(oracle.query(Word{'a', 'b'}) == 9);

    std::cout << "[OK] AutomatonOracle\n";
}


void testCacheStoresResults() {
    CountingOracle baseOracle;
    CachedMembershipOracle oracle(baseOracle);

    Word word{'a', 'b'};

    Weight first = oracle.query(word);
    Weight second = oracle.query(word);

    assert(first == second);
    assert(oracle.cachedQueries() == 1);

    assert(baseOracle.getQueryCount() == 1);

    std::cout << "[OK] Cache stores results\n";
}


void testCacheDifferentWords() {
    CountingOracle baseOracle;
    CachedMembershipOracle oracle(baseOracle);

    Weight resultA = oracle.query(Word{'a'});
    Weight resultB = oracle.query(Word{'b'});

    assert(resultA != resultB);
    assert(oracle.cachedQueries() == 2);
    assert(baseOracle.getQueryCount() == 2);

    std::cout << "[OK] Cache handles different words\n";
}


void testCacheEmptyWord() {
    CountingOracle baseOracle;
    CachedMembershipOracle oracle(baseOracle);

    Weight first = oracle.query(Word{});
    Weight second = oracle.query(Word{});

    assert(first == 0);
    assert(second == 0);

    assert(oracle.cachedQueries() == 1);
    assert(baseOracle.getQueryCount() == 1);

    std::cout << "[OK] Cache handles empty word\n";
}


void testCacheClear() {
    CountingOracle baseOracle;
    CachedMembershipOracle oracle(baseOracle);

    Word word{'a'};

    oracle.query(word);

    assert(oracle.cachedQueries() == 1);
    assert(baseOracle.getQueryCount() == 1);

    oracle.clearCache();

    assert(oracle.cachedQueries() == 0);

    oracle.query(word);

    assert(baseOracle.getQueryCount() == 2);

    std::cout << "[OK] Cache clear\n";
}


void testResidualFunction() {
    CountingOracle oracle;
    SumAlgebra algebra;

    ResidualFunction residual(oracle, algebra);

    // f("ab") - f("a") = ('a' + 'b') - 'a' = 'b'
    Weight result = residual.compute(
        Word{'a'},
        Word{'b'}
    );

    assert(result == static_cast<Weight>('b'));

    std::cout << "[OK] ResidualFunction computes residual\n";
}


void testResidualFunctionEmptyPrefix() {
    CountingOracle oracle;
    SumAlgebra algebra;

    ResidualFunction residual(oracle, algebra);

    // f("a") - f("") = 'a' - 0 = 'a'
    Weight result = residual.compute(
        Word{},
        Word{'a'}
    );

    assert(result == static_cast<Weight>('a'));

    std::cout << "[OK] ResidualFunction handles empty prefix\n";
}


void testResidualFunctionEmptySuffix() {
    CountingOracle oracle;
    SumAlgebra algebra;

    ResidualFunction residual(oracle, algebra);

    // f("a") - f("a") = 0
    Weight result = residual.compute(
        Word{'a'},
        Word{}
    );

    assert(result == 0);

    std::cout << "[OK] ResidualFunction handles empty suffix\n";
}


int main() {
    testAutomatonOracle();
    testCacheStoresResults();
    testCacheDifferentWords();
    testCacheEmptyWord();
    testCacheClear();

    testResidualFunction();
    testResidualFunctionEmptyPrefix();
    testResidualFunctionEmptySuffix();

    std::cout << "\nAll MembershipOracle tests passed!\n";

    return 0;
}