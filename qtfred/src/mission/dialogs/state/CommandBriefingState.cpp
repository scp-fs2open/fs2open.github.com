// captureState() and restoreState() for CommandBriefingDialogModel.
// Snapshots all command briefing stages from the live globals so that
// undo/redo can restore them without reopening the dialog.
// No sexp fields — serialization is complete.

#include <mission/dialogs/CommandBriefingDialogModel.h>

#include <globalincs/globals.h>
#include <missionui/missioncmdbrief.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray CommandBriefingDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	for (const auto& cb : Cmd_briefs) {
		ds << static_cast<qint32>(cb.num_stages);

		for (const auto& stage : cb.stage) {
			ds << QString::fromStdString(stage.text);
			ds << QString::fromLatin1(stage.ani_filename);
			ds << QString::fromLatin1(stage.wave_filename);
		}

		for (const auto& background : cb.background)
			ds << QString::fromLatin1(background);
	}

	return data;
}

void CommandBriefingDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	for (auto& cb : Cmd_briefs) {
		qint32 numStages;
		ds >> numStages;
		cb.num_stages = static_cast<int>(numStages);

		for (auto& stage : cb.stage) {
			QString text, ani, wave;
			ds >> text >> ani >> wave;
			stage.text = text.toStdString();
			strncpy(stage.ani_filename, ani.toLatin1().constData(), MAX_FILENAME_LEN - 1);
			stage.ani_filename[MAX_FILENAME_LEN - 1] = '\0';
			strncpy(stage.wave_filename, wave.toLatin1().constData(), MAX_FILENAME_LEN - 1);
			stage.wave_filename[MAX_FILENAME_LEN - 1] = '\0';
			stage.wave = -1; // runtime field, not persisted
		}

		for (auto& background : cb.background) {
			QString bg;
			ds >> bg;
			strncpy(background, bg.toLatin1().constData(), MAX_FILENAME_LEN - 1);
			background[MAX_FILENAME_LEN - 1] = '\0';
		}
	}
}

} // namespace fso::fred::dialogs
