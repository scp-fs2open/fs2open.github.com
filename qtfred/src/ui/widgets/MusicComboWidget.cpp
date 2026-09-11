#include "ui/widgets/MusicComboWidget.h"
#include "ui/Theme.h"

#include <gamesnd/eventmusic.h>
#include <sound/audiostr.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>

namespace fso::fred {

MusicComboWidget::MusicComboWidget(QWidget* parent)
	: QWidget(parent), _comboBox(new QComboBox(this)), _playButton(new QPushButton(this))
{
	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(_comboBox);
	layout->addWidget(_playButton);

	// Populate combo: "None" first, then all Spooled_music entries.
	// Item data stores the Spooled_music index (-1 for None).
	_comboBox->addItem(tr("None"), -1);
	for (int i = 0; i < static_cast<int>(Spooled_music.size()); ++i) {
		_comboBox->addItem(QString::fromStdString(Spooled_music[i].name), i);
	}

	fso::fred::bindStandardIcon(_playButton, QStyle::SP_MediaPlay);
	_playButton->setEnabled(false); // disabled until a real track is selected

	connect(_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &MusicComboWidget::onComboChanged);
	connect(_playButton, &QPushButton::clicked, this, &MusicComboWidget::onPlayButtonClicked);
	connect(&_timer, &QTimer::timeout, this, &MusicComboWidget::onTimerTick);
	_timer.start(250);
}

MusicComboWidget::~MusicComboWidget()
{
	stopPlayback();
}

int MusicComboWidget::currentMusicIndex() const
{
	return _comboBox->currentData().value<int>();
}

void MusicComboWidget::setCurrentMusicIndex(int spooledIdx)
{
	const int pos = _comboBox->findData(spooledIdx);
	if (pos >= 0)
		_comboBox->setCurrentIndex(pos);
	// Signals may be blocked by the caller (e.g. SignalBlockers in updateUi), so
	// update the button state directly rather than relying on onComboChanged firing.
	_playButton->setEnabled(currentMusicIndex() >= 0);
}

void MusicComboWidget::stopPlayback()
{
	if (_streamId >= 0) {
		audiostream_close_file(_streamId, false);
		_streamId = -1;
	}
	fso::fred::bindStandardIcon(_playButton, QStyle::SP_MediaPlay);
}

void MusicComboWidget::onPlayButtonClicked()
{
	if (_streamId >= 0) {
		// Already playing — stop.
		stopPlayback();
		return;
	}

	const int smIdx = currentMusicIndex();
	if (smIdx < 0 || smIdx >= static_cast<int>(Spooled_music.size()))
		return;

	// Strip any extension already present in the filename before trying .wav then .ogg
	SCP_string base = Spooled_music[smIdx].filename;
	const auto dot = base.rfind('.');
	if (dot != SCP_string::npos)
		base = base.substr(0, dot);

	int id = audiostream_open((base + ".wav").c_str(), ASF_EVENTMUSIC);
	if (id < 0)
		id = audiostream_open((base + ".ogg").c_str(), ASF_EVENTMUSIC);

	if (id >= 0) {
		audiostream_play(id, 1.0f, 0);
		_streamId = id;
		fso::fred::bindStandardIcon(_playButton, QStyle::SP_MediaStop);
		Q_EMIT playbackStarted();
	}
}

void MusicComboWidget::onComboChanged(int /*comboIdx*/)
{
	// Stop any active preview when the user changes the selection.
	stopPlayback();

	const int smIdx = currentMusicIndex();
	_playButton->setEnabled(smIdx >= 0);
	Q_EMIT currentIndexChanged(smIdx);
}

void MusicComboWidget::onTimerTick()
{
	if (_streamId >= 0 && !audiostream_is_playing(_streamId)) {
		stopPlayback();
	}
}

} // namespace fso::fred
