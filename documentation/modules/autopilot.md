# Module: autopilot — `code/autopilot/`

## Purpose
**Nav points and autopilot**: the system that lets the player select a
destination and have the ship fly there at high time compression, with the wing
following. It owns the nav-point list a mission defines, the rules about when
autopilot may engage, and the messages explaining why it may not.

## Key files
- `autopilot.cpp` / `autopilot.h` — the whole module.

## Core data structures / globals
- `class NavPoint` + `NavPoint Navs[MAX_NAVPOINTS]` — one destination. A nav
  point is **bound to something else** rather than being a bare position:
  `target_index` is a waypoint-list index when `NP_WAYPOINT` is set (with
  `waypoint_num` selecting the node) and an object index when `NP_SHIP` is set.
  That is why a nav point can follow a moving ship.
- `CurrentNav` — the selected nav point; `AutoPilotEngaged`.
- `NavMessage NavMsgs[NP_NUM_MESSAGES]` — the table-defined messages shown when
  autopilot engages or refuses.
- `LockAPConv` — the timestamp that locks out re-engaging.

## Major constants
- `MAX_NAVPOINTS` (8), `NPS_TICKRATE` (125).
- Nav flags: `NP_WAYPOINT`, `NP_SHIP` (the two binding types, masked by
  `NP_VALIDTYPE`), `NP_HIDDEN` (not on the map), `NP_NOACCESS` (not
  selectable) — together `NP_NOSELECT` — and `NP_VISITED`, set once the player
  has been within 1000 metres.
- Refusal reasons: `NP_MSG_FAIL_NOSEL` (nothing selected),
  `NP_MSG_FAIL_GLIDING`, `NP_MSG_FAIL_TOCLOSE`, `NP_MSG_FAIL_HOSTILES`,
  `NP_MSG_FAIL_HAZARD`, `NP_MSG_FAIL_SUPPORT_PRESENT`,
  `NP_MSG_FAIL_SUPPORT_WORKING`, plus `NP_MSG_MISC_LINKED`;
  `NP_NUM_MESSAGES` (8).

## Key entry points
- `CanAutopilot(targetPos, send_msg)` — the gate. Pass `send_msg` to have it
  also display the reason it said no.
- `StartAutopilot()` / `EndAutoPilot()`.
- `NavSystem_Init()`, and `NavSystem_Do()`, which runs from
  `game_simulation_frame()` right after the AWACS update.
- Mission and SEXP control: `AddNav_Ship()`, `AddNav_Waypoint()`,
  `DelNavPoint()`, `SelectNav()` / `DeselectNav()`, `FindNav()`,
  the `Nav_Set_*` / `Nav_UnSet_*` flag helpers, `IsVisited()`, and
  `Nav_SetColor()`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `autopilot.tbl` (+ `*-aplt.tbm`) | `parse_autopilot_table()` (`autopilot.cpp`) | Autopilot messages, sounds, and link distance |

The nav points themselves are per-mission `.fs2` data.

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/ai/` (the wing follows under AI control), `code/parse/sexp.*` (the
  nav-point SEXPs), `code/hud/` (the nav display and autopilot cue),
  `code/mission/missionparse.*` (nav points and waypoint paths).
