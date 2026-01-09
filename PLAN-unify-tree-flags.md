# Plan: Unify m_mode/ST_* into TreeFlags

## Problem

Two parallel configuration systems control sexp_tree behavior:

1. **FRED2's `m_mode` (legacy)** — an integer combining a mode ID (1–4) with `ST_*`
   capability bits. Used for dialog dispatch (`m_mode == MODE_GOALS` → call
   `Goal_editor_dlg->handler()`), capability checks (`m_mode & ST_ROOT_EDITABLE`),
   and annotation gating (`mode == MODE_EVENTS`).

2. **QtFRED's `TreeFlags` (newer)** — a flagset on `SexpTreeEditorInterface` with
   `LabeledRoot`, `RootDeletable`, `RootEditable`, `AnnotationsAllowed`. Clean,
   declarative, but incomplete — `AnnotationsAllowed` is never read, and `m_mode`
   is never set in QtFRED (defaults to 0).

### Consequences

- **Annotations broken in QtFRED context menu** — `m_mode` is always 0, so
  `mode == MODE_EVENTS` never fires, meaning "Edit Comment"/"Edit Color" are
  always disabled.
- **Root return type wrong in QtFRED Events editor** — should be `OPR_NULL` but
  `m_mode` is 0 so it defaults to `OPR_BOOL`.
- **Ship/Wing dialogs broken** — never set TreeFlags, call `initializeEditor()`
  repeatedly in `updateUI()`, missing signal connections.
- **Brief/Debrief dialogs minimal** — empty TreeFlags, late initialization.
- **`getRootReturnType()` virtual exists but is never called.**

## Goal

Make `TreeFlags` + `SexpTreeEditorInterface` the **single source of truth** for
tree behavior in both FRED2 and QtFRED. Remove `m_mode`, `ST_*` constants, and
`MODE_*` defines entirely.

## Detailed Steps

### Step 1: Replace mode checks in compute_context_menu_state()

In `code/missioneditor/sexp_tree_model.cpp`, `compute_context_menu_state(int mode)`:

**1a.** Replace annotation gating:
```cpp
// Before:
if (mode == MODE_EVENTS) {
    state.can_edit_comment = true;
    state.can_edit_bg_color = true;
}

// After:
if (_interface && _interface->getFlags()[TreeFlags::AnnotationsAllowed]) {
    state.can_edit_comment = true;
    state.can_edit_bg_color = true;
}
```

**1b.** Replace root return type logic. Use `getRootReturnType()` (already
exists, default returns `OPR_BOOL`):
```cpp
// Before:
if (mode == MODE_EVENTS) {
    state.replace_type = OPR_NULL;
} else {
    state.replace_type = OPR_BOOL;
}

// After:
state.replace_type = _interface ? _interface->getRootReturnType() : OPR_BOOL;
```

Same for `insert_opf_type` — derive it from the return type:
```cpp
// Before:
if (mode == MODE_EVENTS)
    state.insert_opf_type = OPF_NULL;
else
    state.insert_opf_type = OPF_BOOL;

// After:
state.insert_opf_type = (state.replace_type == OPR_NULL) ? OPF_NULL : OPF_BOOL;
```

**1c.** Replace campaign mode check:
```cpp
// Before:
state.campaign_mode = (mode == MODE_CAMPAIGN);

// After:
state.campaign_mode = _interface && _interface->requireCampaignOperators();
```

**1d.** Change the function signature from `compute_context_menu_state(int mode)`
to `compute_context_menu_state()` — it no longer needs a mode parameter since
all information comes from `_interface`.

### Step 2: Override getRootReturnType() in Events dialogs

**MissionEventsDialog** (QtFRED) already inherits `SexpTreeEditorInterface`.
Add the override:
```cpp
int getRootReturnType() const override { return OPR_NULL; }
```

**event_editor** (FRED2) — if it uses `SexpTreeEditorInterface`, same override.
If not, this will be handled in Step 4 when FRED2 is migrated.

### Step 3: Replace m_mode checks in FRED2's sexp_tree_ui.cpp

FRED2's `sexp_tree_ui.cpp` uses `m_mode` for two distinct purposes:

**3a. Capability checks (replace with TreeFlags):**

```cpp
// edit_label() — replace:
if (m_mode & ST_ROOT_EDITABLE) → if (_interface && _interface->getFlags()[TreeFlags::RootEditable])

// NodeDelete() — replace:
if (m_mode & ST_ROOT_DELETABLE) → if (_interface && _interface->getFlags()[TreeFlags::RootDeletable])

// OnBegindrag() — replace:
if (!m_mode || ...) → if (!_interface || !_interface->getFlags()[TreeFlags::LabeledRoot] || ...)
```

**3b. Dialog dispatch (replace with signals):**

FRED2 uses `m_mode == MODE_GOALS` etc. to call the right global dialog pointer
(`Goal_editor_dlg->handler()`). This is the MFC equivalent of QtFRED's
signal/slot pattern.

There are 4 dispatch sites in sexp_tree_ui.cpp:
1. `end_label_edit()` → ROOT_RENAMED dispatch
2. `add_operator()` → `insert_handler()` dispatch
3. `NodeDelete()` → ROOT_DELETED dispatch
4. `OnLButtonUp()` → `move_handler()` dispatch

**Option A (minimal, recommended):** Replace the if/else chains with virtual
method calls through `SexpTreeEditorInterface`. Add 3 new virtuals:
```cpp
virtual void onRootRenamed(int node, const char* new_name) {}
virtual void onRootDeleted(int node) {}
virtual void onRootInserted(int old_formula, int new_formula) {}
virtual void onRootMoved(int node1, int node2, bool before) {}
```
Each FRED2 dialog overrides these. The tree calls
`_interface->onRootDeleted(node)` instead of dispatching on mode.

**Option B (larger):** Add full Qt-style signal support to FRED2's sexp_tree.
Not worth it — FRED2 is legacy.

### Step 4: Remove m_mode, ST_*, and MODE_* entirely

Once all reads of `m_mode` are replaced:

1. Remove from `SexpTreeModel`: `int m_mode;`
2. Remove from `sexp_tree_model.h`:
   ```
   #define ST_LABELED_ROOT   0x10000
   #define ST_ROOT_DELETABLE 0x20000
   #define ST_ROOT_EDITABLE  0x40000
   #define MODE_GOALS     ...
   #define MODE_EVENTS    ...
   #define MODE_CAMPAIGN  ...
   #define MODE_CUTSCENES ...
   ```
3. Remove `m_mode` references from FRED2's `sexp_tree_ui.h` (`int& m_mode = _model.m_mode;`)
4. Remove the `mode` parameter from `compute_context_menu_state()`
5. Remove `m_mode = MODE_*` assignments from all FRED2 dialog init code
6. Remove `right_clicked(int mode)` mode parameter in FRED2 (callers currently
   pass `m_mode` which is already on the model)

### Step 5: Fix broken QtFRED dialogs

**5a. ShipEditorDialog:**
- Move `initializeEditor()` call from `updateUI()` to constructor
- Add proper TreeFlags (empty flagset is correct — no labeled root)
- Verify signal connections for `nodeChanged`/`rootNodeFormulaChanged`

**5b. WingEditorDialog:**
- Same as Ship: move `initializeEditor()` to constructor
- Fix the TODO "This seems broken in a weird way" — investigate `nodeChanged`
  handler
- Add proper signal connections

**5c. BriefingEditorDialog:**
- Move `initializeEditor()` to constructor (currently in `updateUi()`)
- Empty TreeFlags is correct for this dialog

**5d. DebriefingDialog:**
- Same as Briefing: move `initializeEditor()` to constructor

For all four: verify that `load_tree()` can be called multiple times after
a single `initializeEditor()` — it should just reload data without
re-initializing the editor/interface.

### Step 6: Audit FRED2 dialogs for consistency

Ensure each FRED2 dialog properly sets TreeFlags on its `SexpTreeEditorInterface`:

| Dialog | TreeFlags |
|--------|-----------|
| event_editor | LabeledRoot, RootDeletable, RootEditable, AnnotationsAllowed |
| CMissionGoalsDlg | LabeledRoot, RootDeletable |
| CMissionCutscenesDlg | LabeledRoot, RootDeletable |
| campaign_editor | LabeledRoot, RootDeletable |
| Ship editor | (none) |
| Wing editor | (none) |
| Briefing | (none) |
| Debriefing | (none) |

Currently FRED2 dialogs don't use `SexpTreeEditorInterface` at all (they set
`m_mode` instead). After Step 3b adds virtual callbacks to the interface, FRED2
dialogs will need to:
- Inherit `SexpTreeEditorInterface`
- Pass flags in their constructor
- Override the callback virtuals
- Pass `this` to `sexp_tree` initialization

## Ordering & Dependencies

```
Step 1 (compute_context_menu_state) — no deps, can be done first
    ↓
Step 2 (getRootReturnType override) — needed by Step 1b
    ↓
Step 3a (FRED2 capability checks) — needs Step 1 done
Step 3b (FRED2 dialog dispatch) — independent of 3a
    ↓
Step 4 (remove m_mode) — needs Steps 1, 3a, 3b all done
    ↓
Step 5 (fix broken QtFRED dialogs) — independent, can be done any time
Step 6 (FRED2 audit) — needs Step 3b done
```

Steps 1+2 and Step 5 can be done in parallel.

## Risk Assessment

- **Low risk:** Steps 1, 2, 5 — straightforward replacements and fixes
- **Medium risk:** Steps 3a, 4 — touching FRED2 tree core, can't easily test
- **Higher risk:** Step 3b — replacing FRED2's ad-hoc dispatch with virtual
  callbacks changes the control flow for 4 dialog types. Each dispatch site
  has slightly different behavior that needs careful migration.

## Files Modified

| File | Steps |
|------|-------|
| `code/missioneditor/sexp_tree_model.h` | 1d, 3b, 4 |
| `code/missioneditor/sexp_tree_model.cpp` | 1a-d, 4 |
| `fred2/sexp_tree_ui.h` | 3a, 4 |
| `fred2/sexp_tree_ui.cpp` | 3a, 3b, 4 |
| `fred2/eventeditor.cpp` | 3b, 4, 6 |
| `fred2/eventeditor.h` | 3b, 6 |
| `fred2/missiongoalsdlg.cpp` | 3b, 4, 6 |
| `fred2/missiongoalsdlg.h` | 3b, 6 |
| `fred2/missioncutscenesdlg.cpp` | 3b, 4, 6 |
| `fred2/missioncutscenesdlg.h` | 3b, 6 |
| `fred2/campaigneditordlg.cpp` | 3b, 4, 6 |
| `fred2/campaigneditordlg.h` | 3b, 6 |
| `qtfred/.../MissionEventsDialog.cpp` | 2 |
| `qtfred/.../MissionEventsDialog.h` | 2 |
| `qtfred/.../ShipEditorDialog.cpp` | 5a |
| `qtfred/.../WingEditorDialog.cpp` | 5b |
| `qtfred/.../BriefingEditorDialog.cpp` | 5c |
| `qtfred/.../DebriefingDialog.cpp` | 5d |
| `qtfred/.../sexp_tree_ui.cpp` | 1d (call site update) |
