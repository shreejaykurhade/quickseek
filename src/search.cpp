#include "search.h"

#include <algorithm>

#include "format.h"

namespace quickseek {
namespace {

bool HasTokenMatch(const FileRecord& record, const std::string& query_token) {
  return std::any_of(record.tokens.begin(), record.tokens.end(),
                     [&](const std::string& token) {
                       return token == query_token ||
                              token.find(query_token) != std::string::npos;
                     });
}

}  // namespace

std::vector<SearchResult> SearchFiles(const std::vector<FileRecord>& index,
                                      const std::string& query) {
  const std::vector<std::string> query_tokens = Tokenize(query);
  std::vector<SearchResult> results;

  if (query_tokens.empty()) {
    return results;
  }

  for (const FileRecord& record : index) {
    const std::string lower_name = ToLower(record.name);
    int score = 0;
    std::vector<std::string> reasons;

    for (const std::string& token : query_tokens) {
      if (lower_name.rfind(token, 0) == 0) {
        score += 100;
        reasons.push_back("filename starts with '" + token + "'");
      } else if (lower_name.find(token) != std::string::npos) {
        score += 60;
        reasons.push_back("filename contains '" + token + "'");
      } else if (HasTokenMatch(record, token)) {
        score += 25;
        reasons.push_back("path contains '" + token + "'");
      }
    }

    if (score > 0) {
      results.push_back(
          {record.id, score, reasons.empty() ? "matched" : reasons.front()});
    }
  }

  std::sort(results.begin(), results.end(),
            [&](const SearchResult& a, const SearchResult& b) {
              if (a.score != b.score) {
                return a.score > b.score;
              }
              return index[a.file_id].name < index[b.file_id].name;
            });

  return results;
}

std::vector<FileRecord> LargestFiles(std::vector<FileRecord> index,
                                     std::size_t limit) {
  std::sort(index.begin(), index.end(),
            [](const FileRecord& a, const FileRecord& b) {
              return a.size > b.size;
            });
  if (index.size() > limit) {
    index.resize(limit);
  }
  return index;
}

std::vector<FileRecord> RecentFiles(std::vector<FileRecord> index,
                                    std::size_t limit) {
  std::sort(index.begin(), index.end(),
            [](const FileRecord& a, const FileRecord& b) {
              return a.modified > b.modified;
            });
  if (index.size() > limit) {
    index.resize(limit);
  }
  return index;
}

std::vector<FileRecord> FilesByExtension(const std::vector<FileRecord>& index,
                                         std::string extension,
                                         std::size_t limit) {
  extension = ToLower(extension);
  if (!extension.empty() && extension[0] != '.') {
    extension = "." + extension;
  }

  std::vector<FileRecord> matches;
  for (const FileRecord& file : index) {
    if (file.extension == extension) {
      matches.push_back(file);
      if (matches.size() == limit) {
        break;
      }
    }
  }
  return matches;
}

}  // namespace quickseek
