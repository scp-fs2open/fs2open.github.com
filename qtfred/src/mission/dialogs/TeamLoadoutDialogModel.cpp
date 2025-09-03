#include "TeamLoadoutDialogModel.h"
#include "FredApplication.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "mission/Editor.h"
#include "ship/ship.cpp"
#include "weapon/weapon.h"

namespace fso::fred::dialogs {

// for variable-enabled weapons and ships, these are the defaults.
constexpr int ShipVarDefault = 4;
constexpr int WeaponVarDefault = 40;

TeamLoadoutDialogModel::TeamLoadoutDialogModel(QObject* parent, EditorViewport* viewport) 
		: AbstractDialogModel(parent, viewport)
{
		initializeData();
}

bool TeamLoadoutDialogModel::apply()
{

	for (int t = 0; t < Num_teams; ++t) {
		auto& in = _teams[t];
		auto& out = Team_data[t];

		// reset per team outputs
		out.num_ship_choices = 0;
		out.num_weapon_choices = 0;
		for (int k = 0; k < MAX_WEAPON_TYPES; ++k)
			out.weapon_required[k] = false;

		// Ships
		int s = 0;

		// var ships first
		for (const auto& it : in.varShips)
			if (it.enabled && it.infoIndex >= 0) {
				out.ship_list[s] = -1;
				// enabling var name
				strcpy_s(out.ship_list_variables[s], Sexp_variables[it.infoIndex].variable_name);
				// count: number-var or literal
				if (it.varCountIndex >= 0) {
					strcpy_s(out.ship_count_variables[s], Sexp_variables[it.varCountIndex].variable_name);
					out.ship_count[s] = 0;
				} else {
					out.ship_count_variables[s][0] = '\0';
					out.ship_count[s] = it.extraAllocated;
				}
				++s;
			}

		// static ships
		auto presentShip = [](const LoadoutItem& it) {
			return it.enabled && (it.extraAllocated > 0 || it.varCountIndex != -1);
		};
		for (const auto& it : in.ships)
			if (presentShip(it)) {
				out.ship_list[s] = it.infoIndex;
				out.ship_list_variables[s][0] = '\0';
				if (it.varCountIndex >= 0) {
					strcpy_s(out.ship_count_variables[s], Sexp_variables[it.varCountIndex].variable_name);
					out.ship_count[s] = 0;
				} else {
					out.ship_count_variables[s][0] = '\0';
					out.ship_count[s] = it.extraAllocated;
				}
				++s;
			}

		out.num_ship_choices = s;

		// Weapons
		int w = 0;

		// var weapons first
		for (const auto& it : in.varWeapons)
			if (it.enabled && it.infoIndex >= 0) {
				out.weaponry_pool[w] = -1;
				strcpy_s(out.weaponry_pool_variable[w], Sexp_variables[it.infoIndex].variable_name);
				if (it.varCountIndex >= 0) {
					strcpy_s(out.weaponry_amount_variable[w], Sexp_variables[it.varCountIndex].variable_name);
					out.weaponry_count[w] = 0;
				} else {
					out.weaponry_amount_variable[w][0] = '\0';
					out.weaponry_count[w] = it.extraAllocated;
				}
				++w;
			}

		// static weapons
		auto presentWeap = [](const LoadoutItem& it) {
			return it.enabled && (it.extraAllocated > 0 || it.varCountIndex != -1 || it.countInWings > 0);
		};
		for (const auto& it : in.weapons)
			if (presentWeap(it)) {
				out.weaponry_pool[w] = it.infoIndex;
				out.weaponry_pool_variable[w][0] = '\0';

				if (it.varCountIndex >= 0) {
					strcpy_s(out.weaponry_amount_variable[w], Sexp_variables[it.varCountIndex].variable_name);
					out.weaponry_count[w] = 0;
				} else {
					out.weaponry_amount_variable[w][0] = '\0';
					out.weaponry_count[w] = it.extraAllocated;
				}
				++w;
			}

		out.num_weapon_choices = w;

		// required weapons
		for (const auto& it : in.weapons)
			if (presentWeap(it) && it.required && it.infoIndex >= 0 && it.infoIndex < MAX_WEAPON_TYPES) {
				out.weapon_required[it.infoIndex] = true;
			}

		out.do_not_validate = in.skipValidation;
	}

	return true;
}

void TeamLoadoutDialogModel::reject()
{
	// just clear out the info we're not going to end up using.
	_shipList.clear();
	_weaponList.clear();
	_shipVarList.clear();
	_weaponVarList.clear();
	_numberVarList.clear();
	_requiredWeaponsList.clear();
	_teams.clear();

} // let him go Harry, let him go

void TeamLoadoutDialogModel::initializeData()
{
	
	// first, build the team list same as other dialogs
	_team_list.clear();
	for (auto& team : Mission_event_teams_tvt) {
		_team_list.emplace_back(team.first, team.second);
	}

	// need to build list for count variables.
	for (auto& sexp : Sexp_variables) {
		if ((sexp.type & SEXP_VARIABLE_SET) && (sexp.type & SEXP_VARIABLE_NUMBER)) {
			_numberVarList.push_back(sexp.variable_name);
		}
	}

	// and a list of string variables for the enablers.
	for (auto& sexp : Sexp_variables) {
		if ((sexp.type & SEXP_VARIABLE_SET) && (sexp.type & SEXP_VARIABLE_STRING)) {
			_stringVarList.push_back(sexp.variable_name);
		}
	}

	TeamLoadout defaultEntry;
	// make sure we have the correct number of teams.
	for (int i = 0; i < Num_teams; i++) {
		_teams.push_back(defaultEntry);
	}

	// this is basically raw data, so we have to make sure to calculate the indices correctly.
	SCP_vector<int> usage = _editor->getStartingWingLoadoutUseCounts();
	
	Assertion(usage.size() == (MAX_SHIP_CLASSES + MAX_WEAPON_TYPES) * MAX_TVT_TEAMS, "Starting wing loadout usage is unexpected size!");

	for (int i = 0; i < Num_teams; i++) {
		auto& team = _teams[i];

		// First we get the ship pool
		for (int j = 0; j < MAX_SHIP_CLASSES; j++) {
			const auto& ship = Ship_info[j];
			
			if (ship.flags[Ship::Info_Flags::Player_ship]) {
				int countInWings = usage.at((MAX_SHIP_CLASSES * i) + j);

				LoadoutItem item(
					j,                // ship class index
					countInWings > 0, // if it's in a starting wing, it's enabled
					false,            // ships can't be required
					false,            // not from a string variable
					countInWings,
					0,                // extra allocated starts at 0 ???
					-1,               // no variable controlling count
					SCP_string(ship.name)
				);

				// since we're here, add the countInWings to the total
				team.startingShipCount += countInWings;

				team.ships.push_back(item);
			}
		}

		// then the weapon pool
		for (int j = 0; j < MAX_WEAPON_TYPES; j++) {
			const auto& weapon = Weapon_info[j];

			if (weapon.wi_flags[Weapon::Info_Flags::Player_allowed]) {
				int countInWings = usage.at((MAX_SHIP_CLASSES * MAX_TVT_TEAMS) + (MAX_WEAPON_TYPES * i) + j);

				LoadoutItem item(
					j,                 // weapon index
					countInWings > 0,  // if it's in a starting wing, it's enabled
					false,             // required status defaults to false
					false,             // not from a string variable
					countInWings,
					0,                 // extra allocated starts at 0
					-1,                // no variable controlling count
					SCP_string(weapon.name)
				);

				team.weapons.push_back(item);
			}
		}

		// now merge in the data from Team_data
		const auto& teamData = Team_data[i];

		// first the ships
		for (int j = 0; j < teamData.num_ship_choices; j++) {
			// if it has an enabling variable, add it to the correct vector.
			if (strlen(teamData.ship_list_variables[j])) {

				LoadoutItem varItem(
					get_index_sexp_variable_name(teamData.ship_list_variables[j]), // variable index
					true,
					false,
					true,
					0, // 0 until proven otherwise in-game.
					teamData.ship_count[j],
					(strlen(teamData.ship_count_variables[j])) ? get_index_sexp_variable_name(teamData.ship_count_variables[j]) : -1,
					SCP_string(teamData.ship_list_variables[j])
				);

				team.varShips.emplace_back(varItem);

			// if it doesn't, enable the matching item.
			} else {
				for (auto& item : team.ships) {
					if (teamData.ship_list[j] == item.infoIndex) {
						item.enabled = true;
						item.extraAllocated = teamData.ship_count[j];
						if (strlen(teamData.ship_count_variables[j])) {
							item.varCountIndex = get_index_sexp_variable_name(teamData.ship_count_variables[j]);
						} else {
							item.varCountIndex = -1;
						}
						if (Ship_info[item.infoIndex].num_primary_banks > team.largestPrimaryBankCount) {
							team.largestPrimaryBankCount = Ship_info[item.infoIndex].num_primary_banks;
						}
						int capacity = 0;
						for (int k = 0; k < Ship_info[item.infoIndex].num_secondary_banks; k++) {
							capacity += Ship_info[item.infoIndex].secondary_bank_ammo_capacity[k];
						}
						if (capacity > team.largestSecondaryCapacity) {
							team.largestSecondaryCapacity = capacity;
						}
						break;
					}
				}
			}
		}

		// then the weapons
		for (int j = 0; j < teamData.num_weapon_choices; j++) {
			// if it has an enabling variable, add it to the correct vector.
			if (strlen(teamData.weaponry_pool_variable[j])) {

				LoadoutItem varItem(
					get_index_sexp_variable_name(teamData.weaponry_pool_variable[j]), // variable index
					true,
					false, // was teamData.weapon_required[j]... I don't think variables can be required
					true,
					0, // 0 until proven otherwise in-game.
					teamData.weaponry_count[j],
					(strlen(teamData.weaponry_amount_variable[j])) ? get_index_sexp_variable_name(teamData.weaponry_amount_variable[j]) : -1,
					SCP_string(teamData.weaponry_pool_variable[j])
				);

				// it's impossible for this type to tell if it's secondary or its cargo size, so this default allows for a good number.
				if (varItem.extraAllocated == 0) {
					varItem.extraAllocated = WeaponVarDefault * team.startingShipCount;
				}

				team.varWeapons.emplace_back(varItem);
			// if it doesn't, enable the matching item.
			} else {
				for (auto& item : team.weapons) {
					if (teamData.weaponry_pool[j] == item.infoIndex) {
						item.enabled = true;
						item.required = teamData.weapon_required[item.infoIndex];
						item.extraAllocated = teamData.weaponry_count[j];
						if (strlen(teamData.weaponry_amount_variable[j])) {
							item.varCountIndex = get_index_sexp_variable_name(teamData.weaponry_amount_variable[j]);
						} else {
							item.varCountIndex = -1;
						}
						break;
					}
				}
			}
		}
	}
}

int TeamLoadoutDialogModel::getCurrentTeam()
{
	return _currentTeam;
}

const SCP_vector<LoadoutItem>& TeamLoadoutDialogModel::getShipList() const
{
	return _teams[_currentTeam].ships;
}

const SCP_vector<LoadoutItem>& TeamLoadoutDialogModel::getWeaponList() const
{
	return _teams[_currentTeam].weapons;
}

const SCP_vector<SCP_string>& TeamLoadoutDialogModel::getRequiredWeapons() const
{
	return _requiredWeaponsList;
}

const SCP_vector<LoadoutItem>& TeamLoadoutDialogModel::getShipEnablerVariables() const
{
	return _teams[_currentTeam].varShips;
}

const SCP_vector<LoadoutItem>& TeamLoadoutDialogModel::getWeaponEnablerVariables() const
{
	return _teams[_currentTeam].varWeapons;
}

SCP_string TeamLoadoutDialogModel::createItemString(bool ship, bool variable, int itemIndex, const char* variableIn)
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

void TeamLoadoutDialogModel::copyToOtherTeam()
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

void TeamLoadoutDialogModel::switchTeam(int teamIn) 
{
	if (!SCP_vector_inbounds(_teams, teamIn))
		return;

	_currentTeam = teamIn;

	buildCurrentLists();
}

void TeamLoadoutDialogModel::buildCurrentLists()
{
	// we're rebuilding the whole list, so we have to clear the text lists out.
	_shipList.clear();
	_weaponList.clear();
	_shipVarList.clear();
	_weaponVarList.clear();
	_requiredWeaponsList.clear();

	int index = 0;
	for (auto& item : _teams[_currentTeam].ships) {
		_shipList.emplace_back(item);
		index++;
	}

	index = 0;

	for (auto& item : _teams[_currentTeam].weapons) {
		_weaponList.emplace_back(item);
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

			//_shipVarList.emplace_back(name, enabled);

			enabled = false;
			SCP_string name2 = Sexp_variables[x].variable_name;

			for (int y = 0; y < static_cast<int>(_teams[_currentTeam].varWeapons.size()); ++y) {
				if (_teams[_currentTeam].varWeapons[y].name == Sexp_variables[x].variable_name) {
					enabled = _teams[_currentTeam].varWeapons[y].enabled;
					name2 = createItemString(false, true, y, name2.c_str());
					break;
				}
			}

			//_weaponVarList.emplace_back(name2, enabled);
		}
	}
}

SCP_string TeamLoadoutDialogModel::getCountVarShips(SCP_vector<SCP_string> namesIn) 
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

SCP_string TeamLoadoutDialogModel::getCountVarWeapons(SCP_vector<SCP_string> namesIn)
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

SCP_string TeamLoadoutDialogModel::getCountVarShipEnabler(SCP_vector<SCP_string> namesIn) 
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

SCP_string TeamLoadoutDialogModel::getCountVarWeaponEnabler(SCP_vector<SCP_string> namesIn) 
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

int TeamLoadoutDialogModel::getExtraAllocatedShips(SCP_vector<SCP_string> namesIn) 
{
	int out = -1;

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

int TeamLoadoutDialogModel::getExtraAllocatedWeapons(SCP_vector<SCP_string> namesIn) 
{
	int out = -1;

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

int TeamLoadoutDialogModel::getExtraAllocatedShipEnabler(SCP_vector<SCP_string> namesIn) 
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

int TeamLoadoutDialogModel::getExtraAllocatedWeaponEnabler(SCP_vector<SCP_string> namesIn) 
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

SCP_vector<SCP_string> TeamLoadoutDialogModel::getNumberVarList() 
{
	return _numberVarList;
}

const SCP_vector<std::pair<SCP_string, int>>& TeamLoadoutDialogModel::getTeamList()
{
	return _team_list;
}

void TeamLoadoutDialogModel::setShipEnabled(const SCP_vector<SCP_string>& list, bool enabled) 
{
	for (const auto& item : list) {
		for (auto& ship : _teams[_currentTeam].ships) {
			if (item == ship.name) {
				// if this is a first time enabling, set it to the default.				
				if (!ship.enabled && enabled && ship.extraAllocated == 0) {
					modify(ship.extraAllocated, 4); // TODO 4 should be a constant somewhere
				} 

				// If this is present in wings, it cannot be disabled, but we *can* remove the extra.
				if (ship.enabled && !enabled && ship.countInWings > 0) {
					modify(ship.extraAllocated, 0);
				} else {
					modify(ship.enabled, enabled);
				}

				break;
			}
		}
	}

	buildCurrentLists();
}


void TeamLoadoutDialogModel::setShipVariableEnabled(const SCP_vector<SCP_string>& list, bool enabled)
{
	for (const auto& item : list) {
		bool found = false;
		for (auto& ship : _teams[_currentTeam].varShips) {
			if (item == ship.name) {
				found = true;

				// if this is a first time enabling, set it to the default.
				if (!ship.enabled && enabled && ship.extraAllocated == 0) {
					modify(ship.extraAllocated, ShipVarDefault);
				}

				modify(ship.enabled, enabled);
				break;
			}
		}

		if (!found && enabled) {
			_teams[_currentTeam].varShips.emplace_back(get_index_sexp_variable_name(item.c_str()),
				true,
				false, // not required until specified
				true,
				0, // there cannot be any because this is var enabled.
				ShipVarDefault,
				-1, // no var for count until one can be selected.
				item);
			set_modified();
		}
	}

	buildCurrentLists();
}


void TeamLoadoutDialogModel::setWeaponEnabled(const SCP_vector<SCP_string>& list, bool enabled) 
{
	for (const auto& item : list) {
		for (auto& weapon : _teams[_currentTeam].weapons) {
			if (item == weapon.name) {
				// if this is a first time enabling, set it to the default.
				if (!weapon.enabled && enabled && weapon.extraAllocated == 0) {
					modify(weapon.extraAllocated, 8); // TODO 8 should be a constant somewhere
				}

				// If this is present in wings, it cannot be disabled, but we *can* remove the extra.
				if (weapon.enabled && !enabled && weapon.countInWings > 0) {
					modify(weapon.extraAllocated, 0);
				} else {
					modify(weapon.enabled, enabled);
				}

				break;
			}
		}
	}

	buildCurrentLists();
}

void TeamLoadoutDialogModel::setWeaponVariableEnabled(const SCP_vector<SCP_string>& list, bool enabled)
{
	for (const auto& item : list) {
		bool found = false;
		for (auto& weapon : _teams[_currentTeam].varWeapons) {
			if (item == weapon.name) {
				found = true;

				// if this is a first time enabling, set it to the default.
				if (!weapon.enabled && enabled && weapon.extraAllocated == 0) {
					modify(weapon.extraAllocated, WeaponVarDefault);
				}

				modify(weapon.enabled, enabled);
				break;
			}
		}

		if (!found && enabled) {
			_teams[_currentTeam].varWeapons.emplace_back(
				get_index_sexp_variable_name(item.c_str()),
				true,
				false, // not required until specified
				true,
				0, // there cannot be any because this is var enabled.
				WeaponVarDefault,
				-1, // no var for count until one can be selected.
				item);
			set_modified();
		}
	}

	buildCurrentLists();
}


void TeamLoadoutDialogModel::setExtraAllocatedShipCount(const SCP_vector<SCP_string>& list, const int count) 
{
	for (const auto& item : list) {
		for (auto& ship : _teams[_currentTeam].ships) {
			if (item == ship.name) {
				modify(ship.extraAllocated, count);
				break;
			}
		}
	}

	buildCurrentLists();
}

void TeamLoadoutDialogModel::setExtraAllocatedForShipVariablesCount(const SCP_vector<SCP_string>& list, const int count)
{
	for (const auto& item : list) {
		for (auto& ship : _teams[_currentTeam].varShips) {
			if (item == ship.name) {
				modify(ship.extraAllocated, count);
				break;
			}
		}
	}

	buildCurrentLists();
}

void TeamLoadoutDialogModel::setExtraAllocatedWeaponCount(const SCP_vector<SCP_string>& list, const int count)
{
	for (const auto& item : list) {
		for (auto& weapon : _teams[_currentTeam].weapons) {
			if (item == weapon.name) {
				modify(weapon.extraAllocated, count);
				break;
			}
		}
	}

	buildCurrentLists();
}

void TeamLoadoutDialogModel::setExtraAllocatedForWeaponVariablesCount(const SCP_vector<SCP_string>& list, const int count)
{
	for (const auto& item : list) {
		for (auto& weapon : _teams[_currentTeam].varWeapons) {
			if (item == weapon.name) {
				modify(weapon.extraAllocated, count);
				break;
			}
		}
	}

	buildCurrentLists();
}

void TeamLoadoutDialogModel::setExtraAllocatedViaVariable(const SCP_vector<SCP_string>& list, const SCP_string& variable, const bool isShip, const bool variableMode)
{
	int index = -1;

	if (variable != "<none>" && variable != "") {
		index = get_index_sexp_variable_name(variable.c_str());
	}

	for (const auto& item : list) {
		if (isShip && !variableMode) {
			for (auto& ship : _teams[_currentTeam].ships) {
				if (ship.name == item) {
					modify(ship.varCountIndex, index);
					break;
				}
			}
			
		} else if (isShip && variableMode) {
			for (auto& shipVar : _teams[_currentTeam].varShips) {
				if (shipVar.name == item) {
					modify(shipVar.varCountIndex, index);
					break;
				}
			}

		} else if (!isShip && !variableMode) {
			for (auto& weapon : _teams[_currentTeam].weapons) {
				if (weapon.name == item) {
					modify(weapon.varCountIndex, index);
					break;
				}
			}

		} else {
			for (auto& weaponVar : _teams[_currentTeam].varWeapons) {
				if (weaponVar.name == item) {
					modify(weaponVar.varCountIndex, index);
					break;
				}
			}
		}
	}

	buildCurrentLists();
}

void TeamLoadoutDialogModel::setRequiredWeapon(const SCP_vector<SCP_string>& list, const bool required)
{
	for (const auto& item : list) {
		for (auto& weapon : _teams[_currentTeam].weapons) {
			if (item == weapon.name) {
				modify(weapon.required, required);
				break;
			}
		}
	}

	buildCurrentLists();
}

bool TeamLoadoutDialogModel::getSkipValidation() {
	return _teams[_currentTeam].skipValidation;
}

void TeamLoadoutDialogModel::setSkipValidation(const bool skipIt) {
	// this is designed to be a global control, so turn this off in TvT, until we hear from someone otherwise.
	for (auto& team : _teams) {
		team.skipValidation = skipIt;
		modify(team.skipValidation, skipIt);
	}
}

} // namespace fso::fred::dialogs