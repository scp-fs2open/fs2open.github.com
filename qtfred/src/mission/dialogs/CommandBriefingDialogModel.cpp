#include "CommandBriefingDialogModel.h"
#include "missionui/missioncmdbrief.h"
#include "sound/audiostr.h"
#include <QMessageBox>

namespace fso {
namespace fred {
namespace dialogs {

CommandBriefingDialogModel::CommandBriefingDialogModel(QObject* parent, EditorViewport* viewport) 
	: AbstractDialogModel(parent, viewport)
{
		initializeData();
}

void CommandBriefingDialogModel::initializeData() 
{
	Cur_cmd_brief = Cmd_briefs;  // default to first cmd briefing
	_wipCommandBrief.num_stages = Cur_cmd_brief->num_stages;
	strcpy_s(_wipCommandBrief.background[0],Cur_cmd_brief->background[0]);
	strcpy_s(_wipCommandBrief.background[1],Cur_cmd_brief->background[1]);
	
	if (!strlen(_wipCommandBrief.background[0])) {
		strcpy_s(_wipCommandBrief.background[0], "<default>");
	}

	if (!strlen(_wipCommandBrief.background[1])) {
		strcpy_s(_wipCommandBrief.background[1], "<default>");
	}

	if (_wipCommandBrief.num_stages <= 0) {
		_currentStage = 0;
	} else {
		_currentStage = 1;
	}

	int i;

	for (i = 0; i < _wipCommandBrief.num_stages; i++) {
		_wipCommandBrief.stage[i] = Cur_cmd_brief->stage[i];
	}

	for (i = _wipCommandBrief.num_stages; i < CMD_BRIEF_STAGES_MAX; i++) {
		strcpy_s(_wipCommandBrief.stage[i].ani_filename, "<default>");
		_wipCommandBrief.stage[i].text = "<Text here>";
		_wipCommandBrief.stage[i].wave = -1;
		strcpy_s(_wipCommandBrief.stage[i].wave_filename, "none");
	}

	_briefingTextUpdateRequired = false;
	_stageNumberUpdateRequired = true; // always need to start off setting the correct stage.
	_soundTestUpdateRequired = true;
	_currentlyPlayingSound = -1;

	_currentTeam = 0; // this is forced to zero and kept there until multiple teams command briefing is supported.
	modelChanged();
}

bool CommandBriefingDialogModel::apply()
{
	stopSpeech();

	// Copy the bits that are global to the Command Briefing
	Cur_cmd_brief->num_stages = _wipCommandBrief.num_stages;
	strcpy_s(Cur_cmd_brief->background[0], _wipCommandBrief.background[0]);
	strcpy_s(Cur_cmd_brief->background[1], _wipCommandBrief.background[1]);

	int i = 0;

	for (i = 0; i < _wipCommandBrief.num_stages; i++) {
		Cur_cmd_brief->stage[i] =_wipCommandBrief.stage[i];
	}

	for (i = _wipCommandBrief.num_stages; i < CMD_BRIEF_STAGES_MAX; i++) {
		Cur_cmd_brief->stage[i].ani_filename[0] = 0;
		Cur_cmd_brief->stage[i].text.clear();
		audiostream_close_file(_wipCommandBrief.stage[i].wave, false);
		Cur_cmd_brief->stage[i].wave = -1;
	}

	_wipCommandBrief.num_stages = 0;

	return true;
}

void CommandBriefingDialogModel::reject() 
{

	stopSpeech();

	for (int i = _wipCommandBrief.num_stages; i < CMD_BRIEF_STAGES_MAX; i++) {
		memset(&_wipCommandBrief.stage[i].ani_filename, 0, CF_MAX_FILENAME_LENGTH);
		_wipCommandBrief.stage[i].text.clear();
		audiostream_close_file(_wipCommandBrief.stage[i].wave, false);
		_wipCommandBrief.stage[i].wave = -1;
		memset(&_wipCommandBrief.stage[i].wave_filename, 0, CF_MAX_FILENAME_LENGTH);	
	}

	_wipCommandBrief.num_stages = 0;
	memset(&_wipCommandBrief.background[0], 0, CF_MAX_FILENAME_LENGTH);
	memset(&_wipCommandBrief.background[1], 0, CF_MAX_FILENAME_LENGTH);

}

void CommandBriefingDialogModel::update_init() {}

void CommandBriefingDialogModel::gotoPreviousStage()
{
	// make sure 
	if (_currentStage <= 0) {
		_currentStage = 0;
		return;
	}

	_briefingTextUpdateRequired = true;
	_stageNumberUpdateRequired = true;
	stopSpeech();
	_currentStage--;
	modelChanged();
}

void CommandBriefingDialogModel::gotoNextStage()
{
	_currentStage++;

	if (_currentStage >= _wipCommandBrief.num_stages) {
		_currentStage = _wipCommandBrief.num_stages - 1;
	}
	else {
		stopSpeech();
		_briefingTextUpdateRequired = true;
		_stageNumberUpdateRequired = true;
	}

	// should update regardless, who knows, maybe there was an inexplicable invalid index before.
	modelChanged();
}

void CommandBriefingDialogModel::addStage()
{
	_stageNumberUpdateRequired = true;
	_briefingTextUpdateRequired = true;

	stopSpeech();

	if (_wipCommandBrief.num_stages >= CMD_BRIEF_STAGES_MAX) {
		_wipCommandBrief.num_stages = CMD_BRIEF_STAGES_MAX;
		_currentStage = _wipCommandBrief.num_stages - 1;
		modelChanged(); // signal that the model has changed, in case of inexplicable invalid index.
		return;
	}

	_wipCommandBrief.num_stages++;
	_currentStage = _wipCommandBrief.num_stages - 1;
	
	modelChanged();
}

// copies the current stage as the next stage and then moves the rest of the stages over.
void CommandBriefingDialogModel::insertStage()
{
	_stageNumberUpdateRequired = true;

	if (_wipCommandBrief.num_stages >= CMD_BRIEF_STAGES_MAX) {
		_wipCommandBrief.num_stages = CMD_BRIEF_STAGES_MAX;
		modelChanged(); // signal that the model has changed, in case of inexplicable invalid index.
		return;
	}

	_wipCommandBrief.num_stages++;

	for (int i = _wipCommandBrief.num_stages - 1; i > _currentStage; i--) {
		_wipCommandBrief.stage[i] = _wipCommandBrief.stage[i - 1];
	}
	modelChanged();
}

void CommandBriefingDialogModel::deleteStage()
{
	_briefingTextUpdateRequired = true;
	_stageNumberUpdateRequired = true;

	stopSpeech();

	// Clear everything if we were on the last stage.
	if (_wipCommandBrief.num_stages <= 1) {
		_wipCommandBrief.num_stages = 0;
		_wipCommandBrief.stage[0].text.clear();
		_wipCommandBrief.stage[0].wave = -1;
		memset(_wipCommandBrief.stage[0].wave_filename, 0, CF_MAX_FILENAME_LENGTH);
		memset(_wipCommandBrief.stage[0].ani_filename, 0, CF_MAX_FILENAME_LENGTH);
		modelChanged();
		return;
	}
	
	// copy the stages backwards until we get to the stage we're on
	for (int i = _currentStage; i + 1 < _wipCommandBrief.num_stages; i++){
		_wipCommandBrief.stage[i] = _wipCommandBrief.stage[i + 1];
	}

	_wipCommandBrief.num_stages--;

	// make sure that the current stage is valid.
	if (_wipCommandBrief.num_stages <= _currentStage) {
		_currentStage = _wipCommandBrief.num_stages - 1;
	}

	modelChanged();
}

void CommandBriefingDialogModel::setWaveID()
{
	// close the old one
	if (_wipCommandBrief.stage[_currentStage].wave >= 0) {
		audiostream_close_file(_wipCommandBrief.stage[_currentStage].wave, false);
	}

	// we use ASF_EVENTMUSIC here so that it will keep the extension in place
	_wipCommandBrief.stage[_currentStage].wave = audiostream_open(_wipCommandBrief.stage[_currentStage].wave_filename, ASF_EVENTMUSIC);
	_soundTestUpdateRequired = true;
}

void CommandBriefingDialogModel::testSpeech() 
{
	if (_wipCommandBrief.stage[_currentStage].wave >= 0 && !audiostream_is_playing(_wipCommandBrief.stage[_currentStage].wave)) {
		stopSpeech();
		audiostream_play(_wipCommandBrief.stage[_currentStage].wave, 1.0f, 0);
		_currentlyPlayingSound = _wipCommandBrief.stage[_currentStage].wave;
	}
}

void CommandBriefingDialogModel::stopSpeech()
{
	if (_currentlyPlayingSound >= -1) {
		audiostream_stop(_currentlyPlayingSound,1,0);
		_currentlyPlayingSound = -1;
	}
}

bool CommandBriefingDialogModel::briefingUpdateRequired() 
{
	return _briefingTextUpdateRequired; 
}

bool CommandBriefingDialogModel::stageNumberUpdateRequired() 
{ 
	return _stageNumberUpdateRequired; 
}

bool CommandBriefingDialogModel::soundTestUpdateRequired() 
{ 
	return _soundTestUpdateRequired; 
}

SCP_string CommandBriefingDialogModel::getBriefingText() 
{ 
	_briefingTextUpdateRequired = false; 
	return _wipCommandBrief.stage[_currentStage].text; 
}

SCP_string CommandBriefingDialogModel::getAnimationFilename() 
{ 
	return _wipCommandBrief.stage[_currentStage].ani_filename; 
}

SCP_string CommandBriefingDialogModel::getSpeechFilename() 
{ 
	return _wipCommandBrief.stage[_currentStage].wave_filename; 
}

ubyte CommandBriefingDialogModel::getCurrentTeam() 
{ 
	return _currentTeam; 
}

SCP_string CommandBriefingDialogModel::getLowResolutionFilename() 
{ 
	return _wipCommandBrief.background[0]; 
}

SCP_string CommandBriefingDialogModel::getHighResolutionFilename() 
{ 
	return _wipCommandBrief.background[1]; 
}

int CommandBriefingDialogModel::getTotalStages() 
{ 
	_stageNumberUpdateRequired = false; 
	return _wipCommandBrief.num_stages; 
}

int CommandBriefingDialogModel::getCurrentStage() 
{ 
	return _currentStage; 
}

int CommandBriefingDialogModel::getSpeechInstanceNumber() 
{ 
	return _wipCommandBrief.stage[_currentStage].wave; 
}

void CommandBriefingDialogModel::setBriefingText(const SCP_string& briefingText) 
{ 
	_wipCommandBrief.stage[_currentStage].text = briefingText; 
	modelChanged(); 
}

void CommandBriefingDialogModel::setAnimationFilename(const SCP_string& animationFilename) 
{ 
	strcpy_s(_wipCommandBrief.stage[_currentStage].ani_filename, animationFilename.c_str()); 
	modelChanged(); 
}

void CommandBriefingDialogModel::setSpeechFilename(const SCP_string& speechFilename) 
{ 
	_soundTestUpdateRequired = true; 
	strcpy_s(_wipCommandBrief.stage[_currentStage].wave_filename, speechFilename.c_str());
	setWaveID(); 
	modelChanged(); 
}

void CommandBriefingDialogModel::setCurrentTeam(const ubyte& teamIn) 
{ 
	_currentTeam = teamIn;
}; // not yet fully supported

void CommandBriefingDialogModel::setLowResolutionFilename(const SCP_string& lowResolutionFilename) 
{ 
	strcpy_s(_wipCommandBrief.background[0], lowResolutionFilename.c_str()); 
	modelChanged();  
}

void CommandBriefingDialogModel::setHighResolutionFilename(const SCP_string& highResolutionFilename) 
{ 
	strcpy_s(_wipCommandBrief.background[1], highResolutionFilename.c_str()); 
	modelChanged(); 
}

void CommandBriefingDialogModel::requestInitialUpdate() 
{ 
	_briefingTextUpdateRequired = true;  
}


}
}
}