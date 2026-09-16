# Module: ui — `code/ui/`

## Purpose
The **low-level UI widget toolkit** used by the classic (retail-style) interface
screens: buttons, checkboxes, sliders, input boxes, list boxes, windows, and
gadget focus/keyboard handling. Higher-level screens are built on top of this in
`code/menuui/`, `code/missionui/`, and the newer `code/scpui/`.

## Key files
- `ui.h` — the widget classes (`UI_WINDOW`, `UI_BUTTON`, `UI_GADGET`, …).
- `window.cpp`, `button.cpp`, `gadget.cpp`, `checkbox.cpp`, `slider.cpp`,
  `inputbox.cpp`, `listbox.cpp`, `scroll.cpp`, `icon.cpp`, `radio.cpp`.

## Core data structures
- `UI_GADGET` (base) and the concrete widgets; `UI_WINDOW` owns a gadget list.

## Major constants
- Gadget state/style flags and key/mouse handling defines in `ui.h`.

## Configuration tables
| File | Parsed in | Purpose |
| --- | --- | --- |
| `tooltips.tbl` | `window.cpp` | UI tooltip strings |

## Related UI modules
- `code/scpui/` — modern libRocket-based UI (newer screens; data in retail UI VPs).
- `code/menuui/` — main hall, barracks, tech room, credits, player select
  (`mainhall.tbl`, `tips.tbl`, `menu.tbl`, `intel.tbl`).
- `code/missionui/` — briefing, ship/weapon select, debrief screens.

Table option reference: https://wiki.hard-light.net/index.php/Tables

## See also
- `code/graphics/` (rendering), `code/io/` (input), `code/localization/` (string tables).
