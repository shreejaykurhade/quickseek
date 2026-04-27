#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "quickseek/format.h"
#include "quickseek/index.h"
#include "quickseek/search.h"

namespace fs = std::filesystem;

namespace {

std::string Trim(std::string text) {
  const auto first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return "";
  }

  const auto last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1);
}

std::string StripMatchingQuotes(std::string text) {
  text = Trim(text);
  if (text.size() >= 2 &&
      ((text.front() == '"' && text.back() == '"') ||
       (text.front() == '\'' && text.back() == '\''))) {
    return text.substr(1, text.size() - 2);
  }
  return text;
}

void PrintHelp() {
  std::cout << "\nCommands:\n";
  std::cout << "  any words        search filenames and folders\n";
  std::cout << "  large            show 10 biggest files\n";
  std::cout << "  recent           show 10 most recently changed files\n";
  std::cout << "  ext .cpp         show files with an extension\n";
  std::cout << "  root             show current search root\n";
  std::cout << "  root <path>      set search root and rebuild index\n";
  std::cout << "  rescan           rebuild index for current root\n";
  std::cout << "  help             show this help\n";
  std::cout << "  exit             quit\n\n";
}

fs::path PromptForInitialRoot() {
  std::cout << "Choose a folder to scan.\n";
  std::cout << "Press Enter for current folder: "
            << fs::current_path().string() << "\n";
  std::cout << "Root > ";

  std::string input;
  if (!std::getline(std::cin, input)) {
    return {};
  }

  input = StripMatchingQuotes(input);
  if (input.empty()) {
    return fs::current_path();
  }

  return fs::path(input);
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

bool RebuildIndex(const fs::path& requested_root, fs::path& current_root,
                  std::vector<quickseek::FileRecord>& index) {
  std::error_code error;
  const fs::path absolute_root = fs::weakly_canonical(requested_root, error);
  const fs::path root = error ? fs::absolute(requested_root) : absolute_root;

  if (!fs::exists(root) || !fs::is_directory(root)) {
    std::cerr << "Folder does not exist: " << root.string() << "\n";
    return false;
  }

  std::cout << "Indexing: " << root.string() << "\n";
  const auto start = std::chrono::steady_clock::now();
  std::vector<quickseek::FileRecord> next_index = quickseek::BuildIndex(root);
  const auto end = std::chrono::steady_clock::now();

  const auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  current_root = root;
  index = std::move(next_index);

  std::cout << "Indexed " << index.size() << " files in " << millis << " ms.\n";
  return true;
}

}  // namespace

int main(int argc, char* argv[]) {
  fs::path current_root;
  std::vector<quickseek::FileRecord> index;

  std::cout << "QuickSeek\n";
  const fs::path initial_root =
      argc > 1 ? fs::path(argv[1]) : PromptForInitialRoot();
  if (initial_root.empty()) {
    return 1;
  }
  if (!RebuildIndex(initial_root, current_root, index)) {
    return 1;
  }
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
    if (command == "root" || command == "pwd") {
      std::cout << "Current root: " << current_root.string() << "\n";
      std::cout << "Indexed files: " << index.size() << "\n";
      continue;
    }
    if (command == "rescan") {
      RebuildIndex(current_root, current_root, index);
      continue;
    }
    if (command.rfind("root ", 0) == 0 || command.rfind("cd ", 0) == 0) {
      const std::size_t command_size = command.rfind("root ", 0) == 0 ? 5 : 3;
      const std::string path_text = StripMatchingQuotes(input.substr(command_size));
      if (path_text.empty()) {
        std::cout << "Usage: root <path>\n";
        continue;
      }
      RebuildIndex(fs::path(path_text), current_root, index);
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
