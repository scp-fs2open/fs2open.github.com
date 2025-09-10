#pragma once

#include "globalincs/pstypes.h"

#include "AbstractDialogModel.h"

#include "mission/missionbriefcommon.h"
#include "missionui/missioncmdbrief.h" //TODO remove this?

namespace fso::fred::dialogs {

class BriefingEditorDialogModel : public AbstractDialogModel {
  public:
	BriefingEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	int getCurrentTeam() const;
	void setCurrentTeam(int teamIn);
	int getCurrentStage() const;
	int getTotalStages();

	SCP_string getStageText();
	void setStageText(const SCP_string& text);
	SCP_string getSpeechFilename();
	void setSpeechFilename(const SCP_string& speechFilename);
	int getFormula() const;
	void setFormula(int formula);

	void gotoPreviousStage();
	void gotoNextStage();
	void addStage();
	void insertStage();
	void deleteStage();
	void testSpeech();
	void copyToOtherTeams();
	const SCP_vector<std::pair<SCP_string, int>>& getTeamList();
	static bool getMissionIsMultiTeam();

	static SCP_vector<SCP_string> getMusicList();

  private:
	void initializeData();
	void stopSpeech();
	void initializeTeamList();

	briefing _wipBriefings[MAX_TVT_TEAMS];

	int _currentTeam;
	int _currentStage;
	int _currentIcon;
	int _waveId;
	SCP_vector<std::pair<SCP_string, int>> _teamList;
};

} // namespace fso::fred::dialogs
