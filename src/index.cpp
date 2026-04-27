#include "quickseek/index.h"

#include <fstream>
#include <system_error>

#include "quickseek/format.h"

namespace quickseek {
namespace {

struct IgnoreRule {
  std::string pattern;
  bool directory_only = false;
  bool path_pattern = false;
};

std::string Trim(std::string text) {
  const auto first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return "";
  }

  const auto last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1);
}

std::string NormalizePattern(std::string pattern) {
  for (char& ch : pattern) {
    if (ch == '\\') {
      ch = '/';
    }
  }
  return ToLower(pattern);
}

std::string RelativePathString(const std::filesystem::path& path,
                               const std::filesystem::path& root) {
  std::error_code error;
  std::filesystem::path relative = std::filesystem::relative(path, root, error);
  if (error) {
    relative = path.filename();
  }
  return NormalizePattern(relative.generic_string());
}

std::vector<IgnoreRule> BuildIgnoreRules(const IndexOptions& options) {
  std::vector<IgnoreRule> rules;
  rules.reserve(options.ignore_patterns.size());

  for (std::string pattern : options.ignore_patterns) {
    pattern = Trim(pattern);
    if (pattern.empty() || pattern[0] == '#') {
      continue;
    }

    pattern = NormalizePattern(pattern);
    IgnoreRule rule;
    rule.directory_only = !pattern.empty() && pattern.back() == '/';
    if (rule.directory_only) {
      pattern.pop_back();
    }
    rule.path_pattern = pattern.find('/') != std::string::npos;
    rule.pattern = pattern;

    if (!rule.pattern.empty()) {
      rules.push_back(rule);
    }
  }

  return rules;
}

bool MatchesRule(const IgnoreRule& rule, const std::string& name,
                 const std::string& relative_path, bool is_directory) {
  if (rule.directory_only && !is_directory) {
    return false;
  }

  if (!rule.path_pattern) {
    return name == rule.pattern;
  }

  if (relative_path == rule.pattern) {
    return true;
  }

  return is_directory && relative_path.rfind(rule.pattern + "/", 0) == 0;
}

bool ShouldIgnoreEntry(const std::filesystem::directory_entry& entry,
                       const std::filesystem::path& root,
                       const std::vector<IgnoreRule>& rules) {
  std::error_code error;
  const bool is_directory = entry.is_directory(error);
  const std::string name = NormalizePattern(entry.path().filename().string());
  const std::string relative_path = RelativePathString(entry.path(), root);

  for (const IgnoreRule& rule : rules) {
    if (MatchesRule(rule, name, relative_path, is_directory)) {
      return true;
    }
  }

  return false;
}

}  // namespace

IndexOptions LoadIndexOptions(const std::filesystem::path& root) {
  IndexOptions options;
  const std::filesystem::path ignore_file = root / options.ignore_file_name;

  std::ifstream input(ignore_file);
  std::string line;
  while (std::getline(input, line)) {
    options.ignore_patterns.push_back(line);
  }

  return options;
}

std::vector<FileRecord> BuildIndex(const std::filesystem::path& root,
                                   const IndexOptions& options) {
  std::vector<FileRecord> index;
  int next_id = 0;
  IndexOptions effective_options = options;
  if (!effective_options.ignore_file_name.empty()) {
    const IndexOptions file_options = LoadIndexOptions(root);
    effective_options.ignore_patterns.insert(
        effective_options.ignore_patterns.end(),
        file_options.ignore_patterns.begin(), file_options.ignore_patterns.end());
  }
  const std::vector<IgnoreRule> ignore_rules = BuildIgnoreRules(effective_options);

  std::error_code error;
  std::filesystem::recursive_directory_iterator it(
      root, std::filesystem::directory_options::skip_permission_denied, error);
  std::filesystem::recursive_directory_iterator end;

  while (it != end) {
    error.clear();
    if (ShouldIgnoreEntry(*it, root, ignore_rules)) {
      if (it->is_directory(error)) {
        it.disable_recursion_pending();
      }
      it.increment(error);
      continue;
    }

    if (!it->is_regular_file(error)) {
      it.increment(error);
      continue;
    }

    FileRecord record;
    record.id = next_id++;
    record.path = it->path();
    record.name = record.path.filename().string();
    record.extension = ToLower(record.path.extension().string());
    record.size = it->file_size(error);
    record.modified = it->last_write_time(error);

    const std::string searchable =
        record.path.filename().string() + " " + record.path.parent_path().string();
    record.tokens = Tokenize(searchable);
    index.push_back(record);

    it.increment(error);
  }

  return index;
}

}  // namespace quickseek
