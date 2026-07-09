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

	QByteArray captureState() const override;
	void restoreState(const QByteArray& state) override;

	// Working-state serialization for the in-dialog undo stack: the WIP
	// briefings plus the current team/stage context.
	QByteArray captureWorkingState() const;
	void restoreWorkingState(const QByteArray& state);

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

	// Index-addressed setters for undo commands: undo must target the stage
	// that was edited even if the team/stage context has moved since.
	void setBriefingTextAt(int team, int stage, const SCP_string& text);
	void setAnimationFilenameAt(int team, int stage, const SCP_string& name);
	void setSpeechFilenameAt(int team, int stage, const SCP_string& name);
	void setLowResolutionFilenameAt(int team, const SCP_string& name);
	void setHighResolutionFilenameAt(int team, const SCP_string& name);

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
	int _waveId = -1;
	SCP_vector<std::pair<SCP_string, int>> _teamList;
};


} // namespace fso::fred::dialogs
