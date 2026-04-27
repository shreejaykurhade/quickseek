#include "quickseek/index.h"

#include <system_error>

#include "quickseek/format.h"

namespace quickseek {

std::vector<FileRecord> BuildIndex(const std::filesystem::path& root) {
  std::vector<FileRecord> index;
  int next_id = 0;

  std::filesystem::recursive_directory_iterator it(
      root, std::filesystem::directory_options::skip_permission_denied);
  std::filesystem::recursive_directory_iterator end;

  for (; it != end; ++it) {
    std::error_code error;
    if (!it->is_regular_file(error)) {
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
  }

  return index;
}

}  // namespace quickseek
