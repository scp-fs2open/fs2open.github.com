#include "ShipFlagsDialogModel.h"

#include <object/object.h>

namespace fso::fred::dialogs {

// Flags that are not applicable to player start ships and should be hidden/skipped for them.
// Add entries here to extend the list.
static const Ship::Ship_Flags player_start_hidden_flags[] = {
	Ship::Ship_Flags::Reinforcement,
	Ship::Ship_Flags::Kill_before_mission,
};

int ShipFlagsDialogModel::tristateSet(int val, int curState)
{
	// curState uses Qt::CheckState encoding (0=Unchecked, 1=PartiallyChecked, 2=Checked)
	if (curState == Qt::PartiallyChecked)
		return Qt::PartiallyChecked;
	bool cur_bool = (curState == Qt::Checked);
	if (static_cast<bool>(val) != cur_bool)
		return Qt::PartiallyChecked;
	return curState;
}

std::pair<SCP_string, int>* ShipFlagsDialogModel::getFlag(const SCP_string& flagName)
{
	for (auto& flag : _flags) {
		if (!stricmp(flagName.c_str(), flag.first.c_str())) {
			return &flag;
		}
	}
	// Only assert if the name isn't a known flag at all; it may have been legitimately filtered out for ship starts
	bool known = false;
	for (size_t i = 0; i < Num_Parse_ship_flags && !known; ++i)
		known = !stricmp(flagName.c_str(), Parse_ship_flags[i].name);
	for (size_t i = 0; i < Num_Parse_ship_ai_flags && !known; ++i)
		known = !stricmp(flagName.c_str(), Parse_ship_ai_flags[i].name);
	for (size_t i = 0; i < Num_Parse_ship_object_flags && !known; ++i)
		known = !stricmp(flagName.c_str(), Parse_ship_object_flags[i].name);
	Assertion(known, "Illegal flag name \"%s\"", flagName.c_str());
	return nullptr;
}

void ShipFlagsDialogModel::setFlag(const SCP_string& flagName, int state)
{
	for (auto& flag : _flags) {
		if (!stricmp(flagName.c_str(), flag.first.c_str())) {
			flag.second = state;
			set_modified();
		}
	}
}

void ShipFlagsDialogModel::setDestroyTime(int destroyTime)
{
	modify(_destroyTime, destroyTime);
}

int ShipFlagsDialogModel::getDestroyTime() const
{
	return _destroyTime;
}

void ShipFlagsDialogModel::setEscortPriority(int priority)
{
	modify(_escortPriority, priority);
}

int ShipFlagsDialogModel::getEscortPriority() const
{
	return _escortPriority;
}

void ShipFlagsDialogModel::setKamikazeDamage(int damage)
{
	modify(_kamikazeDamage, damage);
}

int ShipFlagsDialogModel::getKamikazeDamage() const
{
	return _kamikazeDamage;
}

void ShipFlagsDialogModel::updateShip(int shipNum)
{
	ship* shipp = &Ships[shipNum];
	object* objp = &Objects[shipp->objnum];
	for (const auto& [name, checked] : _flags) {
		// PartiallyChecked means mixed selection — leave each ship's flag as-is
		if (checked == Qt::PartiallyChecked)
			continue;
		const bool set = (checked == Qt::Checked);
		for (size_t i = 0; i < Num_Parse_ship_flags; ++i) {
			if (!stricmp(name.c_str(), Parse_ship_flags[i].name)) {

				// Skip flags that aren't applicable to player start ships, even if they were shown and edited in multi edit mode
				if (objp->type == OBJ_START) {
					bool hidden = false;
					for (const auto& hf : player_start_hidden_flags) {
						if (Parse_ship_flags[i].def == hf) { hidden = true; break; }
					}
					if (hidden) continue;
				}

				if (Parse_ship_flags[i].def == Ship::Ship_Flags::Reinforcement) {
					_editor->set_reinforcement(shipp->ship_name, set);
				} else {
					if (set) {
						shipp->flags.set(Parse_ship_flags[i].def);
					} else {
						shipp->flags.remove(Parse_ship_flags[i].def);
					}
				}
				continue;
			}
		}
		for (size_t i = 0; i < Num_Parse_ship_ai_flags; ++i) {
			if (!stricmp(name.c_str(), Parse_ship_ai_flags[i].name)) {
				if (set) {
					Ai_info[shipp->ai_index].ai_flags.set(Parse_ship_ai_flags[i].def);
				} else {
					Ai_info[shipp->ai_index].ai_flags.remove(Parse_ship_ai_flags[i].def);
				}
				continue;
			}
		}
		for (size_t i = 0; i < Num_Parse_ship_object_flags; ++i) {
			if (!stricmp(name.c_str(), Parse_ship_object_flags[i].name)) {
				if (Parse_ship_object_flags[i].def == Object::Object_Flags::Collides) {
					if (set) {
						objp->flags.remove(Parse_ship_object_flags[i].def);
					} else {
						objp->flags.set(Parse_ship_object_flags[i].def);
					}
				} else {
					if (set) {
						objp->flags.set(Parse_ship_object_flags[i].def);
					} else {
						objp->flags.remove(Parse_ship_object_flags[i].def);
					}
				}
				continue;
			}
		}
	}
	Ai_info[shipp->ai_index].kamikaze_damage = _kamikazeDamage;
	shipp->escort_priority = _escortPriority;
	shipp->final_death_time = _destroyTime;
}

ShipFlagsDialogModel::ShipFlagsDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

bool ShipFlagsDialogModel::apply()
{
	object* objp;

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				updateShip(objp->instance);
			}
		}
		objp = GET_NEXT(objp);
	}

	return true;
}

void ShipFlagsDialogModel::reject() {}

const SCP_vector<std::pair<SCP_string, int>>& ShipFlagsDialogModel::getFlagsList()
{
	return _flags;
}

SCP_vector<std::pair<SCP_string, SCP_string>> ShipFlagsDialogModel::getShipFlagDescriptions()
{
	const size_t num_ship_descs = Num_ship_flag_descriptions;
	const size_t num_ai_descs   = Num_ai_flag_descriptions;
	const size_t num_obj_descs  = Num_object_flag_descriptions;

	SCP_vector<std::pair<SCP_string, SCP_string>> descriptions;

	for (size_t i = 0; i < Num_Parse_ship_flags; ++i) {
		const auto& flagDef = Parse_ship_flags[i];
		if (flagDef.def == Ship::Ship_Flags::No_arrival_warp ||
			flagDef.def == Ship::Ship_Flags::No_departure_warp ||
			flagDef.def == Ship::Ship_Flags::Same_arrival_warp_when_docked ||
			flagDef.def == Ship::Ship_Flags::Same_departure_warp_when_docked ||
			flagDef.def == Ship::Ship_Flags::Primaries_locked ||
			flagDef.def == Ship::Ship_Flags::Secondaries_locked ||
			flagDef.def == Ship::Ship_Flags::Ship_locked ||
			flagDef.def == Ship::Ship_Flags::Weapons_locked ||
			flagDef.def == Ship::Ship_Flags::Afterburner_locked ||
			flagDef.def == Ship::Ship_Flags::Lock_all_turrets_initially ||
			flagDef.def == Ship::Ship_Flags::Force_shields_on) {
			continue;
		}
		for (size_t j = 0; j < num_ship_descs; ++j) {
			if (Ship_flag_descriptions[j].flag == flagDef.def) {
				descriptions.emplace_back(flagDef.name, Ship_flag_descriptions[j].flag_desc);
				break;
			}
		}
	}

	for (size_t i = 0; i < Num_Parse_ship_ai_flags; ++i) {
		const auto& flagDef = Parse_ship_ai_flags[i];
		for (size_t j = 0; j < num_ai_descs; ++j) {
			if (Ai_flag_descriptions[j].flag == flagDef.def) {
				descriptions.emplace_back(flagDef.name, Ai_flag_descriptions[j].flag_desc);
				break;
			}
		}
	}

	for (size_t i = 0; i < Num_Parse_ship_object_flags; ++i) {
		const auto& flagDef = Parse_ship_object_flags[i];
		for (size_t j = 0; j < num_obj_descs; ++j) {
			if (Object_flag_descriptions[j].flag == flagDef.def) {
				descriptions.emplace_back(flagDef.name, Object_flag_descriptions[j].flag_desc);
				break;
			}
		}
	}

	return descriptions;
}

void ShipFlagsDialogModel::initializeData()
{
	object* objp;
	ship* shipp;
	int first;

	first = 1;

	bool all_player_ships = false;
	bool any_marked = false;
	for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		if (((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) && objp->flags[Object::Object_Flags::Marked]) {
			if (!any_marked) {
				all_player_ships = true;
				any_marked = true;
			}
			if (objp->type != OBJ_START) {
				all_player_ships = false;
				break;
			}
		}
	}

	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				shipp = &Ships[objp->instance];
				if (first) {
					_kamikazeDamage = Ai_info[shipp->ai_index].kamikaze_damage;
					_escortPriority = shipp->escort_priority;
					_destroyTime = shipp->final_death_time;
					for (size_t i = 0; i < Num_Parse_ship_flags; i++) {
						auto flagDef = Parse_ship_flags[i];

						// Skip flags that have checkboxes elsewhere in the dialog
						if (flagDef.def == Ship::Ship_Flags::No_arrival_warp ||
							flagDef.def == Ship::Ship_Flags::No_departure_warp ||
							flagDef.def == Ship::Ship_Flags::Same_arrival_warp_when_docked ||
							flagDef.def == Ship::Ship_Flags::Same_departure_warp_when_docked ||
							flagDef.def == Ship::Ship_Flags::Primaries_locked ||
							flagDef.def == Ship::Ship_Flags::Secondaries_locked ||
							flagDef.def == Ship::Ship_Flags::Ship_locked ||
							flagDef.def == Ship::Ship_Flags::Weapons_locked ||
							flagDef.def == Ship::Ship_Flags::Afterburner_locked ||
							flagDef.def == Ship::Ship_Flags::Lock_all_turrets_initially ||
							flagDef.def == Ship::Ship_Flags::Force_shields_on) {
							continue;
						}
						if (all_player_ships) {
							bool hidden = false;
							for (const auto& hf : player_start_hidden_flags) {
								if (flagDef.def == hf) { hidden = true; break; }
							}
							if (hidden) continue;
						}
						bool checked = shipp->flags[flagDef.def];
						_flags.emplace_back(flagDef.name, checked ? Qt::Checked : Qt::Unchecked);
					}
					for (size_t i = 0; i < Num_Parse_ship_ai_flags; i++) {
						auto flagDef = Parse_ship_ai_flags[i];
						bool checked = Ai_info[shipp->ai_index].ai_flags[flagDef.def];
						_flags.emplace_back(flagDef.name, checked ? Qt::Checked : Qt::Unchecked);
					}
					for (size_t i = 0; i < Num_Parse_ship_object_flags; i++) {
						auto flagDef = Parse_ship_object_flags[i];
						bool checked;
						if (flagDef.def == Object::Object_Flags::Collides) {
							checked = !objp->flags[flagDef.def];
						} else {
							checked = objp->flags[flagDef.def];
						}
						_flags.emplace_back(flagDef.name, checked ? Qt::Checked : Qt::Unchecked);
					}
					first = 0;
				} else {
					for (size_t i = 0; i < Num_Parse_ship_flags; i++) {
						auto flagDef = Parse_ship_flags[i];
						// Skip flags that have checkboxes elsewhere in the dialog
						if (flagDef.def == Ship::Ship_Flags::No_arrival_warp ||
							flagDef.def == Ship::Ship_Flags::No_departure_warp ||
							flagDef.def == Ship::Ship_Flags::Same_arrival_warp_when_docked ||
							flagDef.def == Ship::Ship_Flags::Same_departure_warp_when_docked ||
							flagDef.def == Ship::Ship_Flags::Primaries_locked ||
							flagDef.def == Ship::Ship_Flags::Secondaries_locked ||
							flagDef.def == Ship::Ship_Flags::Ship_locked ||
							flagDef.def == Ship::Ship_Flags::Weapons_locked ||
							flagDef.def == Ship::Ship_Flags::Afterburner_locked ||
							flagDef.def == Ship::Ship_Flags::Lock_all_turrets_initially ||
							flagDef.def == Ship::Ship_Flags::Force_shields_on) {
							continue;
						}
						if (all_player_ships) {
							bool hidden = false;
							for (const auto& hf : player_start_hidden_flags) {
								if (flagDef.def == hf) { hidden = true; break; }
							}
							if (hidden) continue;
						}
						bool checked = shipp->flags[flagDef.def];
						getFlag(flagDef.name)->second = tristateSet(checked, getFlag(flagDef.name)->second);
					}
					for (size_t i = 0; i < Num_Parse_ship_ai_flags; i++) {
						auto flagDef = Parse_ship_ai_flags[i];
						bool checked = Ai_info[shipp->ai_index].ai_flags[flagDef.def];
						getFlag(flagDef.name)->second = tristateSet(checked, getFlag(flagDef.name)->second);
					}
					for (size_t i = 0; i < Num_Parse_ship_object_flags; i++) {
						auto flagDef = Parse_ship_object_flags[i];
						// Skip flags that have checkboxes elsewhere in the dialog
						if (flagDef.def == Object::Object_Flags::No_shields) {
							continue;
						}
						bool checked;
						if (flagDef.def == Object::Object_Flags::Collides) {
							checked = !objp->flags[flagDef.def];
						} else {
							checked = objp->flags[flagDef.def];
						}
						getFlag(flagDef.name)->second = tristateSet(checked, getFlag(flagDef.name)->second);
					}
				}
			}
		}

		objp = GET_NEXT(objp);
	}
	_modified = false;
	modelChanged();
	_modified = false;
}

} // namespace fso::fred::dialogs
