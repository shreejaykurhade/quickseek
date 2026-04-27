#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace quickseek {

std::string FormatSize(std::uintmax_t bytes);
std::string ToLower(std::string text);
std::vector<std::string> Tokenize(const std::string& text);

}  // namespace quickseek
