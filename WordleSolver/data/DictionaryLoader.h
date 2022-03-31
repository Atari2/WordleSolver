#pragma once
#include <numeric>
#include <span>
#include <string_view>

constexpr double evaluate_word(const std::string_view& word) {
    uint8_t letter_counts[26]{};

    constexpr char vowels[]{'a', 'e', 'i', 'o', 'u'};

    constexpr double occurrence_frequency[]{4.7, 6.8, 7.1, 6.1,  3.9, 4.1, 3.3, 7.2, 3.9, 1.1, 2.5,  3.1,  5.6,
                                            2.2, 2.5, 7.7, 0.96, 6.0, 4.1, 5.0, 2.9, 0.7, 2.7, 0.05, 0.36, 0.24};

    double total_occurrence_freq =
    std::accumulate(word.begin(), word.end(), 0.0, [&occurrence_frequency, &letter_counts](double s, char c) {
        auto idx = static_cast<size_t>(c - 'a');
        letter_counts[idx]++;
        return s + occurrence_frequency[idx];
    });

    // boost for vowels (but not too much)
    double vowel_boost = 0.0;
    for (char c : vowels)
        vowel_boost += word.contains(c) ? occurrence_frequency[c - 'a'] : 0.0;

    size_t double_penalty =
    std::accumulate(std::begin(letter_counts), std::end(letter_counts), 0ull, [](size_t ret, uint8_t occ) -> size_t {
        if (occ == 0) return ret;
        return ret + static_cast<size_t>(occ - 1);
    });

    return total_occurrence_freq + vowel_boost - (double_penalty * 2.0);
}

constexpr uint32_t construct_word_mask(const std::string_view& word) {
    uint32_t mask = 0;
    for (char c : word) {
        mask |= (1 << (c - 'a'));
    }
    return mask;
}

struct WordView {
    std::string_view word;
    uint32_t word_mask;
    double value;

    constexpr WordView(std::string_view word_) :
        word(word_), word_mask(construct_word_mask(word_)), value(evaluate_word(word_)) {}
};

constexpr WordView operator""_w(const char* ptr, size_t sz) {
    return WordView{std::string_view{ptr, sz}};
}

std::span<WordView> get_dictionary();
std::span<std::string_view> get_solutions();