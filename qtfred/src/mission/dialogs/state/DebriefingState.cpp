// captureState() and restoreState() for DebriefingDialogModel.
// Snapshots all debriefing stages and music track selections from the live
// globals so that undo/redo can restore them without reopening the dialog.
//
// TODO(sexp_tree_refactor): debrief_stage::formula is not serialized yet.
// Replace writeSexpStub/readSexpStub with real sexp tree serialization.

#include <mission/dialogs/DebriefingDialogModel.h>

#include "DialogStateHelpers.h"
#include <gamesnd/eventmusic.h>
#include <globalincs/globals.h>
#include <mission/missionbriefcommon.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray DebriefingDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	for (const auto& deb : Debriefings) {
		ds << static_cast<qint32>(deb.num_stages);

		for (const auto& stage : deb.stages) {
			fso::fred::state::writeSexpStub(ds); // formula — TODO(sexp_tree_refactor)
			ds << QString::fromStdString(stage.text);
			ds << QString::fromLatin1(stage.voice);
			ds << QString::fromStdString(stage.recommendation_text);
		}

		for (const auto& background : deb.background)
			ds << QString::fromLatin1(background);
	}

	ds << static_cast<qint32>(Mission_music[SCORE_DEBRIEFING_SUCCESS]);
	ds << static_cast<qint32>(Mission_music[SCORE_DEBRIEFING_AVERAGE]);
	ds << static_cast<qint32>(Mission_music[SCORE_DEBRIEFING_FAILURE]);

	return data;
}

void DebriefingDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	for (auto& deb : Debriefings) {
		qint32 numStages;
		ds >> numStages;
		deb.num_stages = static_cast<int>(numStages);

		for (auto& stage : deb.stages) {
			QString text, voice, recText;
			fso::fred::state::readSexpStub(ds); // formula — TODO(sexp_tree_refactor)
			ds >> text >> voice >> recText;
			stage.text = text.toStdString();
			strncpy(stage.voice, voice.toLatin1().constData(), MAX_FILENAME_LEN - 1);
			stage.voice[MAX_FILENAME_LEN - 1] = '\0';
			stage.recommendation_text = recText.toStdString();
		}

		for (auto& background : deb.background) {
			QString bg;
			ds >> bg;
			strncpy(background, bg.toLatin1().constData(), MAX_FILENAME_LEN - 1);
			background[MAX_FILENAME_LEN - 1] = '\0';
		}
	}

	qint32 success, average, failure;
	ds >> success >> average >> failure;
	Mission_music[SCORE_DEBRIEFING_SUCCESS] = static_cast<int>(success);
	Mission_music[SCORE_DEBRIEFING_AVERAGE] = static_cast<int>(average);
	Mission_music[SCORE_DEBRIEFING_FAILURE] = static_cast<int>(failure);
}

} // namespace fso::fred::dialogs
