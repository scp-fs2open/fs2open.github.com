#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso::fred::dialogs {

class GlobalShipFlagsDialogModel : public AbstractDialogModel {
	Q_OBJECT

  public:
	GlobalShipFlagsDialogModel(QObject* parent, EditorViewport* viewport);
	~GlobalShipFlagsDialogModel() override = default;

	bool apply() override;
	void reject() override;

	void setNoShieldsAll();

	void setNoSubspaceDriveOnFightersBombers();

	void setPrimitiveSensorsOnFightersBombers();

	void setAffectedByGravityOnFightersBombers();
};

} // namespace fso::fred::dialogs
