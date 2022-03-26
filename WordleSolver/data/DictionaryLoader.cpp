#include "DictionaryLoader.h"
#include "../Common.h"

#include <algorithm>

// I have this include here because including this file
// makes Visual Studio intellisense die completely from the include point onwards.
// this way the only part of the file that doesn't have intellisense
// are these lines here
#include "Dictionary.h"
std::span<WordView> get_dictionary() {
    static bool sorted = false;
    if (!sorted) {
        std::sort(words.begin(), words.end(),
                  [](const WordView& lhs, const WordView& rhs) { return lhs.value > rhs.value; });
        sorted = true;
    }
    return std::span{words.begin(), words.end()};
}

std::span<const std::string_view> get_solutions() {
    return std::span{solutions.begin(), solutions.end()};
}