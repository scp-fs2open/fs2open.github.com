#pragma once

#include "AbstractDialogModel.h"
#include "globalincs/pstypes.h"

namespace fso::fred::dialogs {

struct LoadoutItem {
	LoadoutItem() = default;

	LoadoutItem(int infoIn,
		bool enabledIn,
		bool requiredIn,
		bool fromVariableIn,
		int countIn,
		int extraIn,
		int varCountIn,
		SCP_string nameIn)
	{
		infoIndex = infoIn;
		enabled = enabledIn;
		required = requiredIn;
		fromVariable = fromVariableIn;
		countInWings = countIn;
		extraAllocated = extraIn;
		varCountIndex = varCountIn;
		name = nameIn;
	}
	int infoIndex; // for var items, this points to the sexp index.
	bool enabled;
	bool required;
	bool fromVariable;
	int countInWings;
	int extraAllocated;
	int varCountIndex;
	SCP_string name;
};

struct TeamLoadout {
	TeamLoadout()
	{
		startingShipCount = 0;
		largestPrimaryBankCount = 0;
		largestSecondaryCapacity = 0;
		ships = {};
		weapons = {};
		varShips = {};
		varWeapons = {};
		skipValidation = false;
	}

	int startingShipCount;
	int largestPrimaryBankCount;
	int largestSecondaryCapacity;
	SCP_vector<LoadoutItem> ships;
	SCP_vector<LoadoutItem> weapons;
	SCP_vector<LoadoutItem> varShips;
	SCP_vector<LoadoutItem> varWeapons;
	bool skipValidation;
};

class TeamLoadoutDialogModel : public AbstractDialogModel {
  public:
	TeamLoadoutDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	int getCurrentTeam();

	const SCP_vector<LoadoutItem>& getShipList() const;
	const SCP_vector<LoadoutItem>& getWeaponList() const;
	const SCP_vector<SCP_string>& getRequiredWeapons() const;
	const SCP_vector<LoadoutItem>& getShipEnablerVariables() const;
	const SCP_vector<LoadoutItem>& getWeaponEnablerVariables() const;

	SCP_string getCountVarShips(SCP_vector<SCP_string> namesIn);
	SCP_string getCountVarWeapons(SCP_vector<SCP_string> namesIn);
	SCP_string getCountVarShipEnabler(SCP_vector<SCP_string> namesIn);
	SCP_string getCountVarWeaponEnabler(SCP_vector<SCP_string> namesIn);

	SCP_vector<SCP_string> getNumberVarList();
	const SCP_vector<std::pair<SCP_string, int>>& getTeamList();

	int getExtraAllocatedShips(SCP_vector<SCP_string> namesIn);
	int getExtraAllocatedWeapons(SCP_vector<SCP_string> namesIn);
	int getExtraAllocatedShipEnabler(SCP_vector<SCP_string> namesIn);
	int getExtraAllocatedWeaponEnabler(SCP_vector<SCP_string> namesIn);
	bool getSkipValidation();

	void setShipEnabled(const SCP_vector<SCP_string>& list, bool enabled);
	void setShipVariableEnabled(const SCP_vector<SCP_string>& list, bool enabled);
	void setWeaponEnabled(const SCP_vector<SCP_string>& list, bool enabled);
	void setWeaponVariableEnabled(const SCP_vector<SCP_string>& list, bool enabled);
	void setExtraAllocatedShipCount(const SCP_vector<SCP_string>& list, const int count);
	void setExtraAllocatedForShipVariablesCount(const SCP_vector<SCP_string>& list, const int count);
	void setExtraAllocatedWeaponCount(const SCP_vector<SCP_string>& list, const int count);
	void setExtraAllocatedForWeaponVariablesCount(const SCP_vector<SCP_string>& list, const int count);

	void setExtraAllocatedViaVariable(const SCP_vector<SCP_string>& list, const SCP_string& variable, const bool ship, const bool variableMode);
	void setRequiredWeapon(const SCP_vector<SCP_string>& list, const bool required);
	void setSkipValidation(const bool skipIt);

	void switchTeam(int teamIn);
	void copyToOtherTeam();

  private:
	SCP_string createItemString(bool ship, bool variable, int itemIndex, const char* variableIn = "");
	void buildCurrentLists();
	void initializeData();

	int _currentTeam;

	SCP_vector<TeamLoadout> _teams; // all loadout info for each team
	SCP_vector<LoadoutItem> _shipList;
	SCP_vector<LoadoutItem> _weaponList;
	SCP_vector<LoadoutItem> _shipVarList;
	SCP_vector<LoadoutItem> _weaponVarList;
	SCP_vector<SCP_string> _numberVarList;
	SCP_vector<SCP_string> _requiredWeaponsList;
	SCP_vector<std::pair<SCP_string, int>> _team_list;
};

} // namespace fso::fred::dialogs