// captureState() and restoreState() for ReinforcementsDialogModel.
// Snapshots Reinforcements[] so that undo/redo can restore the
// reinforcement list to its pre-accept state.

#include <mission/dialogs/ReinforcementsEditorDialogModel.h>

#include <globalincs/globals.h>
#include <ship/ship.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray ReinforcementsDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(Reinforcements.size());

	for (const auto& r : Reinforcements) {
		ds << QString::fromLatin1(r.name);
		ds << static_cast<qint32>(r.type);
		ds << static_cast<qint32>(r.uses);
		ds << static_cast<qint32>(r.arrival_delay);

		for (const auto& msg : r.no_messages)
			ds << QString::fromLatin1(msg);
		for (const auto& msg : r.yes_messages)
			ds << QString::fromLatin1(msg);
	}

	return data;
}

void ReinforcementsDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 num;
	ds >> num;
	Reinforcements.resize(static_cast<size_t>(num));

	for (auto& r : Reinforcements) {
		QString name;
		qint32 type, uses, arrival_delay;
		ds >> name >> type >> uses >> arrival_delay;

		strncpy(r.name, name.toLatin1().constData(), NAME_LENGTH - 1);
		r.name[NAME_LENGTH - 1] = '\0';
		r.type          = static_cast<int>(type);
		r.uses          = static_cast<int>(uses);
		r.arrival_delay = static_cast<int>(arrival_delay);

		for (auto& slot : r.no_messages) {
			QString msg;
			ds >> msg;
			strncpy(slot, msg.toLatin1().constData(), NAME_LENGTH - 1);
			slot[NAME_LENGTH - 1] = '\0';
		}
		for (auto& slot : r.yes_messages) {
			QString msg;
			ds >> msg;
			strncpy(slot, msg.toLatin1().constData(), NAME_LENGTH - 1);
			slot[NAME_LENGTH - 1] = '\0';
		}
	}
}

} // namespace fso::fred::dialogs
