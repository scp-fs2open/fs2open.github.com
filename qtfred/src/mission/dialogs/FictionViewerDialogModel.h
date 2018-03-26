#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class FictionViewerDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	 struct MusicOptionElement {
		 SCP_string name;
		 int id = -1;

		 MusicOptionElement(const char* Name, int Id)
		 : name(Name), id(Id) {
		 }
	 };

	FictionViewerDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;

	void reject() override;

	const SCP_string& getStoryFile() const { return _storyFile; }
	const SCP_string& getFontFile() const { return _fontFile; }
	const SCP_string& getVoiceFile() const { return _voiceFile; }
	int getFictionMusic() const { return _fictionMusic; }
	const SCP_vector<MusicOptionElement>& getMusicOptions() const { return _musicOptions; }

	void setStoryFile(const SCP_string& storyFile) { _storyFile = storyFile; }
	void setFontFile(const SCP_string& fontFile) { _fontFile = fontFile; }
	void setVoiceFile(const SCP_string& voiceFile) { _voiceFile = voiceFile; }
	// TODO input validation on passed in fictionMusic?
	void setFictionMusic(int fictionMusic) { _fictionMusic = fictionMusic; }

	int getMaxStoryFileLength() const { return _maxStoryFileLength; }
	int getMaxFontFileLength() const { return _maxFontFileLength; }
	int getMaxVoiceFileLength() const { return _maxVoiceFileLength; }
 private:
	void initializeData();
	bool hasUnsavedChanges() const;

	SCP_string _storyFile;
	SCP_string _fontFile;
	SCP_string _voiceFile;
	int		_fictionMusic;
	SCP_vector<MusicOptionElement> _musicOptions;

	int _maxStoryFileLength, _maxFontFileLength, _maxVoiceFileLength;

	// TODO do I need this?
	// bool _enabled = false;
};

}
}
}
