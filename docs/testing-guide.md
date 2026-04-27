# QuickSeek Testing

QuickSeek uses small assert-based C++ test executables registered with CTest.

## Running Tests

With presets:

```powershell
cmake --preset release
cmake --build --preset release
ctest --preset release --output-on-failure
```

Without presets:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

## Test Layout

```text
test/
  CMakeLists.txt
  format_tests.cpp       Tokenization, lowercase conversion, size formatting
  search_tests.cpp       Ranking, path matches, filters, sorting helpers
  index_tests.cpp        Root scope, .quickseekignore, programmatic ignore rules
  support/
    test_utils.h         Shared test record and temporary directory helpers
```

Each test source builds into its own executable. This keeps failures focused:

- `quickseek_format_tests`
- `quickseek_search_tests`
- `quickseek_index_tests`

## Test Style

The project currently uses standard `assert()` instead of an external testing framework. That keeps the dependency graph small while the project is still young. A future version can move to GoogleTest or Catch2 if richer diagnostics become useful.
