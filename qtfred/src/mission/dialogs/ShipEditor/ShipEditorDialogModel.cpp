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
#include "mission/missionparse.h"
#include "missioneditor/common.h"

#include <globalincs/linklist.h>
#include <hud/hudsquadmsg.h>
#include <localization/localize.h>
#include <mission/object.h>

#include <QtWidgets>

namespace fso::fred::dialogs {
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
	} else {
		if (cur_state) {
			return Qt::PartiallyChecked;
		}
	}
	if (cur_state == 1) {

		return Qt::Checked;
	} else {
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
std::vector<std::pair<SCP_string, bool>> ShipEditorDialogModel::getAcceptedOrders() const
{
	std::vector<std::pair<SCP_string, bool>> acceptedOrders;
	object* objp;
	SCP_set<size_t> default_orders;
	if (!multi_edit) {
		default_orders = ship_get_default_orders_accepted(&Ship_info[Ships[_editor->cur_ship].ship_info_index]);
	} else {
		for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				(objp->flags[Object::Object_Flags::Marked])) {
				const SCP_set<size_t>& these_orders =
					ship_get_default_orders_accepted(&Ship_info[Ships[objp->instance].ship_info_index]);

				if (default_orders.empty()) {
					default_orders = these_orders;
				} else {
					Assert(default_orders == these_orders);
				}
			}
		}
	}

	for (size_t order_id : default_orders) {
		SCP_string name = Player_orders[order_id].localized_name;
		bool state = false;
		const SCP_set<size_t>& orders_accepted = Ships[_editor->cur_ship].orders_accepted;
		if (orders_accepted.contains(order_id))
			state = true;
		acceptedOrders.emplace_back(name, state);
	}
	return acceptedOrders;
}
void ShipEditorDialogModel::setAcceptedOrders(const std::vector<std::pair<SCP_string, bool>>& newOrders)
{
	orders = newOrders;
	// Write directly to all marked ships
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			auto i = ptr->instance;
			SCP_set<size_t> default_orders = ship_get_default_orders_accepted(&Ship_info[Ships[i].ship_info_index]);
			SCP_set<size_t> new_orders_set;
			for (size_t order_id : default_orders) {
				for (const auto& order : orders) {
					if (order.first == Player_orders[order_id].localized_name) {
						if (order.second) {
							new_orders_set.insert(order_id);
						}
					}
				}
			}
			Ships[i].orders_accepted = new_orders_set;
		}
	}
	set_modified();
	_editor->missionChanged();
}

void ShipEditorDialogModel::setArrivalPaths(const std::vector<std::pair<SCP_string, bool>>& newPaths)
{
	arrivalPaths = newPaths;
	if (!newPaths.empty() && !multi_edit && single_ship >= 0) {
		int num_allowed = 0;
		int m_path_mask = 0;
		for (int i = 0; i < static_cast<int>(newPaths.size()); i++) {
			if (newPaths[i].second) {
				m_path_mask |= (1 << i);
				num_allowed++;
			}
		}
		if (num_allowed == static_cast<int>(newPaths.size())) {
			m_path_mask = 0;
		}
		Ships[single_ship].arrival_path_mask = m_path_mask;
		set_modified();
		_editor->missionChanged();
	}
}

void ShipEditorDialogModel::setDeparturePaths(const std::vector<std::pair<SCP_string, bool>>& newPaths)
{
	departurePaths = newPaths;
	if (!newPaths.empty() && !multi_edit && single_ship >= 0) {
		int num_allowed = 0;
		int m_path_mask = 0;
		for (int i = 0; i < static_cast<int>(newPaths.size()); i++) {
			if (newPaths[i].second) {
				m_path_mask |= (1 << i);
				num_allowed++;
			}
		}
		if (num_allowed == static_cast<int>(newPaths.size())) {
			m_path_mask = 0;
		}
		Ships[single_ship].departure_path_mask = m_path_mask;
		set_modified();
		_editor->missionChanged();
	}
}
void ShipEditorDialogModel::initializeData()
{
	int type, wing = -1;
	int cargo = 0, base_ship, base_player, pship = -1;
	int escort_count;
	respawn_priority = 0;
	texenable = true;
	std::set<size_t> current_orders;
	pship_count = 0; // a total count of the player ships not marked
	player_count = 0;
	ship_count = 0;
	cue_init = 0;
	total_count = 0;
	pvalid_count = 0;
	player_ship = 0;
	ship_orders.clear();
	arrivalPaths.clear();
	departurePaths.clear();
	_m_alt_name = "";
	_m_callsign = "";
	object* objp;
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		mission_type = 0; // multi player mission
	} else {
		mission_type = 1; // non-multiplayer mission (implies single player mission I guess)
	}
	ship_count = player_count = escort_count = pship_count = pvalid_count = 0;
	base_ship = base_player = -1;
	enable = true;
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_SHIP) && (Ships[objp->instance].flags[Ship::Ship_Flags::Escort])) {
			escort_count++; // get a total count of escort ships
		}

		if (objp->type == OBJ_START) {
			pship_count++; // count all player starts in mission
		}

		if (objp->flags[Object::Object_Flags::Marked]) {
			type = objp->type;
			if ((type == OBJ_START) && !mission_type) { // in multiplayer missions, starts act like ships
				type = OBJ_SHIP;
			}

			auto i = -1;
			if (type == OBJ_START) {
				player_count++;
				// if player_count is 1, base_player will be the one and only player
				i = base_player = objp->instance;

			} else if (type == OBJ_SHIP) {
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

	total_count = ship_count + player_count; // get total number of objects being edited.
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

	ship_orders.clear(); // assume they are all the same type
	if (ship_count) {
		if (!multi_edit) {
			Assert((ship_count == 1) && (base_ship >= 0)); // NOLINT
			_m_ship_name = Ships[base_ship].ship_name;
			_m_ship_display_name = Ships[base_ship].has_display_name() ? Ships[base_ship].get_display_name() : "<none>";
		} else {
			_m_ship_name = "";
			_m_ship_display_name = "";
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
					} else {
						if (Ships[i].ship_info_index != _m_ship_class) {
							_m_ship_class = -1;
							texenable = false;
						}
						if (Ships[i].team != _m_team) {
							_m_team = -1;
						}
						pship = tristate_set(Objects[Ships[i].objnum].type == OBJ_START, pship);
					}
					// 'and' in the ship type of this ship to our running bitfield
					current_orders = ship_get_default_orders_accepted(&Ship_info[Ships[i].ship_info_index]);
					if (ship_orders.empty()) {
						ship_orders = current_orders;
					} else if (ship_orders != current_orders) {
						ship_orders = {std::numeric_limits<size_t>::max()};
					}

					if (Ships[i].flags[Ship::Ship_Flags::Escort]) {
						escort_count--; // remove marked escorts from count
					}

					if (Objects[Ships[i].objnum].type == OBJ_START) {
						pship_count--; // removed marked starts from count
					}
					if ((objp->type == OBJ_SHIP) || ((objp->type == OBJ_START) && !mission_type)) {
						// process this if ship not in a wing
						if (Ships[i].wingnum < 0) {
							if (!cue_init) {
								cue_init = 1;
								_m_arrival_tree_formula = Ships[i].arrival_cue;
								_m_departure_tree_formula = Ships[i].departure_cue;
								_m_arrival_location = static_cast<int>(Ships[i].arrival_location);
								_m_arrival_dist = Ships[i].arrival_distance;
								_m_arrival_target = anchor_to_target(Ships[i].arrival_anchor);
								_m_arrival_delay = Ships[i].arrival_delay;
								_m_departure_location = static_cast<int>(Ships[i].departure_location);
								_m_departure_delay = Ships[i].departure_delay;
								_m_departure_target = anchor_to_target(Ships[i].departure_anchor);

							} else {
								cue_init++;
								if (static_cast<int>(Ships[i].arrival_location) != _m_arrival_location) {
									_m_arrival_location = -1;
								}

								if (static_cast<int>(Ships[i].departure_location) != _m_departure_location) {
									_m_departure_location = -1;
								}

								_m_arrival_dist = Ships[i].arrival_distance;
								_m_arrival_delay = Ships[i].arrival_delay;
								_m_departure_delay = Ships[i].departure_delay;

								if (Ships[i].arrival_anchor != target_to_anchor(_m_arrival_target)) {
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

								if (Ships[i].departure_anchor != target_to_anchor(_m_departure_target)) {
									_m_departure_target = -1;
								}
							}
						}
						// process the first ship in group, else process the rest
						if (base_ship >= 0) {
							_m_ai_class = Ships[i].weapons.ai_class;
							cargo = Ships[i].cargo1;
							_m_cargo1 = Cargo_names[cargo];
							_m_cargo_title = Ships[i].cargo_title;
							_m_hotkey = Ships[i].hotkey + 1;
							_m_score = Ships[i].score;
							_m_assist_score = static_cast<int>(Ships[i].assist_score_pct * 100);

							_m_persona = Ships[i].persona_index;
							_m_alt_name = Fred_alt_names[base_ship];
							_m_callsign = Fred_callsigns[base_ship];
							if (The_mission.game_type & MISSION_TYPE_MULTI) {
								respawn_priority = Ships[i].respawn_priority;
							}
							// we use final_death_time member of ship structure for holding the amount of time before a
							// mission to destroy this ship
							wing = Ships[i].wingnum;
							if (wing < 0) {
								m_wing = "None";

							} else {
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

						} else {
							if (Ships[i].weapons.ai_class != _m_ai_class) {
								_m_ai_class = -1;
							}

							if (Ships[i].cargo1 != cargo) {
								_m_cargo1 = "";
							}

							if (_m_cargo_title != Ships[i].cargo_title) {
								_m_cargo_title = "";
							}

							_m_score = Ships[i].score;
							_m_assist_score = static_cast<int>(Ships[i].assist_score_pct * 100);

							if (Ships[i].hotkey != _m_hotkey - 1) {
								_m_hotkey = -1;
							}

							if (Ships[i].persona_index != _m_persona) {
								_m_persona = -2;
							}

							if (Ships[i].wingnum != wing) {
								m_wing = "";
							}

							_m_no_arrival_warp =
								tristate_set(Ships[i].flags[Ship::Ship_Flags::No_arrival_warp], _m_no_arrival_warp);
							_m_no_departure_warp =
								tristate_set(Ships[i].flags[Ship::Ship_Flags::No_departure_warp], _m_no_departure_warp);
						}
					}
				}
			}

			objp = GET_NEXT(objp);
		}

		_m_player_ship = pship;

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
			_m_ship_display_name = "";
			_m_player_ship = true;
			objp = GET_FIRST(&obj_used_list);
			while (objp != END_OF_LIST(&obj_used_list)) {
				if ((objp->type == OBJ_START) && (objp->flags[Object::Object_Flags::Marked])) {
					auto i = objp->instance;
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

				objp = GET_NEXT(objp);
			}
			// only 1 player selected..
		} else if (query_valid_object(_editor->currentObject) && (Objects[_editor->currentObject].type == OBJ_START)) {
			// Assert((player_count == 1) && !multi_edit);
			player_ship = Objects[_editor->currentObject].instance;
			_m_ship_name = Ships[player_ship].ship_name;
			_m_ship_display_name =
				Ships[player_ship].has_display_name() ? Ships[player_ship].get_display_name() : "<none>";
			_m_ship_class = Ships[player_ship].ship_info_index;
			_m_team = Ships[player_ship].team;
			_m_player_ship = true;
			_m_alt_name = Fred_alt_names[player_ship];
			_m_callsign = Fred_callsigns[player_ship];

		} else { // no ships or players selected..
			_m_ship_name = "";
			_m_ship_display_name = "";
			_m_ship_class = -1;
			_m_team = -1;
			_m_persona = -1;
			_m_player_ship = false;
		}

		_m_ai_class = -1;
		_m_cargo1 = "";
		_m_cargo_title = "";
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
		arrivalPaths.clear();
		departurePaths.clear();
	}

	// Compute common layer across all marked ships/players
	{
		_m_layer = "";
		bool first = true;
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) &&
				ptr->flags[Object::Object_Flags::Marked]) {
				SCP_string layer = _viewport->getObjectLayerName(OBJ_INDEX(ptr));
				if (first) {
					_m_layer = layer;
					first = false;
				} else if (_m_layer != layer) {
					_m_layer = "";
					break;
				}
			}
		}
	}

	modelChanged();
}

std::vector<std::pair<SCP_string, bool>> ShipEditorDialogModel::getArrivalPaths() const
{
	std::vector<std::pair<SCP_string, bool>> m_path_list;
	if (_m_arrival_target < 0 || _m_arrival_target >= MAX_SHIPS)
		return m_path_list;
	int target_class = Ships[_m_arrival_target].ship_info_index;
	auto m_model = model_get(Ship_info[target_class].model_num);
	if (!m_model || !m_model->ship_bay || m_model->ship_bay->num_paths <= 0)
		return m_path_list;
	auto m_num_paths = m_model->ship_bay->num_paths;
	auto m_path_mask = Ships[single_ship].arrival_path_mask;

	for (int i = 0; i < m_num_paths; i++) {
		SCP_string name("Path ");
		name += i2ch(i + 1);
		bool allowed;
		if (m_path_mask == 0) {
			allowed = true;
		} else {
			allowed = (m_path_mask & (1 << i)) != 0;
		}
		m_path_list.emplace_back(name, allowed);
	}
	return m_path_list;
}
std::vector<std::pair<SCP_string, bool>> ShipEditorDialogModel::getDeparturePaths() const
{
	std::vector<std::pair<SCP_string, bool>> m_path_list;
	if (_m_departure_target < 0 || _m_departure_target >= MAX_SHIPS)
		return m_path_list;
	int target_class = Ships[_m_departure_target].ship_info_index;
	auto m_model = model_get(Ship_info[target_class].model_num);
	if (!m_model || !m_model->ship_bay || m_model->ship_bay->num_paths <= 0)
		return m_path_list;
	auto m_num_paths = m_model->ship_bay->num_paths;
	auto m_path_mask = Ships[single_ship].departure_path_mask;

	for (int i = 0; i < m_num_paths; i++) {
		SCP_string name("Path ");
		name += i2ch(i + 1);
		bool allowed;
		if (m_path_mask == 0) {
			allowed = true;
		} else {
			allowed = (m_path_mask & (1 << i)) != 0;
		}
		m_path_list.emplace_back(name, allowed);
	}
	return m_path_list;
}
bool ShipEditorDialogModel::apply()
{
	return true;
}

void ShipEditorDialogModel::reject() {}
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
		{DialogButton::Ok});
}

void ShipEditorDialogModel::ship_callsign_close(int ship)
{

	if (multi_edit) {
		return;
	}
	drop_white_space(_m_callsign);
	if (_m_callsign.empty()) {
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
		{DialogButton::Ok});
}

void ShipEditorDialogModel::setShipName(const SCP_string& m_ship_name)
{
	if (_m_ship_name == m_ship_name)
		return;

	// Name changes only apply to single-ship editing
	if (multi_edit || single_ship < 0)
		return;

	SCP_string new_name = m_ship_name;
	drop_white_space(new_name);

	if (new_name.empty()) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Ship Name Error",
			"A ship name cannot be empty.",
			{DialogButton::Ok});
		_m_ship_name = Ships[single_ship].ship_name;
		modelChanged();
		return;
	}

	if (new_name[0] == '<') {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Ship Name Error",
			"Ship names not allowed to begin with <.",
			{DialogButton::Ok});
		_m_ship_name = Ships[single_ship].ship_name;
		modelChanged();
		return;
	}

	// Check for duplicate ship names
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->instance != single_ship)) {
			if (!stricmp(new_name.c_str(), Ships[ptr->instance].ship_name)) {
				_viewport->dialogProvider->showButtonDialog(DialogType::Error,
					"Ship Name Error",
					"This ship name is already being used by another ship.",
					{DialogButton::Ok});
				_m_ship_name = Ships[single_ship].ship_name;
				modelChanged();
				return;
			}
		}
	}

	// Check for conflict with wing names
	for (auto& w : Wings) {
		if (w.wave_count && !stricmp(w.name, new_name.c_str())) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Ship Name Error",
				"This ship name is already being used by a wing.",
				{DialogButton::Ok});
			_m_ship_name = Ships[single_ship].ship_name;
			modelChanged();
			return;
		}
	}

	// Check for conflict with AI target priority group names
	for (auto& tp : Ai_tp_list) {
		if (!stricmp(new_name.c_str(), tp.name)) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Ship Name Error",
				"This ship name is already being used by a target priority group.",
				{DialogButton::Ok});
			_m_ship_name = Ships[single_ship].ship_name;
			modelChanged();
			return;
		}
	}

	// Check for conflict with waypoint paths
	if (find_matching_waypoint_list(new_name.c_str()) != nullptr) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Ship Name Error",
			"This ship name is already being used by a waypoint path.",
			{DialogButton::Ok});
		_m_ship_name = Ships[single_ship].ship_name;
		modelChanged();
		return;
	}

	// Check for conflict with jump nodes
	if (jumpnode_get_by_name(new_name.c_str()) != nullptr) {
		_viewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Ship Name Error",
			"This ship name is already being used by a jump node.",
			{DialogButton::Ok});
		_m_ship_name = Ships[single_ship].ship_name;
		modelChanged();
		return;
	}

	// Wing member ships cannot be renamed independently
	int wing = Ships[single_ship].wingnum;
	if (wing >= 0) {
		Assert((wing < MAX_WINGS) && Wings[wing].wave_count); // NOLINT
		char expected_name[255];
		int j;
		for (j = 0; j < Wings[wing].wave_count; j++)
			if (_editor->wing_objects[wing][j] == Ships[single_ship].objnum)
				break;
		Assert(j < Wings[wing].wave_count);
		wing_bash_ship_name(expected_name, Wings[wing].name, static_cast<int>(j + 1));
		if (strcmp(expected_name, new_name.c_str())) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Ship Name Error",
				"This ship is part of a wing, and its name cannot be changed",
				{DialogButton::Ok});
			_m_ship_name = Ships[single_ship].ship_name;
			modelChanged();
			return;
		}
	}

	// All validation passed — write the new name
	char old_name[NAME_LENGTH];
	strcpy_s(old_name, Ships[single_ship].ship_name);
	strcpy_s(Ships[single_ship].ship_name, new_name.c_str());
	_m_ship_name = new_name;

	if (strcmp(old_name, Ships[single_ship].ship_name)) {
		update_sexp_references(old_name, Ships[single_ship].ship_name);
		_editor->ai_update_goal_references(sexp_ref_type::SHIP, old_name, Ships[single_ship].ship_name);
		_editor->update_texture_replacements(old_name, Ships[single_ship].ship_name);
		for (int j = 0; j < Num_reinforcements; j++) {
			if (!strcmp(old_name, Reinforcements[j].name)) {
				Assert(strlen(Ships[single_ship].ship_name) < NAME_LENGTH);
				strcpy_s(Reinforcements[j].name, Ships[single_ship].ship_name);
			}
		}
	}

	set_modified();
	_editor->missionChanged();
	modelChanged();
}
SCP_string ShipEditorDialogModel::getShipName() const
{
	return _m_ship_name;
}

void ShipEditorDialogModel::setShipDisplayName(const SCP_string& m_ship_display_name)
{
	if (_m_ship_display_name == m_ship_display_name)
		return;
	_m_ship_display_name = m_ship_display_name;

	// Display name only applies to single-ship editing
	if (!multi_edit && single_ship >= 0) {
		SCP_string display = m_ship_display_name;
		lcl_fred_replace_stuff(display);
		if (display == _m_ship_name || stricmp(display.c_str(), "<none>") == 0) {
			Ships[single_ship].display_name = "";
			Ships[single_ship].flags.remove(Ship::Ship_Flags::Has_display_name);
		} else {
			Ships[single_ship].display_name = display;
			Ships[single_ship].flags.set(Ship::Ship_Flags::Has_display_name);
		}
		set_modified();
		_editor->missionChanged();
	}
	modelChanged();
}
SCP_string ShipEditorDialogModel::getShipDisplayName() const
{
	return _m_ship_display_name;
}

void ShipEditorDialogModel::setShipClass(int m_ship_class)
{
	if (_m_ship_class == m_ship_class)
		return;
	_m_ship_class = m_ship_class;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].ship_info_index != m_ship_class) {
				change_ship_type(ptr->instance, m_ship_class);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getShipClass() const
{
	return _m_ship_class;
}

void ShipEditorDialogModel::setAIClass(const int m_ai_class)
{
	if (_m_ai_class == m_ai_class)
		return;
	_m_ai_class = m_ai_class;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].weapons.ai_class = m_ai_class;
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getAIClass() const
{
	return _m_ai_class;
}

void ShipEditorDialogModel::setTeam(const int m_team)
{
	if (_m_team == m_team)
		return;
	_m_team = m_team;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].team = m_team;
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getTeam() const
{
	return _m_team;
}

SCP_string ShipEditorDialogModel::getLayer() const
{
	return _m_layer;
}

void ShipEditorDialogModel::setLayer(const SCP_string& layer)
{
	if (_m_layer == layer)
		return;
	_m_layer = layer;

	for (auto objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (objp->flags[Object::Object_Flags::Marked]) {
			if (objp->type == OBJ_SHIP || objp->type == OBJ_START) {
				int obj_idx = OBJ_INDEX(objp);
				_viewport->moveObjectToLayer(obj_idx, layer);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

void ShipEditorDialogModel::setCargo(const SCP_string& m_cargo)
{
	if (_m_cargo1 == m_cargo)
		return;
	_m_cargo1 = m_cargo;

	SCP_string cargo_name = m_cargo;
	lcl_fred_replace_stuff(cargo_name);
	int z = string_lookup(cargo_name.c_str(), Cargo_names, Num_cargo);
	if (z == -1) {
		if (Num_cargo < MAX_CARGO) {
			z = Num_cargo++;
			strcpy(Cargo_names[z], cargo_name.c_str());
		} else {
			SCP_string str;
			sprintf(str, "Maximum number of cargo names %d reached.\nIgnoring new name.\n", MAX_CARGO);
			_viewport->dialogProvider->showButtonDialog(DialogType::Warning, "Cargo Error", str, {DialogButton::Ok});
			z = 0;
			_m_cargo1 = Cargo_names[z];
		}
	}
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].cargo1 = (char)z;
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

SCP_string ShipEditorDialogModel::getCargo() const
{
	return _m_cargo1;
}

void ShipEditorDialogModel::setCargoTitle(const SCP_string& title)
{
	if (_m_cargo_title == title)
		return;
	_m_cargo_title = title;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			strcpy_s(Ships[ptr->instance].cargo_title, title.c_str());
		}
	}
	set_modified();
}

SCP_string ShipEditorDialogModel::getCargoTitle() const
{
	return _m_cargo_title;
}

void ShipEditorDialogModel::setAltName(const SCP_string& m_altName)
{
	if (_m_alt_name == m_altName)
		return;
	_m_alt_name = m_altName;
	if (!multi_edit && single_ship >= 0) {
		ship_alt_name_close(single_ship);
		set_modified();
		_editor->missionChanged();
	}
	modelChanged();
}

SCP_string ShipEditorDialogModel::getAltName() const
{
	return _m_alt_name;
}

void ShipEditorDialogModel::setCallsign(const SCP_string& m_callsign)
{
	if (_m_callsign == m_callsign)
		return;
	_m_callsign = m_callsign;
	if (!multi_edit && single_ship >= 0) {
		ship_callsign_close(single_ship);
		set_modified();
		_editor->missionChanged();
	}
	modelChanged();
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
	if (_m_hotkey == m_hotkey)
		return;
	_m_hotkey = m_hotkey;
	// hotkey stored as 1-indexed in model (0 = none), Ships[] uses 0-indexed (-1 = none, hotkey-1 otherwise)
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].hotkey = m_hotkey - 1;
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getHotkey() const
{
	return _m_hotkey;
}

void ShipEditorDialogModel::setPersona(const int m_persona)
{
	if (_m_persona == m_persona)
		return;
	_m_persona = m_persona;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].persona_index = m_persona;
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getPersona() const
{
	return _m_persona;
}

void ShipEditorDialogModel::setScore(const int m_score)
{
	if (_m_score == m_score)
		return;
	_m_score = m_score;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].score = m_score;
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getScore() const
{
	return _m_score;
}

void ShipEditorDialogModel::setAssist(const int m_assist_score)
{
	if (_m_assist_score == m_assist_score)
		return;
	_m_assist_score = m_assist_score;
	float pct = static_cast<float>(m_assist_score) / 100.0f;
	pct = std::clamp(pct, 0.0f, 1.0f);
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			Ships[ptr->instance].assist_score_pct = pct;
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getAssist() const
{
	return _m_assist_score;
}

void ShipEditorDialogModel::setPlayer(const bool m_player)
{
	if (_m_player_ship == m_player)
		return;
	_m_player_ship = m_player;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (m_player) {
				if (Objects[Ships[ptr->instance].objnum].type != OBJ_START) {
					Player_starts++;
				}
				Objects[Ships[ptr->instance].objnum].type = OBJ_START;
			} else {
				if (Objects[Ships[ptr->instance].objnum].type == OBJ_START) {
					Player_starts--;
				}
				Objects[Ships[ptr->instance].objnum].type = OBJ_SHIP;
			}
		}
	}
	// Fix up Player_start_shipnum if it became invalid
	if (Player_start_shipnum < 0 || Objects[Ships[Player_start_shipnum].objnum].type != OBJ_START) {
		for (auto* p = GET_FIRST(&obj_used_list); p != END_OF_LIST(&obj_used_list); p = GET_NEXT(p)) {
			if (p->type == OBJ_START) {
				Player_start_shipnum = p->instance;
				break;
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

bool ShipEditorDialogModel::getPlayer() const
{
	return _m_player_ship;
}

void ShipEditorDialogModel::setRespawn(const int value)
{
	if (respawn_priority == value)
		return;
	respawn_priority = value;
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				Ships[ptr->instance].respawn_priority = value;
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getRespawn() const
{
	return respawn_priority;
}

void ShipEditorDialogModel::setArrivalLocationIndex(const int value)
{
	if (_m_arrival_location == value)
		return;
	_m_arrival_location = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].arrival_location = static_cast<ArrivalLocation>(value);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getArrivalLocationIndex() const
{
	return _m_arrival_location;
}

void ShipEditorDialogModel::setArrivalLocation(const ArrivalLocation value)
{
	setArrivalLocationIndex(static_cast<int>(value));
}

ArrivalLocation ShipEditorDialogModel::getArrivalLocation() const
{
	return static_cast<ArrivalLocation>(_m_arrival_location);
}

int ShipEditorDialogModel::computeArrivalMinDist() const
{
	// Validation only applies when arriving near a ship (not hyperspace or dock bay)
	if (getArrivalLocation() == ArrivalLocation::AT_LOCATION ||
	    getArrivalLocation() == ArrivalLocation::FROM_DOCK_BAY)
		return 0;

	// Validation doesn't apply to special anchors (negative value or ANCHOR_SPECIAL_ARRIVAL flag set)
	if (_m_arrival_target < 0 || (_m_arrival_target & ANCHOR_SPECIAL_ARRIVAL))
		return 0;

	// Compute the most restrictive minimum distance across all marked arriving ships
	int max_d = 0;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) &&
		    ptr->flags[Object::Object_Flags::Marked] &&
		    Ships[ptr->instance].wingnum < 0 &&
		    Ships[ptr->instance].objnum >= 0) {
			const int d = static_cast<int>(std::min(MIN_TARGET_ARRIVAL_DISTANCE,
			    MIN_TARGET_ARRIVAL_MULTIPLIER * Objects[Ships[ptr->instance].objnum].radius));
			max_d = std::max(max_d, d);
		}
	}
	return max_d;
}

void ShipEditorDialogModel::setArrivalTarget(const int value)
{
	if (_m_arrival_target == value)
		return;
	_m_arrival_target = value;

	// Re-validate the existing arrival distance now that the target has changed.
	// A target change from a special anchor to a real ship can make a previously
	// acceptable distance too close.
	const int min_dist = computeArrivalMinDist();
	if (min_dist > 0 && _m_arrival_dist > -min_dist && _m_arrival_dist < min_dist) {
		const int clamped = (_m_arrival_dist < 0) ? -min_dist : min_dist;
		QMessageBox::warning(nullptr,
		    tr("Arrival Distance"),
		    tr("Ship must arrive at least %1 meters away from target.\n"
		       "Value has been reset to this.  Use with caution!")
		        .arg(min_dist));
		_m_arrival_dist = clamped;
		for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) &&
			    ptr->flags[Object::Object_Flags::Marked] &&
			    Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].arrival_distance = clamped;
			}
		}
	}

	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0 && value >= 0) {
				Ships[ptr->instance].arrival_anchor = target_to_anchor(value);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getArrivalTarget() const
{
	return _m_arrival_target;
}

void ShipEditorDialogModel::setArrivalDistance(const int value)
{
	if (_m_arrival_dist == value)
		return;

	const int min_dist = computeArrivalMinDist();
	int effective_value = value;
	if (min_dist > 0 && value > -min_dist && value < min_dist) {
		effective_value = (value < 0) ? -min_dist : min_dist;
		QMessageBox::warning(nullptr,
		    tr("Arrival Distance"),
		    tr("Ship must arrive at least %1 meters away from target.\n"
		       "Value has been reset to this.  Use with caution!")
		        .arg(min_dist));
	}

	_m_arrival_dist = effective_value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].arrival_distance = effective_value;
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getArrivalDistance() const
{
	return _m_arrival_dist;
}

void ShipEditorDialogModel::setArrivalDelay(const int value)
{
	if (_m_arrival_delay == value)
		return;
	_m_arrival_delay = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].arrival_delay = value;
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
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
	if (_m_no_arrival_warp == value)
		return;
	_m_no_arrival_warp = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (value) {
				Ships[ptr->instance].flags.set(Ship::Ship_Flags::No_arrival_warp);
			} else {
				Ships[ptr->instance].flags.remove(Ship::Ship_Flags::No_arrival_warp);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getNoArrivalWarp() const
{
	return _m_no_arrival_warp;
}

void ShipEditorDialogModel::setDepartureLocationIndex(const int value)
{
	if (_m_departure_location == value)
		return;
	_m_departure_location = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].departure_location = static_cast<DepartureLocation>(value);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getDepartureLocationIndex() const
{
	return _m_departure_location;
}

void ShipEditorDialogModel::setDepartureLocation(const DepartureLocation value)
{
	setDepartureLocationIndex(static_cast<int>(value));
}

DepartureLocation ShipEditorDialogModel::getDepartureLocation() const
{
	return static_cast<DepartureLocation>(_m_departure_location);
}

void ShipEditorDialogModel::setDepartureTarget(const int value)
{
	if (_m_departure_target == value)
		return;
	_m_departure_target = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0 && value >= 0) {
				Ships[ptr->instance].departure_anchor = target_to_anchor(value);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getDepartureTarget() const
{
	return _m_departure_target;
}

void ShipEditorDialogModel::setDepartureDelay(const int value)
{
	if (_m_departure_delay == value)
		return;
	_m_departure_delay = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].wingnum < 0) {
				Ships[ptr->instance].departure_delay = value;
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
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
	if (_m_no_departure_warp == value)
		return;
	_m_no_departure_warp = value;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (value) {
				Ships[ptr->instance].flags.set(Ship::Ship_Flags::No_departure_warp);
			} else {
				Ships[ptr->instance].flags.remove(Ship::Ship_Flags::No_departure_warp);
			}
		}
	}
	set_modified();
	_editor->missionChanged();
	modelChanged();
}

int ShipEditorDialogModel::getNoDepartureWarp() const
{
	return _m_no_departure_warp;
}

void ShipEditorDialogModel::OnPrevious()
{
	int i, n, arr[MAX_SHIPS];
	n = make_ship_list(arr);
	if (!n) {
		return;
	}
	if (_editor->cur_ship < 0) {
		i = n - 1;
	} else {
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
}

void ShipEditorDialogModel::OnNext()
{
	int i, n, arr[MAX_SHIPS];
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

	setCargo("Nothing");
	setAIClass(AI_DEFAULT_CLASS);
	if (_m_ship_class >= 0) {
		setTeam(Species_info[Ship_info[_m_ship_class].species].default_iff);
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
			{DialogButton::Ok});
	} else {

		_viewport->dialogProvider->showButtonDialog(DialogType::Information,
			"Reset",
			"Ship reset to ship class defaults",
			{DialogButton::Ok});
	}
}

bool ShipEditorDialogModel::wing_is_player_wing(const int wing) const
{
	return _editor->wing_is_player_wing(wing);
}

const std::set<size_t>& ShipEditorDialogModel::getShipOrders() const
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
} // namespace fso::fred::dialogs