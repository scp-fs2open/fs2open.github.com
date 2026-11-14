# Module: decals — `code/decals/`

## Purpose
**Surface decals**: the scorch marks, bullet holes, and impact damage projected
onto ship hulls. A decal is a named definition (diffuse/glow/normal bitmaps) that
gets instantiated at a hit position, attached to a submodel of the target object,
and faded out over its lifetime. The whole system can be switched off, both by
the renderer and by a player option.

## Key files
- `decals.cpp` / `decals.h` — the entire module: `DecalDefinition`, the live
  `Decal` list, table parsing, creation, and per-frame update.

## Core data structures / globals
- `class DecalDefinition` — one entry from the decal table: the diffuse, glow,
  and normal bitmaps plus whether each animates on a loop. Loaded lazily
  (`loadBitmaps()`, `bitmapsLoaded()`, `pageIn()`).
- `SCP_vector<DecalDefinition> DecalDefinitions` — all parsed definitions;
  resolve a name with `findDecalDefinition()`.
- `struct Decal` — one live decal. `definition_handle` is a
  `std::variant<int, std::tuple<int,int,int>>`: either an index into
  `DecalDefinitions` or an immediate diffuse/glow/normal bitmap triple. It also
  holds the host `object_h` (plus `orig_obj_type` and `submodel`), position,
  scale, orientation, `creation_time`, and `lifetime` (negative means it never
  expires). Removed by `markForDeletion()`; `isValid()` catches the case where
  the host object died and its slot was reused.
- `decals::creation_info` — the parsed "which decal, how big, random rotation?"
  block that other tables embed; read with `parseDecalReference()`.
- `Decal_system_active` — forced false under the stub (headless) backend.
  `Decal_option_active` — the player's setting. Always query through
  `decalSystemActive()`, which additionally requires
  `gr_capability::CAPABILITY_INSTANCED_RENDERING`, so decals degrade to "off"
  on hardware that cannot do instanced rendering.

## Notes and limits
- **Ships are the only supported host type.** `decals.cpp` asserts on anything
  else, so a new decal-receiving object type needs work here.
- Decals are graphics-only: they never affect collision, damage, or gameplay.
- Drawing goes through `graphics::decal_draw_list`, which is set up in
  `initialize()` and torn down in `shutdown()`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `*-dcl.tbm` | `parse_decals_table()` (`decals.cpp`) | Named decal definitions. Modular-only — there is no stock `decals.tbl` |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/graphics/` (`decal_draw_list`, the deferred decal pass),
  `code/weapon/` and `code/ship/shiphit.*` (impact sites that spawn decals),
  `code/particle/` (the other hit-effect system; a particle effect can also
  place a decal — see `ParticleEffect::DecalOrientationMode`).
