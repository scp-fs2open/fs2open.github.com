# Sexp Tree Model/UI Separation Plan

## Problem

FRED2's `sexp_tree.cpp` (8,288 lines) and QtFRED's `sexp_tree.cpp` (8,027 lines) are near-identical implementations of the same logic, duplicated across two UI frameworks. Both mix pure model logic with UI-specific code. New features/fixes must be applied to both files.

## Goal

Extract all UI-independent model logic into shared files under `code/missioneditor/`, leaving FRED2 and QtFRED as thin UI wrappers that delegate to the shared model.

## Architecture: Three Shared Files

### 1. `code/missioneditor/sexp_tree_model.h/.cpp` — Core Tree Model

The main model class `SexpTreeModel` owns the tree node data and all pure-logic operations.

**Data structures (moved from both headers):**
- `sexp_tree_item` — node struct, but with `void* handle` instead of `HTREEITEM`/`QTreeWidgetItem*` (UI layers cast as needed)
- `sexp_list_item` — linked list for option listings (already UI-independent)
- Shared constants: `SEXPT_*` type/flag defines, `BITMAP_*`/`NodeImage` enum (unified into a single `NodeImage` enum)
- `TreeFlags` flagset (already exists in QtFRED, would be shared)

**SexpTreeModel class — pure model operations (no UI calls):**

*Tree node management:*
- `tree_nodes` vector, `total_nodes`, `root_item`, `item_index`
- `allocate_node()` (index-only overload), `find_free_node()`, `set_node()`, `free_node2()`
- `clear_nodes()` — resets node array only (UI layer calls this then clears its widget)

*Tree serialization (works with global Sexp_nodes array):*
- `save_tree()`, `save_branch()`
- `load_sub_tree()` — builds node data without inserting UI items

*Query/analysis (pure computation on tree_nodes):*
- `count_args()`, `identify_arg_type()`
- `find_argument_number()`, `find_ancestral_argument_number()`
- `query_node_argument_type()`, `query_restricted_opf_range()`
- `get_default_value()`, `query_default_argument_available()`
- `get_data_image()` — returns `NodeImage` enum
- `get_sibling_place()`
- `query_false()`
- `match_closest_operator()`
- `verify_and_fix_arguments()`
- `is_node_eligible_for_special_argument()`

*Variable/container utilities:*
- `delete_sexp_tree_variable()`, `modify_sexp_tree_variable()`
- `get_item_index_to_var_index()`, `get_tree_name_to_sexp_variable_index()`
- `get_modify_variable_type()`, `get_variable_count()`, `get_loadout_variable_count()`
- `get_container_usage_count()`, `rename_container_nodes()`
- `is_matching_container_node()`, `is_container_name_argument()`
- `is_container_name_opf_type()` (static)

*Help text:*
- `help()` (static) — returns help string for an operator code

*Tree search (model portion):*
- `find_text_in_nodes()` — searches `tree_nodes[]` text, returns node index (UI layer handles highlighting/scrolling)

**Context interface (`SexpTreeEditorInterface`):**
Moved here from QtFRED. Both FRED2 and QtFRED dialogs implement this interface. This provides the contextual data that OPF functions and menu logic need:
- `getMessages()`, `hasDefaultMessageParameter()`
- `getMissionGoals()`, `hasDefaultGoal()`
- `getMissionEvents()`, `hasDefaultEvent()`
- `getMissionNames()`, `hasDefaultMissionName()`
- `getRootReturnType()`
- `requireCampaignOperators()`
- `getFlags()` — returns `TreeFlags` flagset

Note: The Qt-specific `getContextMenuExtras()` would NOT move here — that stays in QtFRED's UI layer.

### 2. `code/missioneditor/sexp_tree_opf.h/.cpp` — OPF Listings

**All ~70+ `get_listing_opf_*()` functions.** These are the functions that get added to most frequently and are 100% model logic — they query game data arrays (Ships[], Wings[], Ai_info[], weapon tables, etc.) and build `sexp_list_item` linked lists.

**Functions:**
- `get_listing_opf()` — main dispatcher (giant switch on OPF type)
- `get_listing_opf_bool()`, `get_listing_opf_ship()`, `get_listing_opf_wing()`, ... (all 70+)
- `get_container_modifiers()`, `get_list_container_modifiers()`, `get_map_container_modifiers()`
- `check_for_dynamic_sexp_enum()`

**Design consideration:** These functions currently access `tree_nodes[]` (e.g., `get_listing_opf_bool` checks parent node type, `get_listing_opf_subsystem` looks up parent ship). They also use `_interface` for context (messages, goals, events, mission names). So these are methods on `SexpTreeModel` (or could be free functions that take a `SexpTreeModel&` reference). Keeping them as methods is simpler.

**Why separate file:** Contributors frequently add new OPF types. Having them isolated means:
- Easy to find where to add new types
- Merge conflicts are localized
- The main model file stays manageable

### 3. `code/missioneditor/sexp_tree_actions.h/.cpp` — Action Logic

A **separate class** `SexpTreeActions`, constructed with `SexpTreeModel&` + `ISexpTreeUI&`. This handles the "what to do" when a user triggers a menu action, separated from the "how to show the menu" (UI) and "how to render the result" (UI).

```cpp
class SexpTreeActions {
public:
    SexpTreeActions(SexpTreeModel& model, ISexpTreeUI& ui);

    // Menu structure computation
    SexpMenuItems compute_menu_items(int selected_node);

    // Action execution
    void add_or_replace_operator(int op, int replace_flag = 0);
    void replace_operator(const char* op);
    void replace_data(const char* data, int type);
    void replace_variable_data(int var_idx, int type);
    void replace_container_name(const sexp_container& container);
    void replace_container_data(const sexp_container& container, int type,
                                 bool test_child_nodes, bool delete_child_nodes,
                                 bool set_default_modifier);
    void add_default_modifier(const sexp_container& container);
    void expand_operator(int node);
    void merge_operator(int node);
    void add_default_operator(int op, int argnum);

    // Clipboard operations
    void node_cut();
    void node_copy();
    void node_paste(bool replace);
    void node_delete();

private:
    SexpTreeModel& _model;
    ISexpTreeUI& _ui;
};
```

**`SexpMenuItems` data structure** — returned by `compute_menu_items()`, describes what menu items should be available:
- Which operator categories to show for add/replace/insert
- Which variables are valid replacements
- Which container names/data are valid
- Whether cut/copy/paste/delete are enabled
- Whether edit-data, edit-comment, edit-color are enabled
- Replaces the bulk of `right_clicked()` (FRED2) / `buildContextMenu()` (QtFRED) logic

**`ISexpTreeUI` callback interface** — defined in `sexp_tree_model.h`, implemented by both UI layers:
```cpp
class ISexpTreeUI {
public:
    virtual ~ISexpTreeUI() = default;

    // Tree widget manipulation
    virtual void* ui_insert_item(const char* text, NodeImage image,
                                  void* parent, void* insert_after) = 0;
    virtual void ui_delete_item(void* handle) = 0;
    virtual void ui_set_item_text(void* handle, const char* text) = 0;
    virtual void ui_set_item_image(void* handle, NodeImage image) = 0;
    virtual void* ui_get_parent_item(void* handle) = 0;
    virtual void* ui_get_child_item(void* handle) = 0;
    virtual void* ui_get_next_item(void* handle) = 0;
    virtual void ui_ensure_visible(void* handle) = 0;
    virtual void ui_select_item(void* handle) = 0;
    virtual void ui_expand_item(void* handle) = 0;

    // Notifications
    virtual void ui_notify_modified() = 0;
    virtual void ui_show_node_error(int node, const char* msg) = 0;
};
```

FRED2 implements this with MFC calls (`InsertItem`, `DeleteItem`, etc.).
QtFRED implements this with Qt calls (`addTopLevelItem`, `removeChild`, etc.).

UI layers own both a `SexpTreeModel` and a `SexpTreeActions`:
```cpp
// QtFRED example
class sexp_tree : public QTreeWidget, public ISexpTreeUI {
    SexpTreeModel _model;
    SexpTreeActions _actions;  // constructed with _model + *this
    // ...
};
```

### What Stays in the UI Layers

**FRED2 `fred2/sexp_tree.h/.cpp`:**
- `class sexp_tree : public CTreeCtrl, public ISexpTreeUI`
- Owns a `SexpTreeModel` (composition, not inheritance)
- MFC message map handlers: `OnBegindrag`, `OnMouseMove`, `OnLButtonUp`, `OnKeyDown`, etc.
- `right_clicked()` — calls model's `compute_menu_items()`, builds MFC `CMenu`, calls `TrackPopupMenu()`
- `OnCommand()` — dispatches MFC command IDs to model action functions
- `build_tree()` — iterates model nodes, inserts MFC tree items
- `load_tree()`/`load_branch()` — calls model load, then syncs UI
- `edit_label()`, `end_label_edit()` — MFC inline editing
- `start_operator_edit()`, `end_operator_edit()` — MFC combo box
- `OperatorComboBox` — MFC-specific widget
- Drag-and-drop event handling
- `ISexpTreeUI` implementation (MFC calls)

**QtFRED `qtfred/src/ui/widgets/sexp_tree.h/.cpp`:**
- `class sexp_tree : public QTreeWidget, public ISexpTreeUI`
- Owns a `SexpTreeModel` (composition)
- Qt event overrides: `keyPressEvent`, `mousePressEvent`, `mouseMoveEvent`, etc.
- `buildContextMenu()` — calls model's `compute_menu_items()`, builds `QMenu` with `QAction`s
- `customMenuHandler()` — Qt context menu slot
- `handleNewItemSelected()` — Qt selection change
- Operator quick-search popup (`_opPopup`, `_opEdit`, `_opList`)
- Qt signals for `modified()`, `rootNodeDeleted()`, etc.
- `NoteBadgeDelegate` — Qt custom painting
- `ISexpTreeUI` implementation (Qt calls)

## Node Handle Strategy

`sexp_tree_item::handle` becomes `void*` in the shared model. UI layers cast:
- FRED2: `static_cast<HTREEITEM>(node.handle)` / `reinterpret_cast<void*>(htreeitem)`
- QtFRED: `static_cast<QTreeWidgetItem*>(node.handle)` / `static_cast<void*>(qtreewidgetitem)`

This is pragmatic — these are opaque pointers the model never dereferences. The model stores them; only the UI layer reads/writes them through `ISexpTreeUI`.

## Tree Coloring

Node coloring determination is model logic (which color to assign based on node state/type) — goes in `sexp_tree_model`. The actual rendering (setting background brush, calling `setBackground()` or `SetItemColor()`) stays in the UI layer.

The model provides:
```cpp
// Returns the appropriate color for a node based on its state
QColor get_node_color(int node) const;  // or use a framework-agnostic color struct
```

Or simpler: define a `NodeColorState` enum in the model and let UI map it to actual colors.

## Tree Searching

`find_text()` splits:
- **Model** (`sexp_tree_model`): `find_text_in_nodes(const char* text, int* find)` — walks `tree_nodes[]`, returns the node index of the match
- **UI**: calls model search, then does `ensure_visible()` + `hilite_item()` on the returned node

## Migration Strategy (Incremental)

This is too large for a single PR. Suggested order:

### Phase 1: Foundation
1. Create `sexp_tree_model.h/.cpp` with data structures (`sexp_tree_item` with `void*` handle, `sexp_list_item`, `NodeImage` enum, `TreeFlags`, `SexpTreeEditorInterface`)
2. Create empty `sexp_tree_opf.h/.cpp` and `sexp_tree_actions.h/.cpp`
3. Add all files to `code/source_groups.cmake`
4. Both FRED2 and QtFRED include the new headers but don't use them yet

### Phase 2: OPF Functions (biggest bang for buck)
5. Move all `get_listing_opf_*()` implementations to `sexp_tree_opf.cpp`
6. Both UI sexp_tree classes call the shared implementations
7. Delete duplicate code from both `sexp_tree.cpp` files
   - This alone removes ~4,000 lines of duplication

### Phase 3: Pure Model Functions
8. Move pure query/analysis functions to `sexp_tree_model.cpp`
9. Move save_tree/save_branch to model
10. Move variable/container utilities to model
11. UI classes delegate to model

### Phase 4: Actions + ISexpTreeUI
12. Define `ISexpTreeUI` interface
13. Implement in both FRED2 and QtFRED
14. Move action functions to `sexp_tree_actions.cpp`
15. Move mixed functions (load_tree, expand_operator, etc.) using the UI callback interface
16. Move menu computation logic

### Phase 5: Cleanup
17. Remove remaining duplication
18. Both UI `sexp_tree.cpp` files should be ~500-1000 lines each (down from ~8,000)

## File Size Estimates (Final State)

| File | Lines | Content |
|------|-------|---------|
| `code/missioneditor/sexp_tree_model.h` | ~200 | Data structures, model class declaration, ISexpTreeUI, SexpTreeEditorInterface |
| `code/missioneditor/sexp_tree_model.cpp` | ~1,500 | Core model logic, tree management, queries, variables, help |
| `code/missioneditor/sexp_tree_opf.h` | ~10 | Just the header (declarations are in model class) |
| `code/missioneditor/sexp_tree_opf.cpp` | ~4,000 | All OPF listing functions |
| `code/missioneditor/sexp_tree_actions.h` | ~100 | SexpTreeActions class, SexpMenuItems data structure |
| `code/missioneditor/sexp_tree_actions.cpp` | ~1,500 | Menu computation, action execution |
| `fred2/sexp_tree.cpp` | ~800 | MFC UI wrapper |
| `qtfred/sexp_tree.cpp` | ~800 | Qt UI wrapper |

## Resolved Design Decisions

1. **Composition for model.** UI classes (`sexp_tree`) own a `SexpTreeModel` via composition, not inheritance. Cleaner separation even though it requires forwarding.

2. **SCP types in the model.** `SexpTreeEditorInterface` uses `SCP_vector<SCP_string>` instead of `QStringList`/`QString`. The Qt-specific `getContextMenuExtras(QObject*)` stays in a QtFRED-only subclass. UI layers translate between SCP and framework types at the boundary.

3. **Color representation.** Model uses a `NodeColorState` enum or simple struct. UI layers map to `QColor` / MFC color as needed.

4. **OPF: same class, split file.** `get_listing_opf_*()` remain methods on `SexpTreeModel`, declared in `sexp_tree_model.h`, implemented in `sexp_tree_opf.cpp`. No call-site changes needed.

5. **Actions: separate class.** `SexpTreeActions` is its own class, constructed with `SexpTreeModel&` + `ISexpTreeUI&`. Provides menu computation and action execution. UI layers own an instance alongside their model.
