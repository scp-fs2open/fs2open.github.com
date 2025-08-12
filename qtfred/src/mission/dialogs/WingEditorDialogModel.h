#pragma once

#include "AbstractDialogModel.h"
#include "mission/util.h"
#include "mission/Editor.h"
#include <globalincs/linklist.h>
#include <ship/ship.h>
#include <playerman/player.h> // for max hotkeys
#include "ui/widgets/sexp_tree.h"

#include "globalincs/pstypes.h"
#include <utility>
#include <QObject>

namespace fso::fred::dialogs {

/**
 * @brief QTFred's Wing Editor's Model
 */
class WingEditorDialogModel : public AbstractDialogModel {
		Q_OBJECT

	public:
		WingEditorDialogModel(QObject* parent, EditorViewport* viewport);
	
		// The model in this dialog directly applies changes to the mission, so apply and reject are superfluous	
		bool apply() { return true; }
		void reject() {}

		bool wingIsValid() const;

		bool isPlayerWing() const;
		bool containsPlayerStart() const;
		bool wingAllFighterBombers() const;

		bool arrivalIsDockBay() const;
		bool arrivalNeedsTarget() const;
		bool departureIsDockBay() const;
		bool departureNeedsTarget() const;
		int getMaxWaveThreshold() const;
		int getMinArrivalDistance() const;

		SCP_vector<SCP_string> getCurrentSelectableWings(); //unused?

		std::pair<int, SCP_vector<SCP_string>> getLeaderList(); //used
		std::vector<std::pair<int, std::string>> getHotkeyList() const;
		std::vector<std::pair<int, std::string>> getFormationList() const;
		std::vector<std::pair<int, std::string>> getArrivalLocationList() const;
		std::vector<std::pair<int, std::string>> getDepartureLocationList() const;
		std::vector<std::pair<int, std::string>> getArrivalTargetList() const;
		std::vector<std::pair<int, std::string>> getDepartureTargetList() const;

		// Top section, first column
		SCP_string getWingName() const;
		void setWingName(const SCP_string& name);
		int getWingLeaderIndex() const;
		void setWingLeaderIndex(int newLeaderIndex);
		int getNumberOfWaves() const;
		void setNumberOfWaves(int newTotalWaves);
		int getWaveThreshold() const;
		void setWaveThreshold(int newThreshhold);
		int getHotkey() const;
		void setHotkey(int newHotkeyIndex);

		// Top section, second column
		int getFormationId() const;
		void setFormationId(int newFormationId);
		float getFormationScale() const;
		void setFormationScale(float newScale);
		void alignWingFormation();
		void showWingFlagsDialog();

		// Top section, third column
		void selectPreviousWing();
		void selectNextWing();
		void deleteCurrentWing();
		void disbandCurrentWing();

		// Second section
		SCP_string getSquadronLogo() const;

		// Arrival controls
		ArrivalLocation getArrivalType() const;
		void setArrivalType(ArrivalLocation arrivalType);
		int getArrivalDelay() const;
		void setArrivalDelay(int delayIn);
		int getMinWaveDelay() const;
		void setMinWaveDelay(int newMin);
		int getMaxWaveDelay() const;
		void setMaxWaveDelay(int newMax);
		int getArrivalTarget() const;
		void setArrivalTarget(int targetIndex);
		int getArrivalDistance() const;
		void setArrivalDistance(int newDistance);
		// void setArrivalPaths(SCP_vector<std::pair<int, bool>> pathStatusIn);
		// void setCustomWarpIn(int warpIn);
		int getArrivalTree() const;
		void setArrivalTree(int /*oldTree*/, int newTree);
		bool getNoArrivalWarpFlag() const;
		void setNoArrivalWarpFlag(bool flagIn);
		bool getNoArrivalWarpAdjustFlag() const;
		void setNoArrivalWarpAdjustFlag(bool flagIn);

		// Departure controls
		DepartureLocation getDepartureType() const;
		void setDepartureType(DepartureLocation departureType);
		int getDepartureDelay() const;
		void setDepartureDelay(int delayIn);
		int getDepartureTarget() const;
		void setDepartureTarget(int targetIndex);
		// void setDeparturePaths(SCP_vector<std::pair<int, bool>> pathStatusIn);
		// void setCustomWarpOut(int warpOut);
		int getDepartureTree() const;
		void setDepartureTree(int /*oldTree*/, int newTree);
		bool getNoDepartureWarpFlag() const;
		void setNoDepartureWarpFlag(bool flagIn);
		bool getNoDepartureWarpAdjustFlag() const;
		void setNoDepartureWarpAdjustFlag(bool flagIn);

		SCP_string setSquadLogo(SCP_string filename);

		bool getReinforcementFlag();
		bool getCountingGoalsFlag();
		bool getArrivalMusicFlag();
		bool getArrivalMessageFlag();
		bool getFirstWaveMessageFlag();
		bool getDynamicGoalsFlag();

		bool setReinforcementFlag(bool flagIn);
		bool setCountingGoalsFlag(bool flagIn);
		bool setArrivalMusicFlag(bool flagIn);
		bool setArrivalMessageFlag(bool flagIn);
		bool setFirstWaveMessageFlag(bool flagIn);
		bool setDynamicGoalsFlag(bool flagIn);

		SCP_vector<std::pair<SCP_string, bool>> getArrivalPathStatus();
		SCP_vector<std::pair<SCP_string, bool>> getDeparturePathStatus();
		bool setArrivalPath(std::pair<int, bool> pathStatusIn);
		bool resetArrivalPaths();
		bool resetDeparturePaths();
		bool setDeparturePath(std::pair<int, bool> pathStatusIn);

	signals:
		void wingChanged();

	private slots:
		void onEditorSelectionChanged(int); // currentObjectChanged
	private:
		void reloadFromCurWing();
		wing* getCurrentWing() const;

		int _currentWingIndex = -1;
		SCP_string _currentWingName;
};
} // namespace fso::fred::dialogs