# FSO Project Skills

Vendor-agnostic Agent Skills for the FreeSpace 2 Open engine. Each skill is
a directory containing a `SKILL.md` that encodes an FSO-specific workflow
(conventions, the exact files/functions to touch, and how to verify). Skills
auto-trigger from natural requests; you can also invoke one explicitly by name.

This directory (`.agents/skills/`) is the single source of truth. Vendor skill
directories contain thin pointer stubs back to it (see "Cross-agent compatibility"
below).

## Available skills

| Skill | Use it when you want to… | Key code | Module guide |
| --- | --- | --- | --- |
| `fso-add-table-field` | Add/modify a parsed option in a `.tbl`/`.tbm` (ships, weapons, ai_profiles, game_settings, hud_gauges) and wire it into an `*_info` struct | `code/parse/parselo.*` + the owning module's parser | `documentation/modules/parse.md` |
| `fso-add-sexp` | Add a new SEXP operator (mission-scripting function): `OP_*`, `Operators` table, arg typing, `eval_sexp`, help text | `code/parse/sexp.{h,cpp}` | `documentation/modules/parse.md` |
| `fso-add-lua-api` | Expose engine data/functions to Lua via ADE bindings, or add a `scripting::Hook` mods can subscribe to | `code/scripting/` (`api/`, `global_hooks.*`) | `documentation/modules/scripting.md` |
| `fso-add-object-type` | **Last resort** — introduce a new in-world `OBJ_*` entity with create/move/delete + collision + render; only when no existing object type (ship/weapon/debris/asteroid/etc.) can represent it | `code/object/object.{h,cpp}` | `documentation/modules/object.md` |
| `fso-add-hud-gauge` | Add a new built-in HUD gauge (`HudGauge` subclass + `HUD_OBJECT_*` + `hud_gauges.tbl` parsing) | `code/hud/hud.h`, `code/hud/hudparse.*` | `documentation/modules/hud.md` |
| `fso-build-and-test` | Configure/build with CMake+Ninja, run `unittests`, and check clang-format/clang-tidy compliance, mirroring CI | `CMakeLists.txt`, `ci/linux/*` | root `AGENTS.md` |

## Review skills

| Skill | Use it when you want to… | Invocation |
| --- | --- | --- |
| `thermo-nuclear-code-quality-review` | Run an extremely strict maintainability/abstraction audit of the current branch's changes (file-size growth, spaghetti conditionals, missed "code-judo" simplifications) | Explicit only (`disable-model-invocation`); ask for a "thermo-nuclear review" |

## Conventions shared by all skills

- Builds use `FSO_FATAL_WARNINGS=ON` (CI default) — keep changes warning-clean.
- Code must compile on GCC, Clang, and MSVC; isolate platform code under
  `code/osapi/`, `code/windows_stub/`, or platform guards.
- New parsed options use `optional_string` (never `required_string`) to preserve
  backward compatibility with existing tables — unless nested inside a block
  that's itself only entered after a new `optional_string` was matched (e.g.
  sub-fields of a new optional block), where `required_string` is fine since
  the whole block is already optional.
- Author-facing ship/weapon/SEXP changes may also need FRED updates (`fred2/`, `qtfred/`).
- After any change, validate with the `fso-build-and-test` skill.

## How skills load

- **Automatic:** the agent matches a request to a skill's `description` and reads
  its `SKILL.md` before acting.
- **Explicit:** reference the skill by name (e.g. "use `fso-add-sexp`").

## Cross-agent compatibility

The skills here live in a vendor-agnostic folder. Each vendor reads its own skills
directory, so the per-vendor directories carry thin pointer stubs (plain files, not
symlinks — symlinks are avoided for Windows checkout compatibility).

| Path | Agent | Kind |
| --- | --- | --- |
| `.agents/skills/` | opencode (native), shared canonical | full playbooks |
| `.cursor/skills/` | Cursor | pointer stubs → `.agents/skills` |
| `.claude/skills/` | Claude Code, opencode | pointer stubs → `.agents/skills` |

Project guidance (build/style/test conventions) is shared via the root `AGENTS.md`,
read natively by Cursor and opencode; Claude Code reads it through the root
`CLAUDE.md`, which imports `AGENTS.md`.

Each stub at `.cursor/skills/<name>/SKILL.md` and `.claude/skills/<name>/SKILL.md`
carries the discovery frontmatter (`name` + `description`) and a body that points
to the canonical playbook here. Edit the playbook here in `.agents/skills/`; if you
change a skill's `name`/`description`, mirror that line into the matching stubs so
triggering stays in sync. The Cursor stub for `thermo-nuclear-code-quality-review`
also keeps `disable-model-invocation: true` (a Cursor-only field) to preserve its
explicit-only behavior.

## Authoring more skills

Use the built-in `create-skill` workflow. Place new project skills here
(`.agents/skills/<name>/SKILL.md`); keep descriptions third-person with clear
WHAT + WHEN trigger terms, and link one level deep to the relevant
`documentation/modules/*.md`. Then add a matching pointer stub under
`.cursor/skills/<name>/SKILL.md` and `.claude/skills/<name>/SKILL.md`.

## Related documentation

- Engine architecture overview: `documentation/ARCHITECTURE.md`
- Per-module entry-point guides: `documentation/modules/`
- Build/style/test conventions: root `AGENTS.md`
- Table-option reference (wiki): https://wiki.hard-light.net/index.php/Tables
