#include "missioneditor/sexp_tree_opf.h"
#include "missioneditor/sexp_tree_model.h"

#include "parse/sexp.h"
#include "globalincs/linklist.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "ship/ship.h"
#include "prop/prop.h"
#include "iff_defs/iff_defs.h"
#include "ai/ai.h"
#include "ai/aigoals.h"
#include "hud/hudartillery.h"
#include "gamesnd/eventmusic.h"
#include "mission/missionparse.h"
#include "mission/missionmessage.h"
#include "missioneditor/common.h"
#include "model/model.h"
#include "sound/ds.h"
#include "hud/hud.h"
#include "graphics/software/FontManager.h"
#include "hud/hudsquadmsg.h"
#include "controlconfig/controlsconfig.h"
#include "mission/missiongoals.h"
#include "mission/missioncampaign.h"
#include "stats/medals.h"
#include "menuui/techmenu.h"
#include "weapon/weapon.h"
#include "jumpnode/jumpnode.h"
#include "starfield/starfield.h"
#include "nebula/neblightning.h"
#include "nebula/neb.h"
#include "graphics/2d.h"
#include "model/animation/modelanimation.h"
#include "globalincs/alphacolors.h"
#include "asteroid/asteroid.h"
#include "fireball/fireballs.h"
#include "species_defs/species_defs.h"
#include "localization/localize.h"
#include "gamesnd/gamesnd.h"
#include "parse/sexp/sexp_lookup.h"
#include "ai/ailua.h"
#include "stats/scoring.h"
#include "parse/sexp_container.h"
#include "weapon/emp.h"

// OPF listing functions for the sexp tree.
// All get_listing_opf_*() functions are implemented here as SexpTreeOPF methods.
// Uses _model back-reference for read-only access to tree_nodes[] and _interface.

SexpTreeOPF::SexpTreeOPF(SexpTreeModel& model) : _model(model) {}

sexp_list_item *SexpTreeOPF::get_listing_opf_null()
{
	sexp_list_item head;

	for (int i=0; i<static_cast<int>(Operators.size()); i++)
		if (query_operator_return_type(i) == OPR_NULL)
			head.add_op(i);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_flexible_argument()
{
	sexp_list_item head;

	for (int i=0; i<static_cast<int>(Operators.size()); i++)
		if (query_operator_return_type(i) == OPR_FLEXIBLE_ARGUMENT)
			head.add_op(i);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_bool(int parent_node) const
{
	sexp_list_item head;

	// search for the previous goal/event operators.  If found, only add the true/false
	// sexpressions to the list
	bool only_basic = false;
	if ( parent_node != -1 ) {
		int op = get_operator_const(_model.tree_nodes[parent_node].text);
		if ( (op == OP_PREVIOUS_GOAL_TRUE) || (op == OP_PREVIOUS_GOAL_FALSE) || (op == OP_PREVIOUS_EVENT_TRUE) || (op == OP_PREVIOUS_EVENT_FALSE) )
			only_basic = true;
	}

	for (int i=0; i<static_cast<int>(Operators.size()); i++) {
		if (query_operator_return_type(i) == OPR_BOOL) {
			if ( !only_basic || (only_basic && ((Operators[i].value == OP_TRUE) || (Operators[i].value == OP_FALSE))) ) {
				head.add_op(i);
			}
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_positive()
{
	sexp_list_item head;

	for (int i=0; i<static_cast<int>(Operators.size()); i++) {
		int z = query_operator_return_type(i);
		// Goober5000's number hack
		if ((z == OPR_NUMBER) || (z == OPR_POSITIVE))
			head.add_op(i);
	}

	return head.next;
}

// This intentionally returns the same set of operators as get_listing_opf_positive():
// per "Goober5000's number hack" (see that function), OPR_NUMBER and OPR_POSITIVE
// operators are interchangeable in the menu. Entry of negative literals in positive
// slots is rejected separately at data-entry time (Mantis 1813 fix).
sexp_list_item *SexpTreeOPF::get_listing_opf_number()
{
	sexp_list_item head;

	for (int i=0; i<static_cast<int>(Operators.size()); i++) {
		int z = query_operator_return_type(i);
		if ((z == OPR_NUMBER) || (z == OPR_POSITIVE))
			head.add_op(i);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship(int parent_node) const
{
	sexp_list_item head;

	// look at the parent node and get the operator. Some ship lists should be
	// filtered based on what the parent operator is. op stays 0 when there's
	// no parent, which falls through to the unfiltered branch below.
	int op = 0;
	int dock_ship = -1;
	if ( parent_node >= 0 ) {
		op = get_operator_const(_model.tree_nodes[parent_node].text);

		// for ai-dock goals, look up the ship we're docking with so we can
		// prune the listing to ships it can actually dock with
		if ( op == OP_AI_DOCK ) {
			int z = _model.tree_nodes[parent_node].parent;
			Assertion(z >= 0, "Invalid parent node");
			Assertion(!stricmp(_model.tree_nodes[z].text, "add-ship-goal") || !stricmp(_model.tree_nodes[z].text, "add-wing-goal") || !stricmp(_model.tree_nodes[z].text, "add-goal"), "Invalid parent node type");

			z = _model.tree_nodes[z].child;
			Assertion(z >= 0, "Invalid child node");

			dock_ship = ship_name_lookup(_model.tree_nodes[z].text, 1);
			Assertion(dock_ship != -1, "Invalid dock ship");
		}
	}

	for (object* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if ((ptr->type != OBJ_SHIP) && (ptr->type != OBJ_START))
			continue;

		if ( op == OP_AI_DOCK ) {
			// only include those ships in the list which the given ship can dock with.
			if ( (dock_ship != ptr->instance) && ship_docking_valid(dock_ship , ptr->instance) )
				head.add_data(Ships[ptr->instance].ship_name );
		}
		else if (op == OP_CAP_SUBSYS_CARGO_KNOWN_DELAY) {
			if ( ((Ship_info[Ships[ptr->instance].ship_info_index].is_huge_ship()) &&	// big ship
				!(Ships[ptr->instance].flags[Ship::Ship_Flags::Toggle_subsystem_scanning]) )||				// which is not flagged OR
				((!(Ship_info[Ships[ptr->instance].ship_info_index].is_huge_ship())) &&  // small ship
				(Ships[ptr->instance].flags[Ship::Ship_Flags::Toggle_subsystem_scanning]) ) ) {				// which is flagged

					head.add_data(Ships[ptr->instance].ship_name);
			}
		}
		else {
			head.add_data(Ships[ptr->instance].ship_name);
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_prop()
{
	sexp_list_item head;

	object* ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_PROP) {
			auto p = prop_id_lookup(ptr->instance);
			if (p != nullptr) {
				head.add_data(p->prop_name);
			}
		}

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_wing()
{
	sexp_list_item head;

	for (const auto& w : Wings) {
		if (w.wave_count) {
			head.add_data(w.name);
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_point()
{
	sexp_list_item head;

	for (const auto &ii: Waypoint_lists)
	{
		for (const auto &jj: ii.get_waypoints())
		{
			char buf[NAME_LENGTH];
			waypoint_stuff_name(buf, jj);
			head.add_data(buf);
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_iff()
{
	sexp_list_item head;

	for (const auto& ii : Iff_info) {
		head.add_data(ii.iff_name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ai_class()
{
	sexp_list_item head;

	for (int i=0; i<Num_ai_classes; i++)
		head.add_data(Ai_class_names[i]);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_support_ship_class()
{
	sexp_list_item head;

	head.add_data("<species support ship class>");

    for (const auto& ship_info : Ship_info) {
		if (ship_info.flags[Ship::Info_Flags::Support]) {
			head.add_data(ship_info.name);
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ssm_class()
{
	sexp_list_item head;

	for (const auto& ssmp : Ssm_info) {
		head.add_data(ssmp.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_with_bay()
{
	sexp_list_item head;

	head.add_data("<no anchor>");

	object* objp;
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
	{
		if ( (objp->type == OBJ_SHIP) || (objp->type == OBJ_START) )
		{
			// determine if this ship has a hangar bay
			if (ship_has_hangar_bay(objp->instance))
			{
				head.add_data(Ships[objp->instance].ship_name);
			}
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_soundtrack_name()
{
	sexp_list_item head;

	head.add_data("<No Music>");

	for (auto &st: Soundtracks)
		head.add_data(st.name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_arrival_location()
{
	sexp_list_item head;

	for (const auto& name : Arrival_location_names)
		head.add_data(name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_departure_location()
{
	sexp_list_item head;

	for (const auto& name : Departure_location_names)
		head.add_data(name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_arrival_anchor_all()
{
	sexp_list_item head;

	for (int restrict_to_players = 0; restrict_to_players < 2; restrict_to_players++)
	{
		for (int i = 0; i < static_cast<int>(Iff_info.size()); i++)
		{
			char tmp[NAME_LENGTH + 15];
			stuff_special_arrival_anchor_name(tmp, i, restrict_to_players, false);

			head.add_data(tmp);
		}
	}

	object* objp;
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
	{
		if ( (objp->type == OBJ_SHIP) || (objp->type == OBJ_START) )
		{
			head.add_data(Ships[objp->instance].ship_name);
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ai_goal(int parent_node) const
{
	sexp_list_item head;

	Assertion(parent_node >= 0, "Invalid parent node");
	int child = _model.tree_nodes[parent_node].child;
	if (child < 0)
		return nullptr;

	int n = ship_name_lookup(_model.tree_nodes[child].text, 1);
	if (n >= 0) {
		// add operators if it's an ai-goal and ai-goal is allowed for that ship
		for (int i=0; i<static_cast<int>(Operators.size()); i++) {
			if ( (query_operator_return_type(i) == OPR_AI_GOAL) && query_sexp_ai_goal_valid(Operators[i].value, n) )
				head.add_op(i);
		}

	} else {
		int z = wing_name_lookup(_model.tree_nodes[child].text);
		if (z >= 0) {
			for (int w=0; w<Wings[z].wave_count; w++) {
				n = Wings[z].ship_index[w];
				// add operators if it's an ai-goal and ai-goal is allowed for that ship
				for (int i=0; i<static_cast<int>(Operators.size()); i++) {
					if ( (query_operator_return_type(i) == OPR_AI_GOAL) && query_sexp_ai_goal_valid(Operators[i].value, n) )
						head.add_op(i);
				}
			}
		// when dealing with the special argument add them all. It's up to the FREDder to ensure invalid orders aren't given
		} else if (!strcmp(_model.tree_nodes[child].text, SEXP_ARGUMENT_STRING)) {
			for (int i=0; i<static_cast<int>(Operators.size()); i++) {
				if (query_operator_return_type(i) == OPR_AI_GOAL) {
					head.add_op(i);
				}
			}
		} else
			return nullptr;  // no valid ship or wing to check against, make nothing available
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_message() const
{
	sexp_list_item head;

	for (auto& msg : _model._interface->getMessages()) {
		head.add_data(msg.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_docker_point(int parent_node, int arg_num) const
{
	sexp_list_item head;

	Assertion(parent_node >= 0, "Invalid parent node");
	Assertion(!stricmp(_model.tree_nodes[parent_node].text, "ai-dock") || !stricmp(_model.tree_nodes[parent_node].text, "set-docked") ||
		   get_operator_const(_model.tree_nodes[parent_node].text) >= static_cast<int>(First_available_operator_id), "Invalid node type");

	int sh = -1;
	if (!stricmp(_model.tree_nodes[parent_node].text, "ai-dock"))
	{
		int z = _model.tree_nodes[parent_node].parent;
		if (z < 0)
			return nullptr;
		Assertion(!stricmp(_model.tree_nodes[z].text, "add-ship-goal") || !stricmp(_model.tree_nodes[z].text, "add-wing-goal") || !stricmp(_model.tree_nodes[z].text, "add-goal"), "Invalid parent node type");

		z = _model.tree_nodes[z].child;
		if (z < 0)
			return nullptr;
		sh = ship_name_lookup(_model.tree_nodes[z].text, 1);
	}
	else if (!stricmp(_model.tree_nodes[parent_node].text, "set-docked"))
	{
		//Docker ship should be the first child node
		int z = _model.tree_nodes[parent_node].child;
		if (z < 0)
			return nullptr;
		sh = ship_name_lookup(_model.tree_nodes[z].text, 1);
	}
	// for Lua sexps
	else if (get_operator_const(_model.tree_nodes[parent_node].text) >= static_cast<int>(First_available_operator_id))
	{
		int this_index = get_dynamic_parameter_index(_model.tree_nodes[parent_node].text, arg_num);

		if (this_index >= 0) {
			int z = _model.tree_nodes[parent_node].child;

			for (int j = 0; j < this_index; j++) {
				z = _model.tree_nodes[z].next;
			}

			sh = ship_name_lookup(_model.tree_nodes[z].text, 1);
		} else {
			error_display(1, "Expected to find a dynamic lua parent parameter for node %i in operator %s but found nothing!",
				arg_num,
				_model.tree_nodes[parent_node].text);
		}
	}

	if (sh >= 0)
	{
		auto model_num = Ship_info[Ships[sh].ship_info_index].model_num;
		if (model_num >= 0) {
			polymodel* pm = model_get(model_num);
			if (pm != nullptr) {
				for (int i = 0; i < pm->n_docks; i++) {
					head.add_data(pm->docking_bays[i].name);
				}
			}
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_dockee_point(int parent_node) const
{
	sexp_list_item head;

	Assertion(parent_node >= 0, "Invalid parent node");
	Assertion(!stricmp(_model.tree_nodes[parent_node].text, "ai-dock") || !stricmp(_model.tree_nodes[parent_node].text, "set-docked"), "Invalid node type");

	int sh = -1;
	if (!stricmp(_model.tree_nodes[parent_node].text, "ai-dock"))
	{
		int z = _model.tree_nodes[parent_node].child;
		if (z < 0)
			return nullptr;

		sh = ship_name_lookup(_model.tree_nodes[z].text, 1);
	}
	else if (!stricmp(_model.tree_nodes[parent_node].text, "set-docked"))
	{
		//Dockee ship should be the third child node
		int z = _model.tree_nodes[parent_node].child;	// 1
		if (z < 0) return nullptr;
		z = _model.tree_nodes[z].next;				// 2
		if (z < 0) return nullptr;
		z = _model.tree_nodes[z].next;				// 3
		if (z < 0) return nullptr;

		sh = ship_name_lookup(_model.tree_nodes[z].text, 1);
	}

	if (sh >= 0)
	{
		auto model_num = Ship_info[Ships[sh].ship_info_index].model_num;
		if (model_num >= 0) {
			polymodel* pm = model_get(model_num);
			if (pm != nullptr) {
				for (int i = 0; i < pm->n_docks; i++) {
					head.add_data(pm->docking_bays[i].name);
				}
			}
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_persona()
{
	sexp_list_item head;

	for (const auto &persona: Personas) {
		if (persona.flags & PERSONA_FLAG_WINGMAN) {
			head.add_data(persona.name);
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_font()
{
	sexp_list_item head;

	for (int i = 0; i < font::FontManager::numberOfFonts(); i++) {
		head.add_data(font::FontManager::getFont(i)->getName().c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_who_from()
{
	sexp_list_item head;

	//head.add_data("<any allied>");
	head.add_data("#Command");
	head.add_data("<any wingman>");
	head.add_data("<none>");

	object* ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START))
			if (Ship_info[Ships[ptr->instance].ship_info_index].is_flyable())
				head.add_data(Ships[ptr->instance].ship_name);

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_priority()
{
	sexp_list_item head;

	head.add_data("High");
	head.add_data("Normal");
	head.add_data("Low");
	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_sound_environment()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	for (const auto& efx_preset : EFX_presets) {
		head.add_data(efx_preset.name.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_sound_environment_option()
{
	sexp_list_item head;

	for (int i = 0; i < Num_sound_environment_options; i++)
		head.add_data(Sound_environment_option[i]);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_adjust_audio_volume()
{
	sexp_list_item head;

	for (int i = 0; i < Num_adjust_audio_options; i++)
		head.add_data(Adjust_audio_options[i]);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_builtin_hud_gauge()
{
	sexp_list_item head;

	for (int i = 0; i < Num_hud_gauge_types; i++)
		head.add_data(Hud_gauge_types[i].name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_custom_hud_gauge()
{
	sexp_list_item head;
	// prevent duplicate names, comparing case-insensitively
	SCP_unordered_set<SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to> all_gauges;

	for (auto &gauge : default_hud_gauges)
	{
		SCP_string name = gauge->getCustomGaugeName();
		if (!name.empty() && all_gauges.count(name) == 0)
		{
			head.add_data(name.c_str());
			all_gauges.insert(std::move(name));
		}
	}

	for (auto &si : Ship_info)
	{
		for (auto &gauge : si.hud_gauges)
		{
			SCP_string name = gauge->getCustomGaugeName();
			if (!name.empty() && all_gauges.count(name) == 0)
			{
				head.add_data(name.c_str());
				all_gauges.insert(std::move(name));
			}
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_any_hud_gauge()
{
	sexp_list_item head;

	head.add_list(get_listing_opf_builtin_hud_gauge());
	head.add_list(get_listing_opf_custom_hud_gauge());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_effect()
{
	sexp_list_item head;

	for (const auto& sp_effect : Ship_effects) {
		head.add_data(sp_effect.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_explosion_option()
{
	sexp_list_item head;

	for (int i = 0; i < Num_explosion_options; i++)
		head.add_data(Explosion_option[i]);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_waypoint_path()
{
	sexp_list_item head;

	for (const auto &ii: Waypoint_lists)
		head.add_data(ii.get_name());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_point() const
{
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_point());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_wing_wholeteam() const
{
	sexp_list_item head;

	for (const auto& ii : Iff_info) {
		head.add_data(ii.iff_name);
	}

	head.add_list(get_listing_opf_ship_wing());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_wing_shiponteam_point() const
{
	sexp_list_item head;

	for (const auto& ii : Iff_info) {
		char tmp[NAME_LENGTH + 7];
		sprintf(tmp, "<any %s>", ii.iff_name);
		strlwr(tmp);
		head.add_data(tmp);
	}

	head.add_list(get_listing_opf_ship_wing_point());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_wing_point() const
{
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());
	head.add_list(get_listing_opf_point());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_wing_point_or_none() const
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_ship_wing_point());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_mission_name() const
{
	sexp_list_item head;

	for (auto& mission_name : _model._interface->getMissionNames()) {
		head.add_data(mission_name.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_goal_name(int parent_node) const
{
	sexp_list_item head;

	Assertion(parent_node >= 0, "Invalid parent node");
	int child = _model.tree_nodes[parent_node].child;

	// reference_name is used by campaign editor to filter goals for a specific mission
	SCP_string reference_name = (child >= 0) ? _model.tree_nodes[child].text : "";

	for (auto& entry : _model._interface->getMissionGoals(reference_name)) {
		head.add_data(entry.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_wing() const
{
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_prop() const
{
	sexp_list_item head;

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_prop());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_order_recipient() const
{
	sexp_list_item head;

	head.add_data("<all fighters>");

	head.add_list(get_listing_opf_ship());
	head.add_list(get_listing_opf_wing());
	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_type()
{
	sexp_list_item head;

	for (const auto& st : Ship_types) {
		head.add_data(st.name);
	}
	if (Fighter_bomber_valid) {
		head.add_data(Fighter_bomber_type_name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_keypress()
{
	sexp_list_item head;
	const auto& Default_config = Control_config_presets[0].bindings;

	for (size_t i = 0; i < Control_config.size(); ++i) {
		auto btn = Default_config[i].get_btn(CID_KEYBOARD);

		if ((btn >= 0) && !Control_config[i].disabled) {
			head.add_data(textify_scancode_universal(btn));
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_event_name(int parent_node) const
{
	sexp_list_item head;

	Assertion(parent_node >= 0, "Invalid parent node");
	int child = _model.tree_nodes[parent_node].child;

	// reference_name is used by campaign editor to filter events for a specific mission
	SCP_string reference_name = (child >= 0) ? _model.tree_nodes[child].text : "";

	for (auto& entry : _model._interface->getMissionEvents(reference_name)) {
		head.add_data(entry.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ai_order()
{
	sexp_list_item head;

	for (const auto& order : Player_orders)
		head.add_data(order.hud_name.c_str());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_skill_level()
{
	sexp_list_item head;

	for (int i=0; i<NUM_SKILL_LEVELS; i++)
		head.add_data(Skill_level_names(i, 0));

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_cargo()
{
	sexp_list_item head;

	head.add_data("Nothing");
	for (int i=0; i<Num_cargo; i++)
	{
		if (stricmp(Cargo_names[i], "nothing"))
			head.add_data(Cargo_names[i]);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_string()
{
	sexp_list_item head;

	head.add_data(SEXP_ANY_STRING);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_medal_name()
{
	sexp_list_item head;

	for (int i = 0; i < static_cast<int>(Medals.size()); i++)
	{
		// don't add Rank or the Ace badges
		if ((i == Rank_medal_index) || (Medals[i].kills_needed > 0))
			continue;
		head.add_data(Medals[i].name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_weapon_name()
{
	sexp_list_item head;

	for (auto &wi : Weapon_info)
		head.add_data(wi.name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_intel_name()
{
	sexp_list_item head;

	for (auto &ii : Intel_info)
		head.add_data(ii.name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_class_name()
{
	sexp_list_item head;

	for (auto &si : Ship_info)
		head.add_data(si.name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_prop_class_name()
{
	sexp_list_item head;

	for (auto& pi : Prop_info)
		head.add_data(pi.name.c_str());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_huge_weapon()
{
	sexp_list_item head;

	for (auto &wi : Weapon_info) {
		if (wi.wi_flags[Weapon::Info_Flags::Huge])
			head.add_data(wi.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_not_player()
{
	sexp_list_item head;

	object* ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_SHIP)
			head.add_data(Ships[ptr->instance].ship_name);

		ptr = GET_NEXT(ptr);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_or_none() const
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_ship());

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_subsystem_or_none(int parent_node, int arg_index) const
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);
	head.add_list(get_listing_opf_subsystem(parent_node, arg_index));

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_subsys_or_generic(int parent_node, int arg_index) const
{
	sexp_list_item head;
	char buffer[NAME_LENGTH];

	for (int i = 0; i < SUBSYSTEM_MAX; ++i)
	{
		// it's not clear what the "activator" subsystem was intended to do, so let's not display it by default
		if (i != SUBSYSTEM_NONE && i != SUBSYSTEM_UNKNOWN && i != SUBSYSTEM_ACTIVATION)
		{
			sprintf(buffer, SEXP_ALL_GENERIC_SUBSYSTEM_STRING, Subsystem_types[i]);
			SCP_tolower(buffer);
			head.add_data(buffer);
		}
	}
	head.add_data(SEXP_ALL_SUBSYSTEMS_STRING);
	head.add_list(get_listing_opf_subsystem(parent_node, arg_index));

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_jump_nodes()
{
	sexp_list_item head;

	for (auto &jn : Jump_nodes) {
		head.add_data(jn.GetName());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_variable_names()
{
	sexp_list_item head;

	for (const auto& var : Sexp_variables) {
		if (var.type & SEXP_VARIABLE_SET) {
			int t = 0;
			if (var.type & SEXP_VARIABLE_NUMBER) {
				t = SEXPT_NUMBER;
			} else if (var.type & SEXP_VARIABLE_STRING) {
				t = SEXPT_STRING;
			} else {
				Assertion(false, "SEXP variable must be a string or a number!");
			}
			head.add_data(var.variable_name, (t | SEXPT_VALID | SEXPT_VARIABLE));
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_skybox_model()
{
	sexp_list_item head;
	head.add_data("default");
	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_skybox_flags()
{
	sexp_list_item head;

	for (int i = 0; i < Num_skybox_flags; ++i) {
		head.add_data(Skybox_flags[i]);
	}
	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_background_bitmap()
{
	sexp_list_item head;

	for (int i=0; i < stars_get_num_entries(false, true); i++)
	{
		head.add_data( stars_get_name_FRED(i, false) );
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_sun_bitmap()
{
	sexp_list_item head;

	for (int i=0; i < stars_get_num_entries(true, true); i++)
	{
		head.add_data( stars_get_name_FRED(i, true) );
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_nebula_storm_type()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (const auto& storm_t : Storm_types) {
		head.add_data(storm_t.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_nebula_poof()
{
	sexp_list_item head;

	for (poof_info &pf : Poof_info)
	{
		head.add_data(pf.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_turret_target_order()
{
	sexp_list_item head;

	for (const auto& order_name : Turret_target_order_names)
		head.add_data(order_name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_turret_types()
{
	sexp_list_item head;

	for (const auto& turret_valid_type : Turret_valid_types)
		head.add_data(turret_valid_type);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_post_effect()
{
	sexp_list_item head;

	SCP_vector<SCP_string> ppe_names;
	gr_get_post_process_effect_names(ppe_names);
	for (const auto& ppe_name : ppe_names) {
		head.add_data(ppe_name.c_str());
	}
	head.add_data("lightshafts");

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_turret_target_priorities()
{
	sexp_list_item head;

	for(const auto& ai_tp : Ai_tp_list) {
		head.add_data(ai_tp.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_armor_type()
{
	sexp_list_item head;
	head.add_data(SEXP_NONE_STRING);
	for (auto& armor_type : Armor_types) {
		head.add_data(armor_type.GetNamePtr());
	}
	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_damage_type()
{
	sexp_list_item head;
	head.add_data(SEXP_NONE_STRING);
	for (const auto& damage_type : Damage_types) {
		head.add_data(damage_type.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_animation_type()
{
	sexp_list_item head;

	for (const auto &animation_type : animation::Animation_types) {
		head.add_data(animation_type.second.first);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_hud_elements()
{
	sexp_list_item head;
	head.add_data("warpout");

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_weapon_banks()
{
	sexp_list_item head;
	head.add_data(SEXP_ALL_BANKS_STRING);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_mission_moods()
{
	sexp_list_item head;
	for (const auto& builtin_mood : Builtin_moods) {
		head.add_data(builtin_mood.c_str());
	}

	return head.next;
}

// Helper template for flag name lists with deduplication
template <typename M, typename T, typename PTM>
static void add_flag_name_helper(M& flag_name_map, sexp_list_item& head, T flag_name_array[], PTM T::* member, size_t flag_name_count)
{
	for (size_t i = 0; i < flag_name_count; i++)
	{
		auto name = flag_name_array[i].*member;
		if (flag_name_map.count(name) == 0)
		{
			head.add_data(name);
			flag_name_map.insert(name);
		}
	}
}

sexp_list_item *SexpTreeOPF::get_listing_opf_ship_flags()
{
	sexp_list_item head;
	// prevent duplicate names, comparing case-insensitively
	SCP_unordered_set<SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to> all_flags;

	add_flag_name_helper(all_flags, head, Object_flag_names, &obj_flag_name::flag_name, (size_t)Num_object_flag_names);
	add_flag_name_helper(all_flags, head, Ship_flag_names, &ship_flag_name::flag_name, Num_ship_flag_names);
	add_flag_name_helper(all_flags, head, Parse_object_flags, &flag_def_list_new<Mission::Parse_Object_Flags>::name, Num_parse_object_flags);
	add_flag_name_helper(all_flags, head, Ai_flag_names, &ai_flag_name::flag_name, (size_t)Num_ai_flag_names);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_wing_flags()
{
	sexp_list_item head;
	// wing flags
	for (size_t i = 0; i < Num_wing_flag_names; i++) {
		head.add_data(Wing_flag_names[i].flag_name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_team_colors()
{
	sexp_list_item head;
	head.add_data("None"); // Deliberately not SEXP_NONE_STRING
	for (const auto& tc: Team_Colors) {
		head.add_data(tc.first.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_nebula_patterns()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (const auto& neb2_bitmap_filename : Neb2_bitmap_filenames) {
		head.add_data(neb2_bitmap_filename.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_asteroid_types()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	auto list = get_list_valid_asteroid_subtypes();

	for (const auto& this_asteroid : list) {
		head.add_data(this_asteroid.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_debris_types()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (const auto& this_asteroid : Asteroid_info) {
		if (this_asteroid.type == ASTEROID_TYPE_DEBRIS) {
			head.add_data(this_asteroid.name);
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_motion_debris()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (const auto& motion_debris_info : Motion_debris_info) {
		head.add_data(motion_debris_info.name.c_str());
	}

	return head.next;
}

extern SCP_vector<game_snd> Snds;

sexp_list_item *SexpTreeOPF::get_listing_opf_game_snds()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (const auto& snd : Snds) {
		if (!can_construe_as_integer(snd.name.c_str())) {
			head.add_data(snd.name.c_str());
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_fireball()
{
	sexp_list_item head;

	for (const auto &fi: Fireball_info)
	{
		auto unique_id = fi.unique_id;

		if (strlen(unique_id) > 0)
			head.add_data(unique_id);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_species()
{
	sexp_list_item head;

	for (auto &species : Species_info)
		head.add_data(species.species_name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_language()
{
	sexp_list_item head;

	for (auto &lang: Lcl_languages)
		head.add_data(lang.lang_name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_functional_when_eval_type()
{
	sexp_list_item head;

	for (int i = 0; i < Num_functional_when_eval_types; i++)
		head.add_data(Functional_when_eval_type[i]);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_animation_name(int parent_node) const
{
	sexp_list_item head;

	Assertion(parent_node >= 0, "Invalid parent node");

	// get the operator type of the node
	const int op = get_operator_const(_model.tree_nodes[parent_node].text);

	// first child node is the ship name
	int child = _model.tree_nodes[parent_node].child;
	if (child < 0)
		return nullptr;
	const int sh = ship_name_lookup(_model.tree_nodes[child].text, 1);
	if (sh < 0) {
		return nullptr;
	}

	switch(op) {
		case OP_TRIGGER_ANIMATION_NEW:
		case OP_STOP_LOOPING_ANIMATION: {
			child = _model.tree_nodes[child].next;
			if (child < 0) {
				return head.next;
			}
			auto triggerType = animation::anim_match_type(_model.tree_nodes[child].text);

			for (const auto& anim_ref : Ship_info[Ships[sh].ship_info_index].animations.getRegisteredTriggers()) {
				if (anim_ref.type != triggerType)
					continue;

				if (anim_ref.subtype != animation::ModelAnimationSet::SUBTYPE_DEFAULT) {
					int animationSubtype = anim_ref.subtype;

					if (anim_ref.type == animation::ModelAnimationTriggerType::DockBayDoor) {
						//Because of the old system, this is this weird exception. Don't explicitly suggest the NOT doors, as they cannot be explicitly targeted anyways
						if (anim_ref.subtype < 0)
							continue;

						animationSubtype--;
					}

					head.add_data(std::to_string(animationSubtype).c_str());
				}
				else
					head.add_data(anim_ref.name.c_str());
			}

			break;
		}

		case OP_UPDATE_MOVEABLE:
			for(const auto& moveable : Ship_info[Ships[sh].ship_info_index].animations.getRegisteredMoveables())
				head.add_data(moveable.c_str());

			break;
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_sexp_containers(ContainerType con_type)
{
	sexp_list_item head;

	for (const auto &container : get_all_sexp_containers()) {
		if (any(container.type & con_type)) {
			head.add_data(container.container_name.c_str(), (SEXPT_CONTAINER_NAME | SEXPT_STRING | SEXPT_VALID));
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_wing_formation()
{
	sexp_list_item head;

	head.add_data("Default");
	for (const auto &formation : Wing_formations)
		head.add_data(formation.name);

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_bolt_types()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (const auto& b_type : Bolt_types) {
		head.add_data(b_type.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_traitor_overrides()
{
	sexp_list_item head;

	head.add_data(SEXP_NONE_STRING);

	for (const auto& traitor_override : Traitor_overrides) {
		head.add_data(traitor_override.name.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_lua_general_orders()
{
	sexp_list_item head;

	SCP_vector<SCP_string> orders = ai_lua_get_general_orders();

	for (const auto& val : orders) {
		head.add_data(val.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_message_types()
{
	sexp_list_item head;

	for (const auto& val : Builtin_messages) {
		head.add_data(val.name);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_mission_custom_strings()
{
	sexp_list_item head;

	for (const auto& val : The_mission.custom_strings) {
		head.add_data(val.name.c_str());
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_lua_enum(int parent_node, int arg_index) const
{
	// first child node
	int child = _model.tree_nodes[parent_node].child;
	if (child < 0)
		return nullptr;

	int this_index = get_dynamic_parameter_index(_model.tree_nodes[parent_node].text, arg_index);

	if (this_index >= 0) {
		for (int count = 0; count < this_index; count++) {
			child = _model.tree_nodes[child].next;
		}
	} else {
		error_display(1,
			"Expected to find an enum parent parameter for node %i in operator %s but found nothing!",
			arg_index,
			_model.tree_nodes[parent_node].text);
		return nullptr;
	}

	// Append the suffix if it exists
	SCP_string enum_name = _model.tree_nodes[child].text + get_child_enum_suffix(_model.tree_nodes[parent_node].text, arg_index);

	sexp_list_item head;

	int item = get_dynamic_enum_position(enum_name);

	if (item >= 0 && item < static_cast<int>(Dynamic_enums.size())) {

		for (const SCP_string& enum_item : Dynamic_enums[item].list) {
			head.add_data(enum_item.c_str());
		}
	} else {
		// else if enum is invalid do this
		mprintf(("Could not find Lua Enum %s! Using <none> instead!", enum_name.c_str()));
		head.add_data("<none>");
	}
	return head.next;
}

// specific types of subsystems we're looking for
enum : int {
	OPS_CAP_CARGO = 1,
	OPS_STRENGTH = 2,
	OPS_BEAM_TURRET = 3,
	OPS_AWACS = 4,
	OPS_ROTATE = 5,
	OPS_TRANSLATE = 6,
	OPS_ARMOR = 7,
};

sexp_list_item *SexpTreeOPF::get_listing_opf_subsystem(int parent_node, int arg_index) const
{
	sexp_list_item head;
	

	// determine if the parent is one of the set subsystem strength items.  If so,
	// we want to append the "Hull" name onto the end of the menu
	Assertion(parent_node >= 0, "Invalid parent node");

	// get the operator type of the node
	int op = get_operator_const(_model.tree_nodes[parent_node].text);

	// first child node
	int child = _model.tree_nodes[parent_node].child;
	if (child < 0)
		return nullptr;

	int special_subsys = 0;
	switch(op)
	{
		// where we care about hull strength
		case OP_REPAIR_SUBSYSTEM:
		case OP_SABOTAGE_SUBSYSTEM:
		case OP_SET_SUBSYSTEM_STRNGTH:
			special_subsys = OPS_STRENGTH;
			break;

		// Armor types need Hull and Shields but not Simulated Hull
		case OP_SET_ARMOR_TYPE:
		case OP_HAS_ARMOR_TYPE:
			special_subsys = OPS_ARMOR;
			break;

		// awacs subsystems
		case OP_AWACS_SET_RADIUS:
			special_subsys = OPS_AWACS;
			break;

		// rotating
		case OP_LOCK_ROTATING_SUBSYSTEM:
		case OP_FREE_ROTATING_SUBSYSTEM:
		case OP_REVERSE_ROTATING_SUBSYSTEM:
		case OP_ROTATING_SUBSYS_SET_TURN_TIME:
			special_subsys = OPS_ROTATE;
			break;

		// translating
		case OP_LOCK_TRANSLATING_SUBSYSTEM:
		case OP_FREE_TRANSLATING_SUBSYSTEM:
		case OP_REVERSE_TRANSLATING_SUBSYSTEM:
		case OP_TRANSLATING_SUBSYS_SET_SPEED:
			special_subsys = OPS_TRANSLATE;
			break;

		// where we care about capital ship subsystem cargo
		case OP_CAP_SUBSYS_CARGO_KNOWN_DELAY:
			special_subsys = OPS_CAP_CARGO;

			// get the next sibling
			child = _model.tree_nodes[child].next;
			break;

		// where we care about turrets carrying beam weapons
		case OP_BEAM_FIRE:
			special_subsys = OPS_BEAM_TURRET;

			// if this is arg index 3 (targeted ship)
			if(arg_index == 3)
			{
				special_subsys = OPS_STRENGTH;

				// iterate to the next field two times
				child = _model.tree_nodes[child].next;
				if (child < 0) return nullptr;
				child = _model.tree_nodes[child].next;
			}
			else
			{
				Assertion(arg_index == 1, "Invalid argument index");
			}
			break;

		case OP_BEAM_FIRE_COORDS:
			special_subsys = OPS_BEAM_TURRET;
			break;

		// these sexps check the subsystem of the *second entry* on the list, not the first
		case OP_DISTANCE_CENTER_SUBSYSTEM:
		case OP_DISTANCE_BBOX_SUBSYSTEM:
		case OP_SET_CARGO:
		case OP_IS_CARGO:
		case OP_CHANGE_AI_CLASS:
		case OP_IS_AI_CLASS:
		case OP_MISSILE_LOCKED:
		case OP_SHIP_SUBSYS_GUARDIAN_THRESHOLD:
		case OP_IS_IN_TURRET_FOV:
		case OP_TURRET_SET_FORCED_TARGET:
			// iterate to the next field
			child = _model.tree_nodes[child].next;
			break;

		// this sexp checks the subsystem of the *fourth entry* on the list
		case OP_QUERY_ORDERS:
			// iterate to the next field three times
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			break;

		// this sexp checks the subsystem of the *seventh entry* on the list
		case OP_BEAM_FLOATING_FIRE:
			// iterate to the next field six times
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			break;

		// this sexp checks the subsystem of the *ninth entry* on the list
		case OP_WEAPON_CREATE:
			// iterate to the next field eight times
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			if (child < 0) return nullptr;
			child = _model.tree_nodes[child].next;
			break;

		// this sexp checks the third entry, but only for the 4th argument
		case OP_TURRET_SET_FORCED_SUBSYS_TARGET:
			if (arg_index >= 3) {
				child = _model.tree_nodes[child].next;
				if (child < 0) return nullptr;
				child = _model.tree_nodes[child].next;
			}
			break;

		default:
			if (op < First_available_operator_id) {
				break;
			} else {
				int this_index = get_dynamic_parameter_index(_model.tree_nodes[parent_node].text, arg_index);

				if (this_index >= 0) {
					for (int count = 0; count < this_index; count++) {
						child = _model.tree_nodes[child].next;
					}
				} else {
					error_display(1, "Expected to find a dynamic lua parent parameter for node %i in operator %s but found nothing!",
						arg_index,
						_model.tree_nodes[parent_node].text);
				}
			}
	}

	if (child < 0)
		return nullptr;


	// if one of the subsystem strength operators, append the Hull string and the Simulated Hull string
	if (special_subsys == OPS_STRENGTH) {
		head.add_data(SEXP_HULL_STRING);
		head.add_data(SEXP_SIM_HULL_STRING);
	}

	// if setting armor type we only need Hull and Shields
	if (special_subsys == OPS_ARMOR) {
		head.add_data(SEXP_HULL_STRING);
		head.add_data(SEXP_SHIELD_STRING);
	}


	// now find the ship and add all relevant subsystems
	int sh = ship_name_lookup(_model.tree_nodes[child].text, 1);
	if (sh >= 0) {
		ship_subsys*  subsys = GET_FIRST(&Ships[sh].subsys_list);
		while (subsys != END_OF_LIST(&Ships[sh].subsys_list)) {
			// add stuff
			switch(special_subsys){
			// subsystem cargo
			case OPS_CAP_CARGO:
				head.add_data(subsys->system_info->subobj_name);
				break;

			// beam fire
			case OPS_BEAM_TURRET:
				head.add_data(subsys->system_info->subobj_name);
				break;

			// awacs level
			case OPS_AWACS:
				if (subsys->system_info->flags[Model::Subsystem_Flags::Awacs]) {
					head.add_data(subsys->system_info->subobj_name);
				}
				break;

			// rotating
			case OPS_ROTATE:
				if (subsys->system_info->flags[Model::Subsystem_Flags::Rotates]) {
					head.add_data(subsys->system_info->subobj_name);
				}
				break;

			// translating
			case OPS_TRANSLATE:
				if (subsys->system_info->flags[Model::Subsystem_Flags::Translates]) {
					head.add_data(subsys->system_info->subobj_name);
				}
				break;

			// everything else
			default:
				head.add_data(subsys->system_info->subobj_name);
				break;
			}

			// next subsystem
			subsys = GET_NEXT(subsys);
		}
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_listing_opf_subsystem_type(int parent_node) const
{
	sexp_list_item head;

	// first child node
	int child = _model.tree_nodes[parent_node].child;
	if (child < 0)
		return nullptr;

	// now find the ship
	int shipnum = ship_name_lookup(_model.tree_nodes[child].text, 1);
	if (shipnum < 0) {
		return head.next;
	}

	// add all relevant subsystem types
	int num_added = 0;
	for (int i = 0; i < SUBSYSTEM_MAX; i++) {
		// don't allow these two
		if (i == SUBSYSTEM_NONE || i == SUBSYSTEM_UNKNOWN)
			continue;

		// loop through all ship subsystems
		ship_subsys* subsys = GET_FIRST(&Ships[shipnum].subsys_list);
		while (subsys != END_OF_LIST(&Ships[shipnum].subsys_list)) {
			// check if this subsystem is of this type
			if (i == subsys->system_info->type) {
				// subsystem type is applicable, so add it
				head.add_data(Subsystem_types[i]);
				num_added++;
				break;
			}

			// next subsystem
			subsys = GET_NEXT(subsys);
		}
	}

	// if no subsystem types, go ahead and add NONE (even though it won't be checked)
	if (num_added == 0) {
		head.add_data(Subsystem_types[SUBSYSTEM_NONE]);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_container_modifiers(int con_data_node) const
{
	Assertion(con_data_node != -1, "Attempt to get modifiers for invalid container node. Please report!");
	Assertion(_model.tree_nodes[con_data_node].type & SEXPT_CONTAINER_DATA,
		"Attempt to get modifiers for non-container data node %s. Please report!",
		_model.tree_nodes[con_data_node].text);

	const auto *p_container = get_sexp_container(_model.tree_nodes[con_data_node].text);
	Assertion(p_container,
		"Attempt to get modifiers for unknown container %s. Please report!",
		_model.tree_nodes[con_data_node].text);
	const auto &container = *p_container;

	sexp_list_item head;
	sexp_list_item *list = nullptr;

	if (container.is_list()) {
		list = get_list_container_modifiers();
	} else if (container.is_map()) {
		// start the list with "<argument>" if relevant
		if (_model.is_node_eligible_for_special_argument(con_data_node) &&
			any(container.type & ContainerType::STRING_KEYS)) {
			head.add_data(SEXP_ARGUMENT_STRING, (SEXPT_VALID | SEXPT_STRING | SEXPT_MODIFIER));
		}

		list = get_map_container_modifiers(con_data_node);
	} else {
		UNREACHABLE("Unknown container type %d", static_cast<int>(p_container->type));
	}

	if (list) {
		head.add_list(list);
	}

	return head.next;
}

sexp_list_item *SexpTreeOPF::get_list_container_modifiers()
{
	sexp_list_item head;

	for (const auto &modifier : get_all_list_modifiers()) {
		head.add_data(modifier.name, SEXPT_VALID | SEXPT_MODIFIER | SEXPT_STRING);
	}

	return head.next;
}

// FIXME TODO: if you use this function with remove-from-map SEXP, don't use SEXPT_MODIFIER
sexp_list_item *SexpTreeOPF::get_map_container_modifiers(int con_data_node) const
{
	sexp_list_item head;

	Assertion(_model.tree_nodes[con_data_node].type & SEXPT_CONTAINER_DATA,
		"Found map modifier for non-container data node %s. Please report!",
		_model.tree_nodes[con_data_node].text);

	const auto *p_container = get_sexp_container(_model.tree_nodes[con_data_node].text);
	Assertion(p_container != nullptr,
		"Found map modifier for unknown container %s. Please report!",
		_model.tree_nodes[con_data_node].text);

	const auto &container = *p_container;
	Assertion(container.is_map(),
		"Found map modifier for non-map container %s with type %d. Please report!",
		_model.tree_nodes[con_data_node].text,
		static_cast<int>(container.type));

	int type = SEXPT_VALID | SEXPT_MODIFIER;
	if (any(container.type & ContainerType::STRING_KEYS)) {
		type |= SEXPT_STRING;
	} else if (any(container.type & ContainerType::NUMBER_KEYS)) {
		type |= SEXPT_NUMBER;
	} else {
		UNREACHABLE("Unknown map container key type %d", static_cast<int>(container.type));
	}

	for (const auto &kv_pair : container.map_data) {
		head.add_data(kv_pair.first.c_str(), type);
	}

	return head.next;
}

// get potential options for container multidimensional modifiers
// the value could be either string or number, checked in-mission
sexp_list_item *SexpTreeOPF::get_container_multidim_modifiers(int con_data_node) const
{
	Assertion(con_data_node != -1,
		"Attempt to get multidimensional modifiers for invalid container node. Please report!");
	Assertion(_model.tree_nodes[con_data_node].type & SEXPT_CONTAINER_DATA,
		"Attempt to get multidimensional modifiers for non-container data node %s. Please report!",
		_model.tree_nodes[con_data_node].text);

	sexp_list_item head;

	if (_model.is_node_eligible_for_special_argument(con_data_node)) {
		head.add_data(SEXP_ARGUMENT_STRING, (SEXPT_VALID | SEXPT_STRING | SEXPT_MODIFIER));
	}

	// the FREDder might want to use a list modifier
	sexp_list_item *list = get_list_container_modifiers();

	head.add_list(list);

	return head.next;
}

sexp_list_item *SexpTreeOPF::check_for_dynamic_sexp_enum(int opf)
{
	sexp_list_item head;

	int item = opf - First_available_opf_id;

	if (item < static_cast<int>(Dynamic_enums.size())) {

		for (const SCP_string& enum_item : Dynamic_enums[item].list) {
			head.add_data(enum_item.c_str());
		}
		return head.next;
	} else {
		// else if opf is invalid do this
		UNREACHABLE("Unhandled SEXP argument type!"); // unknown OPF code
		return nullptr;
	}
}

bool SexpTreeOPF::is_container_name_opf_type(const int op_type)
{
	return (op_type == OPF_CONTAINER_NAME) ||
		   (op_type == OPF_LIST_CONTAINER_NAME) ||
		   (op_type == OPF_MAP_CONTAINER_NAME);
}

// -----------------------------------------------------------------------
// Main OPF dispatch function
// -----------------------------------------------------------------------

sexp_list_item *SexpTreeOPF::get_listing_opf(int opf, int parent_node, int arg_index) const
{
	sexp_list_item head;
	sexp_list_item *list = nullptr;

	switch (opf) {
		case OPF_NONE:
			list = nullptr;
			break;

		case OPF_NULL:
			list = get_listing_opf_null();
			break;

		case OPF_BOOL:
			list = get_listing_opf_bool(parent_node);
			break;

		case OPF_NUMBER:
			list = get_listing_opf_number();
			break;

		case OPF_SHIP:
			list = get_listing_opf_ship(parent_node);
			break;

		case OPF_PROP:
			list = get_listing_opf_prop();
			break;

		case OPF_WING:
			list = get_listing_opf_wing();
			break;

		case OPF_AWACS_SUBSYSTEM:
		case OPF_ROTATING_SUBSYSTEM:
		case OPF_TRANSLATING_SUBSYSTEM:
		case OPF_SUBSYSTEM:
			list = get_listing_opf_subsystem(parent_node, arg_index);
			break;

		case OPF_SUBSYSTEM_TYPE:
			list = get_listing_opf_subsystem_type(parent_node);
			break;

		case OPF_POINT:
			list = get_listing_opf_point();
			break;

		case OPF_IFF:
			list = get_listing_opf_iff();
			break;

		case OPF_AI_CLASS:
			list = get_listing_opf_ai_class();
			break;

		case OPF_SUPPORT_SHIP_CLASS:
			list = get_listing_opf_support_ship_class();
			break;

		case OPF_SSM_CLASS:
			list = get_listing_opf_ssm_class();
			break;

		case OPF_ARRIVAL_LOCATION:
			list = get_listing_opf_arrival_location();
			break;

		case OPF_DEPARTURE_LOCATION:
			list = get_listing_opf_departure_location();
			break;

		case OPF_ARRIVAL_ANCHOR_ALL:
			list = get_listing_opf_arrival_anchor_all();
			break;

		case OPF_SHIP_WITH_BAY:
			list = get_listing_opf_ship_with_bay();
			break;

		case OPF_SOUNDTRACK_NAME:
			list = get_listing_opf_soundtrack_name();
			break;

		case OPF_AI_GOAL:
			list = get_listing_opf_ai_goal(parent_node);
			break;

		case OPF_FLEXIBLE_ARGUMENT:
			list = get_listing_opf_flexible_argument();
			break;

		case OPF_DOCKER_POINT:
			list = get_listing_opf_docker_point(parent_node, arg_index);
			break;

		case OPF_DOCKEE_POINT:
			list = get_listing_opf_dockee_point(parent_node);
			break;

		case OPF_MESSAGE:
			list = get_listing_opf_message();
			break;

		case OPF_WHO_FROM:
			list = get_listing_opf_who_from();
			break;

		case OPF_PRIORITY:
			list = get_listing_opf_priority();
			break;

		case OPF_WAYPOINT_PATH:
			list = get_listing_opf_waypoint_path();
			break;

		case OPF_POSITIVE:
			list = get_listing_opf_positive();
			break;

		case OPF_MISSION_NAME:
			list = get_listing_opf_mission_name();
			break;

		case OPF_SHIP_POINT:
			list = get_listing_opf_ship_point();
			break;

		case OPF_GOAL_NAME:
			list = get_listing_opf_goal_name(parent_node);
			break;

		case OPF_SHIP_WING:
			list = get_listing_opf_ship_wing();
			break;

		case OPF_SHIP_PROP:
			list = get_listing_opf_ship_prop();
			break;

		case OPF_SHIP_WING_WHOLETEAM:
			list = get_listing_opf_ship_wing_wholeteam();
			break;

		case OPF_SHIP_WING_SHIPONTEAM_POINT:
			list = get_listing_opf_ship_wing_shiponteam_point();
			break;

		case OPF_SHIP_WING_POINT:
			list = get_listing_opf_ship_wing_point();
			break;

		case OPF_SHIP_WING_POINT_OR_NONE:
			list = get_listing_opf_ship_wing_point_or_none();
			break;

		case OPF_ORDER_RECIPIENT:
			list = get_listing_opf_order_recipient();
			break;

		case OPF_SHIP_TYPE:
			list = get_listing_opf_ship_type();
			break;

		case OPF_KEYPRESS:
			list = get_listing_opf_keypress();
			break;

		case OPF_EVENT_NAME:
			list = get_listing_opf_event_name(parent_node);
			break;

		case OPF_AI_ORDER:
			list = get_listing_opf_ai_order();
			break;

		case OPF_SKILL_LEVEL:
			list = get_listing_opf_skill_level();
			break;

		case OPF_CARGO:
			list = get_listing_opf_cargo();
			break;

		case OPF_STRING:
			list = get_listing_opf_string();
			break;

		case OPF_MEDAL_NAME:
			list = get_listing_opf_medal_name();
			break;

		case OPF_WEAPON_NAME:
			list = get_listing_opf_weapon_name();
			break;

		case OPF_INTEL_NAME:
			list = get_listing_opf_intel_name();
			break;

		case OPF_SHIP_CLASS_NAME:
			list = get_listing_opf_ship_class_name();
			break;

		case OPF_PROP_CLASS_NAME:
			list = get_listing_opf_prop_class_name();
			break;

		case OPF_HUGE_WEAPON:
			list = get_listing_opf_huge_weapon();
			break;

		case OPF_SHIP_NOT_PLAYER:
			list = get_listing_opf_ship_not_player();
			break;

		case OPF_SHIP_OR_NONE:
			list = get_listing_opf_ship_or_none();
			break;

		case OPF_SUBSYSTEM_OR_NONE:
			list = get_listing_opf_subsystem_or_none(parent_node, arg_index);
			break;

		case OPF_SUBSYS_OR_GENERIC:
			list = get_listing_opf_subsys_or_generic(parent_node, arg_index);
			break;

		case OPF_JUMP_NODE_NAME:
			list = get_listing_opf_jump_nodes();
			break;

		case OPF_VARIABLE_NAME:
			list = get_listing_opf_variable_names();
			break;

		case OPF_AMBIGUOUS:
			list = nullptr;
			break;

		case OPF_ANYTHING:
			list = nullptr;
			break;

		case OPF_SKYBOX_MODEL_NAME:
			list = get_listing_opf_skybox_model();
			break;

		case OPF_SKYBOX_FLAGS:
			list = get_listing_opf_skybox_flags();
			break;

		case OPF_BACKGROUND_BITMAP:
			list = get_listing_opf_background_bitmap();
			break;

		case OPF_SUN_BITMAP:
			list = get_listing_opf_sun_bitmap();
			break;

		case OPF_NEBULA_STORM_TYPE:
			list = get_listing_opf_nebula_storm_type();
			break;

		case OPF_NEBULA_POOF:
			list = get_listing_opf_nebula_poof();
			break;

		case OPF_TURRET_TARGET_ORDER:
			list = get_listing_opf_turret_target_order();
			break;

		case OPF_TURRET_TYPE:
			list = get_listing_opf_turret_types();
			break;

		case OPF_TARGET_PRIORITIES:
			list = get_listing_opf_turret_target_priorities();
			break;

		case OPF_ARMOR_TYPE:
			list = get_listing_opf_armor_type();
			break;

		case OPF_DAMAGE_TYPE:
			list = get_listing_opf_damage_type();
			break;

		case OPF_ANIMATION_TYPE:
			list = get_listing_opf_animation_type();
			break;

		case OPF_PERSONA:
			list = get_listing_opf_persona();
			break;

		case OPF_POST_EFFECT:
			list = get_listing_opf_post_effect();
			break;

		case OPF_FONT:
			list = get_listing_opf_font();
			break;

		case OPF_HUD_ELEMENT:
			list = get_listing_opf_hud_elements();
			break;

		case OPF_SOUND_ENVIRONMENT:
			list = get_listing_opf_sound_environment();
			break;

		case OPF_SOUND_ENVIRONMENT_OPTION:
			list = get_listing_opf_sound_environment_option();
			break;

		case OPF_AUDIO_VOLUME_OPTION:
			list = get_listing_opf_adjust_audio_volume();
			break;

		case OPF_EXPLOSION_OPTION:
			list = get_listing_opf_explosion_option();
			break;

		case OPF_WEAPON_BANK_NUMBER:
			list = get_listing_opf_weapon_banks();
			break;

		case OPF_MESSAGE_OR_STRING:
			list = get_listing_opf_message();
			break;

		case OPF_BUILTIN_HUD_GAUGE:
			list = get_listing_opf_builtin_hud_gauge();
			break;

		case OPF_CUSTOM_HUD_GAUGE:
			list = get_listing_opf_custom_hud_gauge();
			break;

		case OPF_ANY_HUD_GAUGE:
			list = get_listing_opf_any_hud_gauge();
			break;

		case OPF_SHIP_EFFECT:
			list = get_listing_opf_ship_effect();
			break;

		case OPF_MISSION_MOOD:
			list = get_listing_opf_mission_moods();
			break;

		case OPF_SHIP_FLAG:
			list = get_listing_opf_ship_flags();
			break;

		case OPF_WING_FLAG:
			list = get_listing_opf_wing_flags();
			break;

		case OPF_TEAM_COLOR:
			list = get_listing_opf_team_colors();
			break;

		case OPF_NEBULA_PATTERN:
			list = get_listing_opf_nebula_patterns();
			break;

		case OPF_GAME_SND:
			list = get_listing_opf_game_snds();
			break;

		case OPF_FIREBALL:
			list = get_listing_opf_fireball();
			break;

		case OPF_SPECIES:
			list = get_listing_opf_species();
			break;

		case OPF_LANGUAGE:
			list = get_listing_opf_language();
			break;

		case OPF_FUNCTIONAL_WHEN_EVAL_TYPE:
			list = get_listing_opf_functional_when_eval_type();
			break;

		case OPF_ANIMATION_NAME:
			list = get_listing_opf_animation_name(parent_node);
			break;

		case OPF_CONTAINER_NAME:
			list = get_listing_opf_sexp_containers(ContainerType::LIST | ContainerType::MAP);
			break;

		case OPF_LIST_CONTAINER_NAME:
			list = get_listing_opf_sexp_containers(ContainerType::LIST);
			break;

		case OPF_MAP_CONTAINER_NAME:
			list = get_listing_opf_sexp_containers(ContainerType::MAP);
			break;

		case OPF_CONTAINER_VALUE:
			list = nullptr;
			break;

		case OPF_DATA_OR_STR_CONTAINER:
			list = nullptr;
			break;

		case OPF_ASTEROID_TYPES:
			list = get_listing_opf_asteroid_types();
			break;

		case OPF_DEBRIS_TYPES:
			list = get_listing_opf_debris_types();
			break;

		case OPF_WING_FORMATION:
			list = get_listing_opf_wing_formation();
			break;

		case OPF_MOTION_DEBRIS:
			list = get_listing_opf_motion_debris();
			break;

		case OPF_BOLT_TYPE:
			list = get_listing_opf_bolt_types();
			break;

		case OPF_TRAITOR_OVERRIDE:
			list = get_listing_opf_traitor_overrides();
			break;

		case OPF_LUA_GENERAL_ORDER:
			list = get_listing_opf_lua_general_orders();
			break;

		case OPF_MESSAGE_TYPE:
			list = get_listing_opf_message_types();
			break;

		case OPF_CHILD_LUA_ENUM:
			list = get_listing_opf_lua_enum(parent_node, arg_index);
			break;

		case OPF_MISSION_CUSTOM_STRING:
			list = get_listing_opf_mission_custom_strings();
			break;

		default:
			//We're at the end of the list so check for any dynamic enums
			list = check_for_dynamic_sexp_enum(opf);
			break;
	}


	// skip OPF_NONE, also skip for OPF_NULL, because it takes no data (though it can take plenty of operators)
	if (opf == OPF_NULL || opf == OPF_NONE) {
		return list;
	}

	// skip the special argument if we aren't at the right spot in when-argument or
	// every-time-argument
	if (!_model.is_node_eligible_for_special_argument(parent_node)) {
		return list;
	}

	// the special item is a string and should not be added for numeric lists
	if (opf != OPF_NUMBER && opf != OPF_POSITIVE) {
		head.add_data(SEXP_ARGUMENT_STRING);
	}

	if (list != nullptr) {
		// append other list
		head.add_list(list);
	}

	// return listing
	return head.next;
}

// -----------------------------------------------------------------------
// Default argument availability and values
// -----------------------------------------------------------------------


// Returns non-zero if all minimum required arguments of operator 'op' have default values.
// Checks each argument position from 0 to Operators[op].min-1.
int SexpTreeOPF::query_default_argument_available(int op) const
{
	int i;

	Assertion(op >= 0, "Invalid operator index");
	for (i = 0; i < Operators[op].min; i++)
		if (!query_default_argument_available(op, i))
			return 0;

	return 1;
}

// Returns non-zero if argument position 'i' of operator 'op' has a default value available.
// Checks based on the OPF_* type: some types always have defaults, others depend on
// whether ships, wings, goals, events, or other game data currently exist.
int SexpTreeOPF::query_default_argument_available(int op, int i) const
{
	int j, type;
	object* ptr;

	type = query_operator_argument_type(op, i);
	switch (type) {
		case OPF_NONE:
		case OPF_NULL:
		case OPF_BOOL:
		case OPF_NUMBER:
		case OPF_POSITIVE:
		case OPF_IFF:
		case OPF_AI_CLASS:
		case OPF_WHO_FROM:
		case OPF_PRIORITY:
		case OPF_SHIP_TYPE:
		case OPF_SUBSYSTEM:
		case OPF_AWACS_SUBSYSTEM:
		case OPF_ROTATING_SUBSYSTEM:
		case OPF_TRANSLATING_SUBSYSTEM:
		case OPF_SUBSYSTEM_TYPE:
		case OPF_DOCKER_POINT:
		case OPF_DOCKEE_POINT:
		case OPF_AI_GOAL:
		case OPF_KEYPRESS:
		case OPF_AI_ORDER:
		case OPF_SKILL_LEVEL:
		case OPF_MEDAL_NAME:
		case OPF_WEAPON_NAME:
		case OPF_INTEL_NAME:
		case OPF_SHIP_CLASS_NAME:
		case OPF_PROP_CLASS_NAME:
		case OPF_HUGE_WEAPON:
		case OPF_JUMP_NODE_NAME:
		case OPF_AMBIGUOUS:
		case OPF_CARGO:
		case OPF_ARRIVAL_LOCATION:
		case OPF_DEPARTURE_LOCATION:
		case OPF_ARRIVAL_ANCHOR_ALL:
		case OPF_SUPPORT_SHIP_CLASS:
		case OPF_SHIP_WITH_BAY:
		case OPF_SOUNDTRACK_NAME:
		case OPF_STRING:
		case OPF_FLEXIBLE_ARGUMENT:
		case OPF_ANYTHING:
		case OPF_DATA_OR_STR_CONTAINER:
		case OPF_SKYBOX_MODEL_NAME:
		case OPF_SKYBOX_FLAGS:
		case OPF_SHIP_OR_NONE:
		case OPF_SUBSYSTEM_OR_NONE:
		case OPF_SHIP_WING_POINT_OR_NONE:
		case OPF_SUBSYS_OR_GENERIC:
		case OPF_BACKGROUND_BITMAP:
		case OPF_SUN_BITMAP:
		case OPF_NEBULA_STORM_TYPE:
		case OPF_NEBULA_POOF:
		case OPF_TURRET_TARGET_ORDER:
		case OPF_TURRET_TYPE:
		case OPF_POST_EFFECT:
		case OPF_TARGET_PRIORITIES:
		case OPF_ARMOR_TYPE:
		case OPF_DAMAGE_TYPE:
		case OPF_FONT:
		case OPF_HUD_ELEMENT:
		case OPF_SOUND_ENVIRONMENT:
		case OPF_SOUND_ENVIRONMENT_OPTION:
		case OPF_EXPLOSION_OPTION:
		case OPF_AUDIO_VOLUME_OPTION:
		case OPF_WEAPON_BANK_NUMBER:
		case OPF_MESSAGE_OR_STRING:
		case OPF_BUILTIN_HUD_GAUGE:
		case OPF_CUSTOM_HUD_GAUGE:
		case OPF_ANY_HUD_GAUGE:
		case OPF_SHIP_EFFECT:
		case OPF_ANIMATION_TYPE:
		case OPF_SHIP_FLAG:
		case OPF_WING_FLAG:
		case OPF_NEBULA_PATTERN:
		case OPF_NAV_POINT:
		case OPF_TEAM_COLOR:
		case OPF_GAME_SND:
		case OPF_FIREBALL:
		case OPF_SPECIES:
		case OPF_LANGUAGE:
		case OPF_FUNCTIONAL_WHEN_EVAL_TYPE:
		case OPF_ANIMATION_NAME:
		case OPF_CONTAINER_VALUE:
		case OPF_WING_FORMATION:
		case OPF_CHILD_LUA_ENUM:
		case OPF_MESSAGE_TYPE:
			return 1;

		case OPF_SHIP:
		case OPF_SHIP_WING:
		case OPF_SHIP_POINT:
		case OPF_SHIP_WING_POINT:
		case OPF_SHIP_WING_WHOLETEAM:
		case OPF_SHIP_WING_SHIPONTEAM_POINT:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START)
					return 1;

				ptr = GET_NEXT(ptr);
			}

			return 0;

		case OPF_SHIP_PROP:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START || ptr->type == OBJ_PROP)
					return 1;

				ptr = GET_NEXT(ptr);
			}

			return 0;

		case OPF_PROP:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_PROP)
					return 1;

				ptr = GET_NEXT(ptr);
			}
			return 0;

		case OPF_SHIP_NOT_PLAYER:
		case OPF_ORDER_RECIPIENT:
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP)
					return 1;

				ptr = GET_NEXT(ptr);
			}

			return 0;

		case OPF_WING:
			for (j = 0; j < MAX_WINGS; j++)
				if (Wings[j].wave_count)
					return 1;

			return 0;

		case OPF_PERSONA:
			return Personas.empty() ? 0 : 1;

		case OPF_POINT:
		case OPF_WAYPOINT_PATH:
			return Waypoint_lists.empty() ? 0 : 1;

		case OPF_MISSION_NAME:
			if (!_model._interface || !_model._interface->requireCampaignOperators()) {
				if (_model._interface && !_model._interface->hasDefaultMissionName())
					return 0;

				return 1;
			}

			if (Campaign.num_missions > 0)
				return 1;

			return 0;

		case OPF_GOAL_NAME: {
			int value;

			value = Operators[op].value;

			if (_model._interface && _model._interface->requireCampaignOperators())
				return 1;

			else if (_model._interface && _model._interface->hasDefaultGoal(value))
				return 1;

			return 0;
		}

		case OPF_EVENT_NAME: {
			int value;

			value = Operators[op].value;

			if (_model._interface && _model._interface->requireCampaignOperators())
				return 1;

			else if (_model._interface && _model._interface->hasDefaultEvent(value))
				return 1;

			return 0;
		}

		case OPF_MESSAGE:
			if (_model._interface && _model._interface->hasDefaultMessageParameter())
				return 1;

			return 0;

		case OPF_VARIABLE_NAME:
			return (sexp_variable_count() > 0) ? 1 : 0;

		case OPF_SSM_CLASS:
			return Ssm_info.empty() ? 0 : 1;

		case OPF_MISSION_MOOD:
			return Builtin_moods.empty() ? 0 : 1;

		case OPF_CONTAINER_NAME:
			return get_all_sexp_containers().empty() ? 0 : 1;

		case OPF_LIST_CONTAINER_NAME:
			for (const auto& container : get_all_sexp_containers()) {
				if (container.is_list()) {
					return 1;
				}
			}
			return 0;

		case OPF_MAP_CONTAINER_NAME:
			for (const auto& container : get_all_sexp_containers()) {
				if (container.is_map()) {
					return 1;
				}
			}
			return 0;

		case OPF_ASTEROID_TYPES:
			if (!get_list_valid_asteroid_subtypes().empty()) {
				return 1;
			}
			return 0;

		case OPF_DEBRIS_TYPES:
			for (const auto& this_asteroid : Asteroid_info) {
				if (this_asteroid.type == ASTEROID_TYPE_DEBRIS) {
					return 1;
				}
			}
			return 0;

		case OPF_MOTION_DEBRIS:
			if (!Motion_debris_info.empty()) {
				return 1;
			}
			return 0;

		case OPF_BOLT_TYPE:
			if (!Bolt_types.empty()) {
				return 1;
			}
			return 0;

		case OPF_TRAITOR_OVERRIDE:
			return Traitor_overrides.empty() ? 0 : 1;

		case OPF_LUA_GENERAL_ORDER:
			return (ai_lua_get_num_general_orders() > 0) ? 1 : 0;

		case OPF_MISSION_CUSTOM_STRING:
			return The_mission.custom_strings.empty() ? 0 : 1;

		default:
			if (!Dynamic_enums.empty()) {
				if ((type - First_available_opf_id) < static_cast<int>(Dynamic_enums.size())) {
					return 1;
				} else {
					UNREACHABLE("Unhandled SEXP argument type!");
				}
			} else {
				UNREACHABLE("Unhandled SEXP argument type!");
			}
	}

	return 0;
}

// Determine and return the default value for operator argument position i.
// Returns 0 on success, -1 if no default available.
int SexpTreeOPF::get_default_value(sexp_list_item* item, int op, int i) const
{
	const char* str = nullptr;
	int type, index;
	sexp_list_item* list;

	index = _model.item_index;
	type = query_operator_argument_type(op, i);
	switch (type)
	{
		case OPF_NULL:
			item->set_op(OP_NOP);
			return 0;

		case OPF_BOOL:
			item->set_op(OP_TRUE);
			return 0;

		case OPF_ANYTHING:
			if (Operators[op].value == OP_INVALIDATE_ARGUMENT || Operators[op].value == OP_VALIDATE_ARGUMENT)
				item->set_data(SEXP_ARGUMENT_STRING);
			else
				item->set_data("<any data>");
			return 0;

		case OPF_DATA_OR_STR_CONTAINER:
			item->set_data("<any data or string container>");
			return 0;

		case OPF_NUMBER:
		case OPF_POSITIVE:
		case OPF_AMBIGUOUS:
			// if the top level operator is an AI goal, and we are adding the last number required,
			// assume that this number is a priority and make it 89 instead of 1.
			if ((query_operator_return_type(op) == OPR_AI_GOAL) && (i == (Operators[op].min - 1)))
			{
				item->set_data("89", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (((Operators[op].value == OP_HAS_DOCKED_DELAY) || (Operators[op].value == OP_HAS_UNDOCKED_DELAY) || (Operators[op].value == OP_TIME_DOCKED) || (Operators[op].value == OP_TIME_UNDOCKED)) && (i == 2))
			{
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if ((Operators[op].value == OP_SHIP_TYPE_DESTROYED) || (Operators[op].value == OP_GOOD_SECONDARY_TIME))
			{
				item->set_data("100", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_SET_SUPPORT_SHIP)
			{
				item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (((Operators[op].value == OP_SHIP_TAG) && (i == 1)) || ((Operators[op].value == OP_TRIGGER_SUBMODEL_ANIMATION) && (i == 3)))
			{
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_EXPLOSION_EFFECT)
			{
				int temp;
				char sexp_str_token[TOKEN_LENGTH];

				switch (i)
				{
					case 3:
						temp = 10;
						break;
					case 4:
						temp = 10;
						break;
					case 5:
						temp = 100;
						break;
					case 6:
						temp = 10;
						break;
					case 7:
						temp = 100;
						break;
					case 11:
						temp = static_cast<int>(EMP_DEFAULT_INTENSITY);
						break;
					case 12:
						temp = static_cast<int>(EMP_DEFAULT_TIME);
						break;
					default:
						temp = 0;
						break;
				}

				sprintf(sexp_str_token, "%d", temp);
				item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_WARP_EFFECT)
			{
				int temp;
				char sexp_str_token[TOKEN_LENGTH];

				switch (i)
				{
					case 6:
						temp = 100;
						break;
					case 7:
						temp = 10;
						break;
					default:
						temp = 0;
						break;
				}

				sprintf(sexp_str_token, "%d", temp);
				item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_CHANGE_BACKGROUND)
			{
				item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_ADD_BACKGROUND_BITMAP || Operators[op].value == OP_ADD_BACKGROUND_BITMAP_NEW)
			{
				int temp = 0;
				char sexp_str_token[TOKEN_LENGTH];

				switch (i)
				{
					case 4:
					case 5:
						temp = 100;
						break;

					case 6:
					case 7:
						temp = 1;
						break;
				}

				sprintf(sexp_str_token, "%d", temp);
				item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_ADD_SUN_BITMAP || Operators[op].value == OP_ADD_SUN_BITMAP_NEW)
			{
				int temp = 0;
				char sexp_str_token[TOKEN_LENGTH];

				if (i == 4)
					temp = 100;

				sprintf(sexp_str_token, "%d", temp);
				item->set_data(sexp_str_token, (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_MISSION_SET_NEBULA)
			{
				if (i == 0)
					item->set_data("1", (SEXPT_NUMBER | SEXPT_VALID));
				else
					item->set_data("3000", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_MODIFY_VARIABLE)
			{
				if (_model.get_modify_variable_type(index) == OPF_NUMBER)
					item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
				else
					item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_MODIFY_VARIABLE_XSTR)
			{
				if (i == 1)
					item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
				else
					item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_SET_VARIABLE_BY_INDEX)
			{
				if (i == 0)
					item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
				else
					item->set_data("<any data>", (SEXPT_STRING | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_JETTISON_CARGO_NEW)
			{
				item->set_data("25", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else if (Operators[op].value == OP_TECH_ADD_INTEL_XSTR || Operators[op].value == OP_TECH_REMOVE_INTEL_XSTR)
			{
				item->set_data("-1", (SEXPT_NUMBER | SEXPT_VALID));
			}
			else
			{
				item->set_data("0", (SEXPT_NUMBER | SEXPT_VALID));
			}

			return 0;

		// Goober5000 - special cases that used to be numbers but are now hybrids
		case OPF_GAME_SND:
		{
			gamesnd_id sound_index;

			if (Operators[op].value == OP_EXPLOSION_EFFECT)
			{
				sound_index = GameSounds::SHIP_EXPLODE_1;
			}
			else if (Operators[op].value == OP_WARP_EFFECT)
			{
				sound_index = (i == 8) ? GameSounds::CAPITAL_WARP_IN : GameSounds::CAPITAL_WARP_OUT;
			}

			if (sound_index.isValid())
			{
				game_snd* snd = gamesnd_get_game_sound(sound_index);
				if (can_construe_as_integer(snd->name.c_str()))
					item->set_data(snd->name.c_str(), (SEXPT_NUMBER | SEXPT_VALID));
				else
					item->set_data(snd->name.c_str(), (SEXPT_STRING | SEXPT_VALID));
				return 0;
			}

			// if no hardcoded default, just use the listing default
			break;
		}

		// Goober5000 - ditto
		case OPF_FIREBALL:
		{
			int fireball_index = -1;

			if (Operators[op].value == OP_EXPLOSION_EFFECT)
			{
				fireball_index = FIREBALL_MEDIUM_EXPLOSION;
			}
			else if (Operators[op].value == OP_WARP_EFFECT)
			{
				fireball_index = FIREBALL_WARP;
			}

			if (fireball_index >= 0)
			{
				char* unique_id = Fireball_info[fireball_index].unique_id;
				if (strlen(unique_id) > 0)
					item->set_data(unique_id, (SEXPT_STRING | SEXPT_VALID));
				else
				{
					char num_str[NAME_LENGTH];
					sprintf(num_str, "%d", fireball_index);
					item->set_data(num_str, (SEXPT_NUMBER | SEXPT_VALID));
				}
				return 0;
			}

			// if no hardcoded default, just use the listing default
			break;
		}

		// new default value
		case OPF_PRIORITY:
			item->set_data("Normal", (SEXPT_STRING | SEXPT_VALID));
			return 0;
	}

	list = get_listing_opf(type, index, i);

	// Goober5000 - the way this is done is really stupid, so stupid hacks are needed to deal with it
	// this particular hack is necessary because the argument string should never be a default
	if (list && list->text == SEXP_ARGUMENT_STRING)
	{
		sexp_list_item* first_ptr;

		first_ptr = list;
		list = list->next;

		delete first_ptr;
	}

	if (list)
	{
		// copy the information from the list to the passed-in item
		*item = *list;

		// get rid of the list, since we're done with it
		list->destroy();
		item->next = nullptr;

		return 0;
	}

	// catch anything that doesn't have a default value.  Just describe what should be here instead
	switch (type)
	{
		case OPF_SHIP:
		case OPF_SHIP_NOT_PLAYER:
		case OPF_SHIP_POINT:
		case OPF_SHIP_WING:
		case OPF_SHIP_PROP:
		case OPF_SHIP_WING_WHOLETEAM:
		case OPF_SHIP_WING_SHIPONTEAM_POINT:
		case OPF_SHIP_WING_POINT:
			str = "<name of ship here>";
			break;

		case OPF_PROP:
			str = "<name of prop here>";
			break;

		case OPF_ORDER_RECIPIENT:
			str = "<all fighters>";
			break;

		case OPF_SHIP_OR_NONE:
		case OPF_SUBSYSTEM_OR_NONE:
		case OPF_SHIP_WING_POINT_OR_NONE:
			str = SEXP_NONE_STRING;
			break;

		case OPF_WING:
			str = "<name of wing here>";
			break;

		case OPF_DOCKER_POINT:
			str = "<docker point>";
			break;

		case OPF_DOCKEE_POINT:
			str = "<dockee point>";
			break;

		case OPF_SUBSYSTEM:
		case OPF_AWACS_SUBSYSTEM:
		case OPF_ROTATING_SUBSYSTEM:
		case OPF_TRANSLATING_SUBSYSTEM:
		case OPF_SUBSYS_OR_GENERIC:
			str = "<name of subsystem>";
			break;

		case OPF_SUBSYSTEM_TYPE:
			str = Subsystem_types[SUBSYSTEM_NONE];
			break;

		case OPF_POINT:
			str = "<waypoint>";
			break;

		case OPF_MESSAGE:
			str = "<Message>";
			break;

		case OPF_WHO_FROM:
			str = "<any wingman>";
			break;

		case OPF_WAYPOINT_PATH:
			str = "<waypoint path>";
			break;

		case OPF_MISSION_NAME:
			str = "<mission name>";
			break;

		case OPF_GOAL_NAME:
			str = "<goal name>";
			break;

		case OPF_SHIP_TYPE:
			str = "<ship type here>";
			break;

		case OPF_EVENT_NAME:
			str = "<event name>";
			break;

		case OPF_HUGE_WEAPON:
			str = "<huge weapon type>";
			break;

		case OPF_JUMP_NODE_NAME:
			str = "<Jump node name>";
			break;

		case OPF_NAV_POINT:
			str = "<Nav 1>";
			break;

		case OPF_ANYTHING:
			str = "<any data>";
			break;

		case OPF_DATA_OR_STR_CONTAINER:
			str = "<any data or string container>";
			break;

		case OPF_PERSONA:
			str = "<persona name>";
			break;

		case OPF_FONT:
			str = font::FontManager::getFont(0)->getName().c_str();
			break;

		case OPF_AUDIO_VOLUME_OPTION:
			str = "Music";
			break;

		case OPF_POST_EFFECT:
			str = "<Effect Name>";
			break;

		case OPF_CUSTOM_HUD_GAUGE:
			str = "<Custom hud gauge>";
			break;

		case OPF_ANY_HUD_GAUGE:
			str = "<Custom or builtin hud gauge>";
			break;

		case OPF_ANIMATION_NAME:
			str = "<Animation trigger name>";
			break;

		case OPF_CONTAINER_VALUE:
			str = "<container value>";
			break;

		case OPF_MESSAGE_TYPE:
			str = Builtin_messages[0].name;
			break;

		case OPF_VARIABLE_NAME:
			str = "<variable name>";
			break;

		case OPF_CONTAINER_NAME:
			str = "<container name>";
			break;

		case OPF_LIST_CONTAINER_NAME:
			str = "<list container name>";
			break;

		case OPF_MAP_CONTAINER_NAME:
			str = "<map container name>";
			break;

		default:
			str = "<new default required!>";
			break;
	}

	item->set_data(str, (SEXPT_STRING | SEXPT_VALID));
	return 0;
}
