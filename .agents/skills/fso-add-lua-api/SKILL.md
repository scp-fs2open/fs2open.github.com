---
name: fso-add-lua-api
description: >-
  Expose engine functionality to Lua scripting in FSO via ADE bindings or add a
  new scripting hook. Use when adding a Lua API object/library/function/property,
  writing ADE_OBJ/ADE_FUNC/ADE_VIRTVAR/ADE_LIB bindings, or creating a
  scripting::Hook/OverridableHook that mods can subscribe to.
---

# FSO: Add a Lua/ADE Binding or Hook

FSO's Lua layer (ADE) lives in `code/scripting/`. There are two common tasks:
**A)** expose data/functions to Lua, and **B)** fire an engine event mods can hook.
(For mission-designer scripting use the `fso-add-sexp` skill instead.)

## A) Add a Lua binding (ADE)

Bound **objects** live in `code/scripting/api/objs/`, **libraries** in
`code/scripting/api/libs/`. Copy an existing file (e.g. `objs/wing.cpp`) as a model.

- **Define an object type:**
```cpp
ADE_OBJ(l_MyThing, my_thing_h, "mything", "My thing handle");
```
- **Add a method:**
```cpp
ADE_FUNC(doStuff, l_MyThing, "number amount",
    "Does stuff to the thing.", "boolean", "true on success")
{ /* parse args with ade_get_args, act, return with ade_set_args */ }
```
- **Add a gettable/settable property:**
```cpp
ADE_VIRTVAR(Name, l_MyThing, "string", "The name.", "string", "name or empty")
{ /* ... */ }
```
- **Add to a library:** use `ADE_LIB` / `ADE_LIB_DERIV` and attach functions.

Argument marshalling uses `ade_get_args(L, "...", ...)` and
`ade_set_args(L, "...", ...)`; format strings are documented in
`code/scripting/ade_args.h`. Match the documented type string in the macro to the
actual returned type, since the Lua API docs are generated from these.

## B) Add a scripting hook

1. Declare the hook in `code/scripting/global_hooks.h` and define it in
   `global_hooks.cpp`, choosing `scripting::Hook<...>` (non-overridable) or
   `scripting::OverridableHook<...>` (script can replace default behaviour).
2. Define any condition/parameter types via the hook's template args (see
   `hook_conditions.h` / `hook_api.h`).
3. **Fire it** from the relevant subsystem at the right point:
```cpp
if (scripting::hooks::OnMyEvent->isActive()) {
    scripting::hooks::OnMyEvent->run(scripting::hook_param_list(/* params */));
}
```
   For overridable hooks, check `isOverride(...)` to let scripts suppress default logic.

## Conventions

- Keep binding docs accurate — they are the generated Lua API reference (`doc_*`).
- Register script files/tables via `scripting.tbl` (`script_parse_table`).
- Keep warning-clean (`FSO_FATAL_WARNINGS=ON`).

## Verify

- Build (see `fso-build-and-test`).
- Test from a Lua script (a `scripting.tbl` `$On Game Init:` block or a hook) that
  calls the new API / responds to the new hook.

## Reference

- `code/scripting/ade_api.h`, `code/scripting/ade_args.h`, `code/scripting/hook_api.h`.
- `documentation/modules/scripting.md`.
- Lua API wiki: https://wiki.hard-light.net/index.php/Scripting
