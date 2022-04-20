#pragma once
#include <numeric>
#include <span>
#include <string_view>
#include "../Common.h"

constexpr double evaluate_word(const std::string_view& word) {
    struct MappedLetter {
        const uint8_t value;
        constexpr MappedLetter(char c) : value{static_cast<uint8_t>(c - 'a')} {}
        constexpr operator uint8_t() const { return value; }
    };

    uint8_t letter_counts[26]{};

    constexpr MappedLetter vowels[]{'a', 'e', 'i', 'o', 'u', 'y'};

    constexpr double occurrence_frequency[]{4.7, 6.8, 7.1, 6.1,  3.9, 4.1, 3.3, 7.2, 3.9, 1.1, 2.5,  3.1,  5.6,
                                            2.2, 2.5, 7.7, 0.96, 6.0, 4.1, 5.0, 2.9, 0.7, 2.7, 0.05, 0.36, 0.24};

    double total_occurrence_freq =
    std::accumulate(word.begin(), word.end(), 0.0, [&occurrence_frequency, &letter_counts](double s, char c) {
        auto idx = static_cast<size_t>(c - 'a');
        ++letter_counts[idx];
        return s + occurrence_frequency[idx];
    });

    // boost for vowels (but not too much)
    double vowel_boost = 0.0;
    for (uint8_t c : vowels)
        vowel_boost += occurrence_frequency[c] * (letter_counts[c] > 0);

    size_t double_penalty =
    std::accumulate(std::begin(letter_counts), std::end(letter_counts), 0ull, [](size_t ret, uint8_t occ) -> size_t {
        if (occ == 0) return ret;
        return ret + static_cast<size_t>(occ - 1);
    });

    return total_occurrence_freq + vowel_boost - (double_penalty * 2.0);
}

enum class WordMask : uint32_t {
    NOLETTER = 0,
    A = 0b000000000000000000000000000001,
    B = 0b000000000000000000000000000010,
    C = 0b000000000000000000000000000100,
    D = 0b000000000000000000000000001000,
    E = 0b000000000000000000000000010000,
    F = 0b000000000000000000000000100000,
    G = 0b000000000000000000000001000000,
    H = 0b000000000000000000000010000000,
    I = 0b000000000000000000000100000000,
    J = 0b000000000000000000001000000000,
    K = 0b000000000000000000010000000000,
    L = 0b000000000000000000100000000000,
    M = 0b000000000000000001000000000000,
    N = 0b000000000000000010000000000000,
    O = 0b000000000000000100000000000000,
    P = 0b000000000000001000000000000000,
    Q = 0b000000000000010000000000000000,
    R = 0b000000000000100000000000000000,
    S = 0b000000000001000000000000000000,
    T = 0b000000000010000000000000000000,
    U = 0b000000000100000000000000000000,
    V = 0b000000001000000000000000000000,
    W = 0b000000010000000000000000000000,
    X = 0b000000100000000000000000000000,
    Y = 0b000001000000000000000000000000,
    Z = 0b000010000000000000000000000000,
};

constexpr WordMask construct_word_mask(const std::string_view& word) {
    WordMask mask = WordMask::NOLETTER;
    for (char c : word) {
        mask |= to_enum<WordMask>(1 << (c - 'a'));
    }
    return mask;
}

struct WordView {
    std::string_view word;
    WordMask word_mask;
    double value;

    constexpr WordView(std::string_view word_) :
        word(word_), word_mask(construct_word_mask(word_)), value(evaluate_word(word_)) {}
};

constexpr WordView operator""_w(const char* ptr, size_t sz) {
    return WordView{std::string_view{ptr, sz}};
}

std::span<WordView> get_dictionary();
std::span<std::string_view> get_solutions();