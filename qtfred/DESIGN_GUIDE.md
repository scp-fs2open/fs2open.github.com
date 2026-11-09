# QtFRED Design Guide

This guide captures the design preferences we want QtFRED contributors to follow as the editor evolves. It is intentionally practical: use it when adding or changing dialogs, moving logic out of legacy FRED code, reviewing UI patches, or deciding how a new editor workflow should behave.

QtFRED is still being ported and refined, so some existing code may not match every preference below. Treat this document as the direction of travel for new code and for cleanup work when touching older code.

## Core principles

1. **Keep mission logic out of Qt widgets.** UI classes gather input, display state, and translate Qt events into model calls. Mission validation, mutation, and persistence decisions belong in mission/editor classes and dialog models.
2. **Make dialog commit semantics obvious.** Every editor dialog should be one of three interaction types: Modal edit, Direct Edit, or Bulk action. Do not mix these styles in one dialog.
3. **Update user help with the feature.** If a dialog, workflow, or user-visible control changes, update the corresponding built-in QtFRED help page in the same change.
4. **Prefer Qt Designer plus autoconnect for static UI.** Keep `.ui` files as the source of truth for dialog layout, object names, tab order, and basic widget properties. Prefer `on_<objectName>_<signalName>()` slots over a constructor full of `connect()` calls for Designer-created controls.
5. **Preserve mission safety.** Mark edits modified only when data really changes, validate before committing, and emit mission change signals after the backing mission state is updated.

## Architecture boundaries

QtFRED separates the Qt front end from mission editing logic:

- `qtfred/src/ui` contains Qt-specific widgets, dialogs, panels, and view code.
- `qtfred/src/mission` contains mission-management and editor logic. This layer should stay UI-framework agnostic except where Qt's object/signaling infrastructure is already part of the model pattern.
- `qtfred/src/mission/dialogs` contains dialog models. A mission-editing `QDialog` should normally have a matching model class derived from `AbstractDialogModel`.
- `qtfred/ui` contains Qt Designer `.ui` form files.
- `qtfred/help-src/doc` contains the built-in user help.

### The `Editor` and `EditorViewport` classes

The original FRED freely mixed MFC (the UI framework it used) and mission editor logic, and relied on global dialog and window instances for handling callbacks. QtFRED avoids this by extracting the editor logic into two UI-framework-agnostic classes (they use Qt only for signals and slots):

- `Editor` is an instance of a mission editor and keeps all state related to that mission. It plays the role the original FRED's `management.cpp` did with its many global state variables. The field and function names are mostly kept the same as the original FRED, so porting FRED code into `Editor` should be straightforward.
- `EditorViewport` is a single view into an `Editor` instance. It encapsulates the state of a single main window and exists so QtFRED can support multiple viewports in the future. Dialogs hold a reference to an `EditorViewport` to reach their parent editor.

Use Qt's signal/slot mechanism for event notification rather than the original FRED's global-instance callback pattern.

### UI classes should do

- Own `Ui::<DialogName>` and Qt-only widgets.
- Call `setupUi()`, initialize control state, validators, and presentation-only defaults.
- Forward user actions to the model through small slots.
- React to `modelChanged()` by refreshing controls.
- Use `util::SignalBlockers` when programmatic UI refreshes would otherwise re-trigger editing slots.
- Show user-facing prompts, file pickers, and messages through the existing dialog-provider/UI patterns.

### UI classes should not do

- Directly mutate `The_mission`, global mission arrays, ship/wing/object data, or parser structures except for narrow bridge code that has not yet been modeled.
- Contain mission validation rules that belong to the editor domain.
- Decide whether mission data is dirty by comparing UI widgets.
- Use Qt widget state as the source of truth after the model has been initialized.

### Dialog models should do

- Keep a working copy of editable mission data for Modal edit dialogs and for Bulk action dialogs that collect parameters before execution.
- Provide typed getters and setters for the UI.
- Use `AbstractDialogModel::modify()` or `set_modified()` for changes so dirty state remains reliable.
- Implement `apply()` or an explicitly named action method, such as `generateFilenames()`, as the only normal path that writes staged data back to mission state.
- Implement `reject()` to discard staged changes or undo temporary model state.
- Emit `modelChanged()` after model data changes and the view needs to refresh.
- Call `Editor::missionChanged()` only after the real mission data has been successfully changed.

### Dialog models should not do

- Depend on Qt widgets or `.ui` objects.
- Present dialogs or message boxes directly. If user feedback is required, the model should report back to the UI with a Boolean, Enum, or other method and they the calling UI handle the issue from there.
- Hide UI-specific concepts such as pixel sizes, control visibility, or tab ordering unless those concepts are true editor preferences.

## Dialog behavior patterns

Pick one of these patterns before implementing or changing a dialog. Document unusual choices in code comments and keep the help page consistent with the chosen pattern.

### Modal edit dialogs

Use this for edits where users expect to review changes and either keep or discard them as a batch.

Characteristics:

- The dialog has explicit **OK/Cancel** or **Apply/Cancel** semantics.
- UI slots update only the dialog model's working copy.
- `accept()` calls `_model->apply()` and closes only when it succeeds.
- `reject()` uses `rejectOrCloseHandler()` or equivalent behavior when there are pending changes.
- The window close button follows the same path as Cancel; override `closeEvent()` when needed so the X button cannot bypass dirty-change handling.
- On successful apply, ensure `missionChanged()` is emitted by the model or editor logic after the real data changes.

Do not make individual controls in a Modal edit dialog immediately write to mission state. If the preview must update while the dialog is open, keep the preview path explicit and reversible.

### Direct Edit dialogs and panels

Use this for preferences, tool panels, and workflows where each control is expected to take effect immediately.

Characteristics:

- Controls commit immediately through the model/editor.
- The dialog does not offer a misleading Cancel button for already-applied changes.
- The model applies changes as part of the setter path or in response to `modelChanged()`.
- Each change should be undo/mission-safe at the granularity users expect.
- Help text should make clear that changes take effect immediately.

### Bulk action dialogs

Use this for dialogs that gather parameters for a larger operation, then apply that operation to mission data when the user presses an explicit action button such as **Apply**, **Generate**, **Copy**, or **Sync**. These are similar to Modal edit dialogs because the controls usually edit a temporary setup state, but the operation itself directly modifies the mission when triggered and the dialog often remains open for additional bulk operations.

Current examples include the Shield System dialog, Voice Acting Manager, and Waypoint Path Generator.

Characteristics:

- Parameter controls edit model state and should not partially mutate mission data before the action runs.
- The action button name should describe the committed operation, not imply ordinary dialog acceptance when the dialog remains open.
- The action path validates its parameters, modifies mission data in one deliberate step, and emits `missionChanged()` after the data changes.
- The dialog should clearly communicate whether the action is repeatable and whether the dialog will remain open afterward.
- Cancel/Close only dismisses the dialog or discards unexecuted parameter edits; it must not pretend to undo bulk operations that have already run.

### No hybrids

Avoid dialogs where some controls are staged until OK, other controls silently commit immediately, and bulk action buttons also mutate mission data without a clear boundary. Hybrids confuse users, make Cancel unreliable, and complicate undo. If an existing dialog has mixed behavior, choose one primary pattern the next time you significantly touch it and refactor toward that pattern.

## Signals, slots, and connection style

### Prefer autoconnect for Designer-created controls

For widgets declared in `.ui` files, prefer Qt's autoconnect naming convention:

```cpp
private slots:
	void on_okAndCancelButtons_accepted();
	void on_enabled_toggled(bool enabled);
	void on_lineEditAvgSpeed_textEdited(const QString& text);
```

This keeps the constructor focused on setup and makes signal ownership discoverable from the slot name. Match the `.ui` object name exactly and use the signal name that represents user intent.

Use `textEdited` instead of `textChanged` when only user edits should update the model. Use `currentIndexChanged`, `valueChanged`, or `toggled` when programmatic updates are protected by `util::SignalBlockers`.

### Use explicit `connect()` when it is the clearer tool

Explicit `connect()` calls are appropriate for:

- Dynamic widgets created at runtime.
- Lambdas that capture context.
- Cross-object model/view wiring such as `modelChanged()` to `updateUi()`.
- Main-window action wiring when an action is not easily or safely expressed by autoconnect.
- Overloaded signals where the selected overload should be explicit.

Avoid long constructor blocks that manually connect every static button, checkbox, and line edit from a `.ui` file. If a dialog needs many such connections, prefer renaming the widgets and adding autoconnect slots.

## UI refresh and dirty state

- Initialize the model before populating the UI.
- Use a clear split between `initializeUi()` for one-time setup and `updateUi()` for state refreshes.
- Wrap programmatic widget updates with `util::SignalBlockers` to avoid false dirty flags and recursive updates.
- Prefer model getters as the source for all widget values during refresh.
- Set validators, ranges, and enabled states in UI code; enforce mission validity in model/editor code.
- Do not call `modelChanged()` as a substitute for dirty tracking. Dirty tracking should happen when model data actually changes.

## Mission change expectations

- A model's `apply()` should return `false` when validation fails and the dialog must stay open.
- Emit `Editor::missionChanged()` after successful changes to mission state, not before.
- Keep temporary preview state reversible if the user can still cancel the dialog.

## Adding new files

- New source files need to be added to the appropriate folders in `qtfred/source_groups.cmake`. If a new source folder is created, add it there with an appropriate name.
- New UI form files go in the `qtfred/ui` folder. They are automatically included in the build that way, and they will be recompiled when the form is changed later, so no separate build wiring is required for a `.ui` file beyond placing it in that folder.

## Help documentation requirements

When a user-visible dialog changes, update the relevant help document in `qtfred/help-src/doc` in the same patch.

Update help when you:

- Add, remove, rename, move, or substantially restyle a control.
- Change a default value, validation rule, option meaning, or workflow order.
- Change whether a dialog is Modal edit, Direct Edit, or Bulk action.
- Add a new dialog, panel, or major workflow.
- Add or change menu paths, keyboard shortcuts, or toolbar actions that users rely on.

For a new built-in help page:

1. Add the HTML file under the appropriate `qtfred/help-src/doc` subdirectory.
2. Link it from `qtfred/help-src/doc/qtfred.qhp` so it appears in the Help contents tree.
3. Use `../css/help.css` and existing help pages as style examples.
4. Keep names consistent with the dialog title and menu path.

## Adding or changing dialogs: checklist

Before review, verify the following:

- [ ] The dialog is clearly Modal edit, Direct Edit, or Bulk action, not a hybrid.
- [ ] Mission logic lives in a model/editor class, not in widget code.
- [ ] Static `.ui` controls use autoconnect slots unless explicit `connect()` is justified.
- [ ] Programmatic UI refreshes use signal blockers where needed.
- [ ] `accept()`, `reject()`, and `closeEvent()` preserve dirty-change behavior.
- [ ] `apply()` or the explicit bulk action validates and reports failure without committing partial mission changes.
- [ ] Mission changes emit `missionChanged()` only after mission data is updated.
- [ ] The relevant help page and contents tree are updated.
- [ ] New source files are added to `qtfred/source_groups.cmake`, and new `.ui` form files are placed in `qtfred/ui`, when required.