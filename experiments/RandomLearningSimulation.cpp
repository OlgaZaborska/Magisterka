#include "algebra/SumAlgebra.h"
#include "automaton/RandomAutomatonGenerator.h"
#include "learning/WeightedTTT.h"
#include "oracle/AutomatonOracle.h"
#include "oracle/SumEquivalenceChecker.h"

#include <charconv>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

bool parseUnsigned(
    std::string_view text,
    std::uint64_t& value
) {
    const auto [end, error] = std::from_chars(
        text.data(),
        text.data() + text.size(),
        value
    );
    return error == std::errc{} && end == text.data() + text.size();
}

}

int main(int argc, char* argv[]) {
    if (argc > 3) {
        std::cerr
            << "Usage: random_learning_simulation "
            << "[number_of_states] [seed]\n";
        return 1;
    }

    std::uint64_t requestedStates = 5;
    std::uint64_t seed = std::random_device{}();

    if (argc >= 2 && !parseUnsigned(argv[1], requestedStates)) {
        std::cerr << "number_of_states must be a positive integer\n";
        return 1;
    }

    if (argc == 3 && !parseUnsigned(argv[2], seed)) {
        std::cerr << "seed must be an unsigned integer\n";
        return 1;
    }

    if (requestedStates == 0 ||
        requestedStates > std::numeric_limits<std::size_t>::max() ||
        seed > std::mt19937::max()) {
        std::cerr << "number_of_states or seed is out of range\n";
        return 1;
    }

    const std::size_t numberOfStates =
        static_cast<std::size_t>(requestedStates);
    std::mt19937 random(static_cast<std::mt19937::result_type>(seed));
    SumAlgebra algebra;
    const std::vector<Symbol> alphabet(
        std::begin(RandomAutomatonAlphabet),
        std::end(RandomAutomatonAlphabet)
    );

    WeightedAutomaton target = generateRandomAutomaton(
        numberOfStates,
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
    const bool equivalent =
        !equivalenceOracle.findCounterexample(hypothesis).has_value();

    std::cout << "Seed: " << seed << '\n'
              << "Alphabet: a, b, c, d\n"
              << "Target states: " << target.numberOfStates() << '\n'
              << "Learned states: " << hypothesis.numberOfStates() << '\n'
              << "Equivalent: " << (equivalent ? "yes" : "no") << '\n';

    return equivalent ? 0 : 1;
}
