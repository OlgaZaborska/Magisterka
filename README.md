# Uczenie ważonych automatów

Projekt implementuje ważone automaty deterministyczne oraz algorytm uczenia
`WeightedTTT`. W przykładach i symulacji używana jest algebra sumy, a losowy
automat ma alfabet `a`, `b`, `c`, `d`.

## Wymagania

- CMake 3.20 lub nowszy
- kompilator C++ obsługujący C++20

Projekt nie wymaga zewnętrznych bibliotek.

## Budowanie

W katalogu głównym repozytorium wykonaj:

```bash
cmake -S . -B build
cmake --build build -j
```

CMake tworzy bibliotekę `weighted_automaton`, przykładowy program `example`,
program symulacyjny `random_learning_simulation` oraz programy testowe.

## Interaktywna symulacja uczenia

```bash
./build/example
```

Program pyta o liczbę stanów automatu, użycie porady, ziarno losowe oraz —
jeśli wybrano poradę — jej typ i słowa `u`, `v`. Słowa podaje się nad
alfabetem `a,b,c,d`; puste słowo zapisuje się jako `<empty>`. Obecnie
obsługiwany jest wyłącznie typ porady `EQ`. Po uczeniu program wypisuje
statystyki zapytań i informację o równoważności hipotezy z automatem docelowym.

## Symulacja losowego automatu i uczenia

Uruchomienie z ustawieniami domyślnymi:

```bash
./build/random_learning_simulation
```

Domyślnie tworzony jest automat o 5 stanach, seed jest pobierany z
`std::random_device`, a porada typu `Eq` ma postać `u=a`, `v=b`.

Można podać liczbę stanów, seed oraz słowa porady, aby uzyskać powtarzalny
eksperyment:

```bash
./build/random_learning_simulation 8 123 ab cdd
```

Argumenty porady są słowami nad alfabetem `a,b,c,d`; puste słowo można podać
jako `""`. Pierwszy argument musi być dodatnią liczbą stanów, drugi jest
seedem generatora. Program uczy ten sam automat dwa razy: bez porady oraz
z poradą. Dla obu przebiegów wypisuje liczbę stanów hipotezy, zapytań
członkostwa zgłoszonych przez algorytm, zapytań rzeczywiście wysłanych do
automatu, zapytań równoważności oraz rund refinowania. Rozróżnia łączną liczbę
żądań algorytmu od liczby wywołań oracle’a po uwzględnieniu pamięci podręcznej.
Końcowa weryfikacja równoważności jest wykonywana osobno i nie wchodzi do tych
statystyk.

Generator znajduje się w
[`include/automaton/RandomAutomatonGenerator.h`](include/automaton/RandomAutomatonGenerator.h)
i [`src/automaton/RandomAutomatonGenerator.cpp`](src/automaton/RandomAutomatonGenerator.cpp).
`generateRandomAutomatonWithAdvice` buduje kompletny automat, dla którego
z każdego stanu przejście słowem `u` i przejście słowem `v` kończą w tym samym
stanie i mają tę samą łączną wagę. Wagi przejść są różnicami losowych
potencjałów stanów, więc sumują się teleskopowo; wagi końcowe wynoszą `0`.
Dlatego zachodzi `f(xuy)=f(xvy)` dla dowolnych słów `x` i `y`. Jest to
kontrolowana rodzina losowych automatów skonstruowana tak, by spełniała
poradę `Eq`, a nie losowanie ze wszystkich możliwych automatów.

Wariant symulacji z poradą kanonizuje zapytania członkostwa, zastępując
wystąpienia jednego z równoważnych słów drugim, i buforuje odpowiedzi.
Podczas budowania hipotezy stany osiągane po `u` i `v` są scalane, jeśli
bieżąca hipoteza jest zgodna z poradą dotyczącą wag przejść.

Porada jest opisana przez `WordPairAdvice`, która przechowuje słowa `u`, `v`
oraz pole `type` typu `AdviceType`. Dostępne wartości to `Eq`, `Leq` i `Geq`.
Na razie zaimplementowana jest wyłącznie obsługa `Eq`; generator zgłasza błąd
dla pozostałych typów, a learner pomija dla nich operacje specyficzne dla `Eq`.

## Jak działa uczenie

1. `AutomatonOracle` udostępnia automat docelowy jako oracle członkostwa:
   odpowiada wartością dla zadanego słowa.
2. `ResidualFunction` oblicza przyrost wartości
   `D(u, e) = f(ue) - f(u)`.
3. `WeightedTTT` wykorzystuje drzewo dyskryminacyjne do reprezentowania
   rozpoznanych stanów i buduje automat-hipotezę.
4. `SumEquivalenceChecker` porównuje hipotezę z automatem docelowym i zwraca
   kontrprzykład, jeśli znajdzie różnicę.
5. Jeśli pojawi się kontrprzykład, algorytm uzupełnia drzewo i buduje kolejną
   hipotezę. Kończy, gdy checker nie znajduje kontrprzykładu.

W symulacji te elementy łączy
[`experiments/RandomLearningSimulation.cpp`](experiments/RandomLearningSimulation.cpp).

## Testy

Po zbudowaniu projektu uruchom wszystkie testy:

```bash
ctest --test-dir build --output-on-failure
```

Testy obejmują automat i algebrę, oracle członkostwa, drzewo dyskryminacyjne,
budowanie hipotezy, sprawdzanie równoważności, algorytm `WeightedTTT` oraz
generator losowych automatów.

## Układ projektu

- `include/` — publiczne nagłówki automatu, algebry, oracle’ów i algorytmu
  uczenia.
- `src/` — implementacje automatu, generatora, oracle’ów i uczenia.
- `tests/` — testy jednostkowe uruchamiane przez CTest.
- `experiments/` — programy do eksperymentów; obecnie symulacja uczenia
  losowego automatu.
- `main.cpp` — prosty przykład ręcznie zdefiniowanego automatu.
