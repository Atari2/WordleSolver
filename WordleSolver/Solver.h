#pragma once
#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "data/DictionaryLoader.h"
#include "Board.h"

class Solver;

class SolverFilter {
    Solver& solver;

    public:
    SolverFilter(Solver& s);
    bool operator()(const WordView& wordt);
};

class Solver {
    friend SolverFilter;
    const std::span<WordView>& m_dictionary;
    std::array<std::string_view, Board::max_guesses()> m_history;

    enum class GuessState : uint8_t { NotGuessed = 0, Wrong = 1, Misplaced = 2, Correct = 4 };

    struct LetterState {
        GuessState state;

        // using std::basic_string enables the use of SSO, avoiding allocation
        // since the inline buffer is more than 5 bytes, this is guaranteed to work and never allocate
        using bytebuffer = std::basic_string<uint8_t>;

        bytebuffer indexes_misplaced{};
        bytebuffer indexes_correct{};
        constexpr LetterState(char _c) : state(GuessState::NotGuessed) {}
    };

    std::array<LetterState, 26> alphabet{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                         'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    uint32_t alphabet_mask = 0;

    decltype(m_dictionary | std::views::filter(std::declval<SolverFilter>())) m_filtered_view;
    decltype(std::declval<decltype(m_filtered_view)>().begin()) m_filtered_iter;

    public:
    Solver(const std::span<WordView>& dictionary);

    std::string_view history(size_t idx) const { return m_history[idx]; }
    std::string_view next_guess(const Board&);
};