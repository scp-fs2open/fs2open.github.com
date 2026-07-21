# QtFRED Undo Guide

QtFRED implements undo/redo with Qt's undo framework (`QUndoCommand`, `QUndoStack`, `QUndoGroup`). It replaces the original FRED backup-file system (which serialized the whole mission to rotating backup files and reloaded one on undo) with fine-grained, per-edit commands.

This guide explains the architecture and the patterns every mutation path must follow. Read it alongside `DESIGN_GUIDE.md`: the dialog interaction types defined there (Modal edit, Direct Edit, Bulk action) each map to a specific undo integration pattern described below. **Every control that mutates mission data or a dialog's working state must have an undo path.** A control without one is a bug: the next Ctrl+Z will undo an older edit while the unrecorded change persists, silently desynchronizing the undo history from the mission.

## Stack topology

| Stack | Owner | Scope |
|---|---|---|
| Main stack | `FredView` | Viewport edits, Direct Edit dialogs, Bulk actions, and one `ApplyDialogCommand` per accepted Modal edit dialog |
| Camera stack | `FredView` | Camera moves only (Ctrl+Shift+Z / Ctrl+Shift+Y); deliberately outside the group so view changes never consume a mission undo step |
| Per-dialog stacks | Each Modal edit dialog | Granular in-dialog edits; cleared when the dialog closes |

All stacks except the camera stack are registered in `FredView`'s `QUndoGroup`. The active stack follows window activation: activating a Modal edit dialog routes its Edit menu and shortcuts to its own stack, and activating the main window (or any Direct Edit / Bulk dialog) routes back to the main stack.

Supporting utilities in `qtfred/src/ui/util/DialogUndo.h`:

- `util::setupDialogUndo(dialog, group, stack, scopeName)` — gives a Modal edit dialog an Edit menu with Undo/Redo (Ctrl+Z / Ctrl+Y), registers its stack in the group, installs the activation watcher, and applies the undo depth limit.
- `util::installMainStackUndoShortcuts(window, mainStack)` — gives Direct Edit and Bulk dialogs window-scoped Ctrl+Z / Ctrl+Y bound to the main stack.

The undo depth is a preference (`Undo history depth`). Qt cannot change a non-empty stack's limit, so a new depth takes effect at the next mission load. The editor clears the main stack at the **top** of `loadMission()` / `createNewMission()`, before mission teardown — command destructors free captured sexp chains, which must happen while the pool they were allocated from is still alive. Preserve this ordering.

## Command toolbox

All command classes live in `qtfred/src/mission/commands/`.

### `FieldEditCommand<T>` — single-field edits

The workhorse for simple values. Each command holds one or more entries of `{before, after, setter}`; multi-select edits add one entry per changed object.

- **Merging.** Consecutive pushes with the same `fieldId`, the same target key, and the same entry count merge into one undo step (rapid spinbox clicks collapse). Call `setTargetKey()` with an identity for the edited object(s) — e.g. the concatenated marked-ship signatures or a wing index — so changing selection between edits starts a new step instead of cross-wiring the previous command's setters. Call `setNoMerge()` when each invocation is a discrete action (subdialog visits, renames whose setters capture names, buttons).
- **`skipFirstRedo`.** Pass `true` when the caller already applied the change before pushing; `QUndoStack::push()` always calls `redo()` immediately, and the first call must then be a no-op.
- **Editor pointer.** Pass the real `Editor*` for main-stack commands (the command emits `missionChanged()` on undo/redo). Pass `nullptr` for dialog-internal commands: the setters write the dialog's working copy and each setter is responsible for refreshing the dialog UI.

### `DialogSnapshotCommand` — working-state blobs

For in-dialog edits where per-field commands don't fit: sexp tree mutations, list add/delete/move/reorder, type changes, and any lossy or structural operation. The dialog serializes the relevant slice of its working state to a `QByteArray` and provides a restore callback that applies a blob and refreshes the UI.

- Pushing a snapshot whose before and after blobs are equal is absorbed as a no-op; this makes it safe to push defensively from signal handlers that sometimes fire redundantly.
- A non-negative `mergeId` merges consecutive pushes with the same id (first before, latest after), collapsing continuous gestures. Merge ids share the FieldId number space.

### `ApplyDialogCommand` — Modal edit accept

The accept flow of every Modal edit dialog: capture model state, `apply()`, capture again, push one `ApplyDialogCommand` to the main stack, clear the dialog's own stack. The command takes ownership of the dialog model (call `setParent(nullptr)` before moving it in) and restores via the model's `captureState()`/`restoreState()`, implemented in `qtfred/src/mission/dialogs/state/<Name>State.cpp`. Sexp formulas serialize for real via `DialogStateHelpers::writeSexp`/`readSexp`.

The shared `rejectOrCloseHandler()` routes "keep your changes? → Yes" through the dialog's `accept()` so a close-with-save is exactly as undoable as OK. Never call `model->apply()` outside the accept flow.

### Structural commands and `ObjectCapture`

`CreateObjectCommand`, `DeleteObjectsCommand`, `MoveObjectsCommand`, the wing commands, the clone commands, and friends capture full object state via `ObjectCapture.h` (`CapturedShip` and relatives, with sexp cue ownership transfer).

### Specialized commands

- `SexpCueEditCommand` — one arrival/departure cue tree edit in the ship/wing editors, pushed per `sexp_tree_view::modified`.
- `BatchFlagCommand` / `VoiceActingBatchCommand` — before/after snapshots around Bulk actions.
- `CameraTransformCommand` — camera stack only.

## Core invariants

These are the rules the whole system depends on. Violating any of them produces bugs that pass casual testing and corrupt missions later.

1. **Identity is signature or name, never a pool index.** Object pool slots (`objNum`) and `Ships[]` instances are reused; commands that outlive a delete/undelete cycle must locate their targets via `obj_get_by_signature()` (ships, jump nodes, props, a waypoint path's first point) or by name (wings). Restore paths reapply the original signature on undelete precisely so that earlier commands' lookups stay valid. Setter lambdas must re-resolve identity on every invocation — never cache a pointer or index in a lambda.
2. **Read-back capture.** Model setters clamp, truncate, and validate. Always capture `before`, call the setter, capture `after` from the model/mission again, and skip the push when they are equal. Never assume the value you sent is the value that was applied.
3. **Push after success for abortable operations.** Deletions can be refused (reference-check prompts, player-start rules). Construct the command first (captures happen while the data is alive), run the operation, and push only when it reports success; otherwise `delete` the command.
4. **Reference prompts are suppressed during replay.** `undo()` of a delete restores the sexp references that the original deletion invalidated, so replaying the delete would re-prompt — and canceling a modal inside `QUndoStack::undo()/redo()` desyncs the stack. Command `undo()`/`redo()` bodies that delete objects wrap the call in `AutoConfirmReferences` (see `FredCommands.cpp`), which makes `reference_handler()` proceed as if the user confirmed. The user already consented at the original action.
5. **Structural operations are snapshots, so index-keyed field ids are safe.** In-dialog field commands may embed an element index in their `fieldId` (e.g. `Event_Name + index * stride`) because every add/delete/move/reorder is a whole-blob snapshot: undo replays strictly in reverse, so element indices always match the state each command was recorded against. If you ever add a structural op as a non-snapshot command, this invariant breaks.
6. **Merge keys must change when the edited target changes.** Field ids alone are not identity. Use `setTargetKey()` (Direct Edit dialogs), index-strided ids (per-element dialog fields), or a selection generation counter bumped on every selection change (tables where the same cell id can refer to different rows).
7. **Working-state blobs and selection.** If a dialog uses the cached-blob pattern (capture once, compare on every `modified()` signal to absorb no-ops), the blob must **exclude** volatile selection state, or selection changes will produce spurious diffs. If every operation fresh-captures immediately before mutating, the blob may include selection (and restoring it is a UX nicety). Pick one convention per state scope and comment it.
8. **Programmatic UI refresh must not push commands.** Wrap every `updateUi()`-style refresh in `util::SignalBlockers`, use `QSignalBlocker` for point writes, and prefer `textEdited` over `textChanged` for line edits. After undo/redo, verify every affected widget visually refreshes (combos and radio groups are the usual stragglers).
9. **One user gesture, one undo step.** A drag is captured at release; a subdialog visit is one command; related writes triggered by one action (e.g. a rename plus its derived display-name reset) belong in one command or one `beginMacro`/`endMacro` group.
10. **Capture parity.** When you add a field to anything captured for undo, update capture **and** restore — and for `CapturedShip`, the hand-written move assignment in `ObjectCapture.cpp`, which is memberwise and will silently drop your field otherwise. `ObjectCapture.h`'s banner lists the missionsave/missionparse locations that must stay in sync.

## Wiring a new control

### In a Modal edit dialog (per-dialog stack)

For a simple field:

1. Allocate a `FieldId` constant in the dialog's range in `FredCommands.h` (per-element fields use `base + index * stride`; check the range comments for collisions).
2. In the autoconnect slot: read-back capture around the model setter, then push a merging `FieldEditCommand<T>` (editor = `nullptr`) whose setter lambda calls the model's index-explicit setter (`setThingAt(team, stage, v)`-style) and refreshes the UI. Several dialogs have a `pushValueCommand` helper that packages this — prefer it where it exists.

For a structural or lossy operation: capture the working-state blob before mutating, mutate, then push through the dialog's snapshot helper (`pushWorkingStateSnapshot` or equivalent), with a `mergeId` if the gesture is continuous.

Nothing else is needed: the accept flow already wraps the whole session in an `ApplyDialogCommand`, but only for state included in `captureState()`/`restoreState()` — if your control touches mission data outside that capture, extend the state file too.

### In a Direct Edit dialog (main stack)

Read-back capture per affected object (usually `forEachMarkedShip` or the dialog's selection list), push a `FieldEditCommand<T>` with the real `Editor*`, `skipFirstRedo = true`, a `FieldId` from the dialog's range, and `setTargetKey()` with the selection's signatures. Setter lambdas capture signatures and re-resolve on each call. Add entries only for objects whose value actually changed; delete the command instead of pushing when nothing changed.

### In a Bulk action dialog (main stack)

Snapshot the affected state before the action, run it, snapshot after, and push a batch command (reuse `BatchFlagCommand` for flag sweeps, or the snapshot pattern in `VoiceActingBatchCommand`). Skip the push when before equals after. The parameter controls themselves stage state and need no undo — only the action button mutates the mission.

### A new Modal edit dialog

1. Give it a `QUndoStack` and call `util::setupDialogUndo()` in the constructor.
2. Implement `captureState()`/`restoreState()` in a new `state/<Name>State.cpp` (add it to `qtfred/source_groups.cmake`), restoring **everything** `apply()` can change — including apply-time side effects such as reference renames, which must be inverted on undo.
3. Implement the accept flow (capture → `apply()` → capture → push `ApplyDialogCommand` → clear dialog stack → reactivate main stack) and route `reject()`/`closeEvent()` through `rejectOrCloseHandler()`.
4. Add working-state capture/restore for the in-dialog stack, then wire each control as above.
5. Guard the `FredView` action with `raiseExistingEditor<T>()` — two live instances of the same editor would interleave apply commands.

## Review checklist

- [ ] Every mutating control pushes a command or is covered by a snapshot; view-only controls are demonstrably view-only.
- [ ] Read-back capture around every model setter; no-op edits push nothing.
- [ ] Merge identity is correct: target key, index stride, or selection generation.
- [ ] Setter lambdas resolve targets by signature/name at call time.
- [ ] Abortable operations push after success; replayed deletes use `AutoConfirmReferences`.
- [ ] `updateUi()` paths are signal-blocked, and undo visually refreshes every affected widget.
- [ ] New captured fields appear in capture, restore, and (for `CapturedShip`) the move assignment.
- [ ] Undo/redo cycles round-trip: edit → undo → redo → undo leaves the mission byte-identical to the start.

## Known limitations

- Undoing an object deletion does not reverse the sexp-reference invalidation the deletion performed (ship references stay bashed to `<name>`; mid-path waypoint deletions renumber references and the renumbering is not reversed). Fully-deleted wings are the exception.
- Renaming or deleting events, goals, and messages rewrites references in foreign sexp trees (e.g. ship arrival cues) at apply time; those rewrites are outside the owning dialog's capture scope and survive an undo of the apply.
- Undoing the deletion of docked ships does not restore the dock pairing (`TODO(docking-undo)`).
- Undoing a wing formation cannot restore the members' pre-formation arrival/departure cues; `create_wing()` destroys them, matching original FRED behavior.
- Text widgets keep Qt's internal text-undo history; Ctrl+Z with focus in a line edit undoes typing first, then mission edits. The two histories stay consistent through the read-back pattern.
