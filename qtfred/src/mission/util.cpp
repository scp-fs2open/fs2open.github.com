//
//

#include <globalincs/pstypes.h>
#include <iff_defs/iff_defs.h>
#include <mission/missionparse.h>
#include <ship/ship.h>
#include <weapon/weapon.h>

#include "util.h"

// Goober5000
void stuff_special_arrival_anchor_name(char *buf, int iff_index, int restrict_to_players, int retail_format)
{
	const char *iff_name = Iff_info[iff_index].iff_name;

	// stupid retail hack
	if (retail_format && !stricmp(iff_name, "hostile") && !restrict_to_players)
		iff_name = "enemy";

	if (restrict_to_players)
		sprintf(buf, "<any %s player>", iff_name);
	else
		sprintf(buf, "<any %s>", iff_name);

	strlwr(buf);
}
void stuff_special_arrival_anchor_name(char* buf, int anchor_num, int retail_format) {
	// filter out iff
	int iff_index = anchor_num;
	iff_index &= ~SPECIAL_ARRIVAL_ANCHOR_FLAG;
	iff_index &= ~SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG;

	// filter players
	int restrict_to_players = (anchor_num & SPECIAL_ARRIVAL_ANCHOR_PLAYER_FLAG);

	// get name
	stuff_special_arrival_anchor_name(buf, iff_index, restrict_to_players, retail_format);
}

void generate_weaponry_usage_list_team(int team, int* arr) {
	int i;

	for (i = 0; i < MAX_WEAPON_TYPES; i++) {
		arr[i] = 0;
	}

	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		Assert (team >= 0 && team < MAX_TVT_TEAMS);

		for (i = 0; i < static_cast<int>(TVT_wings[team].size()); i++) {
			generate_weaponry_usage_list_wing(TVT_wings[team][i], arr);
		}
	} else {
		for (auto& index : Starting_wings) {
			generate_weaponry_usage_list_wing(index, arr);
		}
	}
}
void generate_weaponry_usage_list_wing(int wing_num, int* arr) {
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
			if (swp->primary_bank_weapons[j] >= 0 && swp->primary_bank_weapons[j] < static_cast<int>(Weapon_info.size())) {
				arr[swp->primary_bank_weapons[j]]++;
			}
		}

		j = swp->num_secondary_banks;
		while (j--) {
			if (swp->secondary_bank_weapons[j] >= 0 && swp->secondary_bank_weapons[j] < static_cast<int>(Weapon_info.size())) {
				arr[swp->secondary_bank_weapons[j]] += (int) floor(
					(swp->secondary_bank_ammo[j] * swp->secondary_bank_capacity[j] / 100.0f
						/ Weapon_info[swp->secondary_bank_weapons[j]].cargo_size) + 0.5f);
			}
		}
	}
}
