#pragma once
#include "globalincs/globals.h"

struct FredShipSlotConfig
{
	char (*fred_alt_names)[NAME_LENGTH + 1] = nullptr;
	char (*fred_callsigns)[NAME_LENGTH + 1] = nullptr;

	int *cur_ship = nullptr;
};

// Move the ship currently in Ships[from] into Ships[to], updating every
// back-reference (Objects, Ai_info, Wings, Player_start_shipnum, Ship_registry,
// and editor-side fields supplied via cfg).  Leaves Ships[from] empty.
// Preconditions: from != to, Ships[from].objnum >= 0, Ships[to].objnum < 0.
// No caller may hold a ship* to either slot across this call.
// Fields in cfg whose pointers are nullptr are skipped.
void reassign_ship_slot(int from, int to, const FredShipSlotConfig& cfg, bool resort_obj_list = true);

// Swap the contents of two slots.  Both must be valid (Ships[a].objnum >= 0
// and Ships[b].objnum >= 0).  Implemented as three calls to reassign_ship_slot
// via a temporary empty slot.
void swap_ship_slots(int a, int b, const FredShipSlotConfig& cfg);

// Move the item at position from_pos in slots to position to_pos, shifting the
// items in between by one position.
void rotate_ship_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredShipSlotConfig& cfg);

struct FredWingSlotConfig
{
	int (*wing_objects)[MAX_SHIPS_PER_WING] = nullptr;
	int *cur_wing = nullptr;
};

// Move the wing currently in Wings[from] into Wings[to], updating every
// back-reference (Ships[i].wingnum, Starting/Squadron/TVT_wings caches, and
// editor-side fields supplied via cfg).  Leaves Wings[from] empty.
// Preconditions: from != to, Wings[from].wave_count > 0, Wings[to].wave_count == 0.
// No caller may hold a wing* to either slot across this call.
// Fields in cfg whose pointers are nullptr are skipped.
void reassign_wing_slot(int from, int to, const FredWingSlotConfig& cfg, bool update_wing_indexes = true);

// Swap the contents of two slots.  Both must be valid (Wings[a].wave_count > 0
// and Wings[b].wave_count > 0).  Implemented as three calls to
// reassign_wing_slot via a temporary empty slot.
void swap_wing_slots(int a, int b, const FredWingSlotConfig& cfg);

// Move the item at position from_pos in slots to position to_pos, shifting the
// items in between by one position.
void rotate_wing_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredWingSlotConfig& cfg);

struct FredPropSlotConfig
{
	// No editor-side prop state needs fixups today (the only index-based
	// back-reference is Objects[].instance, handled internally).  Reserved for
	// future fields.
};

// Swap the props in Props[] slots a and b.  Both slots must be occupied
// (Props[i].has_value()).  Also resorts obj_used_list so UI lists that walk it,
// like the Scene Browser, match.
void swap_prop_slots(int a, int b, const FredPropSlotConfig& cfg);

// Move the prop at display position from_pos to to_pos within `slots` (the
// occupied Props[] indices, in display order), shifting the props in between by
// one and preserving their relative order.  Props[] may contain empty nullopt
// holes; those stay put while the occupants are permuted among their slots.
// Also resorts obj_used_list so Scene Browser matches.
void rotate_prop_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredPropSlotConfig& cfg);

class waypoint;
class waypoint_list;

struct FredWaypointConfig
{
	// The editor's current-waypoint tracking is raw pointers into vectors that
	// this machinery reorders (and possibly reallocates), so they are re-derived
	// after the operation from the stable object index.  Fields whose pointers
	// are nullptr are skipped.
	waypoint** cur_waypoint = nullptr;
	waypoint_list** cur_waypoint_list = nullptr;
	const int* cur_object_index = nullptr;  // FRED2: &cur_object_index; qtFRED: &editor.currentObject
};

// Re-derive *cfg.cur_waypoint and *cfg.cur_waypoint_list from
// *cfg.cur_object_index via find_waypoint_with_instance.  No-op unless
// *cfg.cur_waypoint is currently non-null and the object is a valid
// OBJ_WAYPOINT.  Called internally on the final leg of the waypoint-list
// operations below; public for callers whose own mutations (e.g. waypoint
// creation or deletion) can also invalidate the pointers.
void rederive_cur_waypoint(const FredWaypointConfig& cfg);

// Swap the waypoint lists at indices a and b within Waypoint_lists.  Also
// resorts obj_used_list and re-derives the cfg's current-waypoint pointers.
void swap_waypoint_lists(int a, int b, const FredWaypointConfig& cfg);

// Move the waypoint list at index from_pos to to_pos within Waypoint_lists,
// shifting the lists in between by one and preserving their relative order.
// Waypoint_lists is kept compact, so positions are plain indices.  Also
// resorts obj_used_list and re-derives the cfg's current-waypoint pointers.
void rotate_waypoint_lists(int from_pos, int to_pos, const FredWaypointConfig& cfg);

// Move the waypoint at 0-based index `from` to index `to` within
// Waypoint_lists[list_index], shifting the waypoints in between by one.  The
// waypoint positions shuffle while the objects stay in place, so obj_used_list
// and any waypoint pointers are unaffected; SEXP and AI goal references to the
// affected "listname:N" names are renamed to track the move.
void move_waypoint_within_list(int list_index, int from, int to);

// Swap the waypoints at 0-based indices a and b within
// Waypoint_lists[list_index], with the same reference renaming.
void swap_waypoints_within_list(int list_index, int a, int b);

struct FredJumpNodeSlotConfig
{
	// No back-references exist (the node links to its object via m_objnum,
	// which travels with it, and Objects[].instance is unused for jump nodes).
	// Reserved for future fields.
};

// Swap the jump nodes in Jump_nodes[] slots a and b.  Both slots must be
// attached (GetSCPObjectNumber() >= 0).  Also resorts obj_used_list so UI
// lists that walk it match.
void swap_jump_node_slots(int a, int b, const FredJumpNodeSlotConfig& cfg);

// Move the jump node at display position from_pos to to_pos within `slots`
// (the attached Jump_nodes[] indices, in display order), shifting the nodes in
// between by one and preserving their relative order.  Also resorts
// obj_used_list so UI lists that walk it match.
void rotate_jump_node_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredJumpNodeSlotConfig& cfg);
