#include "DictionaryLoader.h"
#include "../Common.h"
#include <numeric>

Word::Word(std::string word_) : word(std::move(word_)) {
    evaluate();
}

constexpr double evaluate_word(std::string_view word) {
    uint8_t letter_counts[26]{};
    constexpr char vowels[]{'a', 'e', 'i', 'o', 'u'};

    constexpr double occurrence_frequency[]{4.7, 6.8, 7.1, 6.1,  3.9, 4.1, 3.3, 7.2, 3.9, 1.1, 2.5,  3.1,  5.6,
                                            2.2, 2.5, 7.7, 0.96, 6.0, 4.1, 5.0, 2.9, 0.7, 2.7, 0.05, 0.36, 0.24};

    double total_occurrence_freq =
    std::reduce(word.begin(), word.end(), 0.0, [&occurrence_frequency, &letter_counts](double s, char c) {
        auto idx = static_cast<size_t>(c - 'a');
        letter_counts[idx]++;
        return s + occurrence_frequency[idx];
    });

    // boost for vowels (but not too much)
    double vowel_boost = 0.0;
    for (char c : vowels)
        vowel_boost += word.contains(c) ? occurrence_frequency[c - 'a'] : 0.0;

    size_t double_penalty =
    std::reduce(std::begin(letter_counts), std::end(letter_counts), 0ull, [](size_t ret, uint8_t occ) -> size_t {
        if (occ == 0) return ret;
        return ret + static_cast<size_t>(occ - 1);
    });

    return total_occurrence_freq + vowel_boost - (double_penalty * 2.0);
}

void Word::evaluate() {
    value = evaluate_word(word);
}

void WordView::evaluate() const {
    value = evaluate_word(word);
}

// I have this include here because including this file 
// makes Visual Studio intellisense die completely from the include point onwards.
// this way the only part of the file that doesn't have intellisense
// are these lines here
#include "Dictionary.h"
std::span<const WordView> get_dictionary() {
    return std::span{words.begin(), words.end()};
}

std::span<const std::string_view> get_solutions() {
    return std::span{solutions.begin(), solutions.end()};
}