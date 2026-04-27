#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "quickseek/file_record.h"
#include "quickseek/format.h"
#include "quickseek/index.h"
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

void ExtensionFilterHonorsLimit() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(MakeRecord(0, "src/one.cpp", 10));
  index.push_back(MakeRecord(1, "src/two.cpp", 10));
  index.push_back(MakeRecord(2, "src/three.cpp", 10));

  const std::vector<quickseek::FileRecord> files =
      quickseek::FilesByExtension(index, ".cpp", 2);

  assert(files.size() == 2);
}

void LargestFilesSortsDescending() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(MakeRecord(0, "small.txt", 10));
  index.push_back(MakeRecord(1, "large.txt", 100));
  index.push_back(MakeRecord(2, "medium.txt", 50));

  const std::vector<quickseek::FileRecord> files =
      quickseek::LargestFiles(index, 2);

  assert(files.size() == 2);
  assert(files[0].name == "large.txt");
  assert(files[1].name == "medium.txt");
}

void BuildIndexStaysInsideRequestedRoot() {
  const std::filesystem::path base =
      std::filesystem::temp_directory_path() / "quickseek_index_scope_test";
  const std::filesystem::path root = base / "root";
  const std::filesystem::path sibling = base / "sibling";

  std::error_code error;
  std::filesystem::remove_all(base, error);
  std::filesystem::create_directories(root / "nested");
  std::filesystem::create_directories(sibling);

  std::ofstream(root / "alpha.txt") << "alpha";
  std::ofstream(root / "nested" / "beta.md") << "beta";
  std::ofstream(sibling / "outside.txt") << "outside";

  const std::vector<quickseek::FileRecord> index = quickseek::BuildIndex(root);

  assert(index.size() == 2);
  for (const quickseek::FileRecord& file : index) {
    assert(file.path.string().find(sibling.string()) == std::string::npos);
  }

  std::filesystem::remove_all(base, error);
}

}  // namespace

int main() {
  TokenizeSplitsUsefulWords();
  SearchRanksPrefixBeforeContains();
  ExtensionFilterAcceptsDotlessInput();
  ExtensionFilterHonorsLimit();
  LargestFilesSortsDescending();
  BuildIndexStaysInsideRequestedRoot();
  return 0;
}
