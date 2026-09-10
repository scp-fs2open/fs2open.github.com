---
name: fso-add-table-field
description: >-
  Add or modify a parsed option in an FSO table file (.tbl/.tbm) such as
  ships.tbl, weapons.tbl, ai_profiles.tbl, game_settings.tbl, hud_gauges.tbl.
  Use when adding a new ships.tbl/weapons.tbl entry field, a new table option,
  a new $Property to a table, or wiring a parsed value into an *_info struct.
---

# FSO: Add a Table (.tbl) Field

FSO parses `.tbl`/`.tbm` text into global `*_info` structs using the hand-rolled
parser in `code/parse/parselo.*`. Adding an option means: extend the struct, parse
it, give it a default, and (if applicable) expose it to FRED and reset it on remix.

## Locate the owning module and parser

Tables map to modules (see `documentation/modules/<module>.md`). Common ones:

| Table | Parser function | File |
| --- | --- | --- |
| `ships.tbl` | `parse_ship_values` / `parse_shiptbl` | `code/ship/ship.cpp` |
| `weapons.tbl` | `parse_weapon` / `parse_weaponstbl` | `code/weapon/weapons.cpp` |
| `ai_profiles.tbl` | `parse_ai_profiles_tbl` | `code/ai/ai_profiles.cpp` |
| `game_settings.tbl` | `parse_mod_table` | `code/mod_table/mod_table.cpp` |
| `hud_gauges.tbl` | `parse_hud_gauges_tbl` | `code/hud/hudparse.cpp` |

Find the exact spot by searching the parse function for a nearby `optional_string`.

## Steps

1. **Add the field to the `*_info` struct** (e.g. `ship_info` in `code/ship/ship.h`).
   Give it a sensible default in the struct or its reset/`clear()` path.
2. **Parse it.** In the module's parse function, copy an adjacent block:

```cpp
if (optional_string("$My New Option:")) {
    stuff_float(&sip->my_new_option);   // or stuff_int / stuff_string / stuff_boolean / stuff_vec3d
}
```
   - Use `optional_string` for new fields (never `required_string` — it breaks old tables).
   - For names that reference other tables, use the existing lookup helpers
     (e.g. `ship_info_lookup`, `weapon_info_lookup`).
3. **Respect `.tbm` modularity.** Most parsers run once per table file and merge
   `*-shp.tbm`/`*-wep.tbm` modular tables; ensure your `optional_string` sits inside
   the same per-entry loop so modular edits apply.
4. **Use the parsed value** where the behaviour lives.
5. **FRED (if author-facing):** mirror new ship/weapon fields in `fred2/` and
   `qtfred/` if they should be editable in the mission editor.
6. **Defaults & tables:** if you ship a new stock option, update the in-tree
   default tables under `def_files/` only when appropriate.

## Conventions

- Parsing advances the global pointer `Mp`; don't read raw text yourself.
- Keep the option name style consistent: `$Title Case:` for fields, `+Sub Option:`
  for sub-fields, `#Section` for sections.
- Build with `FSO_FATAL_WARNINGS=ON` (CI default) — keep it warning-clean.

## Verify

- Build (see the `fso-build-and-test` skill) and load a mission/table that uses it.
- Add a unit test under `test/src` if the parsed value drives testable logic.

## Reference

- Parser API: `code/parse/parselo.h` (`optional_string`, `stuff_*`, field-type `F_*`).
- Table option docs: https://wiki.hard-light.net/index.php/Tables
