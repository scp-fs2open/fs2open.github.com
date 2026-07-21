// captureWorkingState() and restoreWorkingState() for CampaignEditorDialogModel.
//
// The campaign editor is file-scoped: there is no apply-into-mission step and
// no whole-dialog ApplyDialogCommand, so this is the only serialization it
// needs — snapshots for the in-dialog undo stack. The blob covers the whole
// working copy: specs, flags, custom data, tech arrays, and the mission list
// with branches. Branch conditions are internal tree ids, so they round-trip
// through Sexp_nodes via the tree ops (saveSexp -> writeSexp / readSexp ->
// loadSexp).
//
// Deliberately excluded: the campaign filename (save-scoped), the
// available-missions list (derived from disk and the mission list; restore
// reloads it), and the mission/branch selections (they change without a stack
// push, which would break the blob-equality check that absorbs the tree's
// redundant modified() signals).

#include <mission/dialogs/CampaignEditorDialogModel.h>

#include "DialogStateHelpers.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

QByteArray CampaignEditorDialogModel::captureWorkingState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << QString::fromStdString(m_campaign_name);
	ds << QString::fromStdString(m_campaign_descr);
	ds << static_cast<qint32>(m_campaign_type);
	ds << static_cast<qint32>(m_num_players);
	ds << static_cast<qint32>(m_flags);
	ds << static_cast<qint32>(m_save_format);

	ds << static_cast<qint32>(m_custom_data.size());
	for (const auto& [key, value] : m_custom_data) {
		ds << QString::fromStdString(key);
		ds << QString::fromStdString(value);
	}

	ds << static_cast<qint32>(m_ships_allowed.size());
	for (bool allowed : m_ships_allowed)
		ds << static_cast<quint8>(allowed ? 1 : 0);
	ds << static_cast<qint32>(m_weapons_allowed.size());
	for (bool allowed : m_weapons_allowed)
		ds << static_cast<quint8>(allowed ? 1 : 0);

	ds << static_cast<qint32>(m_missions.size());
	for (const auto& mission : m_missions) {
		ds << QString::fromStdString(mission.filename);
		ds << static_cast<qint32>(mission.level);
		ds << static_cast<qint32>(mission.position);
		ds << static_cast<qint32>(mission.graph_x);
		ds << static_cast<qint32>(mission.graph_y);
		ds << static_cast<qint32>(mission.graph_color);
		ds << QString::fromStdString(mission.briefing_cutscene);
		ds << QString::fromStdString(mission.main_hall);
		ds << QString::fromStdString(mission.substitute_main_hall);
		ds << static_cast<qint32>(mission.debrief_persona_index);
		ds << static_cast<qint32>(mission.special_mode_hint);

		ds << static_cast<qint32>(mission.branches.size());
		for (const auto& branch : mission.branches) {
			ds << static_cast<qint32>(branch.id);
			ds << QString::fromStdString(branch.next_mission_name);
			ds << static_cast<quint8>(branch.is_loop ? 1 : 0);
			ds << static_cast<quint8>(branch.is_fork ? 1 : 0);
			ds << QString::fromStdString(branch.loop_description);
			ds << QString::fromStdString(branch.loop_briefing_anim);
			ds << QString::fromStdString(branch.loop_briefing_sound);
			// branch.sexp_formula is an internal tree id; round-trip the
			// condition through Sexp_nodes to serialize its content.
			const int sexp = m_tree_ops.saveSexp(branch.sexp_formula);
			fso::fred::state::writeSexp(ds, sexp);
			fso::fred::state::freeSexpFormula(sexp);
		}
	}

	return data;
}

void CampaignEditorDialogModel::restoreWorkingState(const QByteArray& state)
{
	QDataStream ds(state);

	// All branch trees live in the shared tree model; drop them wholesale
	// before reloading, or the node pool grows with every restore.
	m_tree_ops.clearTree();
	m_missions.clear();

	QString name, descr;
	qint32 type, numPlayers, flags, saveFormat;
	ds >> name >> descr >> type >> numPlayers >> flags >> saveFormat;
	m_campaign_name  = name.toStdString();
	m_campaign_descr = descr.toStdString();
	m_campaign_type  = static_cast<int>(type);
	m_num_players    = static_cast<int>(numPlayers);
	m_flags          = static_cast<int>(flags);
	m_save_format    = static_cast<CampaignFormat>(saveFormat);

	m_custom_data.clear();
	qint32 customCount;
	ds >> customCount;
	for (int i = 0; i < customCount; ++i) {
		QString key, value;
		ds >> key >> value;
		m_custom_data[key.toStdString()] = value.toStdString();
	}

	auto readBools = [&ds](SCP_vector<bool>& out) {
		out.clear();
		qint32 count;
		ds >> count;
		out.reserve(count);
		for (int i = 0; i < count; ++i) {
			quint8 allowed;
			ds >> allowed;
			out.push_back(allowed != 0);
		}
	};
	readBools(m_ships_allowed);
	readBools(m_weapons_allowed);

	qint32 missionCount;
	ds >> missionCount;
	m_missions.reserve(missionCount);
	for (int i = 0; i < missionCount; ++i) {
		auto& mission = m_missions.emplace_back();

		QString filename, cutscene, hall, subHall;
		qint32 level, position, graphX, graphY, graphColor, persona, modeHint;
		ds >> filename >> level >> position >> graphX >> graphY >> graphColor;
		ds >> cutscene >> hall >> subHall >> persona >> modeHint;

		mission.filename              = filename.toStdString();
		mission.level                 = static_cast<int>(level);
		mission.position              = static_cast<int>(position);
		mission.graph_x               = static_cast<int>(graphX);
		mission.graph_y               = static_cast<int>(graphY);
		mission.graph_color           = static_cast<int>(graphColor);
		mission.briefing_cutscene     = cutscene.toStdString();
		mission.main_hall             = hall.toStdString();
		mission.substitute_main_hall  = subHall.toStdString();
		mission.debrief_persona_index = static_cast<int>(persona);
		mission.special_mode_hint     = static_cast<CampaignSpecialMode>(modeHint);

		qint32 branchCount;
		ds >> branchCount;
		mission.branches.reserve(branchCount);
		for (int j = 0; j < branchCount; ++j) {
			auto& branch = mission.branches.emplace_back();

			QString nextName, loopDescr, loopAnim, loopVoice;
			qint32 id;
			quint8 isLoop, isFork;
			ds >> id >> nextName >> isLoop >> isFork;
			ds >> loopDescr >> loopAnim >> loopVoice;

			branch.id                  = static_cast<int>(id);
			branch.next_mission_name   = nextName.toStdString();
			branch.is_loop             = (isLoop != 0);
			branch.is_fork             = (isFork != 0);
			branch.loop_description    = loopDescr.toStdString();
			branch.loop_briefing_anim  = loopAnim.toStdString();
			branch.loop_briefing_sound = loopVoice.toStdString();

			const int sexp = fso::fred::state::readSexp(ds);
			branch.sexp_formula = m_tree_ops.loadSexp(sexp);
			fso::fred::state::freeSexpFormula(sexp);
		}
	}

	// Selections do not survive the rebuild; the dialog re-selects by
	// filename where possible.
	m_current_mission_index = -1;
	m_current_branch_index = -1;

	loadAvailableMissions();
	syncCampaignMissionList();

	set_modified();
}

} // namespace fso::fred::dialogs
