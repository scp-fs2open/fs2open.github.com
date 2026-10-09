# Module: observer — `code/observer/`

## Purpose
The **multiplayer observer**: a free-flying, non-combat viewpoint for a player
watching a game rather than flying in it. An observer is a real `OBJ_OBSERVER`
object so the camera, HUD, and networking treat it like any other entity, but it
has no model, no weapons, and no collision.

At about 130 lines this is one of the smallest modules in the tree — it is worth
reading `observer.h` directly.

## Core data structures / globals
- `struct observer` — `objnum`, `flags`, and `target_objnum` (declared but noted
  in the header as not yet used).
- `observer Observers[MAX_OBSERVER_OBS]`.

## Major constants
- `MAX_OBSERVER_OBS` (17) — one more than `MAX_PLAYERS` (12) allows for, so
  observer slots are not the limiting factor.
- `OBS_MAX_VEL_X`, `OBS_MAX_VEL_Y`, `OBS_MAX_VEL_Z` (all 85.0f) — the movement
  limits.
- `OBS_FLAG_USED`.

## Key entry points
- `observer_init()`.
- `observer_create(orient, pos)` — returns an objnum.
- `observer_delete(obj)`.

## Configuration tables
None.

## See also
- `code/object/` (`OBJ_OBSERVER`), `code/network/` (`MAX_OBSERVERS` and the
  multiplayer join-as-observer path), `code/playerman/`, `code/camera/`.
