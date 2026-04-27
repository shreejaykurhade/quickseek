#include "search.h"
#include "test_utils.h"

#include <cassert>
#include <string>
#include <vector>

namespace {

void SearchRanksPrefixBeforeContains() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(
      quickseek::test::MakeRecord(0, "notes/project_report.txt", 10));
  index.push_back(quickseek::test::MakeRecord(1, "notes/report.txt", 10));

  const std::vector<quickseek::SearchResult> results =
      quickseek::SearchFiles(index, "report");

  assert(results.size() == 2);
  assert(results[0].file_id == 1);
  assert(results[0].score > results[1].score);
}

void SearchFindsPathTokenMatches() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(quickseek::test::MakeRecord(0, "docs/private/notes.txt", 10));

  const std::vector<quickseek::SearchResult> results =
      quickseek::SearchFiles(index, "private");

  assert(results.size() == 1);
  assert(results[0].reason.find("path contains") != std::string::npos);
}

void SearchReturnsEmptyForBlankQuery() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(quickseek::test::MakeRecord(0, "docs/readme.md", 10));

  assert(quickseek::SearchFiles(index, "   ").empty());
}

void ExtensionFilterAcceptsDotlessInput() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(quickseek::test::MakeRecord(0, "src/main.cpp", 10));
  index.push_back(quickseek::test::MakeRecord(1, "README.md", 10));

  const std::vector<quickseek::FileRecord> files =
      quickseek::FilesByExtension(index, "cpp", 10);

  assert(files.size() == 1);
  assert(files[0].name == "main.cpp");
}

void ExtensionFilterHonorsLimit() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(quickseek::test::MakeRecord(0, "src/one.cpp", 10));
  index.push_back(quickseek::test::MakeRecord(1, "src/two.cpp", 10));
  index.push_back(quickseek::test::MakeRecord(2, "src/three.cpp", 10));

  const std::vector<quickseek::FileRecord> files =
      quickseek::FilesByExtension(index, ".cpp", 2);

  assert(files.size() == 2);
}

void LargestFilesSortsDescending() {
  std::vector<quickseek::FileRecord> index;
  index.push_back(quickseek::test::MakeRecord(0, "small.txt", 10));
  index.push_back(quickseek::test::MakeRecord(1, "large.txt", 100));
  index.push_back(quickseek::test::MakeRecord(2, "medium.txt", 50));

  const std::vector<quickseek::FileRecord> files =
      quickseek::LargestFiles(index, 2);

  assert(files.size() == 2);
  assert(files[0].name == "large.txt");
  assert(files[1].name == "medium.txt");
}

}  // namespace

int main() {
  SearchRanksPrefixBeforeContains();
  SearchFindsPathTokenMatches();
  SearchReturnsEmptyForBlankQuery();
  ExtensionFilterAcceptsDotlessInput();
  ExtensionFilterHonorsLimit();
  LargestFilesSortsDescending();
  return 0;
}
