#pragma once

#include "AbstractDialogModel.h"
#include "mission/util.h"
#include "mission/Editor.h"
#include <globalincs/linklist.h>
#include <ship/ship.h>
#include <playerman/player.h> // for max hotkeys
#include <playerman/managepilot.h> // for squad logos
#include "ui/widgets/sexp_tree.h"

#include "globalincs/pstypes.h"
#include <utility>
#include <QObject>

namespace fso::fred::dialogs {

	//TODO: This dialog currently works on the wing data directly instead of model members
	// so it does not support temporary changes. This will need to be changed in a future PR

/**
 * @brief QTFred's Wing Editor's Model
 */
class WingEditorDialogModel : public AbstractDialogModel {
		Q_OBJECT

	public:
		WingEditorDialogModel(QObject* parent, EditorViewport* viewport);
	
		// The model in this dialog directly applies changes to the mission, so apply and reject are superfluous	
		bool apply() override { return true; }
		void reject() override {}

		int getCurrentWingIndex() const { return _currentWingIndex; };

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

		std::pair<int, SCP_vector<SCP_string>> getLeaderList() const;
		static std::vector<std::pair<int, std::string>> getHotkeyList();
		static std::vector<std::pair<int, std::string>> getFormationList();
		static std::vector<std::pair<int, std::string>> getArrivalLocationList();
		static std::vector<std::pair<int, std::string>> getDepartureLocationList();
		std::vector<std::pair<int, std::string>> getArrivalTargetList() const;
		std::vector<std::pair<int, std::string>> getDepartureTargetList() const;
		std::vector<std::string> getSquadLogoList() const { return squadLogoList; };

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
		SCP_string getSquadLogo() const;
		void setSquadLogo(const SCP_string& filename);

		// Top section, third column
		void selectPreviousWing();
		void selectNextWing();
		void deleteCurrentWing();
		void disbandCurrentWing();
		// Initial orders is handled by its own dialog, so no model function here
		std::vector<std::pair<SCP_string, bool>> getWingFlags() const;
		void setWingFlags(const std::vector<std::pair<SCP_string, bool>>& newFlags);
		

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
		std::vector<std::pair<SCP_string, bool>> getArrivalPaths() const;
		void setArrivalPaths(const std::vector<std::pair<SCP_string, bool>>& newFlags);
		int getArrivalTree() const;
		void setArrivalTree(int newTree);
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
		std::vector<std::pair<SCP_string, bool>> getDeparturePaths() const;
		void setDeparturePaths(const std::vector<std::pair<SCP_string, bool>>& newFlags);
		int getDepartureTree() const;
		void setDepartureTree(int newTree);
		bool getNoDepartureWarpFlag() const;
		void setNoDepartureWarpFlag(bool flagIn);
		bool getNoDepartureWarpAdjustFlag() const;
		void setNoDepartureWarpAdjustFlag(bool flagIn);

	signals:
		void wingChanged();

	private slots:
		void onEditorSelectionChanged(int); // currentObjectChanged
		void onEditorMissionChanged(); // missionChanged

	private: // NOLINT(readability-redundant-access-specifiers)
		void reloadFromCurWing();
		wing* getCurrentWing() const;
		static std::vector<std::pair<SCP_string, bool>> getDockBayPathsForWingMask(uint32_t mask, int anchorShipnum);
		void prepareSquadLogoList();

		int _currentWingIndex = -1;
		SCP_string _currentWingName;

		SCP_vector<SCP_string> squadLogoList;
};
} // namespace fso::fred::dialogs