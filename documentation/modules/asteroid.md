# Module: asteroid — `code/asteroid/`

## Purpose
The **asteroid field**: a mission-defined volume that spawns and continuously
recycles asteroids around the player, plus the asteroid entities themselves.
Asteroids are `OBJ_ASTEROID` objects that collide, take damage, split into
smaller pieces, and can be aimed at a target. The same code also drives
**mission debris fields**, which use ship-debris models instead of rocks.

## Key files
- `asteroid.cpp` / `asteroid.h` — the whole module: the class table, the field,
  spawning and wrapping, collision, splitting, and rendering.

## Core data structures / globals
- `class asteroid_info` + `SCP_vector<asteroid_info> Asteroid_info` — the class
  table from `asteroid.tbl`: `type`, detail levels and distances,
  `damage_type_idx` (with `damage_type_idx_sav` so a SEXP change can be reset),
  and the split rules.
- `struct asteroid` + `asteroid Asteroids[MAX_ASTEROIDS]` — live instances:
  `objnum`, `model_instance_num`, `asteroid_type`, `asteroid_subtype`,
  `collide_objnum`/`collide_objsig` (the object it is about to hit), and
  `target_objnum` — asteroids can be aimed, see `asteroid_aim_at_target()`.
- `asteroid_obj Asteroid_obj_list` — the intrusive list of live asteroid objects.
- `asteroid_field Asteroid_field` — the mission's field: bounds, an optional
  inner bound (`has_inner_bound`), `num_initial_asteroids`, and
  `enhanced_visibility_checks`, which overrides the range checks used when
  deciding where to spawn and wrap asteroids.
- `asteroid_split_info` — what a destroyed asteroid turns into (`min`/`max`
  count of a given `asteroid_type`).
- `asteroid_subtype_info` — a subtype's model.
- `Num_asteroids`, `Asteroids_enabled`.
- Briefing icon rendering: `Asteroid_icon_closeup_model`,
  `Asteroid_icon_closeup_position`, `Asteroid_icon_closeup_zoom`.

## Major constants
- `MAX_ASTEROIDS` (2000), `NUM_ASTEROID_SIZES` (3).
- Types: `ASTEROID_TYPE_DEBRIS` (-1), `ASTEROID_TYPE_SMALL` (0),
  `ASTEROID_TYPE_MEDIUM` (1), `ASTEROID_TYPE_LARGE` (2).
- `MAX_RETAIL_DEBRIS_TYPES` (3), `MAX_ASTEROID_DETAIL_LEVELS` (5).
- `AF_USED` — the in-use bit in `asteroid::flags`.

## Field behaviour worth knowing
Asteroids **wrap**: rather than simulating a whole field, the engine keeps a
fixed population near the player and teleports asteroids that leave the bounds
around to the other side. That is why the field's bounds and the visibility
checks matter more than the asteroid count.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `asteroid.tbl` (+ `*-ast.tbm`) | `asteroid.cpp` | Asteroid and debris-field class definitions |

The field itself (bounds, density, which subtypes) is per-mission `.fs2` data.
`code/species_defs/` also references `asteroid.tbl` entries, since debris types
are per-species.

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/object/` (`OBJ_ASTEROID` and the collision matrix), `code/debris/` (ship
  debris, a different system), `code/model/`, `code/lab/`
  (`LabMode::Asteroid`), `code/mission/missionparse.*` (field definition).
