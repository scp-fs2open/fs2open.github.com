// Helpers for editor-side object duplication (Ctrl-drag, Clone Marked Objects).
// Shared between FRED and QtFRED.

#include "objectduplication.h"

#include "ai/ai.h"
#include "globalincs/linklist.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "parse/sexp.h"
#include "ship/ship.h"

// FRED/QtFRED each own their own copy of these arrays (defined in
// fred2/management.cpp and qtfred/src/mission/Editor.cpp)
extern char Fred_alt_names[MAX_SHIPS][NAME_LENGTH + 1];
extern char Fred_callsigns[MAX_SHIPS][NAME_LENGTH + 1];

// Per-ship field handling here MUST stay in sync with:
//   Fred_mission_save::save_objects() / save_common_object_data() in missionsave.cpp
//   parse_create_object_sub() in missionparse.cpp
// Field order below mirrors save_objects() so the two can be diffed.
void clone_ship_instance_data(int src_shipnum, int dest_shipnum)
{
	Assertion(src_shipnum >= 0 && src_shipnum < MAX_SHIPS, "clone_ship_instance_data: bad src %d", src_shipnum);
	Assertion(dest_shipnum >= 0 && dest_shipnum < MAX_SHIPS, "clone_ship_instance_data: bad dest %d", dest_shipnum);
	Assertion(src_shipnum != dest_shipnum, "clone_ship_instance_data: src and dest are the same ship");

	ship* src = &Ships[src_shipnum];
	ship* dest = &Ships[dest_shipnum];
	Assertion(src->ship_info_index == dest->ship_info_index, "clone_ship_instance_data: dest must be the same ship class as src");

	object* src_obj = &Objects[src->objnum];
	object* dest_obj = &Objects[dest->objnum];
	ai_info* src_ai = &Ai_info[src->ai_index];
	ai_info* dest_ai = &Ai_info[dest->ai_index];

	// display name (the Has_display_name flag is carried via the bitset copy below)
	dest->display_name = src->display_name;

	// alt classes
	dest->s_alt_classes = src->s_alt_classes;

	// alt name / callsign
	strcpy_s(Fred_alt_names[dest_shipnum], Fred_alt_names[src_shipnum]);
	strcpy_s(Fred_callsigns[dest_shipnum], Fred_callsigns[src_shipnum]);
	dest->alt_type_index = src->alt_type_index;
	dest->callsign_index = src->callsign_index;

	// team / team color
	dest->team = src->team;
	dest->team_name = src->team_name;
	dest->ship_iff_color = src->ship_iff_color;

	// AI class (lives both on the AI and mirrored on the ship's weapons struct;
	// the weapons.ai_class part is included in the full weapons-struct copy below)
	dest_ai->ai_class = src_ai->ai_class;

	// AI goals
	for (int i = 0; i < MAX_AI_GOALS; i++) {
		dest_ai->goals[i] = src_ai->goals[i];
	}

	// cargo
	dest->cargo1 = src->cargo1;
	strcpy_s(dest->cargo_title, src->cargo_title);

	// ===== begin save_common_object_data mirror =====

	// initial velocity / hull / shields
	dest_obj->phys_info.speed = src_obj->phys_info.speed;
	dest_obj->phys_info.fspeed = src_obj->phys_info.fspeed;
	dest_obj->hull_strength = src_obj->hull_strength;
	dest_obj->shield_quadrant = src_obj->shield_quadrant;

	// weapons (banks, ammo, ai_class... full struct, the assignment is a value copy)
	dest->weapons = src->weapons;

	// subsystems: walk the lists pairwise.  current_hits and max_hits both
	// matter so that special-hitpoint scaling carries over; subsystem cargo
	// and per-turret weapon loadout / ai_class are also editor-editable.
	{
		ship_subsys* sp_src = GET_FIRST(&src->subsys_list);
		ship_subsys* sp_dest = GET_FIRST(&dest->subsys_list);
		while (sp_dest != END_OF_LIST(&dest->subsys_list) && sp_src != END_OF_LIST(&src->subsys_list)) {
			sp_dest->current_hits = sp_src->current_hits;
			sp_dest->max_hits = sp_src->max_hits;
			sp_dest->subsys_cargo_name = sp_src->subsys_cargo_name;
			strcpy_s(sp_dest->subsys_cargo_title, sp_src->subsys_cargo_title);

			if (sp_dest->system_info && sp_dest->system_info->type == SUBSYSTEM_TURRET) {
				sp_dest->weapons = sp_src->weapons;
			}

			sp_src = GET_NEXT(sp_src);
			sp_dest = GET_NEXT(sp_dest);
		}
	}

	// ===== end save_common_object_data mirror =====

	// arrival
	dest->arrival_location = src->arrival_location;
	dest->arrival_distance = src->arrival_distance;
	dest->arrival_anchor = src->arrival_anchor;
	dest->arrival_path_mask = src->arrival_path_mask;
	dest->arrival_delay = src->arrival_delay;
	dest->arrival_cue = dup_sexp_chain(src->arrival_cue);

	// departure
	dest->departure_location = src->departure_location;
	dest->departure_anchor = src->departure_anchor;
	dest->departure_path_mask = src->departure_path_mask;
	dest->departure_delay = src->departure_delay;
	dest->departure_cue = dup_sexp_chain(src->departure_cue);

	// warp params
	dest->warpin_params_index = src->warpin_params_index;
	dest->warpout_params_index = src->warpout_params_index;

	// ship flags
	dest->flags = src->flags;
	dest->flags.remove(Ship::Ship_Flags::Dying);
	dest->flags.remove(Ship::Ship_Flags::Disabled);
	dest->flags.remove(Ship::Ship_Flags::Depart_warp);
	dest->flags.remove(Ship::Ship_Flags::Depart_dockbay);

	// object flags (Explicit list of the editor-editable bits from save_objects)
	const Object::Object_Flags editor_object_flags[] = {
		Object::Object_Flags::Protected,
		Object::Object_Flags::No_shields,
		Object::Object_Flags::Invulnerable,
		Object::Object_Flags::Beam_protected,
		Object::Object_Flags::Flak_protected,
		Object::Object_Flags::Laser_protected,
		Object::Object_Flags::Missile_protected,
		Object::Object_Flags::Special_warpin,
		Object::Object_Flags::Targetable_as_bomb,
		Object::Object_Flags::Dont_change_position,
		Object::Object_Flags::Dont_change_orientation,
		Object::Object_Flags::Collides,
		Object::Object_Flags::Attackable_if_no_collide,
	};
	for (auto flag : editor_object_flags) {
		if (src_obj->flags[flag]) {
			dest_obj->flags.set(flag);
		} else {
			dest_obj->flags.remove(flag);
		}
	}

	// AI flags
	dest_ai->ai_flags = src_ai->ai_flags;
	dest_ai->kamikaze_damage = src_ai->kamikaze_damage;

	// respawn / escort / guardian
	dest->respawn_priority = src->respawn_priority;
	dest->escort_priority = src->escort_priority;
	dest->ship_guardian_threshold = src->ship_guardian_threshold;

	// special explosion
	dest->use_special_explosion = src->use_special_explosion;
	dest->special_exp_damage = src->special_exp_damage;
	dest->special_exp_blast = src->special_exp_blast;
	dest->special_exp_inner = src->special_exp_inner;
	dest->special_exp_outer = src->special_exp_outer;
	dest->use_shockwave = src->use_shockwave;
	dest->special_exp_shockwave_speed = src->special_exp_shockwave_speed;
	dest->special_exp_deathroll_time = src->special_exp_deathroll_time;

	// special hitpoints / shield (and the derived max strengths the parse
	// path sets at lines 2321-2326 of missionparse.cpp)
	dest->special_hitpoints = src->special_hitpoints;
	dest->special_shield = src->special_shield;
	dest->ship_max_hull_strength = src->ship_max_hull_strength;
	dest->ship_max_shield_strength = src->ship_max_shield_strength;
	dest->max_shield_recharge = src->max_shield_recharge;
	if (dest->special_hitpoints > 0) {
		ship_recalc_subsys_strength(dest);
	}

	// hotkey
	dest->hotkey = src->hotkey;

	// destroy-at (Kill_before_mission flag already carried via shipp->flags)
	dest->final_death_time = src->final_death_time;

	// orders
	dest->orders_accepted = src->orders_accepted;

	// group / layer
	dest->group = src->group;
	dest->fred_layer = src->fred_layer;

	// scoring / persona
	dest->score = src->score;
	dest->assist_score_pct = src->assist_score_pct;
	dest->persona_index = src->persona_index;

	// texture replacement... copy any entries whose ship_name matches src
	{
		SCP_vector<texture_replace> new_entries;
		for (const auto& tr : Fred_texture_replacements) {
			if (!stricmp(tr.ship_name, src->ship_name) && !(tr.from_table)) {
				texture_replace copy = tr;
				strcpy_s(copy.ship_name, dest->ship_name);
				new_entries.push_back(copy);
			}
		}
		for (auto& entry : new_entries) {
			Fred_texture_replacements.push_back(std::move(entry));
		}
	}
}
