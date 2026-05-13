#include "ui/util/ErrorChecker.h"

#include <ai/ai.h>
#include <coordinate_points/coordinate_point.h>
#include <ai/aigoals.h>
#include <globalincs/linklist.h>
#include <mission/missionbriefcommon.h>
#include <mission/missiongoals.h>
#include <mission/missionmessage.h>
#include <mission/missionparse.h>
#include <parse/sexp.h>
#include <missioneditor/common.h>
#include <model/model.h>
#include <object/object.h>
#include <object/objectdock.h>
#include <object/waypoint.h>
#include <ship/ship.h>

#include <algorithm>
#include <iterator>

#include <asteroid/asteroid.h>

#include "mission/Editor.h"
#include "mission/object.h"

namespace fso::fred {

ErrorChecker::ErrorChecker(EditorViewport* viewport) : _viewport(viewport) {}

bool ErrorChecker::runFullCheck() {
	_collected_errors.clear();
	_anchors_checked.clear();
	_object_names.clear();
	g_err = 0;

	// Surface any auto-corrections the parser had to apply at load time. These would
	// otherwise be invisible: parse-time mutates negative delays to 0, out-of-range
	// persona indices to -1, etc., so by the time the checker runs the live data is
	// already clean and the per-check predicates would never fire.
	for (const auto& msg : Mission_parse_warnings) {
		_collected_errors.push_back({msg, ErrorSeverity::Warning});
		g_err = 1;
	}
	Mission_parse_warnings.clear();

	if (checkObjectList() != 0)        return true;
	if (checkShips() != 0)             return true;
	if (checkWings() != 0)             return true;
	if (checkWaypointPaths() != 0)     return true;
	if (checkPlayerStarts() != 0)      return true;
	if (checkReinforcements() != 0)    return true;
	if (checkPlayerWings() != 0)       return true;
	if (checkTeamLoadout() != 0)       return true;
	if (checkMissionEvents() != 0)     return true;
	if (checkMissionGoals() != 0)      return true;
	if (checkBriefings() != 0)         return true;
	if (checkDebriefings() != 0)       return true;
	if (checkWingOrders() != 0)        return true;
	if (checkAsteroidTargets() != 0)   return true;
	if (checkDockingGroupCues() != 0)  return true;

	return g_err != 0;
}

bool ErrorChecker::runCheck(ErrorCheckType type, const ErrorCheckContext& ctx) {
	g_err = 0;
	_collected_errors.clear();
	_anchors_checked.clear();
	_object_names.clear();

	switch (type) {
	case ErrorCheckType::InitialOrders:     checkInitialOrders(ctx.goals, ctx.ship, ctx.wing); break;
	case ErrorCheckType::ObjectList:        checkObjectList(); break;
	case ErrorCheckType::Ships:             checkShips(); break;
	case ErrorCheckType::Wings:             checkWings(); break;         // calls populateNames() internally
	case ErrorCheckType::WaypointPaths:     checkWaypointPaths(); break; // calls populateNames() internally
	case ErrorCheckType::PlayerStarts:      checkPlayerStarts(); break;
	case ErrorCheckType::Reinforcements:    checkReinforcements(); break;
	case ErrorCheckType::PlayerWings:       checkPlayerWings(); break;
	case ErrorCheckType::MissionEvents:     checkMissionEvents(); break;
	case ErrorCheckType::MissionGoals:      checkMissionGoals(); break;
	case ErrorCheckType::Briefings:         checkBriefings(); break;
	case ErrorCheckType::Debriefings:       checkDebriefings(); break;
	case ErrorCheckType::WingOrders:        checkWingOrders(); break;
	case ErrorCheckType::AsteroidTargets:   checkAsteroidTargets(); break;
	case ErrorCheckType::DockingGroupCues:  checkDockingGroupCues(); break;
	case ErrorCheckType::TeamLoadout:       checkTeamLoadout(); break;
	}

	return g_err != 0;
}

const SCP_vector<ErrorEntry>& ErrorChecker::getErrors() const {
	return _collected_errors;
}

void ErrorChecker::error(const char* msg, ...) {
	char buf[2048];
	va_list args;

	va_start(args, msg);
	vsnprintf(buf, sizeof(buf) - 1, msg, args);
	va_end(args);
	buf[sizeof(buf) - 1] = '\0';

	g_err = 1;
	_collected_errors.push_back({buf, ErrorSeverity::Error});
}

int ErrorChecker::internal_error(const char* msg, ...) {
	SCP_string buf;
	va_list args;

	va_start(args, msg);
	vsprintf(buf, msg, args);
	va_end(args);

	g_err = 1;
	_collected_errors.push_back({buf, ErrorSeverity::InternalError});
	return -1;
}

void ErrorChecker::warning(const char* msg, ...) {
	char buf[2048];
	va_list args;

	va_start(args, msg);
	vsnprintf(buf, sizeof(buf) - 1, msg, args);
	va_end(args);
	buf[sizeof(buf) - 1] = '\0';

	g_err = 1;
	_collected_errors.push_back({buf, ErrorSeverity::Warning});
}

void ErrorChecker::potential(const char* msg, ...) {
	char buf[2048];
	va_list args;

	va_start(args, msg);
	vsnprintf(buf, sizeof(buf) - 1, msg, args);
	va_end(args);
	buf[sizeof(buf) - 1] = '\0';

	_collected_errors.push_back({buf, ErrorSeverity::Potential});
}

int ErrorChecker::fred_check_sexp(int sexp, int type, const char* location, ...) {
	SCP_string location_buf, sexp_buf, error_buf, bad_node_str, issue_msg;
	int err = 0, faulty_node;
	va_list args;

	va_start(args, location);
	vsprintf(location_buf, location, args);
	va_end(args);

	if (sexp == -1)
		return 0;

	int z = check_sexp_syntax(sexp, type, 1, &faulty_node);
	if (z) {
		convert_sexp_to_string(sexp_buf, sexp, SEXP_ERROR_CHECK_MODE);
		truncate_message_lines(sexp_buf, 30);

		stuff_sexp_text_string(bad_node_str, faulty_node, SEXP_ERROR_CHECK_MODE);
		if (!bad_node_str.empty())
			bad_node_str.pop_back();

		sprintf(error_buf,
				"Error in %s: %s\n\n%s\n\n(Bad node appears to be: %s)",
				location_buf.c_str(),
				sexp_error_message(z),
				sexp_buf.c_str(),
				bad_node_str.c_str());

		if (z < 0 && z > -100)
			err = 1;

		if (err)
			return internal_error("%s", error_buf.c_str());

		error("%s", error_buf.c_str());
	}

	{
		int potential_z = check_sexp_potential_issues(sexp, &faulty_node, issue_msg);
		if (potential_z) {
			convert_sexp_to_string(sexp_buf, sexp, SEXP_ERROR_CHECK_MODE);
			truncate_message_lines(sexp_buf, 30);

			stuff_sexp_text_string(bad_node_str, faulty_node, SEXP_ERROR_CHECK_MODE);
			if (!bad_node_str.empty())
				bad_node_str.pop_back();

			sprintf(error_buf,
					"Potential issue detected in %s:\n\n%s\n\n%s\n\n(Suspect node appears to be: %s)",
					location_buf.c_str(),
					issue_msg.c_str(),
					sexp_buf.c_str(),
					bad_node_str.c_str());

			_collected_errors.push_back({error_buf, ErrorSeverity::Potential});
		}
	}

	return 0;
}

void ErrorChecker::populateNames() {
	_object_names.clear();
	object* ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ObjectName entry;
		int i = ptr->instance;
		if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
			if (i >= 0 && i < MAX_SHIPS)
				entry.name = Ships[i].ship_name;
		} else if (ptr->type == OBJ_WAYPOINT) {
			int waypoint_num;
			waypoint_list* wp_list = find_waypoint_list_with_instance(i, &waypoint_num);
			if (wp_list != nullptr && waypoint_num >= 0 && (uint)waypoint_num < wp_list->get_waypoints().size()) {
				char buf[256];
				waypoint_stuff_name(buf, i);
				entry.name = buf;
			}
		}
		_object_names.push_back(std::move(entry));
		ptr = GET_NEXT(ptr);
	}
}

int ErrorChecker::checkObjectList() {

	int t = 0;
	_object_names.clear();
	object* ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		ObjectName entry;
		int i = ptr->instance;
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			if (i < 0 || i >= MAX_SHIPS) {
				return internal_error("An object has an illegal ship index");
			}

			int z = Ships[i].ship_info_index;
			if (!SCP_vector_inbounds(Ship_info, z)) {
				return internal_error("A ship has an illegal class");
			}

			if (ptr->type == OBJ_START) {
				t++;
				if (!(Ship_info[z].flags[Ship::Info_Flags::Player_ship])) {
					if (_viewport->Error_checker_apply_auto_corrections) {
						ptr->type = OBJ_SHIP;
						Player_starts--;
						t--;

						ensure_valid_player_start_shipnum();
					}
					warning("Invalid ship type for a player.%s",
							_viewport->Error_checker_apply_auto_corrections
								? "  Ship has been reset to non-player ship."
								: "  Can be auto-corrected to non-player ship.");
				}

				int count = 0;
				for (int primary_bank_weapon : Ships[i].weapons.primary_bank_weapons) {
					if (primary_bank_weapon >= 0) {
						count++;
					}
				}

				if (!count) {
					error("Player \"%s\" has no primary weapons.  Should have at least 1", Ships[i].ship_name);
				}
			}

			if (Ships[i].objnum != OBJ_INDEX(ptr)) {
				return internal_error("Object/ship references are corrupt");
			}

			entry.name = Ships[i].ship_name;
			int w = Ships[i].wingnum;
			if (w >= 0) {
				if (w >= MAX_WINGS) {
					return internal_error("A ship has an illegal wing index");
				}

				int j = Wings[w].wave_count;
				if (!j) {
					return internal_error("A ship is in a non-existent wing");
				}

				if (j < 0 || j > MAX_SHIPS_PER_WING) {
					return internal_error("Invalid number of ships in wing \"%s\"", Wings[w].name);
				}

				while (j--) {
					if (_viewport->editor->wing_objects[w][j] == OBJ_INDEX(ptr)) {
						break;
					}
				}

				if (j < 0) {
					return internal_error("Ship/wing references are corrupt");
				}

				if (strlen(Wings[w].wing_squad_filename) > 0) //-V805
				{
					if (The_mission.game_type & MISSION_TYPE_MULTI) {
						potential("Wing squad logos are not displayed in multiplayer games.");
					} else {
						if (ptr->type == OBJ_START) {
							potential("A squad logo was assigned to the player's wing.  The player's squad logo will be displayed instead of the wing squad logo on ships in this wing.");
						}
					}
				}
			}

			if ((Ships[i].flags[Ship::Ship_Flags::Kill_before_mission]) && (Ships[i].hotkey >= 0)) {
				potential("Ship flagged as \"destroy before mission start\" has a hotkey assignment");
			}

			if ((Ships[i].flags[Ship::Ship_Flags::Kill_before_mission]) && (ptr->type == OBJ_START)) {
				error("Player start flagged as \"destroy before mission start\"");
			}

			if ((Ships[i].flags[Ship::Ship_Flags::Kill_before_mission]) && (Ships[i].final_death_time < 0)) {
				error("Ship \"%s\" is flagged as \"destroy before mission start\" but has a negative destroy time",
					  Ships[i].ship_name);
			}
		} else if (ptr->type == OBJ_WAYPOINT) {
			int waypoint_num;
			waypoint_list* wp_list = find_waypoint_list_with_instance(i, &waypoint_num);

			if (wp_list == nullptr) {
				return internal_error("Object references an illegal waypoint path number");
			}

			if (waypoint_num < 0 || (uint)waypoint_num >= wp_list->get_waypoints().size()) {
				return internal_error("Object references an illegal waypoint number in path");
			}

			char buf[256];
			waypoint_stuff_name(buf, i);
			entry.name = buf;
		} else if (ptr->type == OBJ_JUMP_NODE || ptr->type == OBJ_PROP) {
			// nothing needed
		} else if (ptr->type == OBJ_COORDINATE_POINT) {
			auto* cp = find_coordinate_point_by_objnum(OBJ_INDEX(ptr));
			if (cp == nullptr) {
				return internal_error("Coordinate point object has no entry in Coordinate_points");
			}
			entry.name = cp->name;
		} else {
			return internal_error("An unknown object type (%d) was detected", ptr->type);
		}

		if (!entry.name.empty()) {
			for (const auto& existing : _object_names) {
				if (!existing.name.empty() && !stricmp(existing.name.c_str(), entry.name.c_str())) {
					return internal_error("Duplicate object names (%s)", existing.name.c_str());
				}
			}
		}

		_object_names.push_back(std::move(entry));
		ptr = GET_NEXT(ptr);
	}

	// t and Player_starts are decremented in lockstep when auto-correcting bad player
	// starts above, so any mismatch here indicates a data-integrity problem regardless
	// of the auto-correct setting.
	if (t != Player_starts) {
		return internal_error("Total number of player ships is incorrect");
	}

	if (static_cast<int>(_object_names.size()) != Num_objects) {
		return internal_error("Num_objects is incorrect");
	}

	return 0;
}

int ErrorChecker::checkShips() {
	int multi = (The_mission.game_type & MISSION_TYPE_MULTI) ? 1 : 0;

	int count = 0;
	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0) {
			count++;
			if (!query_valid_object(Ships[i].objnum)) {
				return internal_error("Ship uses an unused object");
			}

			int z = Objects[Ships[i].objnum].type;
			if ((z != OBJ_SHIP) && (z != OBJ_START)) {
				return internal_error("Object should be a ship, but isn't");
			}

			if (fred_check_sexp(Ships[i].arrival_cue, OPR_BOOL, "arrival cue of ship \"%s\"", Ships[i].ship_name)) {
				return -1;
			}

			if (fred_check_sexp(Ships[i].departure_cue, OPR_BOOL, "departure cue of ship \"%s\"", Ships[i].ship_name)) {
				return -1;
			}

			if (Ships[i].arrival_location != ArrivalLocation::AT_LOCATION) {
				if (!Ships[i].arrival_anchor.isValid()) {
					error("Ship \"%s\" requires a valid arrival target", Ships[i].ship_name);
				}

				if (Ships[i].arrival_location == ArrivalLocation::FROM_DOCK_BAY) {
					SCP_string anchor_message;
					check_anchor_for_hangar_bay(anchor_message, _anchors_checked, Ships[i].arrival_anchor, Ships[i].ship_name, true, true);
					if (!anchor_message.empty())
						error("%s", anchor_message.c_str());
				}
			}

			if (Ships[i].departure_location != DepartureLocation::AT_LOCATION) {
				if (!Ships[i].departure_anchor.isValid()) {
					error("Ship \"%s\" requires a valid departure target", Ships[i].ship_name);
				}
				if (Ships[i].departure_location == DepartureLocation::TO_DOCK_BAY) {
					SCP_string anchor_message;
					check_anchor_for_hangar_bay(anchor_message, _anchors_checked, Ships[i].departure_anchor, Ships[i].ship_name, true, false);
					if (!anchor_message.empty())
						error("%s", anchor_message.c_str());
				}
			}

			if (Ships[i].arrival_delay < 0) {
				error("Ship \"%s\" has a negative arrival delay", Ships[i].ship_name);
			}

			if (Ships[i].departure_delay < 0) {
				error("Ship \"%s\" has a negative departure delay", Ships[i].ship_name);
			}

			if (Ships[i].arrival_location != ArrivalLocation::AT_LOCATION &&
				Ships[i].arrival_location != ArrivalLocation::FROM_DOCK_BAY &&
				Ships[i].arrival_distance <= 0) {
				error("Arrival distance for ship \"%s\" must be greater than 0", Ships[i].ship_name);
			}

			if (Ships[i].flags[Ship::Ship_Flags::Force_shields_on] &&
				Ship_info[Ships[i].ship_info_index].flags[Ship::Info_Flags::Intrinsic_no_shields]) {
				potential("Ship \"%s\" has both \"force shields on\" and \"no shields\" set, which is inconsistent",
						  Ships[i].ship_name);
			}

			if (Ships[i].persona_index >= 0 && Ships[i].persona_index >= (int)Personas.size()) {
				potential("Ship \"%s\" has an invalid persona index", Ships[i].ship_name);
			}

			for (const auto& alt : Ships[i].s_alt_classes) {
				if (alt.ship_class >= 0 && !SCP_vector_inbounds(Ship_info, alt.ship_class)) {
					error("Ship \"%s\" has an invalid alternate class index", Ships[i].ship_name);
				}
			}

			int ai = Ships[i].ai_index;
			if (ai < 0 || ai >= MAX_AI_INFO) {
				return internal_error("AI index out of range for ship \"%s\"", Ships[i].ship_name);
			}

			if (Ai_info[ai].shipnum != i) {
				return internal_error("AI/ship references are corrupt");
			}

			if (Ai_info[ai].ai_class < 0 || Ai_info[ai].ai_class >= Num_ai_classes) {
				error("Ship \"%s\" has an invalid AI class", Ships[i].ship_name);
			}

			if (checkInitialOrders(Ai_info[ai].goals, i, -1) != 0)
				return -1;

			SCP_set<int> used_dockpoints;
			for (dock_instance* dock_ptr = Objects[Ships[i].objnum].dock_list; dock_ptr != nullptr;
				 dock_ptr = dock_ptr->next) {
				int obj = OBJ_INDEX(dock_ptr->docked_objp);

				if (!query_valid_object(obj)) {
					return internal_error("Ship \"%s\" initially docked with non-existent ship", Ships[i].ship_name);
				}

				if (Objects[obj].type != OBJ_SHIP && Objects[obj].type != OBJ_START) {
					return internal_error("Ship \"%s\" initially docked with non-ship object", Ships[i].ship_name);
				}

				int sp = get_ship_from_obj(obj);
				if (!ship_docking_valid(i, sp) && !ship_docking_valid(sp, i)) {
					return internal_error("Docking illegal between \"%s\" and \"%s\" (initially docked)",
										  Ships[i].ship_name,
										  Ships[sp].ship_name);
				}

				auto dock_list = Editor::get_docking_list(Ship_info[Ships[i].ship_info_index].model_num);
				int point = dock_ptr->dockpoint_used;
				if (point < 0 || point >= (int)dock_list.size()) {
					return internal_error("Invalid docker point (\"%s\" initially docked with \"%s\")",
										  Ships[i].ship_name,
										  Ships[sp].ship_name);
				} else if (!used_dockpoints.insert(point).second) {
					return internal_error("Ship \"%s\" has the same dockpoint used in multiple initial dock pairings",
										  Ships[i].ship_name);
				}

				dock_list = Editor::get_docking_list(Ship_info[Ships[sp].ship_info_index].model_num);
				point = dock_find_dockpoint_used_by_object(dock_ptr->docked_objp, &Objects[Ships[i].objnum]);
				if (point < 0 || point >= (int)dock_list.size()) {
					return internal_error("Invalid dockee point (\"%s\" initially docked with \"%s\")",
										  Ships[i].ship_name,
										  Ships[sp].ship_name);
				}
			}

			int w = Ships[i].wingnum;
			bool is_in_loadout_screen = (z == OBJ_START);
			if (!is_in_loadout_screen && w >= 0) {
				if (multi && The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
					for (const char* tvt_wing_name : TVT_wing_names) {
						if (!strcmp(Wings[w].name, tvt_wing_name)) {
							is_in_loadout_screen = true;
							break;
						}
					}
				} else {
					for (const char* starting_wing_name : Starting_wing_names) {
						if (!strcmp(Wings[w].name, starting_wing_name)) {
							is_in_loadout_screen = true;
							break;
						}
					}
				}
			}
			if (is_in_loadout_screen) {
				int illegal = 0;
				z = Ships[i].ship_info_index;
				for (int primary_bank_weapon : Ships[i].weapons.primary_bank_weapons) {
					if (primary_bank_weapon >= 0
						&& !Ship_info[z].allowed_weapons[primary_bank_weapon]) {
						illegal++;
					}
				}

				for (int secondary_bank_weapon : Ships[i].weapons.secondary_bank_weapons) {
					if (secondary_bank_weapon >= 0
						&& !Ship_info[z].allowed_weapons[secondary_bank_weapon]) {
						illegal++;
					}
				}

				if (illegal)
					error("%d illegal weapon(s) found on ship \"%s\"", illegal, Ships[i].ship_name);
			}

			// Orders accepted must be a subset of the class's default orders — older missions
			// sometimes saved orders from a different class. Auto-fix by discarding the extras.
			{
				const SCP_set<size_t>& default_orders =
					ship_get_default_orders_accepted(&Ship_info[Ships[i].ship_info_index]);
				SCP_set<size_t> extras;
				std::set_difference(Ships[i].orders_accepted.begin(), Ships[i].orders_accepted.end(),
									default_orders.begin(), default_orders.end(),
									std::inserter(extras, extras.begin()));
				if (!extras.empty()) {
					if (_viewport->Error_checker_apply_auto_corrections) {
						for (auto order : extras)
							Ships[i].orders_accepted.erase(order);
					}
					warning("Ship \"%s\" accepts orders that are not part of its class's default orders.%s",
							Ships[i].ship_name,
							_viewport->Error_checker_apply_auto_corrections
								? "  The extra orders have been removed."
								: "");
				}
			}

		}
	}

	if (count != ship_get_num_ships()) {
		return internal_error("num_ships is incorrect");
	}

	return 0;
}

int ErrorChecker::checkWings() {
	populateNames();

	int count = 0;
	for (int i = 0; i < MAX_WINGS; i++) {
		int team = -1;
		int j = Wings[i].wave_count;
		if (j) {
			count++;
			if (j < 0 || j > MAX_SHIPS_PER_WING) {
				return internal_error("Invalid number of ships in wing \"%s\"", Wings[i].name);
			}

			while (j--) {
				int obj = _viewport->editor->wing_objects[i][j];
				if (obj < 0 || obj >= MAX_OBJECTS) {
					return internal_error("Wing_objects has an illegal object index");
				}

				if (!query_valid_object(obj)) {
					return internal_error("Wing_objects references an unused object");
				}

				if (Objects[obj].type != OBJ_SHIP && Objects[obj].type != OBJ_START) {
					return internal_error("Wing_objects of \"%s\" references an illegal object type", Wings[i].name);
				}
				int sp = Objects[obj].instance;

				char buf[256];
				wing_bash_ship_name(buf, Wings[i].name, j + 1);
				if (stricmp(buf, Ships[sp].ship_name) != 0) {
					return internal_error("Ship \"%s\" in wing should be called \"%s\"",
										  Ships[sp].ship_name,
										  buf);
				}

				int ship_type = ship_query_general_type(sp);
				if (ship_type < 0 || !(Ship_types[ship_type].flags[Ship::Type_Info_Flags::AI_can_form_wing])) {
					potential("Ship \"%s\" is an illegal type to be in a wing", Ships[sp].ship_name);
				}

				if (Ships[sp].wingnum != i) {
					return internal_error("Wing/ship references are corrupt");
				}

				if (sp != Wings[i].ship_index[j]) {
					return internal_error("Ship/wing references are corrupt");
				}

				if (team < 0) {
					team = Ships[sp].team;
				} else if (team != Ships[sp].team && team < 999) {
					potential("Ship teams mixed within same wing (\"%s\")", Wings[i].name);
				}
			}

			if ((Wings[i].special_ship < 0) || (Wings[i].special_ship >= Wings[i].wave_count)) {
				return internal_error("Special ship out of range for \"%s\"", Wings[i].name);
			}

			if (Wings[i].num_waves < 0) {
				return internal_error("Number of waves for \"%s\" is negative", Wings[i].name);
			}

			if (Wings[i].threshold < 0) {
				return internal_error("Threshold for \"%s\" is invalid", Wings[i].name);
			}

			if (Wings[i].threshold + Wings[i].wave_count > MAX_SHIPS_PER_WING) {
				if (_viewport->Error_checker_apply_auto_corrections) {
					Wings[i].threshold = MAX_SHIPS_PER_WING - Wings[i].wave_count;
					warning("Threshold for wing \"%s\" is higher than allowed.  Reset to %d",
							Wings[i].name,
							Wings[i].threshold);
				} else {
					warning("Threshold for wing \"%s\" is higher than allowed.", Wings[i].name);
				}
			}

			for (const auto& entry : _object_names) {
				if (!entry.name.empty() && !stricmp(entry.name.c_str(), Wings[i].name)) {
					return internal_error("Wing name is also used by an object (%s)", entry.name.c_str());
				}
			}

			if (fred_check_sexp(Wings[i].arrival_cue, OPR_BOOL, "arrival cue of wing \"%s\"", Wings[i].name)) {
				return -1;
			}

			if (fred_check_sexp(Wings[i].departure_cue, OPR_BOOL, "departure cue of wing \"%s\"", Wings[i].name)) {
				return -1;
			}

			if (Wings[i].arrival_location != ArrivalLocation::AT_LOCATION) {
				if (!Wings[i].arrival_anchor.isValid()) {
					error("Wing \"%s\" requires a valid arrival target", Wings[i].name);
				}
				if (Wings[i].arrival_location == ArrivalLocation::FROM_DOCK_BAY) {
					SCP_string anchor_message;
					check_anchor_for_hangar_bay(anchor_message, _anchors_checked, Wings[i].arrival_anchor, Wings[i].name, false, true);
					if (!anchor_message.empty())
						error("%s", anchor_message.c_str());
				}
			}

			if (Wings[i].departure_location != DepartureLocation::AT_LOCATION) {
				if (!Wings[i].departure_anchor.isValid()) {
					error("Wing \"%s\" requires a valid departure target", Wings[i].name);
				}
				if (Wings[i].departure_location == DepartureLocation::TO_DOCK_BAY) {
					SCP_string anchor_message;
					check_anchor_for_hangar_bay(anchor_message, _anchors_checked, Wings[i].departure_anchor, Wings[i].name, false, false);
					if (!anchor_message.empty())
						error("%s", anchor_message.c_str());
				}
			}

			if (Wings[i].arrival_delay < 0) {
				error("Wing \"%s\" has a negative arrival delay", Wings[i].name);
			}

			if (Wings[i].departure_delay < 0) {
				error("Wing \"%s\" has a negative departure delay", Wings[i].name);
			}

			if (Wings[i].arrival_location != ArrivalLocation::AT_LOCATION &&
				Wings[i].arrival_location != ArrivalLocation::FROM_DOCK_BAY &&
				Wings[i].arrival_distance <= 0) {
				error("Arrival distance for wing \"%s\" must be greater than 0", Wings[i].name);
			}

			if (Wings[i].formation >= 0 && Wings[i].formation >= (int)Wing_formations.size()) {
				potential("Wing \"%s\" has an invalid formation", Wings[i].name);
			}

			{
				bool has_player = false;
				for (int k = 0; k < Wings[i].wave_count; k++) {
					if (Objects[Ships[Wings[i].ship_index[k]].objnum].type == OBJ_START) {
						has_player = true;
						break;
					}
				}
				if (has_player && Wings[i].arrival_delay > 0) {
					potential("Wing \"%s\" contains a player start but has a non-zero arrival delay; the delay will be ignored",
							  Wings[i].name);
				}
			}

			if (checkInitialOrders(Wings[i].ai_goals, -1, i) != 0)
				return -1;
		}
	}

	if (count != Num_wings) {
		return internal_error("Num_wings is incorrect");
	}

	return 0;
}

int ErrorChecker::checkWaypointPaths() {
	populateNames();

	for (const auto& ii : Waypoint_lists) {
		for (const auto& entry : _object_names) {
			if (!entry.name.empty() && !stricmp(entry.name.c_str(), ii.get_name())) {
				return internal_error("Waypoint path name is also used by an object (%s)", entry.name.c_str());
			}
		}

		for (const auto& jj : ii.get_waypoints()) {
			char buf[256];
			waypoint_stuff_name(buf, jj);
			bool found = false;
			for (const auto& entry : _object_names) {
				if (!entry.name.empty() && !stricmp(entry.name.c_str(), buf)) {
					found = true;
					break;
				}
			}

			if (!found) {
				return internal_error("Waypoint \"%s\" not linked to an object", buf);
			}
		}
	}

	return 0;
}

int ErrorChecker::checkPlayerStarts() {
	int multi = (The_mission.game_type & MISSION_TYPE_MULTI) ? 1 : 0;

	if (Player_starts > MAX_PLAYERS) {
		return internal_error("Number of player starts exceeds max limit");
	}

	if (!multi && (Player_starts > 1)) {
		error("Multiple player starts exist, but this is a single player mission");
	}

	return 0;
}

int ErrorChecker::checkReinforcements() {

	for (const auto& reinforcement : Reinforcements) {
		if (reinforcement.arrival_delay < 0) {
			error("Reinforcement \"%s\" has a negative arrival delay", reinforcement.name);
		}

		int z = 0;
		int ship_wingnum = -1;
		for (const auto& ship : Ships) {
			if ((ship.objnum >= 0) && !stricmp(ship.ship_name, reinforcement.name)) {
				z = 1;
				ship_wingnum = ship.wingnum;
				break;
			}
		}

		for (const auto& wing : Wings) {
			if (wing.wave_count && !stricmp(wing.name, reinforcement.name)) {
				z = 1;
				break;
			}
		}

		if (!z) {
			return internal_error("Reinforcement name not found in ships or wings");
		}

		if (ship_wingnum >= 0) {
			potential("Reinforcement \"%s\" is a ship that belongs to a wing; the reinforcement flag will be ignored",
					  reinforcement.name);
		}
	}

	return 0;
}

int ErrorChecker::checkPlayerWings() {
	if (Player_start_shipnum < 0 || Player_start_shipnum >= MAX_SHIPS || Ships[Player_start_shipnum].objnum < 0) {
		return internal_error("Mission has no valid player start ship");
	}

	const int multi = (The_mission.game_type & MISSION_TYPE_MULTI) ? 1 : 0;

	// The first starting wing and the first TVT wing must share a name — missionparse/post_process_ships_wings
	// treats this as a fatal Error() in-game, so the mission will not run otherwise.
	if (strcmp(Starting_wing_names[0], TVT_wing_names[0]) != 0) {
		error("The first starting wing (\"%s\") and the first team-versus-team wing (\"%s\") must have the same name",
			  Starting_wing_names[0], TVT_wing_names[0]);
	}

	auto checkMixedSpecies = [this](int w) {
		int species = -1;
		bool mixed = false;
		for (int i = 0; i < Wings[w].wave_count; i++) {
			int s = Wings[w].ship_index[i];
			if (species < 0)
				species = Ship_info[Ships[s].ship_info_index].species;
			else if (Ship_info[Ships[s].ship_info_index].species != species)
				mixed = true;
		}
		if (mixed)
			error("%s wing must all be of the same species", Wings[w].name);
	};

	// In non-TVT multiplayer, squadron wings must be contiguous from the front — if wing[i] exists
	// but wing[i-1] doesn't, wings can disappear at runtime (missionparse.cpp line 5605).
	if (multi && !(The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)) {
		bool found[MAX_STARTING_WINGS] = {};
		for (int i = 0; i < MAX_STARTING_WINGS; i++) {
			for (const auto& wing : Wings) {
				if (wing.wave_count && !strcmp(wing.name, Squadron_wing_names[i])) {
					found[i] = true;
					break;
				}
			}
		}
		for (int i = 1; i < MAX_STARTING_WINGS; i++) {
			if (found[i] && !found[i - 1]) {
				potential("Squadron wings are not in the correct order: wing \"%s\" exists but \"%s\" does not; this may cause wings to disappear in multiplayer",
						  Squadron_wing_names[i], Squadron_wing_names[i - 1]);
			}
		}
	}

	if (multi && The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		for (const char* tvt_wing_name : TVT_wing_names) {
			if (ship_tvt_wing_lookup(tvt_wing_name) == -1) {
				error("%s wing is required for multiplayer team vs. team missions", tvt_wing_name);
			}
		}
	}

	if (multi) {
		if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
			for (int i = 0; i < MAX_TVT_WINGS; i++) {
				if (TVT_wings[i] >= 0 && Wings[TVT_wings[i]].num_waves > 1) {
					if (_viewport->Error_checker_apply_auto_corrections)
						Wings[TVT_wings[i]].num_waves = 1;
					warning("%s wing must contain only 1 wave.%s", TVT_wing_names[i],
							_viewport->Error_checker_apply_auto_corrections
								? "  Reset to 1 wave."
								: "  Can be auto-corrected to 1 wave.");
				}
			}
		} else {
			for (int i = 0; i < MAX_STARTING_WINGS; i++) {
				if (Starting_wings[i] >= 0 && Wings[Starting_wings[i]].num_waves > 1) {
					if (_viewport->Error_checker_apply_auto_corrections)
						Wings[Starting_wings[i]].num_waves = 1;
					warning("%s wing must contain only 1 wave.%s", Starting_wing_names[i],
							_viewport->Error_checker_apply_auto_corrections
								? "  Reset to 1 wave."
								: "  Can be auto-corrected to 1 wave.");
				}
			}
		}
	}

	if (multi && The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		for (int i = 0; i < MAX_TVT_WINGS; i++) {
			if (TVT_wings[i] >= 0 && Wings[TVT_wings[i]].wave_count > 4) {
				error("%s wing has too many ships.  Should only have 4 max.", TVT_wing_names[i]);
			}
		}
	} else {
		for (int i = 0; i < MAX_STARTING_WINGS; i++) {
			if (Starting_wings[i] >= 0 && Wings[Starting_wings[i]].wave_count > 4) {
				error("%s wing has too many ships.  Should only have 4 max.", Starting_wing_names[i]);
			}
		}
	}

	if (multi && The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		for (int i = 0; i < MAX_TVT_WINGS; i++) {
			if (TVT_wings[i] >= 0 && Wings[TVT_wings[i]].arrival_delay > 0) {
				potential("%s wing shouldn't have a non-zero arrival delay", TVT_wing_names[i]);
			}
		}
	}

	if (multi) {
		if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
			for (int tvt_wing : TVT_wings) {
				if (tvt_wing >= 0) {
					checkMixedSpecies(tvt_wing);
				}
			}
		} else {
			for (int starting_wing : Starting_wings) {
				if (starting_wing >= 0) {
					checkMixedSpecies(starting_wing);
				}
			}
		}
	}

	int starting_wing_count[MAX_STARTING_WINGS];
	int tvt_wing_count[MAX_TVT_WINGS];
	SCP_string starting_wing_list;
	SCP_string tvt_wing_list;

	for (int i = 0; i < MAX_STARTING_WINGS; i++) {
		starting_wing_count[i] = 0;

		if (i < MAX_STARTING_WINGS - 1) {
			starting_wing_list += Starting_wing_names[i];
			if (MAX_STARTING_WINGS > 2)
				starting_wing_list += ",";
			starting_wing_list += " ";
		} else {
			starting_wing_list += "or ";
			starting_wing_list += Starting_wing_names[i];
		}
	}
	for (int i = 0; i < MAX_TVT_WINGS; i++) {
		tvt_wing_count[i] = 0;

		if (i < MAX_TVT_WINGS - 1) {
			tvt_wing_list += TVT_wing_names[i];
			if (MAX_TVT_WINGS > 2)
				tvt_wing_list += ",";
			tvt_wing_list += " ";
		} else {
			tvt_wing_list += "or ";
			tvt_wing_list += TVT_wing_names[i];
		}
	}

	object* ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		int ship_instance = ptr->instance;
		int err = 0;

		if (ptr->type == OBJ_START) {
			int z = Ships[ship_instance].wingnum;
			if (z < 0) {
				err = 1;
			} else {
				int in_starting_wing = 0;
				int in_tvt_wing = 0;

				for (int i = 0; i < MAX_STARTING_WINGS; i++) {
					if (Starting_wings[i] == z) {
						in_starting_wing = 1;
						starting_wing_count[i]++;
					}
				}
				for (int i = 0; i < MAX_TVT_WINGS; i++) {
					if (TVT_wings[i] == z) {
						in_tvt_wing = 1;
						tvt_wing_count[i]++;
					}
				}

				if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
					if (!in_tvt_wing)
						err = 1;
				} else {
					if (!in_starting_wing)
						err = 1;
				}
			}

			if (err) {
				if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
					error("Player %s should be part of %s wing", Ships[ship_instance].ship_name, tvt_wing_list.c_str());
				} else {
					error("Player %s should be part of %s wing", Ships[ship_instance].ship_name, starting_wing_list.c_str());
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		for (int i = 0; i < MAX_TVT_WINGS; i++) {
			if (!tvt_wing_count[i]) {
				error("%s wing does not contain any players (which it should)", TVT_wing_names[i]);
			}
		}
	}

	return 0;
}

int ErrorChecker::checkMissionEvents() {
	for (const auto& event : Mission_events) {
		if (event.repeat_count == 0) {
			error("Mission event \"%s\" has a repeat count of 0; must be at least 1 (or negative for unlimited)",
				  event.name.c_str());
		}

		if (event.trigger_count == 0) {
			error("Mission event \"%s\" has a trigger count of 0; must be at least 1 (or negative for unlimited)",
				  event.name.c_str());
		}

		if (fred_check_sexp(event.formula, OPR_NULL, "mission event \"%s\"", event.name.c_str())) {
			return -1;
		}
	}
	return 0;
}

int ErrorChecker::checkMissionGoals() {
	for (const auto& goal : Mission_goals) {
		if (fred_check_sexp(goal.formula, OPR_BOOL, "mission goal \"%s\"", goal.name.c_str())) {
			return -1;
		}
	}
	return 0;
}

int ErrorChecker::checkBriefings() {

	for (int bs = 0; bs < Num_teams; bs++) {
		for (int s = 0; s < Briefings[bs].num_stages; s++) {
			brief_stage* sp = &Briefings[bs].stages[s];

			if (fred_check_sexp(sp->formula, OPR_BOOL, "briefing stage %d (team %d)", s + 1, bs + 1)) {
				return -1;
			}

			int t = sp->num_icons;
			for (int i = 0; i < t - 1; i++) {
				for (int j = i + 1; j < t; j++) {
					if ((sp->icons[i].id > 0) && (sp->icons[i].id == sp->icons[j].id)) {
						potential("Duplicate icon IDs %d in briefing stage %d", sp->icons[i].id, s + 1);
					}
				}
			}
		}
	}
	return 0;
}

int ErrorChecker::checkDebriefings() {
	for (int j = 0; j < Num_teams; j++) {
		for (int i = 0; i < Debriefings[j].num_stages; i++) {
			if (fred_check_sexp(Debriefings[j].stages[i].formula, OPR_BOOL, "debriefing stage %d", i + 1)) {
				return -1;
			}
		}
	}
	return 0;
}

int ErrorChecker::checkWingOrders() {

	for (const auto& wing : Wings) {
		if (!wing.wave_count) {
			continue;
		}

		int starting_wing = (ship_starting_wing_lookup(wing.name) != -1);

		if (starting_wing && (wing.flags[Ship::Wing_Flags::Reinforcement])) {
			error("Starting Wing %s marked as reinforcement.  This wing\nshould either be renamed, or unmarked as reinforcement.",
				  wing.name);
		}

		std::set<size_t> default_orders;
		int default_orders_idx = -1;
		for (int j = 0; j < wing.wave_count; j++) {
			if (Objects[Ships[wing.ship_index[j]].objnum].type == OBJ_START) {
				continue;
			}

			const std::set<size_t>& orders = Ships[wing.ship_index[j]].orders_accepted;

			if (default_orders_idx < 0) {
				default_orders_idx = j;
				default_orders = orders;
			} else if (default_orders != orders) {
				potential("%s and %s will accept different orders. All ships in a wing must accept the same Player Orders.",
						  Ships[wing.ship_index[j]].ship_name,
						  Ships[wing.ship_index[default_orders_idx]].ship_name);
			}
		}
	}

	return 0;
}

int ErrorChecker::checkAsteroidTargets() {
	for (const auto& name : Asteroid_field.target_names) {
		if (ship_name_lookup(name.c_str(), 1) < 0) {
			error("Asteroid target '%s' is not a valid ship", name.c_str());
		}
	}
	return 0;
}

int ErrorChecker::checkDockingGroupCues() {
	SCP_set<int> visited; // ship indices already accounted for

	for (int i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum < 0) continue;
		if (!query_valid_object(Ships[i].objnum)) continue;
		if (!object_is_docked(&Objects[Ships[i].objnum])) continue;
		if (visited.count(i)) continue;

		// BFS to collect all ships in this docking group
		SCP_vector<int> group;
		SCP_vector<int> queue = {i};
		while (!queue.empty()) {
			int si = queue.back();
			queue.pop_back();
			if (visited.count(si)) continue;
			visited.insert(si);
			group.push_back(si);

			for (dock_instance* dp = Objects[Ships[si].objnum].dock_list; dp != nullptr; dp = dp->next) {
				int docked_obj = OBJ_INDEX(dp->docked_objp);
				if (Objects[docked_obj].type == OBJ_SHIP || Objects[docked_obj].type == OBJ_START) {
					int docked_ship = Objects[docked_obj].instance;
					if (!visited.count(docked_ship))
						queue.push_back(docked_ship);
				}
			}
		}

		// Count non-false arrival cues; winged ships share the wing cue
		SCP_set<int> checked_wings;
		int non_false_count = 0;
		for (int si : group) {
			int w = Ships[si].wingnum;
			if (w >= 0 && w < MAX_WINGS) {
				if (!checked_wings.insert(w).second) continue;
				if (Wings[w].arrival_cue != Locked_sexp_false)
					non_false_count++;
			} else {
				if (Ships[si].arrival_cue != Locked_sexp_false)
					non_false_count++;
			}
		}

		if (non_false_count == 0) {
			error("Docking group containing \"%s\" has no ship with a non-false arrival cue; the group will not appear in-mission",
				  Ships[i].ship_name);
		} else if (non_false_count > 1) {
			error("Docking group containing \"%s\" has more than one non-false arrival cue; only one is allowed",
				  Ships[i].ship_name);
		}
	}

	return 0;
}

int ErrorChecker::checkTeamLoadout() {
	for (int i = 0; i < Num_teams; i++) {
		// Respect the per-team "Bypass Loadout Validation" flag (set in the loadout editor).
		if (Team_data[i].do_not_validate)
			continue;

		// Build a fresh usage list for this team's starting wings.
		int usage[MAX_WEAPON_TYPES];
		_viewport->editor->generate_team_weaponry_usage_list(i, usage);

		// Zero out weapons that are accounted for in the loadout pool, so that
		// only weapons missing from the pool remain non-zero.
		for (int j = 0; j < Team_data[i].num_weapon_choices; j++) {
			int wi = Team_data[i].weaponry_pool[j];
			if (wi >= 0 && wi < MAX_WEAPON_TYPES)
				usage[wi] = 0;
		}

		// Any non-zero entry is a weapon used in wings but absent from the loadout.
		for (int j = 0; j < MAX_WEAPON_TYPES; j++) {
			if (usage[j] <= 0)
				continue;

			if (_viewport->Error_checker_apply_auto_corrections && Team_data[i].num_weapon_choices < MAX_WEAPON_TYPES) {
				// Add the missing weapon to the pool so the mission is structurally valid.
				int slot = Team_data[i].num_weapon_choices;
				Team_data[i].weaponry_pool[slot] = j;
				Team_data[i].weaponry_count[slot] = usage[j];
				strcpy_s(Team_data[i].weaponry_amount_variable[slot], "");
				strcpy_s(Team_data[i].weaponry_pool_variable[slot], "");
				Team_data[i].num_weapon_choices++;
				warning("Weapon \"%s\" is used in wings of team %d but was not in the team loadout pool — added automatically.",
						Weapon_info[j].name, i + 1);
			} else {
				warning("Weapon \"%s\" is used in wings of team %d but is not in the team loadout pool — can be auto-corrected by adding it.",
						Weapon_info[j].name, i + 1);
			}
		}
	}
	return 0;
}

int ErrorChecker::checkInitialOrders(ai_goal* goals, int ship, int wing) {
	if (_viewport->editor == nullptr) {
		return internal_error("checkInitialOrders requires a valid editor");
	}

	auto get_order_name = [](ai_goal_mode order) -> const char* {
		if (order == AI_GOAL_NONE)
			return "None";
		const ai_goal_list* list = Editor::getAi_goal_list();
		int size = Editor::getAigoal_list_size();
		for (int i = 0; i < size; i++)
			if (list[i].def == order)
				return list[i].name;
		return "???";
	};

	int team, team2;
	char* source;
	const char* entity = (ship >= 0) ? "ship" : "wing";

	if (ship >= 0) {
		source = Ships[ship].ship_name;
		team = Ships[ship].team;
		for (int i = 0; i < MAX_AI_GOALS; i++) {
			if (!ai_query_goal_valid(ship, goals[i].ai_mode))
				potential("Order \"%s\" is not allowed for ship \"%s\"", get_order_name(goals[i].ai_mode), source);
		}
	} else {
		Assert(wing >= 0);
		Assert(Wings[wing].wave_count > 0);
		source = Wings[wing].name;
		team = Ships[Objects[_viewport->editor->wing_objects[wing][0]].instance].team;
		for (int j = 0; j < Wings[wing].wave_count; j++) {
			for (int i = 0; i < MAX_AI_GOALS; i++) {
				if (!ai_query_goal_valid(Wings[wing].ship_index[j], goals[i].ai_mode))
					potential("Order \"%s\" is not allowed for ship \"%s\"", get_order_name(goals[i].ai_mode),
							  Ships[Wings[wing].ship_index[j]].ship_name);
			}
		}
	}

	int flag, found;
	for (int i = 0; i < MAX_AI_GOALS; i++) {
		switch (goals[i].ai_mode) {
		case AI_GOAL_NONE:
		case AI_GOAL_CHASE_ANY:
		case AI_GOAL_CHASE_SHIP_CLASS:
		case AI_GOAL_UNDOCK:
		case AI_GOAL_KEEP_SAFE_DISTANCE:
		case AI_GOAL_PLAY_DEAD:
		case AI_GOAL_PLAY_DEAD_PERSISTENT:
		case AI_GOAL_WARP:
			flag = 0;
			break;

		case AI_GOAL_WAYPOINTS:
		case AI_GOAL_WAYPOINTS_ONCE:
			flag = 1;
			break;

		case AI_GOAL_DOCK:
			if (ship < 0) {
				error("Initial orders error for wing \"%s\"\n\nWings cannot dock", source);
				continue;
			}
			FALLTHROUGH;

		case AI_GOAL_DESTROY_SUBSYSTEM:
		case AI_GOAL_CHASE:
		case AI_GOAL_GUARD:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_DISARM_SHIP_TACTICAL:
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_DISABLE_SHIP_TACTICAL:
		case AI_GOAL_EVADE_SHIP:
		case AI_GOAL_STAY_NEAR_SHIP:
		case AI_GOAL_FORM_ON_WING:
		case AI_GOAL_IGNORE:
		case AI_GOAL_IGNORE_NEW:
			flag = 2;
			break;

		case AI_GOAL_CHASE_WING:
		case AI_GOAL_GUARD_WING:
			flag = 3;
			break;

		case AI_GOAL_STAY_STILL:
			flag = 4;
			break;

		default:
			return internal_error("Initial orders error for %s \"%s\"\n\nInvalid goal type", entity, source);
		}

		found = 0;
		if (flag > 0) {
			if (*goals[i].target_name == '<') {
				error("Initial orders error for %s \"%s\"\n\nInvalid target", entity, source);
				continue;
			}

			if (!stricmp(goals[i].target_name, source)) {
				error("Initial orders error for %s \"%s\"\n\nTarget of %s's goal is itself", entity, source, entity);
				continue;
			}
		}

		int inst = team2 = -1;
		if (flag == 1) {
			if (find_matching_waypoint_list(goals[i].target_name) == nullptr)
				return internal_error("Initial orders error for %s \"%s\"\n\nInvalid target waypoint path name", entity, source);

		} else if (flag == 2) {
			object* ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
					inst = ptr->instance;
					if (!stricmp(goals[i].target_name, Ships[inst].ship_name)) {
						found = 1;
						break;
					}
				}
				ptr = GET_NEXT(ptr);
			}

			if (!found)
				return internal_error("Initial orders error for %s \"%s\"\n\nInvalid target ship name", entity, source);

			if (wing >= 0) {
				if (Ships[inst].wingnum == wing && Objects[Ships[inst].objnum].type != OBJ_START) {
					error("Initial orders error for wing \"%s\"\n\nTarget ship of wing's goal is within said wing", source);
					continue;
				}
			}

			team2 = Ships[inst].team;

		} else if (flag == 3) {
			int j;
			for (j = 0; j < MAX_WINGS; j++)
				if (Wings[j].wave_count && !stricmp(Wings[j].name, goals[i].target_name))
					break;

			if (j >= MAX_WINGS)
				return internal_error("Initial orders error for %s \"%s\"\n\nInvalid target wing name", entity, source);

			if (ship >= 0) {
				if (Ships[ship].wingnum == j) {
					error("Initial orders error for ship \"%s\"\n\nTarget wing of ship's goal is same wing said ship is part of", source);
					continue;
				}
			}

			team2 = Ships[Objects[_viewport->editor->wing_objects[j][0]].instance].team;

		} else if (flag == 4) {
			object* ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
					inst = ptr->instance;
					if (!stricmp(goals[i].target_name, Ships[inst].ship_name)) {
						found = 2;
						break;
					}
				} else if (ptr->type == OBJ_WAYPOINT) {
					if (!stricmp(goals[i].target_name, object_name(OBJ_INDEX(ptr)))) {
						found = 1;
						break;
					}
				}
				ptr = GET_NEXT(ptr);
			}

			if (!found)
				return internal_error("Initial orders error for %s \"%s\"\n\nInvalid target ship or waypoint name", entity, source);

			if (found == 2) {
				if (wing >= 0) {
					if (Ships[inst].wingnum == wing && Objects[Ships[inst].objnum].type != OBJ_START) {
						error("Initial orders error for wing \"%s\"\n\nTarget ship of wing's goal is within said wing", source);
						continue;
					}
				}
				team2 = Ships[inst].team;
			}
		}

		switch (goals[i].ai_mode) {
		case AI_GOAL_DESTROY_SUBSYSTEM:
			Assert(flag == 2 && inst >= 0); // NOLINT(readability-simplify-boolean-expr)
			if (ship_find_subsys(&Ships[inst], goals[i].docker.name) < 0) {
				potential("Initial orders error for ship \"%s\"\n\nUnknown subsystem type", source);
				continue;
			}
			break;

		case AI_GOAL_DOCK: {
			int dock1 = -1, dock2 = -1, model1, model2;

			Assert(flag == 2 && inst >= 0); // NOLINT(readability-simplify-boolean-expr)
			if (!ship_docking_valid(ship, inst)) {
				error("Initial orders error for ship \"%s\"\n\nDocking illegal between given ship types", source);
				continue;
			}

			model1 = Ship_info[Ships[ship].ship_info_index].model_num;
			auto model1Docks = Editor::get_docking_list(model1);
			for (int j = 0; j < (int)model1Docks.size(); ++j) {
				if (!stricmp(goals[i].docker.name, model1Docks[j].c_str())) {
					dock1 = j;
					break;
				}
			}

			model2 = Ship_info[Ships[inst].ship_info_index].model_num;
			auto model2Docks = Editor::get_docking_list(model2);
			for (int j = 0; j < (int)model2Docks.size(); ++j) {
				if (!stricmp(goals[i].dockee.name, model2Docks[j].c_str())) {
					dock2 = j;
					break;
				}
			}

			if (dock1 < 0) {
				error("Initial orders error for ship \"%s\"\n\nInvalid docker point", source);
				continue;
			}

			if (dock2 < 0) {
				error("Initial orders error for ship \"%s\"\n\nInvalid dockee point", source);
				continue;
			}

			if (!(model_get_dock_index_type(model1, dock1) & model_get_dock_index_type(model2, dock2))) {
				error("Initial orders error for ship \"%s\"\n\nDock points are incompatible", source);
			}

			break;
		}

		default:
			break;
		}

		switch (goals[i].ai_mode) {
		case AI_GOAL_GUARD:
		case AI_GOAL_GUARD_WING:
			if (team != team2)
				potential("Initial orders error for %s \"%s\"\n\n%s assigned to guard a different team",
						  entity, source, ship >= 0 ? "Ship" : "Wing");
			break;

		case AI_GOAL_CHASE:
		case AI_GOAL_CHASE_WING:
		case AI_GOAL_DESTROY_SUBSYSTEM:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_DISARM_SHIP_TACTICAL:
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_DISABLE_SHIP_TACTICAL:
			if (team == team2)
				potential("Initial orders error for %s \"%s\"\n\n%s assigned to attack same team",
						  entity, source, ship >= 0 ? "Ship" : "Wing");
			break;

		default:
			break;
		}
	}

	return 0;
}

} // namespace fso::fred
