#include "algebra/SumAlgebra.h"
#include "automaton/RandomAutomatonGenerator.h"
#include "learning/WeightedTTT.h"
#include "oracle/AutomatonOracle.h"
#include "oracle/EquivalenceOracle.h"
#include "oracle/MembershipOracle.h"
#include "oracle/SumEquivalenceChecker.h"
#include "utils/QueryCache.h"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

class CountingMembershipOracle : public MembershipOracle {
public:
    explicit CountingMembershipOracle(
        MembershipOracle& oracle,
        std::optional<WordPairAdvice> advice = std::nullopt
    )
        : oracle_(oracle),
          advice_(std::move(advice)) {
    }

    Weight query(const Word& word) override {
        ++requestCount_;
        const bool usesEqAdvice =
            advice_.has_value() &&
            advice_->type == AdviceType::Eq;
        const Word queryWord = usesEqAdvice
            ? normalize(word)
            : word;

        if (usesEqAdvice) {
            const auto cachedResult = cache_.find(queryWord);
            if (cachedResult.has_value()) {
                return cachedResult.value();
            }
        }

        ++queryCount_;
        const Weight result = oracle_.query(queryWord);
        if (usesEqAdvice) {
            cache_.insert(queryWord, result);
        }
        return result;
    }

    std::size_t queryCount() const {
        return queryCount_;
    }

    std::size_t requestCount() const {
        return requestCount_;
    }

private:
    Word normalize(const Word& word) const {
        const Word* source = &advice_->u;
        const Word* replacement = &advice_->v;
        if (source->size() < replacement->size() ||
            (source->size() == replacement->size() &&
             *source < *replacement)) {
            std::swap(source, replacement);
        }

        if (*source == *replacement) {
            return word;
        }

        Word normalized = word;
        while (true) {
            const auto match = std::search(
                normalized.begin(),
                normalized.end(),
                source->begin(),
                source->end()
            );
            if (match == normalized.end()) {
                return normalized;
            }

            const std::size_t prefixLength =
                static_cast<std::size_t>(
                    std::distance(normalized.begin(), match)
                );
            Word replaced;
            replaced.reserve(
                normalized.size() - source->size() +
                replacement->size()
            );
            replaced.insert(
                replaced.end(),
                normalized.begin(),
                normalized.begin() + prefixLength
            );
            replaced.insert(
                replaced.end(),
                replacement->begin(),
                replacement->end()
            );
            replaced.insert(
                replaced.end(),
                match + source->size(),
                normalized.end()
            );
            normalized = std::move(replaced);
        }
    }

    MembershipOracle& oracle_;
    std::optional<WordPairAdvice> advice_;
    QueryCache cache_;
    std::size_t queryCount_ = 0;
    std::size_t requestCount_ = 0;
};

class CountingEquivalenceOracle : public EquivalenceOracle {
public:
    explicit CountingEquivalenceOracle(EquivalenceOracle& oracle)
        : oracle_(oracle) {
    }

    std::optional<Word> findCounterexample(
        const WeightedAutomaton& hypothesis
    ) override {
        ++queryCount_;
        return oracle_.findCounterexample(hypothesis);
    }

    std::size_t queryCount() const {
        return queryCount_;
    }

private:
    EquivalenceOracle& oracle_;
    std::size_t queryCount_ = 0;
};

struct LearningRun {
    WeightedAutomaton hypothesis;
    std::size_t membershipQueries;
    std::size_t membershipRequests;
    std::size_t equivalenceQueries;
    bool equivalent;
};

LearningRun runLearning(
    const WeightedAutomaton& target,
    const WeightAlgebra& algebra,
    const std::vector<Symbol>& alphabet,
    const WordPairAdvice* advice
) {
    AutomatonOracle targetMembershipOracle(target);
    std::optional<WordPairAdvice> learnerAdvice;
    if (advice != nullptr) {
        learnerAdvice = *advice;
    }
    CountingMembershipOracle membershipOracle(
        targetMembershipOracle,
        learnerAdvice
    );
    SumEquivalenceChecker equivalenceOracle(target, alphabet);
    CountingEquivalenceOracle countingEquivalenceOracle(equivalenceOracle);

    WeightedTTT learner(
        membershipOracle,
        countingEquivalenceOracle,
        algebra,
        alphabet,
        learnerAdvice
    );
    WeightedAutomaton hypothesis = learner.learn();
    const std::size_t membershipQueries = membershipOracle.queryCount();
    const std::size_t equivalenceQueries =
        countingEquivalenceOracle.queryCount();
    const bool equivalent =
        !equivalenceOracle.findCounterexample(hypothesis).has_value();

    return {
        std::move(hypothesis),
        membershipQueries,
        membershipOracle.requestCount(),
        equivalenceQueries,
        equivalent
    };
}

bool parseWord(
    std::string_view text,
    const std::vector<Symbol>& alphabet,
    Word& word
) {
    word.clear();
    for (char symbol : text) {
        if (std::find(alphabet.begin(), alphabet.end(), symbol) ==
            alphabet.end()) {
            return false;
        }
        word.push_back(symbol);
    }
    return true;
}

std::string displayWord(const Word& word) {
    return word.empty()
        ? "<empty>"
        : std::string(word.begin(), word.end());
}

void printLearningRun(
    const char* label,
    const LearningRun& run
) {
    const std::size_t totalQueries =
        run.membershipRequests + run.equivalenceQueries;
    const std::size_t targetQueries =
        run.membershipQueries + run.equivalenceQueries;
    const std::size_t refinementRounds =
        run.equivalenceQueries == 0 ? 0 : run.equivalenceQueries - 1;

    std::cout << label << ":\n"
              << "  Learned states: "
              << run.hypothesis.numberOfStates() << '\n'
              << "  Membership queries to target: "
              << run.membershipQueries << '\n'
              << "  Membership requests from learner: "
              << run.membershipRequests << '\n'
              << "  Equivalence queries: " << run.equivalenceQueries << '\n'
              << "  Total oracle requests from learner: "
              << totalQueries << '\n'
              << "  Total queries sent to target/oracle: "
              << targetQueries << '\n'
              << "  Refinement rounds: " << refinementRounds << '\n'
              << "  Equivalent: " << (run.equivalent ? "yes" : "no") << '\n';
}

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
    if (argc > 5 || argc == 4) {
        std::cerr
            << "Usage: random_learning_simulation "
            << "[number_of_states] [seed] [u v]\n";
        return 1;
    }

    std::uint64_t requestedStates = 5;
    std::uint64_t seed = std::random_device{}();

    if (argc >= 2 && !parseUnsigned(argv[1], requestedStates)) {
        std::cerr << "number_of_states must be a positive integer\n";
        return 1;
    }

    if (argc >= 3 && !parseUnsigned(argv[2], seed)) {
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
    WordPairAdvice advice{{'a'}, {'b'}, AdviceType::Eq};
    if (argc == 5 &&
        (!parseWord(argv[3], alphabet, advice.u) ||
         !parseWord(argv[4], alphabet, advice.v))) {
        std::cerr
            << "Advice words must contain only symbols a, b, c, d\n";
        return 1;
    }

    WeightedAutomaton target = generateRandomAutomatonWithAdvice(
        numberOfStates,
        advice,
        algebra,
        random
    );
    const LearningRun withoutAdvice =
        runLearning(target, algebra, alphabet, nullptr);
    const LearningRun withAdvice =
        runLearning(target, algebra, alphabet, &advice);

    std::cout << "Seed: " << seed << '\n'
              << "Alphabet: a, b, c, d\n"
              << "Target states: " << target.numberOfStates() << '\n'
              << "Advice type: EQ\n"
              << "Advice: u=" << displayWord(advice.u)
              << ", v=" << displayWord(advice.v) << '\n';
    printLearningRun("Learning without advice", withoutAdvice);
    printLearningRun("Learning with advice", withAdvice);

    return withoutAdvice.equivalent && withAdvice.equivalent ? 0 : 1;
}
