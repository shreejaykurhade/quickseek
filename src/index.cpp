#include "quickseek/index.h"

#include <system_error>

#include "quickseek/format.h"

namespace quickseek {

std::vector<FileRecord> BuildIndex(const std::filesystem::path& root) {
  std::vector<FileRecord> index;
  int next_id = 0;

  std::error_code error;
  std::filesystem::recursive_directory_iterator it(
      root, std::filesystem::directory_options::skip_permission_denied, error);
  std::filesystem::recursive_directory_iterator end;

  while (it != end) {
    error.clear();
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
