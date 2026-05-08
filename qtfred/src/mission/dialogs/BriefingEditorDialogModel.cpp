#include "BriefingEditorDialogModel.h"

#include "gamesnd/eventmusic.h"
#include "mission/missionparse.h"
#include "sound/audiostr.h"
#include "iff_defs/iff_defs.h"
#include "ship/ship.h"
#include "object/object.h"
#include "mission/missiongrid.h"

#include <QMessageBox>

namespace fso::fred::dialogs {

namespace {
void copyStageData(brief_stage& dst, const brief_stage& src)
{
	if (&dst == &src) {
		return;
	}

	dst.text = src.text;
	strcpy_s(dst.voice, src.voice);
	dst.camera_pos = src.camera_pos;
	dst.camera_orient = src.camera_orient;
	dst.camera_time = src.camera_time;
	dst.flags = src.flags;
	dst.formula = src.formula;
	dst.draw_grid = src.draw_grid;
	dst.grid_color = src.grid_color;
	dst.num_icons = src.num_icons;
	dst.num_lines = src.num_lines;

	if (dst.icons != nullptr && src.icons != nullptr) {
		std::memcpy(dst.icons, src.icons, sizeof(brief_icon) * MAX_STAGE_ICONS);
	}
	if (dst.lines != nullptr && src.lines != nullptr) {
		std::memcpy(dst.lines, src.lines, sizeof(brief_line) * MAX_BRIEF_STAGE_LINES);
	}
}

void copyBriefingData(briefing& dst, const briefing& src)
{
	dst.num_stages = src.num_stages;
	std::memcpy(dst.background, src.background, sizeof(dst.background));
	std::memcpy(dst.ship_select_background, src.ship_select_background, sizeof(dst.ship_select_background));
	std::memcpy(dst.weapon_select_background, src.weapon_select_background, sizeof(dst.weapon_select_background));

	for (int i = 0; i < MAX_BRIEF_STAGES; ++i) {
		copyStageData(dst.stages[i], src.stages[i]);
	}
}
} // namespace

BriefingEditorDialogModel::BriefingEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

BriefingEditorDialogModel::~BriefingEditorDialogModel()
{
	stopSpeech();
	for (auto& wip : _wipBriefings) {
		for (auto& stage : wip.stages) {
			if (stage.icons != nullptr) {
				vm_free(stage.icons);
				stage.icons = nullptr;
			}
			if (stage.lines != nullptr) {
				vm_free(stage.lines);
				stage.lines = nullptr;
			}
		}
	}
}

bool BriefingEditorDialogModel::apply()
{
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		copyBriefingData(Briefings[i], _wipBriefings[i]);
	}

	Mission_music[SCORE_BRIEFING] = _briefingMusicIndex - 1;
	strcpy_s(The_mission.substitute_briefing_music_name, _subBriefingMusic.c_str());

	return true;
}

void BriefingEditorDialogModel::reject()
{
	stopSpeech();
}

void BriefingEditorDialogModel::initializeData()
{
	initializeTeamList();

	// Make a working copy with independent icon/line storage so edits don't
	// mutate live Briefings[] data and cancel can be a true no-op.
	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		for (int j = 0; j < MAX_BRIEF_STAGES; j++) {
			if (Briefings[i].stages[j].icons != nullptr) {
				_wipBriefings[i].stages[j].icons =
					(brief_icon*)vm_malloc(sizeof(brief_icon) * MAX_STAGE_ICONS);
			}
			if (Briefings[i].stages[j].lines != nullptr) {
				_wipBriefings[i].stages[j].lines =
					(brief_line*)vm_malloc(sizeof(brief_line) * MAX_BRIEF_STAGE_LINES);
			}
		}
		copyBriefingData(_wipBriefings[i], Briefings[i]);
	}

	_briefingMusicIndex = Mission_music[SCORE_BRIEFING] + 1;
	if (_briefingMusicIndex < 0)
		_briefingMusicIndex = 0;
	const int maxIdx = static_cast<int>(Spooled_music.size());
	if (_briefingMusicIndex > maxIdx)
		_briefingMusicIndex = maxIdx;

	_subBriefingMusic = The_mission.substitute_briefing_music_name;

	_currentTeam = 0;
	_currentStage = 0;
	_currentIcon = -1;
	_modified = false;
}

void BriefingEditorDialogModel::stopSpeech()
{
	if (_waveId >= 0) {
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

bool BriefingEditorDialogModel::valid_icon_index(const brief_stage& s, int idx)
{
	return idx >= 0 && idx < s.num_icons;
}
bool BriefingEditorDialogModel::same_line_unordered(int a0, int a1, int b0, int b1)
{
	return (a0 == b0 && a1 == b1) || (a0 == b1 && a1 == b0);
}

void BriefingEditorDialogModel::applyToIconCurrentAndForward(const std::function<void(brief_icon&)>& mutator)
{
	auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages)
		return;

	auto& s = briefing.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return;

	const int targetId = s.icons[_currentIcon].id;

	// Always mutate the icon in the current stage
	mutator(s.icons[_currentIcon]);

	if (_changeLocally) {
		set_modified();
		return;
	}

	// Propagate forward: for each later stage in this team, if an icon with the same id exists, mutate it too.
	for (int st = _currentStage + 1; st < briefing.num_stages; ++st) {
		auto& stg = briefing.stages[st];
		for (int i = 0; i < stg.num_icons; ++i) {
			if (stg.icons[i].id == targetId) {
				mutator(stg.icons[i]);
				// do not break: multiple instances with same id in a stage should all receive the change if present
			}
		}
	}

	set_modified();
}

SCP_vector<int> BriefingEditorDialogModel::getEffectiveSelection(const brief_stage& s) const
{
	SCP_vector<int> selection;
	selection.reserve(_lineSelection.size() + 1);

	for (auto idx : _lineSelection) {
		if (valid_icon_index(s, idx) && std::find(selection.begin(), selection.end(), idx) == selection.end()) {
			selection.push_back(idx);
		}
	}

	if (selection.empty() && valid_icon_index(s, _currentIcon)) {
		selection.push_back(_currentIcon);
	}

	return selection;
}

void BriefingEditorDialogModel::applyToSelectedIconsCurrentAndForward(const std::function<void(brief_icon&)>& mutator)
{
	auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages)
		return;

	auto& stage = briefing.stages[_currentStage];
	const auto selection = getEffectiveSelection(stage);
	if (selection.empty())
		return;

	SCP_unordered_set<int> selectedIds;
	for (auto idx : selection) {
		if (valid_icon_index(stage, idx)) {
			selectedIds.insert(stage.icons[idx].id);
			mutator(stage.icons[idx]);
		}
	}

	if (_changeLocally) {
		set_modified();
		return;
	}

	for (int st = _currentStage + 1; st < briefing.num_stages; ++st) {
		auto& stg = briefing.stages[st];
		for (int i = 0; i < stg.num_icons; ++i) {
			if (selectedIds.find(stg.icons[i].id) != selectedIds.end()) {
				mutator(stg.icons[i]);
			}
		}
	}

	set_modified();
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

	stopSpeech();
	_currentStage++;
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
	_currentIcon = -1;

	// Seed from previous stage if available, otherwise from sane defaults.
	brief_stage& dst = _wipBriefings[_currentTeam].stages[_currentStage];

	if (_currentStage > 0) {
		const brief_stage& prev = _wipBriefings[_currentTeam].stages[_currentStage - 1];

		copyStageData(dst, prev); // start by copying stage data without aliasing storage, then clear fields that should not carry over by default
		dst.text = "<Text here>";
		dst.voice[0] = '\0';
	} else {
		// First stage in an empty briefing
		dst.text = "<Text here>";
		dst.voice[0] = '\0';
		dst.camera_pos = vmd_zero_vector;
		dst.camera_pos.xyz.y = 1000.0f;

		angles a;
		a.h = fl_radians(90.0f);
		a.p = fl_radians(90.0f);
		a.b = fl_radians(90.0f);
		vm_angles_2_matrix(&dst.camera_orient, &a);
		dst.camera_time = 500;
		dst.flags = 0;
		dst.draw_grid = true;
		dst.grid_color = Color_briefing_grid;
		dst.formula = -1;
		dst.num_icons = 0;
		dst.num_lines = 0;
	}

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
		copyStageData(_wipBriefings[_currentTeam].stages[i], _wipBriefings[_currentTeam].stages[i - 1]);
	}

	_currentIcon = -1;

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
		copyStageData(_wipBriefings[_currentTeam].stages[i], _wipBriefings[_currentTeam].stages[i + 1]);
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

	_currentIcon = -1;

	set_modified();
}

// Eventually the Ui will pass in the data from the render view
void BriefingEditorDialogModel::saveStageView(const vec3d& pos, const matrix& orient)
{
	auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages) {
		return;
	}

	brief_stage& s = briefing.stages[_currentStage];
	s.camera_pos = pos; //TODO make modify() support these
	s.camera_orient = orient;
	set_modified();
}

// Returns the camera position and orientation for the current stage so the UI can tell the render camera to move there
std::pair<vec3d, matrix> BriefingEditorDialogModel::getStageView() const
{
	const auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages) {
		return {vmd_zero_vector, vmd_identity_matrix};
	}

	const brief_stage& s = briefing.stages[_currentStage];
	return {s.camera_pos, s.camera_orient};
}

void BriefingEditorDialogModel::copyStageViewToClipboard()
{
	const auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages) {
		_viewClipboardSet = false;
		return;
	}

	const brief_stage& s = briefing.stages[_currentStage];
	_viewClipboardPos = s.camera_pos;
	_viewClipboardOri = s.camera_orient;
	_viewClipboardSet = true;
}

void BriefingEditorDialogModel::pasteClipboardViewToStage()
{
	if (!_viewClipboardSet)
		return;

	auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages) {
		return;
	}

	brief_stage& s = briefing.stages[_currentStage];
	s.camera_pos = _viewClipboardPos; // TODO make modify() support these
	s.camera_orient = _viewClipboardOri;
	set_modified();
}

void BriefingEditorDialogModel::testSpeech()
{
	// May cause unloading/reloading but it's just the mission editor
	// we don't need to keep all the waves loaded only to have to unload them
	// later anyway. This ensures we have one wave loaded and stopSpeech always unloads it

	stopSpeech();

	_waveId = audiostream_open(_wipBriefings[_currentTeam].stages[_currentStage].voice, ASF_EVENTMUSIC);
	if (_waveId >= 0) {
		audiostream_play(_waveId, 1.0f, 0);
	}
}

void BriefingEditorDialogModel::copyToOtherTeams()
{
	stopSpeech();

	for (int i = 0; i < MAX_TVT_TEAMS; i++) {
		if (i != _currentTeam) {
			copyBriefingData(_wipBriefings[i], _wipBriefings[_currentTeam]);
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

int BriefingEditorDialogModel::getCameraTransitionTime() const
{
	const auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages) {
		return 0;
	}
	return briefing.stages[_currentStage].camera_time;
}

void BriefingEditorDialogModel::setCameraTransitionTime(int ms)
{
	auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages) {
		return;
	}
	modify(briefing.stages[_currentStage].camera_time, ms);
}

vec3d BriefingEditorDialogModel::getCameraPosition() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return vmd_zero_vector;
	const auto& s = b.stages[_currentStage];
	return s.camera_pos;
}

matrix BriefingEditorDialogModel::getCameraOrientation() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return vmd_identity_matrix;
	const auto& s = b.stages[_currentStage];
	return s.camera_orient;
}

void BriefingEditorDialogModel::setCameraPosition(const vec3d& pos)
{
	auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return;
	auto& s = b.stages[_currentStage];
	s.camera_pos = pos; // TODO make modify() support these
	set_modified();
}

void BriefingEditorDialogModel::setCameraOrientation(const matrix& orient)
{
	auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return;
	auto& s = b.stages[_currentStage];
	s.camera_orient = orient; // TODO make modify() support these
	set_modified();
}

bool BriefingEditorDialogModel::getCutToNext() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return false;
	return (b.stages[_currentStage].flags & BS_FORWARD_CUT) != 0;
}

void BriefingEditorDialogModel::setCutToNext(bool enabled)
{
	auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return;
	auto& s = b.stages[_currentStage];

	int flags = s.flags;
	if (enabled)
		flags |= BS_FORWARD_CUT;
	else
		flags &= ~BS_FORWARD_CUT;

	modify(s.flags, flags);
}

bool BriefingEditorDialogModel::getCutFromPrev() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return false;
	return (b.stages[_currentStage].flags & BS_BACKWARD_CUT) != 0;
}

void BriefingEditorDialogModel::setCutFromPrev(bool enabled)
{
	auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return;
	auto& s = b.stages[_currentStage];

	int flags = s.flags;
	if (enabled)
		flags |= BS_BACKWARD_CUT;
	else
		flags &= ~BS_BACKWARD_CUT;

	modify(s.flags, flags);
}

bool BriefingEditorDialogModel::getDisableGrid() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return false;
	// disable = !draw_grid
	return !b.stages[_currentStage].draw_grid;
}

void BriefingEditorDialogModel::setDisableGrid(bool disabled)
{
	auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return;

	modify(b.stages[_currentStage].draw_grid, !disabled);
}

int BriefingEditorDialogModel::getCurrentIconIndex() const
{
	return _currentIcon; // -1 means no icon selected
}

void BriefingEditorDialogModel::setCurrentIconIndex(int idx)
{
	const auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages) {
		_currentIcon = -1;
		return;
	}

	const auto& stage = briefing.stages[_currentStage];
	if (idx < 0) {
		_currentIcon = -1;
	} else if (idx >= stage.num_icons) {
		_currentIcon = (stage.num_icons > 0) ? (stage.num_icons - 1) : -1;
	} else {
		_currentIcon = idx;
	}
}

vec3d BriefingEditorDialogModel::getIconPosition() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return vmd_zero_vector;
	const auto& s = b.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return vmd_zero_vector;
	return s.icons[_currentIcon].pos;
}

void BriefingEditorDialogModel::setIconPosition(const vec3d& pos)
{
	// Honors Change Locally: current stage only if enabled, else propagate forward by icon id
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) { modify(ic.pos, pos); });
}

int BriefingEditorDialogModel::getIconId() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return -1;

	const auto& s = b.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return -1;

	return s.icons[_currentIcon].id;
}

void BriefingEditorDialogModel::setIconId(int id)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) { modify(ic.id, id); });
}

SCP_string BriefingEditorDialogModel::getIconLabel() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return {};

	const auto& s = b.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return {};

	return s.icons[_currentIcon].label;
}

void BriefingEditorDialogModel::setIconLabel(const SCP_string& text)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) {
		SCP_string oldLabel = ic.label;
		modify(oldLabel, text);
		strcpy_s(ic.label, oldLabel.c_str());
	});
}

SCP_string BriefingEditorDialogModel::getIconCloseupLabel() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return {};

	const auto& s = b.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return {};

	return s.icons[_currentIcon].closeup_label;
}

void BriefingEditorDialogModel::setIconCloseupLabel(const SCP_string& text)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) {
		SCP_string oldLabel = ic.closeup_label;
		modify(oldLabel, text);
		strcpy_s(ic.closeup_label, oldLabel.c_str());
	});
}

int BriefingEditorDialogModel::getIconTypeIndex() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return -1;
	const auto& s = b.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return -1;
	return s.icons[_currentIcon].type; // 0..MIN_BRIEF_ICONS-1
}

void BriefingEditorDialogModel::setIconTypeIndex(int idx)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) { modify(ic.type, idx); });
}

int BriefingEditorDialogModel::getIconShipTypeIndex() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return -1;
	const auto& s = b.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return -1;
	return s.icons[_currentIcon].ship_class; // may be -1 for unset depending on icon type
}

void BriefingEditorDialogModel::setIconShipTypeIndex(int idx)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) { modify(ic.ship_class, idx); });
}

int BriefingEditorDialogModel::getIconTeamIndex() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return -1;
	const auto& s = b.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return -1;
	return s.icons[_currentIcon].team;
}

void BriefingEditorDialogModel::setIconTeamIndex(int idx)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) { modify(ic.team, idx); });
}

float BriefingEditorDialogModel::getIconScaleFactor() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return 1.0f;
	const auto& s = b.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= s.num_icons)
		return 1.0f;

	return s.icons[_currentIcon].scale_factor;
}

void BriefingEditorDialogModel::setIconScaleFactor(float factor)
{
	// basic clamp for sanity
	if (factor < 0.01f)
		factor = 0.01f;
	if (factor > 10.0f)
		factor = 10.0f;

	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) { modify(ic.scale_factor, factor); });
}

void BriefingEditorDialogModel::setLineSelection(const SCP_vector<int>& indices)
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages) {
		_lineSelection.clear();
		return;
	}
	const auto& s = b.stages[_currentStage];

	SCP_vector<int> cleaned;
	cleaned.reserve(indices.size());
	for (int idx : indices) {
		if (!valid_icon_index(s, idx))
			continue;
		if (std::find(cleaned.begin(), cleaned.end(), idx) == cleaned.end())
			cleaned.push_back(idx);
	}

	_lineSelection = std::move(cleaned);
}

void BriefingEditorDialogModel::clearLineSelection()
{
	_lineSelection.clear();
}

const SCP_vector<int>& BriefingEditorDialogModel::getLineSelection() const
{
	return _lineSelection;
}

BriefingEditorDialogModel::DrawLinesState BriefingEditorDialogModel::getDrawLinesState() const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return DrawLinesState::None;
	const auto& s = b.stages[_currentStage];

	if (_lineSelection.size() < 2)
		return DrawLinesState::None;

	// Count how many unordered pairs exist as lines
	int connected = 0, totalPairs = 0;

	for (size_t i = 0; i + 1 < _lineSelection.size(); ++i) {
		int a = _lineSelection[i];
		if (!valid_icon_index(s, a))
			continue;

		for (size_t j = i + 1; j < _lineSelection.size(); ++j) {
			int bidx = _lineSelection[j];
			if (!valid_icon_index(s, bidx))
				continue;

			++totalPairs;
			bool exists = false;
			for (int l = 0; l < s.num_lines; ++l) {
				if (same_line_unordered(s.lines[l].start_icon, s.lines[l].end_icon, a, bidx)) {
					exists = true;
					break;
				}
			}
			if (exists)
				++connected;
		}
	}

	if (totalPairs == 0)
		return DrawLinesState::None;
	if (connected == 0)
		return DrawLinesState::None;
	if (connected == totalPairs)
		return DrawLinesState::All;
	return DrawLinesState::Partial;
}


void BriefingEditorDialogModel::applyDrawLines(bool checked)
{
	auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return;
	auto& s = b.stages[_currentStage];

	if (_lineSelection.size() < 2)
		return;

	// Build a valid, deduped selection snapshot against current stage
	SCP_vector<int> sel;
	sel.reserve(_lineSelection.size());
	for (int idx : _lineSelection) {
		if (valid_icon_index(s, idx) && std::find(sel.begin(), sel.end(), idx) == sel.end()) {
			sel.push_back(idx);
		}
	}
	if (sel.size() < 2)
		return;

	bool changed = false;

	if (checked) {
		// Add missing lines for every unordered pair
		for (size_t i = 0; i + 1 < sel.size(); ++i) {
			for (size_t j = i + 1; j < sel.size(); ++j) {
				const int a = sel[i], bidx = sel[j];

				bool exists = false;
				for (int l = 0; l < s.num_lines; ++l) {
					if (same_line_unordered(s.lines[l].start_icon, s.lines[l].end_icon, a, bidx)) {
						exists = true;
						break;
					}
				}
				if (!exists && s.num_lines < MAX_BRIEF_STAGE_LINES) {
					s.lines[s.num_lines].start_icon = a;
					s.lines[s.num_lines].end_icon = bidx;
					s.num_lines++;
					changed = true;
				}
			}
		}
	} else {
		// Remove any lines whose endpoints are both in the selection
		for (int i = s.num_lines - 1; i >= 0; --i) {
			const int a = s.lines[i].start_icon;
			const int bidx = s.lines[i].end_icon;
			const bool aSel = std::find(sel.begin(), sel.end(), a) != sel.end();
			const bool bSel = std::find(sel.begin(), sel.end(), bidx) != sel.end();
			if (aSel && bSel) {
				// delete by shifting down
				for (int k = i; k + 1 < s.num_lines; ++k) {
					s.lines[k] = s.lines[k + 1];
				}
				s.num_lines--;
				changed = true;
			}
		}
	}

	// Clean any broken lines just in case
	for (int i = s.num_lines - 1; i >= 0; --i) {
		const int a = s.lines[i].start_icon;
		const int bidx = s.lines[i].end_icon;
		if (!valid_icon_index(s, a) || !valid_icon_index(s, bidx)) {
			for (int k = i; k + 1 < s.num_lines; ++k) {
				s.lines[k] = s.lines[k + 1];
			}
			s.num_lines--;
			changed = true;
		}
	}

	if (changed)
		set_modified();
}

bool BriefingEditorDialogModel::getChangeLocally() const
{
	return _changeLocally;
}

void BriefingEditorDialogModel::setChangeLocally(bool enabled)
{
	// Editor-only; do not mark modified
	modify(_changeLocally, enabled);
}

TriStateBool BriefingEditorDialogModel::getSelectedIconFlagState(int flag) const
{
	const auto& b = _wipBriefings[_currentTeam];
	if (b.num_stages <= 0 || _currentStage < 0 || _currentStage >= b.num_stages)
		return TriStateBool::FALSE_;
	const auto& s = b.stages[_currentStage];

	const auto selection = getEffectiveSelection(s);
	if (selection.empty())
		return TriStateBool::FALSE_;

	bool anyChecked = false;
	bool anyUnchecked = false;
	for (auto idx : selection) {
		if (!valid_icon_index(s, idx)) {
			continue;
		}
		if ((s.icons[idx].flags & flag) != 0) {
			anyChecked = true;
		} else {
			anyUnchecked = true;
		}
	}

	if (anyChecked && anyUnchecked)
		return TriStateBool::UNKNOWN_;
	return anyChecked ? TriStateBool::TRUE_ : TriStateBool::FALSE_;
}

TriStateBool BriefingEditorDialogModel::getIconHighlightedState() const
{
	return getSelectedIconFlagState(BI_HIGHLIGHT);
}

void BriefingEditorDialogModel::setIconHighlighted(bool enabled)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) {
		int newFlags = ic.flags;
		if (enabled) {
			newFlags |= BI_HIGHLIGHT;
		} else {
			newFlags &= ~BI_HIGHLIGHT;
		}
		modify(ic.flags, newFlags);
	});
}

TriStateBool BriefingEditorDialogModel::getIconFlippedState() const
{
	return getSelectedIconFlagState(BI_MIRROR_ICON);
}

void BriefingEditorDialogModel::setIconFlipped(bool enabled)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) {
		int newFlags = ic.flags;
		if (enabled) {
			newFlags |= BI_MIRROR_ICON;
		} else {
			newFlags &= ~BI_MIRROR_ICON;
		}
		modify(ic.flags, newFlags);
	});
}

TriStateBool BriefingEditorDialogModel::getIconUseWingState() const
{
	return getSelectedIconFlagState(BI_USE_WING_ICON);
}

void BriefingEditorDialogModel::setIconUseWing(bool enabled)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) {
		int newFlags = ic.flags;
		if (enabled) {
			newFlags |= BI_USE_WING_ICON;
		} else {
			newFlags &= ~BI_USE_WING_ICON;
		}
		modify(ic.flags, newFlags);
	});
}

TriStateBool BriefingEditorDialogModel::getIconUseCargoState() const
{
	return getSelectedIconFlagState(BI_USE_CARGO_ICON);
}

void BriefingEditorDialogModel::setIconUseCargo(bool enabled)
{
	applyToSelectedIconsCurrentAndForward([&](brief_icon& ic) {
		int newFlags = ic.flags;
		if (enabled) {
			newFlags |= BI_USE_CARGO_ICON;
		} else {
			newFlags &= ~BI_USE_CARGO_ICON;
		}
		modify(ic.flags, newFlags);
	});
}

void BriefingEditorDialogModel::makeIcon(const SCP_string& label, int typeIndex, int teamIndex, int shipClassIndex)
{
	auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages)
		return;

	auto& s = briefing.stages[_currentStage];
	if (s.num_icons >= MAX_STAGE_ICONS)
		return; // at capacity

	// Small scoped helpers
	auto clamp = [](int v, int lo, int hi) { return (v < lo) ? lo : (v > hi ? hi : v); };
	auto copy_cstr = [&](char* dst, size_t cap, const SCP_string& src) {
		if (cap == 0)
			return;
		const size_t n = std::min(cap - 1, src.size());
		if (n)
			std::memcpy(dst, src.data(), n);
		dst[n] = '\0';
	};
	auto next_icon_id = [&]() {
		int maxId = -1;
		for (int st = 0; st < briefing.num_stages; ++st) {
			const auto& bs = briefing.stages[st];
			for (int i = 0; i < bs.num_icons; ++i)
				maxId = std::max(maxId, bs.icons[i].id);
		}
		return maxId + 1; // unique within this team's briefing
	};

	// Clamp incoming indices to safe ranges
	const int safeType = clamp(typeIndex, 0, MIN_BRIEF_ICONS - 1);
	const int safeTeam = clamp(teamIndex, 0, static_cast<int>(Iff_info.size()) - 1);
	const int safeClass = (shipClassIndex < 0) ? -1 : clamp(shipClassIndex, 0, static_cast<int>(Ship_info.size()) - 1);

	// Allocate slot
	const int idx = s.num_icons;
	brief_icon& ic = s.icons[idx];

	// Minimal identity/classification
	ic.id = next_icon_id();
	ic.type = safeType;
	ic.team = safeTeam;
	ic.ship_class = safeClass;

	// Labels
	copy_cstr(ic.label, MAX_LABEL_LEN, label);
	copy_cstr(ic.closeup_label, MAX_LABEL_LEN, SCP_string()); // empty by default

	// Defaults
	ic.pos = vmd_zero_vector; // renderer can move it after creation
	ic.scale_factor = 1.0f;
	ic.flags = 0;             // not flipped/highlighted/wing/cargo
	ic.modelnum = -1;
	ic.model_instance_num = -1;

	// Commit
	s.num_icons++;
	_currentIcon = idx;
	_lineSelection.clear();
	set_modified();
}

void BriefingEditorDialogModel::deleteCurrentIcon()
{
	auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages)
		return;

	auto& s = briefing.stages[_currentStage];
	const int del = _currentIcon;
	if (del < 0 || del >= s.num_icons)
		return;

	// Remove any lines that reference the icon being deleted
	for (int i = s.num_lines - 1; i >= 0; --i) {
		const int a = s.lines[i].start_icon;
		const int b = s.lines[i].end_icon;
		if (a == del || b == del) {
			for (int k = i; k + 1 < s.num_lines; ++k) {
				s.lines[k] = s.lines[k + 1];
			}
			--s.num_lines;
		}
	}

	// Reindex remaining line endpoints
	for (int i = 0; i < s.num_lines; ++i) {
		if (s.lines[i].start_icon > del)
			--s.lines[i].start_icon;
		if (s.lines[i].end_icon > del)
			--s.lines[i].end_icon;
	}

	// Shift icons down to fill the gap
	for (int i = del; i + 1 < s.num_icons; ++i) {
		s.icons[i] = s.icons[i + 1];
	}
	--s.num_icons;

	// Update selection
	_lineSelection.clear();
	_currentIcon = -1;

	set_modified();
}

void BriefingEditorDialogModel::propagateCurrentIconForward()
{
	auto& briefing = _wipBriefings[_currentTeam];
	if (briefing.num_stages <= 0 || _currentStage < 0 || _currentStage >= briefing.num_stages)
		return;

	auto& curStage = briefing.stages[_currentStage];
	if (_currentIcon < 0 || _currentIcon >= curStage.num_icons)
		return;

	const brief_icon src = curStage.icons[_currentIcon]; // snapshot of current icon
	bool changed = false;

	for (int st = _currentStage + 1; st < briefing.num_stages; ++st) {
		auto& s = briefing.stages[st];

		// Match original FRED behavior: only add missing icons to later stages.
		// If an icon with the same id already exists in a stage, leave it unchanged.
		bool found = false;
		for (int i = 0; i < s.num_icons; ++i) {
			if (s.icons[i].id == src.id) {
				found = true;
				break;
			}
		}

		if (!found && s.num_icons < MAX_STAGE_ICONS) {
			s.icons[s.num_icons] = src;
			s.num_icons++;
			changed = true;
		}
		// If at capacity and missing, skip this stage.
	}

	if (changed)
		set_modified();
}

int BriefingEditorDialogModel::getBriefingMusicIndex() const
{
	return _briefingMusicIndex;
}

void BriefingEditorDialogModel::setBriefingMusicIndex(int idx)
{
	const int maxIdx = static_cast<int>(Spooled_music.size());
	if (idx < 0)
		idx = 0;
	if (idx > maxIdx)
		idx = maxIdx;
	modify(_briefingMusicIndex, idx);
}

SCP_string BriefingEditorDialogModel::getSubstituteBriefingMusicName() const
{
	return _subBriefingMusic;
}

void BriefingEditorDialogModel::setSubstituteBriefingMusicName(const SCP_string& name)
{
	modify(_subBriefingMusic, name);
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

SCP_vector<std::pair<int, SCP_string>> BriefingEditorDialogModel::getIconList()
{
	SCP_vector<std::pair<int, SCP_string>> out;
	out.reserve(MIN_BRIEF_ICONS);
	for (int i = 0; i < MIN_BRIEF_ICONS; ++i) {
		out.emplace_back(i, Icon_names[i]);
	}
	return out;
}

SCP_vector<std::pair<int, SCP_string>> BriefingEditorDialogModel::getShipList()
{
	SCP_vector<std::pair<int, SCP_string>> out;
	out.reserve(static_cast<int>(Ship_info.size()));
	int idx = 0;
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it, ++idx) {
		out.emplace_back(idx, it->name);
	}
	return out;
}

SCP_vector<std::pair<int, SCP_string>> BriefingEditorDialogModel::getIffList()
{
	SCP_vector<std::pair<int, SCP_string>> out;
	out.reserve(static_cast<int>(Iff_info.size()));
	for (int i = 0; i < static_cast<int>(Iff_info.size()); ++i) {
		out.emplace_back(i, Iff_info[i].iff_name);
	}
	return out;
}

briefing* BriefingEditorDialogModel::getWipBriefingPtr(int team)
{
	// NOLINTNEXTLINE(readability-simplify-boolean-expr)
	Assertion(team >= 0 && team < MAX_TVT_TEAMS, "Invalid team index %d", team);
	return &_wipBriefings[team];
}

void BriefingEditorDialogModel::makeIconFromShip(int shipIndex)
{
	if (shipIndex < 0 || shipIndex >= MAX_SHIPS)
		return;

	const auto& shipp = Ships[shipIndex];
	if (shipp.objnum < 0)
		return;

	// Determine icon type from ship class
	int iconType = ICON_FIGHTER; // default
	if (shipp.ship_info_index >= 0 && shipp.ship_info_index < static_cast<int>(Ship_info.size())) {
		const auto& sip = Ship_info[shipp.ship_info_index];
		// Map ship type flags to briefing icon types
		if (sip.is_big_or_huge()) {
			if (sip.flags[Ship::Info_Flags::Corvette])
				iconType = ICON_CORVETTE;
			else if (sip.flags[Ship::Info_Flags::Supercap])
				iconType = ICON_SUPERCAP;
			else
				iconType = ICON_CAPITAL;
		} else if (sip.flags[Ship::Info_Flags::Transport]) {
			iconType = ICON_TRANSPORT;
		} else if (sip.flags[Ship::Info_Flags::Freighter]) {
			iconType = ICON_FREIGHTER_NO_CARGO;
		} else if (sip.flags[Ship::Info_Flags::Cruiser]) {
			iconType = ICON_CRUISER;
		} else if (sip.flags[Ship::Info_Flags::Fighter]) {
			iconType = ICON_FIGHTER;
		} else if (sip.flags[Ship::Info_Flags::Bomber]) {
			iconType = ICON_BOMBER;
		} else if (sip.flags[Ship::Info_Flags::Awacs]) {
			iconType = ICON_AWACS;
		} else if (sip.flags[Ship::Info_Flags::Gas_miner]) {
			iconType = ICON_GAS_MINER;
		} else if (sip.flags[Ship::Info_Flags::Sentrygun]) {
			iconType = ICON_SENTRYGUN;
		} else if (sip.flags[Ship::Info_Flags::Support]) {
			iconType = ICON_SUPPORT_SHIP;
		}
	}

	// Find the IFF team index
	int teamIndex = shipp.team;

	makeIcon(SCP_string(shipp.ship_name), iconType, teamIndex, shipp.ship_info_index);
}

void BriefingEditorDialogModel::makeIconFromWing(int wingIndex)
{
	if (wingIndex < 0 || wingIndex >= Num_wings)
		return;

	const auto& wingp = Wings[wingIndex];
	if (wingp.wave_count <= 0)
		return;

	const int firstShipIndex = wingp.ship_index[0];
	if (firstShipIndex < 0 || firstShipIndex >= MAX_SHIPS || Ships[firstShipIndex].objnum < 0)
		return;

	const auto& shipp = Ships[firstShipIndex];
	int iconType = ICON_FIGHTER_WING;
	if (shipp.ship_info_index >= 0 && shipp.ship_info_index < static_cast<int>(Ship_info.size())) {
		const auto& sip = Ship_info[shipp.ship_info_index];
		if (sip.flags[Ship::Info_Flags::Bomber]) {
			iconType = ICON_BOMBER_WING;
		}
	}

	makeIcon(SCP_string(wingp.name), iconType, shipp.team, shipp.ship_info_index);
	setIconUseWing(true);
}

SCP_vector<BriefingEditorDialogModel::WingTreeEntry> BriefingEditorDialogModel::getWingShipTree()
{
	SCP_vector<WingTreeEntry> result;

	// Track which ships are in wings so we can list ungrouped ships separately
	SCP_vector<bool> in_wing(MAX_SHIPS, false);

	for (int w = 0; w < Num_wings; ++w) {
		const auto& wingp = Wings[w];
		if (wingp.wave_count <= 0)
			continue;

		WingTreeEntry entry;
		entry.wingName = wingp.name;
		entry.wingIndex = w;

		for (int i = 0; i < wingp.wave_count; ++i) {
			int si = wingp.ship_index[i];
			if (si >= 0 && si < MAX_SHIPS && Ships[si].objnum >= 0) {
				entry.ships.push_back({Ships[si].ship_name, si});
				in_wing[si] = true;
			}
		}

		if (!entry.ships.empty()) {
			result.push_back(std::move(entry));
		}
	}

	// Add ungrouped ships under a pseudo-wing entry with empty name
	WingTreeEntry ungrouped;
	for (int i = 0; i < MAX_SHIPS; ++i) {
		if (Ships[i].objnum >= 0 && !in_wing[i]) {
			ungrouped.ships.push_back({Ships[i].ship_name, i});
		}
	}
	if (!ungrouped.ships.empty()) {
		result.push_back(std::move(ungrouped));
	}

	return result;
}

} // namespace fso::fred::dialogs
