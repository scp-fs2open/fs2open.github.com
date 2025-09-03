#pragma once

#include "AbstractDialogModel.h"

namespace fso::fred::dialogs {
class MusicTBLViewerModel : public AbstractDialogModel {
  private:
	SCP_string text;

  public:
	MusicTBLViewerModel(QObject* parent, EditorViewport* viewport);
	bool apply() override;
	void reject() override;
	void initializeData();

	SCP_string getText() const;
};
} // namespace fso::fred::dialogs