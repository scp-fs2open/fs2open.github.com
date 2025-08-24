#include "CommandBriefingDialogModel.h"
#include "missionui/missioncmdbrief.h"
#include "sound/audiostr.h"
#include <QMessageBox>

namespace fso::fred::dialogs {

CommandBriefingDialogModel::CommandBriefingDialogModel(QObject* parent, EditorViewport* viewport) 
	: AbstractDialogModel(parent, viewport)
{
		initializeData();
}

bool CommandBriefingDialogModel::apply()
{
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		Cmd_briefs[i] = _wipCommandBrief[i];
	}

	return true;
}

void CommandBriefingDialogModel::reject() 
{
	stopSpeech();

}

void CommandBriefingDialogModel::initializeData()
{
	initializeTeamList();
	
	// Make a working copy
	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		_wipCommandBrief[i] = Cmd_briefs[i];
	}

	_currentTeam = 0;  // default to the first team
	_currentStage = 0; // default to the first stage
}

void CommandBriefingDialogModel::gotoPreviousStage()
{
	if (_currentStage <= 0) {
		_currentStage = 0;
		return;
	}

	stopSpeech();
	_currentStage--;
}

void CommandBriefingDialogModel::gotoNextStage()
{
	if (_currentStage >= CMD_BRIEF_STAGES_MAX - 1) {
		_currentStage = CMD_BRIEF_STAGES_MAX - 1;
		return;
	}

	if (_currentStage >= _wipCommandBrief[_currentTeam].num_stages - 1) {
		_currentStage = _wipCommandBrief[_currentTeam].num_stages - 1;
		return;
	}
	
	_currentStage++;
	stopSpeech();
}

void CommandBriefingDialogModel::addStage()
{
	stopSpeech();

	if (_wipCommandBrief[_currentTeam].num_stages >= CMD_BRIEF_STAGES_MAX) {
		_wipCommandBrief[_currentTeam].num_stages = CMD_BRIEF_STAGES_MAX;
		_currentStage = _wipCommandBrief[_currentTeam].num_stages - 1;
		return;
	}

	_wipCommandBrief[_currentTeam].num_stages++;
	_currentStage = _wipCommandBrief[_currentTeam].num_stages - 1;
	_wipCommandBrief[_currentTeam].stage[_currentStage].text = "<Text here>";
	set_modified();
}

// copies the current stage as the next stage and then moves the rest of the stages over.
void CommandBriefingDialogModel::insertStage()
{
	stopSpeech();

	if (_wipCommandBrief[_currentTeam].num_stages >= CMD_BRIEF_STAGES_MAX) {
		_wipCommandBrief[_currentTeam].num_stages = CMD_BRIEF_STAGES_MAX;
		set_modified();
		return;
	}

	_wipCommandBrief[_currentTeam].num_stages++;

	for (int i = _wipCommandBrief[_currentTeam].num_stages - 1; i > _currentStage; i--) {
		_wipCommandBrief[_currentTeam].stage[i] = _wipCommandBrief[_currentTeam].stage[i - 1];
	}

	// Future TODO: Add a QtFRED Option to clear the inserted stage instead of copying the current one.

	set_modified();
}

void CommandBriefingDialogModel::deleteStage()
{
	stopSpeech();

	// Clear everything if we were on the last stage.
	if (_wipCommandBrief[_currentTeam].num_stages <= 1) {
		_wipCommandBrief[_currentTeam].num_stages = 0;
		_wipCommandBrief[_currentTeam].stage[0].text.clear();
		_wipCommandBrief[_currentTeam].stage[0].wave = -1;
		memset(_wipCommandBrief[_currentTeam].stage[0].wave_filename, 0, CF_MAX_FILENAME_LENGTH);
		memset(_wipCommandBrief[_currentTeam].stage[0].ani_filename, 0, CF_MAX_FILENAME_LENGTH);
		set_modified();
		return;
	}
	
	// copy the stages backwards until we get to the stage we're on
	for (int i = _currentStage; i + 1 < _wipCommandBrief[_currentTeam].num_stages; i++) {
		_wipCommandBrief[_currentTeam].stage[i] = _wipCommandBrief[_currentTeam].stage[i + 1];
	}

	_wipCommandBrief[_currentTeam].num_stages--;

	// Clear the tail
	const int tail = _wipCommandBrief[_currentTeam].num_stages; // index of the old last element
	_wipCommandBrief[_currentTeam].stage[tail].text.clear();
	_wipCommandBrief[_currentTeam].stage[tail].wave = -1;
	std::memset(_wipCommandBrief[_currentTeam].stage[tail].wave_filename, 0, CF_MAX_FILENAME_LENGTH);
	std::memset(_wipCommandBrief[_currentTeam].stage[tail].ani_filename, 0, CF_MAX_FILENAME_LENGTH);

	// make sure that the current stage is valid.
	if (_wipCommandBrief[_currentTeam].num_stages <= _currentStage) {
		_currentStage = _wipCommandBrief[_currentTeam].num_stages - 1;
	}

	set_modified();
}

void CommandBriefingDialogModel::testSpeech() 
{
	// May cause unloading/reloading but it's just the mission editor
	// we don't need to keep all the waves loaded only to have to unload them
	// later anyway. This ensures we have one wave loaded and stopSpeech always unloads it
	
	stopSpeech();

	_waveId = audiostream_open(_wipCommandBrief[_currentTeam].stage[_currentStage].wave_filename, ASF_EVENTMUSIC);
	audiostream_play(_waveId, 1.0f, 0);
}

void CommandBriefingDialogModel::copyToOtherTeams()
{
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		if (i != _currentTeam) {
			_wipCommandBrief[i] = _wipCommandBrief[_currentTeam];
		}
	}
	set_modified();
}

const SCP_vector<std::pair<SCP_string, int>>& CommandBriefingDialogModel::getTeamList()
{
	return _teamList;
}

bool CommandBriefingDialogModel::getMissionIsMultiTeam()
{
	return The_mission.game_type & MISSION_TYPE_MULTI_TEAMS;
}

void CommandBriefingDialogModel::stopSpeech()
{
	if (_waveId >= -1) {
		audiostream_close_file(_waveId, false);
		_waveId = -1;
	}
}

void CommandBriefingDialogModel::initializeTeamList()
{
	_teamList.clear();
	for (auto& team : Mission_event_teams_tvt) {
		_teamList.emplace_back(team.first, team.second);
	}
}

int CommandBriefingDialogModel::getCurrentTeam() const
{
	return _currentTeam;
}

void CommandBriefingDialogModel::setCurrentTeam(int teamIn)
{
	modify(_currentTeam, teamIn);
};

int CommandBriefingDialogModel::getCurrentStage() const
{
	return _currentStage;
}

int CommandBriefingDialogModel::getTotalStages()
{
	return _wipCommandBrief[_currentTeam].num_stages;
}

SCP_string CommandBriefingDialogModel::getBriefingText() 
{ 
	return _wipCommandBrief[_currentTeam].stage[_currentStage].text;
}

void CommandBriefingDialogModel::setBriefingText(const SCP_string& briefingText)
{
	modify(_wipCommandBrief[_currentTeam].stage[_currentStage].text, briefingText);
}

SCP_string CommandBriefingDialogModel::getAnimationFilename() 
{ 
	return _wipCommandBrief[_currentTeam].stage[_currentStage].ani_filename;
}

void CommandBriefingDialogModel::setAnimationFilename(const SCP_string& animationFilename)
{
	strcpy_s(_wipCommandBrief[_currentTeam].stage[_currentStage].ani_filename, animationFilename.c_str());
	set_modified();
}

SCP_string CommandBriefingDialogModel::getSpeechFilename() 
{ 
	return _wipCommandBrief[_currentTeam].stage[_currentStage].wave_filename;
}

void CommandBriefingDialogModel::setSpeechFilename(const SCP_string& speechFilename)
{
	strcpy_s(_wipCommandBrief[_currentTeam].stage[_currentStage].wave_filename, speechFilename.c_str());
	set_modified();
}

SCP_string CommandBriefingDialogModel::getLowResolutionFilename() 
{ 
	return _wipCommandBrief[_currentTeam].background[0];
}

void CommandBriefingDialogModel::setLowResolutionFilename(const SCP_string& lowResolutionFilename)
{
	strcpy_s(_wipCommandBrief[_currentTeam].background[0], lowResolutionFilename.c_str());
	set_modified();
}

SCP_string CommandBriefingDialogModel::getHighResolutionFilename() 
{ 
	return _wipCommandBrief[_currentTeam].background[1];
}

void CommandBriefingDialogModel::setHighResolutionFilename(const SCP_string& highResolutionFilename) 
{ 
	strcpy_s(_wipCommandBrief[_currentTeam].background[1], highResolutionFilename.c_str()); 
	set_modified();
}


} // namespace fso::fred::dialogs