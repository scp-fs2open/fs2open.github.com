//
//

#include <globalincs/pstypes.h>
#include <iff_defs/iff_defs.h>
#include <mission/missionparse.h>
#include <ship/ship.h>
#include <weapon/weapon.h>

#include <QDialog>

#include "util.h"

bool rejectOrCloseHandler(QDialog* dialog,
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
			// Route through the dialog's accept() rather than calling
			// model->apply() directly — the accept flow is what pushes the
			// apply-undo command and cleans up the dialog's undo stack.
			// accept() closes the dialog on success and keeps it open on
			// failure, so the caller must not close it again.
			dialog->accept();
			return false;
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