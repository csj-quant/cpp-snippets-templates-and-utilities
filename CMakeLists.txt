cmake_minimum_required(VERSION 3.16)
project(cpp_snippets_templates_and_utilities CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# enable warnings
add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)

# output binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# SSO Inspector
add_executable(string_sso_inspector src/string_sso_inspector.cpp)

# Copy vs Move benchmarking
add_executable(string_copy_move_bench src/string_copy_move_bench.cpp)

# Capacity growth analysis
add_executable(string_capacity_growth src/string_capacity_growth.cpp)
add_executable(string_practice_problem_1 src/string_practice_problem_1.cpp)
add_executable(bench_insert_vector_vs_list src/bench_insert_vector_vs_list.cpp)
add_executable(bench_erase_vector_vs_list src/bench_erase_vector_vs_list.cpp)
add_executable(bench_splice_vs_vector_move src/bench_splice_vs_vector_move.cpp)
add_executable(benchmark_list_vs_intrusivelist src/benchmark_list_vs_intrusivelist.cpp)
