# QuickSeek Design

QuickSeek is a small C++17 library plus a command-line frontend.

## Architecture

```text
include/quickseek/      Public API
src/                    Implementation
tools/quickseek_cli.cpp Interactive CLI
test/                   CTest executable
```

The CLI is intentionally thin. It owns the active root and the current index, while the library provides indexing, tokenization, formatting, and search behavior.

## Indexing

`BuildIndex(root)` recursively walks the selected root and creates a `FileRecord` for each regular file.

Each record contains:

- numeric id
- full path
- filename
- extension
- file size
- modification time
- lowercase searchable tokens

The index is currently stored in memory. Changing roots or running `rescan` rebuilds it.

## Search

`SearchFiles(index, query)` tokenizes the query and scores each record:

- filename starts with token: high score
- filename contains token: medium score
- path token contains token: lower score

Results are sorted by score, then filename.

## Root Scope

The selected root defines the search boundary. QuickSeek does not scan sibling folders unless the user sets a higher root that contains them.

For example:

```text
root C:\Users\Shreejay\Desktop\cppsomething
```

Only files under `cppsomething` are indexed.

## Future Work

- Persistent index file
- Incremental rescan
- File content search
- Fuzzy matching
- Benchmarks for scan and query time
