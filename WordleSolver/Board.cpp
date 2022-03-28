#include "Board.h"
#include "Solver.h"
#include <iostream>
#include <algorithm>
#include <Windows.h>

Board::Board(const std::span<const std::string_view>& sols, size_t i) {
    m_solution = sols[i];
}

void Board::guess(std::tuple<std::string_view, bool> word_special) {
    const auto& [guessword, special] = word_special;
    uint8_t marked[26]{};
    uint8_t misplaced_letters = 0;
    uint8_t known_letters = 0;
    if (n_guess == max_guesses()) throw std::runtime_error("Maximum number of guesses reached");
    for (char c : m_solution)
        marked[c - 'a']++;

    for (size_t i = 0; i < guessword.size(); i++) {
        if (guessword[i] == m_solution[i]) {
            m_board[n_guess][i] = CharState::Correct;
            known_letters++;
            marked[guessword[i] - 'a']--;
        }
    }

    for (size_t i = 0; i < guessword.size(); i++) {
        if (m_board[n_guess][i] == CharState::Correct) continue;
        uint8_t& times_found = marked[guessword[i] - 'a'];
        if (times_found > 0 && m_solution.find(guessword[i]) != std::string_view::npos) {
            m_board[n_guess][i] = CharState::Misplaced;
            misplaced_letters++;
            times_found--;
        }
    }
    if (!special) {
        m_new_info_obtained = known_letters * 2 + misplaced_letters > m_correct_letters * 2 + m_misplaced_letters;
        m_correct_letters = known_letters;
        m_misplaced_letters = misplaced_letters;
    } else {
        m_new_info_obtained = true;
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

void Board::print(const Solver& solv) const {
    for (size_t i = 0; i < n_guess; i++) {
        for (size_t j = 0; j < array_size(m_board[i]); j++) {
            print_with_color(solv.history(i)[j], m_board[i][j]);
        }
        std::cout << '\n';
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
    }
}
