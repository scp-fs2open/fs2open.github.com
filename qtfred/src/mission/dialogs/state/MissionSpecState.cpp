// captureState() and restoreState() for MissionSpecDialogModel.
// Reads/writes The_mission.* live data directly so that undo/redo bypasses the
// model's working copy and avoids double-mangling from lcl_fred_replace_stuff().

#include <mission/dialogs/MissionSpecDialogModel.h>

#include <ai/ai.h>
#include <ai/ai_profiles.h>
#include <gamesnd/eventmusic.h>
#include <globalincs/globals.h>
#include <mission/Editor.h>
#include <mission/missionparse.h>
#include <weapon/weapon.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QString>

namespace fso::fred::dialogs {

QByteArray MissionSpecDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	// Mission identity strings
	ds << QString::fromStdString(The_mission.name);
	ds << QString::fromStdString(The_mission.author);
	ds << QString(The_mission.notes);
	ds << QString(The_mission.mission_desc);
	ds << QString(The_mission.loading_screen[GR_640]);
	ds << QString(The_mission.loading_screen[GR_1024]);
	ds << QString(The_mission.squad_name);
	ds << QString(The_mission.squad_filename);

	// Game type and multiplayer
	ds << static_cast<qint32>(The_mission.game_type);
	ds << static_cast<quint32>(The_mission.num_respawns);
	ds << static_cast<qint32>(The_mission.max_respawn_delay);
	ds << static_cast<float>(f2fl(Entry_delay_time));

	// Large ship collision exclusion group
	ds << static_cast<qint32>(The_mission.large_ship_no_collide_collision_group);

	// Support ships
	ds << static_cast<qint32>(The_mission.support_ships.max_support_ships);
	ds << The_mission.support_ships.max_hull_repair_val;
	ds << The_mission.support_ships.max_subsys_repair_val;
	ds << static_cast<qint8>(The_mission.support_ships.disallow_rearm ? 1 : 0);
	ds << static_cast<qint8>(The_mission.support_ships.rearm_pool_from_loadout ? 1 : 0);
	ds << static_cast<qint8>(The_mission.support_ships.allow_rearm_weapon_precedence ? 1 : 0);
	// rearm_weapon_pool — only serialize weapon_info_size() entries per team
	const auto numWeapons = static_cast<qint32>(weapon_info_size());
	ds << numWeapons;
	for (const auto& pool : The_mission.support_ships.rearm_weapon_pool) {
		for (int i = 0; i < numWeapons; ++i)
			ds << static_cast<qint32>(pool[i]);
	}

	// Mission flags as a 64-bit integer
	ds << static_cast<quint64>(The_mission.flags.to_u64());

	// Contrail threshold
	ds << static_cast<qint32>(The_mission.contrail_threshold);

	// AI profile index
	ds << static_cast<qint32>(AI_PROFILES_INDEX(The_mission.ai_profile));

	// Music
	ds << static_cast<qint32>(Current_soundtrack_num);
	ds << QString(The_mission.substitute_event_music_name);

	// Command persona / sender
	ds << static_cast<qint32>(The_mission.command_persona);
	ds << QString(The_mission.command_sender);

	// Mission all attack
	ds << static_cast<qint32>(Mission_all_attack);

	// Custom data (map<SCP_string, SCP_string>)
	ds << static_cast<qint32>(static_cast<int>(The_mission.custom_data.size()));
	for (const auto& [k, v] : The_mission.custom_data)
		ds << QString(k.c_str()) << QString(v.c_str());

	// Custom strings (vector<custom_string>: name, value, text)
	ds << static_cast<qint32>(static_cast<int>(The_mission.custom_strings.size()));
	for (const auto& cs : The_mission.custom_strings)
		ds << QString(cs.name.c_str()) << QString(cs.value.c_str()) << QString(cs.text.c_str());

	// Sound environment
	ds << static_cast<qint32>(The_mission.sound_environment.id);
	ds << The_mission.sound_environment.volume;
	ds << The_mission.sound_environment.damping;
	ds << The_mission.sound_environment.decay;

	// Wing name arrays
	for (const auto& name : Starting_wing_names)
		ds << QString(name);
	for (const auto& name : Squadron_wing_names)
		ds << QString(name);
	for (const auto& name : TVT_wing_names)
		ds << QString(name);

	return data;
}

void MissionSpecDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	QString str;
	qint32  i32;
	quint32 u32;
	quint64 u64;
	float   f;

	// Mission identity strings
	QString name, author, notes, desc, loading640, loading1024, squad_name, squad_filename;
	ds >> name >> author >> notes >> desc >> loading640 >> loading1024 >> squad_name >> squad_filename;

	The_mission.name   = name.toStdString();
	The_mission.author = author.toStdString();
	strncpy(The_mission.notes, notes.toUtf8().constData(), NOTES_LENGTH - 1);
	The_mission.notes[NOTES_LENGTH - 1] = '\0';
	strncpy(The_mission.mission_desc, desc.toUtf8().constData(), MISSION_DESC_LENGTH - 1);
	The_mission.mission_desc[MISSION_DESC_LENGTH - 1] = '\0';
	strncpy(The_mission.loading_screen[GR_640], loading640.toUtf8().constData(), NAME_LENGTH - 1);
	The_mission.loading_screen[GR_640][NAME_LENGTH - 1] = '\0';
	strncpy(The_mission.loading_screen[GR_1024], loading1024.toUtf8().constData(), NAME_LENGTH - 1);
	The_mission.loading_screen[GR_1024][NAME_LENGTH - 1] = '\0';
	strncpy(The_mission.squad_name, squad_name.toUtf8().constData(), NAME_LENGTH - 1);
	The_mission.squad_name[NAME_LENGTH - 1] = '\0';
	strncpy(The_mission.squad_filename, squad_filename.toUtf8().constData(), MAX_FILENAME_LEN - 1);
	The_mission.squad_filename[MAX_FILENAME_LEN - 1] = '\0';

	// Game type and multiplayer
	ds >> i32; The_mission.game_type = i32;
	ds >> u32; The_mission.num_respawns = u32;
	ds >> i32; The_mission.max_respawn_delay = i32;
	ds >> f;   Entry_delay_time = fl2f(f);

	// Recompute derived Num_teams
	Num_teams = 1;
	if ((The_mission.game_type & MISSION_TYPE_MULTI) && (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS))
		Num_teams = 2;

	// Large ship collision exclusion group
	ds >> i32; The_mission.large_ship_no_collide_collision_group = i32;

	// Support ships
	ds >> i32; The_mission.support_ships.max_support_ships = i32;
	ds >> f;   The_mission.support_ships.max_hull_repair_val = f;
	ds >> f;   The_mission.support_ships.max_subsys_repair_val = f;
	qint8 b8;
	ds >> b8; The_mission.support_ships.disallow_rearm             = (b8 != 0);
	ds >> b8; The_mission.support_ships.rearm_pool_from_loadout    = (b8 != 0);
	ds >> b8; The_mission.support_ships.allow_rearm_weapon_precedence = (b8 != 0);
	qint32 numWeapons;
	ds >> numWeapons;
	for (auto& pool : The_mission.support_ships.rearm_weapon_pool) {
		for (int i = 0; i < numWeapons; ++i) {
			ds >> i32;
			if (i < MAX_WEAPON_TYPES) {
				pool[i] = i32;
			}
		}
	}

	// Mission flags
	ds >> u64; The_mission.flags.from_u64(u64);

	// Contrail threshold
	ds >> i32; The_mission.contrail_threshold = i32;

	// AI profile
	ds >> i32;
	if (i32 >= 0 && i32 < Num_ai_profiles)
		The_mission.ai_profile = &Ai_profiles[i32];

	// Music
	ds >> i32; Current_soundtrack_num = i32;
	ds >> str; strcpy_s(The_mission.substitute_event_music_name, str.toUtf8().constData());

	// Command persona / sender
	ds >> i32; The_mission.command_persona = i32;
	ds >> str; strcpy_s(The_mission.command_sender, str.toUtf8().constData());

	// Mission all attack
	ds >> i32; Mission_all_attack = i32;

	// Custom data
	ds >> i32;
	The_mission.custom_data.clear();
	for (int j = 0; j < i32; j++) {
		QString k, v;
		ds >> k >> v;
		The_mission.custom_data[k.toUtf8().constData()] = v.toUtf8().constData();
	}

	// Custom strings
	ds >> i32;
	The_mission.custom_strings.clear();
	The_mission.custom_strings.reserve(static_cast<size_t>(i32));
	for (int j = 0; j < i32; j++) {
		QString nm, val, txt;
		ds >> nm >> val >> txt;
		custom_string cs;
		cs.name  = nm.toUtf8().constData();
		cs.value = val.toUtf8().constData();
		cs.text  = txt.toUtf8().constData();
		The_mission.custom_strings.push_back(std::move(cs));
	}

	// Sound environment
	ds >> i32; The_mission.sound_environment.id = i32;
	ds >> The_mission.sound_environment.volume;
	ds >> The_mission.sound_environment.damping;
	ds >> The_mission.sound_environment.decay;

	// Wing name arrays
	for (auto& wingName : Starting_wing_names) {
		ds >> str;
		strcpy_s(wingName, str.toUtf8().constData());
	}
	for (auto& wingName : Squadron_wing_names) {
		ds >> str;
		strcpy_s(wingName, str.toUtf8().constData());
	}
	for (auto& wingName : TVT_wing_names) {
		ds >> str;
		strcpy_s(wingName, str.toUtf8().constData());
	}

	Editor::update_custom_wing_indexes();
}

} // namespace fso::fred::dialogs
