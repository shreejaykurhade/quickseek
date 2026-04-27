#pragma once

#include "file_record.h"

#include <filesystem>
#include <string>
#include <vector>

namespace quickseek {

struct IndexOptions {
  std::filesystem::path ignore_file_name = ".quickseekignore";
  std::vector<std::string> ignore_patterns;
};

IndexOptions LoadIndexOptions(const std::filesystem::path& root);

std::vector<FileRecord> BuildIndex(
    const std::filesystem::path& root,
    const IndexOptions& options = IndexOptions{});

}  // namespace quickseek
