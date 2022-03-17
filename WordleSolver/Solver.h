#pragma once
#include <array>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include "Board.h"

class Solver;

class SolverFilter {
    Solver& solver;

    public:
    SolverFilter(Solver& s);
    bool operator()(const std::string& word);
};

class Solver {
    friend SolverFilter;
    const std::vector<std::string>& m_dictionary;
    std::array<std::string_view, Board::max_guesses()> m_history;
    size_t m_starting_index;

    enum class GuessState : uint8_t { NotGuessed = 0, Wrong = 1, Misplaced = 2, Correct = 4 };

    struct LetterState {
        GuessState state;
        std::vector<uint8_t> indexes_misplaced{};
        std::vector<uint8_t> indexes_correct{};
        constexpr LetterState(char _c) : state(GuessState::NotGuessed) {}
    };

    static inline std::array<LetterState, 26> alphabet{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                                       'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    decltype(m_dictionary | std::views::filter(std::declval<SolverFilter>())) m_filtered_view;
    decltype(std::declval<decltype(m_filtered_view)>().begin()) m_filtered_iter;

    public:
    Solver(const std::vector<std::string>& dictionary, size_t idx);

    std::string_view history(size_t idx) const { return m_history[idx]; }
    std::string_view next_guess(const Board&);
};