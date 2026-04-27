#include "format.h"

#include <cassert>
#include <string>
#include <vector>

namespace {

void TokenizeSplitsUsefulWords() {
  const std::vector<std::string> tokens =
      quickseek::Tokenize("Final_Report-2026.pdf");
  assert(
      (tokens == std::vector<std::string>{"final", "report", "2026", "pdf"}));
}

void TokenizeIgnoresRepeatedSeparators() {
  const std::vector<std::string> tokens =
      quickseek::Tokenize("  build///release\\quickseek.exe  ");
  assert((tokens ==
          std::vector<std::string>{"build", "release", "quickseek", "exe"}));
}

void ToLowerHandlesMixedCaseAscii() {
  assert(quickseek::ToLower("QuickSeek CPP") == "quickseek cpp");
}

void FormatSizeUsesReadableUnits() {
  assert(quickseek::FormatSize(512) == "512 B");
  assert(quickseek::FormatSize(2048) == "2.0 KB");
}

}  // namespace

int main() {
  TokenizeSplitsUsefulWords();
  TokenizeIgnoresRepeatedSeparators();
  ToLowerHandlesMixedCaseAscii();
  FormatSizeUsesReadableUnits();
  return 0;
}
