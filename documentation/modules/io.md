# Module: io — `code/io/`

## Purpose
**Low-level input and timing**: keyboard, mouse, joystick/gamepad (SDL-based),
force feedback, the cursor, and the millisecond/mission timers. Raw input is
translated into game *actions* by the separate `code/controlconfig/` module.

## Key files
- `key.cpp` / `key.h` — keyboard state, scancodes, key queue.
- `mouse.cpp` / `mouse.h` — mouse position/buttons.
- `joy-sdl.cpp` / `joy.h`, `joy_ff-sdl.cpp` / `joy_ff.h` — joystick + force feedback.
- `timer.cpp` / `timer.h` — `timer_get_milliseconds()`, timestamps.
- `keycontrol.cpp` — in-mission key handling (`game_process_keys`).
- `cursor.*`, `spacemouse.*`.

## Core data structures / globals
- Key/mouse global state queried via `key_*` / `mouse_*` functions.
- `timestamp` and `Missiontime`/`flFrametime` timing (timing helpers here;
  game-time globals in `code/globalincs/systemvars.*`).

## Major constants
- Key scancodes `KEY_*` and modifier masks (`KEY_SHIFTED`, etc.) in `key.h`.
- Mouse button bits `MOUSE_LEFT_BUTTON`, etc. (`mouse.h`).

## Configuration tables
Input *bindings* are owned by `code/controlconfig/` →
`controlconfigdefaults.tbl` (`controlsconfigcommon.cpp`).

## See also
- `code/controlconfig/` (maps inputs → game actions / presets),
  `code/osapi/` (window + OS event pump that feeds this).
- Table option reference: https://wiki.hard-light.net/index.php/Tables
