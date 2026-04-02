// methods and members common to any mission editor FSO may have
#include "common.h"
#include "mission/missionparse.h"
#include "iff_defs/iff_defs.h"
#include "ship/ship.h"

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

// TODO: The implementation here is a direct copy of what was in the original FRED code. It is not clear whether the
// original implementation was intentionally designed to produce the same output as the incorrect implementations in the
// two editors, or whether it was intended to be a correct normalization but was implemented incorrectly. If the former,
// then this implementation should be changed to produce the same output as the original implementations in the two
// editors, even if that output is mathematically incorrect. If the latter, then this implementation should be changed
// to produce correct normalization into [-180, 180] using a step of 360 instead of 180. This should be evaluated and
// decided upon before making any changes to this implementation.

// NOTE: FRED and QtFRED Relative coordinates used this version exactly while QtFRED Object orient editor used the
// correct 360 step version. This version was preserved here to avoid silent behavior changes to FRED during a routine 
// cleanup pass. This should be fixed intentionally in a dedicated pass once the impact on both editors has been evaluated.
float normalize_degrees(float deg)
{
	while (deg < -180.0f)
		deg += 180.0f; // was 360.0f in the qtfred object orient version, but this is the original FRED implementation
	while (deg > 180.0f)
		deg -= 180.0f; // was 360.0f in the qtfred object orient version, but this is the original FRED implementation
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
