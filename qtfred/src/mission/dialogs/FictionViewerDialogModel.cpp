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
	if (strlen(stagep->story_filename) == 0) {
		Fiction_viewer_stages.erase(Fiction_viewer_stages.begin());
		stagep = nullptr;
		Mission_music[SCORE_FICTION_VIEWER] = -1; // TODO right?
	} else {
		strcpy_s(stagep->story_filename, _storyFile.c_str());
		strcpy_s(stagep->font_filename, _fontFile.c_str());
		strcpy_s(stagep->voice_filename, _voiceFile.c_str());
		Mission_music[SCORE_FICTION_VIEWER] = _fictionMusic - 1;
	}
	return true;
}

void FictionViewerDialogModel::reject() {
	// TODO what to do here depends on whether dialog is created each time the user opens it or just created once
	// currently assuming it's just once
	if (!Fiction_viewer_stages.empty()) {
		// init fields based on first fiction viewer stage
		const fiction_viewer_stage *stagep = &Fiction_viewer_stages.at(0);
		_storyFile = stagep->story_filename;
		_fontFile = stagep->font_filename;
		_voiceFile = stagep->voice_filename;

		_fictionMusic = Mission_music[SCORE_FICTION_VIEWER] + 1;
	} else {
		_storyFile = "";
		_fontFile = "";
		_voiceFile = "";
		_fictionMusic = 0;
	}
}

// TODO!
void FictionViewerDialogModel::initializeData() {
	// make sure we have at least one stage
	if (Fiction_viewer_stages.empty()) {
		fiction_viewer_stage stage;
		memset(&stage, 0, sizeof(fiction_viewer_stage));
		stage.formula = Locked_sexp_true;

		Fiction_viewer_stages.push_back(stage);
	}
	// FIXME TODO figure out what to do here
	// else if (Fiction_viewer_stages.size() > 1)
	// {
		// MessageBox("You have multiple fiction viewer stages defined for this mission.  At present, FRED will only allow you to edit the first stage.");
	// }
	
	_musicOptions.emplace_back("None", 0);
	for (int i = 0; i < Num_music_files; ++i) {
		_musicOptions.emplace_back(Spooled_music[i].name, i + 1); // + 1 because option 0 is None
	}
	
	// init fields based on first fiction viewer stage
	const fiction_viewer_stage *stagep = &Fiction_viewer_stages.at(0);
	_storyFile = stagep->story_filename;
	_fontFile = stagep->font_filename;
	_voiceFile = stagep->voice_filename;

	// initialize file name length limits
	_maxStoryFileLength = sizeof(stagep->story_filename);
	_maxFontFileLength = sizeof(stagep->font_filename);
	_maxVoiceFileLength = sizeof(stagep->voice_filename);

	// music is managed through the mission
	_fictionMusic = Mission_music[SCORE_FICTION_VIEWER] + 1;
}

bool FictionViewerDialogModel::hasUnsavedChanges() const {
	// TODO assert !Fiction_viewer_stages.empty() ?

	const fiction_viewer_stage *stagep = &Fiction_viewer_stages.at(0);
	
	return strcmp(_storyFile.c_str(), stagep->story_filename) != 0
	    || strcmp(_fontFile.c_str(), stagep->font_filename) != 0
		|| strcmp(_voiceFile.c_str(), stagep->voice_filename) != 0
		|| _fictionMusic != (Mission_music[SCORE_FICTION_VIEWER] + 1);
}


}
}
}
