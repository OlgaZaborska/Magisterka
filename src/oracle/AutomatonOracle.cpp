#include "oracle/AutomatonOracle.h"

AutomatonOracle::AutomatonOracle(const WeightedAutomaton& automaton)
    : automaton_(automaton) {
}

Weight AutomatonOracle::query(const Word& word) {
    return automaton_.evaluate(word);
}