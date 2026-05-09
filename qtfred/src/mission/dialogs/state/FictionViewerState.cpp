// captureState() and restoreState() for FictionViewerDialogModel.
// Snapshots all fiction viewer stages and music selection from live globals.
//
// TODO(sexp_tree_refactor): fiction_viewer_stage::formula is not serialized.
// Replace writeSexpStub/readSexpStub with real sexp tree serialization.

#include <mission/dialogs/FictionViewerDialogModel.h>

#include "DialogStateHelpers.h"
#include <gamesnd/eventmusic.h>
#include <globalincs/globals.h>
#include <missionui/fictionviewer.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray FictionViewerDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(static_cast<int>(Fiction_viewer_stages.size()));
	for (const fiction_viewer_stage& stage : Fiction_viewer_stages) {
		ds << QString::fromLatin1(stage.story_filename);
		ds << QString::fromLatin1(stage.font_filename);
		ds << QString::fromLatin1(stage.voice_filename);
		ds << QString::fromLatin1(stage.ui_name);
		for (const auto& background : stage.background)
			ds << QString::fromLatin1(background);
		fso::fred::state::writeSexpStub(ds); // formula — TODO(sexp_tree_refactor)
	}

	ds << static_cast<qint32>(Mission_music[SCORE_FICTION_VIEWER]);

	return data;
}

void FictionViewerDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 count;
	ds >> count;

	Fiction_viewer_stages.clear();
	Fiction_viewer_stages.resize(static_cast<size_t>(count));

	for (int i = 0; i < count; ++i) {
		fiction_viewer_stage& stage = Fiction_viewer_stages[static_cast<size_t>(i)];
		QString story, font, voice, uiName;
		ds >> story >> font >> voice >> uiName;
		strncpy(stage.story_filename, story.toLatin1().constData(), MAX_FILENAME_LEN - 1);
		stage.story_filename[MAX_FILENAME_LEN - 1] = '\0';
		strncpy(stage.font_filename, font.toLatin1().constData(), MAX_FILENAME_LEN - 1);
		stage.font_filename[MAX_FILENAME_LEN - 1] = '\0';
		strncpy(stage.voice_filename, voice.toLatin1().constData(), MAX_FILENAME_LEN - 1);
		stage.voice_filename[MAX_FILENAME_LEN - 1] = '\0';
		strncpy(stage.ui_name, uiName.toLatin1().constData(), NAME_LENGTH - 1);
		stage.ui_name[NAME_LENGTH - 1] = '\0';
		for (auto& background : stage.background) {
			QString bg;
			ds >> bg;
			strncpy(background, bg.toLatin1().constData(), MAX_FILENAME_LEN - 1);
			background[MAX_FILENAME_LEN - 1] = '\0';
		}
		fso::fred::state::readSexpStub(ds); // formula — TODO(sexp_tree_refactor)
	}

	qint32 music;
	ds >> music;
	Mission_music[SCORE_FICTION_VIEWER] = static_cast<int>(music);
}

} // namespace fso::fred::dialogs
