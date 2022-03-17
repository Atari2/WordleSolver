#pragma once
#include <string_view>
#include <vector>
#include <span>

const std::vector<std::string>& get_dictionary(std::span<std::string_view> filenames);
const std::vector<std::string>& get_solutions(std::string_view filename);