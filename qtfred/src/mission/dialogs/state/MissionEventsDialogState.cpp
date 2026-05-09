// captureState() and restoreState() for MissionEventsDialogModel.
//
// Snapshots Mission_events, Messages (non-builtin), and Event_annotations so
// that undo/redo can restore event parameters, messages, and annotations.
//
// TODO(sexp_tree_refactor): mission_event::formula is not serialized.
// formula is set to -1 on restore; sexp tree content is lost on undo/redo.
//
// TODO: MMessage filter fields (sender_filter, subject_filter, outer_filter,
// mood, excluded_moods, boost_level) are not serialized.  These fields are not
// editable in the Mission Events dialog and are assumed to be empty in
// FRED-authored missions.

#include <mission/dialogs/MissionEventsDialogModel.h>

#include "DialogStateHelpers.h"
#include <mission/missiongoals.h>
#include <mission/missionmessage.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#include <cstring>

namespace fso::fred::dialogs {

QByteArray MissionEventsDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	// --- Mission_events ---
	ds << static_cast<qint32>(Mission_events.size());
	for (const mission_event& e : Mission_events) {
		ds << QString::fromStdString(e.name);
		fso::fred::state::writeSexpStub(ds); // formula — TODO(sexp_tree_refactor)
		ds << static_cast<qint32>(e.repeat_count);
		ds << static_cast<qint32>(e.trigger_count);
		ds << static_cast<qint32>(e.interval);
		ds << static_cast<qint32>(e.score);
		ds << static_cast<qint32>(e.chain_delay);
		ds << static_cast<qint32>(e.flags);
		ds << QString::fromStdString(e.objective_text);
		ds << QString::fromStdString(e.objective_key_text);
		ds << static_cast<qint32>(e.team);
		ds << static_cast<qint32>(e.mission_log_flags);
	}

	// --- Messages (non-builtin only) ---
	int nonBuiltin = Num_messages - Num_builtin_messages;
	ds << static_cast<qint32>(nonBuiltin);
	for (int i = Num_builtin_messages; i < Num_messages; ++i) {
		const MMessage& m = Messages[i];
		ds << QString::fromLatin1(m.name);
		ds << QString::fromLatin1(m.message);
		ds << QString::fromStdString(m.note);
		ds << QString::fromLatin1(m.avi_info.name  ? m.avi_info.name  : "");
		ds << QString::fromLatin1(m.wave_info.name ? m.wave_info.name : "");
		ds << static_cast<qint32>(m.persona_index);
		ds << static_cast<qint32>(m.multi_team);
	}

	// --- Event_annotations ---
	ds << static_cast<qint32>(Event_annotations.size());
	for (const event_annotation& ea : Event_annotations) {
		ds << static_cast<qint32>(ea.path.size());
		for (int idx : ea.path)
			ds << static_cast<qint32>(idx);
		ds << QString::fromStdString(ea.comment);
		ds << static_cast<quint8>(ea.r);
		ds << static_cast<quint8>(ea.g);
		ds << static_cast<quint8>(ea.b);
	}

	return data;
}

void MissionEventsDialogModel::restoreState(const QByteArray& state)
{
	QDataStream ds(state);

	// --- Mission_events ---
	qint32 eventCount;
	ds >> eventCount;

	Mission_events.clear();
	Mission_events.resize(static_cast<size_t>(eventCount));

	for (int i = 0; i < eventCount; ++i) {
		mission_event& e = Mission_events[static_cast<size_t>(i)];
		QString name, objText, objKeyText;
		qint32 repeat, trigger, interval, score, chainDelay, flags, team, logFlags;

		ds >> name;
		fso::fred::state::readSexpStub(ds); // formula — TODO(sexp_tree_refactor)
		e.formula = -1;
		ds >> repeat >> trigger >> interval >> score >> chainDelay >> flags;
		ds >> objText >> objKeyText;
		ds >> team >> logFlags;

		e.name               = name.toStdString();
		e.repeat_count       = static_cast<int>(repeat);
		e.trigger_count      = static_cast<int>(trigger);
		e.interval           = static_cast<int>(interval);
		e.score              = static_cast<int>(score);
		e.chain_delay        = static_cast<int>(chainDelay);
		e.flags              = static_cast<int>(flags);
		e.objective_text     = objText.toStdString();
		e.objective_key_text = objKeyText.toStdString();
		e.team               = static_cast<int>(team);
		e.mission_log_flags  = static_cast<int>(logFlags);
	}

	// --- Messages (non-builtin) ---
	qint32 msgCount;
	ds >> msgCount;

	// Free existing non-builtin name pointers before clearing.
	for (int i = Num_builtin_messages; i < Num_messages; ++i) {
		if (Messages[i].avi_info.name)  { free(Messages[i].avi_info.name);  Messages[i].avi_info.name  = nullptr; }
		if (Messages[i].wave_info.name) { free(Messages[i].wave_info.name); Messages[i].wave_info.name = nullptr; }
	}
	Num_messages = Num_builtin_messages + static_cast<int>(msgCount);
	Messages.resize(static_cast<size_t>(Num_messages));

	for (int i = 0; i < static_cast<int>(msgCount); ++i) {
		MMessage& m = Messages[Num_builtin_messages + i];
		m = MMessage{}; // zero-initialize
		m.avi_info.name  = nullptr;
		m.wave_info.name = nullptr;

		QString name, message, note, avi, wave;
		qint32 persona, team;
		ds >> name >> message >> note >> avi >> wave >> persona >> team;

		strncpy(m.name,    name.toLatin1().constData(),    NAME_LENGTH    - 1);
		m.name[NAME_LENGTH - 1] = '\0';
		strncpy(m.message, message.toLatin1().constData(), MESSAGE_LENGTH - 1);
		m.message[MESSAGE_LENGTH - 1] = '\0';
		m.note          = note.toStdString();
		m.persona_index = static_cast<int>(persona);
		m.multi_team    = static_cast<int>(team);

		if (!avi.isEmpty())
			m.avi_info.name  = strdup(avi.toLatin1().constData());
		if (!wave.isEmpty())
			m.wave_info.name = strdup(wave.toLatin1().constData());
	}

	// --- Event_annotations ---
	qint32 annotCount;
	ds >> annotCount;

	Event_annotations.clear();
	Event_annotations.resize(static_cast<size_t>(annotCount));

	for (int i = 0; i < annotCount; ++i) {
		event_annotation& ea = Event_annotations[static_cast<size_t>(i)];
		qint32 pathLen;
		ds >> pathLen;
		ea.path.clear();
		for (int j = 0; j < pathLen; ++j) {
			qint32 idx;
			ds >> idx;
			ea.path.push_back(static_cast<int>(idx));
		}
		QString comment;
		quint8 r, g, b;
		ds >> comment >> r >> g >> b;
		ea.comment    = comment.toStdString();
		ea.r          = r;
		ea.g          = g;
		ea.b          = b;
		ea.node_index = -1;
		ea.item_image = -1;
	}
}

} // namespace fso::fred::dialogs
