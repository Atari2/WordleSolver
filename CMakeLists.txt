cmake_minimum_required (VERSION 3.15)

project ("WordleSolver")
if (WIN32)
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_STANDARD_REQUIRED True)
endif()
add_subdirectory ("WordleSolver")
