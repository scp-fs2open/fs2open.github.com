// captureState() and restoreState() for BriefingEditorDialogModel.
// Snapshots Briefings[], Mission_music[SCORE_BRIEFING], and
// The_mission.substitute_briefing_music_name from the live globals.
//
// brief_icon authoring fields are serialized individually; runtime fields
// (screen coords, bitmap/model handles, anim state) are skipped and
// zero-initialised on restore.

#include <mission/dialogs/BriefingEditorDialogModel.h>

#include "DialogStateHelpers.h"
#include <gamesnd/eventmusic.h>
#include <globalincs/memory/memory.h>
#include <mission/missionbriefcommon.h>
#include <mission/missionparse.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

namespace fso::fred::dialogs {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void writeIcon(QDataStream& ds, const brief_icon& ic)
{
	ds << static_cast<qint32>(ic.type);
	ds << static_cast<qint32>(ic.id);
	ds << static_cast<qint32>(ic.team);
	ds << static_cast<qint32>(ic.ship_class);
	ds << ic.pos.xyz.x << ic.pos.xyz.y << ic.pos.xyz.z;
	ds << QString::fromLatin1(ic.label);
	ds << QString::fromLatin1(ic.closeup_label);
	ds << static_cast<qint32>(ic.flags);
	ds << ic.scale_factor;
}

static void readIcon(QDataStream& ds, brief_icon& ic)
{
	memset(&ic, 0, sizeof(brief_icon));
	// hud_anim::first_frame == 0 is a valid "not loaded" sentinel for runtime reload,
	// but the model/bitmap handles use -1 (0 is a valid handle) — match live icon creation.
	ic.modelnum           = -1;
	ic.model_instance_num = -1;
	ic.bitmap_id          = -1;

	qint32 type, id, team, shipClass, flags;
	float px, py, pz, scale;
	QString label, closeupLabel;

	ds >> type >> id >> team >> shipClass;
	ds >> px >> py >> pz;
	ds >> label >> closeupLabel >> flags >> scale;

	ic.type       = static_cast<int>(type);
	ic.id         = static_cast<int>(id);
	ic.team       = static_cast<int>(team);
	ic.ship_class = static_cast<int>(shipClass);
	vm_vec_make(&ic.pos, px, py, pz);
	strncpy(ic.label,         label.toLatin1().constData(),        MAX_LABEL_LEN - 1);
	ic.label[MAX_LABEL_LEN - 1] = '\0';
	strncpy(ic.closeup_label, closeupLabel.toLatin1().constData(), MAX_LABEL_LEN - 1);
	ic.closeup_label[MAX_LABEL_LEN - 1] = '\0';
	ic.flags        = static_cast<int>(flags);
	ic.scale_factor = scale;
}

static void writeStage(QDataStream& ds, const brief_stage& s)
{
	ds << QString::fromStdString(s.text);
	ds << QString::fromLatin1(s.voice);
	ds << s.camera_pos.xyz.x << s.camera_pos.xyz.y << s.camera_pos.xyz.z;
	// camera_orient: row-major rvec/uvec/fvec
	ds << s.camera_orient.vec.rvec.xyz.x << s.camera_orient.vec.rvec.xyz.y << s.camera_orient.vec.rvec.xyz.z;
	ds << s.camera_orient.vec.uvec.xyz.x << s.camera_orient.vec.uvec.xyz.y << s.camera_orient.vec.uvec.xyz.z;
	ds << s.camera_orient.vec.fvec.xyz.x << s.camera_orient.vec.fvec.xyz.y << s.camera_orient.vec.fvec.xyz.z;
	ds << static_cast<qint32>(s.camera_time);
	ds << static_cast<qint32>(s.flags);
	fso::fred::state::writeSexp(ds, s.formula);
	ds << s.draw_grid;
	ds << static_cast<quint8>(s.grid_color.red)
	   << static_cast<quint8>(s.grid_color.green)
	   << static_cast<quint8>(s.grid_color.blue)
	   << static_cast<quint8>(s.grid_color.alpha)
	   << static_cast<quint8>(s.grid_color.ac_type);

	// Icons
	const auto numIcons = static_cast<qint32>(
		(s.icons != nullptr) ? s.num_icons : 0);
	ds << numIcons;
	if (s.icons != nullptr) {
		for (int i = 0; i < numIcons; ++i)
			writeIcon(ds, s.icons[i]);
	}

	// Lines
	const auto numLines = static_cast<qint32>(
		(s.lines != nullptr) ? s.num_lines : 0);
	ds << numLines;
	if (s.lines != nullptr) {
		for (int i = 0; i < numLines; ++i) {
			ds << static_cast<qint32>(s.lines[i].start_icon);
			ds << static_cast<qint32>(s.lines[i].end_icon);
		}
	}
}

static void readStage(QDataStream& ds, brief_stage& s)
{
	QString text, voice;
	ds >> text >> voice;
	s.text = text.toStdString();
	strncpy(s.voice, voice.toLatin1().constData(), MAX_FILENAME_LEN - 1);
	s.voice[MAX_FILENAME_LEN - 1] = '\0';

	ds >> s.camera_pos.xyz.x >> s.camera_pos.xyz.y >> s.camera_pos.xyz.z;
	ds >> s.camera_orient.vec.rvec.xyz.x >> s.camera_orient.vec.rvec.xyz.y >> s.camera_orient.vec.rvec.xyz.z;
	ds >> s.camera_orient.vec.uvec.xyz.x >> s.camera_orient.vec.uvec.xyz.y >> s.camera_orient.vec.uvec.xyz.z;
	ds >> s.camera_orient.vec.fvec.xyz.x >> s.camera_orient.vec.fvec.xyz.y >> s.camera_orient.vec.fvec.xyz.z;

	qint32 camTime, flags;
	ds >> camTime >> flags;
	s.camera_time = static_cast<int>(camTime);
	s.flags       = static_cast<int>(flags);

	fso::fred::state::freeSexpFormula(s.formula);
	s.formula = fso::fred::state::readSexp(ds);

	ds >> s.draw_grid;
	quint8 cr, cg, cb, ca, cat;
	ds >> cr >> cg >> cb >> ca >> cat;
	s.grid_color.red    = cr;
	s.grid_color.green  = cg;
	s.grid_color.blue   = cb;
	s.grid_color.alpha  = ca;
	s.grid_color.ac_type = cat;

	// Icons
	qint32 numIcons;
	ds >> numIcons;
	s.num_icons = static_cast<int>(numIcons);
	if (numIcons > 0) {
		if (s.icons == nullptr)
			s.icons = static_cast<brief_icon*>(vm_malloc(sizeof(brief_icon) * MAX_STAGE_ICONS));
		for (int i = 0; i < static_cast<int>(numIcons); ++i)
			readIcon(ds, s.icons[i]);
	}

	// Lines
	qint32 numLines;
	ds >> numLines;
	s.num_lines = static_cast<int>(numLines);
	if (numLines > 0) {
		if (s.lines == nullptr)
			s.lines = static_cast<brief_line*>(vm_malloc(sizeof(brief_line) * MAX_BRIEF_STAGE_LINES));
		for (int i = 0; i < static_cast<int>(numLines); ++i) {
			qint32 si, ei;
			ds >> si >> ei;
			s.lines[i].start_icon = static_cast<int>(si);
			s.lines[i].end_icon   = static_cast<int>(ei);
		}
	}
}

// ---------------------------------------------------------------------------
// captureState / restoreState
// ---------------------------------------------------------------------------

QByteArray BriefingEditorDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	for (const auto& b : Briefings) {
		ds << static_cast<qint32>(b.num_stages);

		for (const auto& stage : b.stages)
			writeStage(ds, stage);

		for (int r = 0; r < GR_NUM_RESOLUTIONS; ++r) {
			ds << QString::fromLatin1(b.background[r]);
			ds << QString::fromLatin1(b.ship_select_background[r]);
			ds << QString::fromLatin1(b.weapon_select_background[r]);
		}
	}

	ds << static_cast<qint32>(Mission_music[SCORE_BRIEFING]);
	ds << QString::fromLatin1(The_mission.substitute_briefing_music_name);

	return data;
}

void BriefingEditorDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	for (auto& b : Briefings) {
		qint32 numStages;
		ds >> numStages;
		b.num_stages = static_cast<int>(numStages);

		for (auto& stage : b.stages)
			readStage(ds, stage);

		for (int r = 0; r < GR_NUM_RESOLUTIONS; ++r) {
			QString bg, shipBg, wpnBg;
			ds >> bg >> shipBg >> wpnBg;
			strncpy(b.background[r],              bg.toLatin1().constData(),    MAX_FILENAME_LEN - 1);
			b.background[r][MAX_FILENAME_LEN - 1] = '\0';
			strncpy(b.ship_select_background[r],   shipBg.toLatin1().constData(), MAX_FILENAME_LEN - 1);
			b.ship_select_background[r][MAX_FILENAME_LEN - 1] = '\0';
			strncpy(b.weapon_select_background[r], wpnBg.toLatin1().constData(),  MAX_FILENAME_LEN - 1);
			b.weapon_select_background[r][MAX_FILENAME_LEN - 1] = '\0';
		}
	}

	qint32 music;
	ds >> music;
	Mission_music[SCORE_BRIEFING] = static_cast<int>(music);

	QString subMusic;
	ds >> subMusic;
	strncpy(The_mission.substitute_briefing_music_name,
	        subMusic.toLatin1().constData(), NAME_LENGTH - 1);
	The_mission.substitute_briefing_music_name[NAME_LENGTH - 1] = '\0';
}

} // namespace fso::fred::dialogs
