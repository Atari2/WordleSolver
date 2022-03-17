#include "data/DictionaryLoader.h"

// leave as 1 to reproduce the bug with iter_move ambiguous symbol
// has to be changed in parallel with the if (0) in the inner CMakeLists
#if 1

#include "Board.h"
#include "Solver.h"
#include <iostream>
#include <charconv>

#else

constexpr bool DEBUG_BOARD = true;
constexpr size_t static_starting_word_idx = 11452;
constexpr size_t static_solution_idx = 271;
constexpr size_t static_max_guesses = 6;


#include <array>
#include <charconv>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include "Common.h"
#include <Windows.h>


template <typename T>
requires std::is_array_v<T>
constexpr size_t type_array_size() {
    return std::extent_v<T>;
}

enum class CharState { Wrong = 0, Misplaced = 1, Correct = 2 };

class Solver;

class Board {
    std::string_view m_solution;
    CharState m_board[static_max_guesses][5]{};
    size_t n_guess = 0;

    public:
    Board(const std::vector<std::string>& sols, size_t index);

    std::string_view solution() const { return m_solution; }
    void guess(std::string_view guessword);
    bool solved() const noexcept;
    size_t guesses() const noexcept { return n_guess; }

    using type = decltype(m_board);

    static constexpr size_t max_guesses() noexcept { return type_array_size<type>(); };
    const auto& board() const noexcept { return m_board; }
    void print(const Solver&) const;
};

Board::Board(const std::vector<std::string>& sols, size_t index) {
    m_solution = sols[index];
}

void Board::guess(std::string_view guessword) {
    uint8_t marked[26]{};
    if (n_guess == max_guesses()) throw std::runtime_error("Maximum number of guesses reached");
    for (char c : m_solution)
        marked[c - 'a']++;

    for (size_t i = 0; i < guessword.size(); i++) {
        if (guessword[i] == m_solution[i]) {
            m_board[n_guess][i] = CharState::Correct;
            marked[guessword[i] - 'a']--;
        }
    }

    for (size_t i = 0; i < guessword.size(); i++) {
        if (m_board[n_guess][i] == CharState::Correct) continue;
        uint8_t& times_found = marked[guessword[i] - 'a'];
        if (times_found > 0 && m_solution.find(guessword[i]) != std::string_view::npos) {
            m_board[n_guess][i] = CharState::Misplaced;
            times_found--;
        }
    }
    n_guess++;
}

bool Board::solved() const noexcept {
    if (n_guess < 1) return false;
    return std::all_of(std::begin(m_board[n_guess - 1]), std::end(m_board[n_guess - 1]),
                       [](CharState state) { return state == CharState::Correct; });
}

void print_with_color(char c, CharState col) {
    char upper_c = std::toupper(c);
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    constexpr auto yellow = FOREGROUND_GREEN | FOREGROUND_RED;
    constexpr auto green = FOREGROUND_GREEN;
    constexpr auto red = FOREGROUND_RED;
    switch (col) {
    case CharState::Correct:
        SetConsoleTextAttribute(out, green);
        break;
    case CharState::Wrong:
        SetConsoleTextAttribute(out, red);
        break;
    case CharState::Misplaced:
        SetConsoleTextAttribute(out, yellow);
        break;
    }
    std::cout << upper_c;
}

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

    std::array<LetterState, 26> alphabet{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                         'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    decltype(m_dictionary | std::views::filter(std::declval<SolverFilter>())) m_filtered_view;
    decltype(std::declval<decltype(m_filtered_view)>().begin()) m_filtered_iter;

    public:
    Solver(const std::vector<std::string>& dictionary, size_t starting_index);

    std::string_view history(size_t idx) const { return m_history[idx]; }
    std::string_view next_guess(const Board& board);
};

void Board::print(const Solver& solv) const {
    for (size_t i = 0; i < n_guess; i++) {
        for (size_t j = 0; j < array_size(m_board[i]); j++) {
            print_with_color(solv.history(i)[j], m_board[i][j]);
        }
        std::cout << '\n';
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
    }
}

SolverFilter::SolverFilter(Solver& s) : solver(s) {}

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

static DebugStream dbg{};

bool SolverFilter::operator()(const std::string& word) {
    const auto& alphabet = solver.alphabet;
    using enum Solver::GuessState;
    for (char c : word) {
        if (alphabet[c - 'a'].state == Wrong) {
            dbg << "Excluding " << word << " because it had a character that's not in the solution\n";
            return false;
        }
    }
    if (std::find(solver.m_history.begin(), solver.m_history.end(), word) != solver.m_history.end()) {
        dbg << "Excluding " << word << " because it has already been guessed\n";
        return false;
    }
    for (size_t i = 0; i < alphabet.size(); i++) {
        if ((alphabet[i].state & Correct) == Correct) {
            for (auto idx : alphabet[i].indexes_correct) {
                if (word[idx] != static_cast<char>(i + 'a')) {
                    dbg << "Excluding " << word << " because it doesn't have the letter " << (char)(i + 'a')
                        << " in the correct spot\n";
                    return false;
                }
            }
            if ((alphabet[i].state & Wrong) == Wrong) { 
                char letter = static_cast<char>(i + 'a');
                size_t idx = word.find(letter, 0);
                while (idx != std::string_view::npos) {
                    if (std::ranges::find(alphabet[i].indexes_correct, idx) == alphabet[i].indexes_correct.end()) {
                        dbg << "Excluding " << word << " because has the letter " << (char)(i + 'a')
                            << " in 2 spots, only one of which is correct\n";
                        return false;
                    }
                    idx = word.find(letter, idx + 1);
                }
            }
        }
        if ((alphabet[i].state & Misplaced) == Misplaced) {
            for (auto idx : alphabet[i].indexes_misplaced) {
                if (word[idx] == static_cast<char>(i + 'a')) {
                    dbg << "Excluding " << word << " because it has the letter " << (char)(i + 'a')
                        << " in the wrong spot\n";
                    return false;
                }
            }
            if (word.find(i + 'a') == std::string::npos) {
                dbg << "Excluding " << word << " because it doesn't have the letter " << (char)(i + 'a') << '\n';
                return false;
            }
        }
    }
    dbg << "Including " << word << '\n';
    return true;
};

Solver::Solver(const std::vector<std::string>& dictionary, size_t starting_index) :
    m_dictionary(dictionary),
    m_starting_index(starting_index),
    m_filtered_view(m_dictionary | std::views::filter(SolverFilter{*this})) {
    m_filtered_iter = m_filtered_view.end();
}

std::string_view Solver::next_guess(const Board& board) {
    std::string_view guess;
    if (board.guesses() == 0) {
        if constexpr (DEBUG_BOARD) {
            // starts out by guessing "range" always
            guess = m_dictionary[m_starting_index];
        } else {
            std::random_device rd;
            std::mt19937 mt{rd()};
            std::uniform_int_distribution dist{0ull, m_dictionary.size()};
            guess = m_dictionary[dist(mt)];
        }
    } else {
        const auto& m = board.board();
        const auto& prev_guess_row = m[board.guesses() - 1];
        const auto& prev_guess = m_history[board.guesses() - 1];

        // store information gathered from previous guess
        for (size_t i = 0; i < array_size(prev_guess_row); i++) {
            using enum GuessState;
            size_t index = static_cast<size_t>(prev_guess[i] - 'a');
            auto& entry = alphabet[index];
            switch (prev_guess_row[i]) {
            case CharState::Wrong:
                entry.state |= Wrong;
                break;
            case CharState::Misplaced:
                entry.state |= Misplaced;
                entry.indexes_misplaced.push_back(i);
                break;
            case CharState::Correct:
                entry.state |= Correct;
                entry.indexes_correct.push_back(i);
                break;
            }
        }
        if (m_filtered_iter == m_filtered_view.end()) {
            m_filtered_iter = m_filtered_view.begin();
            guess = *m_filtered_iter;
        } else {
            guess = *++m_filtered_iter;
        }
    }
    m_history[board.guesses()] = guess;
    return guess;
}

#endif

using namespace std::string_view_literals;

int main(int argc, char** argv) {
    std::string_view files[2]{"data/Words.txt"sv, "data/Solutions.txt"sv};
    const auto& solutions = get_solutions(files[1]);
    const auto& dict = get_dictionary(files);
    size_t idx = 0;
    size_t sol_idx = 0;
    if (argc != 3) {
        idx = static_starting_word_idx;
        sol_idx = static_solution_idx;
    } else {
        auto res1 = std::from_chars(argv[1], argv[1] + strlen(argv[1]), idx);
        auto res2 = std::from_chars(argv[2], argv[2] + strlen(argv[2]), sol_idx);
        if (res1.ec != std::errc{} || res2.ec != std::errc{}) {
            std::cout << "Invalid program arguments\n";
            return EXIT_FAILURE;
        }
    }
    size_t total_guesses = 0;
    size_t guessed = 0;
    size_t max_guesses = 0;
    size_t min_guesses = 0;
    for (size_t i = 0; i < 271; i++) {
        Board b{solutions, i};
        Solver s{dict, idx};
        while (!b.solved() && b.guesses() < b.max_guesses()) {
            b.guess(s.next_guess(b));
        }
        b.print(s);
        std::cout << "[" << i << "] ";
        size_t n_guesses = b.guesses();
        if (b.solved()) {
            total_guesses += n_guesses;
            guessed++;
            std::cout << "Solved in " << b.guesses() << " guesse(s), the word was " << b.solution() << "\n";
            if (i == 0) {
                max_guesses = n_guesses;
                min_guesses = n_guesses;
            } else {
                if (max_guesses < n_guesses) max_guesses = n_guesses;
                else if (min_guesses > n_guesses) min_guesses = n_guesses;
            }
        } else {
            std::cout << "Didn't solve it, the word was " << b.solution() << '\n';
        }
    }
    std::cout << "Correctly guessed " << guessed << " out of " << static_solution_idx << '\n';
    std::cout << "Max guesses " << max_guesses << ", Min guesses " << min_guesses << '\n';
    std::cout << "Average guesses: " << (static_cast<double>(total_guesses) / static_cast<double>(static_solution_idx))
              << '\n';
    return EXIT_SUCCESS;
}
