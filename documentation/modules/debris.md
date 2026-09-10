# Module: debris — `code/debris/`

## Purpose
**Ship debris**: the chunks thrown off when a ship is damaged or destroyed.
Debris is an `OBJ_DEBRIS` object with physics and collision; large "hull" pieces
persist and can be shot, while small pieces expire. Debris also carries the
crackling electrical arcs seen on wrecked hulls.

> Not the same as an asteroid **debris field** (`code/asteroid/`), which reuses
> debris *models* for a mission-scale hazard.

## Key files
- `debris.cpp` / `debris.h` — the whole module.

## Core data structures / globals
- `struct debris` + `SCP_vector<debris> Debris` — one live piece: the
  `source_objnum` and `ship_info_index` it came from, `team`, `species`,
  `damage_type_idx`, its `model_num` / `model_instance_num` / `submodel_num`,
  `arc_frequency`, and `parent_alt_name`.
- `debris_electrical_arc` — one arc on a piece; up to `MAX_DEBRIS_ARCS`.
- The **hull list** — large pieces are tracked separately via
  `debris_add_to_hull_list()` / `debris_remove_from_hull_list()`, because they
  persist and collide while small pieces do not.

## Major constants
- `MAX_DEBRIS_ARCS` (8).
- `SOFT_LIMIT_DEBRIS_PIECES` (64) — a *soft* limit: the engine starts retiring
  old pieces past it rather than refusing to create new ones.

## Key entry points
- `debris_init()`, `debris_process_post(obj, frame_time)`, `debris_render()`,
  `debris_delete()` — the standard object-type lifecycle.
- `debris_create_set_velocity(db, source_shipp, exp_center, exp_force, source_subsys)`
  — how an explosion throws a piece.
- `debris_check_collision()` / `debris_hit()` — the collision-matrix entries.
- `debris_get_team()`, `debris_is_generic()`, `debris_is_vaporized()` — queries
  other systems use; vaporized debris is the "no wreckage" case.
- `debris_create_fire_hook()` — the Lua hook firing site.

## Configuration tables
None of its own. Which debris models a ship produces comes from `ships.tbl`
(`code/ship/`) and per-species defaults in `code/species_defs/`.

## See also
- `code/object/` (`OBJ_DEBRIS`, collision), `code/ship/shipfx.*` and
  `shiphit.*` (what creates debris), `code/asteroid/` (debris fields),
  `code/model/` (submodels become debris pieces).
