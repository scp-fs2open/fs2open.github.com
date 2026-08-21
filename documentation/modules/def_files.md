# Module: def_files — `code/def_files/`

## Purpose
**Built-in default data files, compiled into the executable.** FSO ships a
fallback copy of the tables, shaders, and scripts it cannot start without, so
the engine runs even when a mod (or a bare install) does not supply them.
`cfile` falls back to these when a file is not found on disk or in a VP.

This is why the engine can boot with no `iff_defs.tbl` present, and why a mod
only has to ship the tables it actually changes.

## Key files
- `def_files.cpp` / `def_files.h` — the lookup API. The `.cpp` is **generated
  at build time** from the contents of `data/`; do not edit it by hand.
- `data/tables/` — the default tables: `ai_profiles.tbl`, `autopilot.tbl`,
  `cheats.tbl`, `controlconfigdefaults.tbl`, `fonts.tbl`, `game_settings.tbl`,
  `iff_defs.tbl`, `objecttypes.tbl`, `post_processing.tbl`,
  `species_defs.tbl`, and others.
- `data/effects/` — the built-in shaders (`.sdr`).
- `data/scripts/`, `data/maps/` — default scripts and images.

## Core data structures
- `struct default_file` — one embedded file: its `path_type` (the `CF_TYPE_*`
  it belongs to), `filename`, a pointer to the `data`, and its `size`.
- `defaults_get_file(filename)` — fetch one.
- `defaults_get_all()` — enumerate them.

## Working with it
- To change a default, edit the file under `data/`, not `def_files.cpp`.
- A new default file must be registered with the build (see
  `code/source_groups.cmake` and `cmake/embed_file.cmake`, whose
  `target_embed_files()` does the embedding) or it silently will not be
  embedded.
- Note that the shader sources under `data/effects/` are excluded from
  clang-tidy in `ci/linux/clang_tidy.sh`.

## Configuration tables
It **contains** the default copies of many tables but parses none of them; each
owning module still does the parsing.

## See also
- `code/cfile/` (the fallback lookup), `code/graphics/shaders/` and the
  `data/effects/` shaders, and every module that owns one of the embedded
  tables — `code/iff_defs/`, `code/species_defs/`, `code/mod_table/`,
  `code/ai/`, `code/autopilot/`, `code/controlconfig/`, `code/cheats_table/`.
