#pragma once

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {
class ShipTBLViewerModel : public AbstractDialogModel {
  private:
	SCP_string text;

  public:
	ShipTBLViewerModel(QObject* parent, EditorViewport* viewport, int sc);
	bool apply() override;
	void reject() override;
	void initializeData(const int ship_class);

	SCP_string getText() const;
};
} // namespace dialogs
} // namespace fred
} // namespace fso