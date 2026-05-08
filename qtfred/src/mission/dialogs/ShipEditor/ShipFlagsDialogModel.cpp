//
//

#include "ShipFlagsDialogModel.h"

#include <object/object.h>

namespace fso::fred::dialogs {

// Flags that are not applicable to player start ships and should be hidden/skipped for them.
// Add entries here to extend the list.
static const Ship::Ship_Flags player_start_hidden_flags[] = {
	Ship::Ship_Flags::Reinforcement,
	Ship::Ship_Flags::Kill_before_mission,
};
int ShipFlagsDialogModel::tristate_set(int val, int cur_state)
{
	// cur_state uses Qt::CheckState encoding (0=Unchecked, 1=PartiallyChecked, 2=Checked)
	if (cur_state == Qt::PartiallyChecked)
		return Qt::PartiallyChecked;
	bool cur_bool = (cur_state == Qt::Checked);
	if (static_cast<bool>(val) != cur_bool)
		return Qt::PartiallyChecked;
	return cur_state;
}
std::pair<SCP_string, int>* ShipFlagsDialogModel::getFlag(const SCP_string& flag_name)
{

	for (auto& flag : flags) {
		if (!stricmp(flag_name.c_str(), flag.first.c_str())) {
			return &flag;
		}
	}
	// Only assert if the name isn't a known flag at all; it may have been legitimately filtered out for ship starts
	bool known = false;
	for (size_t i = 0; i < Num_Parse_ship_flags && !known; ++i)
		known = !stricmp(flag_name.c_str(), Parse_ship_flags[i].name);
	for (size_t i = 0; i < Num_Parse_ship_ai_flags && !known; ++i)
		known = !stricmp(flag_name.c_str(), Parse_ship_ai_flags[i].name);
	for (size_t i = 0; i < Num_Parse_ship_object_flags && !known; ++i)
		known = !stricmp(flag_name.c_str(), Parse_ship_object_flags[i].name);
	Assertion(known, "Illegal flag name \"%s\"", flag_name.c_str());
	return nullptr;
}

void ShipFlagsDialogModel::setFlag(const SCP_string& flag_name, int value)
{
	for (auto& flag : flags) {
		if (!stricmp(flag_name.c_str(), flag.first.c_str())) {
			flag.second = value;
			set_modified();
		}
	}
}

void ShipFlagsDialogModel::setDestroyTime(int value)
{
	modify(destroytime, value);
}

int ShipFlagsDialogModel::getDestroyTime() const
{
	return destroytime;
}

void ShipFlagsDialogModel::setEscortPriority(int value)
{
	modify(escortp, value);
}

int ShipFlagsDialogModel::getEscortPriority() const
{
	return escortp;
}

void ShipFlagsDialogModel::setKamikazeDamage(int value)
{
	modify(kamikazed, value);
}

int ShipFlagsDialogModel::getKamikazeDamage() const
{
	return kamikazed;
}

void ShipFlagsDialogModel::update_ship(const int shipnum)
{
	ship* shipp = &Ships[shipnum];
	object* objp = &Objects[shipp->objnum];
	for (const auto& [name, checked] : flags) {
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
	Ai_info[shipp->ai_index].kamikaze_damage = kamikazed;
	shipp->escort_priority = escortp;
	shipp->final_death_time = destroytime;
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
				update_ship(objp->instance);
			}
		}
		objp = GET_NEXT(objp);
	}

	return true;
}

void ShipFlagsDialogModel::reject() {}

const SCP_vector<std::pair<SCP_string, int>>& ShipFlagsDialogModel::getFlagsList()
{
	return flags;
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
					kamikazed = Ai_info[shipp->ai_index].kamikaze_damage;
					escortp = shipp->escort_priority;
					destroytime = shipp->final_death_time;
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
						flags.emplace_back(flagDef.name, checked ? Qt::Checked : Qt::Unchecked);
					}
					for (size_t i = 0; i < Num_Parse_ship_ai_flags; i++) {
						auto flagDef = Parse_ship_ai_flags[i];
						bool checked = Ai_info[shipp->ai_index].ai_flags[flagDef.def];
						flags.emplace_back(flagDef.name, checked ? Qt::Checked : Qt::Unchecked);
					}
					for (size_t i = 0; i < Num_Parse_ship_object_flags; i++) {
						auto flagDef = Parse_ship_object_flags[i];
						bool checked;
						if (flagDef.def == Object::Object_Flags::Collides) {
							checked = !objp->flags[flagDef.def];
						} else {
							checked = objp->flags[flagDef.def];
						}
						flags.emplace_back(flagDef.name, checked ? Qt::Checked : Qt::Unchecked);
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
						getFlag(flagDef.name)->second = tristate_set(checked, getFlag(flagDef.name)->second);
					}
					for (size_t i = 0; i < Num_Parse_ship_ai_flags; i++) {
						auto flagDef = Parse_ship_ai_flags[i];
						bool checked = Ai_info[shipp->ai_index].ai_flags[flagDef.def];
						getFlag(flagDef.name)->second = tristate_set(checked, getFlag(flagDef.name)->second);
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
						getFlag(flagDef.name)->second = tristate_set(checked, getFlag(flagDef.name)->second);
					}
				}
			}
		}

		objp = GET_NEXT(objp);
	}
	modelChanged();
	_modified = false;
}
} // namespace fso::fred::dialogs
