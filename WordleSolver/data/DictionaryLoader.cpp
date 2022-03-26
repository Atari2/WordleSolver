#include "DictionaryLoader.h"
#include "Dictionary.h"
#include <algorithm>
#include <ranges>

namespace r = std::ranges;

std::span<WordView> get_dictionary() {
    static bool sorted = false;
    if (!sorted) {
        r::sort(words, [](const WordView& lhs, const WordView& rhs) { return lhs.value > rhs.value; });
        sorted = true;
    }
    return std::span{words.begin(), words.end()};
}

std::span<std::string_view> get_solutions() {
    return std::span{solutions.begin(), solutions.end()};
}