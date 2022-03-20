#include "DictionaryLoader.h"
#include "../Common.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <locale>
#include <numeric>

Word::Word(std::string word_) : word(std::move(word_)) {
    evaluate();
}

constexpr double evalute_word(std::string_view word) {
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
    constexpr auto awake = evalute_word("stiff");
    constexpr auto neafe = evalute_word("stimy");
    value = evalute_word(word);
}

struct comma_sep : std::ctype<char> {
    comma_sep() : std::ctype<char>(get_table()) {}
    static mask const* get_table() {
        static mask rc[table_size]{};
        rc[','] = std::ctype_base::space;
        rc['\n'] = std::ctype_base::space;
        return &rc[0];
    }
};

const std::vector<Word>& get_dictionary(std::span<std::string_view> filenames) {
    static std::vector<Word> dictionary{};
    std::string curr_word{};
    if (dictionary.empty()) {
        for (const auto& filename : filenames) {
            std::ifstream file{filename.data()};
            file.imbue(std::locale(file.getloc(), new comma_sep));
            if (file) {
                while (!file.eof()) {
                    file >> std::quoted(curr_word);
                    dictionary.push_back(Word{curr_word});
                }
            } else {
                throw std::runtime_error{"Failed to open file"};
            }
        }
        std::sort(dictionary.begin(), dictionary.end(),
                  [](const Word& lhs, const Word& rhs) { return lhs.value > rhs.value; });
    }
    return dictionary;
}

const std::vector<std::string>& get_solutions(std::string_view filename) {
    static std::vector<std::string> solutions{};
    if (solutions.empty()) {
        std::ifstream file{filename.data()};
        file.imbue(std::locale(file.getloc(), new comma_sep));
        if (file) {
            std::string curr_word{};
            while (!file.eof()) {
                file >> std::quoted(curr_word);
                solutions.push_back(curr_word);
            }
        } else {
            throw std::runtime_error{"Failed to open file"};
        }
    }
    return solutions;
}