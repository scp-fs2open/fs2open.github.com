# Module: mod_table — `code/mod_table/`

## Purpose
Owns **`game_settings.tbl`** — the engine-wide, per-mod tunables table. This is
where global feature toggles and default behaviour switches live (collision
behaviour, default detail, scoring, AI/physics defaults exposed at the mod level,
fiction/cutscene options, and many compatibility flags). When a mod needs to
change engine-global behaviour rather than a specific ship/weapon, it goes here.

## Key files
- `mod_table.cpp` / `mod_table.h` — `parse_mod_table()` and the global flag set.

## Core data structures / globals
- Many free-standing `extern` globals (e.g. `Weapons_inherit_parent_collision_group`)
  set from the table and read across the engine.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `game_settings.tbl` | `parse_mod_table()` | Engine-wide / per-mod behaviour flags |

Table option reference: https://wiki.hard-light.net/index.php/Game_settings.tbl
and the general index: https://wiki.hard-light.net/index.php/Tables

## Related global-settings modules
- `code/options/` — `default_settings.tbl` (default player option values).
- `code/globalincs/` — compile-time `MAX_*` limits (not table-driven).

## See also
- Almost every gameplay module reads one or more flags defined here.
