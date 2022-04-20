#include "Solver.h"

namespace r = std::ranges;
constexpr size_t npos = std::string_view::npos;

SolverFilter::SolverFilter(Solver& s) : solver(s) {}

bool SolverFilter::operator()(const WordView& wordt) {
    const auto& word = wordt.word;
    const auto& alphabet = solver.alphabet;
    using enum Solver::GuessState;

    if ((solver.alphabet_mask & wordt.word_mask) != WordMask::NOLETTER) {
        dbg("Excluding " << word << " because it had a character that's not in the solution\n");
        return false;
    }
    if (r::find(solver.m_history, word) != solver.m_history.end()) {
        dbg("Excluding " << word << " because it has already been guessed\n");
        return false;
    }
    for (size_t i = 0; i < alphabet.size(); i++) {
        const auto& entry = alphabet[i];
        char letter = static_cast<char>(i + 'a');
        if ((entry.state & Correct) == Correct) {
            for (auto idx : entry.indexes_correct) {
                if (word[idx] != letter) {
                    dbg("Excluding " << word << " because it doesn't have the letter " << letter
                                     << " in the correct spot\n");
                    return false;
                }
            }
            if ((entry.state & Wrong) == Wrong && (entry.state & Misplaced) == NotGuessed) {
                size_t idx = word.find(letter);
                while (idx != npos) {
                    if (!entry.indexes_correct.contains(static_cast<uint8_t>(idx))) {
                        dbg("Excluding " << word << " because it has the letter " << letter
                                         << " in 2 spots, only one of which is correct\n");
                        return false;
                    }
                    idx = word.find(letter, idx + 1);
                }
            }
        }
        if ((entry.state & Misplaced) == Misplaced) {
            for (auto idx : entry.indexes_misplaced) {
                if (word[idx] == letter) {
                    dbg("Excluding " << word << " because it has the letter " << letter << " in the wrong spot\n");
                    return false;
                }
            }
            if (word.find(letter) == npos) {
                dbg("Excluding " << word << " because it doesn't have the letter " << letter << '\n');
                return false;
            }
        }
    }
    dbg("Including " << word << '\n');
    return true;
};

Solver::Solver(const std::span<WordView>& dictionary) :
    m_dictionary(dictionary), m_filtered_view(m_dictionary | std::views::filter(SolverFilter{*this})) {
    m_filtered_iter = m_filtered_view.end();
}

Solver::opt_ref Solver::next_guess_special(Solver::word_iter begin, Solver::word_iter end) {
    auto iter = std::find_if(begin, end, [&](const WordView& view) {
        uint32_t encountered = 0;
        bool has_double = false;
        for (auto c : view.word) {
            uint32_t val = 1 << (c - 'a');
            if (encountered & val) {
                has_double = true;
                break;
            }
            encountered |= val;
        }
        if (has_double) return false;
        return !std::any_of(view.word.begin(), view.word.end(),
                            [&](char c) { return alphabet[c - 'a'].state != GuessState::NotGuessed; });
    });
    if (iter == end) { return std::nullopt; }
    return {*iter};
}

std::tuple<std::string_view, bool> Solver::next_guess(const Board& board) {
    std::string_view guess;
    bool special_guess = false;
    if (board.guesses() == 0) {
        guess = m_dictionary.front().word;
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
                alphabet_mask |= to_enum<WordMask>((entry.state == NotGuessed) << index);
                entry.state |= Wrong;
                break;
            case CharState::Misplaced:
                alphabet_mask &= to_enum<WordMask>(~(1 << index));
                entry.state |= Misplaced;
                entry.indexes_misplaced.push_back(i);
                break;
            case CharState::Correct:
                alphabet_mask &= to_enum<WordMask>(~(1 << index));
                entry.state |= Correct;
                entry.indexes_correct.push_back(i);
                break;
            }
        }

        if (!board.info_obtained() && board.guesses() < board.max_guesses() - 1) {
            auto wordview = next_guess_special(m_filtered_iter.base(), m_dictionary.end());
            if (wordview.has_value()) {
                guess = wordview->get().word;
                special_guess = true;
            }
        }
        if (!special_guess) {
            if (m_filtered_iter == m_filtered_view.end()) {
                m_filtered_iter = m_filtered_view.begin();
                auto& wordview = (*m_filtered_iter);
                guess = wordview.word;
            } else {
                auto& wordview = (*++m_filtered_iter);
                guess = wordview.word;
            }
        }
    }
    m_history[board.guesses()] = guess;
    return {guess, special_guess};
}
