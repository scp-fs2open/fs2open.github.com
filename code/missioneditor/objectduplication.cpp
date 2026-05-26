// Helpers for editor-side object duplication (Ctrl-drag, Clone Marked Objects).
// Shared between FRED and QtFRED.

#include "objectduplication.h"

#include "ai/ai.h"
#include "globalincs/linklist.h"
#include "jumpnode/jumpnode.h"
#include "mission/missionparse.h"
#include "model/model.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "parse/sexp.h"
#include "prop/prop.h"
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

	// display name (the Has_display_name flag is NOT in Parse_ship_flags, so the
	// flag-copy loop below doesn't carry it - mirror it here alongside the string).
	dest->display_name = src->display_name;
	if (src->flags[Ship::Ship_Flags::Has_display_name]) {
		dest->flags.set(Ship::Ship_Flags::Has_display_name);
	} else {
		dest->flags.remove(Ship::Ship_Flags::Has_display_name);
	}

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

	// arrival / departure - skip entirely for player starts: create_player has
	// already locked arrival_cue to Locked_sexp_true and departure_cue to
	// Locked_sexp_false, and player starts ignore the other arrival/departure
	// fields.  Overwriting the cues here would break the locked-cue invariant.
	if (!dest_obj->flags[Object::Object_Flags::Player_ship]) {
		dest->arrival_location = src->arrival_location;
		dest->arrival_distance = src->arrival_distance;
		dest->arrival_anchor = src->arrival_anchor;
		dest->arrival_path_mask = src->arrival_path_mask;
		dest->arrival_delay = src->arrival_delay;
		dest->arrival_cue = dup_sexp_chain(src->arrival_cue);

		dest->departure_location = src->departure_location;
		dest->departure_anchor = src->departure_anchor;
		dest->departure_path_mask = src->departure_path_mask;
		dest->departure_delay = src->departure_delay;
		dest->departure_cue = dup_sexp_chain(src->departure_cue);
	}

	// warp params
	dest->warpin_params_index = src->warpin_params_index;
	dest->warpout_params_index = src->warpout_params_index;

	// ship flags - driven by Parse_ship_flags so duplication stays in sync with
	// parse/save automatically when new editor-editable ship flags are added.
	// Runtime/transient flags (Dying, Depart_warp, From_player_wing, ...) and
	// instance-coupled flags (Dock_leader) are not in Parse_ship_flags and so
	// are correctly NOT copied here.
	for (size_t i = 0; i < Num_Parse_ship_flags; ++i) {
		const auto flag = Parse_ship_flags[i].def;
		if (src->flags[flag]) {
			dest->flags.set(flag);
		} else {
			dest->flags.remove(flag);
		}
	}

	// object flags - driven by Parse_ship_object_flags.  Skip Player_ship: that
	// bit is managed by create_player/create_ship at the caller and copying it
	// would either redundantly re-set it (player-start -> player-start) or
	// wrongly promote a regular-ship dup to a player ship (demoted dup case).
	for (size_t i = 0; i < Num_Parse_ship_object_flags; ++i) {
		const auto flag = Parse_ship_object_flags[i].def;
		if (flag == Object::Object_Flags::Player_ship) {
			continue;
		}
		if (src_obj->flags[flag]) {
			dest_obj->flags.set(flag);
		} else {
			dest_obj->flags.remove(flag);
		}
	}

	// AI flags - driven by Parse_ship_ai_flags so duplication stays in sync with
	// parse/save automatically when new editor-editable AI flags are added.
	for (size_t i = 0; i < Num_Parse_ship_ai_flags; ++i) {
		const auto flag = Parse_ship_ai_flags[i].def;
		if (src_ai->ai_flags[flag]) {
			dest_ai->ai_flags.set(flag);
		} else {
			dest_ai->ai_flags.remove(flag);
		}
	}
	// kamikaze_damage is editor-editable independent of the Kamikaze flag (see
	// ShipFlagsDialogModel), so copy unconditionally.
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

	// texture replacement... copy any entries whose ship_name matches src.
	// Skip from_table entries: those came from the ship class's .tbl/.tbm and
	// apply to every instance of the class automatically, so re-attaching one
	// to the duplicate would create a duplicate override that save_objects
	// wouldn't write anyway.
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

// Per-prop field handling here MUST stay in sync with:
//   Fred_mission_save::save_props() in missionsave.cpp
//   parse_prop() in missionparse.cpp
void clone_prop_instance_data(int src_prop_id, int dest_prop_id)
{
	Assertion(src_prop_id != dest_prop_id, "clone_prop_instance_data: src and dest are the same prop");

	prop* src = prop_id_lookup(src_prop_id);
	prop* dest = prop_id_lookup(dest_prop_id);
	Assertion(src && dest, "clone_prop_instance_data: invalid src or dest prop id");
	Assertion(src->prop_info_index == dest->prop_info_index, "clone_prop_instance_data: dest must be the same prop class as src");

	// fred view layer
	dest->fred_layer = src->fred_layer;

	// object flag the prop editor exposes (see save_props)
	if (Objects[src->objnum].flags[Object::Object_Flags::Collides]) {
		Objects[dest->objnum].flags.set(Object::Object_Flags::Collides);
	} else {
		Objects[dest->objnum].flags.remove(Object::Object_Flags::Collides);
	}
}

// Per-jump-node field handling here MUST stay in sync with:
//   Fred_mission_save::save_waypoints() jump-node section in missionsave.cpp
//   the "$Jump Node:" parse block in missionparse.cpp
void clone_jump_node_instance_data(const CJumpNode& src, CJumpNode& dest)
{
	// display name (the JN_HAS_DISPLAY_NAME flag is set by SetDisplayName)
	if (src.HasDisplayName()) {
		dest.SetDisplayName(src.GetDisplayName());
	}

	// special model - SetModel also sets m_radius and the JN_SPECIAL_MODEL flag.
	// Parse uses default show_polys=false; JN_SHOW_POLYS isn't persisted, so we
	// don't carry it across either.
	if (src.IsSpecialModel()) {
		polymodel* pm = model_get(src.GetModelNumber());
		if (pm) {
			dest.SetModel(pm->filename);
		}
	}

	// color (SetAlphaColor manages the JN_USE_DISPLAY_COLOR flag)
	if (src.IsColored()) {
		const color& c = src.GetColor();
		dest.SetAlphaColor(c.red, c.green, c.blue, c.alpha);
	}

	// hidden state
	dest.SetVisibility(!src.IsHidden());

	// fred view layer
	dest.SetFredLayer(src.GetFredLayer());
}

// Per-path field handling here MUST stay in sync with:
//   Fred_mission_save::save_waypoints() (waypoint-list section) in missionsave.cpp
//   parse_waypoint_list() in missionparse.cpp
void clone_waypoint_path_instance_data(int src_list_index, int dest_list_index)
{
	waypoint_list* src = find_waypoint_list_at_index(src_list_index);
	waypoint_list* dest = find_waypoint_list_at_index(dest_list_index);
	if (src == nullptr || dest == nullptr || src == dest) {
		return;
	}

	dest->set_no_draw_lines(src->get_no_draw_lines());
	if (src->get_has_custom_color()) {
		dest->set_color(src->get_color_r(), src->get_color_g(), src->get_color_b());
	} else {
		dest->clear_color();
	}
	dest->set_fred_layer(src->get_fred_layer());
}
