#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso::fred::dialogs {

class MusicPlayerDialogModel final : public AbstractDialogModel {
	Q_OBJECT
  public:
	explicit MusicPlayerDialogModel(QObject* parent, EditorViewport* viewport);
	~MusicPlayerDialogModel() override = default;

	bool apply() override;
	void reject() override;

	// lifecycle
	void loadTracks();

	// data
	const SCP_vector<SCP_string>& tracks() const
	{
		return _tracks;
	}
	int currentRow() const
	{
		return _currentRow;
	}
	void setCurrentRow(int row);

	// playback
	bool isPlaying() const;
	void play();
	void stop();
	bool selectNext(); // advances selection (returns true if changed)
	bool selectPrev(); // advances selection (returns true if changed)

	// autoplay
	bool autoplay() const
	{
		return _autoplay;
	}
	void setAutoplay(bool on)
	{
		modify(_autoplay, on);
	}

	// polling tick (call from a QTimer in the dialog)
	void tick();

  private:
	SCP_string currentItemName() const;             // without extension
	static int tryOpenStream(const SCP_string& baseNoExt); // .wav first, then .ogg

	SCP_vector<SCP_string> _tracks;
	int _currentRow = -1;
	int _musicId = -1; // audiostream id or -1 when none
	bool _autoplay = false;
};

} // namespace fso::fred::dialogs