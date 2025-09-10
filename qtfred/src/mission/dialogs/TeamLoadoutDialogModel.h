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
		name = std::move(nameIn);
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

	const SCP_vector<std::pair<SCP_string, int>>& getNumberVarList();
	const SCP_vector<std::pair<SCP_string, int>>& getTeamList();

	void setCurrentTeam(int teamIn);
	int getCurrentTeam() const;

	SCP_vector<LoadoutItem> getVarShips() const;
	void setShipVarEnabled(int varIndex, bool enabled);
	void setShipVarEnabled(const SCP_vector<std::pair<int, bool>>& updates);
	void setShipVarExtra(int varIndex, int extra);
	void setShipVarExtra(const SCP_vector<std::pair<int, int>>& updates);
	void setShipVarCountVar(int varIndex, int numberVarIndex);
	void setShipVarCountVar(const SCP_vector<std::pair<int, int>>& updates);

	SCP_vector<LoadoutItem> getVarWeapons() const;
	void setWeaponVarEnabled(int varIndex, bool enabled);
	void setWeaponVarEnabled(const SCP_vector<std::pair<int, bool>>& updates);
	void setWeaponVarExtra(int varIndex, int extra);
	void setWeaponVarExtra(const SCP_vector<std::pair<int, int>>& updates);
	void setWeaponVarCountVar(int varIndex, int numberVarIndex);
	void setWeaponVarCountVar(const SCP_vector<std::pair<int, int>>& updates);

	const SCP_vector<LoadoutItem>& getShips() const;
	void setShipEnabled(int classIndex, bool on);
	void setShipEnabled(const SCP_vector<std::pair<int, bool>>& updates);
	void setShipExtra(int classIndex, int extra);
	void setShipExtra(const SCP_vector<std::pair<int, int>>& updates);
	void setShipCountVar(int classIndex, int numberVarIndex);
	void setShipCountVar(const SCP_vector<std::pair<int, int>>& updates);

	const SCP_vector<LoadoutItem>& getWeapons() const;
	void setWeaponEnabled(int classIndex, bool on);
	void setWeaponEnabled(const SCP_vector<std::pair<int, bool>>& updates);
	void setWeaponExtra(int classIndex, int extra);
	void setWeaponExtra(const SCP_vector<std::pair<int, int>>& updates);
	void setWeaponCountVar(int classIndex, int numberVarIndex);
	void setWeaponCountVar(const SCP_vector<std::pair<int, int>>& updates);
	void setWeaponRequired(int classIndex, bool required);
	void setWeaponRequired(const SCP_vector<std::pair<int, bool>>& updates);

	bool getSkipValidation();
	void setSkipValidation(const bool skipIt);

	void copyToOtherTeams();

	static SCP_string getVariableName(int varIndex);	
	static SCP_string getVariableValueAsString(int varIndex);

	static int getLoadoutMaxValue();

  private:
	void initializeData();
	static void recalcShipCapacities(TeamLoadout& team);

	int _currentTeam = 0;

	SCP_vector<TeamLoadout> _teams; // all loadout info for each team
	SCP_vector<std::pair<SCP_string, int>> _numberVarList;
	SCP_vector<std::pair<SCP_string, int>> _teamList;
};

} // namespace fso::fred::dialogs