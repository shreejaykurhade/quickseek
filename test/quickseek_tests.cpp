#include <cassert>
#include <filesystem>
#include <string>
#include <vector>

#include "quickseek/file_record.h"
#include "quickseek/format.h"
#include "quickseek/search.h"

namespace {

quickseek::FileRecord MakeRecord(int id, std::string path, std::uintmax_t size) {
  quickseek::FileRecord record;
  record.id = id;
  record.path = std::filesystem::path(path);
  record.name = record.path.filename().string();
  record.extension = quickseek::ToLower(record.path.extension().string());
  record.size = size;
  record.tokens = quickseek::Tokenize(record.path.string());
  return record;
}

void TokenizeSplitsUsefulWords() {
  const std::vector<std::string> tokens =
      quickseek::Tokenize("Final_Report-2026.pdf");
  assert((tokens == std::vector<std::string>{"final", "report", "2026", "pdf"}));
}

void SearchRanksPrefixBeforeContains() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(MakeRecord(0, "notes/project_report.txt", 10));
  index.push_back(MakeRecord(1, "notes/report.txt", 10));

  const std::vector<quickseek::SearchResult> results =
      quickseek::SearchFiles(index, "report");

  assert(results.size() == 2);
  assert(results[0].file_id == 1);
  assert(results[0].score > results[1].score);
}

void ExtensionFilterAcceptsDotlessInput() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(MakeRecord(0, "src/main.cpp", 10));
  index.push_back(MakeRecord(1, "README.md", 10));

  const std::vector<quickseek::FileRecord> files =
      quickseek::FilesByExtension(index, "cpp", 10);

  assert(files.size() == 1);
  assert(files[0].name == "main.cpp");
}

}  // namespace

int main() {
  TokenizeSplitsUsefulWords();
  SearchRanksPrefixBeforeContains();
  ExtensionFilterAcceptsDotlessInput();
  return 0;
}
