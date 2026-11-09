#pragma once

#include <QWidget>
#include <QTimer>

class QComboBox;
class QPushButton;

namespace fso::fred {

// A combo box pre-populated with Spooled_music entries ("None" + all tracks)
// combined with a play/stop toggle button for auditioning the selected track.
// Place multiple instances in the same dialog and connect their playbackStarted()
// signals to each other's stopPlayback() slots to enforce single-track playback.
class MusicComboWidget : public QWidget {
	Q_OBJECT

  public:
	explicit MusicComboWidget(QWidget* parent = nullptr);
	~MusicComboWidget() override;

	// Returns the Spooled_music index of the current selection, or -1 for "None".
	int currentMusicIndex() const;

	// Selects the entry matching the given Spooled_music index (-1 selects "None").
	void setCurrentMusicIndex(int spooledIdx);

	// Stops any active playback and resets the button to its Play state.
	void stopPlayback();

  signals:
	// Emitted when the user changes the combo selection.
	// spooledMusicIdx is -1 for "None", or a valid index into Spooled_music.
	void currentIndexChanged(int spooledMusicIdx);

	// Emitted when playback starts, so sibling widgets can stop themselves.
	void playbackStarted();

  private slots:
	void onPlayButtonClicked();
	void onComboChanged(int comboIdx);
	void onTimerTick();

  private: // NOLINT(readability-redundant-access-specifiers)
	QComboBox* _comboBox;
	QPushButton* _playButton;
	QTimer _timer;
	int _streamId = -1;
};

} // namespace fso::fred
