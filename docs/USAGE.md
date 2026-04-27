# QuickSeek Usage

QuickSeek works in two phases:

1. Select a root folder and scan it.
2. Search the in-memory index created from that root.

It does not search the whole computer unless you explicitly choose a very large root.

If the selected root contains a `.quickseekignore` file, QuickSeek uses it to skip generated files, dependency folders, or anything else you do not want in the index.

## Starting QuickSeek

Start QuickSeek and choose a root when prompted:

```powershell
.\build\release\quickseek.exe
```

If you press Enter at the prompt, QuickSeek scans the current working directory.

```text
QuickSeek
Choose a folder to scan.
Press Enter for current folder: C:\Users\Shreejay\Desktop\cppsomething
Root >
```

Start directly with a specific root:

```powershell
.\build\release\quickseek.exe C:\Users\Shreejay\Documents
```

## Root Commands

Show the current root:

```text
Search > root
```

Change the root and rebuild the index:

```text
Search > root C:\Users\Shreejay\Desktop\cppsomething
```

Paths with spaces can be quoted:

```text
Search > root "C:\Users\Shreejay\Desktop\My Project"
```

Rebuild the index for the current root:

```text
Search > rescan
```

`cd <path>` is accepted as an alias for `root <path>`.

Changing the root discards the old index. All searches after that use the new root only.

## Search Commands

Search filenames and folders:

```text
Search > report
```

Show the largest files under the current root:

```text
Search > large
```

Show recently modified files under the current root:

```text
Search > recent
```

Filter by extension:

```text
Search > ext .cpp
Search > ext pdf
```

Exit:

```text
Search > exit
```

## Scope

The active root is the boundary. If the current root is:

```text
C:\Users\Shreejay\Desktop\cppsomething
```

QuickSeek indexes files inside that folder and its subfolders. Sibling folders on the Desktop are not included.

## Ignoring Files and Folders

Create a `.quickseekignore` file in the root you are scanning:

```text
# Generated build output
build/

# Version-control metadata
.git/

# Dependencies
node_modules/

# A single file name anywhere under the root
debug.log

# A relative folder
docs/private/
```

Rules:

- Empty lines are ignored.
- Lines starting with `#` are comments.
- A trailing `/` means the rule applies to directories.
- A rule without `/`, such as `build`, matches that name anywhere under the root.
- A rule with `/`, such as `docs/private/`, is treated as a path relative to the root.

QuickSeek reads `.quickseekignore` each time it scans. After editing the file, run:

```text
Search > rescan
```
