#include <gamesnd/eventmusic.h>
#include "mission/dialogs/FictionViewerDialogModel.h"

namespace fso::fred::dialogs {

FictionViewerDialogModel::FictionViewerDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {

	initializeData();
}

bool FictionViewerDialogModel::apply() {
	// if the story file for the current stage is empty, treat as no fiction viewer stage
	// currently we only support one stage, so just check the first one
	const auto& stage = _fictionViewerStages.at(0);
	const bool empty = stage.story_filename[0] == '\0';

	if (empty) {
		_fictionViewerStages.clear();
		Mission_music[SCORE_FICTION_VIEWER] = -1;
	} else {
		// Keep whatever you’ve edited in _fictionViewerStages
		Mission_music[SCORE_FICTION_VIEWER] = _fictionMusic; // -1 for none is valid
	}

	// Commit working copy to mission
	Fiction_viewer_stages = _fictionViewerStages;
	return true;
}

void FictionViewerDialogModel::reject() {
	// nothing to do
}

void FictionViewerDialogModel::initializeData() {

	_fictionViewerStages = Fiction_viewer_stages;

	// make sure we have at least one stage
	if (_fictionViewerStages.empty()) {
		fiction_viewer_stage stage;
		memset(&stage, 0, sizeof(fiction_viewer_stage));
		stage.formula = Locked_sexp_true;

		_fictionViewerStages.push_back(stage);
	}
	
	_musicOptions.emplace_back("None", -1);
	for (int i = 0; i < static_cast<int>(Spooled_music.size()); ++i) {
		_musicOptions.emplace_back(Spooled_music[i].name, i);
	}

	// music is managed through the mission
	_fictionMusic = Mission_music[SCORE_FICTION_VIEWER];
}

const SCP_vector<std::pair<SCP_string, int>>& FictionViewerDialogModel::getMusicOptions()
{
	return _musicOptions;
}

SCP_string FictionViewerDialogModel::getStoryFile() const
{
	return _fictionViewerStages[_fictionViewerStageIndex].story_filename;
}

void FictionViewerDialogModel::setStoryFile(const SCP_string& storyFile)
{
	auto& stage = _fictionViewerStages[_fictionViewerStageIndex];

	if (strcmp(stage.story_filename, storyFile.c_str()) != 0) {
		strcpy_s(stage.story_filename, storyFile.c_str());
		set_modified();
	}
}

SCP_string FictionViewerDialogModel::getFontFile() const
{
	return _fictionViewerStages[_fictionViewerStageIndex].font_filename;
}

void FictionViewerDialogModel::setFontFile(const SCP_string& fontFile)
{
	auto& stage = _fictionViewerStages[_fictionViewerStageIndex];

	if (stricmp(stage.font_filename, fontFile.c_str()) != 0) {
		strcpy_s(stage.font_filename, fontFile.c_str());
		set_modified();
	}
}

SCP_string FictionViewerDialogModel::getVoiceFile() const
{
	return _fictionViewerStages[_fictionViewerStageIndex].voice_filename;
}

void FictionViewerDialogModel::setVoiceFile(const SCP_string& voiceFile)
{
	auto& stage = _fictionViewerStages[_fictionViewerStageIndex];

	if (stricmp(stage.voice_filename, voiceFile.c_str()) != 0) {
		strcpy_s(stage.voice_filename, voiceFile.c_str());
		set_modified();
	}
}

int FictionViewerDialogModel::getFictionMusic() const
{
	// TODO research how music is set for multiple fiction viewer stages so we
	// can return the correct index when multiple stages is fully supported
	return _fictionMusic;
}

void FictionViewerDialogModel::setFictionMusic(int fictionMusic) {
	bool valid = fictionMusic == -1 || SCP_vector_inbounds(Spooled_music, fictionMusic);
	Assertion(valid,
		"Fiction music index out of bounds: %d (max %d)",
		fictionMusic,
		static_cast<int>(Spooled_music.size()));

	modify(_fictionMusic, fictionMusic);
}

int FictionViewerDialogModel::getMaxStoryFileLength() const
{
	auto& stage = _fictionViewerStages[_fictionViewerStageIndex];

	return sizeof(stage.story_filename) - 1;
}

int FictionViewerDialogModel::getMaxFontFileLength() const
{
	auto& stage = _fictionViewerStages[_fictionViewerStageIndex];

	return sizeof(stage.font_filename) - 1;
}

int FictionViewerDialogModel::getMaxVoiceFileLength() const
{
	auto& stage = _fictionViewerStages[_fictionViewerStageIndex];

	return sizeof(stage.voice_filename) - 1;
}

} // namespace fso::fred::dialogs
