#include "DictionaryLoader.h"
#include <fstream>
#include <iomanip>
#include <locale>

struct comma_sep : std::ctype<char> {
    comma_sep() : std::ctype<char>(get_table()) {}
    static mask const* get_table() {
        static mask rc[table_size]{};
        rc[','] = std::ctype_base::space;
        rc['\n'] = std::ctype_base::space;
        return &rc[0];
    }
};

const std::vector<std::string>& get_dictionary(std::span<std::string_view> filenames) {
    static std::vector<std::string> dictionary{};
    if (dictionary.empty()) {
        for (const auto& filename : filenames) {
            std::ifstream file{filename.data()};
            file.imbue(std::locale(file.getloc(), new comma_sep));
            if (file) {
                std::string curr_word{};
                while (!file.eof()) {
                    file >> std::quoted(curr_word);
                    dictionary.push_back(curr_word);
                }
            } else {
                throw std::runtime_error{"Failed to open file"};
            }
        }
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