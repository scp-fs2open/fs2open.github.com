# Module: events — `code/events/`

## Purpose
A **tiny set of engine-wide event hooks** that let code react to major lifecycle
moments without the two sides knowing about each other. It is about 30 lines: a
handful of `util::event` objects in `namespace events`, nothing more. Read
`events.h` directly.

## The events
- `events::EngineUpdate` — fired each engine update.
- `events::EngineShutdown` — fired at shutdown.
- `events::GameEnterState(int, int)` / `events::GameLeaveState(int, int)` —
  fired on a game-state transition, with the old and new `GS_STATE`.
- `events::GameMissionLoad(const char*)` — fired when a mission is loaded.

## Using it
Each is a `util::event<Ret, Args...>` from `code/utils/event.h` — a multicast
callback list. Subscribe a callable to be notified.

## Not to be confused with
- **Mission events** — the SEXP-driven events a mission designer writes; those
  live in `code/mission/missiongoals.*` and `code/parse/sexp.*`.
- **Lua hooks** — `scripting::Hook` in `code/scripting/hook_api.h`, the far
  richer event system exposed to mods.
- **SDL events** — the OS input events pumped by `code/osapi/`.

This module is only for C++ engine code that needs a lifecycle notification.

## Configuration tables
None.

## See also
- `code/utils/event.h` (the `util::event` template),
  `code/gamesequence/` (the states these events report),
  `code/scripting/` (the mod-facing event system).
