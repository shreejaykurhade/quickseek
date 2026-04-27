# QuickSeek

QuickSeek is a small C++17 command-line search tool for local files. It scans a directory, builds an in-memory index of filenames and paths, and returns ranked results from an interactive prompt.

The project is intentionally simple, but it is structured like a real C++ library: public headers, private implementation files, a CLI target, CMake presets, and tests.

## Features

- Recursive directory indexing
- Filename and folder-path search
- Ranked results with match reasons
- Largest-file listing
- Recently modified file listing
- Extension filtering, such as `ext .cpp`
- CTest-based test target

## Requirements

- C++17 compiler
- CMake 3.16 or newer
- Ninja, Make, or another CMake-supported generator

This project is currently developed and tested with GCC via MSYS2 on Windows.

## Building

The recommended build path uses CMake presets.

```powershell
cmake --preset release
cmake --build --preset release
```

The executable is written to:

```text
build/release/quickseek.exe
```

For a debug build:

```powershell
cmake --preset debug
cmake --build --preset debug
```

## Running

Search the current directory:

```powershell
.\build\release\quickseek.exe
```

Search a specific directory:

```powershell
.\build\release\quickseek.exe C:\Users\Shreejay\Documents
```

Inside the prompt:

```text
Search > report
Search > large
Search > recent
Search > ext .pdf
Search > help
Search > exit
```

## Testing

Run the test suite with CTest:

```powershell
ctest --preset release
```

The current tests cover tokenization, ranking behavior, and extension filtering.

## Project Layout

```text
quickseek/
  CMakeLists.txt          Root CMake build file
  CMakePresets.json       Debug and release configure/build/test presets
  include/quickseek/      Public headers
  src/                    Library implementation
  tools/                  Command-line frontend
  test/                   CTest test executable
```

The layout follows the same broad organization used by mature C++ projects such as [Google Benchmark](https://github.com/google/benchmark): public API in `include/`, implementation in `src/`, tests in `test/`, and CMake as the main build entry point.

## Design

QuickSeek is split into a reusable library and a thin command-line tool.

`BuildIndex()` walks the target directory recursively and creates one `FileRecord` per regular file. Each record stores the file path, name, extension, size, modification time, and searchable tokens.

`Tokenize()` lowercases names and paths, then splits them into alphanumeric words. For example:

```text
Final_Report_2026.pdf -> final, report, 2026, pdf
```

`SearchFiles()` scores records against a query. Filename prefix matches are ranked highest, filename substring matches come next, and path-token matches are ranked lower. Results are sorted by score before display.

## Manual CMake Flow

If you do not want to use presets:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release
```

## Roadmap

- Persist the index to disk for faster startup
- Search inside text files such as `.txt`, `.cpp`, `.md`, and `.log`
- Add fuzzy matching for typo-tolerant search
- Add `open <n>` and `folder <n>` commands for result actions
- Add benchmarks for indexing and query speed

## License

No license has been selected yet.
