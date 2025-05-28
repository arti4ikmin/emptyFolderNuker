# Empty Folder Nuker but Linux
### My first project made on linux for linux yessir.

A kinda very simple project. Operates simply on recursive calling.

Following setup:
```
Usage: emptyFolderNuker <dir> [options]

Args:
  <dir>         The starting dir to scan.

Options:
--help              Show this help msg and exits
--dry-run           Show what would be deleted without actually deleting yet
--verbose, -v       Print more info about actions taken
--interactive, -i   Ask for confirmation before deleting each one (kinda bad for larger amounts)
--min-depth <N>     Only consider folders at or deeper than N (root is 0)
--max-depth <N>     Only consider folders at or "superficial" than N
```

