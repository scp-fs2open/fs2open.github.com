//
//

#include <globalincs/pstypes.h>
#include <iff_defs/iff_defs.h>
#include <mission/missionparse.h>
#include <ship/ship.h>
#include <weapon/weapon.h>

#include "util.h"

bool rejectOrCloseHandler(__UNUSED QDialog* dialog,
	fso::fred::dialogs::AbstractDialogModel* model,
	fso::fred::EditorViewport* viewport)
{
	if (model->query_modified()) {
		auto button = viewport->dialogProvider->showButtonDialog(fso::fred::DialogType::Question,
			"Changes detected",
			"Do you want to keep your changes?",
			{fso::fred::DialogButton::Yes, fso::fred::DialogButton::No, fso::fred::DialogButton::Cancel});

		if (button == fso::fred::DialogButton::Cancel) {
			return false;
		}

		if (button == fso::fred::DialogButton::Yes) {
			return model->apply(); // only close if apply was successful
		}
		if (button == fso::fred::DialogButton::No) {
			model->reject();
			return true;
		}
		return false;
	} else {
		model->reject();
		return true;
	}
}