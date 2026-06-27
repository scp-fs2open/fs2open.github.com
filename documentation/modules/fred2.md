# Module: fred2 — `fred2/` (FReespace EDitor)

## Purpose
**FRED2** is the original Windows-only mission editor. It is a separate executable
(not part of the game binary) that links against the engine code in `code/` and
adds an **MFC**-based GUI for building `.fs2` missions and campaigns: placing
ships/wings, editing arrivals/departures, briefings/debriefings, events and goals
(SEXP trees), backgrounds, and reinforcements.

It is the legacy editor; the cross-platform replacement is **qtFRED**
(`documentation/modules/qtfred.md`). Both reuse the same engine parsing/data.

## Build
- Built only on Windows when `FSO_BUILD_FRED2=ON` (see top-level `CMakeLists.txt`).
- Target: `Fred2`. Uses MFC; not available on Linux/macOS.

## Key files (representative)
- `fred.cpp`, `freddoc.*`, `fredview.*` — MFC app/document/view skeleton.
- `Management.*` — central editor state and engine glue.
- Editor dialogs (`*dlg` / `*EditorDlg`), e.g. `shipeditordlg.*`,
  `wing_editor.*`, `eventeditor.*`, `briefingeditordlg.*`,
  `campaigneditordlg.*`, `sexp_tree.*` (the SEXP tree control), `bgbitmapdlg.*`.
- `CMakeLists.txt` — target definition and MFC setup.

## Relationship to the engine
- FRED edits the same structures the game parses: ships → `ship_info`/`Ships`,
  parse objects → `p_object`/`Parse_objects` (`code/mission/missionparse.*`),
  events/goals → SEXPs (`code/parse/sexp.*`).
- When you add an author-facing field to a table or a new SEXP, FRED usually needs
  a matching UI/editor change here (and in qtFRED).
- FRED-only stubs replace some game-only systems; guard editor-specific behaviour
  rather than changing shared engine code.

## Configuration tables
None of its own; it reads the same content tables as the game (`ships.tbl`,
`weapons.tbl`, etc.) to populate editor lists.

## See also
- `documentation/modules/qtfred.md` (modern Qt editor), `code/mission/`,
  `code/parse/sexp.*`, `documentation/ARCHITECTURE.md`.
- Wiki: https://wiki.hard-light.net/index.php/FRED2 ·
  Table reference: https://wiki.hard-light.net/index.php/Tables
