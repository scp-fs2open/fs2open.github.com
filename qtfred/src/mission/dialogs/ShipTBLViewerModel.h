#pragma once

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {
class ShipTBLViewerModel : public AbstractDialogModel {
  private:
	void initializeData();
	SCP_string text;
	SCP_vector<SCP_string> tbl_file_names;
	int ship_class;

  public:
	ShipTBLViewerModel(QObject* parent, EditorViewport* viewport, int sc);
	bool apply() override;
	void reject() override;

	SCP_string getText() const;
};
} // namespace dialogs
} // namespace fred
} // namespace fso