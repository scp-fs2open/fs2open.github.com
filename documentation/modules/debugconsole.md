# Module: debugconsole — `code/debugconsole/`

## Purpose
The **developer console**: the in-game command line where a developer can flip a
flag, dump state, or trigger a debug action without rebuilding. Commands are
registered with macros from anywhere in the engine, so a subsystem adds its own
without touching this module.

## Key files
- `console.cpp` / `console.h` — the console itself, the `debug_command` type,
  and the `DCF*` registration macros.
- `consolecmds.cpp` — the built-in commands.
- `consoleparse.cpp` / `consoleparse.h` — the argument parser
  (`dc_stuff_*`, `dc_optional_string`, …), deliberately mirroring the table
  parser in `code/parse/parselo.*`.

## Core data structures / globals
- `class debug_command` — one registered command: its name, help text, and
  handler. Registration happens at static-init time through the macros.
- `Dc_debug_on`, `dc_commands_size`, `lastline`.
- `dc_command_str` — the rest of the command line, from the end of the last
  argument the parser consumed.

## Major constants
- `DC_MAX_COMMANDS` (300) — the registration cap.

## Registering a command
Use the macro that matches what the command does; the simple ones write
themselves:
- `DCF(name, help_text) { ... }` — a full custom command body.
- `DCF_BOOL(name, bool_variable)` and
  `DCF_BOOL2(name, bool_variable, short_help, long_help)` — toggle a bool.
- `DCF_INT(name, int_variable, short_help)` /
  `DCF_INT2(name, var, lower_bounds, upper_bounds, short_help)` — set an int,
  optionally range-checked.
- `DCF_FLOAT(name, float_variable, short_help)` / `DCF_FLOAT2(...)` — likewise
  for floats.

Inside a `DCF` body, read arguments with the `dc_*` parser
(`dc_stuff_int()`, `dc_stuff_float()`, `dc_stuff_boolean()`,
`dc_stuff_string()`, `dc_optional_string()`, `dc_required_string()`) and print
with **`dc_printf()`** — not `mprintf` — so the output goes to the console the
user is typing into. `dc_pause_output()` holds long output for paging.

## Configuration tables
None. Note the separate `debug_filter.cfg`, which controls `nprintf` log
categories (`code/osapi/outwnd.cpp`), is a different mechanism.

## See also
- `code/osapi/` (logging and the debug log window — `modules/osapi.md`),
  `code/tracing/` (the profiler these commands often switch on),
  `code/parse/parselo.*` (the parser this one mirrors),
  `code/cmdline/` (launch-time flags, the non-interactive equivalent).
