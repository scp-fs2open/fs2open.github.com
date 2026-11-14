#pragma once
#include <qevent.h>
#include "dialogs/AbstractDialogModel.h"
#include "EditorViewport.h"

// Shared reject/close flow for dialogs with a "keep your changes?" prompt.
// Returns true if the caller should proceed to close the dialog (No / not
// modified). Returns false if the caller must NOT close it: either the user
// canceled, or they answered Yes — in which case dialog->accept() has been
// invoked and the accept flow owns closing.
bool rejectOrCloseHandler(QDialog* dialog,
	fso::fred::dialogs::AbstractDialogModel* model,
	fso::fred::EditorViewport* viewport);