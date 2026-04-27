# QuickSeek Usage

QuickSeek works in two phases:

1. Select a root folder and scan it.
2. Search the in-memory index created from that root.

It does not search the whole computer unless you explicitly choose a very large root.

## Starting QuickSeek

Start with the default root, which is your Desktop on Windows:

```powershell
.\build\release\quickseek.exe
```

Start with a specific root:

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
