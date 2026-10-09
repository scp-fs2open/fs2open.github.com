# Module: missioneditor — `code/missioneditor/`

## Purpose
**Editor code shared by FRED2 and qtFRED.** It lives in the engine library
(`code/`), not in either editor, because both need it: mission and campaign
*saving*, the SEXP tree model, object duplication, and the editor-side helpers
that keep the engine's global arrays consistent when a designer moves things
around.

This module exists to stop the two editors from drifting apart. Anything both
editors need, and that is not UI, belongs here rather than being written twice.

## Key files
- `common.cpp` / `common.h` — editor-side helpers over the engine globals: ship
  and wing slot management, name-conflict checking, player-start rules, the
  voice-acting manager settings, and arrival-anchor conversion.
- `missionsave.cpp` / `missionsave.h` — `Fred_mission_save`: writes a `.fs2`
  file. `FredSaveConfig` and `MissionTemplateInfo` carry the editor-supplied
  settings (format, view position, alt names, callsigns, backup behaviour).
- `campaignsave.cpp` / `campaignsave.h` — `Fred_campaign_save`, which derives
  from `Fred_mission_save`, plus `campaign_link`.
- `sexp_tree_model.cpp` / `sexp_tree_model.h` — `SexpTreeModel` and the
  UI-agnostic SEXP tree types.
- `sexp_tree_actions.cpp` / `.h` — `SexpTreeActions`: the operations performed
  on a tree.
- `sexp_tree_opf.cpp` / `.h` — `SexpTreeOPF`: builds the list of values valid
  for a given `OPF_*` argument type.
- `sexp_annotation_model.cpp` / `.h` — SEXP comments and annotations.
- `objectduplication.cpp` / `.h` — deep-copy helpers:
  `clone_ship_instance_data()`, `clone_prop_instance_data()`,
  `clone_jump_node_instance_data()`, `clone_waypoint_path_instance_data()`.

## The model/view split
`sexp_tree_model.h` is the clearest example of the pattern this module uses:
- `SexpTreeModel` owns the tree data and all the logic.
- `ISexpTreeUI` is a pure-virtual callback interface the model calls to update
  the widget — `ui_insert_item()`, `ui_delete_item()`, `ui_set_item_text()`,
  `ui_expand_branch()`, and so on. Tree-item handles are opaque `void*`, which
  each editor casts to its own type. **FRED2 implements it with MFC
  `CTreeCtrl`; qtFRED with `QTreeWidget`.**
- `SexpTreeEditorInterface` lets an editor customize what is allowed.
- `SexpContextMenuState` is computed by the model and consumed by the UI, so
  the menu rules live in one place.

Follow the same shape when moving more editor logic here: model and rules in
`code/missioneditor/`, widget calls behind an interface.

## Core data structures / globals
- `FredShipSlotConfig` / `FredWingSlotConfig` — the editor-side arrays (alt
  names, callsigns, current selection, wing objects) passed into the slot
  helpers. Fields left `nullptr` are skipped.
- `sexp_tree_item`, `sexp_list_item`, `NodeImage` — the tree node types.
- The voice-acting manager settings: `Voice_abbrev_*`,
  `Voice_script_entry_format`, `Voice_export_selection`,
  `Voice_group_messages`, `Voice_no_replace_filenames`, `PersonaSyncIndex`.

## Slot-management contract (read before touching it)
`reassign_ship_slot()`, `swap_ship_slots()`, `rotate_ship_slots()` and the wing
equivalents move an entry between slots in `Ships[]` / `Wings[]` and fix up
**every** back-reference: `Objects`, `Ai_info`, `Wings`,
`Player_start_shipnum`, `Ship_registry`, and the editor-side arrays. The headers
state the preconditions explicitly, and note that **no caller may hold a
`ship*` or `wing*` to either slot across the call**. Also call
`ensure_valid_player_start_shipnum()` after anything that changes an
`OBJ_START`, and `update_custom_wing_indexes()` after changing the starting,
squadron, or TVT wing lists.

## Major constants
- `DEFAULT_NEBULA_RANGE` (3000.0f) — the AWACS range used when nebula intensity
  is unset or invalid.
- `ORIENT_INPUT_THRESHOLD` (0.01f) — the smallest meaningful change in an
  orientation field, in degrees.
- `INVALID_MESSAGE`.

## Configuration tables
None. It **writes** `.fs2` mission and campaign files; the corresponding
readers are in `code/mission/`.

## See also
- `fred2/` and `qtfred/` (`modules/fred2.md`, `modules/qtfred.md` — the two
  editors that consume this), `code/mission/missionparse.*` (the reader side of
  `Fred_mission_save`), `code/parse/sexp.*` (the SEXP definitions the tree model
  presents), `code/ship/` and `code/object/` (the globals the slot helpers
  repair).
