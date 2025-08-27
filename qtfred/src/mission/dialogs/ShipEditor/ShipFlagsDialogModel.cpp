//
//

#include "ShipFlagsDialogModel.h"

namespace fso::fred::dialogs {
int ShipFlagsDialogModel::tristate_set(int val, int cur_state)
{
	if (val) {
		if (!cur_state) {
			return CheckState::PartiallyChecked;
		}
	} else {
		if (cur_state) {
			return CheckState::PartiallyChecked;
		}
	}
	if (cur_state == 1) {

		return CheckState::Checked;
	} else {
		return CheckState::Unchecked;
	}
}
std::pair<SCP_string, int>* ShipFlagsDialogModel::getFlag(const SCP_string& flag_name)
{

	for (auto& flag : flags) {
		if (!stricmp(flag_name.c_str(), flag.first.c_str())) {
			return &flag;
		}
	}
	Assertion(false, "Illegal flag name \"[%s]\"", flag_name.c_str());
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
		for (size_t i = 0; i < Num_Parse_ship_flags; ++i) {
			if (!stricmp(name.c_str(), Parse_ship_flags[i].name)) {
				if (Parse_ship_flags[i].def == Ship::Ship_Flags::Reinforcement) {
					_editor->set_reinforcement(shipp->ship_name, checked);
				} else {
					if (checked) {
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
				if (checked) {
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
					if (checked) {
						objp->flags.remove(Parse_ship_object_flags[i].def);
					} else {
						objp->flags.set(Parse_ship_object_flags[i].def);
					}
				} else {
					if (checked) {
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

void ShipFlagsDialogModel::initializeData()
{
	object* objp;
	ship* shipp;
	int first;

	first = 1;

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
						bool checked = shipp->flags[flagDef.def];
						flags.emplace_back(flagDef.name, checked);
					}
					for (size_t i = 0; i < Num_Parse_ship_ai_flags; i++) {
						auto flagDef = Parse_ship_ai_flags[i];
						bool checked = Ai_info[shipp->ai_index].ai_flags[flagDef.def];
						flags.emplace_back(flagDef.name, checked);
					}
					for (size_t i = 0; i < Num_Parse_ship_object_flags; i++) {
						auto flagDef = Parse_ship_object_flags[i];
						bool checked;
						if (flagDef.def == Object::Object_Flags::Collides) {
							checked = !objp->flags[flagDef.def];
						} else {
							checked = objp->flags[flagDef.def];
						}
						flags.emplace_back(flagDef.name, checked);
					}
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
}
} // namespace fso::fred::dialogs
