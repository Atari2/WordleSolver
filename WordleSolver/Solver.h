#pragma once
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "data/DictionaryLoader.h"
#include "Board.h"

class FakeStream : std::ofstream {
    public:
    FakeStream() : std::ofstream{"NUL"} {}
    template <typename T>
    std::ostream& operator<<(const T&) {
        return *this;
    }
};

class DebugStream {
    bool m_enabled = false;

    FakeStream m_dummy_stream{};

    public:
    DebugStream() = default;
    void debug_stream_enabled(bool enabled) { m_enabled = enabled; }
    template <typename T>
    std::ostream& operator<<(const T& t) {
        if (m_enabled) {
            return std::cout << t;
        } else {
            return m_dummy_stream << t;
        }
    }
};

static inline DebugStream dbg{};

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
        std::vector<uint8_t> indexes_misplaced{};
        std::vector<uint8_t> indexes_correct{};
        constexpr LetterState(char _c) : state(GuessState::NotGuessed) {}
    };

    std::array<LetterState, 26> alphabet{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                         'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    decltype(m_dictionary | std::views::filter(std::declval<SolverFilter>())) m_filtered_view;
    decltype(std::declval<decltype(m_filtered_view)>().begin()) m_filtered_iter;

    public:
    Solver(const std::span<WordView>& dictionary);

    std::string_view history(size_t idx) const { return m_history[idx]; }
    std::string_view next_guess(const Board&);
};