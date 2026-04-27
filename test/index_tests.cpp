#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include "index.h"
#include "test_utils.h"

namespace {

std::vector<std::string> FileNames(const std::vector<quickseek::FileRecord>& index) {
  std::vector<std::string> names;
  for (const quickseek::FileRecord& file : index) {
    names.push_back(file.name);
  }
  return names;
}

bool ContainsName(const std::vector<std::string>& names, const std::string& name) {
  return std::find(names.begin(), names.end(), name) != names.end();
}

void BuildIndexStaysInsideRequestedRoot() {
  quickseek::test::ScopedTestDirectory base("quickseek_index_scope_test");
  const std::filesystem::path root = base.path() / "root";
  const std::filesystem::path sibling = base.path() / "sibling";

  std::filesystem::create_directories(root / "nested");
  std::filesystem::create_directories(sibling);
  base.CreateFile("root/alpha.txt", "alpha");
  base.CreateFile("root/nested/beta.md", "beta");
  base.CreateFile("sibling/outside.txt", "outside");

  const std::vector<quickseek::FileRecord> index = quickseek::BuildIndex(root);

  assert(index.size() == 2);
  for (const quickseek::FileRecord& file : index) {
    assert(file.path.string().find(sibling.string()) == std::string::npos);
  }
}

void BuildIndexScansEverythingWithoutIgnoreFile() {
  quickseek::test::ScopedTestDirectory base("quickseek_no_ignore_test");
  base.CreateFile("src/main.cpp", "main");
  base.CreateFile("build/generated.obj", "generated");
  base.CreateFile(".git/HEAD", "ref");

  const std::vector<quickseek::FileRecord> index =
      quickseek::BuildIndex(base.path());

  assert(index.size() == 3);
}

void BuildIndexUsesQuickSeekIgnoreFile() {
  quickseek::test::ScopedTestDirectory base("quickseek_ignore_file_test");
  base.CreateFile(".quickseekignore", "build/\nvendor/\nskip.txt\n");
  base.CreateFile("build/generated.obj", "generated");
  base.CreateFile("vendor/library.hpp", "vendor");
  base.CreateFile("src/app.cpp", "app");
  base.CreateFile("skip.txt", "skip");

  const std::vector<std::string> names =
      FileNames(quickseek::BuildIndex(base.path()));

  assert(ContainsName(names, ".quickseekignore"));
  assert(ContainsName(names, "app.cpp"));
  assert(!ContainsName(names, "generated.obj"));
  assert(!ContainsName(names, "library.hpp"));
  assert(!ContainsName(names, "skip.txt"));
}

void BuildIndexSupportsRelativeIgnorePath() {
  quickseek::test::ScopedTestDirectory base("quickseek_relative_ignore_test");
  base.CreateFile(".quickseekignore", "docs/private/\n");
  base.CreateFile("docs/public/readme.md", "public");
  base.CreateFile("docs/private/secret.md", "secret");

  const std::vector<std::string> names =
      FileNames(quickseek::BuildIndex(base.path()));

  assert(ContainsName(names, "readme.md"));
  assert(!ContainsName(names, "secret.md"));
}

void BuildIndexSupportsProgrammaticIgnorePatterns() {
  quickseek::test::ScopedTestDirectory base("quickseek_programmatic_ignore_test");
  base.CreateFile("cache/data.bin", "cache");
  base.CreateFile("src/app.cpp", "app");

  quickseek::IndexOptions options;
  options.ignore_file_name.clear();
  options.ignore_patterns.push_back("cache/");

  const std::vector<std::string> names =
      FileNames(quickseek::BuildIndex(base.path(), options));

  assert(ContainsName(names, "app.cpp"));
  assert(!ContainsName(names, "data.bin"));
}

}  // namespace

int main() {
  BuildIndexStaysInsideRequestedRoot();
  BuildIndexScansEverythingWithoutIgnoreFile();
  BuildIndexUsesQuickSeekIgnoreFile();
  BuildIndexSupportsRelativeIgnorePath();
  BuildIndexSupportsProgrammaticIgnorePatterns();
  return 0;
}
