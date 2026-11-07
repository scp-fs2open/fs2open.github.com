# Module: cheats_table — `code/cheats_table/`

## Purpose
**Table-defined cheat codes.** The engine watches recently typed keys against a
rolling buffer; when the buffer ends with a registered code, the matching cheat
fires. Mods define their own codes and effects in `cheats.tbl` rather than
needing engine changes.

## Key files
- `cheats_table.cpp` / `cheats_table.h` — the whole module.

## Core data structures
- `class CustomCheat` — one cheat: its `cheatCode` (the string to type), the
  `cheatMsg` shown when it fires, and `requireCheatsEnabled`. Override
  `runCheat()` to add behaviour; `canUseCheat()` applies the gating.
- `class SpawnShipCheat : public CustomCheat` — the one built-in subclass,
  which spawns a named ship of a given class. It is the model to copy for a new
  cheat type.
- `customCheats` — the name-to-cheat map.

## Major constants
- `CHEAT_BUFFER_LEN` (17) — the rolling key buffer, and so the **maximum cheat
  code length**.

## Key entry points
- `cheat_table_init()`, `parse_cheat_table(filename)`.
- `checkForCustomCheats(converted_buffer, buffer_length)` — called from the key
  handling path with the current buffer.

Cheats can also fire a Lua hook, so a mod can react without a new C++ subclass —
see `code/scripting/global_hooks.*`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `cheats.tbl` (+ `*-cht.tbm`) | `parse_cheat_table()` / `cheat_table_init()` | Cheat codes, messages, and effects |

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/io/key.*` and `code/io/keycontrol.cpp` (where the buffer is fed),
  `code/scripting/` (the cheat hook), `code/ship/` (what `SpawnShipCheat`
  creates), `code/hud/hudmessage.*` (the message shown).
