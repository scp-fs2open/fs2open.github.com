//
//

#if defined(_MSC_VER) && _MSC_VER <= 1920
	// work around MSVC 2015 and 2017 compiler bug
	// https://bugreports.qt.io/browse/QTBUG-72073
#define QT_NO_FLOAT16_OPERATORS
#endif

#include "ShipEditorDialogModel.h"

#include "iff_defs/iff_defs.h"
#include "jumpnode/jumpnode.h"
#include "mission/missionmessage.h"

#include <globalincs/linklist.h>
#include <mission/object.h>

#include <QtWidgets>

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
						return Qt::PartiallyChecked;
					}
				}
				else {
					if (cur_state) {
						return Qt::PartiallyChecked;
					}
				}
				if (cur_state == 1) {

					return Qt::Checked;
				}
				else {
					return Qt::Unchecked;
				}
			}
			int ShipEditorDialogModel::getSingleShip() const
			{
				return single_ship;
			}
			bool ShipEditorDialogModel::getIfMultipleShips() const
			{
				return multi_edit;
			}
			int ShipEditorDialogModel::getNumSelectedPlayers() const
			{
				return player_count;
			}
			int ShipEditorDialogModel::getNumUnmarkedPlayers() const
			{
				return pship_count;
			}
			bool ShipEditorDialogModel::getUIEnable() const
			{
				return enable;
			}
			int ShipEditorDialogModel::getNumSelectedShips() const
			{
				return ship_count;
			}
			int ShipEditorDialogModel::getUseCue() const
			{
				return cue_init;
			}
			int ShipEditorDialogModel::getNumSelectedObjects() const
			{
				return total_count;
			}
			int ShipEditorDialogModel::getNumValidPlayers() const
			{
				return pvalid_count;
			}
			int ShipEditorDialogModel::getIfPlayerShip() const
			{
				return player_ship;
			}
			void ShipEditorDialogModel::initializeData() {
				int type, wing = -1;
				int cargo = 0, base_ship, base_player, pship = -1;
				int escort_count;
				texenable = true;
				std::set<size_t> current_orders;
				pship_count = 0;  // a total count of the player ships not marked
				player_count = 0;
				ship_count = 0;
				cue_init = 0;
				total_count = 0;
				pvalid_count = 0;
				player_ship = 0;
				ship_orders.clear();
				object* objp;
				if (The_mission.game_type & MISSION_TYPE_MULTI) {
					mission_type = 0;  // multi player mission
				}
				else {
					mission_type = 1;  // non-multiplayer mission (implies single player mission I guess)
				}
				ship_count = player_count = escort_count = pship_count = pvalid_count = 0;
				base_ship = base_player = -1;
				enable = true;
				objp = GET_FIRST(&obj_used_list);
				while (objp != END_OF_LIST(&obj_used_list)) {
					if ((objp->type == OBJ_SHIP) && (Ships[objp->instance].flags[Ship::Ship_Flags::Escort])) {
						escort_count++;  // get a total count of escort ships
					}

					if (objp->type == OBJ_START) {
						pship_count++;  // count all player starts in mission
					}

					if (objp->flags[Object::Object_Flags::Marked]) {
						type = objp->type;
						if ((type == OBJ_START) && !mission_type) {  // in multiplayer missions, starts act like ships
							type = OBJ_SHIP;
						}

						auto i = -1;
						if (type == OBJ_START) {
							player_count++;
							// if player_count is 1, base_player will be the one and only player
							i = base_player = objp->instance;

						}
						else if (type == OBJ_SHIP) {
							ship_count++;
							// if ship_count is 1, base_ship will be the one and only ship
							i = base_ship = objp->instance;
						}

						if (i >= 0) {
							if (Ship_info[Ships[i].ship_info_index].flags[Ship::Info_Flags::Player_ship]) {
								pvalid_count++;
							}
						}
					}

					objp = GET_NEXT(objp);
				}

				total_count = ship_count + player_count;  // get total number of objects being edited.
				multi_edit = (total_count > 1);

				_m_arrival_tree_formula = _m_departure_tree_formula = -1;
				_m_arrival_location = -1;
				_m_arrival_dist = -1;
				_m_arrival_target = -1;
				_m_arrival_delay = -1;
				_m_departure_location = -1;
				_m_departure_target = -1;
				_m_departure_delay = -1;

				player_ship = single_ship = -1;

				ship_orders.clear();				// assume they are all the same type
				if (ship_count) {
					if (!multi_edit) {
						Assert((ship_count == 1) && (base_ship >= 0));
						_m_ship_name = Ships[base_ship].ship_name;
					}
					else {
						_m_ship_name = "";
					}

					_m_update_arrival = _m_update_departure = true;

					base_player = 0;
					objp = GET_FIRST(&obj_used_list);
					while (objp != END_OF_LIST(&obj_used_list)) {
						if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
							if (objp->flags[Object::Object_Flags::Marked]) {
								// do processing for both ships and players
								auto i = get_ship_from_obj(objp);
								if (base_player >= 0) {
									_m_ship_class = Ships[i].ship_info_index;
									_m_team = Ships[i].team;
									pship = (objp->type == OBJ_START) ? 1 : 0;
									base_player = -1;
								}
								else {
									if (Ships[i].ship_info_index != _m_ship_class) { 
										_m_ship_class = -1; 
										texenable = false; 
									}
									if (Ships[i].team != _m_team) { _m_team = -1; }
									pship = tristate_set(Objects[Ships[i].objnum].type == OBJ_START, pship);
								}
								// 'and' in the ship type of this ship to our running bitfield
								current_orders = ship_get_default_orders_accepted(&Ship_info[Ships[i].ship_info_index]);
								if (ship_orders.empty()) {
									ship_orders = current_orders;
								}
								else if (ship_orders != current_orders) {
									ship_orders = {std::numeric_limits<size_t>::max()};
								}

								if (Ships[i].flags[Ship::Ship_Flags::Escort]) {
									escort_count--;  // remove marked escorts from count
								}

								if (Objects[Ships[i].objnum].type == OBJ_START) {
									pship_count--;  // removed marked starts from count
								}
								if ((objp->type == OBJ_SHIP) || ((objp->type == OBJ_START) && !mission_type)) {
									// process this if ship not in a wing
									if (Ships[i].wingnum < 0) {
										if (!cue_init) {
											cue_init = 1;
											_m_arrival_tree_formula = Ships[i].arrival_cue;
											_m_departure_tree_formula = Ships[i].departure_cue;
											_m_arrival_location = Ships[i].arrival_location;
											_m_arrival_dist = Ships[i].arrival_distance;
											_m_arrival_target = Ships[i].arrival_anchor;
											_m_arrival_delay = Ships[i].arrival_delay;
											_m_departure_location = Ships[i].departure_location;
											_m_departure_delay = Ships[i].departure_delay;
											_m_departure_target = Ships[i].departure_anchor;

										}
										else {
											cue_init++;
											if (Ships[i].arrival_location != _m_arrival_location) {
												_m_arrival_location = -1;
											}

											if (Ships[i].departure_location != _m_departure_location) {
												_m_departure_location = -1;
											}

											_m_arrival_dist = Ships[i].arrival_distance;
											_m_arrival_delay = Ships[i].arrival_delay;
											_m_departure_delay = Ships[i].departure_delay;

											if (Ships[i].arrival_anchor != _m_arrival_target) {
												_m_arrival_target = -1;
											}

											if (!cmp_sexp_chains(_m_arrival_tree_formula, Ships[i].arrival_cue)) {
												_m_arrival_tree_formula = -1;
												_m_update_arrival = false;
											}

											if (!cmp_sexp_chains(_m_departure_tree_formula, Ships[i].departure_cue)) {
												_m_departure_tree_formula = -1;
												_m_update_departure = false;
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
										_m_score = Ships[i].score;
										_m_assist_score = ((int)(Ships[i].assist_score_pct * 100));

										_m_persona = Ships[i].persona_index;

										// we use final_death_time member of ship structure for holding the amount of time before a mission
										// to destroy this ship
										wing = Ships[i].wingnum;
										if (wing < 0) {
											m_wing = "None";

										}
										else {
											m_wing = Wings[wing].name;
											if (!_editor->query_single_wing_marked())
												_m_update_arrival = _m_update_departure = false;
										}

										// set routine local varaiables for ship/object flags
										_m_no_arrival_warp = (Ships[i].flags[Ship::Ship_Flags::No_arrival_warp]) ? 2 : 0;
										_m_no_departure_warp = (Ships[i].flags[Ship::Ship_Flags::No_departure_warp]) ? 2 : 0;

										base_ship = -1;
										if (!multi_edit)
											single_ship = i;

									}
									else {
										if (Ships[i].weapons.ai_class != _m_ai_class) {
											_m_ai_class = -1;
										}

										if (Ships[i].cargo1 != cargo) {
											_m_cargo1 = "";
										}

										_m_score = Ships[i].score;
										_m_assist_score = (int)(Ships[i].assist_score_pct * 100);

										if (Ships[i].hotkey != _m_hotkey - 1) {
											_m_hotkey = -1;
										}

										if (Ships[i].persona_index != _m_persona) {
											_m_persona = -2;
										}

										if (Ships[i].wingnum != wing) {
											m_wing = "";
										}

										_m_no_arrival_warp = tristate_set(Ships[i].flags[Ship::Ship_Flags::No_arrival_warp], _m_no_arrival_warp);
										_m_no_departure_warp = tristate_set(Ships[i].flags[Ship::Ship_Flags::No_departure_warp], _m_no_departure_warp);
									}
								}
							}
						}

						objp = GET_NEXT(objp);
					}

					_m_player_ship = pship;

					_m_persona++;
					if (_m_persona > 0) {
						int persona_index = 0;
						for (int i = 0; i < _m_persona; i++) {
							if (Personas[i].flags & PERSONA_FLAG_WINGMAN)
								persona_index++;
						}
						_m_persona = persona_index;
					}
				}
				else {                    // no ships selected, 0 or more player ships selected
					if (player_count > 1) { // multiple player ships selected
						Assert(base_player >= 0);
						_m_ship_name = "";
						_m_player_ship = true;
						objp = GET_FIRST(&obj_used_list);
						while (objp != END_OF_LIST(&obj_used_list)) {
							if ((objp->type == OBJ_START) && (objp->flags[Object::Object_Flags::Marked])) {
								auto i = objp->instance;
								if (base_player >= 0) {
									_m_ship_class = Ships[i].ship_info_index;
									_m_team = Ships[i].team;
									base_player = -1;

								}
								else {
									if (Ships[i].ship_info_index != _m_ship_class)
										_m_ship_class = -1;
									if (Ships[i].team != _m_team)
										_m_team = -1;
								}
							}

							objp = GET_NEXT(objp);
						}
						// only 1 player selected..
					}
					else if (query_valid_object(_editor->currentObject) && (Objects[_editor->currentObject].type == OBJ_START)) {
						// Assert((player_count == 1) && !multi_edit);
						player_ship = Objects[_editor->currentObject].instance;
						_m_ship_name = Ships[player_ship].ship_name;
						_m_ship_class = Ships[player_ship].ship_info_index;
						_m_team = Ships[player_ship].team;
						_m_player_ship = true;

					}
					else { // no ships or players selected..
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
					_m_no_arrival_warp = false;
					_m_no_departure_warp = false;
					m_wing = "None";
					enable = false;
				}
				modelChanged();

			}
			bool ShipEditorDialogModel::update_data()
			{
				char* str, old_name[255];
				object* ptr;
				int i, z, wing;
				if (multi_edit) {
					ptr = GET_FIRST(&obj_used_list);
					while (ptr != END_OF_LIST(&obj_used_list)) {
						if (((ptr->type == OBJ_START) || (ptr->type == OBJ_SHIP)) && (ptr->flags[Object::Object_Flags::Marked]))
							update_ship(get_ship_from_obj(ptr));

						ptr = GET_NEXT(ptr);
					}
				}
				else if (player_ship >= 0) { // editing a single player
					update_ship(player_ship);
				}
				else if (single_ship >= 0) { // editing a single ship
					drop_white_space(_m_ship_name);
					ptr = GET_FIRST(&obj_used_list);
					while (ptr != END_OF_LIST(&obj_used_list)) {
						if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (_editor->currentObject != OBJ_INDEX(ptr))) {
							str = Ships[ptr->instance].ship_name;
							if (!stricmp(_m_ship_name.c_str(), str)) {
								auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
									"Ship Name Error",
									"This ship name is already being used by another ship.\n Press OK to restore old name",
									{ DialogButton::Ok, DialogButton::Cancel });
								if (button == DialogButton::Cancel) {
									return false;
								}
								else {
									_m_ship_name = Ships[single_ship].ship_name;
									modelChanged();
								}
							}
						}

						ptr = GET_NEXT(ptr);
					}

					for (i = 0; i < MAX_WINGS; i++) {
						if (Wings[i].wave_count && !stricmp(Wings[i].name, _m_ship_name.c_str())) {
							auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
								"Ship Name Error",
								"This ship name is already being used by a wing.\n Press OK to restore old name",
								{ DialogButton::Ok, DialogButton::Cancel });
							if (button == DialogButton::Cancel) {
								return false;
							}
							else {
								_m_ship_name = Ships[single_ship].ship_name;
								modelChanged();
							}
						}
					}

					// We don't need to check teams.  "Unknown" is a valid name and also an IFF.

					for (i = 0; i < (int)Ai_tp_list.size(); i++) {
						if (!stricmp(_m_ship_name.c_str(), Ai_tp_list[i].name)) {

							auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
								"Ship Name Error",
								"This ship name is already being used by a target priority group.\n Press OK to restore old "
								"name",
								{ DialogButton::Ok, DialogButton::Cancel });
							if (button == DialogButton::Cancel) {
								return false;
							}
							else {
								_m_ship_name = Ships[single_ship].ship_name;
								modelChanged();
							}
						}
					}

					if (find_matching_waypoint_list(_m_ship_name.c_str()) != nullptr) {

						auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
							"Ship Name Error",
							"This ship name is already being used by a a waypoint path.\n Press OK to restore old "
							"name",
							{ DialogButton::Ok, DialogButton::Cancel });
						if (button == DialogButton::Cancel) {
							return false;
						}
						else {
							_m_ship_name = Ships[single_ship].ship_name;
							modelChanged();
						}
					}

					if (jumpnode_get_by_name(_m_ship_name.c_str()) != nullptr) {

						auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
							"Ship Name Error",
							"This ship name is already being used by a a jump node.\n Press OK to restore old "
							"name",
							{ DialogButton::Ok, DialogButton::Cancel });
						if (button == DialogButton::Cancel) {
							return false;
						}
						else {
							_m_ship_name = Ships[single_ship].ship_name;
							modelChanged();
						}
					}

					if (_m_ship_name[0] == '<') {

						auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
							"Ship Name Error",
							"Ship names not allowed to begin with <.\n Press OK to restore old "
							"name",
							{ DialogButton::Ok, DialogButton::Cancel });
						if (button == DialogButton::Cancel) {
							return false;
						}
						else {
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
								{ DialogButton::Ok, DialogButton::Cancel });
							if (button == DialogButton::Cancel) {
								return false;
							}
							else {
								_m_ship_name = old_name;
								modelChanged();
							}
						}
					}

					z = update_ship(single_ship);
					if (z)
						return z;

					strcpy_s(old_name, Ships[single_ship].ship_name);
					strcpy_s(Ships[single_ship].ship_name, _m_ship_name.c_str());
					str = Ships[single_ship].ship_name;
					if (strcmp(old_name, str)) {
						update_sexp_references(old_name, str);
						_editor->ai_update_goal_references(REF_TYPE_SHIP, old_name, str);
						_editor->update_texture_replacements(old_name, str);
						for (i = 0; i < Num_reinforcements; i++) {
							if (!strcmp(old_name, Reinforcements[i].name)) {
								Assert(strlen(str) < NAME_LENGTH);
								strcpy_s(Reinforcements[i].name, str);
							}
						}

						if (Ships[single_ship].has_display_name()) {
							Ships[single_ship].flags.remove(Ship::Ship_Flags::Has_display_name);
							Ships[single_ship].display_name = "";
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
				_editor->missionChanged();
				return true;
			}

			bool ShipEditorDialogModel::apply()
			{
				update_data();
				_editor->missionChanged();
				initializeData();
				return true;
			}

			void ShipEditorDialogModel::reject() {}


			bool ShipEditorDialogModel::update_ship(int ship)
			{
				int z, d;
				SCP_string str;

				ship_alt_name_close(ship);
				ship_callsign_close(ship);

				if ((Ships[ship].ship_info_index != _m_ship_class) && (_m_ship_class != -1)) {
					change_ship_type(ship, _m_ship_class);
				}
				if (_m_team != -1)
					Ships[ship].team = _m_team;

				if (Objects[Ships[ship].objnum].type != OBJ_SHIP) {
					if (mission_type || (Objects[Ships[ship].objnum].type != OBJ_START)) {
						return false;
					}
				}

				if (_m_ai_class != -1) {
					Ships[ship].weapons.ai_class = _m_ai_class;
				}
				if (!_m_cargo1.empty()) {
					z = string_lookup(_m_cargo1.c_str(), Cargo_names, Num_cargo);
					if (z == -1) {
						if (Num_cargo < MAX_CARGO) {
							z = Num_cargo++;
							strcpy(Cargo_names[z], _m_cargo1.c_str());
						}
						else {
							sprintf(str, "Maximum number of cargo names %d reached.\nIgnoring new name.\n", MAX_CARGO);
							_viewport->dialogProvider->showButtonDialog(DialogType::Warning,
								"Cargo Error",
								str,
								{ DialogButton::Ok });
							z = 0;
							_m_cargo1 = Cargo_names[z];
						}
					}
					Ships[ship].cargo1 = (char)z;
				}

				Ships[ship].score = _m_score;
				if (_m_assist_score == -1) {
					Ships[ship].assist_score_pct = ((float)_m_assist_score) / 100;
					if (Ships[ship].assist_score_pct < 0) {
						Ships[ship].assist_score_pct = 0;
						_viewport->dialogProvider->showButtonDialog(DialogType::Warning,
							"Ship Error",
							"Assist Percentage too low. Set to 0. No score will be granted for an assist",
							{ DialogButton::Ok });
					}
					else if (Ships[ship].assist_score_pct > 1) {
						Ships[ship].assist_score_pct = 1;
						_viewport->dialogProvider->showButtonDialog(DialogType::Warning,
							"Ship Error",
							"Assist Percentage too high. Set to 1. Assists will score as many points as a kill",
							{ DialogButton::Ok });
					}
				}

				Ships[ship].persona_index = _m_persona;

				if (Ships[ship].wingnum < 0) {

					if (_m_arrival_location != -1)
						Ships[ship].arrival_location = _m_arrival_location;
					if (_m_departure_location != -1)
						Ships[ship].departure_location = _m_departure_location;

					if (!multi_edit || _m_update_arrival) { // should we update the arrival cue?
						if (Ships[ship].arrival_cue >= 0)
							free_sexp2(Ships[ship].arrival_cue);

						Ships[ship].arrival_cue = _m_arrival_tree_formula;
					}

					if (!multi_edit || _m_update_departure) {
						if (Ships[ship].departure_cue >= 0)
							free_sexp2(Ships[ship].departure_cue);

						Ships[ship].departure_cue = _m_departure_tree_formula;
					}
					Ships[ship].arrival_distance = _m_arrival_dist;
					Ships[ship].arrival_delay = _m_arrival_delay;
					Ships[ship].departure_delay = _m_departure_delay;
					if (_m_arrival_target >= 0) {
						Ships[ship].arrival_anchor = _m_arrival_target;

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

								_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", str, { DialogButton::Ok });
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

						Ships[ship].departure_anchor = _m_departure_target;
					}
				}

				if (_m_hotkey != -1)
					Ships[ship].hotkey = _m_hotkey - 1;

				if (!_m_no_arrival_warp) {

					if (Ships[ship].flags[Ship::Ship_Flags::No_arrival_warp])

						Ships[ship].flags.remove(Ship::Ship_Flags::No_arrival_warp);
				}

				else {
					if (!(Ships[ship].flags[Ship::Ship_Flags::No_arrival_warp]))

						Ships[ship].flags.set(Ship::Ship_Flags::No_arrival_warp);
				}

				if (!_m_no_departure_warp) {

					if (Ships[ship].flags[Ship::Ship_Flags::No_departure_warp])

						Ships[ship].flags.remove(Ship::Ship_Flags::No_departure_warp);
				}

				else {
					if (!(Ships[ship].flags[Ship::Ship_Flags::No_departure_warp]))

						Ships[ship].flags.set(Ship::Ship_Flags::No_departure_warp);
				}

				if (_m_player_ship) {
					if (Objects[Ships[ship].objnum].type != OBJ_START) {
						Player_starts++;
					}

					Objects[Ships[ship].objnum].type = OBJ_START;
				}
				else {
					if (Objects[Ships[ship].objnum].type == OBJ_START) {
						Player_starts--;
					}

					Objects[Ships[ship].objnum].type = OBJ_SHIP;
				}
				_editor->missionChanged();
				return false;
			}
			void ShipEditorDialogModel::ship_alt_name_close(int ship)
			{

				if (multi_edit) {
					return;
				}
				drop_white_space(_m_alt_name);
				if (_m_alt_name.empty()) {
					return;
				}

				if (!_m_alt_name.empty() || !stricmp(_m_alt_name.c_str(), "<none>")) {
					if (*Fred_alt_names[ship]) {
						bool used = false;
						for (int i = 0; i < MAX_SHIPS; ++i) {
							if (i != ship && !strcmp(Fred_alt_names[i], Fred_alt_names[ship])) {
								used = true;
								break;
							}
						}
						if (!used) {
							mission_parse_remove_alt(Fred_alt_names[ship]);
						}
						// zero the entry
						strcpy_s(Fred_alt_names[ship], "");
					}
					return;
				}
				// otherwise see if it already exists
				if (mission_parse_lookup_alt(_m_alt_name.c_str()) >= 0) {
					strcpy_s(Fred_alt_names[ship], _m_alt_name.c_str());
					return;
				}
				// otherwise try and add it
				if (mission_parse_add_alt(_m_alt_name.c_str()) >= 0) {
					strcpy_s(Fred_alt_names[ship], _m_alt_name.c_str());
					return;
				}
				strcpy_s(Fred_alt_names[ship], "");
				_viewport->dialogProvider->showButtonDialog(DialogType::Error,
					"Alt Name Error",
					"Couldn't add new alternate type name. Already using too many!",
					{ DialogButton::Ok });
			}

			void ShipEditorDialogModel::ship_callsign_close(int ship)
			{

				if (multi_edit) {
					return;
				}
				drop_white_space(_m_callsign);
				if (_m_alt_name.empty()) {
					return;
				}

				if (!_m_callsign.empty() || !stricmp(_m_callsign.c_str(), "<none>")) {
					if (*Fred_callsigns[ship]) {
						bool used = false;
						for (int i = 0; i < MAX_SHIPS; ++i) {
							if (i != ship && !strcmp(Fred_callsigns[i], Fred_callsigns[ship])) {
								used = true;
								break;
							}
						}
						if (!used) {
							mission_parse_remove_callsign(Fred_callsigns[ship]);
						}
						// zero the entry
						strcpy_s(Fred_callsigns[ship], "");
					}
					return;
				}
				// otherwise see if it already exists
				if (mission_parse_lookup_callsign(_m_callsign.c_str()) >= 0) {
					strcpy_s(Fred_callsigns[ship], _m_callsign.c_str());
					return;
				}
				// otherwise try and add it
				if (mission_parse_add_callsign(_m_callsign.c_str()) >= 0) {
					strcpy_s(Fred_callsigns[ship], _m_callsign.c_str());
					return;
				}
				strcpy_s(Fred_callsigns[ship], "");
				_viewport->dialogProvider->showButtonDialog(DialogType::Error,
					"Alt Name Error",
					"Couldn't add new Callsign. Already using too many!",
					{ DialogButton::Ok });
			}
			void ShipEditorDialogModel::setShipName(const SCP_string& m_ship_name)
			{
				modify(_m_ship_name, m_ship_name);
			}
			SCP_string ShipEditorDialogModel::getShipName() const
			{
				return _m_ship_name;
			}

			void ShipEditorDialogModel::setShipClass(int m_ship_class)
			{
				object* ptr;
				ptr = GET_FIRST(&obj_used_list);
				while (ptr != END_OF_LIST(&obj_used_list)) {
					if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
						if (Ships[ptr->instance].ship_info_index != m_ship_class) {
							change_ship_type(ptr->instance, m_ship_class);
							set_modified();
						}
					}
					ptr = GET_NEXT(ptr);
				}
				modify(_m_ship_class, m_ship_class);
				_editor->missionChanged();
			}

			int ShipEditorDialogModel::getShipClass() const
			{
				return _m_ship_class;
			}

			void ShipEditorDialogModel::setAIClass(const int m_ai_class)
			{
				modify(_m_ai_class, m_ai_class);
				modelChanged();
			}

			int ShipEditorDialogModel::getAIClass() const
			{
				return _m_ai_class;
			}

			void ShipEditorDialogModel::setTeam(const int m_team)
			{
				modify(_m_team, m_team);
				modelChanged();
			}

			int ShipEditorDialogModel::getTeam() const
			{
				return _m_team;
			}

			void ShipEditorDialogModel::setCargo(const SCP_string& m_cargo)
			{
				modify(_m_cargo1, m_cargo);
			}

			SCP_string ShipEditorDialogModel::getCargo() const
			{
				return _m_cargo1;
			}

			void ShipEditorDialogModel::setAltName(const SCP_string& m_altName)
			{
				modify(_m_alt_name, m_altName);
			}

			SCP_string ShipEditorDialogModel::getAltName() const
			{
				return _m_alt_name;
			}

			void ShipEditorDialogModel::setCallsign(const SCP_string& m_callsign)
			{
				modify(_m_callsign, m_callsign);
			}

			SCP_string ShipEditorDialogModel::getCallsign() const
			{
				return _m_callsign;
			}

			SCP_string ShipEditorDialogModel::getWing() const
			{
				return m_wing;
			}

			void ShipEditorDialogModel::setHotkey(const int m_hotkey)
			{
				modify(_m_hotkey, m_hotkey);
			}

			int ShipEditorDialogModel::getHotkey() const
			{
				return _m_hotkey;
			}

			void ShipEditorDialogModel::setPersona(const int m_persona)
			{
				modify(_m_persona, m_persona);
			}

			int ShipEditorDialogModel::getPersona() const
			{
				return _m_persona;
			}

			void ShipEditorDialogModel::setScore(const int m_score)
			{
				modify(_m_score, m_score);
			}

			int ShipEditorDialogModel::getScore() const
			{
				return _m_score;
			}

			void ShipEditorDialogModel::setAssist(const int m_assist_score)
			{
				modify(_m_assist_score, m_assist_score);
			}

			int ShipEditorDialogModel::getAssist() const
			{
				return _m_assist_score;
			}

			void ShipEditorDialogModel::setPlayer(const bool m_player)
			{
				modify(_m_player_ship, m_player);
			}

			bool ShipEditorDialogModel::getPlayer() const
			{
				return _m_player_ship;
			}

			void ShipEditorDialogModel::setArrivalLocation(const int value)
			{
				modify(_m_arrival_location, value);
			}

			int ShipEditorDialogModel::getArrivalLocation() const
			{
				return _m_arrival_location;
			}

			void ShipEditorDialogModel::setArrivalTarget(const int value)
			{
				modify(_m_arrival_target, value);
			}

			int ShipEditorDialogModel::getArrivalTarget() const
			{
				return _m_arrival_target;
			}

			void ShipEditorDialogModel::setArrivalDistance(const int value)
			{
				modify(_m_arrival_dist, value);
			}

			int ShipEditorDialogModel::getArrivalDistance() const
			{
				return _m_arrival_dist;
			}

			void ShipEditorDialogModel::setArrivalDelay(const int value)
			{
				modify(_m_arrival_delay, value);
			}

			int ShipEditorDialogModel::getArrivalDelay() const
			{
				return _m_arrival_delay;
			}

			void ShipEditorDialogModel::setArrivalCue(const bool value)
			{
				modify(_m_update_arrival, value);
			}

			bool ShipEditorDialogModel::getArrivalCue() const
			{
				return _m_update_arrival;
			}

			void ShipEditorDialogModel::setArrivalFormula(const int old_form, const int new_form)
			{
				if (old_form != _m_arrival_tree_formula)
					modify(_m_arrival_tree_formula, new_form);
			}

			int ShipEditorDialogModel::getArrivalFormula() const
			{
				return _m_arrival_tree_formula;
			}

			void ShipEditorDialogModel::setNoArrivalWarp(const int value)
			{
				modify(_m_no_arrival_warp, value);
			}

			int ShipEditorDialogModel::getNoArrivalWarp() const
			{
				return _m_no_arrival_warp;
			}

			void ShipEditorDialogModel::setDepartureLocation(const int value)
			{
				modify(_m_departure_location, value);
			}

			int ShipEditorDialogModel::getDepartureLocation() const
			{
				return _m_departure_location;
			}

			void ShipEditorDialogModel::setDepartureTarget(const int value)
			{
				modify(_m_departure_target, value);
			}

			int ShipEditorDialogModel::getDepartureTarget() const
			{
				return _m_departure_target;
			}

			void ShipEditorDialogModel::setDepartureDelay(const int value)
			{
				modify(_m_departure_delay, value);
			}

			int ShipEditorDialogModel::getDepartureDelay() const
			{
				return _m_departure_delay;
			}

			void ShipEditorDialogModel::setDepartureCue(const bool value)
			{
				modify(_m_update_departure, value);
			}

			bool ShipEditorDialogModel::getDepartureCue() const
			{
				return _m_update_departure;
			}

			void ShipEditorDialogModel::setDepartureFormula(const int old_form, const int new_form)
			{
				if (old_form != _m_departure_tree_formula)
					modify(_m_departure_tree_formula, new_form);
			}

			int ShipEditorDialogModel::getDepartureFormula() const
			{
				return _m_departure_tree_formula;
			}

			void ShipEditorDialogModel::setNoDepartureWarp(const int value)
			{
				modify(_m_no_departure_warp, value);
			}

			int ShipEditorDialogModel::getNoDepartureWarp() const
			{
				return _m_no_departure_warp;
			}

			void ShipEditorDialogModel::OnPrevious()
			{
				int i, n, arr[MAX_SHIPS];
				if (!update_data()) {
					n = make_ship_list(arr);
					if (!n) {
						return;
					}
					if (_editor->cur_ship < 0) {
						i = n - 1;
					}

					else {
						for (i = 0; i < n; i++) {
							if (Ships[_editor->cur_ship].objnum == arr[i]) {
								break;
							}
						}

						Assert(i < n);
						i--;
						if (i < 0) {
							i = n - 1;
						}
					}

					_editor->unmark_all();
					_editor->selectObject(arr[i]);
					initializeData();
				}
			}

			void ShipEditorDialogModel::OnNext()
			{
				int i, n, arr[MAX_SHIPS];

				if (!update_data()) {
					n = make_ship_list(arr);
					if (!n)
						return;

					if (_editor->cur_ship < 0)
						i = 0;

					else {
						for (i = 0; i < n; i++)
							if (Ships[_editor->cur_ship].objnum == arr[i])
								break;

						Assert(i < n);
						i++;
						if (i == n)
							i = 0;
					}

					_editor->unmark_all();
					_editor->selectObject(arr[i]);
					initializeData();
				}
			}

			void ShipEditorDialogModel::OnDeleteShip()
			{
				_editor->delete_marked();
				_editor->unmark_all();
			}

			void ShipEditorDialogModel::OnShipReset()
			{
				int i, j, index, ship;
				object* objp;
				ship_info* sip;
				ship_subsys* ptr;
				ship_weapon* wp;
				model_subsystem* sp;

				_m_cargo1 = "Nothing";
				_m_ai_class = AI_DEFAULT_CLASS;
				if (_m_ship_class) {
					_m_team = Species_info[Ship_info[_m_ship_class].species].default_iff;
				}

				objp = GET_FIRST(&obj_used_list);
				while (objp != END_OF_LIST(&obj_used_list)) {
					if (((objp->type == OBJ_SHIP) || ((objp->type == OBJ_START) && !mission_type)) &&
						(objp->flags[Object::Object_Flags::Marked])) {
						ship = objp->instance;

						// reset ship goals
						for (i = 0; i < MAX_AI_GOALS; i++) {
							Ai_info[Ships[ship].ai_index].goals[i].ai_mode = AI_GOAL_NONE;
						}

						objp->phys_info.speed = 0.0f;
						objp->shield_quadrant[0] = 100.0f;
						objp->hull_strength = 100.0f;

						sip = &Ship_info[Ships[ship].ship_info_index];
						for (i = 0; i < sip->num_primary_banks; i++)
							Ships[ship].weapons.primary_bank_weapons[i] = sip->primary_bank_weapons[i];

						for (i = 0; i < sip->num_secondary_banks; i++) {
							Ships[ship].weapons.secondary_bank_weapons[i] = sip->secondary_bank_weapons[i];
							Ships[ship].weapons.secondary_bank_capacity[i] = sip->secondary_bank_ammo_capacity[i];
						}

						index = 0;
						ptr = GET_FIRST(&Ships[ship].subsys_list);
						while (ptr != END_OF_LIST(&Ships[ship].subsys_list)) {
							ptr->current_hits = 0.0f;
							if (ptr->system_info->type == SUBSYSTEM_TURRET) {
								wp = &ptr->weapons;
								sp = &Ship_info[Ships[ship].ship_info_index].subsystems[index];

								j = 0;
								for (i = 0; i < MAX_SHIP_PRIMARY_BANKS; i++) {
									if (sp->primary_banks[i] != -1) {
										wp->primary_bank_weapons[j++] = sp->primary_banks[i];
									}
								}

								wp->num_primary_banks = j;
								j = 0;
								for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
									if (sp->secondary_banks[i] != -1) {
										wp->secondary_bank_weapons[j] = sp->secondary_banks[i];
										wp->secondary_bank_capacity[j++] = sp->secondary_bank_capacity[i];
									}
								}

								wp->num_secondary_banks = j;
								for (i = 0; i < MAX_SHIP_SECONDARY_BANKS; i++) {
									wp->secondary_bank_ammo[i] = 100;
								}
							}

							index++;
							ptr = GET_NEXT(ptr);
						}
					}

					objp = GET_NEXT(objp);
				}
				modelChanged();
				_editor->missionChanged();
				if (multi_edit) {

					_viewport->dialogProvider->showButtonDialog(DialogType::Information,
						"Reset",
						"Ships reset to ship class defaults",
						{ DialogButton::Ok });
				}
				else {

					_viewport->dialogProvider->showButtonDialog(DialogType::Information,
						"Reset",
						"Ship reset to ship class defaults",
						{ DialogButton::Ok });
				}
			}

			bool ShipEditorDialogModel::wing_is_player_wing(const int wing)
			{
				return Editor::wing_is_player_wing(wing);
			}

			std::set<size_t> ShipEditorDialogModel::getShipOrders() const
			{
				return ship_orders;
			}

			bool ShipEditorDialogModel::getTexEditEnable() const
			{
				return texenable;
			}

			int ShipEditorDialogModel::make_ship_list(int* arr)
			{
				int n = 0;
				object* ptr;

				ptr = GET_FIRST(&obj_used_list);
				while (ptr != END_OF_LIST(&obj_used_list)) {
					if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
						arr[n++] = OBJ_INDEX(ptr);
					}

					ptr = GET_NEXT(ptr);
				}

				return n;
			}

			void ShipEditorDialogModel::set_modified()
			{
				if (!_modified) {
					_modified = true;
				}
			}
		} // namespace dialogs
	} // namespace fred
} // namespace fso