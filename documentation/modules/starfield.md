# Module: starfield — `code/starfield/`

## Purpose
The **background**: everything painted behind the mission. Stars, the suns and
their glare, background bitmaps (planets, nebula clouds, debris fields), the
optional skybox model, the drifting motion debris that gives a sense of speed,
and the supernova sequence. A mission can define several **backgrounds** and
switch between them.

> `code/starfield/nebula.*` is the *background* nebula bitmap. The nebula the
> player flies through, with fog and lightning, is `code/nebula/`.

## Key files
- `starfield.cpp` / `starfield.h` — backgrounds, suns, background bitmaps, the
  skybox model, motion debris. Everything is prefixed `stars_`.
- `starfield_flags.h` — background-element flagsets.
- `nebula.cpp` / `nebula.h` — the retail background nebula bitmap
  (`nebula_init()`, and the `Nebula_pitch` / `_bank` / `_heading` orientation).
- `supernova.cpp` / `supernova.h` — the scripted supernova ending.

## Core data structures / globals
- `background_t` + `SCP_vector<background_t> Backgrounds`, selected by
  `Cur_background` — a background is a set of suns and bitmaps. Missions may
  define more than one and swap them.
- `starfield_list_entry` — one sun or background bitmap: filename, angles,
  scale, and `div_x`/`div_y` subdivisions.
- `struct star` — a single point star.
- The skybox: `Nmodel_num`, `Nmodel_instance_num`, `Nmodel_orient`,
  `Nmodel_flags`, `Nmodel_bitmap`, `Nmodel_alpha`.
- Motion debris: `motion_debris_types` / `motion_debris_bitmaps`,
  `SCP_vector<motion_debris_types> Motion_debris_info`, `Motion_debris_ptr`,
  and the `Motion_debris_enabled` / `Motion_debris_override` switches.
- `SUPERNOVA_STAGE` — the supernova state machine.

## Major constants
- `MAX_STARFIELD_BITMAP_LISTS` (1), `MAX_MOTION_DEBRIS_BITMAPS` (4).

## Key entry points
- Editing a background (FRED and SEXPs both use these):
  `stars_add_sun_entry()`, `stars_add_bitmap_entry()`, `stars_get_data()` /
  `stars_set_data()`, `stars_swap_backgrounds()`, `stars_pack_backgrounds()`,
  and the `stars_correct_background_*_angles()` helpers.
- Supernova: `supernova_start(seconds)`, `supernova_process()`,
  `supernova_active()`, `supernova_pct_complete()`,
  `supernova_camera_cut()`, `supernova_get_eye()`. `supernova_process()` is
  called from `game_simulation_frame()`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `stars.tbl` (+ `*-str.tbm`) | `starfield.cpp` | Available suns, background bitmaps, and motion debris |

The background a given mission actually uses is stored in its `.fs2` file.

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/nebula/` (in-mission nebula), `code/lighting/` (suns become directional
  lights; `sun_index` in `struct light` refers to this module),
  `code/graphics/` (rendering), `code/mission/missionparse.*` (per-mission
  background data), `fred2/bgbitmapdlg.*` and the qtFRED background editor.
