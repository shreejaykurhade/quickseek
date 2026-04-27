# Contributing to QuickSeek

## Requirements

- C++17 compiler (GCC 11+, Clang 14+, or MSVC 2022+)
- CMake 3.16 or newer
- Ninja (recommended) or another CMake-supported generator
- Git
- clang-format-18 (optional, for local formatting)
- clang-tidy (optional, for static analysis)

## Getting Started

Clone and install the git hooks:

```bash
git clone https://github.com/shreejaykurhade/quickseek.git
cd quickseek
bash scripts/install-hooks.sh
```

The pre-commit hook runs `clang-format` on every staged `.cpp` and `.h` file before each commit. If any file is reformatted, the changes are re-staged automatically and the commit proceeds — no manual formatting step needed.

## Building

```powershell
cmake --preset release
cmake --build --preset release
```

For a debug build:

```powershell
cmake --preset debug
cmake --build --preset debug
```

## Running Tests

```powershell
ctest --preset release
```

## Code Formatting

Formatting is enforced by `config/.clang-format` (Google C++ style, 80-column limit).

The pre-commit hook handles this automatically. To format manually:

```bash
find src include tools test -name '*.cpp' -o -name '*.h' | \
xargs clang-format-18 --style=file:config/.clang-format -i
```

To check without modifying:

```bash
find src include tools test -name '*.cpp' -o -name '*.h' | \
xargs clang-format-18 --style=file:config/.clang-format --dry-run --Werror
```

## Static Analysis

Static analysis is configured in `config/.clang-tidy`. To run during the build:

```powershell
cmake --preset release -DQUICKSEEK_ENABLE_TIDY=ON
cmake --build --preset release
```

To run standalone on a single file:

```bash
clang-tidy --config-file=config/.clang-tidy src/index.cpp -- -std=c++17 -Iinclude
```

## CI Pipeline

GitHub Actions runs on every push and pull request to `main`.

| Job | Platform | Configs |
|---|---|---|
| `build-linux` | Ubuntu 24.04 + GCC + Ninja | Debug, Release |
| `build-windows` | Windows 2025 + MSVC | Debug, Release |
| `format-check` | Ubuntu 24.04 | clang-format-18 dry-run |

The format-check job will fail if any source file does not match `config/.clang-format`. Install the pre-commit hook to prevent this locally.

## Installing

To install the binary, library, and headers to a local prefix:

```powershell
cmake --install build/release --prefix C:/path/to/install
```

This places:
- `quickseek.exe` → `bin/`
- `libquickseek.a` → `lib/`
- Headers → `include/quickseek/`
- CMake package config → `lib/cmake/quickseek/`

After installing, downstream CMake projects can use:

```cmake
find_package(quickseek REQUIRED)
target_link_libraries(my_target PRIVATE quickseek::quickseek)
```

## Packaging

To produce a distributable `.zip` and `.tar.gz` archive:

```powershell
cmake --build build/release --target package
```

## Project Layout

```text
quickseek/
  CMakeLists.txt              Root CMake build file
  CMakePresets.json           Debug and release presets
  LICENSE                     MIT license
  include/                    Public headers
  src/                        Library implementation
  tools/                      CLI frontend
  test/                       CTest test executables
  test/support/               Shared test helpers
  config/
    .clang-format             Code style config
    .clang-tidy               Static analysis config
  cmake/
    quickseekConfig.cmake.in  find_package() template
  scripts/
    pre-commit                Git pre-commit hook
    install-hooks.sh          Hook installer
  docs/                       Project documentation
  .github/workflows/
    ci.yml                    GitHub Actions CI pipeline
```
