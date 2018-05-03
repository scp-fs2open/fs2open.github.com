#include <ship/ship.h>
#include <mission/missionparse.h>
#include <iff_defs/iff_defs.h>
#include "mission/dialogs/ShieldSystemDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

ShieldSystemDialogModel::ShieldSystemDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport), _teams(MAX_IFFS, 0), _types(MAX_SHIP_CLASSES, 0) {

	initializeData();
}

void ShieldSystemDialogModel::initializeData() {
	_currTeam = 0;
	_currType = 0;

	_editor->normalizeShieldSysData();

	_editor->exportShieldSysData(_teams, _types);

	for (const auto& info : Ship_info) {
		_shipTypeOptions.emplace_back(info.name);
	}

	for (int i = 0; i < Num_iffs; i++) {
		_teamOptions.emplace_back(Iff_info[i].iff_name);
	}

	modelChanged();
}

bool ShieldSystemDialogModel::apply() {
	if (query_modified()) {
		_editor->importShieldSysData(_teams, _types);
	}
	return true;
}
void ShieldSystemDialogModel::reject() {
	// nothing to do
}

bool ShieldSystemDialogModel::query_modified() const {
	return !_editor->compareShieldSysData(_teams, _types);
}

}
}
}
