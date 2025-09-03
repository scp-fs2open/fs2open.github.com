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

	static void setNoShieldsAll();

	static void setNoSubspaceDriveOnFightersBombers();

	static void setPrimitiveSensorsOnFightersBombers();

	static void setAffectedByGravityOnFightersBombers();
};

} // namespace fso::fred::dialogs
