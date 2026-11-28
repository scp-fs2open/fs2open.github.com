# Module: jumpnode — `code/jumpnode/`

## Purpose
**Jump nodes**: the subspace portals a mission places in the world. A jump node
is an `OBJ_JUMP_NODE` object with a model, a name, and a HUD colour. It is a
navigation and mission-scripting marker rather than a simulated entity — ships
do not physically pass through it; SEXPs and the AI reference it by name.

## Key files
- `jumpnode.cpp` / `jumpnode.h` — the whole module: the `CJumpNode` class and
  the lookup helpers.

## Core data structures / globals
- `class CJumpNode` — one node: `m_modelnum`, `m_objnum`,
  `m_polymodel_instance_num` (jump nodes rotate, so they carry a model
  instance), `m_flags`, a name and optional display name, a display colour, and
  a FRED layer. It is **move-only** — the copy constructor and copy assignment
  are deleted — so store an index or look one up rather than copying.
- `SCP_vector<CJumpNode> Jump_nodes` — all nodes in the mission.

## Major constants
- `JN_USE_DISPLAY_COLOR` — use the node's own colour instead of the HUD colour.
- `JN_SHOW_POLYS` — draw the model solid rather than as wireframe.
- `JN_HIDE` — hide the node.
- `JN_SPECIAL_MODEL` — the node uses a model other than the default.
- `JN_HAS_DISPLAY_NAME`.
- `JN_DEFAULT_MODEL` (`"subspacenode.pof"`).

## Key entry points
- Lookups: `jumpnode_get_by_name()`, `jumpnode_lookup()`,
  `jumpnode_get_by_objnum()`, `jumpnode_get_by_objp()`, and
  `jumpnode_get_which_in(objp)` — which node, if any, a given object is inside.
- Per node: `SetModel()`, `SetName()`, `SetDisplayName()`, `SetVisibility()`,
  `SetAlphaColor()`, `Render()`, `FreeModelResources()`.
- `jumpnode_render_all()`, `jumpnode_delete()`, `jumpnode_level_close()`.

## Configuration tables
None. Jump nodes are per-mission `.fs2` data.

## See also
- `code/object/` (`OBJ_JUMP_NODE`), `code/parse/sexp.*` (jump-node SEXPs and
  the `OPF_JUMP_NODE_NAME` argument type), `code/ai/` (departure to a node),
  `code/hud/` (nodes shown on the HUD and escort list), the FRED jump-node
  editors.
