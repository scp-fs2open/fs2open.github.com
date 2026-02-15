#pragma once
#include <qevent.h>
#include "dialogs/AbstractDialogModel.h"
#include "EditorViewport.h"

bool rejectOrCloseHandler(QDialog* dialog,
	fso::fred::dialogs::AbstractDialogModel* model,
	fso::fred::EditorViewport* viewport);