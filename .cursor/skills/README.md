# FSO Project Skills

Project-scoped Cursor Agent Skills for the FreeSpace 2 Open engine. Each skill is
a directory containing a `SKILL.md` that encodes an FSO-specific workflow
(conventions, the exact files/functions to touch, and how to verify). Skills
auto-trigger from natural requests; you can also invoke one explicitly by name.

## Available skills

| Skill | Use it when you want to… | Key code | Module guide |
| --- | --- | --- | --- |
| `fso-add-table-field` | Add/modify a parsed option in a `.tbl`/`.tbm` (ships, weapons, ai_profiles, game_settings, hud_gauges) and wire it into an `*_info` struct | `code/parse/parselo.*` + the owning module's parser | `documentation/modules/parse.md` |
| `fso-add-sexp` | Add a new SEXP operator (mission-scripting function): `OP_*`, `Operators` table, arg typing, `eval_sexp`, help text | `code/parse/sexp.{h,cpp}` | `documentation/modules/parse.md` |
| `fso-add-lua-api` | Expose engine data/functions to Lua via ADE bindings, or add a `scripting::Hook` mods can subscribe to | `code/scripting/` (`api/`, `global_hooks.*`) | `documentation/modules/scripting.md` |
| `fso-add-object-type` | Introduce a new in-world `OBJ_*` entity with create/move/delete + collision + render | `code/object/object.{h,cpp}` | `documentation/modules/object.md` |
| `fso-add-hud-gauge` | Add a new built-in HUD gauge (`HudGauge` subclass + `HUD_OBJECT_*` + `hud_gauges.tbl` parsing) | `code/hud/hud.h`, `code/hud/hudparse.*` | `documentation/modules/hud.md` |
| `fso-build-and-test` | Configure/build with CMake+Ninja, run `unittests`, and run clang-format/clang-tidy, mirroring CI | `CMakeLists.txt`, `ci/linux/*` | root `AGENTS.md` |

## Conventions shared by all skills

- Builds use `FSO_FATAL_WARNINGS=ON` (CI default) — keep changes warning-clean.
- Code must compile on GCC, Clang, and MSVC; isolate platform code under
  `code/osapi/`, `code/windows_stub/`, or platform guards.
- New parsed options use `optional_string` (never `required_string`) to preserve
  backward compatibility with existing tables.
- Author-facing ship/weapon/SEXP changes may also need FRED updates (`fred2/`, `qtfred/`).
- After any change, validate with the `fso-build-and-test` skill.

## How skills load

- **Automatic:** the agent matches a request to a skill's `description` and reads
  its `SKILL.md` before acting.
- **Explicit:** reference the skill by name (e.g. "use `fso-add-sexp`").

## Authoring more skills

Use the built-in `create-skill` workflow. Place new project skills here
(`.cursor/skills/<name>/SKILL.md`); keep descriptions third-person with clear
WHAT + WHEN trigger terms, and link one level deep to the relevant
`documentation/modules/*.md`.

## Related documentation

- Engine architecture overview: `documentation/ARCHITECTURE.md`
- Per-module entry-point guides: `documentation/modules/`
- Build/style/test conventions: root `AGENTS.md`
- Table-option reference (wiki): https://wiki.hard-light.net/index.php/Tables
