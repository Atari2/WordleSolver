cmake_minimum_required (VERSION 3.15)

add_executable (WordleSolver "WordleSolver.cpp" "data/DictionaryLoader.cpp" "Solver.cpp" "Solver.h" "Board.cpp" "Board.h" "Common.h" "data/Dictionary.cpp")

target_compile_definitions(WordleSolver PUBLIC WIN32_LEAN_AND_MEAN VC_EXTRALEAN)
target_compile_options(WordleSolver PUBLIC /utf-8 /permissive-)