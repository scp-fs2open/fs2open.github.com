#include "LoadoutEditorDialogModel.h"
#include "FredApplication.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "mission/Editor.h"
#include "ship/ship.cpp"
#include "weapon/weapon.h"
#include <QMessageBox>

namespace fso {
namespace fred {
namespace dialogs {

// for variable-enabled weapons and ships, these are the defaults.
constexpr int ShipVarDefault = 4;
constexpr int WeaponVarDefault = 40;

LoadoutDialogModel::LoadoutDialogModel(QObject* parent, EditorViewport* viewport) 
		: AbstractDialogModel(parent, viewport)
{
		initializeData();
}

void LoadoutDialogModel::initializeData()
{
	_currentTeam = 0;

	_spinBoxUpdateRequired = true;

	TeamLoadout defaultEntry;
	// make sure we have the correct number of teams.
	for (int i = 0; i < Num_teams; i++) {
		_teams.push_back(defaultEntry);
	}

	// this is basically raw data, so we have to make sure to calculate the indices correctly.
	SCP_vector<int> usage = _editor->getStartingWingLoadoutUseCounts();
	
	Assert(usage.size() == MAX_SHIP_CLASSES * MAX_TVT_TEAMS * 2);

	int currentTeam = 0;
	int index = 0;

	for (auto& ship : Ship_info){
		if (ship.flags[Ship::Info_Flags::Player_ship]) {
			for (auto& team : _teams) {

				// even though it gets created here, it's not complete because the enabled entries are in Team_data
				// also not required until we read that data
				team.ships.emplace_back(
					index,
					false,
					false,
					usage.at((MAX_SHIP_CLASSES * currentTeam) + index),
					0,
					-1,
					SCP_string(ship.name));

				// make sure that starting ships are enabled.
				if (team.ships.back().countInWings > 0) {
					team.ships.back().enabled = true;
				}

				// since we're here, add the countInWings to the total
				_teams[currentTeam].startingShipCount += team.ships.back().countInWings;
				
				currentTeam++;
			}
			
			currentTeam = 0;
		}

		index++;
	}

	index = 0;

	for (auto& weapon : Weapon_info){
		if (weapon.wi_flags[Weapon::Info_Flags::Player_allowed]) {
			for (auto& team : _teams) {
				// even though it gets created here, it's not complete because the enabled entries are in Team_data
				team.weapons.emplace_back(
					index,
					false,
					false,
					usage[(MAX_SHIP_CLASSES * MAX_TVT_TEAMS) + (MAX_SHIP_CLASSES * currentTeam) + index],
					0,
					-1,
					SCP_string(weapon.name));

				// make sure that weapons in starting wing slots are enabled.
				if (team.weapons.back().countInWings > 0) {
					team.weapons.back().enabled = true;
				}

				currentTeam++;
			}
			currentTeam = 0;
		}

		index++;
	}

	currentTeam = 0;

	for (auto& team : Team_data) {		

		for (index = 0; index < team.num_ship_choices; index++){
			// if it has an enabling variable, add it to the correct vector.
			if (strlen(team.ship_list_variables[index])) {
				_teams[currentTeam].varShips.emplace_back(
					get_index_sexp_variable_name(team.ship_list_variables[index]),
					true,
					false,
					0, // 0 until proven otherwise in-game.
					team.ship_count[index],
					(strlen(team.ship_count_variables[index])) ? get_index_sexp_variable_name(team.ship_list_variables[index]) : -1,
					SCP_string(team.ship_list_variables[index])
				);

			} // if it doesn't, enable the matching item.
			else {
				for (auto& item : _teams[currentTeam].ships) {
					if (team.ship_list[index] == item.infoIndex) {
						item.enabled = true;
						item.extraAllocated = team.ship_count[index];

						if (strlen(team.ship_count_variables[index])) {
							item.varCountIndex = get_index_sexp_variable_name(team.ship_count_variables[index]);
						}
						else {
							item.varCountIndex = -1;
						}

						if (Ship_info[item.infoIndex].num_primary_banks > _teams[currentTeam].largestPrimaryBankCount) {
							_teams[currentTeam].largestPrimaryBankCount = Ship_info[item.infoIndex].num_primary_banks;
						}

						int capacity = 0;

						for (int i = 0; i < Ship_info[item.infoIndex].num_secondary_banks; i++){
							capacity += Ship_info[item.infoIndex].secondary_bank_ammo_capacity[i];
						}
							
						if (capacity > _teams[currentTeam].largestSecondaryCapacity) {
							_teams[currentTeam].largestSecondaryCapacity = capacity;
						}

						break;
					}
				}
			}
		}

		for (index = 0; index < team.num_weapon_choices; index++){
			// if it has an enabling variable, add it to the correct vector.
			if (strlen(team.weaponry_pool_variable[index])) {
				_teams[currentTeam].varWeapons.emplace_back(
					get_index_sexp_variable_name(team.weaponry_pool_variable[index]),
					true,
					team.weapon_required[index],
					0, // 0 until proven otherwise in-game.
					team.weaponry_count[index],
					(strlen(team.weaponry_amount_variable[index])) ? get_index_sexp_variable_name(team.weaponry_pool_variable[index]) : -1,
					SCP_string(team.weaponry_pool_variable[index])
				);

				// it's impossible for this type to tell if it's secondary or its cargo size, so this default allows for a good number.
				if (_teams[currentTeam].varWeapons.back().extraAllocated == 0) {
					_teams[currentTeam].varWeapons.back().extraAllocated = WeaponVarDefault * _teams[currentTeam].startingShipCount;
				}

			} // if it doesn't, enable the matching item.
			else {
				for (auto& item : _teams[currentTeam].weapons) {
					if (team.weaponry_pool[index] == item.infoIndex) {
						item.enabled = true;
						item.required = team.weapon_required[index];
						item.extraAllocated = team.weaponry_count[index];

						if (strlen(team.weaponry_amount_variable[index])) {
							item.varCountIndex = get_index_sexp_variable_name(team.weaponry_amount_variable[index]);
						}
						else {
							item.varCountIndex = -1;
						}

						// if none are present set the meaningful default here.
						if (item.extraAllocated == 0 && item.countInWings == 0) {
							if (Weapon_info[item.infoIndex].subtype == WP_LASER) {
								item.extraAllocated = (_teams[currentTeam].startingShipCount * _teams[currentTeam].largestPrimaryBankCount) / 2;
							}
							else {
								Assert(Weapon_info[item.infoIndex].cargo_size > 0);
								item.extraAllocated = (_teams[currentTeam].startingShipCount * _teams[currentTeam].largestSecondaryCapacity) / (2 * Weapon_info[item.infoIndex].cargo_size);
							}
						}

						break;
					}
				}
			}
		}

		currentTeam++;
		if (currentTeam >= static_cast<int>(_teams.size())) {
			break;
		}
	}

	// need to build list for count variables.
	for (auto& sexp : Sexp_variables) {
		if ((sexp.type & SEXP_VARIABLE_SET) && (sexp.type & SEXP_VARIABLE_NUMBER)){	
			_numberVarList.push_back(sexp.variable_name);
		}
	}

	buildCurrentLists();

	_currentTeam = 0;

	_playerEntryDelay = f2fl(Entry_delay_time);

	modelChanged();
}

float LoadoutDialogModel::getPlayerEntryDelay()
{
	return _playerEntryDelay;
}

void LoadoutDialogModel::setPlayerEntryDelay(float delay)
{
	_playerEntryDelay = delay;
	modelChanged();
	set_modified();

}

int LoadoutDialogModel::getCurrentTeam()
{
	return _currentTeam + 1;
}

SCP_vector<std::pair<SCP_string, bool>> LoadoutDialogModel::getShipList()
{
	return _shipList;
}

SCP_vector<std::pair<SCP_string, bool>> LoadoutDialogModel::getWeaponList()
{
	return _weaponList;
}

SCP_vector<SCP_string> LoadoutDialogModel::getRequiredWeapons() 
{
	return _requiredWeaponsList;
}

SCP_vector<std::pair<SCP_string, bool>> LoadoutDialogModel::getShipEnablerVariables()
{
	return _shipVarList;
}

SCP_vector<std::pair<SCP_string, bool>> LoadoutDialogModel::getWeaponEnablerVariables()
{
	buildCurrentLists();
	return _weaponVarList;
}

SCP_string LoadoutDialogModel::createItemString(bool ship, bool variable, int itemIndex, const char* variableIn)
{
	// Using a pointer here seems to lead to memory corruption, so I'll just use a temp Loadout Item
	LoadoutItem item;
	char stringOut[128];

	if (variable) {
		if (ship) {
			item = _teams[_currentTeam].varShips[itemIndex];
			strcpy(stringOut, variableIn);
		} else {
			item = _teams[_currentTeam].varWeapons[itemIndex];
			strcpy(stringOut, variableIn);
		}

	} else {
		if (ship) {
			item = _teams[_currentTeam].ships[itemIndex];
			strcpy(stringOut, Ship_info[item.infoIndex].name);
		} else {
			item = _teams[_currentTeam].weapons[itemIndex];
			strcpy(stringOut, Weapon_info[item.infoIndex].name);
		}
	}

	strcat(stringOut, " ");

	if (!variable) {
		strcat(stringOut, std::to_string(item.countInWings).c_str());
		strcat(stringOut, "/");	
	}

	if (item.varCountIndex < 0) {
		strcat(stringOut, std::to_string(item.extraAllocated).c_str());
	}
	else {
		strcat(stringOut, Sexp_variables[item.varCountIndex].variable_name);
	}

	SCP_string stringOut2 = stringOut;
	return stringOut2;
}

void LoadoutDialogModel::copyToOtherTeam()
{
	int index = 0;

	for (auto& destinationTeam : _teams) {
		if (index == _currentTeam) {
			index++;
			continue;
		}

		destinationTeam.largestPrimaryBankCount = _teams[_currentTeam].largestPrimaryBankCount;
		destinationTeam.largestSecondaryCapacity = _teams[_currentTeam].largestSecondaryCapacity;
		destinationTeam.ships = _teams[_currentTeam].ships;
		destinationTeam.varShips = _teams[_currentTeam].varShips;
		destinationTeam.varWeapons = _teams[_currentTeam].varWeapons;
		destinationTeam.weapons = _teams[_currentTeam].weapons;

		index++;
	}
}

void LoadoutDialogModel::switchTeam(int teamIn) 
{
	if (teamIn < 1 || teamIn > static_cast<int>(_teams.size()))
		return;

	_spinBoxUpdateRequired = true;
	_currentTeam = teamIn - 1;

	buildCurrentLists();

	modelChanged();
}

bool LoadoutDialogModel::apply() {

	int currentTeam = 0;
	int index = 0;

	for (auto& modelTeam : _teams) {
		for (auto& ship : modelTeam.ships) {
			Team_data[currentTeam].ship_count[index] = ship.extraAllocated;

			if (ship.varCountIndex > -1) {
				strcpy_s(Team_data[currentTeam].ship_count_variables[index], Sexp_variables[ship.varCountIndex].variable_name);
			}
			else {
				memset(Team_data[currentTeam].ship_count_variables[index], 0, TOKEN_LENGTH);
			}

			Team_data[currentTeam].ship_list[index] = ship.infoIndex;
			memset(Team_data[currentTeam].ship_list_variables[index], 0, TOKEN_LENGTH);
			index++;
		}

		for (auto& ship : modelTeam.varShips) {
			if (ship.enabled) {

				Team_data[currentTeam].ship_count[index] = ship.extraAllocated;

				if (ship.varCountIndex > -1) {
					strcpy_s(Team_data[currentTeam].ship_count_variables[index],
						Sexp_variables[ship.varCountIndex].variable_name);
				} else {
					memset(Team_data[currentTeam].ship_count_variables[index], 0, TOKEN_LENGTH);
				}

				Team_data[currentTeam].ship_list[index] = ship.infoIndex;
				strcpy_s(Team_data[currentTeam].ship_list_variables[index], ship.name.c_str());
				index++;
			}
		}

		Team_data[currentTeam].num_ship_choices = index;
		index = 0;

		for (auto& weapon : modelTeam.weapons) {
			Team_data[currentTeam].weaponry_count[index] = weapon.extraAllocated;

			if (weapon.varCountIndex > -1) {
				strcpy_s(Team_data[currentTeam].weaponry_amount_variable[index], Sexp_variables[weapon.varCountIndex].variable_name);
			}
			else {
				memset(Team_data[currentTeam].weaponry_amount_variable[index], 0, TOKEN_LENGTH);
			}

			Team_data[currentTeam].weaponry_pool[index] = weapon.infoIndex;
			memset(Team_data[currentTeam].weaponry_pool_variable[index], 0, TOKEN_LENGTH);
			Team_data[currentTeam].weapon_required[index] = weapon.required;
			index++;
		}

		for (auto& weapon : modelTeam.varWeapons) {
			if (weapon.enabled) {

				Team_data[currentTeam].weaponry_count[index] = weapon.extraAllocated;

				if (weapon.varCountIndex > -1) {
					strcpy_s(Team_data[currentTeam].weaponry_amount_variable[index],
						Sexp_variables[weapon.varCountIndex].variable_name);
				} else {
					memset(Team_data[currentTeam].weaponry_amount_variable[index], 0, TOKEN_LENGTH);
				}

				Team_data[currentTeam].weaponry_pool[index] = weapon.infoIndex;
				strcpy_s(Team_data[currentTeam].weaponry_pool_variable[index], weapon.name.c_str());
				index++;
			}
		}


		Team_data[currentTeam].num_weapon_choices = index;
		Team_data[currentTeam].do_not_validate = modelTeam.skipValidation;
	}

	Entry_delay_time = fl2f(_playerEntryDelay);

	
	return true; 

} 

void LoadoutDialogModel::reject() {
	// just clear out the info we're not going to end up using.
	_shipList.clear();
	_weaponList.clear();
	_shipVarList.clear();
	_weaponVarList.clear();
	_numberVarList.clear();
	_requiredWeaponsList.clear();
	_teams.clear();

} // let him go Harry, let him go

void LoadoutDialogModel::buildCurrentLists()
{
	// we're rebuilding the whole list, so we have to clear the text lists out.
	_shipList.clear();
	_weaponList.clear();
	_shipVarList.clear();
	_weaponVarList.clear();
	_requiredWeaponsList.clear();

	int index = 0;
	for (auto& item : _teams[_currentTeam].ships) {
		_shipList.emplace_back(createItemString(true, false, index), item.enabled);
		index++;
	}

	index = 0;

	for (auto& item : _teams[_currentTeam].weapons) {
		_weaponList.emplace_back(createItemString(false, false, index), item.enabled);
		if (item.required) {
			_requiredWeaponsList.push_back(item.name);
		}
		index++;
	}
	
	for (int x = 0; x < MAX_SEXP_VARIABLES; ++x) {
		bool enabled = false;
		SCP_string name = Sexp_variables[x].variable_name;

		if ((Sexp_variables[x].type & SEXP_VARIABLE_SET) && (Sexp_variables[x].type & SEXP_VARIABLE_STRING)) {
			for (int y = 0; y < static_cast<int>(_teams[_currentTeam].varShips.size()); ++y) {
				if (_teams[_currentTeam].varShips[y].name == Sexp_variables[x].variable_name) {
					enabled = _teams[_currentTeam].varShips[y].enabled;
					name = createItemString(true, true, y, name.c_str());
					break;
				}
			}

			_shipVarList.emplace_back(name, enabled);

			enabled = false;
			SCP_string name2 = Sexp_variables[x].variable_name;

			for (int y = 0; y < static_cast<int>(_teams[_currentTeam].varWeapons.size()); ++y) {
				if (_teams[_currentTeam].varWeapons[y].name == Sexp_variables[x].variable_name) {
					enabled = _teams[_currentTeam].varWeapons[y].enabled;
					name2 = createItemString(false, true, y, name2.c_str());
					break;
				}
			}

			_weaponVarList.emplace_back(name2, enabled);
		}
	}
}

SCP_string LoadoutDialogModel::getCountVarShips(SCP_vector<SCP_string> namesIn) 
{
	int tester = -1;
	SCP_string out = "";

	for (auto& name : namesIn) {
		for (auto& currentShip : _teams[_currentTeam].ships) {
			if (currentShip.name == name) {
				if (tester > -1 && currentShip.varCountIndex != tester) {
					return out;
				}
				else {
					tester = currentShip.varCountIndex;
				}
			}
		}
	}

	if (tester > -1) {
		out = SCP_string(Sexp_variables[tester].text);
	} else {
		out = "<none>";
	}

	return out;
}

SCP_string LoadoutDialogModel::getCountVarWeapons(SCP_vector<SCP_string> namesIn)
{
	int tester = -1;
	SCP_string out = "";

	for (auto& name : namesIn) {
		for (auto& currentWep : _teams[_currentTeam].weapons) {
			if (currentWep.name == name) {
				if (tester > -1 && currentWep.varCountIndex != tester) {
					return out;
				}
				else {
					tester = currentWep.varCountIndex;
				}
			}
		}
	}

	if (tester > -1) {
		out = SCP_string(Sexp_variables[tester].text);
	} else {
		out = "<none>";
	}

	return out;
}

SCP_string LoadoutDialogModel::getCountVarShipEnabler(SCP_vector<SCP_string> namesIn) 
{
	int tester = -1;
	SCP_string out = "";

	for (auto& name : namesIn) {
		for (auto& currentShip : _teams[_currentTeam].varShips) {
			if (currentShip.name == name) {
				if (tester > -1 && currentShip.varCountIndex != tester) {
					return out;
				}
				else {
					tester = currentShip.varCountIndex;
				}
			}
		}
	}

	if (tester > -1) {
		out = SCP_string(Sexp_variables[tester].text);
	} else {
		out = "<none>";
	}

	return out;
}

SCP_string LoadoutDialogModel::getCountVarWeaponEnabler(SCP_vector<SCP_string> namesIn) 
{
	int tester = -1;
	SCP_string out = "";

	for (auto& name : namesIn) {
		for (auto& currentWep : _teams[_currentTeam].varWeapons) {
			if (currentWep.name == name) {
				if (tester > -1 && currentWep.varCountIndex != tester) {
					return out;
				}
				else {
					tester = currentWep.varCountIndex;
				}
			}
		}
	}

	if (tester > -1) {
		out = SCP_string(Sexp_variables[tester].text);
	} else {
		out = "<none>";
	}

	return out;

}

int LoadoutDialogModel::getExtraAllocatedShips(SCP_vector<SCP_string> namesIn) 
{
	int out = -1;

	_spinBoxUpdateRequired = false;

	for (auto& name : namesIn) {
		for (auto& currentShip : _teams[_currentTeam].ships) {
			if (currentShip.name == name) {
				// if this ship has a variable establishing count for it, or it doesn't match a previously establish count
				// then we cannot represent all entries with the same amount and should return -1
				if (currentShip.varCountIndex > -1 || (out > -1 && currentShip.extraAllocated != out)) {
					return -1;
				}
				else {
					out = currentShip.extraAllocated;
				}
			}
		}
	}

	return out;
}

int LoadoutDialogModel::getExtraAllocatedWeapons(SCP_vector<SCP_string> namesIn) 
{
	int out = -1;

	_spinBoxUpdateRequired = false;

	for (auto& name : namesIn) {
		for (auto& currentWep : _teams[_currentTeam].weapons) {
			if (currentWep.name == name) {
				if (currentWep.varCountIndex > -1 || (out > -1 && currentWep.extraAllocated != out)) {
					return -1;
				} else {
					out = currentWep.extraAllocated;
				}
			}
		}
	}

	return out;
}

int LoadoutDialogModel::getExtraAllocatedShipEnabler(SCP_vector<SCP_string> namesIn) 
{
	int out = -1;

	for (auto& name : namesIn) {
		for (auto& currentShip : _teams[_currentTeam].varShips) {
			if (currentShip.name == name) {
				if (out > -1 && currentShip.extraAllocated != out) {
					return -1;
				} else {
					out = currentShip.extraAllocated;
				}
			}
		}
	}

	return out;
}

int LoadoutDialogModel::getExtraAllocatedWeaponEnabler(SCP_vector<SCP_string> namesIn) 
{
	int out = -1;

	for (auto& name : namesIn) {
		for (auto& currentWep : _teams[_currentTeam].varWeapons) {
			if (currentWep.name == name) {
				if (out > -1 && currentWep.extraAllocated != out) {
					return -1;
				} else {
					out = currentWep.extraAllocated;
				}
			}
		}
	}

	return out;
}

SCP_vector<SCP_string> LoadoutDialogModel::getNumberVarList() 
{
	return _numberVarList;
}

bool LoadoutDialogModel::spinBoxUpdateRequired() { return _spinBoxUpdateRequired; };


void LoadoutDialogModel::setShipEnabled(const SCP_vector<SCP_string>& list, bool enabled) 
{
	for (const auto& item : list) {
		for (auto& ship : _teams[_currentTeam].ships) {
			if (item == ship.name) {
				// if this is a first time enabling, set it to the default.				
				if (!ship.enabled && enabled && ship.extraAllocated == 0) {
					ship.extraAllocated = 4;
				} 

				// If this is present in wings, it cannot be disabled, but we *can* remove the extra.
				if (ship.enabled && !enabled && ship.countInWings > 0) {
					ship.extraAllocated = 0;
				} else {
					ship.enabled = enabled;
				}

				break;
			}
		}
	}
	set_modified();
	buildCurrentLists();
}


void LoadoutDialogModel::setShipVariableEnabled(const SCP_vector<SCP_string>& list, bool enabled)
{
	for (const auto& item : list) {
		bool found = false;
		for (auto& ship : _teams[_currentTeam].varShips) {
			if (item == ship.name) {
				found = true;

				// if this is a first time enabling, set it to the default.
				if (!ship.enabled && enabled && ship.extraAllocated == 0) {
					ship.extraAllocated = 4;
				}

				ship.enabled = enabled;
				break;
			}
		}

		if (!found && enabled) {
			_teams[_currentTeam].varShips.emplace_back(get_index_sexp_variable_name(item.c_str()),
				true,
				false, // not required until specified
				0, // there cannot be any because this is var enabled.
				ShipVarDefault,
				-1, // no var for count until one can be selected.
				item);
		}
	}
	set_modified();
	buildCurrentLists();
}


void LoadoutDialogModel::setWeaponEnabled(const SCP_vector<SCP_string>& list, bool enabled) 
{
	for (const auto& item : list) {
		for (auto& weapon : _teams[_currentTeam].weapons) {
			if (item == weapon.name) {
				// if this is a first time enabling, set it to the default.
				if (!weapon.enabled && enabled && weapon.extraAllocated == 0) {
					weapon.extraAllocated = 8;
				}

				// If this is present in wings, it cannot be disabled, but we *can* remove the extra.
				if (weapon.enabled && !enabled && weapon.countInWings > 0) {
					weapon.extraAllocated = 0;
				} else {
					weapon.enabled = enabled;		
				}

				break;
			}
		}
	}
	set_modified();
	buildCurrentLists();
}

void LoadoutDialogModel::setWeaponVariableEnabled(const SCP_vector<SCP_string>& list, bool enabled)
{
	for (const auto& item : list) {
		bool found = false;
		for (auto& weapon : _teams[_currentTeam].varWeapons) {
			if (item == weapon.name) {
				found = true;

				// if this is a first time enabling, set it to the default.
				if (!weapon.enabled && enabled && weapon.extraAllocated == 0) {
					weapon.extraAllocated = WeaponVarDefault;
				}

				weapon.enabled = enabled;
				break;
			}
		}

		if (!found && enabled) {
			_teams[_currentTeam].varWeapons.emplace_back(
				get_index_sexp_variable_name(item.c_str()),
				true,
				false, // not required until specified
				0, // there cannot be any because this is var enabled.
				WeaponVarDefault,
				-1, // no var for count until one can be selected.
				item);
		}
	}
	set_modified();
	buildCurrentLists();
}


void LoadoutDialogModel::setExtraAllocatedShipCount(const SCP_vector<SCP_string>& list, const uint count) 
{
	for (const auto& item : list) {
		for (auto& ship : _teams[_currentTeam].ships) {
			if (item == ship.name) {
				ship.extraAllocated = count;
				break;
			}
		}
	}
	set_modified();
	buildCurrentLists();
}

void LoadoutDialogModel::setExtraAllocatedForShipVariablesCount(const SCP_vector<SCP_string>& list, const uint count)
{
	for (const auto& item : list) {
		for (auto& ship : _teams[_currentTeam].varShips) {
			if (item == ship.name) {
				ship.extraAllocated = count;
				break;
			}
		}
	}
	set_modified();
	buildCurrentLists();
}

void LoadoutDialogModel::setExtraAllocatedWeaponCount(const SCP_vector<SCP_string>& list, const uint count)
{
	for (const auto& item : list) {
		for (auto& weapon : _teams[_currentTeam].weapons) {
			if (item == weapon.name) {
				weapon.extraAllocated = count;
				break;
			}
		}
	}
	set_modified();
	buildCurrentLists();
}

void LoadoutDialogModel::setExtraAllocatedForWeaponVariablesCount(const SCP_vector<SCP_string>& list, const uint count)
{
	for (const auto& item : list) {
		for (auto& weapon : _teams[_currentTeam].varWeapons) {
			if (item == weapon.name) {
				weapon.extraAllocated = count;
				break;
			}
		}
	}
	set_modified();
	buildCurrentLists();
}

void LoadoutDialogModel::setExtraAllocatedViaVariable(const SCP_vector<SCP_string>& list, const SCP_string& variable, const bool isShip, const bool variableMode)
{
	int index = -1;

	if (variable != "<none>" && variable != "") {
		index = get_index_sexp_variable_name(variable.c_str());
	}

	for (const auto& item : list) {
		if (isShip && !variableMode) {
			for (auto& ship : _teams[_currentTeam].ships) {
				if (ship.name == item) {
					ship.varCountIndex = index;
					break;
				}
			}
			
		} else if (isShip && variableMode) {
			for (auto& shipVar : _teams[_currentTeam].varShips) {
				if (shipVar.name == item) {
					shipVar.varCountIndex = index;
					break;
				}
			}

		} else if (!isShip && !variableMode) {
			for (auto& weapon : _teams[_currentTeam].weapons) {
				if (weapon.name == item) {
					weapon.varCountIndex = index;
					break;
				}
			}

		} else {
			for (auto& weaponVar : _teams[_currentTeam].varWeapons) {
				if (weaponVar.name == item) {
					weaponVar.varCountIndex = index;
					break;
				}
			}
		}
	}
	set_modified();
	buildCurrentLists();
}

void LoadoutDialogModel::setRequiredWeapon(const SCP_vector<SCP_string>& list, const bool required)
{
	for (const auto& item : list) {
		for (auto& weapon : _teams[_currentTeam].weapons) {
			if (item == weapon.name) {
				weapon.required = required;
				break;
			}
		}
	}
	set_modified();
	buildCurrentLists();
	modelChanged();
}

bool LoadoutDialogModel::getSkipValidation() {
	return _teams[_currentTeam].skipValidation;
}

void LoadoutDialogModel::setSkipValidation(const bool skipIt) {
	// this is designed to be a global control, so turn this off in TvT, until we hear from someone otherwise.
	for (auto& team : _teams) {
		team.skipValidation = skipIt;
		set_modified();
	}
}

}
}
}