#include "ShipEditorDialogModel.h"

#include "iff_defs/iff_defs.h"
#include "mission/missionmessage.h"
#include "ui/dialogs/ShipEditorDialog.h"

#include <globalincs\linklist.h>
#include <mission/object.h>
#include <jumpnode/jumpnode.h>

namespace fso {
namespace fred {
namespace dialogs {
ShipEditorDialogModel::ShipEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

int ShipEditorDialogModel::tristate_set(int val, int cur_state)
{
	if (val) {
		if (!cur_state) {
			return 2;
		}

	} else {
		if (cur_state) {
			return 2;
		}
	}

	return cur_state;
}

void ShipEditorDialogModel::initializeData()
{
	int type, wing = -1;
	int a_cue, d_cue, cargo = 0, base_player = 0, pship = -1;
	int no_arrival_warp = 0, no_departure_warp = 0, escort_count = 0,  current_orders = 0;
	cue_init = 0;
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		mission_type = 0; // multi player mission
	} else {
		mission_type = 1; // non-multiplayer mission (implies single player mission I guess)
	}
	ship_count = player_count = escort_count = pship_count = pvalid_count = 0;
	base_ship = base_player = -1;
	auto ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) && (Ships[ptr->instance].flags[Ship::Ship_Flags::Escort])) {
			escort_count++; // get a total count of escort ships
		}

		if (ptr->type == OBJ_START) {
			pship_count++; // count all player starts in mission
		}

		if (ptr->flags[Object::Object_Flags::Marked]) {
			type = ptr->type;
			if ((type == OBJ_START) && !mission_type) { // in multiplayer missions, starts act like ships
				type = OBJ_SHIP;
			}

			auto i = -1;
			if (type == OBJ_START) {
				player_count++;
				// if player_count is 1, base_player will be the one and only player
				i = base_player = ptr->instance;

			} else if (type == OBJ_SHIP) {
				ship_count++;
				// if ship_count is 1, base_ship will be the one and only ship
				i = base_ship = ptr->instance;
			}

			if (i >= 0) {
				if (Ship_info[Ships[i].ship_info_index].flags[Ship::Info_Flags::Player_ship]) {
					pvalid_count++;
				}
			}
		}
		ptr = GET_NEXT(ptr);
	}

	total_count = ship_count + player_count; // get total number of objects being edited.
	if (total_count > 1) {
		multi_edit = 1;
	} else {
		multi_edit = 0;
	}

	a_cue = d_cue = -1;
	_m_arrival_location = -1;
	_m_arrival_dist = -1;
	_m_arrival_target = -1;
	_m_arrival_delay = -1;
	_m_departure_location = -1;
	_m_departure_target = -1;
	_m_departure_delay = -1;

	player_ship = single_ship = -1;
	_m_arrival_tree.select_sexp_node = _m_departure_tree.select_sexp_node = select_sexp_node;
	select_sexp_node = -1;
	ship_orders = 0; // assume they are all the same type
	if (ship_count) {
		if (!multi_edit) {
			Assert((ship_count == 1) && (base_ship >= 0));
			_m_ship_name = Ships[base_ship].ship_name;
		} else {
			_m_ship_name = "";
		}

		_m_update_arrival = _m_update_departure = 1;
		base_player = 0;
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if ((ptr->type == OBJ_START) || (ptr->type == OBJ_SHIP)) {
				if (ptr->flags[Object::Object_Flags::Marked]) {
					// do processing for both ships and players
					auto i = get_ship_from_obj(ptr);
					if (base_player >= 0) {
						_m_ship_class = Ships[i].ship_info_index;
						_m_team = Ships[i].team;
						pship = (ptr->type == OBJ_START) ? 1 : 0;
						base_player = -1;

					} else {
						if (Ships[i].ship_info_index != _m_ship_class)
							_m_ship_class = -1;
						if (Ships[i].team != _m_team)
							_m_team = -1;

						pship = tristate_set(Objects[Ships[i].objnum].type == OBJ_START, pship);
					}

					// 'and' in the ship type of this ship to our running bitfield
					current_orders = ship_get_default_orders_accepted(&Ship_info[Ships[i].ship_info_index]);
					if (!ship_orders) {
						ship_orders = current_orders;
					} else if (ship_orders != current_orders) {
						ship_orders = -1;
					}

					if (Ships[i].flags[Ship::Ship_Flags::Escort]) {
						escort_count--; // remove marked escorts from count
					}

					if (Objects[Ships[i].objnum].type == OBJ_START) {
						pship_count--; // removed marked starts from count
					}

					// do processing only for ships (plus players if in a multiplayer mission
					if ((ptr->type == OBJ_SHIP) || ((ptr->type == OBJ_START) && !mission_type)) {
						// process this if ship not in a wing
						if (Ships[i].wingnum < 0) {
							if (!cue_init) {
								cue_init = 1;
								a_cue = Ships[i].arrival_cue;
								d_cue = Ships[i].departure_cue;
								_m_arrival_location = Ships[i].arrival_location;
								_m_arrival_dist = (Ships[i].arrival_distance);
								_m_arrival_target = Ships[i].arrival_anchor;
								_m_arrival_delay = (Ships[i].arrival_delay);
								_m_departure_location = Ships[i].departure_location;
								_m_departure_delay = (Ships[i].departure_delay);
								_m_departure_target = Ships[i].departure_anchor;

							} else {
								cue_init++;
								if (Ships[i].arrival_location != _m_arrival_location) {
									_m_arrival_location = -1;
								}

								if (Ships[i].departure_location != _m_departure_location) {
									_m_departure_location = -1;
								}

								_m_arrival_dist = (Ships[i].arrival_distance);
								_m_arrival_delay = (Ships[i].arrival_delay);
								_m_departure_delay = (Ships[i].departure_delay);

								if (Ships[i].arrival_anchor != _m_arrival_target) {
									_m_arrival_target = -1;
								}

								if (!cmp_sexp_chains(a_cue, Ships[i].arrival_cue)) {
									a_cue = -1;
									_m_update_arrival = 0;
								}

								if (!cmp_sexp_chains(d_cue, Ships[i].departure_cue)) {
									d_cue = -1;
									_m_update_departure = 0;
								}

								if (Ships[i].departure_anchor != _m_departure_target) {
									_m_departure_target = -1;
								}
							}
						}

						// process the first ship in group, else process the rest
						if (base_ship >= 0) {
							_m_ai_class = Ships[i].weapons.ai_class;
							cargo = Ships[i].cargo1;
							_m_cargo1 = Cargo_names[cargo];
							_m_hotkey = Ships[i].hotkey + 1;
							_m_score = (Ships[i].score);
							_m_assist_score = ((int)(Ships[i].assist_score_pct * 100));

							_m_persona = Ships[i].persona_index;

							// we use final_death_time member of ship structure for holding the amount of time before a
							// mission to destroy this ship
							wing = Ships[i].wingnum;
							if (wing < 0) {
								m_wing = "None";

							} else {
								m_wing = Wings[wing].name;
								if (!_editor->query_single_wing_marked())
									_m_update_arrival = _m_update_departure = 0;
							}

							// set routine local varaiables for ship/object flags
							no_arrival_warp = (Ships[i].flags[Ship::Ship_Flags::No_arrival_warp]) ? 1 : 0;
							no_departure_warp = (Ships[i].flags[Ship::Ship_Flags::No_departure_warp]) ? 1 : 0;

							base_ship = -1;
							if (!multi_edit)
								single_ship = i;

						} else {
							if (Ships[i].weapons.ai_class != _m_ai_class) {
								_m_ai_class = -1;
							}

							if (Ships[i].cargo1 != cargo) {
								_m_cargo1 = "";
							}

							_m_score = (Ships[i].score);
							_m_assist_score = ((int)(Ships[i].assist_score_pct * 100));

							if (Ships[i].hotkey != _m_hotkey - 1) {
								_m_hotkey = -1;
							}

							if (Ships[i].persona_index != _m_persona) {
								_m_persona = -2;
							}

							if (Ships[i].wingnum != wing) {
								m_wing = "";
							}

							no_arrival_warp =
								tristate_set(Ships[i].flags[Ship::Ship_Flags::No_arrival_warp], no_arrival_warp);
							no_departure_warp =
								tristate_set(Ships[i].flags[Ship::Ship_Flags::No_departure_warp], no_departure_warp);
						}
					}
				}
			}

			ptr = GET_NEXT(ptr);
		}

		if (multi_edit) {
			_m_arrival_tree.clear_tree("");
			_m_departure_tree.clear_tree("");
		}
		if (cue_init) {
			_m_arrival_tree.load_tree(a_cue);
			_m_departure_tree.load_tree(d_cue, "false");

		} else {
			_m_arrival_tree.clear_tree();
			//_m_arrival_tree.DeleteAllItems();
			_m_departure_tree.clear_tree();
			//_m_departure_tree.DeleteAllItems();
		}

		_m_player_ship = pship;
		_m_no_arrival_warp = no_arrival_warp;
		_m_no_departure_warp = no_departure_warp;

		if (!multi_edit) {
			auto i = _m_arrival_tree.select_sexp_node;
			if (i != -1) {
				_m_arrival_tree.hilite_item(i);

			} else {
				i = _m_departure_tree.select_sexp_node;
				if (i != -1) {
					_m_departure_tree.hilite_item(i);
				}
			}
		}

		_m_persona++;
		if (_m_persona > 0) {
			int persona_index = 0;
			for (int i = 0; i < _m_persona; i++) {
				if (Personas[i].flags & PERSONA_FLAG_WINGMAN)
					persona_index++;
			}
			_m_persona = persona_index;
		}
	} else {                    // no ships selected, 0 or more player ships selected
		if (player_count > 1) { // multiple player ships selected
			Assert(base_player >= 0);
			_m_ship_name = "";
			_m_player_ship = true;
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if ((ptr->type == OBJ_START) && (ptr->flags[Object::Object_Flags::Marked])) {
					auto i = ptr->instance;
					if (base_player >= 0) {
						_m_ship_class = Ships[i].ship_info_index;
						_m_team = Ships[i].team;
						base_player = -1;

					} else {
						if (Ships[i].ship_info_index != _m_ship_class)
							_m_ship_class = -1;
						if (Ships[i].team != _m_team)
							_m_team = -1;
					}
				}

				ptr = GET_NEXT(ptr);
			}
			// only 1 player selected..
		} else if (query_valid_object(_editor->currentObject) &&
				   (Objects[_editor->currentObject].type == OBJ_START)) {
			Assert((player_count == 1) && !multi_edit);
			player_ship = Objects[_editor->currentObject].instance;
			_m_ship_name = Ships[player_ship].ship_name;
			_m_ship_class = Ships[player_ship].ship_info_index;
			_m_team = Ships[player_ship].team;
			_m_player_ship = true;

		} else { // no ships or players selected..
			_m_ship_name = "";
			_m_ship_class = -1;
			_m_team = -1;
			_m_persona = -1;
			_m_player_ship = false;
		}

		_m_ai_class = -1;
		_m_cargo1 = "";
		_m_hotkey = 0;
		_m_score = 0; // cause control to be blank
		_m_assist_score = 0;
		_m_arrival_location = -1;
		_m_departure_location = -1;
		_m_arrival_delay = 0;
		_m_departure_delay = 0;
		_m_arrival_dist = 0;
		_m_arrival_target = -1;
		_m_departure_target = -1;
		_m_arrival_tree.clear_tree();
		//_m_arrival_tree.DeleteAllItems();
		_m_departure_tree.clear_tree();
		//_m_departure_tree.DeleteAllItems();
		_m_no_arrival_warp = false;
		_m_no_departure_warp = false;
		m_player = false;
		m_wing = "None";
	}
}

bool ShipEditorDialogModel::update_data(int redraw)
{
	char *str, old_name[255];
	object* ptr;
	int i, z, wing;
	if (multi_edit) {
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_START) || (ptr->type == OBJ_SHIP)) && (ptr->flags[Object::Object_Flags::Marked]))
				update_ship(get_ship_from_obj(ptr));

			ptr = GET_NEXT(ptr);
		}
	} else if (player_ship >= 0) { // editing a single player
		update_ship(player_ship);
	} else if (single_ship >= 0) { // editing a single ship
		drop_white_space(_m_ship_name);
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) &&
				(_editor->currentObject != OBJ_INDEX(ptr))) {
				str = Ships[ptr->instance].ship_name;
				if (!stricmp(_m_ship_name.c_str(), str)) {
					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
						"Ship Name Error",
						"This ship name is already being used by another ship.\n Press OK to restore old name",
						{DialogButton::Ok, DialogButton::Cancel});
					if (button == DialogButton::Cancel) {
						return false;
					} else {
						_m_ship_name = Ships[single_ship].ship_name;
						modelChanged();
					}
				}

				ptr = GET_NEXT(ptr);
			}

			for (i = 0; i < MAX_WINGS; i++) {
				if (Wings[i].wave_count && !stricmp(Wings[i].name, _m_ship_name.c_str())) {
					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
						"Ship Name Error",
						"This ship name is already being used by a wing.\n Press OK to restore old name",
						{DialogButton::Ok, DialogButton::Cancel});
					if (button == DialogButton::Cancel) {
						return false;
					} else {
						_m_ship_name = Ships[single_ship].ship_name;
						modelChanged();
					}
				}
			}

			for (i = 0; i < Num_iffs; i++) {
				if (!stricmp(_m_ship_name.c_str(), Iff_info[i].iff_name)) {
					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
						"Ship Name Error",
						"This ship name is already being used by a team.\n Press OK to restore old name",
						{DialogButton::Ok, DialogButton::Cancel});
					if (button == DialogButton::Cancel) {
						return false;
					} else {
						_m_ship_name = Ships[single_ship].ship_name;
						modelChanged();
					}
				}
			}

			for (i = 0; i < (int)Ai_tp_list.size(); i++) {
				if (!stricmp(_m_ship_name.c_str(), Ai_tp_list[i].name)) {

					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
						"Ship Name Error",
						"This ship name is already being used by a target priority group.\n Press OK to restore old "
						"name",
						{DialogButton::Ok, DialogButton::Cancel});
					if (button == DialogButton::Cancel) {
						return false;
					} else {
						_m_ship_name = Ships[single_ship].ship_name;
						modelChanged();
					}
				}
			}

			if (find_matching_waypoint_list(_m_ship_name.c_str()) != NULL) {

				auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
					"Ship Name Error",
					"This ship name is already being used by a a waypoint path.\n Press OK to restore old "
					"name",
					{DialogButton::Ok, DialogButton::Cancel});
				if (button == DialogButton::Cancel) {
					return false;
				} else {
					_m_ship_name = Ships[single_ship].ship_name;
					modelChanged();
				}
			}

			if (jumpnode_get_by_name(_m_ship_name.c_str()) != NULL) {

				auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
					"Ship Name Error",
					"This ship name is already being used by a a jump node.\n Press OK to restore old "
					"name",
					{DialogButton::Ok, DialogButton::Cancel});
				if (button == DialogButton::Cancel) {
					return false;
				} else {
					_m_ship_name = Ships[single_ship].ship_name;
					modelChanged();
				}
			}

			if (_m_ship_name[0] == '<') {

				auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
					"Ship Name Error",
					"Ship names not allowed to begin with <.\n Press OK to restore old "
					"name",
					{DialogButton::Ok, DialogButton::Cancel});
				if (button == DialogButton::Cancel) {
					return false;
				} else {
					_m_ship_name = Ships[single_ship].ship_name;
					modelChanged();
				}
			}

			wing = Ships[single_ship].wingnum;
			if (wing >= 0) {
				Assert((wing < MAX_WINGS) && Wings[wing].wave_count);
				for (i = 0; i < Wings[wing].wave_count; i++)
					if (_editor->wing_objects[wing][i] == Ships[single_ship].objnum)
						break;

				Assert(i < Wings[wing].wave_count);
				wing_bash_ship_name(old_name, Wings[wing].name, i + 1);
				if (strcmp(old_name, _m_ship_name.c_str())) {

					auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
						"Ship Name Error",
						"This ship is part of a wing, and its name cannot be changed",
						{DialogButton::Ok, DialogButton::Cancel});
					if (button == DialogButton::Cancel) {
						return false;
					} else {
						_m_ship_name = old_name;
						modelChanged();
					}
				}
			}
		}

		z = update_ship(single_ship);
		if (z)
			return true;

		strcpy_s(old_name, Ships[single_ship].ship_name);
		strcpy_s(Ships[single_ship].ship_name, _m_ship_name.c_str());
		set_modified();
		str = Ships[single_ship].ship_name;
		if (strcmp(old_name, str)) {
			update_sexp_references(old_name, str);
			_editor->ai_update_goal_references(REF_TYPE_SHIP, old_name, str);
			_editor->update_texture_replacements(old_name, str);
			for (i = 0; i < Num_reinforcements; i++)
				if (!strcmp(old_name, Reinforcements[i].name)) {
					Assert(strlen(str) < NAME_LENGTH);
					strcpy_s(Reinforcements[i].name, str);
				}

			_editor->missionChanged();
		}
	}

	if (Player_start_shipnum < 0 ||
		Objects[Ships[Player_start_shipnum].objnum].type != OBJ_START) { // need a new single player start.
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->type == OBJ_START) {
				Player_start_shipnum = ptr->instance;
				break;
			}

			ptr = GET_NEXT(ptr);
		}
	}

	return true;
}

bool ShipEditorDialogModel::apply() { update_data(1); 
return true;
}

void ShipEditorDialogModel::reject() {}

bool ShipEditorDialogModel::update_ship(int ship)
{
	int z, d;
	SCP_string str;
	int persona;

	ship_alt_name_close(ship);
	ship_callsign_close(ship);

	if ((Ships[ship].ship_info_index != _m_ship_class) && (_m_ship_class != -1)) {
		change_ship_type(ship, _m_ship_class);
		set_modified();
	}
	if (_m_team != -1)
		modify(Ships[ship].team, _m_team);

	if (Objects[Ships[ship].objnum].type != OBJ_SHIP) {
		if (mission_type || (Objects[Ships[ship].objnum].type != OBJ_START)) {
			return 0;
		}
	}

	if (_m_ai_class != -1) {
		modify(Ships[ship].weapons.ai_class, _m_ai_class);
	}
	if (!_m_cargo1.empty()) {
		z = string_lookup(_m_cargo1.c_str(), Cargo_names, Num_cargo);
		if (z == -1) {
			if (Num_cargo < MAX_CARGO) {
				z = Num_cargo++;
				strcpy(Cargo_names[z], _m_cargo1.c_str());
			} else {
				sprintf(str, "Maximum number of cargo names %s reached.\nIgnoring new name.\n", MAX_CARGO);
				auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Warning,
					"Cargo Error",
					str,
					{DialogButton::Ok});
				z = 0;
				_m_cargo1 = Cargo_names[z];
			}
		}

		modify(Ships[ship].cargo1, (char)z);
	}

	modify(Ships[ship].score, _m_score);
	if (_m_assist_score == -1) {
		Ships[ship].assist_score_pct = ((float)_m_assist_score) / 100;
		if (Ships[ship].assist_score_pct < 0) {
			Ships[ship].assist_score_pct = 0;
			auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Warning,
				"Ship Error",
				"Assist Percentage too low. Set to 0. No score will be granted for an assist",
				{DialogButton::Ok});
		} else if (Ships[ship].assist_score_pct > 1) {
			Ships[ship].assist_score_pct = 1;
			auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Warning,
				"Ship Error",
				"Assist Percentage too high. Set to 1. Assists will score as many points as a kill",
				{DialogButton::Ok});
		}
		set_modified();
	}

	if (_m_persona != -1) {
		modify(Ships[ship].persona_index, _m_persona);
	}

	if (Ships[ship].wingnum < 0) {

		if (_m_arrival_location != -1)
			modify(Ships[ship].arrival_location, _m_arrival_location);
		if (_m_departure_location != -1)
			modify(Ships[ship].departure_location, _m_departure_location);

		if (!multi_edit || _m_update_arrival) { // should we update the arrival cue?
			if (Ships[ship].arrival_cue >= 0)
				free_sexp2(Ships[ship].arrival_cue);

			Ships[ship].arrival_cue = _m_arrival_tree.save_tree();
		}

		if (!multi_edit || _m_update_departure) {
			if (Ships[ship].departure_cue >= 0)
				free_sexp2(Ships[ship].departure_cue);

			Ships[ship].departure_cue = _m_departure_tree.save_tree();
		}
		modify(Ships[ship].arrival_distance, _m_arrival_dist);
		modify(Ships[ship].arrival_delay, _m_arrival_delay);
		modify(Ships[ship].departure_delay, _m_departure_delay);
		if (_m_arrival_target >= 0) {
			modify(Ships[ship].arrival_anchor, _m_arrival_target);

			// if the arrival is not hyperspace or docking bay -- force arrival distance to be
			// greater than 2*radius of target.
			if (((_m_arrival_location != ARRIVE_FROM_DOCK_BAY) && (_m_arrival_location != ARRIVE_AT_LOCATION)) &&
				(_m_arrival_target >= 0) && !(_m_arrival_target & SPECIAL_ARRIVAL_ANCHOR_FLAG)) {
				d = int(std::min(500.0f, 2.0f * Objects[Ships[ship].objnum].radius));
				if ((Ships[ship].arrival_distance < d) && (Ships[ship].arrival_distance > -d)) {
					sprintf(str,
						"Ship must arrive at least %d meters away from target.\n"
						"Value has been reset to this.  Use with caution!\r\n"
						"Recommended distance is %d meters.\r\n",
						d,
						(int)(2.0f * Objects[Ships[ship].objnum].radius));

					_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", str, {DialogButton::Ok});
					if (Ships[ship].arrival_distance < 0)
						Ships[ship].arrival_distance = -d;
					else
						Ships[ship].arrival_distance = d;

					_m_arrival_dist = Ships[ship].arrival_distance;
					modelChanged();
				}
			}
		}
		if (_m_departure_target >= 0) {

			modify(Ships[ship].departure_anchor, _m_departure_target);
		}
	}

	if (_m_hotkey != -1)
		modify(Ships[ship].hotkey, _m_hotkey - 1);

	switch (_m_no_arrival_warp) {
	case 0:
		if (Ships[ship].flags[Ship::Ship_Flags::No_arrival_warp])
			set_modified();

		Ships[ship].flags.remove(Ship::Ship_Flags::No_arrival_warp);
		break;

	case 1:
		if (!(Ships[ship].flags[Ship::Ship_Flags::No_arrival_warp]))
			set_modified();

		Ships[ship].flags.set(Ship::Ship_Flags::No_arrival_warp);
		break;
	}

	switch (_m_no_departure_warp) {
	case 0:
		if (Ships[ship].flags[Ship::Ship_Flags::No_departure_warp])
			set_modified();

		Ships[ship].flags.remove(Ship::Ship_Flags::No_departure_warp);
		break;

	case 1:
		if (!(Ships[ship].flags[Ship::Ship_Flags::No_departure_warp]))
			set_modified();

		Ships[ship].flags.set(Ship::Ship_Flags::No_departure_warp);
		break;
	}

	switch (_m_player_ship) {
	case 1:
		if (Objects[Ships[ship].objnum].type != OBJ_START) {
			Player_starts++;
			set_modified();
		}

		Objects[Ships[ship].objnum].type = OBJ_START;
		break;

	case 0:
		if (Objects[Ships[ship].objnum].type == OBJ_START) {
			Player_starts--;
			set_modified();
		}

		Objects[Ships[ship].objnum].type = OBJ_SHIP;
		break;
	}

	return 0;
}
void ShipEditorDialogModel::ship_alt_name_close(int base_ship)
{

	if (multi_edit) {
		return;
	}
	drop_white_space(_m_alt_name);
	if (_m_alt_name.empty()) {
		return;
	}

	if (!_m_alt_name.empty() || !stricmp(_m_alt_name.c_str(), "<none>")) {
		if (*Fred_alt_names[base_ship]) {
			bool used = false;
			for (int i = 0; i < MAX_SHIPS; ++i) {
				if (i != base_ship && !strcmp(Fred_alt_names[i], Fred_alt_names[base_ship])) {
					used = true;
					break;
				}
			}
			if (!used) {
				mission_parse_remove_alt(Fred_alt_names[base_ship]);
			}
			// zero the entry
			strcpy_s(Fred_alt_names[base_ship], "");
		}
		return;
	}
	// otherwise see if it already exists
	if (mission_parse_lookup_alt(_m_alt_name.c_str()) >= 0) {
		strcpy_s(Fred_alt_names[base_ship], _m_alt_name.c_str());
		return;
	}
	// otherwise try and add it
	if (mission_parse_add_alt(_m_alt_name.c_str()) >= 0) {
		strcpy_s(Fred_alt_names[base_ship], _m_alt_name.c_str());
		return;
	}
	strcpy_s(Fred_alt_names[base_ship], "");
	auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
		"Alt Name Error",
		"Couldn't add new alternate type name. Already using too many!",
		{DialogButton::Ok});
}

void ShipEditorDialogModel::ship_callsign_close(int base_ship)
{

	if (multi_edit) {
		return;
	}
	drop_white_space(_m_callsign);
	if (_m_alt_name.empty()) {
		return;
	}

	if (!_m_callsign.empty() || !stricmp(_m_callsign.c_str(), "<none>")) {
		if (*Fred_callsigns[base_ship]) {
			bool used = false;
			for (int i = 0; i < MAX_SHIPS; ++i) {
				if (i != base_ship && !strcmp(Fred_callsigns[i], Fred_callsigns[base_ship])) {
					used = true;
					break;
				}
			}
			if (!used) {
				mission_parse_remove_callsign(Fred_callsigns[base_ship]);
			}
			// zero the entry
			strcpy_s(Fred_callsigns[base_ship], "");
		}
		return;
	}
	// otherwise see if it already exists
	if (mission_parse_lookup_callsign(_m_callsign.c_str()) >= 0) {
		strcpy_s(Fred_callsigns[base_ship], _m_callsign.c_str());
		return;
	}
	// otherwise try and add it
	if (mission_parse_add_callsign(_m_callsign.c_str()) >= 0) {
		strcpy_s(Fred_callsigns[base_ship], _m_callsign.c_str());
		return;
	}
	strcpy_s(Fred_callsigns[base_ship], "");
	auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
		"Alt Name Error",
		"Couldn't add new Callsign. Already using too many!",
		{DialogButton::Ok});
}
void ShipEditorDialogModel::setShipName(const SCP_string& m_ship_name) { modify(_m_ship_name, m_ship_name); }
SCP_string ShipEditorDialogModel::getShipName() { return _m_ship_name; }

void ShipEditorDialogModel::setShipClass(int m_ship_class)
{
	object* ptr;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked]))
			if (Ships[ptr->instance].ship_info_index != m_ship_class) {
				change_ship_type(ptr->instance, m_ship_class);
				set_modified();
			}

		ptr = GET_NEXT(ptr);
	}
	_editor->missionChanged();
}

int ShipEditorDialogModel::getShipClass() { return _m_ship_class; }

void ShipEditorDialogModel::setAIClass(int m_ai_class)
{
	modify(_m_ai_class, m_ai_class);
	modelChanged();
}

int ShipEditorDialogModel::getAIClass() { return _m_ai_class; }

void ShipEditorDialogModel::setTeam(int m_team)
{
	modify(_m_team, m_team);
	modelChanged();
}

int ShipEditorDialogModel::getTeam() { return _m_team; }

void ShipEditorDialogModel::setCargo(const SCP_string& m_cargo)
{
	modify(_m_cargo1, m_cargo);
	modelChanged();
}

SCP_string ShipEditorDialogModel::getCargo() { return _m_cargo1; }

void ShipEditorDialogModel::setAltName(const SCP_string& m_altName)
{
	modify(_m_alt_name, m_altName);
	modelChanged();
}

SCP_string ShipEditorDialogModel::getAltName() { return _m_alt_name; }

void ShipEditorDialogModel::setCallsign(const SCP_string& m_callsign)
{
	modify(_m_callsign, m_callsign);
	modelChanged();
}

SCP_string ShipEditorDialogModel::getCallsign() { return _m_callsign; }

SCP_string ShipEditorDialogModel::getWing()
{
	return m_wing;
}

bool ShipEditorDialogModel::wing_is_player_wing(int wing) { return _editor->wing_is_player_wing(wing); }

void ShipEditorDialogModel::set_modified()
{
	if (!_modified) {
		_modified = true;
	}
}
} // namespace dialogs
} // namespace fred
} // namespace fso