#pragma once
#include <span>
#include <string_view>

struct Word {
    std::string word;
    double value;

    Word(std::string word_);
    void evaluate();
};

struct WordView {
    std::string_view word;
    mutable double value;

    constexpr WordView(std::string_view word_) : word(word_), value(0.0) {}
    void evaluate() const;
};

constexpr WordView operator""_w(const char* ptr, size_t sz) {
    return WordView{std::string_view{ptr, sz}};
}

std::span<const WordView> get_dictionary();
std::span<const std::string_view> get_solutions();