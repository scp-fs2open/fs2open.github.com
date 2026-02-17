#include "WeaponsTBLViewerModel.h"

#include <weapon/weapon.h>
#include <utils/table_viewer.h>
namespace fso::fred::dialogs {
WeaponsTBLViewerModel::WeaponsTBLViewerModel(QObject* parent, EditorViewport* viewport, int wc)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(wc);
}
bool WeaponsTBLViewerModel::apply()
{
	return true;
}
void WeaponsTBLViewerModel::reject() {}
void WeaponsTBLViewerModel::initializeData(const int wc)
{
	const weapon_info* sip = &Weapon_info[wc];
	text.clear();

	if (!sip) {
		return;
	}

	text = table_viewer::get_table_entry_text("weapons.tbl", "*-wep.tbm", sip->name);
	modelChanged();
}
SCP_string WeaponsTBLViewerModel::getText() const
{
	return text;
}
} // namespace fso::fred::dialogs
