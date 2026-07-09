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

// ---------------------------------------------------------------------------
// Working-state capture/restore for the in-dialog undo stack: the WIP
// reinforcement list and the remaining ship/wing pool. The selection is
// re-resolved against the restored list so the spinbox setters (which apply
// to the model-tracked selection) stay in bounds.
// ---------------------------------------------------------------------------

QByteArray ReinforcementsDialogModel::captureWorkingState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(_reinforcementList.size());
	for (const auto& r : _reinforcementList) {
		ds << QString::fromStdString(std::get<0>(r));
		ds << static_cast<qint32>(std::get<1>(r));
		ds << static_cast<qint32>(std::get<2>(r));
	}

	ds << static_cast<qint32>(_shipWingPool.size());
	for (const auto& name : _shipWingPool)
		ds << QString::fromStdString(name);

	return data;
}

void ReinforcementsDialogModel::restoreWorkingState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 count;
	ds >> count;
	_reinforcementList.clear();
	_reinforcementList.reserve(count);
	for (int i = 0; i < count; ++i) {
		QString name;
		qint32 uses, delay;
		ds >> name >> uses >> delay;
		_reinforcementList.emplace_back(name.toStdString(), static_cast<int>(uses), static_cast<int>(delay));
	}

	qint32 poolCount;
	ds >> poolCount;
	_shipWingPool.clear();
	_shipWingPool.reserve(poolCount);
	for (int i = 0; i < poolCount; ++i) {
		QString name;
		ds >> name;
		_shipWingPool.push_back(name.toStdString());
	}

	// Drop selected names that no longer exist, then rebuild the indices.
	_selectedReinforcements.erase(
		std::remove_if(_selectedReinforcements.begin(), _selectedReinforcements.end(),
			[this](const SCP_string& name) {
				return std::none_of(_reinforcementList.begin(), _reinforcementList.end(),
					[&](const std::tuple<SCP_string, int, int>& r) { return std::get<0>(r) == name; });
			}),
		_selectedReinforcements.end());
	updateSelectedIndices();

	set_modified();
}

} // namespace fso::fred::dialogs
