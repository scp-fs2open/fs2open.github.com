# Module: radar — `code/radar/`

## Purpose
The **radar/sensor display**. It decides which objects are detectable, builds
the per-frame list of "blips", and draws them in one of four visual styles. It
is not a standalone screen: every radar is a `HudGauge` subclass, so a radar is
configured, positioned, and coloured through `hud_gauges.tbl` like any other
gauge.

## Key files
- `radarsetup.cpp` / `radarsetup.h` — the shared model: the `blip` type, the
  blip lists, visibility rules (`radar_is_visible`, `radar_plot_object`), and
  the `HudGaugeRadar` base class every style derives from.
- `radar.cpp` / `radar.h` — `HudGaugeRadarStd`: the retail 2D radar.
- `radarorb.cpp` / `radarorb.h` — `HudGaugeRadarOrb`: the 3D sphere radar.
- `radarngon.cpp` / `radarngon.h` — `HudGaugeRadarNgon` (derives from
  `HudGaugeRadarStd`): the standard radar drawn as an n-sided polygon.
- `radardradis.cpp` / `radardradis.h` — `HudGaugeRadarDradis`: the "BSG" style
  3D radar with its own sound loop.

## How a style is chosen
There is no runtime switch — the **gauge type in `hud_gauges.tbl` picks the
style**:

| Table gauge name | Gauge type | Class |
| --- | --- | --- |
| `Radar` | `HUD_OBJECT_RADAR_STD` (25) | `HudGaugeRadarStd`, or `HudGaugeRadarNgon` when the entry supplies `Ngon Sides:` |
| `Radar orb` | `HUD_OBJECT_RADAR_ORB` (26) | `HudGaugeRadarOrb` |
| `Radar BSG` | `HUD_OBJECT_RADAR_BSG` (27) | `HudGaugeRadarDradis` |

Only `Radar` is in the legacy default gauge set. When no `hud_gauges.tbl`
entry applies, `hudparse.cpp` loads `HUD_OBJECT_RADAR_ORB` if the
`-orbradar` command-line flag (`Cmdline_orb_radar`) is set and
`HUD_OBJECT_RADAR_STD` otherwise.

## Core data structures / globals
- `struct blip` — one contact: linked-list pointers, `position`, `objnum`,
  `dist`, colour, optional radar icon bitmap, and flags.
- `blip Blips[MAX_BLIPS]` — the pool; `N_blips` is the next free index.
  `Blip_bright_list[]` / `Blip_dim_list[]` are per-`BLIP_TYPE_*` linked lists
  built from that pool each frame.
- `Blip_last_update` — `SCP_map<int, TIMESTAMP>` of objnum to last update, used
  for blip flicker/persistence.
- `Radar_bright_range` — contacts nearer than this draw bright, the rest dim;
  recalculated on `Radar_calc_bright_dist_timer`.
- `See_all` — the cheat/debug override that makes everything visible.
- `Radar_2d_icon_mode` (`RadarIconMode`: `Off`, `On`, `TargetOnly`).
- `Radar_static_looping` — the sound handle for sensor-damage static.

## Major constants
- `MAX_BLIPS` (150), `MAX_BLIP_TYPES` (6).
- Blip types: `BLIP_TYPE_JUMP_NODE`, `BLIP_TYPE_NAVBUOY_CARGO`,
  `BLIP_TYPE_BOMB`, `BLIP_TYPE_WARPING_SHIP`, `BLIP_TYPE_TAGGED_SHIP`,
  `BLIP_TYPE_NORMAL_SHIP`.
- Blip flags: `BLIP_CURRENT_TARGET`, `BLIP_DRAW_DIM`, `BLIP_DRAW_DISTORTED`.
- `MAX_RADAR_COLORS` (5) × `MAX_RADAR_LEVELS` (2, bright and dim) →
  `Radar_color_rgb[][]`; colour slots `RCOL_BOMB`, `RCOL_NAVBUOY_CARGO`,
  `RCOL_WARPING_SHIP`, `RCOL_JUMP_NODE`, `RCOL_TAGGED`.
- `NUM_FLICKER_TIMERS` (2).
- `RadarVisibility` — `NOT_VISIBLE`, `VISIBLE`, `DISTORTED` (returned by
  `radar_is_visible()`; sensor-resistant and stealth ships come back
  `DISTORTED`).

## Per-frame flow
`radar_frame_init()` clears the lists, then `radar_plot_object()` is called for
each candidate object; it consults `radar_is_visible()` and, if the object
passes, takes a blip from the pool and links it into the right bright or dim
list. The active radar gauge's `render()` then walks those lists.

## Configuration tables
None of its own — radar gauges are configured through `hud_gauges.tbl`
(parsed in `code/hud/hudparse.cpp`).

Table option reference: https://wiki.hard-light.net/index.php/Hud_gauges.tbl

## See also
- `code/hud/` (`HudGauge`, `hudparse.cpp` — the gauge that hosts this),
  `code/object/` (the objects being plotted), `code/ship/` (awacs, stealth, and
  sensor-strength rules that feed `radar_is_visible`),
  `code/iff_defs/` (blip colours by team).
- The `fso-add-hud-gauge` skill.
