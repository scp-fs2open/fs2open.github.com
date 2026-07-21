// captureState() and restoreState() for MissionCutscenesDialogModel.
// Snapshots The_mission.cutscenes from the live global so that undo/redo
// can restore cutscene type, filename, and sexp formula assignments.

#include <mission/dialogs/MissionCutscenesDialogModel.h>

#include "DialogStateHelpers.h"
#include <globalincs/globals.h>
#include <mission/missionparse.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray MissionCutscenesDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(static_cast<int>(The_mission.cutscenes.size()));
	for (const mission_cutscene& c : The_mission.cutscenes) {
		ds << static_cast<qint32>(c.type);
		ds << QString::fromLatin1(c.filename);
		fso::fred::state::writeSexp(ds, c.formula);
	}

	return data;
}

void MissionCutscenesDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 count;
	ds >> count;

	for (const mission_cutscene& c : The_mission.cutscenes)
		fso::fred::state::freeSexpFormula(c.formula);
	The_mission.cutscenes.clear();
	The_mission.cutscenes.resize(static_cast<size_t>(count));

	for (int i = 0; i < static_cast<int>(count); ++i) {
		mission_cutscene& c = The_mission.cutscenes[static_cast<size_t>(i)];
		qint32 type;
		QString filename;

		ds >> type >> filename;
		c.formula = fso::fred::state::readSexp(ds);

		c.type = static_cast<int>(type);
		strncpy(c.filename, filename.toLatin1().constData(), MAX_FILENAME_LEN - 1);
		c.filename[MAX_FILENAME_LEN - 1] = '\0';
	}
}

} // namespace fso::fred::dialogs
