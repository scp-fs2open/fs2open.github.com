# Module: weapon — `code/weapon/`

## Purpose
Owns **weapons and projectiles**: lasers/ballistics, missiles, beams, plus their
special behaviours (swarm, corkscrew, flak, EMP, electronics, trails, spawning
child weapons, muzzle flashes). Like ships, it uses the info-vs-instance split:
`weapon_info` (class, from `weapons.tbl`) and `weapon` (live projectile instance).

## Key files
- `weapon.h` / `weapons.cpp` — `weapon`, `weapon_info`, core firing/move logic.
- `weapon_flags.h` — weapon and weapon_info flagsets.
- `beam.cpp` / `beam.h` — beam weapons (also object type `OBJ_BEAM`).
- `swarm.cpp`, `corkscrew.cpp`, `flak.cpp`, `emp.cpp`, `trails.cpp`,
  `muzzleflash.cpp`, `shockwave/` (see `code/shockwave` if present).

## Core data structures / globals
- `weapon Weapons[MAX_WEAPONS]` — live projectile instances.
- `SCP_vector<weapon_info> Weapon_info` — class definitions from `weapons.tbl`.

## Major constants
- Subtypes: `WP_UNUSED` (-1), `WP_LASER` (0, incl. ballistic primaries),
  `WP_MISSILE` (1), `WP_BEAM` (2).
- `MAX_WEAPONS` (3000), `MAX_WEAPON_TYPES` (500) — both in `globals.h`.
- `MAX_SPAWN_TYPES_PER_WEAPON` (5), `MAX_SUBSTITUTION_PATTERNS` (10),
  `MAX_BEAM_SECTIONS` (5).

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `weapons.tbl` (+ `*-wep.tbm`) | `parse_weaponstbl()` | Weapon class definitions |
| `mflash.tbl` | `parse_mflash_tbl()` | Muzzle-flash definitions |

Table option reference: https://wiki.hard-light.net/index.php/Tables (see *Weapons.tbl*).

## See also
- `code/ship/` (weapon banks `ship_weapon`), `code/object/` (collision handling),
  `code/fireball/` (impact explosions).
