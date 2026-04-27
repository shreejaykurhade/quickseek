#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct FileRecord {
    int id = 0;
    fs::path path;
    std::string name;
    std::string extension;
    uintmax_t size = 0;
    fs::file_time_type modified;
    std::vector<std::string> tokens;
};

struct SearchResult {
    int fileId = 0;
    int score = 0;
    std::string reason;
};

std::string toLower(std::string text) {
    for (char& ch : text) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return text;
}

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;

    for (char ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            current += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

std::string formatSize(uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    double value = static_cast<double>(bytes);
    int unit = 0;

    while (value >= 1024.0 && unit < 3) {
        value /= 1024.0;
        ++unit;
    }

    std::ostringstream out;
    out << std::fixed << std::setprecision(unit == 0 ? 0 : 1) << value << " " << units[unit];
    return out.str();
}

std::vector<FileRecord> buildIndex(const fs::path& root) {
    std::vector<FileRecord> index;
    int nextId = 0;

    fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied);
    fs::recursive_directory_iterator end;

    for (; it != end; ++it) {
        std::error_code error;
        if (!it->is_regular_file(error)) {
            continue;
        }

        FileRecord record;
        record.id = nextId++;
        record.path = it->path();
        record.name = record.path.filename().string();
        record.extension = toLower(record.path.extension().string());
        record.size = it->file_size(error);
        record.modified = it->last_write_time(error);

        std::string searchable = record.path.filename().string() + " " + record.path.parent_path().string();
        record.tokens = tokenize(searchable);
        index.push_back(record);
    }

    return index;
}

bool hasTokenMatch(const FileRecord& record, const std::string& queryToken) {
    return std::any_of(record.tokens.begin(), record.tokens.end(), [&](const std::string& token) {
        return token == queryToken || token.find(queryToken) != std::string::npos;
    });
}

std::vector<SearchResult> searchFiles(const std::vector<FileRecord>& index, const std::string& query) {
    std::vector<std::string> queryTokens = tokenize(query);
    std::vector<SearchResult> results;

    if (queryTokens.empty()) {
        return results;
    }

    for (const FileRecord& record : index) {
        std::string lowerName = toLower(record.name);
        int score = 0;
        std::vector<std::string> reasons;

        for (const std::string& token : queryTokens) {
            if (lowerName.rfind(token, 0) == 0) {
                score += 100;
                reasons.push_back("filename starts with '" + token + "'");
            } else if (lowerName.find(token) != std::string::npos) {
                score += 60;
                reasons.push_back("filename contains '" + token + "'");
            } else if (hasTokenMatch(record, token)) {
                score += 25;
                reasons.push_back("path contains '" + token + "'");
            }
        }

        if (score > 0) {
            results.push_back({record.id, score, reasons.empty() ? "matched" : reasons.front()});
        }
    }

    std::sort(results.begin(), results.end(), [&](const SearchResult& a, const SearchResult& b) {
        if (a.score != b.score) {
            return a.score > b.score;
        }
        return index[a.fileId].name < index[b.fileId].name;
    });

    return results;
}

void printResults(const std::vector<FileRecord>& index, const std::vector<SearchResult>& results) {
    if (results.empty()) {
        std::cout << "No results.\n";
        return;
    }

    int shown = 0;
    for (const SearchResult& result : results) {
        if (shown == 10) {
            break;
        }

        const FileRecord& file = index[result.fileId];
        std::cout << shown + 1 << ". " << file.name << "\n";
        std::cout << "   " << file.path.parent_path().string() << "\n";
        std::cout << "   " << formatSize(file.size) << " | score " << result.score << " | " << result.reason << "\n";
        ++shown;
    }
}

void printLargeFiles(std::vector<FileRecord> index) {
    std::sort(index.begin(), index.end(), [](const FileRecord& a, const FileRecord& b) {
        return a.size > b.size;
    });

    for (size_t i = 0; i < index.size() && i < 10; ++i) {
        std::cout << i + 1 << ". " << index[i].name << " | " << formatSize(index[i].size) << "\n";
        std::cout << "   " << index[i].path.string() << "\n";
    }
}

void printRecentFiles(std::vector<FileRecord> index) {
    std::sort(index.begin(), index.end(), [](const FileRecord& a, const FileRecord& b) {
        return a.modified > b.modified;
    });

    for (size_t i = 0; i < index.size() && i < 10; ++i) {
        std::cout << i + 1 << ". " << index[i].name << " | " << formatSize(index[i].size) << "\n";
        std::cout << "   " << index[i].path.string() << "\n";
    }
}

void printHelp() {
    std::cout << "\nCommands:\n";
    std::cout << "  any words        search filenames and folders\n";
    std::cout << "  large            show 10 biggest files\n";
    std::cout << "  recent           show 10 most recently changed files\n";
    std::cout << "  ext .cpp         show files with an extension\n";
    std::cout << "  help             show this help\n";
    std::cout << "  exit             quit\n\n";
}

void printExtensionFiles(const std::vector<FileRecord>& index, std::string extension) {
    extension = toLower(extension);
    if (!extension.empty() && extension[0] != '.') {
        extension = "." + extension;
    }

    int shown = 0;
    for (const FileRecord& file : index) {
        if (file.extension == extension) {
            std::cout << shown + 1 << ". " << file.name << " | " << formatSize(file.size) << "\n";
            std::cout << "   " << file.path.string() << "\n";
            ++shown;
            if (shown == 10) {
                break;
            }
        }
    }

    if (shown == 0) {
        std::cout << "No files with extension " << extension << ".\n";
    }
}

int main(int argc, char* argv[]) {
    fs::path root = argc > 1 ? fs::path(argv[1]) : fs::current_path();

    if (!fs::exists(root) || !fs::is_directory(root)) {
        std::cerr << "Folder does not exist: " << root.string() << "\n";
        return 1;
    }

    std::cout << "QuickSeek\n";
    std::cout << "Indexing: " << fs::absolute(root).string() << "\n";

    auto start = std::chrono::steady_clock::now();
    std::vector<FileRecord> index = buildIndex(root);
    auto end = std::chrono::steady_clock::now();

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Indexed " << index.size() << " files in " << millis << " ms.\n";
    printHelp();

    std::string input;
    while (true) {
        std::cout << "Search > ";
        if (!std::getline(std::cin, input)) {
            break;
        }

        std::string command = toLower(input);
        if (command == "exit" || command == "quit") {
            break;
        }
        if (command == "help") {
            printHelp();
            continue;
        }
        if (command == "large") {
            printLargeFiles(index);
            continue;
        }
        if (command == "recent") {
            printRecentFiles(index);
            continue;
        }
        if (command.rfind("ext ", 0) == 0) {
            printExtensionFiles(index, input.substr(4));
            continue;
        }

        printResults(index, searchFiles(index, input));
    }

    return 0;
}
