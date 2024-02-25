#ifndef LOADOUTEDITORDIALOGMODEL
#define LOADOUTEDITORDIALOGMODEL

#include "AbstractDialogModel.h"
#include "globalincs/pstypes.h"

namespace fso {
namespace fred {
namespace dialogs {

struct LoadoutItem {
	LoadoutItem(int infoIn, bool enabledIn, int countIn, int extraIn, int varCountIn, SCP_string nameIn) {
		infoIndex = infoIn; enabled = enabledIn; countInWings = countIn; extraAllocated = extraIn; varCountIndex = varCountIn; name = nameIn;
	}
	int infoIndex; // for var items, this points to the sexp index.
	bool enabled;
	int countInWings;
	int extraAllocated;
	int varCountIndex;
	SCP_string name;
};

struct TeamLoadout {
	TeamLoadout() {
		startingShipCount = 0;
		largestPrimaryBankCount = 0;
		largestSecondaryCapacity = 0;
		ships = {};
		weapons = {};
		varShips = {};
		varWeapons = {};
	}

	int startingShipCount; // TODO: Make sure this gets pop everywhere
	int largestPrimaryBankCount;
	int largestSecondaryCapacity;
	SCP_vector<LoadoutItem> ships;
	SCP_vector<LoadoutItem> weapons;
	SCP_vector<LoadoutItem> varShips; 
	SCP_vector<LoadoutItem> varWeapons;
};

class LoadoutDialogModel : public AbstractDialogModel {
public:
	LoadoutDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	int getCurrentTeam();
	void setPlayerEntryDelay(float delay);
	float getPlayerEntryDelay();

	SCP_vector<std::pair<SCP_string,bool>> getShipList();
	SCP_vector<std::pair<SCP_string,bool>> getWeaponList();
	SCP_vector<std::pair<SCP_string,bool>> getShipEnablerVariables();
	SCP_vector<std::pair<SCP_string,bool>> getWeaponEnablerVariables();

	SCP_string getCountVarShips(SCP_vector<SCP_string> namesIn);
	SCP_string getCountVarWeapons(SCP_vector<SCP_string> namesIn);
	SCP_string getCountVarShipEnabler(SCP_vector<SCP_string> namesIn);
	SCP_string getCountVarWeaponEnabler(SCP_vector<SCP_string> namesIn);

	int getExtraAllocatedShips(SCP_vector<SCP_string> namesIn);
	int getExtraAllocatedWeapons(SCP_vector<SCP_string> namesIn);
	int getExtraAllocatedShipEnabler(SCP_vector<SCP_string> namesIn);
	int getExtraAllocatedWeaponEnabler(SCP_vector<SCP_string> namesIn);

	//TODO! These needs definitions.
	void setShipEnabled(const SCP_vector<SCP_string>& list, bool enabled);
	void setWeaponEnabled(const SCP_vector<SCP_string>& list, bool enabled);
	void setExtraAllocatedShipCount(const SCP_vector<SCP_string>& list, const uint count);
	void setExtraAllocatedWeaponCount(const SCP_vector<SCP_string>& list, const uint count);
	
	void setShipInfo(SCP_string textIn, bool enabled, int extraAllocated, SCP_string varForCount);
	void setWeaponInfo(SCP_string textIn, bool enabled, int extraAllocated, SCP_string varForCount);
	void setShipEnablerVariables(SCP_vector<SCP_string> variablesIn, bool enabled, int extraAllocated, SCP_string varForCount);
	void setWeaponEnablerVariables(SCP_vector<SCP_string> variablesIn, bool enabled, int extraAllocated, SCP_string varForCount);

	void switchTeam(int teamIn);
	void copyToOtherTeam();

	bool spinBoxUpdateRequired();

private:

	SCP_string createItemString(bool ship, int itemIndex);
	void buildCurrentLists();
	void initializeData();

	float _playerEntryDelay;
	int _currentTeam;

	SCP_vector<TeamLoadout> _teams; // all loadout info for each team
	SCP_vector<std::pair<SCP_string,bool>> _shipList;
	SCP_vector<std::pair<SCP_string,bool>> _weaponList;
	SCP_vector<std::pair<SCP_string,bool>> _shipVarList;
	SCP_vector<std::pair<SCP_string,bool>> _weaponVarList;
	SCP_vector<SCP_string> _numberVarList;

	bool _spinBoxUpdateRequired;
};


}
}
}


#endif