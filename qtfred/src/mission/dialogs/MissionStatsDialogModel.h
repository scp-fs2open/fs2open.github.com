#pragma once

#include <mission/EditorViewport.h>
#include "AbstractDialogModel.h"

#include <globalincs/pstypes.h>

namespace fso::fred::dialogs {

class MissionStatsDialogModel : public AbstractDialogModel {
	Q_OBJECT

public:
	struct IFFShipCounts {
		int fighter = 0;
		int bomber  = 0;
		int capital = 0;
		int other   = 0;
		int total() const { return fighter + bomber + capital + other; }
	};

	struct EscortEntry {
		SCP_string name;
		int priority = 0;
	};

	MissionStatsDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	// Summary counts
	static int getNumObjects();
	static int getMaxObjects();
	static int getShipCount();
	static int getMaxShips();
	static int getPropCount();
	static int getWingCount();
	static int getWaypointPathCount();
	static int getJumpNodeCount();
	static int getMessageCount();
	static int getEventCount();
	static int getGoalCount();
	static int getVariableCount();
	static int getContainerCount();

	static SCP_vector<std::pair<SCP_string, IFFShipCounts>> getShipsByIFF();

	static SCP_vector<EscortEntry> getEscortList();

	static SCP_map<int, SCP_vector<SCP_string>> getHotkeyMap();
};

} // namespace fso::fred::dialogs
