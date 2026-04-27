# QuickSeek Design

QuickSeek is a small C++17 library plus a command-line frontend.

## Architecture

```mermaid
graph LR
    subgraph FE ["tools/"]
        CLI["quickseek_cli.cpp"]
    end
    subgraph LIB ["src/ — library"]
        FMT["format.cpp\nTokenize · ToLower · FormatSize"]
        IDX["index.cpp\nBuildIndex · LoadIndexOptions"]
        SCH["search.cpp\nSearchFiles · LargestFiles\nRecentFiles · FilesByExtension"]
    end
    subgraph INC ["include/"]
        FR["file_record.h\nFileRecord"]
        FH["format.h"]
        IH["index.h\nIndexOptions"]
        SH["search.h\nSearchResult"]
    end
    subgraph TST ["test/"]
        FT["format_tests.cpp"]
        IT["index_tests.cpp"]
        ST["search_tests.cpp"]
        TU["support/test_utils.h"]
    end
    CLI --> IDX
    CLI --> SCH
    CLI --> FMT
    IDX --> FMT
    SCH --> FMT
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
    A --> B["LoadIndexOptions(root)\nreads .quickseekignore"]
    B --> C["BuildIgnoreRules(options)"]
    C --> D["recursive_directory_iterator"]
    D --> E{ShouldIgnoreEntry?}
    E -- "yes, directory" --> F["disable_recursion_pending()\nskip entire subtree"]
    E -- "yes, file" --> D
    F --> D
    E -- no --> G{is_regular_file?}
    G -- no --> D
    G -- yes --> H["Create FileRecord\npath · name · ext · size · mtime"]
    H --> I["Tokenize(filename + parent_path)"]
    I --> D
    D -- exhausted --> J(["return vector&lt;FileRecord&gt;"])
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
    A --> B["resolve name, relative_path, is_directory"]
    B --> C["for each IgnoreRule"]
    C --> D{"rule.directory_only\nAND NOT is_directory?"}
    D -- yes --> C
    D -- no --> E{rule.path_pattern?}
    E -- no --> F{"name == rule.pattern?"}
    F -- yes --> G(["return true — ignore"])
    F -- no --> C
    E -- yes --> H{"relative_path == rule.pattern?"}
    H -- yes --> G
    H -- no --> I{"is_directory AND\npath starts with pattern/"}
    I -- yes --> G
    I -- no --> C
    C -- "no rule matched" --> J(["return false — keep"])
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
    B --> C{tokens empty?}
    C -- yes --> Z(["return empty"])
    C -- no --> D["for each FileRecord"]
    D --> E["score = 0"]
    E --> F["for each query token"]
    F --> G{"filename starts\nwith token?"}
    G -- yes --> H["score += 100\nreason: filename starts with"]
    G -- no --> I{"filename contains\ntoken?"}
    I -- yes --> J["score += 60\nreason: filename contains"]
    I -- no --> K{"any path token\ncontains token?"}
    K -- yes --> L["score += 25\nreason: path contains"]
    K -- no --> M
    H --> M{more tokens?}
    J --> M
    L --> M
    M -- yes --> F
    M -- no --> N{score > 0?}
    N -- yes --> O["append SearchResult\nfile_id · score · reason"]
    N -- no --> P{more records?}
    O --> P
    P -- yes --> D
    P -- no --> Q["sort desc by score\nthen asc by filename"]
    Q --> R(["return vector&lt;SearchResult&gt;"])
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
