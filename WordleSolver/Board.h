#pragma once
#include "Common.h"
#include <string>
#include <vector>
#include <type_traits>

constexpr static bool DEBUG_BOARD = true;

constexpr size_t static_starting_word_idx = 11452;
constexpr size_t static_solution_idx = 271;
constexpr size_t static_max_guesses = 6;

enum class CharState { Wrong = 0, Misplaced = 1, Correct = 2 };

template <typename T>
requires std::is_array_v<T>
constexpr size_t type_array_size() {
    return std::extent_v<T>;
}

class Solver;

class Board {
    std::string_view m_solution;
    CharState m_board[static_max_guesses][5]{};
    size_t n_guess = 0;

    public:
    Board(const std::vector<std::string>& sols, size_t i);

    using type = decltype(m_board);

    std::string_view solution() const { return m_solution; }
    void guess(std::string_view guessword);
    bool solved() const noexcept;
    size_t guesses() const noexcept { return n_guess; }
    static constexpr size_t max_guesses() noexcept { return type_array_size<type>(); };
    const auto& board() const noexcept { return m_board; }
    void print(const Solver&) const;
};