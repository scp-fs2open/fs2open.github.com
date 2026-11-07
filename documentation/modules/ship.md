# Module: ship — `code/ship/`

## Purpose
Owns **ships**: both the class definitions (`ship_info`, loaded from `ships.tbl`)
and the live mission instances (`ship`). Handles subsystems (turrets, engines,
sensors), weapon banks, damage/death, warp in/out, afterburners, contrails, and
ship-type categories. This is one of the largest and most central modules.

## Key files
- `ship.h` / `ship.cpp` — `ship`, `ship_info`, `wing`, subsystems, most logic.
- `ship_flags.h` — ship and ship_info flagsets.
- `shipfx.cpp` / `shipfx.h` — visual effects (warp, death roll, sparks, breakup).
- `shiphit.cpp` / `shiphit.h` — damage application, subsystem/hull/shield hits.
- `shipcontrails.cpp`, `afterburner.cpp`, `awacs.cpp`.

## Core data structures / globals
- `ship Ships[MAX_SHIPS]` — live instances; `ship.ship_info_index` → `Ship_info`.
- `SCP_vector<ship_info> Ship_info` — class definitions from `ships.tbl`.
- `wing Wings[MAX_WINGS]` — wing groupings.
- `SCP_vector<exited_ship> Ships_exited` — record of ships that left/died.
- `ship_subsys`, `ship_weapon` — per-instance subsystem and weapon-bank state.

## Major constants (mostly in `globals.h`)
- `MAX_SHIPS` (500), `SHIPS_LIMIT`, `MAX_SHIP_CLASSES` (500).
- `MAX_WINGS` (75), `MAX_SHIPS_PER_WING` (6), `MAX_STARTING_WINGS`,
  `MAX_SQUADRON_WINGS`, `MAX_TVT_TEAMS`, `MAX_TVT_WINGS`.
- `MAX_SHIP_PRIMARY_BANKS` (3), `MAX_SHIP_SECONDARY_BANKS` (4),
  `MAX_SHIP_WEAPONS` (7).
- `MAX_MODEL_SUBSYSTEMS` (200), `MAX_SHIP_BAY_PATHS` (in `model.h`).

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `ships.tbl` (+ `*-shp.tbm`) | `parse_shiptbl()` | Ship class definitions |
| `objecttypes.tbl` | `parse_shiptype_tbl()` | Ship-type categories/behaviour |
| `armor.tbl` | `armor_parse_table()` | Armor/damage-type definitions |

Table option reference: https://wiki.hard-light.net/index.php/Tables (see *Ships.tbl*).

## See also
- `code/weapon/` (weapon banks), `code/ai/` (per-ship AI), `code/model/` (POF + subsystems).
- Mission-side ship spawning: `code/mission/missionparse.*` (`p_object`, `Parse_objects`).
