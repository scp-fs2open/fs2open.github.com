# Module: cmdline — `code/cmdline/`

## Purpose
Parses **command-line arguments** and exposes them as global `Cmdline_*` variables
that the rest of the engine reads during init and at runtime. This is the central
place for launch-time feature toggles (graphics, networking, debug, mod selection,
resolution, etc.). Many flags are also surfaced through the Knossos launcher.

## Key files
- `cmdline.cpp` / `cmdline.h` — `parse_cmdline()` and every `Cmdline_*` global.

## Core data structures / globals
- `int parse_cmdline(int argc, char *argv[])` — called early from `game_main()`.
- A large set of `extern` globals declared in `cmdline.h` and defined in
  `cmdline.cpp`, grouped as **RETAIL OPTIONS** and **FSO OPTIONS** (graphics,
  network, audio, debug). Examples: `Cmdline_window`, `Cmdline_res`,
  `Cmdline_freespace_no_sound`, `Cmdline_network_port`, `Cmdline_spew_pof_info`.

## Adding a new command-line flag
1. Declare `extern <type> Cmdline_my_flag;` in `cmdline.h` (in the right group).
2. Define it and register its parsing in `cmdline.cpp` (follow an adjacent flag:
   add to the options/parameter table and assign the parsed value).
3. Read `Cmdline_my_flag` where the behaviour lives.
4. Ensure it doesn't break FRED or TestCode (the header warns about this).

## Configuration tables
None. (Command-line flags are launch arguments, not table data. Persistent
*player* options live in `code/options/` / `default_settings.tbl`; engine-wide mod
behaviour lives in `code/mod_table/` / `game_settings.tbl`.)

## See also
- `code/mod_table/` (per-mod engine settings), `code/options/` (player option values).
- `freespace2/freespace.cpp` (`game_main` calls `parse_cmdline`).
- Command-line reference: https://wiki.hard-light.net/index.php/Command-Line_Reference
