#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

#include <missionui/fictionviewer.h>

namespace fso::fred::dialogs {

class FictionViewerDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	FictionViewerDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	const SCP_vector<std::pair<SCP_string, int>>& getMusicOptions();

	SCP_string getStoryFile() const;
	void setStoryFile(const SCP_string& storyFile);
	SCP_string getFontFile() const;
	void setFontFile(const SCP_string& fontFile);
	SCP_string getVoiceFile() const;
	void setVoiceFile(const SCP_string& voiceFile);
	int getFictionMusic() const;
	void setFictionMusic(int fictionMusic);

	int getMaxStoryFileLength() const;
	int getMaxFontFileLength() const;
	int getMaxVoiceFileLength() const;
 private:
	void initializeData();

	SCP_vector<fiction_viewer_stage> _fictionViewerStages;
	int _fictionViewerStageIndex = 0;
	int	_fictionMusic = -1;
	SCP_vector<std::pair<SCP_string, int>> _musicOptions;
};

} // namespace fso::fred::dialogs
