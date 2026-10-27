# Module: playerman — `code/playerman/`

## Purpose
Owns the **player**: the `player` struct (callsign, squadron, campaign, stats,
HUD/targeting preferences, hotkey target sets), the per-frame translation of
control input into ship commands, and pilot-record management (create, delete,
pick a portrait). Persisting a pilot to disk is a separate module,
`code/pilotfile/`.

## Key files
- `player.h` — the `player` class, the `Players[]`/`Player` globals, the
  `PLAYER_FLAGS_*` set, and the control-mode constants.
- `playercontrol.cpp` — reads bindings through `code/controlconfig/` and turns
  them into ship control input each frame; also view/slew angles, speed
  matching, and the player's looping sounds.
- `managepilot.cpp` / `managepilot.h` — pilot record management:
  `init_new_pilot()`, `delete_pilot_file()`, portrait and squad-image lists,
  callsign formatting.

## Core data structures / globals
- `class player` — one pilot. Holds `callsign`/`short_callsign`, portrait and
  squadron image filenames, `current_campaign`, `flags`/`save_flags`,
  `keyed_targets[MAX_KEYED_TARGETS]` (the hotkey target lists),
  lead/lock indicator state, and `last_ship_flown_si_index`.
- `player Players[MAX_PLAYERS]` — storage. `Player_num` is this machine's index
  and `player *Player` points at that entry; single-player code uses `Player`.
- `Player_flight_mode` (`FlightMode`), `Player_flight_cursor`,
  `Flight_cursor_extent`, `Flight_cursor_deadzone` — flight-cursor control mode.
- `chase_slew_angles`, `Perspective_locked`, `Slew_locked` — view control.
- `Player_use_ai`, `use_descent`, `lua_game_control` — overrides that let the
  AI, Descent-style physics, or a Lua script take over the player's ship.

## Major constants
- `MAX_KEYED_TARGETS` (8) — hotkey target sets.
- `PLAYER_FLAGS_*` — per-pilot state bits, for example
  `PLAYER_FLAGS_AUTO_TARGETING`, `PLAYER_FLAGS_MATCH_TARGET`,
  `PLAYER_FLAGS_MSG_MODE`, `PLAYER_FLAGS_IS_MULTI`, `PLAYER_FLAGS_PROMOTED`,
  the `PLAYER_FLAGS_KILLED_SELF_*` death causes, and the
  `PLAYER_FLAGS_PLR_VER_*` bits recording how the pilot file compared to
  `PLR_VERSION`. `PLAYER_KILLED_SELF` is the mask of the self-kill causes.
- `PCM_*` — player control mode: `PCM_NORMAL`, the three `PCM_WARPOUT_STAGE*`
  values, `PCM_SUPERNOVA`.
- `LGC_*` — which control layers Lua has taken over.
- `PLAYER_PILOT_PIC_W/H` (160×120), `PLAYER_SQUAD_PIC_W/H` (128×128).

## Configuration tables
None. Pilot data is saved in `.plr`/`.csg` files by `code/pilotfile/`.

## See also
- `code/pilotfile/` (saving and loading everything in `player`),
  `code/controlconfig/` (the bindings `playercontrol.cpp` reads),
  `code/stats/` (scoring written into the pilot), `code/menuui/barracks.cpp` and
  `playermenu.cpp` (the screens that edit pilots),
  `code/hud/` (consumes the lead/lock indicator state).
