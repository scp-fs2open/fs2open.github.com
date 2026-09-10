# CLAUDE.md

This file is the Claude Code entry point for the FreeSpace 2 Open (FSO) repository.

The project guidance is maintained once, tool-agnostically, in `AGENTS.md`
(also read natively by Cursor, opencode, Codex, and other agents). Claude Code
does not read `AGENTS.md` on its own, so it is imported here:

@AGENTS.md

## Skills

Project skills are available under `.claude/skills/` and auto-trigger from natural
requests (or can be invoked explicitly by name). Each one is a thin pointer stub to
the canonical playbook in `.agents/skills/<name>/SKILL.md`, which is the
vendor-agnostic single source of truth shared across agents.

| Skill | Use it when you want to… |
| --- | --- |
| `fso-add-table-field` | Add/modify a parsed option in a `.tbl`/`.tbm` and wire it into an `*_info` struct |
| `fso-add-sexp` | Add a new SEXP operator (mission-scripting function) |
| `fso-add-lua-api` | Expose engine data/functions to Lua (ADE bindings) or add a `scripting::Hook` |
| `fso-add-object-type` | Introduce a new in-world `OBJ_*` entity (create/move/delete + collision + render) |
| `fso-add-hud-gauge` | Add a new built-in HUD gauge (`HudGauge` subclass + `hud_gauges.tbl` parsing) |
| `fso-build-and-test` | Configure/build with CMake+Ninja, run `unittests`, clang-format/clang-tidy |
| `thermo-nuclear-code-quality-review` | Run a strict maintainability/abstraction audit (explicit invocation only) |

See `.agents/skills/README.md` for the full skill index and shared conventions.
