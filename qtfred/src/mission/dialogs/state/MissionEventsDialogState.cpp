// captureState() and restoreState() for MissionEventsDialogModel.
//
// Snapshots Mission_events (including sexp formulas), Messages (non-builtin,
// in full — including the mood/filter fields the dialog cannot edit, so an
// undo cycle never strips data parsed from the mission file), and
// Event_annotations.

#include <mission/dialogs/MissionEventsDialogModel.h>

#include "DialogStateHelpers.h"
#include <mission/missiongoals.h>
#include <mission/missionmessage.h>
#include <sound/audiostr.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#include <cstring>

namespace fso::fred::dialogs {

static void writeMessageFilter(QDataStream& ds, const MessageFilter& f);
static void readMessageFilter(QDataStream& ds, MessageFilter& f);

// Serialize the MMessage fields shared by the apply-level and working-state
// blobs that go beyond what the dialog edits: mood and filter data.
static void writeMessageExtras(QDataStream& ds, const MMessage& m)
{
	ds << static_cast<qint32>(m.mood);
	ds << static_cast<qint32>(m.excluded_moods.size());
	for (int mood : m.excluded_moods)
		ds << static_cast<qint32>(mood);
	ds << static_cast<qint32>(m.outer_filter_radius);
	ds << static_cast<qint32>(m.boost_level);
	writeMessageFilter(ds, m.sender_filter);
	writeMessageFilter(ds, m.subject_filter);
	writeMessageFilter(ds, m.outer_filter);
}

static void readMessageExtras(QDataStream& ds, MMessage& m)
{
	qint32 mood, moodCount, filterRadius, boost;
	ds >> mood;
	m.mood = static_cast<int>(mood);
	ds >> moodCount;
	m.excluded_moods.clear();
	for (int j = 0; j < moodCount; ++j) {
		qint32 excluded;
		ds >> excluded;
		m.excluded_moods.push_back(static_cast<int>(excluded));
	}
	ds >> filterRadius >> boost;
	m.outer_filter_radius = static_cast<int>(filterRadius);
	m.boost_level         = static_cast<int>(boost);
	readMessageFilter(ds, m.sender_filter);
	readMessageFilter(ds, m.subject_filter);
	readMessageFilter(ds, m.outer_filter);
}

QByteArray MissionEventsDialogModel::captureState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	// --- Mission_events ---
	ds << static_cast<qint32>(Mission_events.size());
	for (const mission_event& e : Mission_events) {
		ds << QString::fromStdString(e.name);
		fso::fred::state::writeSexp(ds, e.formula);
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
		writeMessageExtras(ds, m);
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

	for (const mission_event& e : Mission_events)
		fso::fred::state::freeSexpFormula(e.formula);
	Mission_events.clear();
	Mission_events.resize(static_cast<size_t>(eventCount));

	for (int i = 0; i < eventCount; ++i) {
		mission_event& e = Mission_events[static_cast<size_t>(i)];
		QString name, objText, objKeyText;
		qint32 repeat, trigger, interval, score, chainDelay, flags, team, logFlags;

		ds >> name;
		e.formula = fso::fred::state::readSexp(ds);
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

		readMessageExtras(ds, m);
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

// ---------------------------------------------------------------------------
// Working-state capture/restore for the in-dialog undo stack.
//
// Two independent scopes: event/tree state and message state, so undoing a
// message operation never rebuilds the sexp tree and vice versa.
// ---------------------------------------------------------------------------

// Message working state serializes MMessage in full (including mood and
// filter fields the dialog cannot edit) so an undo cycle never strips data
// that was parsed from the mission file.
static void writeMessageFilter(QDataStream& ds, const MessageFilter& f)
{
	auto writeNames = [&ds](const SCP_vector<SCP_string>& names) {
		ds << static_cast<qint32>(names.size());
		for (const auto& n : names)
			ds << QString::fromStdString(n);
	};
	writeNames(f.ship_name);
	writeNames(f.callsign);
	writeNames(f.class_name);
	writeNames(f.wing_name);
	ds << static_cast<qint32>(f.species_bitfield);
	ds << static_cast<qint32>(f.type_bitfield);
	ds << static_cast<qint32>(f.team_bitfield);
}

static void readMessageFilter(QDataStream& ds, MessageFilter& f)
{
	auto readNames = [&ds](SCP_vector<SCP_string>& names) {
		names.clear();
		qint32 count;
		ds >> count;
		for (int i = 0; i < count; ++i) {
			QString n;
			ds >> n;
			names.push_back(n.toStdString());
		}
	};
	readNames(f.ship_name);
	readNames(f.callsign);
	readNames(f.class_name);
	readNames(f.wing_name);
	qint32 species, type, team;
	ds >> species >> type >> team;
	f.species_bitfield = static_cast<int>(species);
	f.type_bitfield    = static_cast<int>(type);
	f.team_bitfield    = static_cast<int>(team);
}

QByteArray MissionEventsDialogModel::captureEventWorkingState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(m_events.size());
	for (size_t i = 0; i < m_events.size(); ++i) {
		const mission_event& e = m_events[i];
		ds << static_cast<qint32>(m_sig[i]);
		ds << QString::fromStdString(e.name);
		// e.formula is a tree-widget node id; round-trip the branch through
		// Sexp_nodes to serialize its content.
		const int sexp = m_tree_model.save_tree(e.formula);
		fso::fred::state::writeSexp(ds, sexp);
		fso::fred::state::freeSexpFormula(sexp);
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
	// m_cur_event is deliberately not serialized: selection changes without a
	// stack push, so including it would break the blob-equality check that
	// absorbs the tree's redundant modified() signals.

	// Annotations in path form: keys are tree_nodes[] indices, which do not
	// survive the rebuild a restore performs.
	SCP_vector<std::pair<SCP_list<int>, const event_annotation*>> annots;
	for (const auto& ea : m_annotation_model.annotations()) {
		if (SexpAnnotationModel::isDefault(ea))
			continue;
		// Skip stale regular keys (node freed by a root delete).
		if (!SexpAnnotationModel::isRootKey(ea.node_index)) {
			if (ea.node_index < 0 || ea.node_index >= static_cast<int>(m_tree_model.tree_nodes.size()) ||
			    m_tree_model.tree_nodes[ea.node_index].type == SEXPT_UNUSED) {
				continue;
			}
		}
		auto path = SexpAnnotationModel::buildPath(ea.node_index, m_tree_model.tree_nodes, m_events);
		if (path.empty())
			continue;
		annots.emplace_back(std::move(path), &ea);
	}
	ds << static_cast<qint32>(annots.size());
	for (const auto& [path, ea] : annots) {
		ds << static_cast<qint32>(path.size());
		for (int p : path)
			ds << static_cast<qint32>(p);
		ds << QString::fromStdString(ea->comment);
		ds << static_cast<quint8>(ea->r) << static_cast<quint8>(ea->g) << static_cast<quint8>(ea->b);
	}

	return data;
}

void MissionEventsDialogModel::restoreEventWorkingState(const QByteArray& state)
{
	QDataStream ds(state);

	m_events.clear();
	m_sig.clear();
	m_tree_model.clear_tree_data(); // resets tree_nodes; the widget is rebuilt below

	qint32 count;
	ds >> count;
	for (int i = 0; i < count; ++i) {
		mission_event e{};
		qint32 sig, repeat, trigger, interval, score, chainDelay, flags, team, logFlags;
		QString name, objText, objKeyText;

		ds >> sig >> name;
		const int sexp = fso::fred::state::readSexp(ds);
		e.formula = m_tree_model.load_sub_tree(sexp, false, "do-nothing");
		fso::fred::state::freeSexpFormula(sexp);
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

		m_events.push_back(e);
		m_sig.push_back(static_cast<int>(sig));
	}
	m_tree_model.post_load();

	m_cur_event = -1; // the tree selection is gone after the rebuild

	rebuildTreeWidget();

	// Annotations: paths were built against the dialog's own event order, so
	// resolution uses an identity index map (m_sig maps to the original
	// Mission_events order and must not be used here).
	m_annotation_model.clear();
	SCP_vector<int> identity;
	identity.reserve(m_events.size());
	for (int i = 0; i < static_cast<int>(m_events.size()); ++i)
		identity.push_back(i);

	qint32 annotCount;
	ds >> annotCount;
	for (int i = 0; i < annotCount; ++i) {
		SCP_list<int> path;
		qint32 pathLen;
		ds >> pathLen;
		for (int j = 0; j < pathLen; ++j) {
			qint32 idx;
			ds >> idx;
			path.push_back(static_cast<int>(idx));
		}
		QString comment;
		quint8 r, g, b;
		ds >> comment >> r >> g >> b;

		const int key = SexpAnnotationModel::resolveFromPath(path, m_tree_model.tree_nodes, m_events, identity);
		if (key == -1)
			continue;
		auto& ea = m_annotation_model.ensureByKey(key);
		ea.comment = comment.toStdString();
		ea.r = r;
		ea.g = g;
		ea.b = b;
		const bool hasColor = (r != 255) || (g != 255) || (b != 255);
		Q_EMIT annotationApplied(key, ea.comment, r, g, b, hasColor);
	}

	set_modified();
}

QByteArray MissionEventsDialogModel::captureMessageWorkingState() const
{
	QByteArray data;
	QDataStream ds(&data, QIODevice::WriteOnly);

	ds << static_cast<qint32>(m_messages.size());
	for (const MMessage& m : m_messages) {
		ds << QString::fromLatin1(m.name);
		ds << QString::fromLatin1(m.message);
		ds << QString::fromStdString(m.note);
		ds << QString::fromLatin1(m.avi_info.name  ? m.avi_info.name  : "");
		ds << QString::fromLatin1(m.wave_info.name ? m.wave_info.name : "");
		ds << static_cast<qint32>(m.persona_index);
		ds << static_cast<qint32>(m.multi_team);
		writeMessageExtras(ds, m);
	}
	ds << static_cast<qint32>(m_cur_msg);

	return data;
}

void MissionEventsDialogModel::restoreMessageWorkingState(const QByteArray& state)
{
	QDataStream ds(state);

	audiostream_close_file(m_wave_id, false);
	m_wave_id = -1;

	for (auto& m : m_messages) {
		if (m.avi_info.name)  { free(m.avi_info.name);  m.avi_info.name  = nullptr; }
		if (m.wave_info.name) { free(m.wave_info.name); m.wave_info.name = nullptr; }
	}
	m_messages.clear();

	qint32 count;
	ds >> count;
	for (int i = 0; i < count; ++i) {
		MMessage m{};
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

		readMessageExtras(ds, m);

		if (!avi.isEmpty())
			m.avi_info.name  = strdup(avi.toLatin1().constData());
		if (!wave.isEmpty())
			m.wave_info.name = strdup(wave.toLatin1().constData());

		m_messages.push_back(std::move(m));
	}

	qint32 cur;
	ds >> cur;
	m_cur_msg = static_cast<int>(cur);

	set_modified();
}

} // namespace fso::fred::dialogs
