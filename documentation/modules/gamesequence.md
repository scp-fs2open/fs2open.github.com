# Module: gamesequence — `code/gamesequence/`

## Purpose
The **game state machine**. Owns the stack of game states (main hall, briefing,
gameplay, options, …), the queue of events that drive transitions between them,
and the dispatch into the per-state enter/leave/frame handlers. It is small and
purely mechanical: it decides *which* state is current, while the bodies of the
states live in `freespace2/freespace.cpp` and the individual UI modules.

## Key files
- `gamesequence.cpp` / `gamesequence.h` — the whole module: the `GS_EVENT` and
  `GS_STATE` enums, the state stack, and the `gameseq_*` API.

## Core data structures / globals
- `enum GS_EVENT` — every transition trigger (e.g. `GS_EVENT_START_GAME`,
  `GS_EVENT_MAIN_MENU`, `GS_EVENT_PREVIOUS_STATE`).
- `enum GS_STATE` — every screen/mode (e.g. `GS_STATE_MAIN_MENU`,
  `GS_STATE_BRIEFING`, `GS_STATE_GAME_PLAY`, `GS_STATE_LAB`).
- `GS_event_text[]` / `GS_state_text[]` — name strings for both enums, with
  `Num_gs_event_text` / `Num_gs_state_text`; scripting and the debug console use
  them, so **a new state or event must get a matching string entry**.
- An internal state stack; `gameseq_get_depth()` reports how deep it is and
  `gameseq_get_state(depth)` reads any level.

## API
- Post a transition: `gameseq_post_event(GS_EVENT_*)`. Events are queued and
  processed by `gameseq_process_events()` at the top of the main loop.
- Replace the current state: `gameseq_set_state()`. Stack a state on top of the
  current one: `gameseq_push_state()`; return with `gameseq_pop_state()` (or by
  posting `GS_EVENT_PREVIOUS_STATE`).
- Query: `gameseq_get_state()`, `gameseq_get_previous_state()`,
  `gameseq_get_event()`, `gameseq_get_state_idx()`,
  `gameseq_get_state_instance_id()`.
- The three hooks each state implements — `game_enter_state()`,
  `game_leave_state()`, `game_do_state()` — are **declared here but defined in
  `freespace2/freespace.cpp`** as large switches over `GS_STATE`.

## Adding a state
1. Add the `GS_STATE_*` value and, if it needs its own trigger, a `GS_EVENT_*`.
2. Add the matching strings to `GS_state_text[]` / `GS_event_text[]`.
3. Add `case` entries to `game_enter_state()`, `game_do_state()`, and
   `game_leave_state()` in `freespace2/freespace.cpp`.
4. Add the event → state mapping in `game_process_event()`.

## Configuration tables
None.

## See also
- `documentation/ARCHITECTURE.md` section 3 — the frame loop this module drives,
  including a diagram of how state dispatch fits into `game_main()`.
- `code/menuui/`, `code/missionui/`, `code/lab/`, `code/options/` — modules whose
  screens are game states.
