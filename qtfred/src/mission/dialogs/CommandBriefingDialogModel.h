#pragma once

#include "AbstractDialogModel.h"
#include "globalincs/pstypes.h"
#include "missionui/missioncmdbrief.h"


namespace fso::fred::dialogs {


class CommandBriefingDialogModel: public AbstractDialogModel {
 public:

	CommandBriefingDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	int getCurrentTeam() const;
	void setCurrentTeam(int teamIn);
	int getCurrentStage() const;
	int getTotalStages();

	SCP_string getBriefingText();
	void setBriefingText(const SCP_string& briefingText);
	SCP_string getAnimationFilename();
	void setAnimationFilename(const SCP_string& animationFilename);
	SCP_string getSpeechFilename();
	void setSpeechFilename(const SCP_string& speechFilename);
	SCP_string getLowResolutionFilename();
	void setLowResolutionFilename(const SCP_string& lowResolutionFilename);
	SCP_string getHighResolutionFilename();
	void setHighResolutionFilename(const SCP_string& highResolutionFilename);
	
	void gotoPreviousStage();
	void gotoNextStage();
	void addStage();
	void insertStage();
	void deleteStage();
	void testSpeech();
	void copyToOtherTeams();
	const SCP_vector<std::pair<SCP_string, int>>& getTeamList();
	static bool getMissionIsMultiTeam();

 private:
	void initializeData();
	void stopSpeech();
	void initializeTeamList();

	cmd_brief _wipCommandBrief[MAX_TVT_TEAMS];
	int _currentTeam;
	int _currentStage;
	int _waveId;
	SCP_vector<std::pair<SCP_string, int>> _teamList;
};


} // namespace fso::fred::dialogs
