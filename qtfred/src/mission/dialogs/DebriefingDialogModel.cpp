#include "DebriefingDialogModel.h"
#include "missionui/missioncmdbrief.h" //TODO remove?
#include "mission/missionparse.h" //TODO remove?
#include "gamesnd/eventmusic.h"
#include "sound/audiostr.h"
#include <QMessageBox>

namespace fso::fred::dialogs {

DebriefingDialogModel::DebriefingDialogModel(QObject* parent, EditorViewport* viewport) 
	: AbstractDialogModel(parent, viewport)
{
		initializeData();
}

bool DebriefingDialogModel::apply()
{
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		Debriefings[i] = _wipDebriefing[i];
	}

	Mission_music[SCORE_DEBRIEFING_SUCCESS] = _successMusic;
	Mission_music[SCORE_DEBRIEFING_AVERAGE] = _averageMusic;
	Mission_music[SCORE_DEBRIEFING_FAILURE] = _failureMusic;

	return true;
}

void DebriefingDialogModel::reject() 
{
	stopSpeech();

}

void DebriefingDialogModel::initializeData()
{
	initializeTeamList();
	
	// Make a working copy
	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		_wipDebriefing[i] = Debriefings[i];
	}

	_successMusic = Mission_music[SCORE_DEBRIEFING_SUCCESS];
	_averageMusic = Mission_music[SCORE_DEBRIEFING_AVERAGE];
	_failureMusic = Mission_music[SCORE_DEBRIEFING_FAILURE];

	_currentTeam = 0;
	_currentStage = 0;
}

void DebriefingDialogModel::gotoPreviousStage()
{
	if (_currentStage <= 0) {
		_currentStage = 0;
		return;
	}

	stopSpeech();
	_currentStage--;
}

void DebriefingDialogModel::gotoNextStage()
{
	if (_currentStage >= MAX_DEBRIEF_STAGES - 1) {
		_currentStage = MAX_DEBRIEF_STAGES - 1;
		return;
	}

	if (_currentStage >= _wipDebriefing[_currentTeam].num_stages - 1) {
		_currentStage = _wipDebriefing[_currentTeam].num_stages - 1;
		return;
	}
	
	_currentStage++;
	stopSpeech();
}

void DebriefingDialogModel::addStage()
{
	stopSpeech();

	if (_wipDebriefing[_currentTeam].num_stages >= MAX_DEBRIEF_STAGES) {
		_wipDebriefing[_currentTeam].num_stages = MAX_DEBRIEF_STAGES;
		_currentStage = _wipDebriefing[_currentTeam].num_stages - 1;
		return;
	}

	_wipDebriefing[_currentTeam].num_stages++;
	_currentStage = _wipDebriefing[_currentTeam].num_stages - 1;
	_wipDebriefing[_currentTeam].stages[_currentStage].text = "<Text here>";
	_wipDebriefing[_currentTeam].stages[_currentStage].recommendation_text = "<Recommendation text here>";
	strcpy_s(_wipDebriefing[_currentTeam].stages[_currentStage].voice, "none.wav"); // Really? Seeding with none.wav is gross
	_wipDebriefing[_currentTeam].stages[_currentStage].formula = -1;

	set_modified();
}

// copies the current stage as the next stage and then moves the rest of the stages over.
void DebriefingDialogModel::insertStage()
{
	stopSpeech();

	if (_wipDebriefing[_currentTeam].num_stages >= MAX_DEBRIEF_STAGES) {
		_wipDebriefing[_currentTeam].num_stages = MAX_DEBRIEF_STAGES;
		set_modified();
		return;
	}

	_wipDebriefing[_currentTeam].num_stages++;

	for (int i = _wipDebriefing[_currentTeam].num_stages - 1; i > _currentStage; i--) {
		_wipDebriefing[_currentTeam].stages[i] = _wipDebriefing[_currentTeam].stages[i - 1];
	}

	// Future TODO: Add a QtFRED Option to clear the inserted stage instead of copying the current one.

	set_modified();
}

void DebriefingDialogModel::deleteStage()
{
	stopSpeech();

	// Clear everything if we were on the last stage.
	if (_wipDebriefing[_currentTeam].num_stages <= 1) {
		_wipDebriefing[_currentTeam].num_stages = 0;
		_wipDebriefing[_currentTeam].stages[0].text.clear();
		_wipDebriefing[_currentTeam].stages[0].recommendation_text.clear();
		memset(_wipDebriefing[_currentTeam].stages[0].voice, 0, CF_MAX_FILENAME_LENGTH);
		_wipDebriefing[_currentTeam].stages[0].formula = -1;
		set_modified();
		return;
	}
	
	// copy the stages backwards until we get to the stage we're on
	for (int i = _currentStage; i + 1 < _wipDebriefing[_currentTeam].num_stages; i++) {
		_wipDebriefing[_currentTeam].stages[i] = _wipDebriefing[_currentTeam].stages[i + 1];
	}

	_wipDebriefing[_currentTeam].num_stages--;

	// Clear the tail
	const int tail = _wipDebriefing[_currentTeam].num_stages; // index of the old last element
	_wipDebriefing[_currentTeam].stages[tail].text.clear();
	_wipDebriefing[_currentTeam].stages[tail].recommendation_text.clear();
	std::memset(_wipDebriefing[_currentTeam].stages[tail].voice, 0, CF_MAX_FILENAME_LENGTH);
	_wipDebriefing[_currentTeam].stages[tail].formula = -1;

	// make sure that the current stage is valid.
	if (_wipDebriefing[_currentTeam].num_stages <= _currentStage) {
		_currentStage = _wipDebriefing[_currentTeam].num_stages - 1;
	}

	set_modified();
}

void DebriefingDialogModel::testSpeech() 
{
	// May cause unloading/reloading but it's just the mission editor
	// we don't need to keep all the waves loaded only to have to unload them
	// later anyway. This ensures we have one wave loaded and stopSpeech always unloads it
	
	stopSpeech();

	_waveId = audiostream_open(_wipDebriefing[_currentTeam].stages[_currentStage].voice, ASF_EVENTMUSIC);
	audiostream_play(_waveId, 1.0f, 0);
}

void DebriefingDialogModel::copyToOtherTeams()
{
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		if (i != _currentTeam) {
			_wipDebriefing[i] = _wipDebriefing[_currentTeam];
		}
	}
	set_modified();
}

const SCP_vector<std::pair<SCP_string, int>>& DebriefingDialogModel::getTeamList()
{
	return _teamList;
}

bool DebriefingDialogModel::getMissionIsMultiTeam()
{
	return The_mission.game_type & MISSION_TYPE_MULTI_TEAMS;
}

void DebriefingDialogModel::stopSpeech()
{
	if (_waveId >= -1) {
		audiostream_close_file(_waveId, false);
		_waveId = -1;
	}
}

void DebriefingDialogModel::initializeTeamList()
{
	_teamList.clear();
	for (auto& team : Mission_event_teams_tvt) {
		_teamList.emplace_back(team.first, team.second);
	}
}

int DebriefingDialogModel::getCurrentTeam() const
{
	return _currentTeam;
}

void DebriefingDialogModel::setCurrentTeam(int teamIn)
{
	modify(_currentTeam, teamIn);
};

int DebriefingDialogModel::getCurrentStage() const
{
	return _currentStage;
}

int DebriefingDialogModel::getTotalStages()
{
	return _wipDebriefing[_currentTeam].num_stages;
}

SCP_string DebriefingDialogModel::getStageText() 
{ 
	return _wipDebriefing[_currentTeam].stages[_currentStage].text;
}

void DebriefingDialogModel::setStageText(const SCP_string& text)
{
	modify(_wipDebriefing[_currentTeam].stages[_currentStage].text, text);
}

SCP_string DebriefingDialogModel::getRecommendationText()
{
	return _wipDebriefing[_currentTeam].stages[_currentStage].recommendation_text;
}

void DebriefingDialogModel::setRecommendationText(const SCP_string& text)
{
	modify(_wipDebriefing[_currentTeam].stages[_currentStage].recommendation_text, text);
}

SCP_string DebriefingDialogModel::getSpeechFilename() 
{ 
	return _wipDebriefing[_currentTeam].stages[_currentStage].voice;
}

void DebriefingDialogModel::setSpeechFilename(const SCP_string& speechFilename)
{
	strcpy_s(_wipDebriefing[_currentTeam].stages[_currentStage].voice, speechFilename.c_str());
	set_modified();
}

int DebriefingDialogModel::getFormula() const
{
	return _wipDebriefing[_currentTeam].stages[_currentStage].formula;
}

void DebriefingDialogModel::setFormula(int formula)
{
	modify(_wipDebriefing[_currentTeam].stages[_currentStage].formula, formula);
}

SCP_vector<SCP_string> DebriefingDialogModel::getMusicList()
{
	SCP_vector<SCP_string> music_list;
	music_list.emplace_back("None");

	for (const auto& sm : Spooled_music) {
		music_list.emplace_back(sm.name);
	}

	return music_list;
}

int DebriefingDialogModel::getSuccessMusicTrack() const
{
	return _successMusic;
}
void DebriefingDialogModel::setSuccessMusicTrack(int trackIndex)
{
	modify(_successMusic, trackIndex);
}
int DebriefingDialogModel::getAverageMusicTrack() const
{
	return _averageMusic;
}
void DebriefingDialogModel::setAverageMusicTrack(int trackIndex)
{
	modify(_averageMusic, trackIndex);
}
int DebriefingDialogModel::getFailureMusicTrack() const
{
	return _failureMusic;
}
void DebriefingDialogModel::setFailureMusicTrack(int trackIndex)
{
	modify(_failureMusic, trackIndex);
}

} // namespace fso::fred::dialogs