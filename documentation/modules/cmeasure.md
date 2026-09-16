# Module: cmeasure — `code/cmeasure/`

## Purpose
**Countermeasures** — the decoys a ship launches to break a missile lock. The
module is tiny (about 130 lines) because a countermeasure is just a weapon:
it is created and simulated by `code/weapon/` as an ordinary weapon object.
What lives here is only the decoy-specific behaviour — how it is launched, how
far away a homing missile can notice it, and telling the player when one worked.

## Key files
- `cmeasure.cpp` / `cmeasure.h` — the whole module.

## Core data structures / globals
- `Cmeasures_homing_check` — how many missiles are currently considering a
  countermeasure; the homing code uses it to avoid redundant work.
- `Countermeasures_enabled` — the global on/off switch.

## Major constants
- `CMEASURE_WAIT` (333) — milliseconds between launches.
- `MAX_CMEASURE_TRACK_DIST` (300.0f) — the furthest distance at which a missile
  can track a countermeasure. The header notes that raising this makes missiles
  track decoys almost always, so it is a balance value, not a limit.
- `CMEASURE_DETONATE_DISTANCE`.

## Key entry points
- `cmeasure_set_ship_launch_vel(objp, parent_objp, arand)` — give a freshly
  launched countermeasure its velocity relative to the launching ship.
- `cmeasure_maybe_alert_success(objp)` — the "decoy worked" notification.

## Configuration tables
None. Countermeasures are weapon entries in `weapons.tbl`; which one a ship
carries is a `ships.tbl` field.

## See also
- `code/weapon/` (countermeasures are weapons; homing lives there),
  `code/ship/` (the launcher and ammo count), `code/ai/` (when the AI launches
  one), `code/hud/` (the countermeasure gauge).
