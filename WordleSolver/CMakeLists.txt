cmake_minimum_required (VERSION 3.15)

if (1)
	set(CONDITIONAL_FILES "Solver.cpp" "Board.cpp")
else()
	set(CONDITIONAL_FILES "")
endif()

add_executable (WordleSolver "WordleSolver.cpp" "data/DictionaryLoader.cpp" ${CONDITIONAL_FILES})

configure_file("data/Words.txt" "${CMAKE_CURRENT_BINARY_DIR}/data/Words.txt" COPYONLY)
configure_file("data/Solutions.txt" "${CMAKE_CURRENT_BINARY_DIR}/data/Solutions.txt" COPYONLY)

target_compile_options(WordleSolver PUBLIC /utf-8 /permissive-)