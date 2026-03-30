// methods and members common to any mission editor FSO may have
#include "common.h"
#include "globalincs/linklist.h"
#include "mission/missionparse.h"
#include "iff_defs/iff_defs.h"
#include "object/object.h"
#include "ship/ship.h"
#include "jumpnode/jumpnode.h"

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

SCP_string check_name_conflict(const char *entity_type, const char *name, int exclude_ship, int exclude_wing, int exclude_waypoint_list, int exclude_jump_node)
{
	SCP_string msg;

	// Name must not be empty
	if (name[0] == '\0') {
		msg += "This ";
		msg += entity_type;
		msg += " name cannot be empty";
		return msg;
	}

	// Check wings
	for (int i = 0; i < MAX_WINGS; i++) {
		if (Wings[i].wave_count && i != exclude_wing && !stricmp(Wings[i].name, name)) {
			msg += "This ";
			msg += entity_type;
			msg += " name is already being used by ";
			msg += (exclude_wing >= 0) ? "another wing" : "a wing";
			return msg;
		}
	}

	// Check ships
	for (auto ptr: list_range(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP || ptr->type == OBJ_START) && ptr->instance != exclude_ship) {
			if (!stricmp(Ships[ptr->instance].ship_name, name)) {
				msg += "This ";
				msg += entity_type;
				msg += " name is already being used by ";
				msg += (exclude_ship >= 0) ? "another ship" : "a ship";
				return msg;
			}
		}
	}

	// We don't need to check teams.  "Unknown" is a valid name and also an IFF.

	// Check target priority groups
	for (const auto &ai_tp: Ai_tp_list) {
		if (!stricmp(ai_tp.name, name)) {
			msg += "This ";
			msg += entity_type;
			msg += " name is already being used by a target priority group";
			return msg;
		}
	}

	// Check waypoint lists
	int wl_size = sz2i(Waypoint_lists.size());
	for (int i = 0; i < wl_size; i++) {
		if (i != exclude_waypoint_list && !stricmp(Waypoint_lists[i].get_name(), name)) {
			msg += "This ";
			msg += entity_type;
			msg += " name is already being used by ";
			msg += (exclude_waypoint_list >= 0) ? "another waypoint path" : "a waypoint path";
			return msg;
		}
	}

	// Check jump nodes
	int jn_size = sz2i(Jump_nodes.size());
	for (int i = 0; i < jn_size; i++) {
		if (i != exclude_jump_node && !stricmp(Jump_nodes[i].GetName(), name)) {
			msg += "This ";
			msg += entity_type;
			msg += " name is already being used by ";
			msg += (exclude_jump_node >= 0) ? "another jump node" : "a jump node";
			return msg;
		}
	}

	// Name must not start with '<' (used for invalidated SEXP references)
	if (name[0] == '<') {
		msg += "This ";
		msg += entity_type;
		msg += " name is not allowed to begin with <";
		return msg;
	}

	return "";	// no error
}
