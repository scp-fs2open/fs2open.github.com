# Module: cfile — `code/cfile/`

## Purpose
The engine's **virtual file system**. Provides `cfopen`/`cfread`/`cfwrite`/… that
transparently read from loose files *or* from inside **VP archives**. Files are
categorized by `CF_TYPE_*` (which map to directories like `maps`, `models`,
`tables`, `sounds`, `missions`). Implements the **mod layering** search order
(primary mod → secondary mods → game root → user dir).

## Key files
- `cfile.h` / `cfile.cpp` — public API, `CFILE` handle, path-type defines.
- `cfilesystem.cpp` / `cfilesystem.h` — directory/VP indexing and search order.
- `cfilearchive.cpp` / `cfilearchive.h` — VP archive reading.
- (Tools: `code/cfilearchiver/`, `code/cfileextractor/` build VP pack/unpack utilities.)

## Core data structures
- `struct CFILE` — opaque file handle (loose file or in-archive offset).
- `file_list_info`, `CFileLocation*` — listing and location results.

## Major constants
- Path types `CF_TYPE_*`: `ROOT`, `DATA`, `MAPS`, `TEXT`, `MODELS`, `TABLES`,
  `SOUNDS`, `VOICE*`, `MUSIC`, `MOVIES`, `INTERFACE`, `FONT`, `EFFECTS`, `HUD`,
  `PLAYERS`, `MISSIONS`, `CACHE`, `SCRIPTS`, `FICTION`, … (`CF_MAX_PATH_TYPES` = 37).
- `CF_TYPE_ANY` (-1), `CF_TYPE_INVALID` (0).
- Seek: `CF_SEEK_SET/CUR/END`. Sort: `CF_SORT_NAME/TIME/REVERSE`.
- Location flags: `CF_LOCATION_ROOT_GAME`, `CF_LOCATION_ROOT_USER`, mod-scope flags.

## Configuration tables
None — this *is* the layer every other module uses to find its tables and assets.

## See also
- `code/parse/parselo.*` (reads table text via this), every asset-loading module.
- Table option reference: https://wiki.hard-light.net/index.php/Tables
