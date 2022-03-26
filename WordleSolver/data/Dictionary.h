#pragma once
#include <array>
#include <string_view>
#include "DictionaryLoader.h"

// extern because if I declared them in the .h file Intellisense dies horribly.
extern std::array<std::string_view, 2309> solutions;
extern std::array<WordView, 12972> words;
