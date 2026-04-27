#include "format.h"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace quickseek {

std::string ToLower(std::string text) {
  for (char& ch : text) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
  return text;
}

std::vector<std::string> Tokenize(const std::string& text) {
  std::vector<std::string> tokens;
  std::string current;

  for (char ch : text) {
    if (std::isalnum(static_cast<unsigned char>(ch))) {
      current +=
          static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    } else if (!current.empty()) {
      tokens.push_back(current);
      current.clear();
    }
  }

  if (!current.empty()) {
    tokens.push_back(current);
  }

  return tokens;
}

std::string FormatSize(std::uintmax_t bytes) {
  const char* units[] = {"B", "KB", "MB", "GB"};
  double value = static_cast<double>(bytes);
  int unit = 0;

  while (value >= 1024.0 && unit < 3) {
    value /= 1024.0;
    ++unit;
  }

  std::ostringstream out;
  out << std::fixed << std::setprecision(unit == 0 ? 0 : 1) << value << " "
      << units[unit];
  return out.str();
}

}  // namespace quickseek
