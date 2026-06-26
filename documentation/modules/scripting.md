# Module: scripting — `code/scripting/`

## Purpose
The **Lua scripting system** exposed to mods (distinct from SEXPs in `code/parse/`).
Provides ADE — the C++↔Lua binding layer — plus the **hook** system that lets Lua
scripts react to engine events (on-frame, on-death, HUD draw, key press, …).

## Key files
- `ade.cpp` / `ade.h`, `ade_api.h`, `ade_args.*` — the binding framework and the
  macros used to register Lua classes/libraries/functions.
- `api/objs/` — bound Lua *objects* (ship, weapon, vector, …).
- `api/libs/` — bound Lua *libraries* (mn, gr, ba, hv, …).
- `hook_api.h` / `hook_api.cpp` — `scripting::Hook` / `OverridableHook`.
- `global_hooks.*` — central registry of built-in hooks.
- `hook_conditions.*` — conditional gating of hooks.
- `scripting.cpp` / `scripting.h` — `script_state`, `Script_system`, table parsing.
- `lua/` — Lua interpreter integration; `doc_*` — documentation generators.

## Core data structures / globals
- `script_state Script_system` — the global Lua state/manager.
- `script_hook` — an override + hook function pair.
- ADE registers types via `ade_obj<>` / `ade_lib` (`ade_api.h`).

## Major constants
- Language flag `SC_LUA` (1<<0).
- Conditional types `CHC_*`, legacy global actions `CHA_*` (`scripting.h`).
- `MAX_HOOK_CONDITIONS` (8).

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `scripting.tbl` (+ `*-sct.tbm`) | `script_parse_table()` | Script files & hook bindings |

Table option reference: https://wiki.hard-light.net/index.php/Tables (see *Scripting.tbl*).
Lua API reference is generated from the ADE bindings (`doc_*`).

## Extending
- Expose engine data to Lua → add an ADE object/library under `api/`.
- React to a new event → declare a `scripting::Hook` (or `OverridableHook`) in
  `global_hooks.*` and fire it from the relevant subsystem.

## See also
- `code/parse/sexp.*` (the other, mission-designer scripting system).
