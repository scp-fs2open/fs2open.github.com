# Module: actions — `code/actions/`

## Purpose
A small **table-driven scripting language for effects**. A mod author writes a
short *program* in a table — a sequence of steps like "move to this submodel,
wait, play a sound, spawn a particle effect" — and the engine runs it when the
matching event happens. It fills the gap between a fixed table field and full
Lua: no script file, no hook, but more than a single value.

It is used for things like a weapon's on-create effect and a beam's warmdown.

## Key files
- `Action.cpp` / `Action.h` — the `Action` base class and `ActionResult`.
- `Program.cpp` / `Program.h` — `Program` (the parsed sequence), `ProgramSet`,
  `ProgramInstance` (one running execution), and `ProgramState`.
- `ActionDefinition.*`, `BuiltinActionDefinition.h`,
  `ActionDefinitionManager.*` — the registry that maps a table keyword to an
  action type and its parameters.
- `common.cpp` / `common.h` — `ProgramLocals` and `ProgramContextFlags`.
- `types/` — the built-in actions: `WaitAction`, `PlaySoundAction`,
  `SetPositionAction`, `SetDirectionAction`, `MoveToSubmodel`,
  `ParticleEffectAction`.
- `expression/` — the expression sub-language: `ActionExpression`,
  `ExpressionParser`, `FunctionManager`, `TypeDefinition`, `Value`,
  `ProgramVariables`, and the `nodes/` AST.

## Core data structures
- `class Action` — one step. Implement `execute(ProgramLocals&)` returning an
  `ActionResult` and `clone()`.
- `enum class ActionResult` — `Finished`, `Errored`, or **`Wait`**. `Wait` is
  what makes programs interesting: an action can suspend and be resumed on a
  later frame, which is how `WaitAction` and timed sequences work.
- `class Program` — a `SCP_vector<std::unique_ptr<Action>>`, copyable via
  `Action::clone()`. `ProgramSet` groups programs parsed for one host.
- `class ProgramInstance` — one independent execution: a current-instruction
  index plus its own `ProgramLocals`. `step()` advances it and returns a
  `ProgramState` (`Running`, `Done`, `Errored`).
- `struct ProgramLocals` — the per-instance state an action can read and write:
  the `host` (`object_h`), `localPosition` / `localOrient`, `hostSubobject`,
  the `waitTimestamp` used by `Wait`, and the expression `variables`.
- `FLAG_LIST(ProgramContextFlags)` — `HasObject`, `HasSubobject`. These are
  **validated at parse time**: a program that needs an object host is rejected
  when parsed in a context that will not supply one, rather than failing at
  runtime.

## Adding an action
1. Add a class under `types/` deriving from `Action`, implementing `execute()`
   and `clone()`.
2. Register it with `ActionDefinitionManager` (see `BuiltinActionDefinition`),
   declaring its `ActionParameter` list and the `ProgramContextFlags` it needs.
3. Return `ActionResult::Wait` rather than blocking if it needs time to pass.

## Configuration tables
None of its own. Programs are parsed **inline** in the tables that host them —
see `weapon_info::on_create_program` (`code/weapon/weapon.h`) and
`beam_warmdown_program` (`code/model/model.h`).

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/weapon/`, `code/model/`, `code/ship/` (the hosts that carry programs),
  `code/particle/` (`ParticleEffectAction`), `code/gamesnd/`
  (`PlaySoundAction`), `code/parse/sexp.*` and `code/scripting/` (the other two
  scripting systems, for mission logic and full Lua respectively).
