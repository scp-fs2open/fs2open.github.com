# Module: popup — `code/popup/`

## Purpose
**Modal dialogs.** The blocking "Are you sure?" / "Mission failed" boxes shown
over whatever screen is active, plus the special death popup that appears when
the player is killed. A popup takes over input and runs its own loop until the
player picks an option, so it works from anywhere without needing its own game
state.

## Key files
- `popup.cpp` / `popup.h` — the general popup: `popup()` and its flag set.
- `popupdead.cpp` / `popupdead.h` — the death popup (respawn / return to
  briefing / quit), which needs its own handling because it appears mid-mission
  and interacts with multiplayer respawn.

## Key entry points
- `popup(flags, nchoices, ...)` — the main call. Variadic: the button texts
  come first, then the message text. It **blocks** and returns the index of the
  choice the player made.
- `popup_till_condition(condition, ...)` — show a popup until a callback says to
  stop; used for "please wait" boxes with a cancel button.
- `popup_conditional_do()` / `popup_conditional_close()` — the same idea for
  code that drives its own loop.
- `popup_input(flags, caption, max_output_len, default_input, vchar)` — a
  text-entry popup; `vchar` is the set of extra non-alphanumeric characters
  accepted.
- `popup_active()`, `popup_running_state()`, `popup_kill_any_active()`,
  `popup_change_text()`.
- Death popup: `popupdead_start()`, `popupdead_do_frame()`,
  `popupdead_is_active()`, `popupdead_close()`.

## Major constants
- Standard button texts, already `XSTR`'d: `POPUP_OK`, `POPUP_CANCEL`,
  `POPUP_YES`, `POPUP_NO`.
- Layout flags: `PF_TITLE` and `PF_TITLE_BIG` (the first line is a centred
  title), `PF_BODY_BIG`.
- Colour flags: `PF_TITLE_RED`, `PF_TITLE_GREEN`, `PF_TITLE_BLUE`,
  `PF_TITLE_WHITE`, `PF_BODY_RED`, `PF_BODY_GREEN`, `PF_BODY_BLUE`.
- Icon flags: `PF_USE_NEGATIVE_ICON` (always drawn first),
  `PF_USE_AFFIRMATIVE_ICON`, `PF_NO_SPECIAL_BUTTONS`.
- Behaviour flags: `PF_RUN_STATE` — keep running the underlying state's
  `do_frame` behind the popup, which is what you want over live gameplay;
  `PF_IGNORE_ESC`; `PF_NO_NETWORKING`; `PF_WEB_CURSOR_1`.
- `POPUP_DEFAULT_VALID_CHARS` (`"_.-"`) and `POPUP_DEFAULT_PLUS_SPACE` — the
  non-alphanumeric characters a text-input popup accepts.

## Conventions
- Because `popup()` blocks, never call it from inside a per-frame render or
  simulation path unless you pass `PF_RUN_STATE` and understand that the state
  underneath keeps ticking.
- Use the `POPUP_*` button constants rather than new literals, so the buttons
  stay translated.

## Configuration tables
None.

## See also
- `code/ui/` (the widget toolkit), `code/gamehelp/` (the other overlay system),
  `code/osapi/dialogs.*` (`Error`/`Warning` — the *developer* dialogs, a
  different thing), `code/localization/` (`XSTR`).
