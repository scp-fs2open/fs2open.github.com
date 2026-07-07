#include "DebriefingDialogModel.h"
#include "mission/missionparse.h"
#include "gamesnd/eventmusic.h"
#include "parse/sexp.h"
#include "sound/audiostr.h"
#include <QMessageBox>

namespace fso::fred::dialogs {

DebriefingDialogModel::DebriefingDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
		initializeData();
}

DebriefingDialogModel::~DebriefingDialogModel()
{
	stopSpeech();
	for (auto& wip : _wipDebriefing) {
		for (auto& stage : wip.stages) {
			if (stage.formula >= 0) {
				free_sexp2(stage.formula);
				stage.formula = -1;
			}
		}
	}
}

bool DebriefingDialogModel::apply()
{
	stopSpeech();

	// Capture any pending edits from the visible tree into _wipDebriefing.
	commitCurrentFormula();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		// Free Debriefings's existing formulas before they get overwritten by the assignment.
		for (auto& stage : Debriefings[i].stages) {
			if (stage.formula >= 0) {
				free_sexp2(stage.formula);
				stage.formula = -1;
			}
		}
		Debriefings[i] = _wipDebriefing[i];
		// Ownership of the formulas has transferred to Debriefings; sever the
		// _wipDebriefing references so the destructor doesn't double-free.
		for (auto& stage : _wipDebriefing[i].stages) {
			stage.formula = -1;
		}
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
		// The struct assignment shallow-copied formula indices; give _wipDebriefing
		// its own sexp trees so edits/frees can't corrupt the live Debriefings[].
		for (auto& stage : _wipDebriefing[i].stages) {
			if (stage.formula >= 0) {
				stage.formula = dup_sexp_chain(stage.formula);
			}
		}
	}

	_successMusic = Mission_music[SCORE_DEBRIEFING_SUCCESS];
	_averageMusic = Mission_music[SCORE_DEBRIEFING_AVERAGE];
	_failureMusic = Mission_music[SCORE_DEBRIEFING_FAILURE];

	_currentTeam = 0;
	_currentStage = 0;
	_modified = false;
}

void DebriefingDialogModel::commitCurrentFormula()
{
	if (_sexpTree == nullptr)
		return;
	if (_currentTeam < 0 || _currentTeam >= MAX_TVT_TEAMS)
		return;
	if (_currentStage < 0 || _currentStage >= _wipDebriefing[_currentTeam].num_stages)
		return;

	int newFormula = _sexpTree->_model.save_tree();
	auto& stage = _wipDebriefing[_currentTeam].stages[_currentStage];
	if (stage.formula >= 0 && stage.formula != newFormula) {
		free_sexp2(stage.formula);
	}
	stage.formula = newFormula;
}

void DebriefingDialogModel::gotoPreviousStage()
{
	if (_currentStage <= 0) {
		_currentStage = 0;
		return;
	}

	commitCurrentFormula();
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

	commitCurrentFormula();
	_currentStage++;
	stopSpeech();
}

void DebriefingDialogModel::addStage()
{
	commitCurrentFormula();
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
	commitCurrentFormula();
	stopSpeech();

	if (_wipDebriefing[_currentTeam].num_stages >= MAX_DEBRIEF_STAGES) {
		_wipDebriefing[_currentTeam].num_stages = MAX_DEBRIEF_STAGES;
		set_modified();
		return;
	}

	const int last = _wipDebriefing[_currentTeam].num_stages; // new tail slot (post-increment index)
	_wipDebriefing[_currentTeam].num_stages++;

	// If the slot we're about to overwrite was owned, free it first.
	if (_wipDebriefing[_currentTeam].stages[last].formula >= 0) {
		free_sexp2(_wipDebriefing[_currentTeam].stages[last].formula);
		_wipDebriefing[_currentTeam].stages[last].formula = -1;
	}

	for (int i = last; i > _currentStage; i--) {
		_wipDebriefing[_currentTeam].stages[i] = _wipDebriefing[_currentTeam].stages[i - 1];
	}

	// The shifted-up slot at _currentStage+1 now shares its formula index with
	// _currentStage. Give it its own owned copy.
	auto& inserted = _wipDebriefing[_currentTeam].stages[_currentStage + 1];
	if (inserted.formula >= 0) {
		inserted.formula = dup_sexp_chain(inserted.formula);
	}

	// Future TODO: Add a QtFRED Option to clear the inserted stage instead of copying the current one.

	set_modified();
}

void DebriefingDialogModel::deleteStage()
{
	commitCurrentFormula();
	stopSpeech();

	// Clear everything if we were on the last stage.
	if (_wipDebriefing[_currentTeam].num_stages <= 1) {
		_wipDebriefing[_currentTeam].num_stages = 0;
		if (_wipDebriefing[_currentTeam].stages[0].formula >= 0) {
			free_sexp2(_wipDebriefing[_currentTeam].stages[0].formula);
		}
		_wipDebriefing[_currentTeam].stages[0].text.clear();
		_wipDebriefing[_currentTeam].stages[0].recommendation_text.clear();
		memset(_wipDebriefing[_currentTeam].stages[0].voice, 0, CF_MAX_FILENAME_LENGTH);
		_wipDebriefing[_currentTeam].stages[0].formula = -1;
		set_modified();
		return;
	}

	// Free the formula at _currentStage before the loop overwrites it.
	if (_wipDebriefing[_currentTeam].stages[_currentStage].formula >= 0) {
		free_sexp2(_wipDebriefing[_currentTeam].stages[_currentStage].formula);
		_wipDebriefing[_currentTeam].stages[_currentStage].formula = -1;
	}

	// copy the stages backwards until we get to the stage we're on
	for (int i = _currentStage; i + 1 < _wipDebriefing[_currentTeam].num_stages; i++) {
		_wipDebriefing[_currentTeam].stages[i] = _wipDebriefing[_currentTeam].stages[i + 1];
	}

	_wipDebriefing[_currentTeam].num_stages--;

	// Clear the tail. The last shift left it sharing its formula with the slot
	// above it, so just sever the reference (no free — the slot above owns it now).
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
	commitCurrentFormula();
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		if (i != _currentTeam) {
			// Free this team's existing formulas before they get overwritten.
			for (auto& stage : _wipDebriefing[i].stages) {
				if (stage.formula >= 0) {
					free_sexp2(stage.formula);
					stage.formula = -1;
				}
			}
			_wipDebriefing[i] = _wipDebriefing[_currentTeam];
			// Shallow copy shared formula indices across teams; give each team its own copy.
			for (auto& stage : _wipDebriefing[i].stages) {
				if (stage.formula >= 0) {
					stage.formula = dup_sexp_chain(stage.formula);
				}
			}
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
	if (_waveId >= 0) {
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
	if (teamIn != _currentTeam) {
		commitCurrentFormula();
	}
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