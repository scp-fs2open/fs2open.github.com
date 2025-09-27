#pragma once

#include "AbstractDialogModel.h"
#include "globalincs/pstypes.h"
#include "missionui/missioncmdbrief.h" //TODO remove this?
#include "mission/missionbriefcommon.h"


namespace fso::fred::dialogs {


class DebriefingDialogModel: public AbstractDialogModel {
 public:

	DebriefingDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	int getCurrentTeam() const;
	void setCurrentTeam(int teamIn);
	int getCurrentStage() const;
	int getTotalStages();

	SCP_string getStageText();
	void setStageText(const SCP_string& text);
	SCP_string getRecommendationText();
	void setRecommendationText(const SCP_string& text);
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
	int getSuccessMusicTrack() const;
	void setSuccessMusicTrack(int trackIndex);
	int getAverageMusicTrack() const;
	void setAverageMusicTrack(int trackIndex);
	int getFailureMusicTrack() const;
	void setFailureMusicTrack(int trackIndex);

 private:
	void initializeData();
	void stopSpeech();
	void initializeTeamList();

	debriefing _wipDebriefing[MAX_TVT_TEAMS];
	int _successMusic;
	int _averageMusic;
	int _failureMusic;

	int _currentTeam;
	int _currentStage;
	int _waveId;
	SCP_vector<std::pair<SCP_string, int>> _teamList;
};


} // namespace fso::fred::dialogs
