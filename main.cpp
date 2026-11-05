
#include "automaton/WeightedAutomaton.h"

#include <iostream>

int main() {
    WeightedAutomaton automaton({'a', 'b'});

    // Tworzenie stanów.
    StateId q0 = automaton.addState(5);
    StateId q1 = automaton.addState(7);
    StateId q2 = automaton.addState(2);

    // Stan początkowy.
    automaton.setInitialState(q0);

    // Definicja przejść.
    automaton.addTransition(q0, 'a', q1, 3);
    automaton.addTransition(q0, 'b', q0, 2);

    automaton.addTransition(q1, 'a', q1, 1);
    automaton.addTransition(q1, 'b', q2, 4);

    automaton.addTransition(q2, 'a', q1, 2);
    automaton.addTransition(q2, 'b', q0, 2);

    // Obliczanie wartości słowa.
    Word word = {'a', 'b', 'b'};

    Weight result = automaton.evaluate(word);

    std::cout << "Automaton evaluation: "
              << result << '\n';

    return 0;
}
