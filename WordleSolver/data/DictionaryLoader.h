#pragma once
#include <string_view>
#include <vector>
#include <span>

struct Word {
    std::string word;
    double value;

    Word(std::string word_);
    void evaluate();
};

const std::vector<Word>& get_dictionary(std::span<std::string_view> filenames);
const std::vector<std::string>& get_solutions(std::string_view filename);