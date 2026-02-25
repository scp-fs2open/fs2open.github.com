#pragma once

#include "mission/dialogs/AbstractDialogModel.h"
#include "mission/dialogs/MissionSpecDialogModel.h"

namespace fso::fred::dialogs {

class SupportRearmDialogModel : public AbstractDialogModel {
  public:
	SupportRearmDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	void setInitial(const SupportRearmSettings& settings);
	SupportRearmSettings& settings();
	const SupportRearmSettings& settings() const;

	const SCP_vector<int>& visibleWeaponClasses() const;

  private:
	SupportRearmSettings _settings;
	SCP_vector<int> _visibleWeaponClasses;
};

} // namespace fso::fred::dialogs
