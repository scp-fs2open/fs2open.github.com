#pragma once

#include "AbstractDialogModel.h"
#include "globalincs/pstypes.h"
#include "mission/missionbriefcommon.h"
#include "ui/widgets/sexp_tree_view.h"


namespace fso::fred::dialogs {


class DebriefingDialogModel: public AbstractDialogModel {
 public:

	DebriefingDialogModel(QObject* parent, EditorViewport* viewport);
	~DebriefingDialogModel() override;

	bool apply() override;
	void reject() override;

	QByteArray captureState() const override;
	void restoreState(const QByteArray& state) override;

	// Serialize/restore the dialog's WORKING state (_wipDebriefing, music,
	// current team/stage) for the in-dialog undo stack. Unlike captureState(),
	// which snapshots the live globals for the main stack's ApplyDialogCommand,
	// these never touch mission data.
	QByteArray captureWorkingState() const;
	void restoreWorkingState(const QByteArray& state);

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

	// Indexed setters for undo commands: a command captured on one stage must
	// still restore that stage after the user navigates elsewhere.
	void setStageTextAt(int team, int stage, const SCP_string& text);
	void setRecommendationTextAt(int team, int stage, const SCP_string& text);
	void setSpeechFilenameAt(int team, int stage, const SCP_string& speechFilename);

	void setTreeControl(sexp_tree_view* tree) { _sexpTree = tree; }
	void setModified() { set_modified(); }
	void commitCurrentFormula();

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
	sexp_tree_view* _sexpTree = nullptr;
	int _successMusic;
	int _averageMusic;
	int _failureMusic;

	int _currentTeam;
	int _currentStage;
	int _waveId = -1;
	SCP_vector<std::pair<SCP_string, int>> _teamList;
};


} // namespace fso::fred::dialogs
