#pragma once

#include "../AbstractDialogModel.h"

namespace fso::fred::dialogs {
class WeaponsTBLViewerModel : public AbstractDialogModel {
  private:
	SCP_string text;

  public:
	WeaponsTBLViewerModel(QObject* parent, EditorViewport* viewport, int wc);
	bool apply() override;
	void reject() override;
	void initializeData(const int ship_class);

	SCP_string getText() const;
};
} // namespace fso::fred::dialogs