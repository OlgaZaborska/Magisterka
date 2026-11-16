
cmake_minimum_required(VERSION 3.20)

project(WeightedAutomataLearning
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(weighted_automaton
    src/automaton/WeightedAutomaton.cpp
    src/learning/DiscriminationTree.cpp
    src/learning/WeightedTTT.cpp
    src/oracle/AutomatonOracle.cpp
    src/learning/HypothesisBuilder.cpp
    src/oracle/SumEquivalenceChecker.cpp
)

target_include_directories(weighted_automaton
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(weighted_automaton PRIVATE /W4)
else()
    target_compile_options(weighted_automaton PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Automaton tests
add_executable(automaton_tests
    tests/AutomatonTests.cpp
)

target_link_libraries(automaton_tests
    PRIVATE weighted_automaton
)

add_test(
    NAME AutomatonTests
    COMMAND automaton_tests
)

# Membership Oracle tests
add_executable(membership_oracle_tests
    tests/MembershipOracleTests.cpp
)

target_link_libraries(membership_oracle_tests
    PRIVATE weighted_automaton
)

add_test(
    NAME MembershipOracleTests
    COMMAND membership_oracle_tests
)

# Discrimination Tree tests
add_executable(discrimination_tree_tests
    tests/DiscriminationTreeTests.cpp
)

target_link_libraries(discrimination_tree_tests
    PRIVATE weighted_automaton
)

add_test(
    NAME DiscriminationTreeTests
    COMMAND discrimination_tree_tests
)

# Access Sequence tests
add_executable(access_sequence_tests
    tests/AccessSequenceTests.cpp
)

target_link_libraries(access_sequence_tests PRIVATE weighted_automaton)

add_test(NAME AccessSequenceTests COMMAND access_sequence_tests)

# Hypothesis Builder tests
add_executable(hypothesis_builder_tests
    tests/HypothesisBuilderTests.cpp
)

target_link_libraries(hypothesis_builder_tests PRIVATE weighted_automaton)

add_test(NAME HypothesisBuilderTests COMMAND hypothesis_builder_tests)

# Sum Equivalence Checker tests
add_executable(sum_equivalence_checker_tests
    tests/SumEquivalenceCheckerTests.cpp
)

target_link_libraries(sum_equivalence_checker_tests PRIVATE weighted_automaton)

add_test(
    NAME SumEquivalenceCheckerTests
    COMMAND sum_equivalence_checker_tests
)

# Weighted TTT tests
add_executable(weighted_ttt_tests
    tests/WeightedTTTTests.cpp
)

target_link_libraries(weighted_ttt_tests PRIVATE weighted_automaton)

add_test(
    NAME WeightedTTTTests
    COMMAND weighted_ttt_tests
)

# Example application
add_executable(example
    main.cpp
)

target_link_libraries(example
    PRIVATE weighted_automaton
)
