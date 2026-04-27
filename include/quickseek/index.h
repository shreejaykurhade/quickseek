#pragma once

#include <filesystem>
#include <vector>

#include "quickseek/file_record.h"

namespace quickseek {

std::vector<FileRecord> BuildIndex(const std::filesystem::path& root);

}  // namespace quickseek
