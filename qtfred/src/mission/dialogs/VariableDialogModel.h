#pragma once

#include "globalincs/pstypes.h"

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class VariableDialogModel : public AbstractDialogModel {
  public:
	VariableDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;
};

} // namespace dialogs
} // namespace fred
} // namespace fso
