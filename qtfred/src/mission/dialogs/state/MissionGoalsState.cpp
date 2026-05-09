// captureState() and restoreState() for MissionGoalsDialogModel.
// Snapshots Mission_goals from the live global so that undo/redo can restore
// goal names, types, messages, scores, flags, and team assignments.
//
// TODO(sexp_tree_refactor): mission_goal::formula is not serialized.
// formula is set to -1 ("no condition") on restore; sexp content is lost.

#include <mission/dialogs/MissionGoalsDialogModel.h>

#include "DialogStateHelpers.h"
#include <mission/missiongoals.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray MissionGoalsDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(static_cast<int>(Mission_goals.size()));
	for (const mission_goal& g : Mission_goals) {
		ds << QString::fromStdString(g.name);
		ds << static_cast<qint32>(g.type);
		ds << static_cast<qint32>(g.satisfied);
		ds << QString::fromStdString(g.message);
		fso::fred::state::writeSexpStub(ds); // formula — TODO(sexp_tree_refactor)
		ds << static_cast<qint32>(g.score);
		ds << static_cast<qint32>(g.flags);
		ds << static_cast<qint32>(g.team);
	}

	return data;
}

void MissionGoalsDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 count;
	ds >> count;

	Mission_goals.clear();
	Mission_goals.resize(static_cast<size_t>(count));

	for (int i = 0; i < static_cast<int>(count); ++i) {
		mission_goal& g = Mission_goals[static_cast<size_t>(i)];
		QString name, message;
		qint32 type, satisfied, score, flags, team;

		ds >> name >> type >> satisfied >> message;
		fso::fred::state::readSexpStub(ds); // formula — TODO(sexp_tree_refactor)
		g.formula = -1;
		ds >> score >> flags >> team;

		g.name      = name.toStdString();
		g.type      = static_cast<int>(type);
		g.satisfied = static_cast<int>(satisfied);
		g.message   = message.toStdString();
		g.score     = static_cast<int>(score);
		g.flags     = static_cast<int>(flags);
		g.team      = static_cast<int>(team);
	}
}

} // namespace fso::fred::dialogs
