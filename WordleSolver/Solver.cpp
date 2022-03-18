#include "Solver.h"
#include "Common.h"

SolverFilter::SolverFilter(Solver& s) : solver(s) {}

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

Solver::Solver(const std::vector<std::string>& dictionary, size_t idx) :
    m_dictionary(dictionary),
    m_starting_index(idx),
    m_filtered_view(m_dictionary | std::views::filter(SolverFilter{*this})) {
    m_filtered_iter = m_filtered_view.end();
}

std::string_view Solver::next_guess(const Board& board) {
    std::string_view guess;
    if (board.guesses() == 0) {
        guess = m_dictionary[m_starting_index];
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
