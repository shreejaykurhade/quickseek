# QuickSeek Design

QuickSeek is a small C++17 library plus a command-line frontend.

## Architecture

```mermaid
graph LR
    subgraph tools ["tools/"]
        CLI["quickseek_cli.cpp"]
    end
    subgraph src ["src/"]
        FMT["format.cpp<br/>Tokenize · ToLower · FormatSize"]
        IDX["index.cpp<br/>BuildIndex · LoadIndexOptions"]
        SCH["search.cpp<br/>SearchFiles · LargestFiles<br/>RecentFiles · FilesByExtension"]
    end
    subgraph inc ["include/"]
        FR["file_record.h<br/>FileRecord"]
        FH["format.h"]
        IH["index.h<br/>IndexOptions"]
        SH["search.h<br/>SearchResult"]
    end
    subgraph test ["test/"]
        FT["format_tests.cpp"]
        IT["index_tests.cpp"]
        ST["search_tests.cpp"]
        TU["support/test_utils.h"]
    end
    CLI --> FH
    CLI --> IH
    CLI --> SH
    FMT --> FH
    IDX --> IH
    IDX --> FH
    SCH --> SH
    SCH --> FH
    IH --> FR
    SH --> FR
    FT --> FH
    IT --> IH
    ST --> SH
    TU --> FR
    TU --> FH
```

The CLI is intentionally thin. It owns the active root and the current index, while the library provides indexing, tokenization, formatting, and search behavior. If no root is passed on the command line, the CLI prompts for one before scanning.

## Indexing

`BuildIndex(root)` recursively walks the selected root and creates a `FileRecord` for each regular file.

The scanner can prune files and directories using rules loaded from `.quickseekignore` in the selected root. There are no built-in ignored directory names; the ignore file is the source of truth.

Directory pruning happens during traversal with `disable_recursion_pending()`, so ignored folders are not scanned at all.

Each record contains:

- numeric id
- full path
- filename
- extension
- file size
- modification time
- lowercase searchable tokens

The index is currently stored in memory. Changing roots or running `rescan` rebuilds it.

`IndexOptions` can be used by callers to provide ignore patterns directly or to change the ignore file name.

```mermaid
flowchart TD
    A(["BuildIndex(root, options)"])
    A --> B["LoadIndexOptions(root)\nread .quickseekignore"]
    B --> C["BuildIgnoreRules(options)"]
    C --> D{"iterator != end?"}
    D -- no --> J(["return FileRecord vector"])
    D -- yes --> E{"ShouldIgnoreEntry?"}
    E -- "yes, directory" --> F["disable_recursion_pending()\nskip entire subtree"]
    F --> ADV["it.increment()"]
    E -- "yes, file" --> ADV
    E -- no --> G{"is_regular_file?"}
    G -- no --> ADV
    G -- yes --> H["Create FileRecord\npath · name · ext · size · mtime"]
    H --> I["Tokenize(filename + parent_path)\nappend to index"]
    I --> ADV
    ADV --> D
```

## Ignore Rules

Ignore rules are line based:

- empty lines are skipped
- lines starting with `#` are comments
- `build/` ignores a directory named `build`
- `debug.log` ignores any file or directory with that name
- `docs/private/` ignores a relative directory under the root

Patterns are matched case-insensitively.

```mermaid
flowchart TD
    A(["ShouldIgnoreEntry(entry, root, rules)"])
    A --> B["normalize name, relative_path, is_directory"]
    B --> C{"more rules to check?"}
    C -- no --> J(["return false — keep entry"])
    C -- yes --> D["get next IgnoreRule"]
    D --> E{"rule.directory_only\nAND NOT is_directory?"}
    E -- yes --> C
    E -- no --> F{"rule.path_pattern?"}
    F -- no --> G{"name == rule.pattern?"}
    G -- yes --> K(["return true — ignore entry"])
    G -- no --> C
    F -- yes --> H{"relative_path == rule.pattern?"}
    H -- yes --> K
    H -- no --> I{"is_directory AND\npath starts with pattern/"}
    I -- yes --> K
    I -- no --> C
```

## Search

`SearchFiles(index, query)` tokenizes the query and scores each record:

- filename starts with token: high score
- filename contains token: medium score
- path token contains token: lower score

Results are sorted by score, then filename.

```mermaid
flowchart TD
    A(["SearchFiles(index, query)"])
    A --> B["Tokenize(query)"]
    B --> C{"tokens empty?"}
    C -- yes --> Z(["return empty vector"])
    C -- no --> D{"more FileRecords\nto score?"}
    D -- no --> Q["sort desc by score\nthen asc by filename"]
    Q --> R(["return SearchResult vector"])
    D -- yes --> E["next FileRecord\nscore = 0"]
    E --> F{"more query tokens?"}
    F -- no --> N{"score > 0?"}
    N -- yes --> O["append SearchResult\nfile_id · score · reason"]
    N -- no --> D
    O --> D
    F -- yes --> G["next token"]
    G --> H{"filename starts\nwith token?"}
    H -- yes --> I["score += 100\nreason: filename starts with"]
    H -- no --> J{"filename contains\ntoken?"}
    J -- yes --> L["score += 60\nreason: filename contains"]
    J -- no --> K{"any path token\ncontains token?"}
    K -- yes --> M["score += 25\nreason: path contains"]
    K -- no --> F
    I --> F
    L --> F
    M --> F
```

## Root Scope

The selected root defines the search boundary. QuickSeek does not scan sibling folders unless the user sets a higher root that contains them.

For example:

```text
root C:\Users\<you>\Desktop\cppsomething
```

Only files under `cppsomething` are indexed.

## Future Work

- Persistent index file
- Incremental rescan
- File content search
- Fuzzy matching
- Benchmarks for scan and query time
