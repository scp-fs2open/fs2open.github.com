// methods and members common to any mission editor FSO may have
#include "common.h"
#include "ai/ai.h"
#include "globalincs/linklist.h"
#include "mission/missionparse.h"
#include "iff_defs/iff_defs.h"
#include "object/object.h"
#include "ship/ship.h"

#include <algorithm>

// to keep track of data
char Voice_abbrev_briefing[NAME_LENGTH];
char Voice_abbrev_campaign[NAME_LENGTH];
char Voice_abbrev_command_briefing[NAME_LENGTH];
char Voice_abbrev_debriefing[NAME_LENGTH];
char Voice_abbrev_message[NAME_LENGTH];
char Voice_abbrev_mission[NAME_LENGTH];
bool Voice_no_replace_filenames;
char Voice_script_entry_format[NOTES_LENGTH];
int Voice_export_selection; // 0=everything, 1=cmd brief, 2=brief, 3=debrief, 4=messages
bool Voice_group_messages;

SCP_string Voice_script_default_string = "Sender: $sender\r\nPersona: $persona\r\nFile: $filename\r\nMessage: $message";
SCP_string Voice_script_instructions_string = "$name - name of the message\r\n"
                                              "$filename - name of the message file\r\n"
                                              "$message - text of the message\r\n"
                                              "$persona - persona of the sender\r\n"
                                              "$sender - name of the sender\r\n"
                                              "$note - message notes\r\n\r\n"
                                              "Note that $persona and $sender will only appear for the Message section.";

float normalize_degrees(float deg)
{
	while (deg < -180.0f)
		deg += 360.0f;
	while (deg > 180.0f)
		deg -= 360.0f;
	// check for negative zero
	if (deg == -0.0f)
		deg = 0.0f;
	return deg;
}

void time_to_mission_info_string(const std::tm* src, char* dest, size_t dest_max_len)
{
	std::strftime(dest, dest_max_len, "%x at %X", src);
}

void stuff_special_arrival_anchor_name(char* buf, int iff_index, int restrict_to_players, bool retail_format)
{
	const char* iff_name = Iff_info[iff_index].iff_name;

	// stupid retail hack
	if (retail_format && !stricmp(iff_name, "hostile") && !restrict_to_players)
		iff_name = "enemy";

	if (restrict_to_players)
		sprintf(buf, "<any %s player>", iff_name);
	else
		sprintf(buf, "<any %s>", iff_name);

	strlwr(buf);
}

void stuff_special_arrival_anchor_name(char* buf, int anchor_num, bool retail_format)
{
	// filter out iff
	int iff_index = anchor_num;
	iff_index &= ~ANCHOR_SPECIAL_ARRIVAL;
	iff_index &= ~ANCHOR_SPECIAL_ARRIVAL_PLAYER;

	// filter players
	int restrict_to_players = (anchor_num & ANCHOR_SPECIAL_ARRIVAL_PLAYER);

	// get name
	stuff_special_arrival_anchor_name(buf, iff_index, restrict_to_players, retail_format);
}

// Ship and wing arrival and departure anchors should always be ship registry entry indexes, except for a very brief window during mission parsing.
// But FRED and QtFRED dialogs use ship indexes instead.  So, rather than refactor all the dialogs, this converts between the two.  If an anchor
// is a valid ship registry index, the equivalent ship index is returned; otherwise the special value (-1 or a flag) is returned instead.
int anchor_to_target(anchor_t anchor)
{
	auto anchor_entry = ship_registry_get(anchor);
	return anchor_entry ? anchor_entry->shipnum : anchor.value();
}

// Ship and wing arrival and departure anchors should always be ship registry entry indexes, except for a very brief window during mission parsing.
// But FRED and QtFRED dialogs use ship indexes instead.  So, rather than refactor all the dialogs, this converts between the two.  If a target
// is a valid ship index, the equivalent ship registry index is returned; otherwise the special value (-1 or a flag) is returned instead.
anchor_t target_to_anchor(int target)
{
	if (target >= 0 && target < MAX_SHIPS)
		return anchor_t(ship_registry_get_index(Ships[target].ship_name));
	else
		return anchor_t(target);
}

void update_custom_wing_indexes()
{
	int i;

	for (i = 0; i < MAX_STARTING_WINGS; i++)
		Starting_wings[i] = wing_name_lookup(Starting_wing_names[i], 1);

	for (i = 0; i < MAX_SQUADRON_WINGS; i++)
		Squadron_wings[i] = wing_name_lookup(Squadron_wing_names[i], 1);

	for (i = 0; i < MAX_TVT_WINGS; i++)
		TVT_wings[i] = wing_name_lookup(TVT_wing_names[i], 1);
}

void generate_weaponry_usage_list_team(int team, int* arr)
{
	int i;

	for (i = 0; i < MAX_WEAPON_TYPES; i++) {
		arr[i] = 0;
	}

	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		Assert(team >= 0 && team < MAX_TVT_TEAMS);

		for (i = 0; i < MAX_TVT_WINGS_PER_TEAM; i++) {
			generate_weaponry_usage_list_wing(TVT_wings[(team * MAX_TVT_WINGS_PER_TEAM) + i], arr);
		}
	} else {
		for (i = 0; i < MAX_STARTING_WINGS; i++) {
			generate_weaponry_usage_list_wing(Starting_wings[i], arr);
		}
	}
}

void generate_weaponry_usage_list_wing(int wing_num, int* arr)
{
	int i, j;
	ship_weapon* swp;

	if (wing_num < 0) {
		return;
	}

	i = Wings[wing_num].wave_count;
	while (i--) {
		swp = &Ships[Wings[wing_num].ship_index[i]].weapons;
		j = swp->num_primary_banks;
		while (j--) {
			if (swp->primary_bank_weapons[j] >= 0 &&
				swp->primary_bank_weapons[j] < static_cast<int>(Weapon_info.size())) {
				arr[swp->primary_bank_weapons[j]]++;
			}
		}

		j = swp->num_secondary_banks;
		while (j--) {
			if (swp->secondary_bank_weapons[j] >= 0 &&
				swp->secondary_bank_weapons[j] < static_cast<int>(Weapon_info.size())) {
				arr[swp->secondary_bank_weapons[j]] +=
					(int)floor((swp->secondary_bank_ammo[j] * swp->secondary_bank_capacity[j] / 100.0f /
								   Weapon_info[swp->secondary_bank_weapons[j]].cargo_size) +
							   0.5f);
			}
		}
	}
}

void ensure_valid_player_start_shipnum()
{
	// nothing to do if the current player start is still valid
	if (Player_start_shipnum >= 0 && Player_start_shipnum < MAX_SHIPS
		&& Ships[Player_start_shipnum].objnum >= 0
		&& Objects[Ships[Player_start_shipnum].objnum].type == OBJ_START)
	{
		return;
	}

	// otherwise repoint to the first remaining player start, or -1 if there are none
	Player_start_shipnum = -1;
	for (auto *objp : list_range(&obj_used_list))
	{
		if (objp->type == OBJ_START)
		{
			Player_start_shipnum = objp->instance;
			break;
		}
	}
}

bool set_single_player_start(int objnum)
{
	if (objnum < 0 || objnum >= MAX_OBJECTS
		|| (Objects[objnum].type != OBJ_SHIP && Objects[objnum].type != OBJ_START))
	{
		Assertion(false, "set_single_player_start() called for object %d, which is not a ship", objnum);
		return false;
	}

	bool changed = false;

	for (auto *objp : list_range(&obj_used_list))
	{
		if (objp->type != OBJ_SHIP && objp->type != OBJ_START)
			continue;

		if (OBJ_INDEX(objp) == objnum)
		{
			// set as player ship
			if (objp->type != OBJ_START)
			{
				objp->type = OBJ_START;
				Player_starts++;
				changed = true;
			}
			objp->flags.set(Object::Object_Flags::Player_ship);
		}
		else
		{
			// set as regular ship
			if (objp->type == OBJ_START)
			{
				objp->type = OBJ_SHIP;
				Player_starts--;
				changed = true;
			}
			objp->flags.remove(Object::Object_Flags::Player_ship);
		}
	}

	ensure_valid_player_start_shipnum();

	return changed;
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

static bool wing_slot_is_empty(int i)
{
	return Wings[i].wave_count == 0;
}

static int find_free_slot(int max_slots, bool (*slot_is_empty)(int))
{
	for (int i = 0; i < max_slots; ++i)
	{
		if (slot_is_empty(i))
			return i;
	}
	return -1;
}

template <typename TConfig>
static void swap_slots(int a, int b, const TConfig& cfg, int max_slots,
	bool (*slot_is_empty)(int), void (*reassign)(int, int, const TConfig&, bool), const char* caller)
{
	if (a == b)
		return;

	Assertion(a >= 0 && a < max_slots, "%s: slot 'a' %d out of range", caller, a);
	Assertion(b >= 0 && b < max_slots, "%s: slot 'b' %d out of range", caller, b);
	Assertion(!slot_is_empty(a) && !slot_is_empty(b),
		"%s: both slots must be valid (a=%d, b=%d)", caller, a, b);

	// Find a free temporary slot.
	int tmp = find_free_slot(max_slots, slot_is_empty);
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
}

template <typename TConfig>
static void rotate_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const TConfig& cfg,
	int max_slots, bool (*slot_is_empty)(int), void (*reassign)(int, int, const TConfig&, bool), const char* caller)
{
	if (from_pos == to_pos)
		return;

	int count = (int)slots.size();
	Assertion(from_pos >= 0 && from_pos < count, "%s: 'from' position %d out of range", caller, from_pos);
	Assertion(to_pos >= 0 && to_pos < count, "%s: 'to' position %d out of range", caller, to_pos);

	// Find a free temporary slot.
	int tmp = find_free_slot(max_slots, slot_is_empty);
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
}

void swap_ship_slots(int a, int b, const FredShipSlotConfig& cfg)
{
	swap_slots(a, b, cfg, MAX_SHIPS, ship_slot_is_empty, reassign_ship_slot, "swap_ship_slots");
}

void rotate_ship_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredShipSlotConfig& cfg)
{
	rotate_slots(slots, from_pos, to_pos, cfg, MAX_SHIPS, ship_slot_is_empty, reassign_ship_slot, "rotate_ship_slots");
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

void swap_wing_slots(int a, int b, const FredWingSlotConfig& cfg)
{
	swap_slots(a, b, cfg, MAX_WINGS, wing_slot_is_empty, reassign_wing_slot, "swap_wing_slots");
}

void rotate_wing_slots(const SCP_vector<int>& slots, int from_pos, int to_pos, const FredWingSlotConfig& cfg)
{
	rotate_slots(slots, from_pos, to_pos, cfg, MAX_WINGS, wing_slot_is_empty, reassign_wing_slot, "rotate_wing_slots");
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

void resort_ships_in_obj_used_list()
{
	resort_obj_used_list_subset(
		[](int t) { return t == OBJ_SHIP || t == OBJ_START; },
		[](const object* o) { return o->instance; });
}
