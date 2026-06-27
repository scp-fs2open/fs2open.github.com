---
name: fso-add-hud-gauge
description: >-
  Add a new HUD gauge to FSO. Use when creating a HUD element/gauge, subclassing
  HudGauge, adding a HUD_OBJECT_ type, or wiring a gauge into hud_gauges.tbl
  parsing and the per-frame HUD render path.
---

# FSO: Add a HUD Gauge

HUD gauges live in `code/hud/`. Each gauge is a subclass of `HudGauge`
(`code/hud/hud.h`, ~line 214) with a `render()` method; gauges are instantiated
from `hud_gauges.tbl` via `code/hud/hudparse.cpp`. Decide first which path you need.

## Choose an approach

- **Mod-authored custom gauge** → usually no engine change: mission/mod authors use
  the existing `HUD_OBJECT_CUSTOM` gauge (and Lua HUD-draw hooks). Point the user to
  the `fso-add-lua-api` skill + `hud_gauges.tbl` if that suffices.
- **New built-in gauge type** → follow the steps below.

## Steps (new built-in gauge)

1. **Subclass `HudGauge`.** Create `hudmygauge.h`/`.cpp` (or add to a related
   existing gauge file). Implement the constructor (gauge type, default position,
   colors) and override `render(float frametime)` using `gr_*` / `renderString` /
   `renderBitmap` helpers from the base class.

2. **Add a gauge type id.** Add `#define HUD_OBJECT_MY_GAUGE NN` in
   `code/hud/hudparse.h` (the `HUD_OBJECT_*` list) and keep the count in sync.

3. **Parse + load.** In `code/hud/hudparse.cpp`:
   - Add a `load_gauge_my_gauge(...)` that reads gauge settings (coords, font,
     color, frames) and constructs your gauge, registering it on the ship/HUD.
   - Add a `case HUD_OBJECT_MY_GAUGE:` to the load dispatch (the switch around
     line 1184) calling `load_gauge_my_gauge(settings)`.
   - Map the gauge's table name string to the new id where gauge names are parsed.

4. **Render path.** Gauges added to the HUD list are rendered automatically each
   frame; confirm your gauge is added to the per-ship gauge list so it draws.

5. **Defaults.** Provide retail-style default coordinates for 640 and 1024 layouts
   like the existing gauges, so it works without an explicit `hud_gauges.tbl` entry.

## Conventions

- Reuse base-class draw helpers; don't call backend `gr_*` for text/bitmaps if a
  `HudGauge::render*` helper exists.
- Respect HUD config (gauge can be toggled); check the gauge's active/visible state.
- Keep warning-clean (`FSO_FATAL_WARNINGS=ON`).

## Verify

- Build (see `fso-build-and-test`), start a mission, confirm the gauge renders at
  the right place, scales across resolutions, and obeys HUD config show/hide.
- Test a `hud_gauges.tbl` entry that positions/recolors it.

## Reference

- `code/hud/hud.h` (`HudGauge`), `code/hud/hudparse.{h,cpp}`, `documentation/modules/hud.md`.
- hud_gauges.tbl docs: https://wiki.hard-light.net/index.php/Hud_gauges.tbl
