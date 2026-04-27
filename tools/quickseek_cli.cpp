#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "quickseek/format.h"
#include "quickseek/index.h"
#include "quickseek/search.h"

namespace fs = std::filesystem;

namespace {

fs::path DefaultSearchRoot() {
  const char* user_profile = std::getenv("USERPROFILE");
  if (user_profile != nullptr) {
    fs::path desktop = fs::path(user_profile) / "Desktop";
    if (fs::exists(desktop) && fs::is_directory(desktop)) {
      return desktop;
    }
  }

  return fs::current_path();
}

void PrintHelp() {
  std::cout << "\nCommands:\n";
  std::cout << "  any words        search filenames and folders\n";
  std::cout << "  large            show 10 biggest files\n";
  std::cout << "  recent           show 10 most recently changed files\n";
  std::cout << "  ext .cpp         show files with an extension\n";
  std::cout << "  help             show this help\n";
  std::cout << "  exit             quit\n\n";
}

void PrintFiles(const std::vector<quickseek::FileRecord>& files) {
  if (files.empty()) {
    std::cout << "No results.\n";
    return;
  }

  for (std::size_t i = 0; i < files.size(); ++i) {
    std::cout << i + 1 << ". " << files[i].name << " | "
              << quickseek::FormatSize(files[i].size) << "\n";
    std::cout << "   " << files[i].path.string() << "\n";
  }
}

void PrintResults(const std::vector<quickseek::FileRecord>& index,
                  const std::vector<quickseek::SearchResult>& results) {
  if (results.empty()) {
    std::cout << "No results.\n";
    return;
  }

  const std::size_t shown = std::min<std::size_t>(results.size(), 10);
  for (std::size_t i = 0; i < shown; ++i) {
    const quickseek::SearchResult& result = results[i];
    const quickseek::FileRecord& file = index[result.file_id];
    std::cout << i + 1 << ". " << file.name << "\n";
    std::cout << "   " << file.path.parent_path().string() << "\n";
    std::cout << "   " << quickseek::FormatSize(file.size) << " | score "
              << result.score << " | " << result.reason << "\n";
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  const fs::path root = argc > 1 ? fs::path(argv[1]) : DefaultSearchRoot();

  if (!fs::exists(root) || !fs::is_directory(root)) {
    std::cerr << "Folder does not exist: " << root.string() << "\n";
    return 1;
  }

  std::cout << "QuickSeek\n";
  std::cout << "Indexing: " << fs::absolute(root).string() << "\n";

  const auto start = std::chrono::steady_clock::now();
  const std::vector<quickseek::FileRecord> index = quickseek::BuildIndex(root);
  const auto end = std::chrono::steady_clock::now();

  const auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  std::cout << "Indexed " << index.size() << " files in " << millis << " ms.\n";
  PrintHelp();

  std::string input;
  while (true) {
    std::cout << "Search > ";
    if (!std::getline(std::cin, input)) {
      break;
    }

    const std::string command = quickseek::ToLower(input);
    if (command == "exit" || command == "quit") {
      break;
    }
    if (command == "help") {
      PrintHelp();
      continue;
    }
    if (command == "large") {
      PrintFiles(quickseek::LargestFiles(index, 10));
      continue;
    }
    if (command == "recent") {
      PrintFiles(quickseek::RecentFiles(index, 10));
      continue;
    }
    if (command.rfind("ext ", 0) == 0) {
      PrintFiles(quickseek::FilesByExtension(index, input.substr(4), 10));
      continue;
    }

    PrintResults(index, quickseek::SearchFiles(index, input));
  }

  return 0;
}
