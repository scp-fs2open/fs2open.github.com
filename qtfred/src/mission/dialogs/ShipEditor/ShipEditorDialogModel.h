#pragma once

#include "../AbstractDialogModel.h"
#include "mission/util.h"
#include "ship/ship.h"

namespace fso::fred::dialogs {

class ShipEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	explicit ShipEditorDialogModel(QObject* parent, EditorViewport* viewport);
	bool apply() override;
	void reject() override;

	void setShipName(const SCP_string& name);
	SCP_string getShipName() const;

	void setShipDisplayName(const SCP_string& displayName);
	SCP_string getShipDisplayName() const;

	void setShipClass(int shipClass);
	int getShipClass() const;

	void setAIClass(int aiClass);
	int getAIClass() const;

	void setTeam(int team);
	int getTeam() const;

	void setLayer(const SCP_string& layer);
	SCP_string getLayer() const;

	void setCargo(const SCP_string& cargo);
	SCP_string getCargo() const;

	void setCargoTitle(const SCP_string& cargoTitle);
	SCP_string getCargoTitle() const;

	void setAltName(const SCP_string& altName);
	SCP_string getAltName() const;

	void setCallsign(const SCP_string& callsign);
	SCP_string getCallsign() const;

	SCP_string getWing() const;

	void setHotkey(int hotkey);
	int getHotkey() const;

	void setPersona(int persona);
	int getPersona() const;

	void setScore(int score);
	int getScore() const;

	void setAssist(int assist);
	int getAssist() const;

	void setPlayer(bool isPlayer);
	bool getPlayer() const;

	void setRespawn(int respawn);
	int getRespawn() const;

	void setArrivalLocationIndex(int index);
	int getArrivalLocationIndex() const;
	void setArrivalLocation(ArrivalLocation location);
	ArrivalLocation getArrivalLocation() const;

	bool arrivalNeedsTarget() const;
	bool arrivalNeedsDistance() const;

	void setArrivalTarget(int targetIndex);
	int getArrivalTarget() const;

	void setArrivalDistance(int distance);
	int getArrivalDistance() const;

	void setArrivalDelay(int delay);
	int getArrivalDelay() const;

	void setArrivalCue(bool updateCue);
	bool getArrivalCue() const;

	void setArrivalTreeDirty(int formula);
	int getArrivalFormula() const;

	void setNoArrivalWarp(int state);
	int getNoArrivalWarp() const;

	void setDockWarpinChange(const int state);
	int getDockWarpinChange() const;

	void setDepartureLocationIndex(int index);
	int getDepartureLocationIndex() const;
	void setDepartureLocation(DepartureLocation location);
	DepartureLocation getDepartureLocation() const;

	void setDepartureTarget(int targetIndex);
	int getDepartureTarget() const;

	void setDepartureDelay(int delay);
	int getDepartureDelay() const;

	void setDepartureCue(bool updateCue);
	bool getDepartureCue() const;

	void setDepartureTreeDirty(int formula);
	int getDepartureFormula() const;
	void setNoDepartureWarp(int state);
	int getNoDepartureWarp() const;

	void setDockWarpoutChange(const int state);
	int getDockWarpoutChange() const;

	void onPrevious();
	void onNext();
	void onDeleteShip();
	void onShipReset();

	bool wingIsPlayerWing(int wingNum) const;
	const SCP_set<size_t>& getShipOrders() const;

	bool getTexEditEnable() const;

	static int tristate_set(int val, int curState);

	int getSingleShip() const;
	bool getIfMultipleShips() const;
	int getNumSelectedPlayers() const;
	int getNumUnmarkedPlayers() const;
	bool getUIEnable() const;
	int getNumSelectedShips() const;
	int getUseCue() const;
	int getNumSelectedObjects() const;
	int getNumValidPlayers() const;
	int getIfPlayerShip() const;

	static SCP_vector<std::pair<SCP_string, int>> getPlayerOrders();
	void applyPlayerOrders(const SCP_vector<std::pair<SCP_string, int>>& orders);

	SCP_vector<std::pair<SCP_string, bool>> getArrivalPaths() const;
	void setArrivalPaths(const SCP_vector<std::pair<SCP_string, bool>>& paths);

	SCP_vector<std::pair<SCP_string, bool>> getDeparturePaths() const;
	void setDeparturePaths(const SCP_vector<std::pair<SCP_string, bool>>& paths);

	void initializeData();

  private: // NOLINT(readability-redundant-access-specifiers)
	void setModified();
	void shipAltNameClose(int baseShip);
	void shipCallsignClose(int baseShip);
	static int makeShipList(int* arr);
	int computeArrivalMinDist() const;

	int _noDepartureWarp;
	int _noArrivalWarp;
	int _dockWarpoutChange;
	int _dockWarpinChange;
	bool _isPlayerShip;
	int _departureTreeFormula;
	int _arrivalTreeFormula;
	SCP_string _shipName;
	SCP_string _shipDisplayName;
	SCP_string _cargo;
	SCP_string _cargoTitle;
	SCP_string _altName;
	SCP_string _callsign;
	int _shipClass;
	int _team;
	SCP_string _layer;
	int _arrivalLocation;
	int _departureLocation;
	int _aiClass;
	int _arrivalDelay;
	int _departureDelay;
	int _hotkey;
	bool _updateArrival;
	bool _updateDeparture;
	int _score;
	int _assistScore;
	int _arrivalDist;
	int _arrivalTarget;
	int _departureTarget;
	int _persona;
	SCP_string _wing;
	int _missionType;
	bool _enable = true;
	int _playerCount;
	int _shipCount;
	bool _multiEdit;
	int _cueInit;
	int _totalCount;
	int _pvalidCount;
	int _pshipCount;
	int _singleShip;
	int _playerShipIndex;
	SCP_set<size_t> _shipOrders;
	bool _texEnable = true;
	int _respawnPriority;
	SCP_vector<std::pair<SCP_string, bool>> _arrivalPaths;
	SCP_vector<std::pair<SCP_string, bool>> _departurePaths;
};

} // namespace fso::fred::dialogs
