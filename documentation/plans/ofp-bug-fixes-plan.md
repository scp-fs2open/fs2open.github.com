# Plan: port the OFP bug fixes to master

Source: [OFP wiki, "OFP Bug Fixes"](https://github.com/ofp-fs2open/fs2open.github.com/wiki/OFP-Bug-Fixes)
(37 entries, newest first). Analysis date: 2026-11-10, on master `81caf06e7f`.

The analysis used only the descriptions on the wiki page. It did not use the OFP pull requests,
because we must make sure that each fix is correct and complete for master. The PR links below are
for traceability only.

In the "Wiki" line of each item, "#N" is the position of the entry in the wiki table (1 = first
row). The quoted title is the "Fix" column of that row. The `file:line` references are for master
`81caf06e7f`. The Phase 1 branch moves some of them, for example in `FredView.cpp` and `Editor.cpp`.

## Status (2026-11-11)

- Phase 1 is complete on the branch `fix/qtfred-various-fixes`, with one commit for each fix. Each
  item below has a "Status" line with its commit.
- The work found 6 more defects that the wiki does not list. See "Additional fixes" after Phase 1.
- Each commit builds with `-Werror`, with and without Vulkan, and the unit tests pass. No fix has
  had a runtime test in QtFRED yet. FRED2 builds only on Windows, so CI is the first build of the
  FRED2 changes.
- The branch is not pushed.

## Status (2026-11-12)

- Phase 2 is complete on the branch `fix/more_qtfred_Fixes`, which starts from master (not from the
  Phase 1 branch). Each fix has one commit. Each item of Phase 2 has a "Status" line.
- Each commit builds with `-Werror` and Vulkan. The tip also builds without Vulkan, and the unit
  tests pass. No fix has had a runtime test in QtFRED. The 2.4 focus behavior needs a test on X11
  and on Wayland. A headless Qt test of the 2.4 key and focus rules passes.
- Five refactor commits follow the fixes. They apply the findings of the thermo-nuclear code quality
  review: the shared `marked_objects_moved()`, one function for next and previous object,
  `mark_wing()` with an explicit `LockedObjects` argument, `Proj_fov` for the editor projection, and
  the new `TransformToolBar` class (FredView.cpp goes from 3194 to 2719 lines).
- 2.3 and 2.5 also change FRED2. FRED2 builds only on Windows, so CI is the first build of these
  changes.
- The branch is not pushed.
- Next: Phase 3.

## Legend

Evidence:

- **C** (confirmed): the defect is visible in the master code.
- **P** (plausible): the code makes the defect probable. It is not reproduced.
- **U** (unverified): the code does not show the defect. Reproduce it first.

Scope:

- **Q**: QtFRED only.
- **S**: shared editor code (`code/missioneditor`). The fix also changes FRED2.
- **G**: engine code. The game uses it too.

## Out of scope: master has no undo system

`qtfred/src/mission/Editor.cpp:429` and `:832` contain only `TODO(Phase 3): _undoStack->clear()`.
These wiki items need an undo stack first:

- "Set Group is undoable" (wiki #5, "QtFRED selection groups overlapped", OFP PR #83).
- "Undo history cleanup" (wiki #27, "QtFRED crash on exit", OFP PR #40).
- Undo of dock, undock, and camera moves (wiki #32, "QtFRED docked ships not moving together",
  OFP PRs #29, #76).
- The FOV toolbar box (wiki #11, "QtFRED toolbar boxes stayed highlighted", OFP PR #73). Master
  has no FOV box.

---

## Phase 1: crashes and data loss

### 1.1 Initial Status with more than one ship — C, Q

- Wiki: #3, "QtFRED Initial Status with several ships" (OFP PR #88). The wiki says: OK with
  several ships selected can crash, and it overwrites shared values (velocity, hull, and shields
  set to 101, guardian threshold, team color). Mixed values must show blank and stay unchanged
  on OK.
- Master:
  - `ShipInitialStatusDialogModel.cpp:281-290` checks `vflag` three times. It never uses
    `sflag` or `hflag`, so mixed shield and hull values do not show blank.
  - `apply()` (`:590-608`) writes `BLANKFIELD` (101) to all ships. It also writes the guardian
    threshold and `team_name` to all ships with no "mixed" check.
  - `updateDockingInfo()` runs in multi-edit mode, but in that mode there is no dockpoint array.
- Fix:
  - Use the correct flag for each field.
  - Add a "mixed" state for the guardian threshold and the team color.
  - Write a field only if the user changed it.
  - Do not call the dock update in multi-edit mode.
- Status: **done** in `e8f50c6402` (model) and `c3e3c22251` (dialog). The "-" display of a spin box
  needed a new widget, `MixedValueSpinBox`, because a typed value equal to the hidden value gave no
  signal. The Reinforcements dialog had the same defect and uses the widget too. The team color goes
  only to ships that use team colors.
- Still open: `changeSubsys()` uses `~` on a bool (see 4.1).

### 1.2 Autosave recovery — C, Q

- Wiki: #21, "QtFRED autosave recovery crash" (OFP PR #49). The wiki says: accepting the
  newer-autosave prompt asserts "We have full path, but no separator!?", and Save writes into
  the autosave folder. The recovered mission must keep its own file name.
- Master:
  - `Editor::maybeUseAutosave` (`Editor.cpp:151`) replaces the load path with an autosave path
    that contains `/`. `FredView::loadMissionFile` changes separators only on the original
    path. On Windows, this probably triggers the assert in `cfilesystem.cpp:1297`.
  - `FredView::on_mission_loaded` (`FredView.cpp:843`) then sets `saveName` to the autosave
    file.
- Fix:
  - Load the autosave content, but keep the original path for `saveName` and the window title.
  - Mark the mission as modified.
  - Use native separators.
- Status: **done** in `20233e1da0`. `Editor::loadMission()` now owns the recovery, and `Editor` holds
  the save path (`FredView::saveName` is removed). The code review found 3 more defects, also fixed
  in this commit:
  - A failed recovery bound an empty mission to the original file. Now a failed load gives an
    untitled mission.
  - Missions with the same file name shared one autosave. The autosave name now contains a hash of
    the full path. Autosaves from older QtFRED builds are not found after the update.
  - The timer wrote autosaves without changes, so Revert could offer the discarded changes. The timer
    now writes only for unsaved changes, and Revert offers no recovery.

### 1.3 Save As and templates — C, Q

- Wiki: #30, "QtFRED Save As didn't switch to the new file" (OFP PR #35). The wiki says: after
  Save As, the title still shows the old name. After Load Template, a later Save can overwrite
  the mission that was open before.
- Master:
  - `saveMissionAs` (`FredView.cpp:517`) does not update the title or `setWindowFilePath`.
  - A cancelled Save As clears `saveName`, because `:525` assigns the dialog result to it
    directly.
  - A `.fst` load (`:843`) keeps the `saveName` of the previous mission.
- Fix:
  - Put the dialog result in a local variable.
  - Clear `saveName` after a template load.
  - Make one helper for the title update and call it after Save As.
- Status: **done** in `58b834e406`. `Editor::loadMission()` makes each `.fst` file a template, so a
  template from the recent-files list or `-start_mission` is also safe.

### 1.4 Shutdown order — P, Q and S

- Wiki:
  - #8, "QtFRED crash on close in Lua shutdown" (OFP PR #75). QtFRED did not close the Lua
    script system, so it crashed during exit.
  - #27, "QtFRED crash on exit" (OFP PR #40). Open dialogs were cleaned up after the SEXP data
    that they used was freed.
  - #26, "QtFRED campaign editor close crash" (OFP PR #44). Closing the Campaign Editor or
    QtFRED asserted or crashed with "Called object is not of the correct type". QtFRED must
    also ask about unsaved campaign changes.
- Master:
  - `FredView::closeEvent` (`FredView.cpp:2279`) calls `shutdown()` while child dialogs that
    hold SEXP trees still exist.
  - `fso::fred::shutdown()` (`qtfred/src/mission/management.cpp:320`) does not call
    `Script_system.Clear()`. FRED2 has the same gap (`fred2/management.cpp:483`). The game
    calls it in `freespace.cpp:7170`.
  - "Called object is not of the correct type" comes from Qt (`qobjectdefs_impl.h`). Qt shows
    it when a slot is called on an object that is partly destroyed.
  - When QtFRED closes, it does not ask about unsaved campaign changes.
- Fix:
  - Close and delete all child dialogs first, including the campaign editor with its prompt.
  - Then call `Script_system.Clear()`, in QtFRED and in FRED2.
  - Then free the engine data.
  - Disconnect the campaign model and graph signals in the destructor of the dialog.
- Status: **done** in `a64b13fc39`. The new shared `editor_shutdown()` frees the engine and SEXP data,
  then closes Lua, sound, and graphics, in the same order as `game_shutdown()`. QtFRED and FRED2 both
  use it. Notes:
  - The fix for the Qt assert is a probable cause. It is not reproduced yet.
  - The campaign editor is application-modal (`CampaignEditorDialog.ui:6`). So the "ask about unsaved
    campaign changes on exit" case cannot occur from the main window.
  - If the user cancels the exit after an editor window closed, that window stays closed. This is
    deliberate, because a dialog model is not usable again after `apply()` or `reject()`.

### 1.5 Selection groups — C, Q

- Wiki: #5, "QtFRED selection groups overlapped" (OFP PR #83). The wiki says: groups were stored
  as numbers instead of one bit for each group, so groups overlapped.
- Master: `FredView.cpp:2914-2933` passes 1..9 as the bit mask. Thus group 3 is the same as
  groups 1 and 2. FRED2 uses `1 << n` (`fredview.cpp:4490`, `:4510`).
- Fix:
  - Pass `1 << (n-1)` to `onGroupSelected` and `onSetGroup`.
  - Make sure that `Ships[].group` has the same values as in FRED2.
- Status: **done** in `3d8ded8268`. A ship in no group has the value -1, and -1 has all bits set, so
  it matched each group, in QtFRED and FRED2. The new shared functions `ship_in_editor_group()` and
  `ship_set_editor_group()` fix this for both editors. Missions saved by older QtFRED builds can
  contain the old group numbers 1 to 9, which now read as bit masks.

### 1.6 Wing arrival distance reset to 0 — C, Q

- Wiki: #16, "QtFRED wing arrival distance reset to 0" (OFP PR #61). The wiki says: choosing an
  arrival target set the distance of the wing to 0, so saving failed with "Arrival distance for
  wing ... must be greater than 0". The distance must stay, unless it is less than the minimum
  for the target.
- Master: `WingEditorDialogModel.cpp:1011-1013` has the comparison reversed, and it sets 0.
- Fix: `if (w->arrival_distance < minDistance) setArrivalDistance(minDistance);`
- Status: **done** in `679a3b3790`. With a special anchor, the minimum was 0, so the save error was
  still possible. Commit `e3d8269edc` fixes that (see "Additional fixes").

### Additional fixes

The code reviews of Phase 1 found these defects, which the wiki does not list:

| Commit | Defect | Scope |
|---|---|---|
| `e3d8269edc` | The wing and ship editors each had a copy of the minimum arrival distance. Both accepted 0 for a special anchor, and the ship editor used the radius of the arriving ships, not the target. Now `arrival_location_uses_distance()` (missionparse) and `arrival_min_distance()` (shared editor code) hold the rule. | S, G (parser) |
| `c3e3c22251` | A mixed-value spin box ignored a typed value equal to its hidden value (Initial Status, Reinforcements). | Q |
| `b164b36ba7` | Lua AI goal arguments in `Ai_info` and `Wings` stayed after the Lua state closed, and their destructors used the closed state at exit. | G |
| `cdae6171e0` | Each recent-files menu update leaked up to 8 actions. | Q |
| `8333920364` | A failed FS1 import left the `.fsm` source file as the save target. | Q |
| `e7381ea72a` | Save and Save As ignored a failed `save_mission_file()`, cleared the modified flag, and showed no error. | Q |

---

## Phase 2: viewport and selection

### 2.1 Docked ships do not move together — C, Q

- Wiki: #32, "QtFRED docked ships not moving together" (OFP PRs #29, #76). The wiki says:
  moving or rotating a docked ship left its docked partners behind. It also lists cancelled
  drags, Orient editor turns, and undocking with "move ships when undocking".
- Master (root cause): `qtfred/src/mission/object.cpp:22` calls
  `flags.set(Docked_already_handled)`. FRED2 (`fred2/management.cpp:2430`) calls `remove`.
  Because the flag is set on all objects, `dock_move_docked_objects` returns immediately.
- Fix:
  - Change `set` to `remove`.
  - Call `object_moved()` from `FredView::onTransformEditingFinished`, which does not call it
    now.
  - Then check cancelled drags, Orient editor turns, and "move ships when undocking".
- Status: **done** in `483934415f`. The toolbar, a cancelled drag, a rotation in the Orient editor,
  and an undock now call `object_moved()`.

### 2.2 Lock Marked Objects can be bypassed — C, Q (FRED2 has some of the same gaps)

- Wiki: #9, "QtFRED Lock Marked Objects could be bypassed" (OFP PR #74). The wiki says: locked
  objects could still be marked and moved through Select All, group selection, Mark Wing, and
  next/previous object.
- Master: only clicks and box selection check `Locked_from_editing`
  (`EditorViewport.cpp:312, 829, 853`). `Editor::markObject` (`Editor.cpp:453`) does not check
  it.
- Fix:
  - Reject locked objects in `markObject`.
  - Then check Select All, group selection, `mark_wing`, and next/previous object.
  - The wiki also moves the menu item to "Select". That is a UI choice, not a bug.
- Status: **done** in `2c305c0acd`. The first version put the check in `markObject()`. The code
  review found that callers expect the mark to succeed (a context menu then edited object -1).
  Now only the selection commands skip locked objects: next/previous object, Select All and Invert
  Selection in the Scene Browser, group selection, and the Select Wing commands. A selection by name
  still marks a locked object, as in FRED2. QtFRED has no Select All menu command.

### 2.3 Object labels offset from their objects — C, Q

- Wiki: #19, "QtFRED object labels offset from their objects" (OFP PR #58, OFP issue #55). The
  wiki says: name labels used a different FOV than the scene render. FRED2 and QtFRED must
  share one FOV define for the editor.
- Master: `FredRenderer.cpp:996` and `:1129` pass the literal `0.5f` to `g3_set_view_matrix`.
  The projection matrix uses `FRED_DEFAULT_HTL_FOV` (0.485, `:46`). This mismatch also affects
  mouse picking.
- Fix: put one editor FOV constant in `code/missioneditor`. Use it for the projection matrix
  and for both view matrices.
- Status: **done** in `2b5990e75e` (`EDITOR_DEFAULT_FOV`). Zoom Extents and the briefing map also
  used 0.5. Commit `986b7f35f5` then makes the projection matrix use `Proj_fov`, which
  `g3_set_view_matrix()` sets from the zoom.

### 2.4 Transform toolbar — 10: C, Q; 11 and 31: U, Q

- Wiki:
  - #10, "QtFRED transform toolbar changed all three axes" (OFP PR #73). Editing one box wrote
    all three boxes to all marked objects. Only the edited axis must change, and the boxes must
    show two decimals.
  - #11, "QtFRED toolbar boxes stayed highlighted" (OFP PR #73). Selected text stayed
    highlighted after the viewport got focus.
  - #31, "QtFRED transform bar X/Y/Z fields unusable" (OFP PR #31). The boxes lost focus
    immediately. Enter must apply the value and give focus back to the viewport.
- Master:
  - `onTransformEditingFinished` (`FredView.cpp:1552-1625`) writes all three boxes in all
    paths.
  - The boxes have one decimal (`:1345`). Thus the single-object path also writes rounded
    values back to the axes that the user did not change.
  - `ensureViewportFocus` (`FredView.cpp:1665`) runs at idle and probably takes focus from the
    toolbar boxes.
- Fix:
  - Find which box sent `editingFinished` and change only that axis. Set two decimals.
  - In `ensureViewportFocus`, accept toolbar children the same way as dock children.
  - Clear the selection on focus-out.
  - Make Enter apply the value and give focus back to the viewport.
- Status: **done** in `967dc96e4a` for #10 and #31. The code review found that an apply on each
  focus loss is not safe (mode changes, popups, a click that selects another object, clamped
  values). Now only Enter applies a value. Escape and all other focus changes discard it. #11 is not
  reproduced: QLineEdit removes the selection on each focus change, except a window or popup change.

### 2.5 Unreadable text on colored annotation rows — C, S/Q

- Wiki: #6, "QtFRED unreadable text on colored annotation rows" (OFP PR #80). Text on
  annotation-colored SEXP tree rows could be unreadable against the background.
- Master: `sexp_tree_view::applyVisuals` (`sexp_tree_view.cpp:2045`) sets only the background.
- Fix: set the foreground to black or white, from the luminance of the background.
- Status: **done** in `4ae8c24172`. The FRED2 contrast rule moves to the shared
  `annotation_text_is_dark()`. The Events editor also showed note-only annotations as white rows.

---

## Phase 3: dialogs

### 3.1 "Attack ship type" initial order — C, Q

- Wiki: #2, "QtFRED Attack ship type initial order" (OFP PR #92). Opening Initial Orders on an
  ai-chase-ship-type order gave "Unhandled AI_GOAL_X #define 27". QtFRED must list, load, save,
  and check the order.
- Master: `AI_GOAL_CHASE_SHIP_TYPE` is missing in:
  - the goal list (`Editor.cpp:84`)
  - the `ShipGoalsDialogModel.cpp` switches (`:173`, `:515`)
  - `ShipGoalsDialog.cpp:212`
  - `ErrorChecker.cpp:1306`
- Fix: add the order next to `AI_GOAL_CHASE_SHIP_CLASS`, with a list of ship types as the
  target. Use FRED2 as the reference (`shipgoalsdlg.cpp:512, 735, 1045`).

### 3.2 Unsaved goal and event names — C, S

- Wiki:
  - #4, "QtFRED Mission Goals editor reset goal names" (OFP PR #84). Unsaved goals were not in
    the goal-name dropdowns. Editing another argument reset a goal name, for example in
    is-goal-true-delay.
  - #24, "QtFRED event name reset to the first event" (OFP PR #48, OFP issue #47). An event used
    as an argument, for example in is-event-true-delay, was reset to the first saved event. The
    unsaved events were not in the list.
- Master: `MissionGoalsDialog` and `MissionEventsDialog` do not override `getMissionGoals` and
  `getMissionEvents`. The default functions (`sexp_tree_model.cpp:161, 182`) read the saved
  globals. FRED2 uses the same shared code, so it probably has the same defect.
- Fix:
  - Override both functions to return the working copies of the dialog.
  - Then find why the tree resets a node to the first item when its name is not in the list.

### 3.3 Ship arrival distance limited to two digits — C, Q

- Wiki: #14, "QtFRED ship arrival distance limited to two digits" (OFP PRs #63, #70). The Ship
  Editor accepted only 0 to 99. It must accept all distances from the minimum for the target.
- Master: `arrivalDistanceEdit` (`ShipEditorDialog.ui:613`) has no `maximum`. The Qt default is
  99.
- Fix: set a large maximum. Enforce the minimum for the target in the model.
- Status: **partly done.** Commit `e3d8269edc` enforces the minimum in the model, with the shared
  `arrival_min_distance()`. The maximum of 99 is still open.

### 3.4 Duplicate wing names — C, Q

- Wiki: #17, "QtFRED duplicate wing names" (OFP PR #60). Form Wing did not check names, so
  duplicate and empty names were accepted.
- Master: `FormWingDialogModel::apply()` has the checks, but nothing calls it. `FormWingDialog`
  has no `accept()` override. `Editor::create_wing` (`EditorWing.cpp:173`) uses the name
  without a check.
- Fix: add `accept()`. It calls `apply()` and closes the dialog only if `apply()` succeeds.

### 3.5 Additional Ship Properties fields move — C, Q

- Wiki: #18, "QtFRED Additional Ship Properties fields jumping around" (OFP PR #59, OFP issue
  #57). The destroy time, escort priority, and kamikaze damage fields appeared and disappeared.
  They must stay in position and be disabled until their flag is set.
- Master: `ShipFlagsDialog.cpp:111-123` uses `setVisible`.
- Fix: use `setEnabled` and keep the widgets in the layout.

### 3.6 Message name warnings while typing — C, Q

- Wiki: #20, "QtFRED message name warnings while typing" (OFP PR #54, OFP issue #50). The Events
  editor gave a warning for each keystroke ("Helprin" gave a warning at "Help"). A name conflict
  must show a red outline while typing. The warning must come only when focus leaves the field.
- Master: `on_messageName_textChanged` calls `setMessageName`, which shows the conflict dialog
  (`MissionEventsDialogModel.cpp:1168`). When there is a conflict, the model also ignores the
  text.
- Fix: while typing, show only a red outline. Check the name and show the warning on
  `editingFinished`.

### 3.7 Event repeat count of -1 lost — C, Q

- Wiki: #28, "QtFRED event repeat count of -1 lost" (OFP PR #41). A repeat count of -1 (repeat
  forever) changed to 1 when the Events editor opened.
- Master: `MissionEventsDialogModel.cpp:132` changes `<= 0` to 1. FRED2 changes only `== 0`
  (`eventeditor.cpp:450`).
- Fix: change only `== 0`.

### 3.8 Typed value lost when clicking away — C, S/Q

- Wiki: #23, "QtFRED typed value lost when clicking away" (OFP PR #48, OFP issue #46). A number
  typed into the SEXP operator search was lost when the user clicked somewhere else.
- Master: `sexp_tree_view::eventFilter` (`sexp_tree_view.cpp:~805`) treats each external close as
  a cancel.
- Fix: on deactivation, commit valid typed text, for example a number. Cancel only on Escape.

### 3.9 Initial Status dockee point list empty — C, Q

- Wiki: #33, "QtFRED Initial Status dockee point list empty" (OFP PR #29). After the user
  selected a dockpoint, the dockee-point dropdown was empty or showed the points of the wrong
  ship.
- Master: `on_dockeeComboBox_currentIndexChanged` (`ShipInitialStatusDialog.cpp:79`) reads
  `dockeePointComboBox` before it fills the list again.
- Fix: call `updateDockee()` first. When the dockpoint changes, set `_curDockee` again from the
  model.

### 3.10 Unreadable hint text in dark mode — C, Q

- Wiki: #29, "QtFRED unreadable hint text in dark mode" (OFP PR #42). Placeholder hints were
  almost invisible in dark mode. The squadron logo field in Mission Specs showed its file name
  in black.
- Master: `Theme.cpp` does not set `QPalette::PlaceholderText`. `MissionSpecDialog.ui:359` sets a
  fixed black `Text` color on `squadronLogo`.
- Fix: set `PlaceholderText` for each theme. Remove the fixed color.

### 3.11 Enter and Space on dialog buttons — U, Q

- Wiki:
  - #35, "QtFRED Enter key in dialogs" (OFP PR #26). Enter clicked the default button. It could
    apply or close an editor, or click an unrelated button (Prev, Add, Reset to Default). Enter
    must do nothing in editor dialogs unless a button has focus.
  - #22, "QtFRED Space after New/Insert Event added another event" (OFP PR #48, OFP issue #45).
    Focus stayed on New Event or Insert Event, so Space added an event.
- Master: no dialog code turns off `autoDefault`. Buttons in a `QDialog` are auto-default in Qt,
  so the defect is probable.
- Fix:
  - Add a helper that turns off `autoDefault` and `default` on all buttons of each editor
    dialog.
  - For #22, give focus back to the event tree after New Event or Insert Event.

### 3.12 Weapons dialog Secondary tab — U, Q

- Wiki: #36, "QtFRED Weapons dialog Secondary tab" (OFP PR #24). On a ship with no primary
  banks, the Secondary tab was grey and could not be edited.
- Master: `ShipWeaponsDialog.cpp:181` enables a tab only if it has banks. Reproduce the defect
  before you change the code.

### 3.13 Defects to reproduce first — U, Q

- Wiki #13, "QtFRED list and dropdown type-ahead stopped cycling" (OFP PR #65). Fast repeated
  letters searched for the repeated letters (for example "ff") instead of the next match. Find
  which widget type has the defect, because the default Qt search behavior is different for each
  widget type.
- Wiki #34, "QtFRED sexp tree Space-to-edit going dead" (OFP PR #27). After some edits, Space,
  double-click edit, Ctrl+S, and the clipboard shortcuts stopped working in all SEXP trees. Check
  whether `_opPopupActive` or the inline-edit state stays set.
- Wiki #25, "QtFRED campaign graph drawing leftovers" (OFP PR #44). Moving cards or arrows in the
  Campaign Editor left pieces on the screen until a full redraw. Check the bounding rectangles of
  the graph items, or use `FullViewportUpdate`.

### 3.14 Warning dialogs minimize QtFRED — P, Q

- Wiki: #7, "QtFRED minimized by warning dialogs" (OFP PR #78). Warning and error dialogs
  minimized the QtFRED window. The dialog must show on top of the editor.
- Master: `os::dialogs::getDialogParent()` (`code/osapi/dialogs.cpp:88`) returns
  `getSDLMainWindow()`. That value is null, because `QtViewport::toSDLWindow()` returns
  `nullptr`. Thus the SDL message boxes have no parent.
- Fix: when `Fred_running`, send the dialogs to a QtFRED handler. The handler shows a
  `QMessageBox` with the main window as the parent.

---

## Phase 4: engine and shared logic (separate PR)

### 4.1 Scanning rule — C, S and G

- Wiki: #12, "Editors ignored Unified Scanning Behavior for subsystem scanning" (OFP PR #67). With
  `$Unify scanning behavior` on, a capital ship with `toggle-subsystem-scanning` was not in the
  list for `cap-subsys-cargo-known-delay`. The editors must use the same rule as the game.
- Master:
  - `sexp_tree_opf.cpp:166` uses only the old XOR rule.
  - `ShipInitialStatusDialogModel::changeSubsys()` uses `~` on a bool. Thus the value is
    always true when the flag is set. It also ignores `Use_new_scanning_behavior`.
  - The game rule is in `playercontrol.cpp:1696`.
- Fix: add one helper next to the game rule, for example `ship_scans_subsystems(const ship*)`.
  Use it in the game, in the SEXP list, and in Initial Status.

### 4.2 Briefing icons disappear after an instant stage change — C, G

- Wiki: #37, "QtFRED briefing icons vanish after an instant stage change" (OFP PR #21). Icons
  disappeared after a cut, a jump of more than one stage, or a camera stage with zero time. A
  zero move time also gave NaN positions.
- Master:
  - `brief_set_move_list` (`missionbriefcommon.cpp:1934-1936`) divides by `time`.
  - `brief_set_camera_target` (`:1616`) divides by `time_in_seconds` after its `time == 0`
    branch.
  - Thus a stage with zero time can give NaN values in the game too.
- Fix:
  - If the time is 0, put the icons and the camera at their final positions and return.
  - Then check cuts and jumps of more than one stage in
    `BriefingMapWidget::applyStageTransition` (`BriefingMapWidget.cpp:226`).
- Test: in the game, with a briefing that has a stage with zero time.

### 4.3 Color tag warnings in the briefing preview — C, G

- Wiki: #1, "Color tag warnings in the editors' briefing preview" (OFP PR #163). The QtFRED
  briefing preview showed an "Invalid text color tag" popup for each `$` sequence that is not a
  retail tag, for example `<$slashspan>`. In the editors, the message must go to the log. The
  game must give the warning as before.
- Master: `brief_verify_color_tag` (`missionbriefcommon.cpp:1683, 1689`) always calls `Warning`.
- Fix: when `Fred_running`, write the message to the log with `mprintf`. The editor error checker
  must still report the bad tag.

### 4.4 Help pages missing from the built help — C, Q

- Wiki: #15, "QtFRED help pages missing from the built help" (OFP PR #62). The Custom Data
  concept page had a link, but it was not registered. An old duplicate help project file was
  removed.
- Master: `concepts/custom-data.html` is not in `qtfred/help-src/doc/qtfred.qhp`.
  `qtfred/help-src/qtfred.qhp` is an old copy. CMake uses only `doc/qtfred.qhp`
  (`qtfred/CMakeLists.txt:143`).
- Fix: register the page and delete the old copy.

---

## Order of work

1. Phase 1. **Done**, with the one-line fixes 1.5 and 1.6.
2. Phase 2. **Done**, with the one-line fix 2.1.
3. Phase 3, with the one-line fixes 3.5 and 3.7 first.
4. Phase 4, as a separate PR, because it changes game code. Note: the Phase 1 branch already
   changes engine code in `code/ai/ailua.cpp` (Lua goal arguments) and `code/mission/missionparse.cpp`
   (`arrival_location_uses_distance()`).

Do the U items only after you reproduce them. Then decide if each one is a real defect or only a
difference between FRED2 and QtFRED.

## Coverage of the wiki entries

"(done)" marks an entry that the Phase 1 or the Phase 2 branch fixes.

| Wiki # | Plan item | Wiki # | Plan item | Wiki # | Plan item |
|---|---|---|---|---|---|
| 1 | 4.3 | 14 | 3.3 | 27 | 1.4 (done) |
| 2 | 3.1 | 15 | 4.4 | 28 | 3.7 |
| 3 | 1.1 (done) | 16 | 1.6 (done) | 29 | 3.10 |
| 4 | 3.2 | 17 | 3.4 | 30 | 1.3 (done) |
| 5 | 1.5 (done) | 18 | 3.5 | 31 | 2.4 (done) |
| 6 | 2.5 (done) | 19 | 2.3 (done) | 32 | 2.1 (done) |
| 7 | 3.14 | 20 | 3.6 | 33 | 3.9 |
| 8 | 1.4 (done) | 21 | 1.2 (done) | 34 | 3.13 |
| 9 | 2.2 (done) | 22 | 3.11 | 35 | 3.11 |
| 10 | 2.4 (done) | 23 | 3.8 | 36 | 3.12 |
| 11 | 2.4 (not reproduced) | 24 | 3.2 | 37 | 4.2 |
| 12 | 4.1 | 25 | 3.13 | | |
| 13 | 3.13 | 26 | 1.4 (done) | | |
