# Module: prop — `code/prop/`

## Purpose
**Props**: static scenery objects (landscape pieces, station clutter, set
dressing). A prop is a real engine `object` of type `OBJ_PROP` with a model and
optional collision, but no physics, no AI, no damage, and no weapons — it is the
cheapest way to put geometry into a mission. Props are the newest `OBJ_*` type
and are a good worked example of what adding an object type actually involves.

## Key files
- `prop.cpp` / `prop.h` — the whole module: `prop_info`, `prop`,
  `parsed_prop`, table parsing, create/delete, render, and collision.
- `prop_flags.h` — `Prop::Prop_Flags` (per-instance render toggles) and
  `Prop::Info_Flags` (per-class behaviour).

## Core data structures / globals
- `prop_info` — the *class* definition from `props.tbl`: name, category, POF
  file, closeup camera position/zoom, detail-level distances, glowpoint bank
  overrides, custom data/strings, and `flagset<Prop::Info_Flags>`.
- `prop` — a *live instance*: name, `objnum`, `prop_info_index`,
  `model_instance_num`, creation time, alpha multiplier, per-bank glowpoint
  state, FRED layer, and `flagset<Prop::Prop_Flags>`.
- `parsed_prop` — a prop entry read out of a mission file, before it is spawned.
- `SCP_vector<prop_info> Prop_info` — class table.
- `SCP_vector<std::optional<prop>> Props` — instance storage. Note this is a
  **vector of optionals**, not the fixed `MAX_*` array used by ships and
  weapons; an empty slot is `std::nullopt`.
- `SCP_vector<prop_category> Prop_categories` — display categories (name +
  list colour) used by FRED and the Lab.

## Major constants / flags
- `MAX_PROP_DETAIL_LEVELS` (aliases `MAX_SHIP_DETAIL_LEVELS`).
- `Prop::Info_Flags` — `No_collide`, `No_fred`, `No_lighting`.
- `Prop::Prop_Flags` — per-instance render overrides: `Glowmaps_disabled`,
  `Draw_as_wireframe`, `Render_full_detail`, `Render_without_light`, and
  per-map disables for diffuse/glowmap/normal/height/ambient/spec/reflect, plus
  `Render_with_alpha_mult`.

## Lifecycle
- `prop_init()` parses the table; `props_level_init()` / `props_level_close()`
  bracket a mission.
- `prop_create(orient, pos, prop_type, name)` wraps `obj_create()`;
  `prop_delete(obj)` tears one down; `change_prop_type()` swaps the class of an
  existing instance.
- `prop_render(obj, scene)` queues the model into the draw list.
- `prop_check_collision()` is the `OBJ_PROP` entry in the collision matrix, and
  is skipped entirely for classes flagged `No_collide`.
- Lookups: `prop_info_lookup(token)` by class name, `prop_name_lookup(name)` by
  instance name.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `props.tbl` (+ `*-prp.tbm`) | `parse_prop_table()` (`prop.cpp`) | Prop class definitions and categories |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/object/` (`OBJ_PROP`, the collision matrix), `code/model/` (the POF and
  its instance), `code/lab/` (`LabMode::Prop` displays props),
  `code/mission/missionparse.*` (props placed in a mission).
- The `fso-add-object-type` skill — props are the most recent worked example.
