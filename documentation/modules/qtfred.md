# Module: qtfred — `qtfred/`

## Purpose
**qtFRED** is the cross-platform (Qt-based) reimplementation of the FRED mission
editor. Like FRED2 it is a separate executable that links the engine in `code/`,
but its GUI is built with **Qt** so it runs on Windows, Linux, and macOS. It edits
the same `.fs2` missions/campaigns (ships, wings, events/goals, briefings,
backgrounds, etc.).

## Build
- Built when `FSO_BUILD_QTFRED=ON` (off by default; see top-level `CMakeLists.txt`).
- Requires Qt5 (`Qt5_DIR`). Target: `qtfred`.

## Layout
- `src/main.cpp`, `qmain.cpp`, `FredApplication.*` — application entry/bootstrap.
- `src/mission/` — editor-side mission model and engine glue.
- `src/ui/` — Qt dialogs/widgets; `ui/` and `resources/` — `.ui` forms and assets.
- `src/fredstubs.cpp` — editor stubs for game-only engine hooks.
- `CMakeLists.txt`, `cmake/`, `source_groups.cmake` — build config. `README.md`,
  `CHANGELOG.md`, `help-src/` — docs.

## Relationship to the engine
- Shares the engine's data model: `ship_info`/`Ships`, `p_object`/`Parse_objects`
  (`code/mission/missionparse.*`), SEXPs (`code/parse/sexp.*`).
- Editor logic in `src/mission/` separates engine state from the Qt UI layer.
- Author-facing engine additions (new table fields, new SEXPs) generally need
  matching qtFRED UI changes (and FRED2).

## Configuration tables
None of its own; reads the same content tables as the game to populate lists.

## See also
- `documentation/modules/fred2.md` (legacy MFC editor), `code/mission/`,
  `code/parse/sexp.*`, `documentation/ARCHITECTURE.md`.
- Wiki: https://wiki.hard-light.net/index.php/Qtate_of_the_Art_(qtFRED)
