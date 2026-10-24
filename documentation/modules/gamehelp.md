# Module: gamehelp — `code/gamehelp/`

## Purpose
**In-game help.** Two separate things:

1. **Context help overlays** — the labelled diagrams that appear over a screen
   when the player presses the help key, pointing at its buttons and regions.
2. **The gameplay help screen** — the scrollable list of keyboard commands and
   explanations reachable from the main hall.

## Key files
- `contexthelp.cpp` / `contexthelp.h` — the per-screen overlays.
- `gameplayhelp.cpp` / `gameplayhelp.h` — the gameplay help screen.

## Core data structures / globals
- `gameplay_help_section` + `SCP_vector<gameplay_help_section> Help_text` — the
  parsed sections of the help screen.
- Overlays are identified by **name string**, not by number. The names are
  defined in `contexthelp.h`: `SS_OVERLAY` (`"ship"`), `WL_OVERLAY`
  (`"weapon"`), `BR_OVERLAY` (`"briefing"`), `MH_OVERLAY` (`"main"`),
  `MH2_OVERLAY` (`"main2"`), `BARRACKS_OVERLAY`, `CONTROL_CONFIG_OVERLAY`,
  `DEBRIEFING_OVERLAY`, `HOTKEY_OVERLAY`, `CAMPAIGN_ROOM_OVERLAY`,
  `SIM_ROOM_OVERLAY`, and the multiplayer ones (`MULTI_CREATE_OVERLAY`,
  `MULTI_START_OVERLAY`, `MULTI_JOIN_OVERLAY`).

## Key entry points
- `help_overlay_get_index(overlay_name)` — resolve a name to an id once, then
  keep the id.
- `help_overlay_active(overlay_id)`, `help_overlay_set_state(overlay_id,
  resolution_index, state)`, `help_overlay_maybe_blit(overlay_id,
  resolution_index)` — the three calls a screen needs to support help.
- `context_help_init()` (once at startup), `context_help_grey_screen()` (dim the
  screen behind an overlay), `launch_context_help()`.
- `gameplay_help_init()`, `gameplay_help_do_frame(frametime)`.

Overlays are laid out per resolution, which is why `resolution_index` is a
parameter on both the state and blit calls.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `help.tbl` (+ `*-hlp.tbm`) | `contexthelp.cpp` | The context-help overlay definitions |

The **gameplay help screen is not table-driven**: `gameplay_help_init_text()`
builds `Help_text` at runtime from the current control bindings, which is why it
always matches what the player has actually bound.

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/menuui/` and `code/missionui/` (the screens that show overlays),
  `code/controlconfig/` (the key bindings the help screen lists),
  `code/popup/` (modal dialogs, a different overlay system),
  `code/localization/` (help text is translated).
