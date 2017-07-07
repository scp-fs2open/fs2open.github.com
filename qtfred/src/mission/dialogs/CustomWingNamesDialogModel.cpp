#include "CustomWingNamesDialogModel.h"

#include "ship/ship.h"

namespace fso {
namespace fred {
namespace dialogs {

CustomWingNamesDialogModel::CustomWingNamesDialogModel(QObject * parent, EditorViewport * viewport) :
	AbstractDialogModel(parent, viewport) {
	initializeData();
}

bool CustomWingNamesDialogModel::apply() {
	return true;
}

void CustomWingNamesDialogModel::reject() {
}

void CustomWingNamesDialogModel::initializeData() {
	int i;

	// init starting wings
	for (i = 0; i < MAX_STARTING_WINGS; i++) {
		_m_starting[i] = Starting_wing_names[i];
	}

	// init squadron wings
	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		_m_squadron[i] = Squadron_wing_names[i];
	}

	// init tvt wings
	for (i = 0; i < MAX_TVT_WINGS; i++) {
		_m_tvt[i] = TVT_wing_names[i];
	}
}

}
}
}