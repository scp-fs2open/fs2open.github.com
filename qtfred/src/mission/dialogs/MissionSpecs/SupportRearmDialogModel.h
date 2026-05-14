#pragma once

#include "mission/dialogs/AbstractDialogModel.h"
#include "mission/dialogs/MissionSpecDialogModel.h"

namespace fso::fred::dialogs {

class SupportRearmDialogModel final : public AbstractDialogModel {
  public:
	SupportRearmDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setInitial(const SupportRearmSettings& settings);
	const SupportRearmSettings& settings() const;

	void setDisallowSupportShips(bool value);
	void setSupportRepairsHull(bool value);
	void setMaxHullRepair(float value);
	void setMaxSubsysRepair(float value);
	void setDisallowSupportRearm(bool value);
	void setLimitRearmToPool(bool value);
	void setRearmPoolFromLoadout(bool value);
	void setAllowWeaponPrecedence(bool value);

	// Pool edits. `amount < 0` becomes -1 (unlimited); disallow_rearm weapons are always pinned to 0.
	void setWeaponPoolEntry(int team, int weaponClass, int amount);
	void setAllVisibleWeaponPoolEntries(int team, int amount);

	const SCP_vector<int>& visibleWeaponClasses() const;

  private:
	SupportRearmSettings _working;
	SCP_vector<int> _visibleWeaponClasses;
};

} // namespace fso::fred::dialogs
