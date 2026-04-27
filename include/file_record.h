#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace quickseek {

struct FileRecord {
  int id = 0;
  std::filesystem::path path;
  std::string name;
  std::string extension;
  std::uintmax_t size = 0;
  std::filesystem::file_time_type modified;
  std::vector<std::string> tokens;
};

}  // namespace quickseek
