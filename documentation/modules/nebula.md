# Module: nebula — `code/nebula/`

## Purpose
The **in-mission nebula**: the fog that limits visibility, the drifting "poof"
sprites, the lightning storms, and the newer volumetric nebula rendering.
A nebula is a gameplay feature as much as a visual one — it shortens sensor
range, hides ships, and changes AI behaviour, so this module is read by more
than the renderer.

> Note the split: **`code/nebula/` is the nebula the player flies through**;
> `code/starfield/nebula.*` is the *background* nebula bitmap painted on the
> skybox. Different systems, similar names.

## Key files
- `neb.cpp` / `neb.h` — the fog model, the poof sprites, and the per-frame
  render setup. Everything is prefixed `neb2_`.
- `neblightning.cpp` / `neblightning.h` — lightning bolts and storms (`nebl_`).
- `volumetrics.cpp` / `volumetrics.h` — `volumetric_nebula`: the modern
  volume-rendered nebula, configured per mission.

## Core data structures / globals
- `Neb2_render_mode` — `NEB2_RENDER_NONE`, `NEB2_RENDER_POF` (the FRED
  preview), or `NEB2_RENDER_HTL` (real fog; what the game uses).
- The fog tunables, all `Neb2_fog_*`: `Neb2_fog_near_distance`,
  `Neb2_fog_1000m_visibility`, `Neb2_fog_clip_distance`,
  `Neb2_fog_skybox_clip_distance`, plus per-effect visibility multipliers
  (`Neb2_fog_visibility_trail`, `Neb2_fog_visibility_thruster`,
  `Neb2_fog_visibility_weapon`, `Neb2_fog_visibility_shield`).
  `Neb2_fog_save_legacy_values`, `Neb2_fog_legacy_near_mult`, and
  `Neb2_fog_legacy_far_mult` preserve pre-rework behaviour for old missions.
- `Neb2_awacs` — the sensor-range effect; this is the gameplay half of a nebula.
- `Nebula_sexp_used` — set when a mission drives the nebula from SEXPs.
- `l_node` / `l_bolt` / `l_section` — lightning geometry;
  `SCP_vector<bolt_type> Bolt_types` and `SCP_vector<storm_type> Storm_types`
  are the table-defined bolt and storm definitions. `Nebl_intensity`.
- `class volumetric_nebula` — the per-mission volumetric settings (noise, edge
  smoothing, the generated volume bitmap). FRED edits it directly.

## Major constants
- `NEB2_RENDER_NONE` (0), `NEB2_RENDER_POF` (1), `NEB2_RENDER_HTL` (2).
- `MAX_LIGHTNING_NODES` (500), `MAX_LIGHTNING_BOLTS` (10),
  `MAX_BOLT_TYPES_PER_STORM` (10).

## Key entry points
- Lifecycle: `neb2_init()`, `neb2_pre_level_init()`, `neb2_level_init()`,
  `neb2_post_level_init()`, `neb2_level_close()`; `nebl_init()` /
  `nebl_level_init()` for lightning.
- Per frame: `neb2_render_setup(camid)`, `neb2_render_poofs()`,
  `nebl_process()`, `nebl_render_all()`.
- Queries other modules use: `neb2_get_fog_visibility(pos, distance_mult)`,
  `neb2_get_adjusted_fog_values()`, `neb2_skip_render(objp, z_depth)`
  (culling), `neb2_get_lod_scale(objnum)` (detail reduction in fog),
  `neb2_get_fog_color()`.
- Poofs can be toggled and faded at runtime: `neb2_toggle_poof()` must be
  followed by `neb2_toggle_poof_finalize()`.
- Storms: `nebl_set_storm(name)`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `nebula.tbl` (+ `*-neb.tbm`) | `neb.cpp` | Poof bitmaps and nebula colours |
| `lightning.tbl` (+ `*-ltng.tbm`) | `neblightning.cpp` | Bolt types and storm types |

Per-mission nebula settings (which poofs, fog range, volumetrics) live in the
`.fs2` file, not a table.

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/starfield/` (the background, including the *other* nebula),
  `code/graphics/` (fog and the volumetric pass), `code/ship/awacs.cpp`
  (`Neb2_awacs` feeds sensor range), `code/ai/` (AI behaviour in nebula),
  `code/parse/sexp.*` (nebula SEXPs).
