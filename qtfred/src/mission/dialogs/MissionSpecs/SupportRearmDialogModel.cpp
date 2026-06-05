#include "SupportRearmDialogModel.h"

#include "weapon/weapon.h"

namespace fso::fred::dialogs {

SupportRearmDialogModel::SupportRearmDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	for (int i = 0; i < weapon_info_size(); ++i) {
		if (Weapon_info[i].wi_flags[Weapon::Info_Flags::Player_allowed]) {
			_visibleWeaponClasses.push_back(i);
		}
	}
}

bool SupportRearmDialogModel::apply()
{
	// No direct application; the parent MissionSpecDialogModel reads settings() and applies them.
	return true;
}

void SupportRearmDialogModel::reject()
{
	// No direct rejection; the parent simply discards this model on cancel/close.
}

void SupportRearmDialogModel::setInitial(const SupportRearmSettings& settings)
{
	_working = settings;
}

const SupportRearmSettings& SupportRearmDialogModel::settings() const
{
	return _working;
}

const SCP_vector<int>& SupportRearmDialogModel::visibleWeaponClasses() const
{
	return _visibleWeaponClasses;
}

void SupportRearmDialogModel::setDisallowSupportShips(bool value)
{
	modify(_working.disallowSupportShips, value);
}

void SupportRearmDialogModel::setSupportRepairsHull(bool value)
{
	modify(_working.supportRepairsHull, value);
}

void SupportRearmDialogModel::setMaxHullRepair(float value)
{
	modify(_working.maxHullRepair, value);
}

void SupportRearmDialogModel::setMaxSubsysRepair(float value)
{
	modify(_working.maxSubsysRepair, value);
}

void SupportRearmDialogModel::setDisallowSupportRearm(bool value)
{
	modify(_working.disallowSupportRearm, value);
}

void SupportRearmDialogModel::setLimitRearmToPool(bool value)
{
	modify(_working.limitRearmToPool, value);
}

void SupportRearmDialogModel::setRearmPoolFromLoadout(bool value)
{
	modify(_working.rearmPoolFromLoadout, value);
}

void SupportRearmDialogModel::setAllowWeaponPrecedence(bool value)
{
	modify(_working.allowWeaponPrecedence, value);
}

void SupportRearmDialogModel::setWeaponPoolEntry(int team, int weaponClass, int amount)
{
	if (team < 0 || team >= MAX_TVT_TEAMS) {
		return;
	}
	if (weaponClass < 0 || weaponClass >= weapon_info_size()) {
		return;
	}

	const int normalized = Weapon_info[weaponClass].disallow_rearm ? 0 : ((amount < 0) ? -1 : amount);
	modify(_working.rearmWeaponPool[team][weaponClass], normalized);
}

void SupportRearmDialogModel::setAllVisibleWeaponPoolEntries(int team, int amount)
{
	if (team < 0 || team >= MAX_TVT_TEAMS) {
		return;
	}

	for (auto cls : _visibleWeaponClasses) {
		const int normalized = Weapon_info[cls].disallow_rearm ? 0 : ((amount < 0) ? -1 : amount);
		modify(_working.rearmWeaponPool[team][cls], normalized);
	}
}

} // namespace fso::fred::dialogs
