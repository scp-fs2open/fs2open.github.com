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
	return true;
}

void SupportRearmDialogModel::reject() {}

void SupportRearmDialogModel::setInitial(const SupportRearmSettings& settings)
{
	_settings = settings;
}

SupportRearmSettings& SupportRearmDialogModel::settings()
{
	return _settings;
}

const SupportRearmSettings& SupportRearmDialogModel::settings() const
{
	return _settings;
}

const SCP_vector<int>& SupportRearmDialogModel::visibleWeaponClasses() const
{
	return _visibleWeaponClasses;
}

} // namespace fso::fred::dialogs
