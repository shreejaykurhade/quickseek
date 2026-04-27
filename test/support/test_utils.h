#pragma once

#include "file_record.h"
#include "format.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace quickseek::test {

inline FileRecord MakeRecord(int id, std::string path, std::uintmax_t size) {
  FileRecord record;
  record.id = id;
  record.path = std::filesystem::path(path);
  record.name = record.path.filename().string();
  record.extension = ToLower(record.path.extension().string());
  record.size = size;
  record.tokens = Tokenize(record.path.string());
  return record;
}

class ScopedTestDirectory {
 public:
  explicit ScopedTestDirectory(std::string name)
      : path_(std::filesystem::temp_directory_path() / name) {
    std::error_code error;
    std::filesystem::remove_all(path_, error);
    std::filesystem::create_directories(path_);
  }

  ScopedTestDirectory(const ScopedTestDirectory&) = delete;
  ScopedTestDirectory& operator=(const ScopedTestDirectory&) = delete;

  ~ScopedTestDirectory() {
    std::error_code error;
    std::filesystem::remove_all(path_, error);
  }

  const std::filesystem::path& path() const { return path_; }

  void CreateFile(const std::filesystem::path& relative_path,
                  const std::string& contents = "") const {
    const std::filesystem::path full_path = path_ / relative_path;
    std::filesystem::create_directories(full_path.parent_path());
    std::ofstream(full_path) << contents;
  }

 private:
  std::filesystem::path path_;
};

}  // namespace quickseek::test
