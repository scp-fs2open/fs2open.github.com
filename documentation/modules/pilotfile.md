# Module: pilotfile — `code/pilotfile/`

## Purpose
**Persists the player.** Two files per pilot: a `.plr` holding everything not
tied to a campaign (callsign, settings, control bindings, multiplayer stats) and
a `.csg` campaign save-game per campaign (progress, per-campaign stats,
persistent variables, ship/weapon availability). This module reads and writes
both, and converts pilots written by older builds.

## Key files
- `pilotfile.cpp` / `pilotfile.h` — the `pilotfile` class: the public
  `load_player` / `save_player` / `load_savefile` / `save_savefile` entry points
  and the shared section machinery.
- `plr.cpp` — the `.plr` reader/writer, section by section.
- `csg.cpp` — the `.csg` campaign save reader/writer.
- `plr_hudprefs.cpp` / `plr_hudprefs.h` — HUD configuration inside the `.plr`.
- `FileHandler.h`, `BinaryFileHandler.*`, `JSONFileHandler.*` — the storage
  back ends. The same section-writing code drives either format.
- `pilotfile_convert.*`, `plr_convert.cpp`, `csg_convert.cpp` — importers for
  older pilot formats.

## Core data structures / globals
- `class pilotfile` — the whole API. Also carries `update_stats()`,
  `update_stats_backout()`, `reset_stats()`, and `export_stats()` for scoring.
- `verify(fname, ...)` — reads just enough of a file to report a pilot's rank,
  language, and flags without a full load; the pilot-select screens use it.
- `is_invalid()` / `m_data_invalid` — set when a campaign save is unusable, so
  callers avoid writing garbage back.
- `startSection()` / `endSection()` — every part of a pilot file is a tagged,
  length-prefixed section, which is what lets an older or newer build skip what
  it does not understand.

## Major constants (`pilotfile.h`)
- `PLR_FILE_ID` (0x5f524c50, `"PLR_"`), `CSG_FILE_ID` (0x5f475343, `"CSG_"`) —
  the magic numbers at the head of each file.
- `PLR_VERSION` (4), `CSG_VERSION` (8), `PLR_VERSION_INVALID` (0xFF).

## Format and compatibility
- Both files may be **binary or JSON**; `load_player()` picks the handler by
  what it finds, and takes a `force_binary` override. New saves are written as
  `.json`.
- A pilot written by a newer build still loads, with the unknown sections
  skipped, and the mismatch is recorded in the player's
  `PLAYER_FLAGS_PLR_VER_IS_HIGHER` and `PLAYER_FLAGS_PLR_VER_IS_LOWER` flags.
- **Bumping `PLR_VERSION` or `CSG_VERSION` is a compatibility event.** Add new
  data in a new section, or behind a version check, so older pilots keep
  loading — the same backwards-compatibility rule that governs table changes.
- Out-of-range values are clamped on load (`clamp_value_with_warn`) rather than
  rejected.

## Configuration tables
None. `.plr` and `.csg` are per-user save files, not content tables.

## See also
- `code/playerman/` (the `player` struct being saved),
  `code/stats/` (scoring), `code/controlconfig/` (bindings stored per pilot),
  `code/mission/missioncampaign.*` (campaign progress in the `.csg`),
  `code/menuui/barracks.cpp` / `playermenu.cpp` (the screens that call this).
