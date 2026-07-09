#pragma once

#include "../AbstractDialogModel.h"

#include <object/objectdock.h>
#include <mission/management.h>

namespace fso::fred::dialogs {

struct dockpoint_information {
	int dockee_shipnum;
	int dockee_point;
};

constexpr auto BLANKFIELD = 101;

void initialStatusMarkDockLeaderHelper(object* objp, dock_function_info* infop, EditorViewport* viewport);
void initialStatusUnmarkDockHandledFlag(object* objp);
void resetArrivalToFalse(int shipnum, bool resetWing, EditorViewport* viewport);
bool setCueToFalse(int* cue);

class ShipInitialStatusDialogModel : public AbstractDialogModel {
	Q_OBJECT
  public:
	ShipInitialStatusDialogModel(QObject* parent, EditorViewport* viewport, bool multi);

	bool apply() override;
	void reject() override;

	void setVelocity(int velocity);
	int getVelocity() const;

	void setHull(int hull);
	int getHull() const;

	void setHasShield(int hasShield);
	int getHasShield() const;

	void setShieldHull(int shieldHull);
	int getShieldHull() const;

	void setForceShield(int forceShield);
	int getForceShield() const;

	void setShipLocked(int locked);
	int getShipLocked() const;

	void setWeaponLocked(int locked);
	int getWeaponLocked() const;

	void setPrimariesDisabled(int disabled);
	int getPrimariesDisabled() const;

	void setSecondariesDisabled(int disabled);
	int getSecondariesDisabled() const;

	void setTurretsDisabled(int disabled);
	int getTurretsDisabled() const;

	void setAfterburnerDisabled(int disabled);
	int getAfterburnerDisabled() const;

	void setDamage(int damage);
	int getDamage() const;

	SCP_string getCargo() const;
	void setCargo(const SCP_string& cargo);

	SCP_string getCargoTitle() const;
	void setCargoTitle(const SCP_string& cargoTitle);

	SCP_string getColor() const;
	void setColor(const SCP_string& color);

	void changeSubsys(int subsysIndex);

	int getShip() const;
	int getNumDockPoints() const;
	int getShipHasScannableSubsystems() const;
	dockpoint_information* getDockpointArray() const;
	void setDockee(int dockPointIndex, int dockeeShipnum);
	void setDockeePoint(int dockPointIndex, int dockeePoint);

	bool getUseTeamcolours() const;
	bool getIfMultipleShips() const;

	bool getToggleSubsystemScanning() const;
	static bool getUseNewScanningBehavior();

	int getGuardian() const;
	void setGuardian(int guardian);

	bool getMoveShipsWhenUndocking() const;
	void setMoveShipsWhenUndocking(bool moveShips);

  private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData(bool multi);
	void updateDockingInfo();
	void undock(object* objp1, object* objp2);
	void dock(object* objp, int dockpoint, object* otherObjp, int otherDockpoint);
	void dockEvaluateAllDockedObjects(object* objp,
		dock_function_info* infop,
		void (*function)(object*, dock_function_info*, EditorViewport*));
	void dockEvaluateAllDockedObjects(object* objp, dock_function_info* infop, void (*function)(object*));
	void dockEvaluateTree(object* objp,
		dock_function_info* infop,
		void (*function)(object*, dock_function_info*, EditorViewport*),
		ubyte* visited_bitstring);
	void dockEvaluateTree(object* objp, dock_function_info* infop, void (*function)(object*), ubyte* visited_bitstring);

	int _guardianThreshold;
	int _ship;
	int _curSubsys = -1;
	int _damage;
	int _shields;
	int _forceShields;
	int _velocity;
	int _hull;
	int _hasShields;
	int _shipLocked;
	int _weaponsLocked;
	SCP_string _cargoName;
	SCP_string _cargoTitle;
	int _primariesLocked;
	int _secondariesLocked;
	int _turretsLocked;
	int _afterburnerLocked;
	SCP_string _teamColorSetting;
	int _shipHasScannableSubsystems;
	int _numDockPoints;
	std::unique_ptr<dockpoint_information[]> _dockpointArray;
	bool _multiEdit;
	bool _useTeams = false;
	bool _moveShipsWhenUndocking = true;
};

template <typename T>
void handle_inconsistent_flag(flagset<T>& flags, T flag, int value)
{
	if (value == CheckState::Checked) {
		flags.set(flag);
	} else if (value == CheckState::Unchecked) {
		flags.remove(flag);
	}
}

} // namespace fso::fred::dialogs
