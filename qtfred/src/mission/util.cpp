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

void generate_weaponry_usage_list_team(int team, int* arr) {
	int i;

	for (i = 0; i < MAX_WEAPON_TYPES; i++) {
		arr[i] = 0;
	}

	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		Assert (team >= 0 && team < MAX_TVT_TEAMS);

		for (i = 0; i < MAX_TVT_WINGS_PER_TEAM; i++) {
			generate_weaponry_usage_list_wing(TVT_wings[(team * MAX_TVT_WINGS_PER_TEAM) + i], arr);
		}
	} else {
		for (i = 0; i < MAX_STARTING_WINGS; i++) {
			generate_weaponry_usage_list_wing(Starting_wings[i], arr);
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

void time_to_mission_info_string(const std::tm* src, char* dest, size_t dest_max_len)
{
	std::strftime(dest, dest_max_len, "%x at %X", src);
}

bool rejectOrCloseHandler(__UNUSED QDialog* dialog,
	fso::fred::dialogs::AbstractDialogModel* model,
	fso::fred::EditorViewport* viewport)
{
	if (model->query_modified()) {
		auto button = viewport->dialogProvider->showButtonDialog(fso::fred::DialogType::Question,
			"Changes detected",
			"Do you want to keep your changes?",
			{fso::fred::DialogButton::Yes, fso::fred::DialogButton::No, fso::fred::DialogButton::Cancel});

		if (button == fso::fred::DialogButton::Cancel) {
			return false;
		}

		if (button == fso::fred::DialogButton::Yes) {
			return model->apply(); // only close if apply was successful
		}
		if (button == fso::fred::DialogButton::No) {
			model->reject();
			return true;
		}
		return false;
	} else {
		model->reject();
		return true;
	}
}