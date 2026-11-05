
cmake_minimum_required(VERSION 3.20)

project(WeightedAutomataLearning
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(weighted_automaton
    src/automaton/WeightedAutomaton.cpp
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

add_executable(automaton_tests
    tests/AutomatonTests.cpp
)

target_link_libraries(automaton_tests
    PRIVATE weighted_automaton
)

enable_testing()

add_test(
    NAME AutomatonTests
    COMMAND automaton_tests
)

add_executable(example
    main.cpp
)

target_link_libraries(example
    PRIVATE weighted_automaton
)
