# Module: hud — `code/hud/`

## Purpose
The in-flight **Heads-Up Display**: all combat gauges (targeting, reticle,
shields, radar/escort lists, messages, directives, lock indicators, wingman
status, artillery/SSM) and the HUD configuration system. Gauges are data-driven
and configurable through `hud_gauges.tbl`.

## Key files
- `hud.cpp` / `hud.h` — HUD framework, gauge base, render dispatch.
- `hudparse.cpp` / `hudparse.h` — parses `hud_gauges.tbl` into gauge layout.
- `hudconfig.*` — player-facing HUD configuration screen.
- Per-gauge files: `hudtarget.*`, `hudreticle.*`, `hudshield.*`, `hudescort.*`,
  `hudmessage.*`, `hudtargetbox.*`, `hudlock.*`, `hudwingmanstatus.*`,
  `hudbrackets.*`, `hudsquadmsg.*`, `hudets.*`, `hudartillery.*`, `hudnavigation.*`.

## Core data structures / globals
- `HudGauge` (base class) and derived gauge classes.
- Per-ship/player HUD config in `hud_config` (`hudconfig.h`).

## Major constants
- `MAX_COMPLETE_ESCORT_LIST` (20, in `code/globalincs/globals.h`).
- Gauge id/coordinate defines and color tags within the per-gauge headers.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `hud_gauges.tbl` | `parse_hud_gauges_tbl()` (`hudparse.cpp`) | Gauge layout/appearance |
| `ssm.tbl` | `parse_ssm()` (`hudartillery.cpp`) | Subspace missile strikes |

Table option reference: https://wiki.hard-light.net/index.php/Tables (see *hud_gauges.tbl*).

## See also
- `code/ship/` (target/subsystem data shown), `code/radar/`, `code/graphics/` (drawing),
  `code/scripting/` (custom Lua HUD gauges via the HUD-draw hook).
