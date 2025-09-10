#include "BriefingEditorDialogModel.h"

#include "gamesnd/eventmusic.h"
#include "mission/missionparse.h"      //TODO remove?
#include "missionui/missioncmdbrief.h" //TODO remove?
#include "sound/audiostr.h"

#include <QMessageBox>

namespace fso::fred::dialogs {

BriefingEditorDialogModel::BriefingEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

bool BriefingEditorDialogModel::apply()
{
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		Briefings[i] = _wipBriefings[i];
	}

	return true;
}

void BriefingEditorDialogModel::reject()
{
	stopSpeech();
}

void BriefingEditorDialogModel::initializeData()
{
	initializeTeamList();

	// Make a working copy
	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		_wipBriefings[i] = Briefings[i];
	}

	_currentTeam = 0;
	_currentStage = 0;
	_currentIcon = 0;
}

void BriefingEditorDialogModel::gotoPreviousStage()
{
	if (_currentStage <= 0) {
		_currentStage = 0;
		return;
	}

	stopSpeech();
	_currentStage--;
}

void BriefingEditorDialogModel::gotoNextStage()
{
	if (_currentStage >= MAX_BRIEF_STAGES - 1) {
		_currentStage = MAX_BRIEF_STAGES - 1;
		return;
	}

	if (_currentStage >= _wipBriefings[_currentTeam].num_stages - 1) {
		_currentStage = _wipBriefings[_currentTeam].num_stages - 1;
		return;
	}

	_currentStage++;
	stopSpeech();
}

void BriefingEditorDialogModel::addStage()
{
	stopSpeech();

	if (_wipBriefings[_currentTeam].num_stages >= MAX_BRIEF_STAGES) {
		_wipBriefings[_currentTeam].num_stages = MAX_BRIEF_STAGES;
		_currentStage = _wipBriefings[_currentTeam].num_stages - 1;
		return;
	}

	_wipBriefings[_currentTeam].num_stages++;
	_currentStage = _wipBriefings[_currentTeam].num_stages - 1;
	_wipBriefings[_currentTeam].stages[_currentStage].text = "<Text here>";
	strcpy_s(_wipBriefings[_currentTeam].stages[_currentStage].voice,
		"none.wav"); // Really? Seeding with none.wav is gross
	_wipBriefings[_currentTeam].stages[_currentStage].formula = -1;

	set_modified();
}

// copies the current stage as the next stage and then moves the rest of the stages over.
void BriefingEditorDialogModel::insertStage()
{
	stopSpeech();

	if (_wipBriefings[_currentTeam].num_stages >= MAX_BRIEF_STAGES) {
		_wipBriefings[_currentTeam].num_stages = MAX_BRIEF_STAGES;
		set_modified();
		return;
	}

	_wipBriefings[_currentTeam].num_stages++;

	for (int i = _wipBriefings[_currentTeam].num_stages - 1; i > _currentStage; i--) {
		_wipBriefings[_currentTeam].stages[i] = _wipBriefings[_currentTeam].stages[i - 1];
	}

	// Future TODO: Add a QtFRED Option to clear the inserted stage instead of copying the current one.

	set_modified();
}

void BriefingEditorDialogModel::deleteStage()
{
	stopSpeech();

	// Clear everything if we were on the last stage.
	if (_wipBriefings[_currentTeam].num_stages <= 1) {
		_wipBriefings[_currentTeam].num_stages = 0;
		_wipBriefings[_currentTeam].stages[0].text.clear();
		memset(_wipBriefings[_currentTeam].stages[0].voice, 0, CF_MAX_FILENAME_LENGTH);
		_wipBriefings[_currentTeam].stages[0].formula = -1;
		set_modified();
		return;
	}

	// copy the stages backwards until we get to the stage we're on
	for (int i = _currentStage; i + 1 < _wipBriefings[_currentTeam].num_stages; i++) {
		_wipBriefings[_currentTeam].stages[i] = _wipBriefings[_currentTeam].stages[i + 1];
	}

	_wipBriefings[_currentTeam].num_stages--;

	// Clear the tail
	const int tail = _wipBriefings[_currentTeam].num_stages; // index of the old last element
	_wipBriefings[_currentTeam].stages[tail].text.clear();
	std::memset(_wipBriefings[_currentTeam].stages[tail].voice, 0, CF_MAX_FILENAME_LENGTH);
	_wipBriefings[_currentTeam].stages[tail].formula = -1;

	// make sure that the current stage is valid.
	if (_wipBriefings[_currentTeam].num_stages <= _currentStage) {
		_currentStage = _wipBriefings[_currentTeam].num_stages - 1;
	}

	set_modified();
}

void BriefingEditorDialogModel::testSpeech()
{
	// May cause unloading/reloading but it's just the mission editor
	// we don't need to keep all the waves loaded only to have to unload them
	// later anyway. This ensures we have one wave loaded and stopSpeech always unloads it

	stopSpeech();

	_waveId = audiostream_open(_wipBriefings[_currentTeam].stages[_currentStage].voice, ASF_EVENTMUSIC);
	audiostream_play(_waveId, 1.0f, 0);
}

void BriefingEditorDialogModel::copyToOtherTeams()
{
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		if (i != _currentTeam) {
			_wipBriefings[i] = _wipBriefings[_currentTeam];
		}
	}
	set_modified();
}

const SCP_vector<std::pair<SCP_string, int>>& BriefingEditorDialogModel::getTeamList()
{
	return _teamList;
}

bool BriefingEditorDialogModel::getMissionIsMultiTeam()
{
	return The_mission.game_type & MISSION_TYPE_MULTI_TEAMS;
}

void BriefingEditorDialogModel::stopSpeech()
{
	if (_waveId >= -1) {
		audiostream_close_file(_waveId, false);
		_waveId = -1;
	}
}

void BriefingEditorDialogModel::initializeTeamList()
{
	_teamList.clear();
	for (auto& team : Mission_event_teams_tvt) {
		_teamList.emplace_back(team.first, team.second);
	}
}

int BriefingEditorDialogModel::getCurrentTeam() const
{
	return _currentTeam;
}

void BriefingEditorDialogModel::setCurrentTeam(int teamIn)
{
	modify(_currentTeam, teamIn);
};

int BriefingEditorDialogModel::getCurrentStage() const
{
	return _currentStage;
}

int BriefingEditorDialogModel::getTotalStages()
{
	return _wipBriefings[_currentTeam].num_stages;
}

SCP_string BriefingEditorDialogModel::getStageText()
{
	return _wipBriefings[_currentTeam].stages[_currentStage].text;
}

void BriefingEditorDialogModel::setStageText(const SCP_string& text)
{
	modify(_wipBriefings[_currentTeam].stages[_currentStage].text, text);
}

SCP_string BriefingEditorDialogModel::getSpeechFilename()
{
	return _wipBriefings[_currentTeam].stages[_currentStage].voice;
}

void BriefingEditorDialogModel::setSpeechFilename(const SCP_string& speechFilename)
{
	strcpy_s(_wipBriefings[_currentTeam].stages[_currentStage].voice, speechFilename.c_str());
	set_modified();
}

int BriefingEditorDialogModel::getFormula() const
{
	return _wipBriefings[_currentTeam].stages[_currentStage].formula;
}

void BriefingEditorDialogModel::setFormula(int formula)
{
	modify(_wipBriefings[_currentTeam].stages[_currentStage].formula, formula);
}

SCP_vector<SCP_string> BriefingEditorDialogModel::getMusicList()
{
	SCP_vector<SCP_string> music_list;
	music_list.emplace_back("None");

	for (const auto& sm : Spooled_music) {
		music_list.emplace_back(sm.name);
	}

	return music_list;
}

} // namespace fso::fred::dialogs