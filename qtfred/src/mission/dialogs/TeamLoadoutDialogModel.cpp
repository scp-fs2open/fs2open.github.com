#include "TeamLoadoutDialogModel.h"
#include "FredApplication.h"
#include "mission/missionparse.h"
#include "object/object.h"
#include "mission/Editor.h"
#include "ship/ship.h"
#include "weapon/weapon.h"

namespace fso::fred::dialogs {

// for variable-enabled weapons and ships, these are the defaults.
constexpr int ShipVarDefault = 4;
constexpr int WeaponVarDefault = 40;

constexpr int ShipStaticDefault = 5; // TODO : make this user settable?
constexpr int WeaponStaticDefault = 100; // TODO : make this user settable?

constexpr int MaxExtraItems = 9999;

TeamLoadoutDialogModel::TeamLoadoutDialogModel(QObject* parent, EditorViewport* viewport) 
		: AbstractDialogModel(parent, viewport)
{
		initializeData();
}

bool TeamLoadoutDialogModel::apply()
{

	auto present = [](const LoadoutItem& it) {
		return it.enabled && (it.extraAllocated > 0 || it.varCountIndex != -1);
	};
	
	for (int t = 0; t < Num_teams; ++t) {
		auto& in = _teams[t];
		auto& out = Team_data[t];

		// reset per team outputs
		out.num_ship_choices = 0;
		out.num_weapon_choices = 0;
		for (auto& w : out.weapon_required) {
			w = false;
		}

		// Ships
		int s = 0;

		// var ships first
		for (const auto& it : in.varShips) {
			if (it.enabled && it.infoIndex >= 0) {
				// validate
				if (ship_info_lookup(Sexp_variables[it.infoIndex].text) < 0) {
					SCP_string error = SCP_string("The variable '") + Sexp_variables[it.infoIndex].variable_name +
									   "' is not a valid ship class. Please correct before applying.";
					_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", error, {DialogButton::Ok});
					continue; // Skip invalid entries
				}

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
		}

		// static ships
		for (const auto& it : in.ships) {
			if (present(it)) {
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
		}

		out.num_ship_choices = s;

		// Weapons
		int w = 0;

		// var weapons first
		for (const auto& it : in.varWeapons) {
			if (it.enabled && it.infoIndex >= 0) {
				// validate
				if (weapon_info_lookup(Sexp_variables[it.infoIndex].text) < 0) {
					SCP_string error = SCP_string("The variable '") + Sexp_variables[it.infoIndex].variable_name +
									   "' is not a valid weapon class. Please correct before applying.";
					_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", error, {DialogButton::Ok});
					continue; // Skip invalid entries
				}

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
		}

		// static weapons
		for (const auto& it : in.weapons) {
			if (present(it)) {
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
		}

		out.num_weapon_choices = w;

		// required weapons
		for (const auto& it : in.weapons)
			if (present(it) && it.required && it.infoIndex >= 0 && it.infoIndex < MAX_WEAPON_TYPES) {
				out.weapon_required[it.infoIndex] = true;
			}

		out.do_not_validate = in.skipValidation;
	}

	return true;
}

void TeamLoadoutDialogModel::reject()
{
	// just clear out the info we're not going to end up using.
	_numberVarList.clear();
	_teams.clear();
	_teamList.clear();

} // let him go Harry, let him go

void TeamLoadoutDialogModel::initializeData()
{
	
	// first, build the team list same as other dialogs
	_teamList.clear();
	for (auto& team : Mission_event_teams_tvt) {
		_teamList.emplace_back(team.first, team.second);
	}

	// need to build list for count variables.
	_numberVarList.clear();
	_numberVarList.emplace_back("<none>", -1); // always have a none option
	for (int i = 0; i < MAX_SEXP_VARIABLES; i++) {
		if ((Sexp_variables[i].type & SEXP_VARIABLE_SET) && (Sexp_variables[i].type & SEXP_VARIABLE_NUMBER)) {
			_numberVarList.emplace_back(Sexp_variables[i].variable_name, i);
		}
	}

	_teams.clear();
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
		for (int j = 0; j < static_cast<int>(Ship_info.size()); j++) {
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
		for (int j = 0; j < static_cast<int>(Weapon_info.size()); j++) {
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

				if (varItem.extraAllocated == 0) {
					varItem.extraAllocated = ShipVarDefault;
				}

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

void TeamLoadoutDialogModel::recalcShipCapacities(TeamLoadout& team)
{
	team.largestPrimaryBankCount = 0;
	team.largestSecondaryCapacity = 0;
	auto present = [](const LoadoutItem& it) {
		return it.enabled && (it.extraAllocated > 0 || it.varCountIndex != -1);
	};
	for (const auto& it : team.ships) {
		if (!present(it))
			continue;
		const auto& si = Ship_info[it.infoIndex];
		if (si.num_primary_banks > team.largestPrimaryBankCount)
			team.largestPrimaryBankCount = si.num_primary_banks;
		int secCap = 0;
		for (int b = 0; b < si.num_secondary_banks; ++b)
			secCap += si.secondary_bank_ammo_capacity[b];
		if (secCap > team.largestSecondaryCapacity)
			team.largestSecondaryCapacity = secCap;
	}
}

const SCP_vector<std::pair<SCP_string, int>>& TeamLoadoutDialogModel::getNumberVarList()
{
	return _numberVarList;
}

const SCP_vector<std::pair<SCP_string, int>>& TeamLoadoutDialogModel::getTeamList()
{
	return _teamList;
}

void TeamLoadoutDialogModel::setCurrentTeam(int teamIn)
{
	if (!SCP_vector_inbounds(_teams, teamIn))
		return;

	_currentTeam = teamIn; // This is explicitly not a modification
}

int TeamLoadoutDialogModel::getCurrentTeam() const
{
	return _currentTeam;
}

SCP_vector<LoadoutItem> TeamLoadoutDialogModel::getVarShips() const
{
	SCP_vector<LoadoutItem> out;
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return out;

	const auto& team = _teams[_currentTeam];

	// Map active rows by name for quick lookup
	SCP_unordered_map<SCP_string, const LoadoutItem*> active;
	for (const auto& li : team.varShips) {
		active.emplace(li.name, &li);
	}

	// Enumerate string vars
	for (int i = 0; i < MAX_SEXP_VARIABLES; ++i) {
		const auto& v = Sexp_variables[i];
		if (!((v.type & SEXP_VARIABLE_SET) && (v.type & SEXP_VARIABLE_STRING)))
			continue;

		if (ship_info_lookup(v.text) < 0)
			continue; // not a valid ship class which is checked on Apply so might as well do it here, too

		auto it = active.find(v.variable_name);
		if (it != active.end()) {
			out.emplace_back(*it->second); // copy the real row
		} else {
			LoadoutItem li;
			li.infoIndex = i;
			li.enabled = false;
			li.required = false;
			li.fromVariable = true;
			li.countInWings = 0;
			li.extraAllocated = 0;
			li.varCountIndex = -1;
			li.name = v.variable_name;
			out.emplace_back(std::move(li));
		}
	}

	// Stable UI order
	std::sort(out.begin(), out.end(), [](const LoadoutItem& a, const LoadoutItem& b) {
		return stricmp(a.name.c_str(), b.name.c_str()) < 0;
	});

	return out;
}

void TeamLoadoutDialogModel::setShipVarEnabled(int varIndex, bool enabled)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (varIndex < 0 || varIndex >= MAX_SEXP_VARIABLES)
		return;
	const auto type = Sexp_variables[varIndex].type;
	if (!(type & SEXP_VARIABLE_SET) || !(type & SEXP_VARIABLE_STRING))
		return;

	// find existing by name
	auto it = std::find_if(team.varShips.begin(), team.varShips.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == varIndex;
	});

	if (it != team.varShips.end()) {
		modify(it->enabled, enabled);
		modify(it->name, SCP_string(Sexp_variables[varIndex].variable_name));

		if (enabled && it->extraAllocated == 0 && it->varCountIndex == -1) {
			modify(it->extraAllocated, ShipVarDefault);
		}
		return;
	}

	if (!enabled)
		return; // no-op

	// Create new var enabled row
	LoadoutItem li;
	li.infoIndex = varIndex;
	li.enabled = true;
	li.required = false;
	li.fromVariable = true;
	li.countInWings = 0;
	li.extraAllocated = ShipVarDefault;
	li.varCountIndex = -1;
	li.name = Sexp_variables[varIndex].variable_name;

	team.varShips.emplace_back(std::move(li));
	set_modified();
}

void TeamLoadoutDialogModel::setShipVarEnabled(const SCP_vector<std::pair<int, bool>>& updates)
{
	for (const auto& [idx, on] : updates)
		setShipVarEnabled(idx, on);
}

void TeamLoadoutDialogModel::setShipVarExtra(int varIndex, int extra)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (varIndex < 0 || varIndex >= MAX_SEXP_VARIABLES)
		return;
	const auto type = Sexp_variables[varIndex].type;
	if (!(type & SEXP_VARIABLE_SET) || !(type & SEXP_VARIABLE_STRING))
		return;

	auto it = std::find_if(team.varShips.begin(), team.varShips.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == varIndex;
	});

	if (it != team.varShips.end()) {
		modify(it->extraAllocated, (extra < 0) ? 0 : extra);
		modify(it->name, SCP_string(Sexp_variables[varIndex].variable_name));
		return;
	}

	// Create
	LoadoutItem li;
	li.infoIndex = varIndex;
	li.enabled = true; // auto enable
	li.required = false;
	li.fromVariable = true;
	li.countInWings = 0;
	li.extraAllocated = (extra < 0) ? 0 : extra;
	li.varCountIndex = -1;
	li.name = Sexp_variables[varIndex].variable_name;

	team.varShips.emplace_back(std::move(li));
	set_modified();
}

void TeamLoadoutDialogModel::setShipVarExtra(const SCP_vector<std::pair<int, int>>& updates)
{
	for (const auto& [idx, val] : updates)
		setShipVarExtra(idx, val);
}

void TeamLoadoutDialogModel::setShipVarCountVar(int varIndex, int numberVarIndex)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (varIndex < 0 || varIndex >= MAX_SEXP_VARIABLES)
		return;
	const auto enType = Sexp_variables[varIndex].type;
	if (!(enType & SEXP_VARIABLE_SET) || !(enType & SEXP_VARIABLE_STRING))
		return;

	int amtVarIdx = numberVarIndex;
	if (amtVarIdx < 0 || amtVarIdx >= MAX_SEXP_VARIABLES ||
		!((Sexp_variables[amtVarIdx].type & SEXP_VARIABLE_SET) &&
			(Sexp_variables[amtVarIdx].type & SEXP_VARIABLE_NUMBER))) {
		amtVarIdx = -1;
	}

	auto it = std::find_if(team.varShips.begin(), team.varShips.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == varIndex;
	});

	if (it != team.varShips.end()) {
		modify(it->varCountIndex, amtVarIdx);
		modify(it->name, SCP_string(Sexp_variables[varIndex].variable_name));

		if (amtVarIdx == -1 && it->extraAllocated == 0) {
			modify(it->extraAllocated, ShipVarDefault);
		}
		return;
	}

	// Create
	LoadoutItem li;
	li.infoIndex = varIndex;
	li.enabled = true; // auto enable
	li.required = false;
	li.fromVariable = true;
	li.countInWings = 0;
	li.extraAllocated = 0; // literal unused when var is set
	li.varCountIndex = amtVarIdx;
	li.name = Sexp_variables[varIndex].variable_name;

	team.varShips.emplace_back(std::move(li));
	set_modified();
}

void TeamLoadoutDialogModel::setShipVarCountVar(const SCP_vector<std::pair<int, int>>& updates)
{
	for (const auto& [idx, numIdx] : updates)
		setShipVarCountVar(idx, numIdx);
}

SCP_vector<LoadoutItem> TeamLoadoutDialogModel::getVarWeapons() const
{
	SCP_vector<LoadoutItem> out;
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return out;

	const auto& team = _teams[_currentTeam];

	SCP_unordered_map<SCP_string, const LoadoutItem*> active;
	for (const auto& li : team.varWeapons) {
		active.emplace(li.name, &li);
	}

	for (int i = 0; i < MAX_SEXP_VARIABLES; ++i) {
		const auto& v = Sexp_variables[i];
		if (!((v.type & SEXP_VARIABLE_SET) && (v.type & SEXP_VARIABLE_STRING)))
			continue;

		if (weapon_info_lookup(v.text) < 0)
			continue; // not a valid weapon class which is checked on Apply so might as well do it here, too

		auto it = active.find(v.variable_name);
		if (it != active.end()) {
			out.emplace_back(*it->second);
		} else {
			LoadoutItem li;
			li.infoIndex = i;
			li.enabled = false;
			li.required = false;
			li.fromVariable = true;
			li.countInWings = 0;
			li.extraAllocated = 0;
			li.varCountIndex = -1;
			li.name = v.variable_name;
			out.emplace_back(std::move(li));
		}
	}

	std::sort(out.begin(), out.end(), [](const LoadoutItem& a, const LoadoutItem& b) {
		return stricmp(a.name.c_str(), b.name.c_str()) < 0;
	});

	return out;
}

void TeamLoadoutDialogModel::setWeaponVarEnabled(int varIndex, bool enabled)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (varIndex < 0 || varIndex >= MAX_SEXP_VARIABLES)
		return;
	const auto type = Sexp_variables[varIndex].type;
	if (!(type & SEXP_VARIABLE_SET) || !(type & SEXP_VARIABLE_STRING))
		return;

	auto it = std::find_if(team.varWeapons.begin(), team.varWeapons.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == varIndex;
	});

	if (it != team.varWeapons.end()) {
		modify(it->enabled, enabled);
		modify(it->name, SCP_string(Sexp_variables[varIndex].variable_name));

		if (enabled && it->extraAllocated == 0 && it->varCountIndex == -1) {
			modify(it->extraAllocated, WeaponVarDefault * team.startingShipCount);
		}
		return;
	}

	if (!enabled)
		return;

	LoadoutItem li;
	li.infoIndex = varIndex;
	li.enabled = true;
	li.required = false;
	li.fromVariable = true;
	li.countInWings = 0;
	li.extraAllocated = WeaponVarDefault * team.startingShipCount;
	li.varCountIndex = -1;
	li.name = Sexp_variables[varIndex].variable_name;

	team.varWeapons.emplace_back(std::move(li));
	set_modified();
}

void TeamLoadoutDialogModel::setWeaponVarEnabled(const SCP_vector<std::pair<int, bool>>& updates)
{
	for (const auto& [idx, on] : updates)
		setWeaponVarEnabled(idx, on);
}

void TeamLoadoutDialogModel::setWeaponVarExtra(int varIndex, int extra)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (varIndex < 0 || varIndex >= MAX_SEXP_VARIABLES)
		return;
	const auto type = Sexp_variables[varIndex].type;
	if (!(type & SEXP_VARIABLE_SET) || !(type & SEXP_VARIABLE_STRING))
		return;

	auto it = std::find_if(team.varWeapons.begin(), team.varWeapons.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == varIndex;
	});

	if (it != team.varWeapons.end()) {
		modify(it->extraAllocated, (extra < 0) ? 0 : extra);
		modify(it->name, SCP_string(Sexp_variables[varIndex].variable_name));
		return;
	}

	// Create
	LoadoutItem li;
	li.infoIndex = varIndex;
	li.enabled = true; // auto enable
	li.required = false;
	li.fromVariable = true;
	li.countInWings = 0;
	li.extraAllocated = (extra < 0) ? 0 : extra;
	li.varCountIndex = -1;
	li.name = Sexp_variables[varIndex].variable_name;

	team.varWeapons.emplace_back(std::move(li));
	set_modified();
}

void TeamLoadoutDialogModel::setWeaponVarExtra(const SCP_vector<std::pair<int, int>>& updates)
{
	for (const auto& [idx, val] : updates)
		setWeaponVarExtra(idx, val);
}

void TeamLoadoutDialogModel::setWeaponVarCountVar(int varIndex, int numberVarIndex)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (varIndex < 0 || varIndex >= MAX_SEXP_VARIABLES)
		return;
	const auto enType = Sexp_variables[varIndex].type;
	if (!(enType & SEXP_VARIABLE_SET) || !(enType & SEXP_VARIABLE_STRING))
		return;

	int amtVarIdx = numberVarIndex;
	if (amtVarIdx < 0 || amtVarIdx >= MAX_SEXP_VARIABLES ||
		!((Sexp_variables[amtVarIdx].type & SEXP_VARIABLE_SET) &&
			(Sexp_variables[amtVarIdx].type & SEXP_VARIABLE_NUMBER))) {
		amtVarIdx = -1;
	}

	auto it = std::find_if(team.varWeapons.begin(), team.varWeapons.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == varIndex;
	});

	if (it != team.varWeapons.end()) {
		modify(it->varCountIndex, amtVarIdx);
		modify(it->name, SCP_string(Sexp_variables[varIndex].variable_name));

		if (amtVarIdx == -1 && it->extraAllocated == 0) {
			modify(it->extraAllocated, WeaponVarDefault * team.startingShipCount);
		}
		return;
	}

	// Create
	LoadoutItem li;
	li.infoIndex = varIndex;
	li.enabled = true; // auto enable
	li.required = false;
	li.fromVariable = true;
	li.countInWings = 0;
	li.extraAllocated = 0;
	li.varCountIndex = amtVarIdx;
	li.name = Sexp_variables[varIndex].variable_name;

	team.varWeapons.emplace_back(std::move(li));
	set_modified();
}

void TeamLoadoutDialogModel::setWeaponVarCountVar(const SCP_vector<std::pair<int, int>>& updates)
{
	for (const auto& [idx, numIdx] : updates)
		setWeaponVarCountVar(idx, numIdx);
}

const SCP_vector<LoadoutItem>& TeamLoadoutDialogModel::getShips() const
{
	static const SCP_vector<LoadoutItem> kEmpty;
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return kEmpty; // should not happen

	return _teams[_currentTeam].ships;
}

void TeamLoadoutDialogModel::setShipEnabled(int classIndex, bool on)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (classIndex < 0 || classIndex >= (int)Ship_info.size())
		return;
	if (!Ship_info[classIndex].flags[Ship::Info_Flags::Player_ship])
		return;

	auto it = std::find_if(team.ships.begin(), team.ships.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == classIndex;
	});
	if (it == team.ships.end())
		return;

	const bool wasPresent = it->enabled && (it->extraAllocated > 0 || it->varCountIndex != -1);

	if (on) {
		modify(it->enabled, true);

		if (it->extraAllocated == 0 && it->varCountIndex == -1) {
			modify(it->extraAllocated, ShipStaticDefault);
		}
	} else {
		modify(it->enabled, false);
		modify(it->extraAllocated, 0);
		modify(it->varCountIndex, -1);
	}

	const bool isPresent = it->enabled && (it->extraAllocated > 0 || it->varCountIndex != -1);
	if (wasPresent != isPresent)
		recalcShipCapacities(team);
}

void TeamLoadoutDialogModel::setShipEnabled(const SCP_vector<std::pair<int, bool>>& updates)
{
	for (const auto& [cls, on] : updates)
		setShipEnabled(cls, on);
}

void TeamLoadoutDialogModel::setShipExtra(int classIndex, int count)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (classIndex < 0 || classIndex >= (int)Ship_info.size())
		return;
	if (!Ship_info[classIndex].flags[Ship::Info_Flags::Player_ship])
		return;

	auto it = std::find_if(team.ships.begin(), team.ships.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == classIndex;
	});
	if (it == team.ships.end())
		return;

	modify(it->extraAllocated, (count < 0) ? 0 : count);
	setShipEnabled(classIndex, (count > 0 || it->varCountIndex >=0));
}

void TeamLoadoutDialogModel::setShipExtra(const SCP_vector<std::pair<int, int>>& updates)
{
	for (const auto& [cls, cnt] : updates)
		setShipExtra(cls, cnt);
}

void TeamLoadoutDialogModel::setShipCountVar(int classIndex, int numberVarIndex)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (classIndex < 0 || classIndex >= (int)Ship_info.size())
		return;
	if (!Ship_info[classIndex].flags[Ship::Info_Flags::Player_ship])
		return;

	auto it = std::find_if(team.ships.begin(), team.ships.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == classIndex;
	});
	if (it == team.ships.end())
		return;

	int idx = numberVarIndex;
	if (idx >= 0) {
		if (idx >= MAX_SEXP_VARIABLES)
			idx = -1;
		else {
			const auto t = Sexp_variables[idx].type;
			if (!((t & SEXP_VARIABLE_SET) && (t & SEXP_VARIABLE_NUMBER)))
				idx = -1;
		}
	}

	modify(it->varCountIndex, idx);

	if (idx == -1 && it->enabled && it->extraAllocated == 0 && it->countInWings == 0) {
		modify(it->extraAllocated, ShipStaticDefault);
	}

	setShipEnabled(classIndex, (numberVarIndex >= 0 || it->extraAllocated > 0));
}

void TeamLoadoutDialogModel::setShipCountVar(const SCP_vector<std::pair<int, int>>& updates)
{
	for (const auto& [cls, idx] : updates)
		setShipCountVar(cls, idx);
}

const SCP_vector<LoadoutItem>& TeamLoadoutDialogModel::getWeapons() const
{
	static const SCP_vector<LoadoutItem> kEmpty;
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return kEmpty; // should not happen

	return _teams[_currentTeam].weapons;
}

void TeamLoadoutDialogModel::setWeaponEnabled(int classIndex, bool on)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (classIndex < 0 || classIndex >= (int)Weapon_info.size())
		return;
	if (!Weapon_info[classIndex].wi_flags[Weapon::Info_Flags::Player_allowed])
		return;

	auto it = std::find_if(team.weapons.begin(), team.weapons.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == classIndex;
	});
	if (it == team.weapons.end())
		return;

	if (on) {
		modify(it->enabled, true);

		if (it->extraAllocated == 0 && it->varCountIndex == -1) {
			modify(it->extraAllocated, WeaponStaticDefault);
		}
	} else {
		modify(it->enabled, false);
		modify(it->extraAllocated, 0);
		modify(it->varCountIndex, -1);
		modify(it->required, false);
	}
}

void TeamLoadoutDialogModel::setWeaponEnabled(const SCP_vector<std::pair<int, bool>>& updates)
{
	for (const auto& [cls, on] : updates)
		setWeaponEnabled(cls, on);
}

void TeamLoadoutDialogModel::setWeaponExtra(int classIndex, int count)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (classIndex < 0 || classIndex >= (int)Weapon_info.size())
		return;
	if (!Weapon_info[classIndex].wi_flags[Weapon::Info_Flags::Player_allowed])
		return;

	auto it = std::find_if(team.weapons.begin(), team.weapons.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == classIndex;
	});
	if (it == team.weapons.end())
		return;

	modify(it->extraAllocated, (count < 0) ? 0 : count);
	setWeaponEnabled(classIndex, (count > 0 || it->varCountIndex >= 0));
}

void TeamLoadoutDialogModel::setWeaponExtra(const SCP_vector<std::pair<int, int>>& updates)
{
	for (const auto& [cls, cnt] : updates)
		setWeaponExtra(cls, cnt);
}

void TeamLoadoutDialogModel::setWeaponCountVar(int classIndex, int numberVarIndex)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (classIndex < 0 || classIndex >= (int)Weapon_info.size())
		return;
	if (!Weapon_info[classIndex].wi_flags[Weapon::Info_Flags::Player_allowed])
		return;

	auto it = std::find_if(team.weapons.begin(), team.weapons.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == classIndex;
	});
	if (it == team.weapons.end())
		return;

	int idx = numberVarIndex;
	if (idx >= 0) {
		if (idx >= MAX_SEXP_VARIABLES)
			idx = -1;
		else {
			const auto t = Sexp_variables[idx].type;
			if (!((t & SEXP_VARIABLE_SET) && (t & SEXP_VARIABLE_NUMBER)))
				idx = -1;
		}
	}

	modify(it->varCountIndex, idx);

	if (idx == -1 && it->enabled && it->extraAllocated == 0 && it->countInWings == 0) {
		modify(it->extraAllocated, WeaponStaticDefault);
	}

	setWeaponEnabled(classIndex, (numberVarIndex >= 0 || it->extraAllocated > 0));
}

void TeamLoadoutDialogModel::setWeaponCountVar(const SCP_vector<std::pair<int, int>>& updates)
{
	for (const auto& [cls, idx] : updates)
		setWeaponCountVar(cls, idx);
}

void TeamLoadoutDialogModel::setWeaponRequired(int classIndex, bool on)
{
	if (!SCP_vector_inbounds(_teams, _currentTeam))
		return;
	auto& team = _teams[_currentTeam];

	if (classIndex < 0 || classIndex >= (int)Weapon_info.size())
		return;
	if (!Weapon_info[classIndex].wi_flags[Weapon::Info_Flags::Player_allowed])
		return;

	auto it = std::find_if(team.weapons.begin(), team.weapons.end(), [&](const LoadoutItem& li) {
		return li.infoIndex == classIndex;
	});
	if (it == team.weapons.end())
		return;

	if (it->fromVariable) {
		on = false; // cannot be required if from variable
	}

	modify(it->required, on);

	if (on && !it->enabled) { // required implies enabled
		setWeaponEnabled(classIndex, true);
	}
}

void TeamLoadoutDialogModel::setWeaponRequired(const SCP_vector<std::pair<int, bool>>& updates)
{
	for (const auto& [cls, on] : updates)
		setWeaponRequired(cls, on);
}

bool TeamLoadoutDialogModel::getSkipValidation()
{
	return _teams[_currentTeam].skipValidation;
}

void TeamLoadoutDialogModel::setSkipValidation(const bool skipIt)
{
	// this is designed to be a global control, so turn this off in TvT, until we hear from someone otherwise.
	for (auto& team : _teams) {
		modify(team.skipValidation, skipIt);
	}
}

void TeamLoadoutDialogModel::copyToOtherTeams()
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

		set_modified();

		index++;
	}
}

SCP_string TeamLoadoutDialogModel::getVariableName(int varIndex)
{
	if (varIndex < 0 || varIndex >= MAX_SEXP_VARIABLES)
		return {};

	return Sexp_variables[varIndex].variable_name;
}

SCP_string TeamLoadoutDialogModel::getVariableValueAsString(int varIndex)
{
	if (varIndex < 0 || varIndex >= MAX_SEXP_VARIABLES)
		return {};
	const auto& v = Sexp_variables[varIndex];
	if (!(v.type & SEXP_VARIABLE_SET))
		return {};
	if (v.type & SEXP_VARIABLE_NUMBER) {
		return v.text;
	} else if (v.type & SEXP_VARIABLE_STRING) {
		return v.text;
	}
	return {};
}

int TeamLoadoutDialogModel::getLoadoutMaxValue()
{
	return MaxExtraItems;
}

} // namespace fso::fred::dialogs