# Module: fireball — `code/fireball/`

## Purpose
**Explosions and warp effects**. A fireball is an `OBJ_FIREBALL` object that
plays an animation and expires. The module covers both the explosion fireballs
(the small ones that precede a ship's death and the large ones when it breaks
apart) and the **warp-in/warp-out effect**, which shares the same machinery
because it is also a short-lived animated effect anchored in the world.

## Key files
- `fireballs.cpp` / `fireballs.h` — the class table, creation, per-frame update,
  and rendering.
- `warpineffect.cpp` — the warp effect specifically.

## Core data structures / globals
- `fireball_info` + `SCP_vector<fireball_info> Fireball_info` — the class table
  from `fireball.tbl`. Each entry holds up to `MAX_FIREBALL_LOD` levels of
  detail (`fireball_lod`: `bitmap_id`, `num_frames`, `fps`), and the warp
  entries additionally carry `warp_glow_bitmap`, `warp_ball_bitmap`, and
  `warp_model_id`.
- `enum class warp_style` — which warp visual to use.

## Major constants
- Render types: `FIREBALL_MEDIUM_EXPLOSION` (0), `FIREBALL_LARGE_EXPLOSION`
  (1), `FIREBALL_WARP_EFFECT` (2).
- Built-in fireball ids: `FIREBALL_EXPLOSION_MEDIUM` (0) — the small explosions
  before a ship dies; `FIREBALL_WARP` (1); `FIREBALL_KNOSSOS` (2);
  `FIREBALL_ASTEROID` (3); `FIREBALL_EXPLOSION_LARGE1` (4) and
  `FIREBALL_EXPLOSION_LARGE2` (5) — the break-up explosions.
  `NUM_DEFAULT_FIREBALLS` (6), `FIREBALL_NUM_LARGE_EXPLOSIONS` (2).
- `MAX_FIREBALL_LOD` (4).
- Warp flags: `FBF_WARP_CLOSE_SOUND_PLAYED`, `FBF_WARP_CAPITAL_SIZE`,
  `FBF_WARP_CRUISER_SIZE`.

## Key entry points
- `fireball_create(pos, fireball_type, render_type, parent_obj, size, ...)` —
  the one creation call. Its long optional tail (velocity, warp lifetime, ship
  class, orientation, open/close sounds and durations) is what makes it serve
  both explosions and warps.
- `fireball_process_post()`, `fireball_render()`, `fireball_delete()` — the
  object-type lifecycle. `fireball_init()` / `fireball_close()` /
  `fireball_parse_tbl()`.
- `fireball_info_lookup(unique_id)` — resolve a table entry by name.
- `fireball_ship_explosion_type(sip)` / `fireball_asteroid_explosion_type(aip)`
  — pick the right entry for what just died.
- `fireball_is_warp(obj)` and `fireball_is_perishable(obj)` — the two questions
  other systems ask; perishable fireballs may be culled under load.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `fireball.tbl` (+ `*-fbl.tbm`) | `fireball_parse_tbl()` (`fireballs.cpp`) | Explosion and warp effect definitions |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/object/` (`OBJ_FIREBALL`), `code/ship/shipfx.*` (death sequences and
  warp), `code/particle/` and `code/decals/` (the other effect systems),
  `code/weapon/shockwave.*` (the separate shockwave effect).
