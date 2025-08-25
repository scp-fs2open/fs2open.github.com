#include <mod_table/mod_table.h>
#include <sound/audiostr.h>
#include <iff_defs/iff_defs.h>
#include "mission/dialogs/MusicPlayerDialogModel.h"

namespace fso::fred::dialogs {

MusicPlayerDialogModel::MusicPlayerDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	// No persisted state to prefill
}

bool MusicPlayerDialogModel::apply()
{
	// No changes to apply, just return true
	return true;
}

void MusicPlayerDialogModel::reject()
{
	stop(); // stop playback on dialog close
	// No other cleanup needed
}

void MusicPlayerDialogModel::loadTracks()
{
	_tracks.clear();

	SCP_vector<SCP_string> files;
	cf_get_file_list(files, CF_TYPE_MUSIC, "*", CF_SORT_NAME);

	for (const auto& f : files) {
		bool add = true;
		for (const auto& ignored : Ignored_music_player_files) {
			if (lcase_equal(ignored, f)) {
				add = false;
				break;
			}
		}
		if (add) {
			_tracks.push_back(f);
		}
	}
	// Reset selection when repopulating
	modify(_currentRow, -1);
}

void MusicPlayerDialogModel::setCurrentRow(int row)
{
	if (row < -1 || row >= static_cast<int>(_tracks.size()))
		return;
	modify(_currentRow, row);
}

SCP_string MusicPlayerDialogModel::currentItemName() const
{
	if (!SCP_vector_inbounds(_tracks, _currentRow))
		return "";
	return _tracks.at(_currentRow);
}

int MusicPlayerDialogModel::tryOpenStream(const SCP_string& baseNoExt)
{
	// cfile strips extensions in some contexts; try .wav then .ogg
	int id = audiostream_open((baseNoExt + ".wav").c_str(), ASF_EVENTMUSIC);
	if (id < 0) {
		id = audiostream_open((baseNoExt + ".ogg").c_str(), ASF_EVENTMUSIC);
	}
	return id;
}

bool MusicPlayerDialogModel::isPlaying() const
{
	return _musicId >= 0 && audiostream_is_playing(_musicId);
}

void MusicPlayerDialogModel::play()
{
	stop(); // close any previous stream as in the original
	const auto name = currentItemName();
	if (name.empty())
		return;

	_musicId = tryOpenStream(name);
	if (_musicId >= 0) {
		audiostream_play(_musicId, 1.0f, 0);
	} else {
		Warning(LOCATION, "FRED failed to open music file %s in the music player\n", name.c_str());
	}
}

void MusicPlayerDialogModel::stop()
{
	if (_musicId >= 0) {
		audiostream_close_file(_musicId, false);
		_musicId = -1;
	}
}

bool MusicPlayerDialogModel::selectNext()
{
	if (_currentRow >= 0 && _currentRow < static_cast<int>(_tracks.size()) - 1) {
		modify(_currentRow, _currentRow + 1);
		return true;
	}
	return false;
}

bool MusicPlayerDialogModel::selectPrev()
{
	if (_currentRow > 0 && _currentRow < static_cast<int>(_tracks.size())) {
		modify(_currentRow, _currentRow - 1);
		return true;
	}
	return false;
}

void MusicPlayerDialogModel::tick()
{
	// If playback just finished: autoplay advances and plays; else stop
	if (_musicId >= 0 && !audiostream_is_playing(_musicId)) {
		if (_autoplay && selectNext()) {
			play();
		} else {
			stop();
		}
	}
}

} // namespace fso::fred::dialogs