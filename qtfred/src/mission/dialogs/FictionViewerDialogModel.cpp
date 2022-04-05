#include <missionui/fictionviewer.h>
#include <gamesnd/eventmusic.h>
#include "mission/dialogs/FictionViewerDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

FictionViewerDialogModel::FictionViewerDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {

	initializeData();
}

bool FictionViewerDialogModel::apply() {
	// store the fields in the data structure
	fiction_viewer_stage *stagep = &Fiction_viewer_stages.at(0);
	if (_storyFile.empty()) {
		Fiction_viewer_stages.erase(Fiction_viewer_stages.begin());
		stagep = nullptr;
		Mission_music[SCORE_FICTION_VIEWER] = -1;
	} else {
		strcpy_s(stagep->story_filename, _storyFile.c_str());
		strcpy_s(stagep->font_filename, _fontFile.c_str());
		strcpy_s(stagep->voice_filename, _voiceFile.c_str());
		Mission_music[SCORE_FICTION_VIEWER] = _fictionMusic - 1;
	}
	return true;
}

void FictionViewerDialogModel::reject() {
	// nothing to do if the dialog is created each time it's opened
}

void FictionViewerDialogModel::initializeData() {
	// make sure we have at least one stage
	if (Fiction_viewer_stages.empty()) {
		fiction_viewer_stage stage;
		memset(&stage, 0, sizeof(fiction_viewer_stage));
		stage.formula = Locked_sexp_true;

		Fiction_viewer_stages.push_back(stage);
	}
	
	_musicOptions.emplace_back("None", 0);
	for (int i = 0; i < (int)Spooled_music.size(); ++i) {
		_musicOptions.emplace_back(Spooled_music[i].name, i + 1); // + 1 because option 0 is None
	}
	
	// init fields based on first fiction viewer stage
	const fiction_viewer_stage *stagep = &Fiction_viewer_stages.at(0);
	_storyFile = stagep->story_filename;
	_fontFile = stagep->font_filename;
	_voiceFile = stagep->voice_filename;

	// initialize file name length limits
	_maxStoryFileLength = sizeof(stagep->story_filename) - 1;
	_maxFontFileLength = sizeof(stagep->font_filename) - 1;
	_maxVoiceFileLength = sizeof(stagep->voice_filename) - 1;

	// music is managed through the mission
	_fictionMusic = Mission_music[SCORE_FICTION_VIEWER] + 1;

	modelChanged();
}

void FictionViewerDialogModel::setFictionMusic(int fictionMusic) {
	Assert(fictionMusic >= 0);
	Assert(fictionMusic <= (int)Spooled_music.size());
	modify<int>(_fictionMusic, fictionMusic);
}

bool FictionViewerDialogModel::query_modified() const {
	Assert(!Fiction_viewer_stages.empty());

	const fiction_viewer_stage *stagep = &Fiction_viewer_stages.at(0);
	
	return strcmp(_storyFile.c_str(), stagep->story_filename) != 0
	    || strcmp(_fontFile.c_str(), stagep->font_filename) != 0
		|| strcmp(_voiceFile.c_str(), stagep->voice_filename) != 0
		|| _fictionMusic != (Mission_music[SCORE_FICTION_VIEWER] + 1);
}

bool FictionViewerDialogModel::hasMultipleStages() const {
	return Fiction_viewer_stages.size() > 1;
}

}
}
}
