#pragma once

#include <string>
#include <vector>

#include "file_record.h"

namespace quickseek {

struct SearchResult {
  int file_id = 0;
  int score = 0;
  std::string reason;
};

std::vector<SearchResult> SearchFiles(const std::vector<FileRecord>& index,
                                      const std::string& query);

std::vector<FileRecord> LargestFiles(std::vector<FileRecord> index,
                                     std::size_t limit);

std::vector<FileRecord> RecentFiles(std::vector<FileRecord> index,
                                    std::size_t limit);

std::vector<FileRecord> FilesByExtension(const std::vector<FileRecord>& index,
                                         std::string extension,
                                         std::size_t limit);

}  // namespace quickseek
