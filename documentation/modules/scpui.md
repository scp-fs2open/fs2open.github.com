# Module: scpui — `code/scpui/`

## Purpose
The **modern, script-driven front end**. It embeds **libRocket** (an HTML/CSS-like
UI toolkit) into the engine and exposes it to Lua, so a mod can replace the
retail menu and pre-mission screens with its own documents instead of the
bitmap-and-widget screens in `code/menuui/` and `code/missionui/`. The engine
side is deliberately thin: it supplies libRocket with the interfaces it needs
(files, rendering, input, sound, scripting) and lets Lua drive everything else.

## Key files
- `rocket_ui.cpp` / `rocket_ui.h` — the `scpui::` entry points:
  `initialize()`, `shutdown()`, `getContext()`, `enableInput()`/`disableInput()`,
  `setOffset()`, `reloadAllContexts()`.
- `RocketRenderingInterface.*` — draws libRocket geometry through `gr_*`.
- `RocketFileInterface.*` — routes libRocket's file access through `cfile`, so
  documents load from VP archives and mod directories like any other asset.
- `RocketSystemInterface.*` — clock, logging, clipboard, cursor.
- `RocketLuaSystemInterface.*` — binds libRocket's scripting layer onto the
  engine's Lua state instead of a separate one.
- `RocketDecorators.*` / `RocketDecoratorsInstancer.*` — custom RCSS decorators.
- `SoundPlugin.*` — plays interface sounds on element events
  (`ui.playElementSound` in Lua).
- `IncludeNodeHandler.*` — an `<include>` custom node so documents can be split.
- `elements/AnimationElement.*`, `elements/ScrollingTextElement.*` — custom
  elements for engine animations and scrolling credits-style text.

## Core concepts
- **A context is a screen.** Lua creates and owns libRocket contexts;
  `scpui::getContext()` returns the current one and `enableInput()` routes mouse
  and text input to exactly one context at a time.
- **The Lua surface is `ui`** — the `UserInterface` library in
  `code/scripting/api/libs/ui.cpp` (`ADE_LIB(l_UserInterface, ..., "ui", ...)`).
  It is the only intended way to drive this module: `ui.enableInput`,
  `ui.disableInput`, `ui.setOffset`, `ui.playElementSound`, `ui.linkTexture`,
  `ui.playCutscene`, plus sub-libraries such as `ui.PilotSelect` that expose the
  engine data a replacement screen needs.
- **Documents are content, not code.** libRocket markup and style sheets ship as
  ordinary mod assets. `RocketFileInterface` maps a libRocket path onto a
  `CF_TYPE_*` by resolving the leading directory with `cfile_get_path_type()`
  (a bare filename searches everywhere, `CF_TYPE_ANY`), so documents load from
  VP archives and mod directories like any other asset. Adding a screen is
  usually Lua and markup work, not engine work.
- Engine work is needed only for a genuinely new capability: a custom element, a
  decorator, or a new `ui.*` binding.

## Configuration tables
None of its own. Screens are registered through `scripting.tbl` like any other
Lua code (see `code/scripting/`).

## See also
- `code/scripting/api/libs/ui.cpp` — the Lua bindings; start here.
- `code/ui/` (the retail widget toolkit this replaces), `code/menuui/` and
  `code/missionui/` (the retail screens), `code/graphics/` (rendering),
  the vendored `lib/libRocket` submodule (do not edit).
