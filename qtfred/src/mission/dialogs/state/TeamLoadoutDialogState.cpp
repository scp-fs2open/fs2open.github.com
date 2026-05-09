// captureState() and restoreState() for TeamLoadoutDialogModel.
// Snapshots Team_data[] (ship/weapon pool lists and required flags) so that
// undo/redo can restore the loadout to its pre-accept state.

#include <mission/dialogs/TeamLoadoutDialogModel.h>

#include <globalincs/globals.h>
#include <mission/missionparse.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

static void writeLoadoutEntries(QDataStream& ds, const SCP_vector<loadout_entry>& entries)
{
	ds << static_cast<qint32>(entries.size());
	for (const auto& entry : entries) {
		ds << static_cast<qint32>(entry.class_index);
		ds << static_cast<qint32>(entry.count);
		ds << QString::fromStdString(entry.class_variable);
		ds << QString::fromStdString(entry.count_variable);
	}
}

static void readLoadoutEntries(QDataStream& ds, SCP_vector<loadout_entry>& entries)
{
	entries.clear();
	qint32 count;
	ds >> count;
	entries.reserve(count);
	for (int i = 0; i < count; ++i) {
		auto& entry = entries.emplace_back();
		qint32 classIndex, entryCount;
		QString classVar, countVar;
		ds >> classIndex >> entryCount >> classVar >> countVar;
		entry.class_index    = static_cast<int>(classIndex);
		entry.count          = static_cast<int>(entryCount);
		entry.class_variable = classVar.toStdString();
		entry.count_variable = countVar.toStdString();
	}
}

QByteArray TeamLoadoutDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(Num_teams);

	for (int t = 0; t < Num_teams; ++t) {
		const team_data& td = Team_data[t];

		ds << static_cast<qint32>(td.default_ship);

		// Ships
		writeLoadoutEntries(ds, td.ship_choices);

		// Weapons
		ds << static_cast<quint8>(td.do_not_validate ? 1 : 0);
		writeLoadoutEntries(ds, td.weapon_choices);

		// required_weapons holds weapon class indices, not pool positions
		ds << static_cast<qint32>(td.required_weapons.size());
		for (int weapon_class : td.required_weapons)
			ds << static_cast<qint32>(weapon_class);
	}

	return data;
}

void TeamLoadoutDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	qint32 num_teams;
	ds >> num_teams;

	for (int t = 0; t < static_cast<int>(num_teams) && t < MAX_TVT_TEAMS; ++t) {
		team_data& td = Team_data[t];

		qint32 default_ship;
		ds >> default_ship;
		td.default_ship = static_cast<int>(default_ship);

		readLoadoutEntries(ds, td.ship_choices);

		quint8 do_not_validate;
		ds >> do_not_validate;
		td.do_not_validate = (do_not_validate != 0);

		readLoadoutEntries(ds, td.weapon_choices);

		qint32 req_count;
		ds >> req_count;
		td.required_weapons.clear();
		for (int i = 0; i < static_cast<int>(req_count); ++i) {
			qint32 weapon_class;
			ds >> weapon_class;
			td.required_weapons.insert(static_cast<int>(weapon_class));
		}
	}
}

} // namespace fso::fred::dialogs
