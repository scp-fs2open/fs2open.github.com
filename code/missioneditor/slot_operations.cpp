#include "slot_operations.h"
#include "common.h"
#include "ai/ai.h"
#include "globalincs/linklist.h"
#include "mission/missionparse.h"
#include "jumpnode/jumpnode.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "parse/sexp.h"
#include "prop/prop.h"
#include "ship/ship.h"

#include <algorithm>

// Forward declarations -------

// Restore the obj_used_list invariant for the OBJ_SHIP/OBJ_START subset:
// among ship-type entries, list order matches Ships[] index order.
static void resort_ships_in_obj_used_list();

// Same, for the OBJ_PROP subset: among prop entries, obj_used_list order matches
// Props[] index order.  UI lists that walk obj_used_list, like the Scene Browser,
// depend on this to show props in their reordered Props[] order.
static void resort_props_in_obj_used_list();

// Restore the OBJ_WAYPOINT subset invariant: list order matches the composite
// (waypoint_list_index, waypoint_index) order encoded in Objects[].instance
// via calc_waypoint_instance.
static void resort_waypoints_in_obj_used_list();

// Restore the OBJ_JUMP_NODE subset invariant: list order matches Jump_nodes[]
// vector order.  Lookup is by CJumpNode::GetSCPObjectNumber() since
// Objects[].instance is unused for jump nodes.
static void resort_jump_nodes_in_obj_used_list();

// ----------------------------

static int find_free_slot(int max_slots, bool (*slot_is_empty)(int))
{
	for (int i = 0; i < max_slots; ++i)
	{
		if (slot_is_empty(i))
			return i;
	}
	return -1;
}

// How the swap/rotate templates obtain (and, for vector-backed storage, give
// back) the temporary slot used to park the moving item, plus the bounds and
// emptiness test for the storage.  Fixed arrays scan for an existing hole and
// have nothing to release; vector-backed types append a fresh empty element,
// so acquisition never fails.
struct SlotOps
{
	int max_slots;                    // one-past-max valid slot index (snapshot taken before temp acquisition)
	bool (*slot_is_empty)(int);
	int (*acquire_temp_slot)();       // returns an empty slot to park the mover in, or -1 if none
	void (*release_temp_slot)(int);   // nullptr if there is nothing to release (fixed arrays)
};

template <typename TConfig>
static void swap_slots(int a, int b, const TConfig& cfg, const SlotOps& ops,
	void (*reassign)(int, int, const TConfig&, bool), const char* caller)
{
	if (a == b)
		return;

	Assertion(a >= 0 && a < ops.max_slots, "%s: slot 'a' %d out of range", caller, a);
	Assertion(b >= 0 && b < ops.max_slots, "%s: slot 'b' %d out of range", caller, b);
	Assertion(!ops.slot_is_empty(a) && !ops.slot_is_empty(b),
		"%s: both slots must be valid (a=%d, b=%d)", caller, a, b);

	// Obtain a free temporary slot.
	int tmp = ops.acquire_temp_slot();
	if (tmp < 0)
	{
		ReleaseWarning(LOCATION, "%s: no free slot available for the temporary leg", caller);
		return;
	}

	// Three-leg swap; each call's preconditions hold by construction.  The
	// total-rebuild fixups are deferred to the final leg.
	reassign(a, tmp, cfg, false);
	reassign(b, a, cfg, false);
	reassign(tmp, b, cfg, true);

	if (ops.release_temp_slot != nullptr)
		ops.release_temp_slot(tmp);
}

template <typename TConfig>
static void rotate_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const TConfig& cfg,
	const SlotOps& ops, void (*reassign)(int, int, const TConfig&, bool), const char* caller)
{
	if (from_pos == to_pos)
		return;

	int count = sz2i(slots.size());
	Assertion(from_pos >= 0 && from_pos < count, "%s: 'from' position %d out of range", caller, from_pos);
	Assertion(to_pos >= 0 && to_pos < count, "%s: 'to' position %d out of range", caller, to_pos);

	// Obtain a free temporary slot.
	int tmp = ops.acquire_temp_slot();
	if (tmp < 0)
	{
		ReleaseWarning(LOCATION, "%s: no free slot available for the temporary leg", caller);
		return;
	}

	// Park the moving item in the free slot, shift everything between the two
	// positions over by one, then drop the item into the slot vacated at the
	// far end.  Preserves the relative order of everything else, in K+2
	// reassignments for a move of K positions (vs 3K for a bubble of swaps).
	// The total-rebuild fixups are deferred to the final leg.
	int step = (to_pos > from_pos) ? 1 : -1;
	reassign(slots[from_pos], tmp, cfg, false);
	for (int j = from_pos; j != to_pos; j += step)
		reassign(slots[j + step], slots[j], cfg, false);
	reassign(tmp, slots[to_pos], cfg, true);

	if (ops.release_temp_slot != nullptr)
		ops.release_temp_slot(tmp);
}

void reassign_ship_slot(int from, int to, const FredShipSlotConfig& cfg, bool resort_obj_list)
{
	Assertion(Fred_running, "reassign_ship_slot is FRED-only: it does not fix up game-context references like Player_ship or runtime Ai_info shipnums");
	Assertion(from != to, "reassign_ship_slot: from == to (%d)", from);
	Assertion(from >= 0 && from < MAX_SHIPS, "reassign_ship_slot: 'from' slot %d out of range", from);
	Assertion(to >= 0 && to < MAX_SHIPS, "reassign_ship_slot: 'to' slot %d out of range", to);
	Assertion(Ships[from].objnum >= 0, "reassign_ship_slot: source slot %d is empty", from);
	Assertion(Ships[to].objnum < 0, "reassign_ship_slot: destination slot %d is occupied", to);

	// Move the ship struct itself.  Ship's move operations handle the members
	// that need care (the subsys_list sentinel re-links its bookend nodes, and
	// the owning pointers are unique_ptrs); this function's job is the external
	// back-references.  Per the engine's convention, a slot with objnum < 0 is
	// considered empty.
	Ships[to] = std::move(Ships[from]);
	Ships[from].objnum = -1;

	// Move FRED-side parallel arrays if the caller supplied them.
	if (cfg.fred_alt_names != nullptr)
	{
		strcpy_s(cfg.fred_alt_names[to], cfg.fred_alt_names[from]);
		cfg.fred_alt_names[from][0] = '\0';
	}
	if (cfg.fred_callsigns != nullptr)
	{
		strcpy_s(cfg.fred_callsigns[to], cfg.fred_callsigns[from]);
		cfg.fred_callsigns[from][0] = '\0';
	}

	// Object back-reference.
	Objects[Ships[to].objnum].instance = to;

	// Keep obj_used_list iteration order in sync with Ships[] slot order.  The
	// re-sort is a total rebuild rather than an incremental fix, so a caller
	// making a batch of reassignments may defer it to the final call.
	if (resort_obj_list)
		resort_ships_in_obj_used_list();

	// AI back-reference (the one invariant codified by internal_integrity_check).
	if (Ships[to].ai_index >= 0)
		Ai_info[Ships[to].ai_index].shipnum = to;

	// Wing membership: scan every wing and re-point any reference to the old slot.
	// (wing.special_ship is wing-relative, NOT a Ships[] index, so it is intentionally
	// not touched here.)
	for (auto &w: Wings)
	{
		if (w.wave_count == 0)
			continue;
		for (int k = 0; k < w.wave_count; ++k)
		{
			if (w.ship_index[k] == from)
				w.ship_index[k] = to;
		}
	}

	// Single-player start.
	if (Player_start_shipnum == from)
		Player_start_shipnum = to;

	// Ship_registry caches the shipnum on its entries (lookup is by name, but the
	// cached integer would otherwise go stale).
	int reg = ship_registry_get_index(Ships[to].ship_name);
	if (reg >= 0)
		Ship_registry[reg].shipnum = to;

	// FRED's current-ship pointer, if the caller is tracking one.
	if (cfg.cur_ship != nullptr && *cfg.cur_ship == from)
		*cfg.cur_ship = to;
}

static bool ship_slot_is_empty(int i)
{
	return Ships[i].objnum < 0;
}

static int ship_acquire_temp_slot()
{
	return find_free_slot(MAX_SHIPS, ship_slot_is_empty);
}

static SlotOps ship_slot_ops()
{
	return { MAX_SHIPS, ship_slot_is_empty, ship_acquire_temp_slot, nullptr };
}

void swap_ship_slots(int a, int b, const FredShipSlotConfig& cfg)
{
	swap_slots(a, b, cfg, ship_slot_ops(), reassign_ship_slot, "swap_ship_slots");
}

void rotate_ship_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredShipSlotConfig& cfg)
{
	rotate_slots(slots, from_pos, to_pos, cfg, ship_slot_ops(), reassign_ship_slot, "rotate_ship_slots");
}

void reassign_wing_slot(int from, int to, const FredWingSlotConfig& cfg, bool update_wing_indexes)
{
	Assertion(Fred_running, "reassign_wing_slot is FRED-only");
	Assertion(from != to, "reassign_wing_slot: from == to (%d)", from);
	Assertion(from >= 0 && from < MAX_WINGS, "reassign_wing_slot: 'from' slot %d out of range", from);
	Assertion(to >= 0 && to < MAX_WINGS, "reassign_wing_slot: 'to' slot %d out of range", to);
	Assertion(Wings[from].wave_count > 0, "reassign_wing_slot: source slot %d is empty", from);
	Assertion(Wings[to].wave_count == 0, "reassign_wing_slot: destination slot %d is occupied", to);

	// Move the wing struct itself; wave_count == 0 is the sentinel for an empty wing.
	Wings[to] = std::move(Wings[from]);
	Wings[from].wave_count = 0;

	// Move FRED-side parallel array if the caller supplied it.
	if (cfg.wing_objects != nullptr)
	{
		for (int k = 0; k < MAX_SHIPS_PER_WING; ++k)
		{
			cfg.wing_objects[to][k] = cfg.wing_objects[from][k];
			cfg.wing_objects[from][k] = -1;
		}
	}

	// Per-ship parent-wing back-reference.
	for (auto &s: Ships)
	{
		if (s.objnum < 0)
			continue;
		if (s.wingnum == from)
			s.wingnum = to;
	}

	// FRED's current-wing pointer, if the caller is tracking one.
	if (cfg.cur_wing != nullptr && *cfg.cur_wing == from)
		*cfg.cur_wing = to;

	// Rebuild Starting/Squadron/TVT_wings caches from the parallel name arrays.
	// The rebuild is total rather than incremental, so a caller making a batch
	// of reassignments may defer it to the final call.
	if (update_wing_indexes)
		update_custom_wing_indexes();
}

static bool wing_slot_is_empty(int i)
{
	return Wings[i].wave_count == 0;
}

static int wing_acquire_temp_slot()
{
	return find_free_slot(MAX_WINGS, wing_slot_is_empty);
}

static SlotOps wing_slot_ops()
{
	return { MAX_WINGS, wing_slot_is_empty, wing_acquire_temp_slot, nullptr };
}

void swap_wing_slots(int a, int b, const FredWingSlotConfig& cfg)
{
	swap_slots(a, b, cfg, wing_slot_ops(), reassign_wing_slot, "swap_wing_slots");
}

void rotate_wing_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredWingSlotConfig& cfg)
{
	rotate_slots(slots, from_pos, to_pos, cfg, wing_slot_ops(), reassign_wing_slot, "rotate_wing_slots");
}

static bool prop_slot_is_empty(int i)
{
	return !Props[i].has_value();
}

static int prop_acquire_temp_slot()
{
	// Reuse an existing hole if one is available; otherwise append one.
	int i = find_free_slot(sz2i(Props.size()), prop_slot_is_empty);
	if (i >= 0)
		return i;
	Props.emplace_back(std::nullopt);
	return sz2i(Props.size()) - 1;
}

static void prop_release_temp_slot(int i)
{
	// Only shrink if the temp slot was appended; a reused hole stays put.
	if (i == sz2i(Props.size()) - 1 && !Props[i].has_value())
		Props.pop_back();
}

static SlotOps prop_slot_ops()
{
	return { sz2i(Props.size()), prop_slot_is_empty, prop_acquire_temp_slot, prop_release_temp_slot };
}

static void reassign_prop_slot(int from, int to, const FredPropSlotConfig& /*cfg*/, bool resort_obj_list)
{
	Assertion(Fred_running, "reassign_prop_slot is FRED-only: it re-points object instances only, not any game-context state");
	Assertion(from != to, "reassign_prop_slot: from == to (%d)", from);
	Assertion(SCP_vector_inbounds(Props, from), "reassign_prop_slot: 'from' slot %d out of range", from);
	Assertion(SCP_vector_inbounds(Props, to), "reassign_prop_slot: 'to' slot %d out of range", to);
	Assertion(Props[from].has_value(), "reassign_prop_slot: source slot %d is empty", from);
	Assertion(!Props[to].has_value(), "reassign_prop_slot: destination slot %d is occupied", to);

	// Move the prop itself.  A moved-from optional is still engaged, so the
	// explicit reset() is what actually empties the source slot.
	Props[to] = std::move(Props[from]);
	Props[from].reset();

	// Object back-reference.
	Objects[Props[to]->objnum].instance = to;

	// Keep obj_used_list prop order in sync with Props[] index order, so UI
	// lists that walk it (like the Scene Browser) reflect the new order.  The
	// re-sort is a total rebuild, so a caller making a batch of reassignments
	// may defer it to the final call.
	if (resort_obj_list)
		resort_props_in_obj_used_list();
}

void swap_prop_slots(int a, int b, const FredPropSlotConfig& cfg)
{
	swap_slots(a, b, cfg, prop_slot_ops(), reassign_prop_slot, "swap_prop_slots");
}

void rotate_prop_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredPropSlotConfig& cfg)
{
	rotate_slots(slots, from_pos, to_pos, cfg, prop_slot_ops(), reassign_prop_slot, "rotate_prop_slots");
}

void rederive_cur_waypoint(const FredWaypointConfig& cfg)
{
	if (cfg.cur_waypoint == nullptr || cfg.cur_waypoint_list == nullptr || cfg.cur_object_index == nullptr)
		return;
	if (*cfg.cur_waypoint == nullptr)
		return;

	int idx = *cfg.cur_object_index;
	if (idx < 0 || idx >= MAX_OBJECTS || Objects[idx].type != OBJ_WAYPOINT)
	{
		// the stable key says no waypoint is selected, so any cached pointer is stale
		*cfg.cur_waypoint = nullptr;
		*cfg.cur_waypoint_list = nullptr;
		return;
	}

	*cfg.cur_waypoint = find_waypoint_with_instance(Objects[idx].instance);
	*cfg.cur_waypoint_list = (*cfg.cur_waypoint != nullptr) ? (*cfg.cur_waypoint)->get_parent_list() : nullptr;
}

static bool waypoint_list_slot_is_empty(int i)
{
	// The engine keeps Waypoint_lists compact (removing a list's last waypoint
	// removes the list), so an empty list only ever exists as the templates'
	// temporary slot.
	return Waypoint_lists[i].get_waypoints().empty();
}

static int waypoint_list_acquire_temp_slot()
{
	Waypoint_lists.emplace_back();
	return sz2i(Waypoint_lists.size()) - 1;
}

static void waypoint_list_release_temp_slot(int i)
{
	Assertion(i == sz2i(Waypoint_lists.size()) - 1 && waypoint_list_slot_is_empty(i),
		"waypoint_list_release_temp_slot: slot %d is not the appended temporary", i);
	Waypoint_lists.pop_back();
}

static SlotOps waypoint_list_slot_ops()
{
	return { sz2i(Waypoint_lists.size()), waypoint_list_slot_is_empty, waypoint_list_acquire_temp_slot, waypoint_list_release_temp_slot };
}

static void reassign_waypoint_list_slot(int from, int to, const FredWaypointConfig& cfg, bool final_leg)
{
	Assertion(Fred_running, "reassign_waypoint_list_slot is FRED-only: it re-points object instances only, not any game-context state");
	Assertion(from != to, "reassign_waypoint_list_slot: from == to (%d)", from);
	Assertion(SCP_vector_inbounds(Waypoint_lists, from), "reassign_waypoint_list_slot: 'from' slot %d out of range", from);
	Assertion(SCP_vector_inbounds(Waypoint_lists, to), "reassign_waypoint_list_slot: 'to' slot %d out of range", to);
	Assertion(!waypoint_list_slot_is_empty(from), "reassign_waypoint_list_slot: source slot %d is empty", from);
	Assertion(waypoint_list_slot_is_empty(to), "reassign_waypoint_list_slot: destination slot %d is occupied", to);

	// Move the list itself.  waypoint_list's move operations are defaulted and
	// its name is a char array, which copies — so the moved-from slot still
	// carries the old name and the explicit re-init is what empties it.
	Waypoint_lists[to] = std::move(Waypoint_lists[from]);
	Waypoint_lists[from] = waypoint_list();

	// A waypoint object encodes its (list index, point index) in its instance,
	// so every waypoint in the moved list now sits at a new list index.
	int wi = 0;
	for (auto& wpt : Waypoint_lists[to].get_waypoints())
	{
		int objnum = wpt.get_objnum();
		if (objnum >= 0)
			Objects[objnum].instance = calc_waypoint_instance(to, wi);
		++wi;
	}

	// Total fixups, deferred to the final leg of a batch: keep obj_used_list
	// waypoint order in sync with the instance encoding, and re-derive the
	// editor's cur_waypoint/cur_waypoint_list pointers (which now point into
	// reshuffled, possibly reallocated storage) from the stable object index.
	if (final_leg)
	{
		resort_waypoints_in_obj_used_list();
		rederive_cur_waypoint(cfg);
	}
}

void swap_waypoint_lists(int a, int b, const FredWaypointConfig& cfg)
{
	swap_slots(a, b, cfg, waypoint_list_slot_ops(), reassign_waypoint_list_slot, "swap_waypoint_lists");
}

void rotate_waypoint_lists(int from_pos, int to_pos, const FredWaypointConfig& cfg)
{
	// Waypoint_lists is compact, so display positions are the slot indices.
	SCP_vector<int> slots(Waypoint_lists.size());
	for (int i = 0; i < sz2i(slots.size()); ++i)
		slots[i] = i;

	rotate_slots(slots, from_pos, to_pos, cfg, waypoint_list_slot_ops(), reassign_waypoint_list_slot, "rotate_waypoint_lists");
}

static bool jump_node_slot_is_empty(int i)
{
	// The engine keeps Jump_nodes compact (deletion erases the element), so a
	// detached node only ever exists as the templates' temporary slot or
	// transiently during deletion.
	return Jump_nodes[i].GetSCPObjectNumber() < 0;
}

static int jump_node_acquire_temp_slot()
{
	// A default-constructed CJumpNode is cheap: no model load, no object.
	Jump_nodes.emplace_back();
	return sz2i(Jump_nodes.size()) - 1;
}

static void jump_node_release_temp_slot(int i)
{
	Assertion(i == sz2i(Jump_nodes.size()) - 1 && jump_node_slot_is_empty(i),
		"jump_node_release_temp_slot: slot %d is not the appended temporary", i);
	Jump_nodes.pop_back();
}

static SlotOps jump_node_slot_ops()
{
	return { sz2i(Jump_nodes.size()), jump_node_slot_is_empty, jump_node_acquire_temp_slot, jump_node_release_temp_slot };
}

static void reassign_jump_node_slot(int from, int to, const FredJumpNodeSlotConfig& /*cfg*/, bool resort_obj_list)
{
	Assertion(Fred_running, "reassign_jump_node_slot is FRED-only: it reorders the editor's jump-node list only");
	Assertion(from != to, "reassign_jump_node_slot: from == to (%d)", from);
	Assertion(SCP_vector_inbounds(Jump_nodes, from), "reassign_jump_node_slot: 'from' slot %d out of range", from);
	Assertion(SCP_vector_inbounds(Jump_nodes, to), "reassign_jump_node_slot: 'to' slot %d out of range", to);
	Assertion(!jump_node_slot_is_empty(from), "reassign_jump_node_slot: source slot %d is empty", from);
	Assertion(jump_node_slot_is_empty(to), "reassign_jump_node_slot: destination slot %d is occupied", to);

	// Move the node itself.  CJumpNode's move operations are defaulted, so the
	// scalar m_objnum is copied and the moved-from node would still look
	// attached — the explicit re-init is what empties the source slot.  (Safe:
	// the destructor frees nothing; obj/model cleanup happens in
	// jumpnode_delete(), per the class comment.)
	Jump_nodes[to] = std::move(Jump_nodes[from]);
	Jump_nodes[from] = CJumpNode();

	// No object back-reference to re-point: the node links to its object via
	// m_objnum, which travels with it, and Objects[].instance is unused for
	// jump nodes.

	// Keep obj_used_list jump-node order in sync with Jump_nodes[] order, so UI
	// lists that walk it reflect the new order.  Deferred to the final leg of a
	// batch.
	if (resort_obj_list)
		resort_jump_nodes_in_obj_used_list();
}

void swap_jump_node_slots(int a, int b, const FredJumpNodeSlotConfig& cfg)
{
	swap_slots(a, b, cfg, jump_node_slot_ops(), reassign_jump_node_slot, "swap_jump_node_slots");
}

void rotate_jump_node_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredJumpNodeSlotConfig& cfg)
{
	rotate_slots(slots, from_pos, to_pos, cfg, jump_node_slot_ops(), reassign_jump_node_slot, "rotate_jump_node_slots");
}

// Individual waypoints are referenced in SEXPs and AI goals positionally by
// name ("listname:N", 1-based), so reordering waypoints within a list must
// rename every affected reference.  Renaming in place would collide when a
// name is both a source and a destination, so the helpers below route through
// temporary names ("<temp_wpt_N>"): rename affected refs to temps, shift the
// positions, then rename the temps to their final names.

static void rename_waypoint_sexp_refs_to_temp(const char *list_name, int one_based, char *temp_buf, size_t buf_size)
{
	char name[NAME_LENGTH];
	waypoint_stuff_name(name, list_name, one_based);
	snprintf(temp_buf, buf_size, "<temp_wpt_%d>", one_based);
	update_sexp_references(name, temp_buf);
	ai_update_goal_references(sexp_ref_type::WAYPOINT, name, temp_buf);
}

static void rename_waypoint_sexp_refs_from_temp(const char *temp_name, const char *list_name, int new_1based)
{
	char new_name[NAME_LENGTH];
	waypoint_stuff_name(new_name, list_name, new_1based);
	update_sexp_references(temp_name, new_name);
	ai_update_goal_references(sexp_ref_type::WAYPOINT, temp_name, new_name);
}

void move_waypoint_within_list(int list_index, int from, int to)
{
	Assertion(Fred_running, "move_waypoint_within_list is FRED-only: it rewrites editor-side SEXP and AI goal references");
	Assertion(SCP_vector_inbounds(Waypoint_lists, list_index), "move_waypoint_within_list: list index %d out of range", list_index);

	auto &wl = Waypoint_lists[list_index];
	auto list_name = wl.get_name();
	auto &wpts = wl.get_waypoints();

	Assertion(wpts.in_bounds(from), "move_waypoint_within_list: 'from' index %d out of range", from);
	Assertion(wpts.in_bounds(to), "move_waypoint_within_list: 'to' index %d out of range", to);
	if (from == to)
		return;

	int lo = std::min(from, to);
	int hi = std::max(from, to);

	// Step 1: Rename all affected SEXP refs to temporary names
	SCP_vector<SCP_string> temp_names(hi - lo + 1);
	for (int i = lo; i <= hi; i++)
	{
		char temp[NAME_LENGTH + 16];
		rename_waypoint_sexp_refs_to_temp(list_name, i + 1, temp, sizeof(temp));
		temp_names[i - lo] = temp;
	}

	// Step 2: Shift positions (waypoint objects stay in place)
	vec3d saved_pos = *wpts[from].get_pos();
	if (from < to)
	{
		for (int i = from; i < to; i++)
			wpts[i].set_pos(wpts[i + 1].get_pos());
	}
	else
	{
		for (int i = from; i > to; i--)
			wpts[i].set_pos(wpts[i - 1].get_pos());
	}
	wpts[to].set_pos(&saved_pos);

	// Step 3: Rename from temp names to final (shifted) names.
	// temp_names[i - lo] holds the temp name for what was originally at 0-based index i.
	// Map each original index to its new index after the move:
	//   - The element at 'from' moved to 'to'
	//   - If from < to: elements at from+1..to shifted down by 1
	//   - If from > to: elements at to..from-1 shifted up by 1
	for (int i = lo; i <= hi; i++)
	{
		int new_index;
		if (i == from)
			new_index = to;
		else if (from < to)
			new_index = i - 1;  // shifted down
		else
			new_index = i + 1;  // shifted up
		rename_waypoint_sexp_refs_from_temp(temp_names[i - lo].c_str(), list_name, new_index + 1);
	}
}

void swap_waypoints_within_list(int list_index, int a, int b)
{
	Assertion(Fred_running, "swap_waypoints_within_list is FRED-only: it rewrites editor-side SEXP and AI goal references");
	Assertion(SCP_vector_inbounds(Waypoint_lists, list_index), "swap_waypoints_within_list: list index %d out of range", list_index);

	auto &wl = Waypoint_lists[list_index];
	auto list_name = wl.get_name();
	auto &wpts = wl.get_waypoints();

	Assertion(wpts.in_bounds(a), "swap_waypoints_within_list: index 'a' %d out of range", a);
	Assertion(wpts.in_bounds(b), "swap_waypoints_within_list: index 'b' %d out of range", b);
	if (a == b)
		return;

	// Step 1: Rename both SEXP refs to temp names
	char temp_a[NAME_LENGTH + 16];
	char temp_b[NAME_LENGTH + 16];
	rename_waypoint_sexp_refs_to_temp(list_name, a + 1, temp_a, sizeof(temp_a));
	rename_waypoint_sexp_refs_to_temp(list_name, b + 1, temp_b, sizeof(temp_b));

	// Step 2: Swap positions (waypoint objects stay in place)
	vec3d pos_a = *wpts[a].get_pos();
	vec3d pos_b = *wpts[b].get_pos();
	wpts[a].set_pos(&pos_b);
	wpts[b].set_pos(&pos_a);

	// Step 3: Rename from temp to final (swapped positions)
	rename_waypoint_sexp_refs_from_temp(temp_a, list_name, b + 1);
	rename_waypoint_sexp_refs_from_temp(temp_b, list_name, a + 1);
}

// Bulk-re-sort one type's subset of obj_used_list while keeping non-matching
// entries in their original relative positions.  Each callsite supplies a
// type matcher and a key function; the i-th matching slot (in original list
// order) receives the i-th smallest matching node by key.
static void resort_obj_used_list_subset(
	bool (*matches_type)(int),
	int (*key)(const object*))
{
	SCP_vector<object*> all;
	SCP_vector<object*> matched;
	for (auto o : list_range(&obj_used_list))
	{
		all.push_back(o);
		if (matches_type(o->type))
			matched.push_back(o);
	}

	std::sort(matched.begin(), matched.end(),
		[&](const object* a, const object* b) { return key(a) < key(b); });

	list_init(&obj_used_list);
	auto it = matched.begin();
	for (auto o : all)
	{
		if (matches_type(o->type))
		{
			list_append(&obj_used_list, *it);
			++it;
		}
		else
		{
			list_append(&obj_used_list, o);
		}
	}
}

static void resort_ships_in_obj_used_list()
{
	resort_obj_used_list_subset(
		[](int t) { return t == OBJ_SHIP || t == OBJ_START; },
		[](const object* o) { return o->instance; });
}

static void resort_props_in_obj_used_list()
{
	resort_obj_used_list_subset(
		[](int t) { return t == OBJ_PROP; },
		[](const object* o) { return o->instance; });
}

static void resort_waypoints_in_obj_used_list()
{
	resort_obj_used_list_subset(
		[](int t) { return t == OBJ_WAYPOINT; },
		[](const object* o) { return o->instance; });
}

static void resort_jump_nodes_in_obj_used_list()
{
	resort_obj_used_list_subset(
		[](int t) { return t == OBJ_JUMP_NODE; },
		[](const object* o)
		{
			for (int i = 0; i < (int)Jump_nodes.size(); ++i)
				if (Jump_nodes[i].GetSCPObjectNumber() == OBJ_INDEX(o))
					return i;

			return INT_MAX;
		});
}
