# QuickSeek

QuickSeek is a small C++ terminal search tool. It scans a folder, builds an in-memory index of file names and paths, then lets you search that index quickly.

The project layout borrows the useful small-library shape from Google Benchmark:

```text
quickseek/
  CMakeLists.txt
  CMakePresets.json
  include/quickseek/     public headers
  src/                   library implementation
  tools/                 command-line executable
  test/                  tests run by CTest
```

## Core Idea

The program has four simple parts:

1. **Crawler**
   - Walks through a folder recursively.
   - Collects file path, name, extension, size, and modified time.

2. **Tokenizer**
   - Breaks filenames and folder paths into lowercase searchable words.
   - Example: `Final_Report_2026.pdf` becomes `final`, `report`, `2026`, `pdf`.

3. **Index**
   - Stores one `FileRecord` per file.
   - This first version keeps the index in memory while the app runs.

4. **Ranker**
   - Gives better scores to stronger matches.
   - Filename prefix match scores higher than folder/path match.

## Necessary First-Version Features

- Recursive folder scan
- Filename and folder search
- Ranked top 10 results
- `large` command for biggest files
- `recent` command for recently modified files
- `ext .cpp` style extension filter

## Run

Configure and build with the preset:

```powershell
cmake --preset release
cmake --build --preset release
```

Search the current folder:

```powershell
.\build\release\quickseek.exe
```

Search another folder:

```powershell
.\build\release\quickseek.exe C:\Users\Shreejay\Documents
```

Run tests:

```powershell
ctest --preset release
```

Manual CMake build without presets:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release
```

## Example

```text
Search > report
Search > large
Search > recent
Search > ext .pdf
Search > exit
```

## Good Next Features

- Save the index to disk so startup is faster.
- Search inside text files such as `.txt`, `.cpp`, `.md`, and `.log`.
- Add fuzzy matching so `resm` can find `resume`.
- Add an `open 1` command to open a selected file.
