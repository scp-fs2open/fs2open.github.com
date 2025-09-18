#include "MissionEventsDialogModel.h"

#include <sound/audiostr.h>
#include <localization/localize.h>

namespace fso::fred::dialogs {

MissionEventsDialogModel::MissionEventsDialogModel(QObject* parent, fso::fred::EditorViewport* viewport, IEventTreeOps& tree_ops)
	: AbstractDialogModel(parent, viewport), m_event_tree_ops(tree_ops)
{
	initializeData();
}

bool MissionEventsDialogModel::apply()
{
	SCP_vector<std::pair<SCP_string, SCP_string>> names;

	audiostream_close_file(m_wave_id, false);
	m_wave_id = -1;

	for (auto& event : Mission_events) {
		free_sexp2(event.formula);
		event.result = 0; // use this as a processed flag
	}

	// rename all sexp references to old events
	for (int i = 0; i < static_cast<int>(m_events.size()); i++) {
		if (m_sig[i] >= 0) {
			names.emplace_back(Mission_events[m_sig[i]].name, m_events[i].name);
			Mission_events[m_sig[i]].result = 1;
		}
	}

	// invalidate all sexp references to deleted events.
	for (const auto& event : Mission_events) {
		if (!event.result) {
			SCP_string buf = "<" + event.name + ">";

			// force it to not be too long
			if (SCP_truncate(buf, NAME_LENGTH - 1))
				buf.back() = '>';

			names.emplace_back(event.name, buf);
		}
	}

	// copy all dialog events to the mission
	Mission_events.clear();
	for (const auto& dialog_event : m_events) {
		Mission_events.push_back(dialog_event);
		Mission_events.back().formula = m_event_tree_ops.save_tree(dialog_event.formula);
	}

	// now update all sexp references
	for (const auto& name_pair : names)
		update_sexp_references(name_pair.first.c_str(), name_pair.second.c_str(), OPF_EVENT_NAME);

	for (int i = Num_builtin_messages; i < Num_messages; i++) {
		if (Messages[i].avi_info.name)
			free(Messages[i].avi_info.name);

		if (Messages[i].wave_info.name)
			free(Messages[i].wave_info.name);
	}

	Num_messages = static_cast<int>(m_messages.size()) + Num_builtin_messages;
	Messages.resize(Num_messages);
	for (int i = 0; i < static_cast<int>(m_messages.size()); i++)
		Messages[i + Num_builtin_messages] = m_messages[i];

	applyAnnotations();

	// Only fire the signal after the changes have been applied to make sure the other parts of the code see the updated
	// state
	if (query_modified()) {
		_editor->missionChanged();
	}
	return true;
}

void MissionEventsDialogModel::reject()
{
	// Nothing to do here
}

void MissionEventsDialogModel::initializeData()
{
	initializeMessages();
	initializeHeadAniList();
	initializeWaveList();
	initializePersonaList();

	initializeTeamList();
	initializeEvents();
}

void MissionEventsDialogModel::initializeEvents()
{
	m_events.clear();
	m_sig.clear();
	m_cur_event = -1;
	for (auto i = 0; i < static_cast<int>(Mission_events.size()); i++) {
		m_events.push_back(Mission_events[i]);
		m_sig.push_back(i);

		if (m_events[i].name.empty()) {
			m_events[i].name = "<Unnamed>";
		}

		m_events[i].formula = m_event_tree_ops.load_sub_tree(Mission_events[i].formula, false, "do-nothing");

		// we must check for the case of the repeat count being 0.  This would happen if the repeat
		// count is not specified in a mission
		if (m_events[i].repeat_count <= 0) {
			m_events[i].repeat_count = 1;
		}
	}

	m_event_tree_ops.post_load();

	m_event_tree_ops.clear();
	for (auto& event : m_events) {
		// set the proper bitmap
		NodeImage image;
		if (event.chain_delay >= 0) {
			image = NodeImage::CHAIN;
			if (!event.objective_text.empty()) {
				image = NodeImage::CHAIN_DIRECTIVE;
			}
		} else {
			image = NodeImage::ROOT;
			if (!event.objective_text.empty()) {
				image = NodeImage::ROOT_DIRECTIVE;
			}
		}

		m_event_tree_ops.add_sub_tree(event.name, image, event.formula);
	}

	initializeEventAnnotations();
}

int MissionEventsDialogModel::findFormulaByOriginalEventIndex(int orig) const
{
	for (int cur = 0; cur < static_cast<int>(m_sig.size()); ++cur)
		if (m_sig[cur] == orig)
			return m_events[cur].formula;
	return -1;
}

void MissionEventsDialogModel::initializeEventAnnotations()
{
	m_event_annotations = Event_annotations; // copy

	for (auto& ea : m_event_annotations) {
		ea.handle = nullptr;
		if (ea.path.empty())
			continue;

		const int origIdx = ea.path.front();
		const int formula = findFormulaByOriginalEventIndex(origIdx);
		if (formula < 0)
			continue;

		IEventTreeOps::Handle h = m_event_tree_ops.get_root_by_formula(formula);
		if (!h)
			continue;

		// walk children
		auto it = ea.path.begin();
		++it; // skip event index
		for (; it != ea.path.end() && h; ++it) {
			const int child = *it;
			if (child < 0 || child >= m_event_tree_ops.child_count(h)) {
				h = nullptr;
				break;
			}
			h = m_event_tree_ops.child_at(h, child);
		}

		ea.handle = h;
		if (h) {
			const bool hasColor = (ea.r != 255) || (ea.g != 255) || (ea.b != 255);
			m_event_tree_ops.set_node_note(h, ea.comment);
			m_event_tree_ops.set_node_bg_color(h, ea.r, ea.g, ea.b, hasColor);
		}
	}
}

// Build the path for a handle (root formula -> orig index; then child indices)
SCP_list<int> MissionEventsDialogModel::buildPathForHandle(IEventTreeOps::Handle h) const
{
	SCP_list<int> path;
	if (!h)
		return path;

	const int rootFormula = m_event_tree_ops.root_formula_of(h);
	if (rootFormula < 0)
		return path;

	// Find the *current* index of this root in m_events
	int curIdx = -1;
	for (int i = 0; i < static_cast<int>(m_events.size()); ++i) {
		if (m_events[i].formula == rootFormula) {
			curIdx = i;
			break;
		}
	}
	if (curIdx < 0)
		return path;

	// persist the current index
	path.push_back(curIdx);

	// Collect child indices from node up to root, then reverse
	std::vector<int> rev;
	IEventTreeOps::Handle cur = h;
	for (;;) {
		IEventTreeOps::Handle parent = m_event_tree_ops.parent_of(cur);
		if (!parent)
			break;
		rev.push_back(m_event_tree_ops.index_in_parent(cur));
		cur = parent;
	}
	for (auto it = rev.rbegin(); it != rev.rend(); ++it)
		path.push_back(*it);

	return path;
}

bool MissionEventsDialogModel::isDefaultAnnotation(const event_annotation& ea)
{
	const bool noNote = ea.comment.empty();
	const bool noColor = (ea.r == 255 && ea.g == 255 && ea.b == 255);
	return noNote && noColor;
}

IEventTreeOps::Handle MissionEventsDialogModel::resolveHandleFromPath(const SCP_list<int>& path) const
{
	if (path.empty())
		return nullptr;
	const int origEvt = path.front();
	const int formula = findFormulaByOriginalEventIndex(origEvt);
	if (formula < 0)
		return nullptr;

	auto h = m_event_tree_ops.get_root_by_formula(formula);
	auto it = path.begin();
	++it; // skip event index
	for (; it != path.end() && h; ++it) {
		const int childIdx = *it;
		if (childIdx < 0 || childIdx >= m_event_tree_ops.child_count(h))
			return nullptr;
		h = m_event_tree_ops.child_at(h, childIdx);
	}
	return h;
}

event_annotation& MissionEventsDialogModel::ensureAnnotationByPath(const SCP_list<int>& path)
{
	for (auto& ea : m_event_annotations)
		if (ea.path == path)
			return ea;
	event_annotation ea{};
	ea.path = path;
	m_event_annotations.push_back(ea);
	return m_event_annotations.back();
}

void MissionEventsDialogModel::initializeTeamList()
{
	m_team_list.clear();
	m_team_list.emplace_back("<None>", -1);
	for (auto& team : Mission_event_teams_tvt) {
		m_team_list.emplace_back(team.first, team.second);
	}
}

mission_event MissionEventsDialogModel::makeDefaultEvent()
{
	mission_event e{};
	e.name = "Event name";
	e.formula = -1;
	// Seems like most initializers are handled by the sexp_tree widget... This is so messy

	return e;
}

void MissionEventsDialogModel::applyAnnotations()
{
	// Recompute paths from whatever we currently have
	for (auto& ea : m_event_annotations) {
		// Prefer live handle if still valid
		if (ea.handle && m_event_tree_ops.is_handle_valid(ea.handle)) {
			ea.path = buildPathForHandle(ea.handle);
		} else {
			// If we lost the handle, try to resolve from the old path
			auto h = resolveHandleFromPath(ea.path);
			if (h) {
				ea.handle = h;                   // refresh cache for the rest of the session
				ea.path = buildPathForHandle(h); // normalize
			} else {
				// Node is gone; mark default so we prune
				ea.comment.clear();
				ea.r = ea.g = ea.b = 255;
				ea.handle = nullptr;
				ea.path.clear();
			}
		}
		// Drop the handle
		ea.handle = nullptr;
	}

	// Prune defaults
	m_event_annotations.erase(std::remove_if(m_event_annotations.begin(), m_event_annotations.end(), [](const event_annotation& ea) { return isDefaultAnnotation(ea); }), m_event_annotations.end());

	// Apply
	Event_annotations = m_event_annotations;
}


void MissionEventsDialogModel::initializeMessages()
{
	int num_messages = Num_messages - Num_builtin_messages;
	m_messages.clear();
	m_messages.reserve(num_messages);
	for (auto i = 0; i < num_messages; i++) {
		auto msg = Messages[i + Num_builtin_messages];
		m_messages.push_back(msg);
		if (m_messages[i].avi_info.name) {
			m_messages[i].avi_info.name = strdup(m_messages[i].avi_info.name);
		}
		if (m_messages[i].wave_info.name) {
			m_messages[i].wave_info.name = strdup(m_messages[i].wave_info.name);
		}
	}

	if (Num_messages > Num_builtin_messages) {
		setCurrentlySelectedMessage(0);
	} else {
		setCurrentlySelectedMessage(-1);
	}
}

void MissionEventsDialogModel::initializeHeadAniList()
{
	m_head_ani_list.clear();
	m_head_ani_list.emplace_back("<None>");

	if (!Disable_hc_message_ani) {
		m_head_ani_list.emplace_back("Head-TP2");
		m_head_ani_list.emplace_back("Head-TP3");
		m_head_ani_list.emplace_back("Head-TP4");
		m_head_ani_list.emplace_back("Head-TP5");
		m_head_ani_list.emplace_back("Head-TP6");
		m_head_ani_list.emplace_back("Head-TP7");
		m_head_ani_list.emplace_back("Head-TP8");
		m_head_ani_list.emplace_back("Head-VP1");
		m_head_ani_list.emplace_back("Head-VP2");
		m_head_ani_list.emplace_back("Head-CM1");
		m_head_ani_list.emplace_back("Head-CM2");
		m_head_ani_list.emplace_back("Head-CM3");
		m_head_ani_list.emplace_back("Head-CM4");
		m_head_ani_list.emplace_back("Head-CM5");
		m_head_ani_list.emplace_back("Head-VC");
		m_head_ani_list.emplace_back("Head-VC2");
		m_head_ani_list.emplace_back("Head-BSH");
	}

	for (auto& thisHead : Custom_head_anis) {
		m_head_ani_list.emplace_back(thisHead);
	}

	for (auto& msg : m_messages) {
		if (msg.avi_info.name) {
			auto it = std::find(m_head_ani_list.begin(), m_head_ani_list.end(), msg.avi_info.name);
			if (it == m_head_ani_list.end()) {
				m_head_ani_list.emplace_back(msg.avi_info.name);
			}
		}
	}
}

void MissionEventsDialogModel::initializeWaveList()
{
	m_wave_list.clear();
	m_wave_list.emplace_back("<None>");

	// Use the main Message vector so we also get the builtins?
	for (auto i = 0; i < Num_messages; i++) {
		if (Messages[i].wave_info.name) {
			auto it = std::find(m_wave_list.begin(), m_wave_list.end(), Messages[i].wave_info.name);
			if (it == m_wave_list.end()) {
				m_wave_list.emplace_back(Messages[i].wave_info.name);
			}
		}
	}
}

void MissionEventsDialogModel::initializePersonaList()
{
	m_persona_list.clear();
	m_persona_list.emplace_back("<None>", -1);
	for (int i = 0; i < static_cast<int>(Personas.size()); ++i) {
		auto& persona = Personas[i];
		m_persona_list.emplace_back(persona.name, i);
	}
}

bool MissionEventsDialogModel::checkMessageNameConflict(const SCP_string& name)
{
	// Validate against builtin messages
	for (auto i = 0; i < Num_builtin_messages; i++) {
		if (!stricmp(name.c_str(), Messages[i].name)) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Warning,
				"Invalid Message Name",
				"Message name cannot be the same as a builtin message name!",
				{DialogButton::Ok});
			return true;
			break;
		}
	}

	// Validate against existing messages
	for (auto i = 0; i < static_cast<int>(m_messages.size()); i++) {
		if ((i != m_cur_msg) && (!stricmp(name.c_str(), m_messages[i].name))) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Warning,
				"Invalid Message Name",
				"Message name cannot be the same another message!",
				{DialogButton::Ok});
			return true;
			break;
		}
	}

	return false;
}

SCP_string MissionEventsDialogModel::makeUniqueMessageName(const SCP_string& base) const
{
	const int maxLen = NAME_LENGTH - 1;

	auto exists = [&](const SCP_string& cand) -> bool {
		for (const auto& m : m_messages) {
			if (m.name[0] != '\0' && stricmp(m.name, cand.c_str()) == 0) {
				return true;
			}
		}
		return false;
	};

	// Try base, then base + " 1", base + " 2", ...
	for (int n = 0;; ++n) {
		SCP_string suffix = (n == 0) ? "" : (" " + std::to_string(n));
		const size_t avail = (maxLen > static_cast<int>(suffix.size()))
								 ? static_cast<size_t>(maxLen - static_cast<int>(suffix.size()))
								 : 0u;
		SCP_string head = base.substr(0, avail);
		SCP_string cand = head + suffix;
		if (!exists(cand))
			return cand;
	}
}

bool MissionEventsDialogModel::eventIsValid() const
{
	return SCP_vector_inbounds(m_events, m_cur_event);
}

bool MissionEventsDialogModel::messageIsValid() const
{
	return SCP_vector_inbounds(m_messages, m_cur_msg);
}

void MissionEventsDialogModel::setCurrentlySelectedEvent(int event)
{
	m_cur_event = event;
}

void MissionEventsDialogModel::setCurrentlySelectedEventByFormula(int formula)
{
	for (auto i = 0; i < static_cast<int>(m_events.size()); i++) {
		if (m_events[i].formula == formula) {
			setCurrentlySelectedEvent(i);
			return;
		}
	}
}

int MissionEventsDialogModel::getCurrentlySelectedEvent() const
{
	return m_cur_event;
}

SCP_vector<mission_event>& MissionEventsDialogModel::getEventList()
{
	return m_events;
}

void MissionEventsDialogModel::deleteRootNode(int node)
{
	int i;
	for (i = 0; i < static_cast<int>(m_events.size()); i++) {
		if (m_events[i].formula == node) {
			break;
		}
	}

	Assertion(i < static_cast<int>(m_events.size()), "Attempt to delete an invalid event!");
	m_events.erase(m_events.begin() + i);
	m_sig.erase(m_sig.begin() + i);

	if (i >= static_cast<int>(m_events.size())) // if we have deleted the last event,
		i--;                                    // i will be set to -1 which is what we want

	setCurrentlySelectedEvent(i);
	set_modified();
}

void MissionEventsDialogModel::renameRootNode(int node, const SCP_string& name)
{
	int i;
	for (i = 0; i < static_cast<int>(m_events.size()); i++) {
		if (m_events[i].formula == node) {
			break;
		}
	}
	Assertion(i < static_cast<int>(m_events.size()), "Attempt to rename an invalid event!");
	m_events[i].name = name;
	set_modified();
}

void MissionEventsDialogModel::changeRootNodeFormula(int old, int node)
{
	int i;
	for (i = 0; i < static_cast<int>(m_events.size()); i++) {
		if (m_events[i].formula == old) {
			break;
		}
	}

	Assertion(i < static_cast<int>(m_events.size()), "Attempt to modify invalid event!");
	m_events[i].formula = node;
	set_modified();
}

void MissionEventsDialogModel::reorderByRootFormulaOrder(const SCP_vector<int>& newOrderedFormulas)
{
	// Basic sanity: must be a 1:1 permutation of current roots
	if (newOrderedFormulas.size() != m_events.size())
		return;

	// Build the permuted arrays (O(n^2) is fine for event counts)
	SCP_vector<mission_event> newEvents;
	SCP_vector<int> newSig;
	newEvents.reserve(m_events.size());
	newSig.reserve(m_sig.size());

	for (int formula : newOrderedFormulas) {
		int oldIdx = -1;
		for (int i = 0; i < static_cast<int>(m_events.size()); ++i) {
			if (m_events[i].formula == formula) {
				oldIdx = i;
				break;
			}
		}
		if (oldIdx < 0) {
			// Unknown formula; bail without mutating state
			return;
		}
		newEvents.push_back(m_events[oldIdx]);
		if (SCP_vector_inbounds(m_sig, oldIdx)) {
			newSig.push_back(m_sig[oldIdx]);
		}
	}

	// Swap in the new order
	m_events.swap(newEvents);
	m_sig.swap(newSig);

	// Keep selection reasonable (select the first event after reorder)
	setCurrentlySelectedEvent(m_events.empty() ? -1 : 0);

	// Rebuild applied annotations against new handles/order if needed
	initializeEventAnnotations();

	set_modified();
}

void MissionEventsDialogModel::setCurrentlySelectedMessage(int msg)
{
	m_cur_msg = msg;

	audiostream_close_file(m_wave_id, false);
	m_wave_id = -1;
}

int MissionEventsDialogModel::getCurrentlySelectedMessage() const
{
	return m_cur_msg;
}

const SCP_vector<SCP_string>& MissionEventsDialogModel::getHeadAniList()
{
	return m_head_ani_list;
}

const SCP_vector<SCP_string>& MissionEventsDialogModel::getWaveList()
{
	return m_wave_list;
}

const SCP_vector<std::pair<SCP_string, int>>& MissionEventsDialogModel::getPersonaList()
{
	return m_persona_list;
}

const SCP_vector<std::pair<SCP_string, int>>& MissionEventsDialogModel::getTeamList()
{
	return m_team_list;
}

void MissionEventsDialogModel::createEvent()
{
	m_events.emplace_back(makeDefaultEvent());
	m_sig.push_back(-1);

	auto& event = m_events.back();

	const int after = event.formula;
	event.formula = m_event_tree_ops.build_default_root(event.name, after);
	m_event_tree_ops.select_root(event.formula);

	setCurrentlySelectedEventByFormula(event.formula);
	set_modified();
}

void MissionEventsDialogModel::insertEvent()
{
	if (m_cur_event < 0 || m_events.empty()) {
		createEvent();
		return;
	}

	const int pos = m_cur_event; // Can shift during tree ops so save our position now
	m_events.insert(m_events.begin() + pos, makeDefaultEvent());
	m_sig.insert(m_sig.begin() + pos, -1);
	auto& event = m_events[pos];

	// Place after the previous root if it exists and is valid; otherwise we’ll fix index explicitly
	int after = (pos > 0 && m_events[pos - 1].formula >= 0) ? m_events[pos - 1].formula : -1;

	event.formula = m_event_tree_ops.build_default_root(event.name, after);

	if (pos == 0) {
		m_event_tree_ops.ensure_top_level_index(event.formula, 0);
	}
	m_event_tree_ops.select_root(event.formula);

	setCurrentlySelectedEventByFormula(event.formula);
	set_modified();
}

void MissionEventsDialogModel::deleteEvent()
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}

	m_event_tree_ops.delete_event();
}

void MissionEventsDialogModel::renameEvent(int id, const SCP_string& name)
{
	// Find by formula id first; fallback to treating id as an index if you want.
	int idx = -1;
	for (int i = 0; i < static_cast<int>(m_events.size()); ++i) {
		if (m_events[i].formula == id) {
			idx = i;
			break;
		}
	}
	if (idx == -1 && id >= 0 && id < static_cast<int>(m_events.size()))
		idx = id;
	if (idx < 0)
		return;

	// Normalize to engine expectations
	SCP_string normalized = name.empty() ? SCP_string("<Unnamed>") : name;
	SCP_truncate(normalized, NAME_LENGTH - 1);

	modify(m_events[idx].name, normalized);
}

int MissionEventsDialogModel::getFormula() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return -1;
	}
	return m_events[m_cur_event].formula;
}

void MissionEventsDialogModel::setFormula(int node)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}

	auto& event = m_events[m_cur_event];
	modify(event.formula, node);
}

int MissionEventsDialogModel::getRepeatCount() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return 1;
	}
	return m_events[m_cur_event].repeat_count;
}

void MissionEventsDialogModel::setRepeatCount(int count)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (count < -1) {
		count = -1;
	}
	modify(event.repeat_count, count);
}

int MissionEventsDialogModel::getTriggerCount() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return 0;
	}
	return m_events[m_cur_event].trigger_count;
}

void MissionEventsDialogModel::setTriggerCount(int count)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (count < -1) {
		count = -1;
	}
	modify(event.trigger_count, count);
}

int MissionEventsDialogModel::getIntervalTime() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return 0;
	}
	return m_events[m_cur_event].interval;
}

void MissionEventsDialogModel::setIntervalTime(int time)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (time < 0) {
		time = 0;
	}
	modify(event.interval, time);
}

bool MissionEventsDialogModel::getChained() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].chain_delay >= 0);
}

void MissionEventsDialogModel::setChained(bool chained)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (chained) {
		modify(event.chain_delay, 0);
	} else {
		modify(event.chain_delay, -1);
	}
}

int MissionEventsDialogModel::getChainDelay() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return -1;
	}
	return m_events[m_cur_event].chain_delay;
}

void MissionEventsDialogModel::setChainDelay(int delay)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (delay < 0) {
		delay = 0;
	}
	modify(event.chain_delay, delay);
}

int MissionEventsDialogModel::getEventScore() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return 0;
	}
	return m_events[m_cur_event].score;
}

void MissionEventsDialogModel::setEventScore(int score)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	modify(event.score, score);
}

int MissionEventsDialogModel::getEventTeam() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return -1;
	}
	return m_events[m_cur_event].team;
}

void MissionEventsDialogModel::setEventTeam(int team)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}

	auto& event = m_events[m_cur_event];
	
	if (team < -1 || team >= MAX_TVT_TEAMS) {
		team = -1;
	}
	modify(event.team, team);
}

SCP_string MissionEventsDialogModel::getEventDirectiveText() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return "";
	}
	return m_events[m_cur_event].objective_text;
}

void MissionEventsDialogModel::setEventDirectiveText(const SCP_string& text)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	modify(event.objective_text, text);
	lcl_fred_replace_stuff(event.objective_text);
}

SCP_string MissionEventsDialogModel::getEventDirectiveKeyText() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return "";
	}
	return m_events[m_cur_event].objective_key_text;
}

void MissionEventsDialogModel::setEventDirectiveKeyText(const SCP_string& text)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	modify(event.objective_key_text, text);
	lcl_fred_replace_stuff(event.objective_key_text);
}

bool MissionEventsDialogModel::getLogTrue() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].mission_log_flags & MLF_SEXP_TRUE) != 0;
}

void MissionEventsDialogModel::setLogTrue(bool log)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (log) {
		event.mission_log_flags |= MLF_SEXP_TRUE;
	} else {
		event.mission_log_flags &= ~MLF_SEXP_TRUE;
	}
	set_modified();
}

bool MissionEventsDialogModel::getLogFalse() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].mission_log_flags & MLF_SEXP_FALSE) != 0;
}

void MissionEventsDialogModel::setLogFalse(bool log)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (log) {
		event.mission_log_flags |= MLF_SEXP_FALSE;
	} else {
		event.mission_log_flags &= ~MLF_SEXP_FALSE;
	}
	set_modified();
}

bool MissionEventsDialogModel::getLogLogPrevious() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].mission_log_flags & MLF_STATE_CHANGE) != 0;
}

void MissionEventsDialogModel::setLogLogPrevious(bool log)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (log) {
		event.mission_log_flags |= MLF_STATE_CHANGE;
	} else {
		event.mission_log_flags &= ~MLF_STATE_CHANGE;
	}
	set_modified();
}

bool MissionEventsDialogModel::getLogAlwaysFalse() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].mission_log_flags & MLF_SEXP_KNOWN_FALSE) != 0;
}

void MissionEventsDialogModel::setLogAlwaysFalse(bool log)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (log) {
		event.mission_log_flags |= MLF_SEXP_KNOWN_FALSE;
	} else {
		event.mission_log_flags &= ~MLF_SEXP_KNOWN_FALSE;
	}
	set_modified();
}

bool MissionEventsDialogModel::getLogFirstRepeat() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].mission_log_flags & MLF_FIRST_REPEAT_ONLY) != 0;
}

void MissionEventsDialogModel::setLogFirstRepeat(bool log)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (log) {
		event.mission_log_flags |= MLF_FIRST_REPEAT_ONLY;
	} else {
		event.mission_log_flags &= ~MLF_FIRST_REPEAT_ONLY;
	}
	set_modified();
}

bool MissionEventsDialogModel::getLogLastRepeat() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].mission_log_flags & MLF_LAST_REPEAT_ONLY) != 0;
}

void MissionEventsDialogModel::setLogLastRepeat(bool log)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (log) {
		event.mission_log_flags |= MLF_LAST_REPEAT_ONLY;
	} else {
		event.mission_log_flags &= ~MLF_LAST_REPEAT_ONLY;
	}
	set_modified();
}

bool MissionEventsDialogModel::getLogFirstTrigger() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].mission_log_flags & MLF_FIRST_TRIGGER_ONLY) != 0;
}

void MissionEventsDialogModel::setLogFirstTrigger(bool log)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (log) {
		event.mission_log_flags |= MLF_FIRST_TRIGGER_ONLY;
	} else {
		event.mission_log_flags &= ~MLF_FIRST_TRIGGER_ONLY;
	}
	set_modified();
}

bool MissionEventsDialogModel::getLogLastTrigger() const
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return false;
	}
	return (m_events[m_cur_event].mission_log_flags & MLF_LAST_TRIGGER_ONLY) != 0;
}

void MissionEventsDialogModel::setLogLastTrigger(bool log)
{
	if (!SCP_vector_inbounds(m_events, m_cur_event)) {
		return;
	}
	auto& event = m_events[m_cur_event];
	if (log) {
		event.mission_log_flags |= MLF_LAST_TRIGGER_ONLY;
	} else {
		event.mission_log_flags &= ~MLF_LAST_TRIGGER_ONLY;
	}
	set_modified();
}

void MissionEventsDialogModel::setNodeAnnotation(IEventTreeOps::Handle h, const SCP_string& note)
{
	auto path = buildPathForHandle(h);
	auto& ea = ensureAnnotationByPath(path);
	ea.handle = h;
	ea.comment = note;
	m_event_tree_ops.set_node_note(h, note);
	set_modified();
}

void MissionEventsDialogModel::setNodeBgColor(IEventTreeOps::Handle h, int r, int g, int b, bool has_color)
{
	auto path = buildPathForHandle(h);
	auto& ea = ensureAnnotationByPath(path);
	ea.handle = h;
	if (has_color) {
		ea.r = (ubyte)r;
		ea.g = (ubyte)g;
		ea.b = (ubyte)b;
	} else {
		ea.r = ea.g = ea.b = 255;
	}
	m_event_tree_ops.set_node_bg_color(h, r, g, b, has_color);
	set_modified();
}

void MissionEventsDialogModel::createMessage()
{
	MMessage msg;

	const SCP_string base = "<new message>";
	const SCP_string unique = makeUniqueMessageName(base);

	strcpy_s(msg.name, unique.c_str());
	strcpy_s(msg.message, "<put description here>");
	msg.avi_info.name = nullptr;
	msg.wave_info.name = nullptr;
	msg.persona_index = -1;
	msg.multi_team = -1;
	m_messages.push_back(msg);
	auto id = static_cast<int>(m_messages.size()) - 1;

	setCurrentlySelectedMessage(id);

	set_modified();
}

void MissionEventsDialogModel::insertMessage()
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		createMessage();
		return;
	}

	MMessage msg;
	const SCP_string base = "<new message>";
	const SCP_string unique = makeUniqueMessageName(base);

	strcpy_s(msg.name, unique.c_str());
	strcpy_s(msg.message, "<put description here>");
	msg.avi_info.name = nullptr;
	msg.wave_info.name = nullptr;
	msg.persona_index = -1;
	msg.multi_team = -1;

	// Insert at requested position
	m_messages.insert(m_messages.begin() + m_cur_msg, msg);

	// Select the new message
	setCurrentlySelectedMessage(m_cur_msg);

	set_modified();
}

void MissionEventsDialogModel::deleteMessage()
{
	// handle this case somewhat gracefully
	Assertion(SCP_vector_inbounds(m_messages, m_cur_msg),
		"Unexpected m_cur_msg value (%d); expected either -1, or between 0-%d. Get a coder!\n",
		m_cur_msg,
		static_cast<int>(m_messages.size()) - 1);
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	audiostream_close_file(m_wave_id, false);
	m_wave_id = -1;

	if (m_messages[m_cur_msg].avi_info.name) {
		free(m_messages[m_cur_msg].avi_info.name);
		m_messages[m_cur_msg].avi_info.name = nullptr;
	}
	if (m_messages[m_cur_msg].wave_info.name) {
		free(m_messages[m_cur_msg].wave_info.name);
		m_messages[m_cur_msg].wave_info.name = nullptr;
	}

	SCP_string buf = "<" + SCP_string(m_messages[m_cur_msg].name) + ">";
	update_sexp_references(m_messages[m_cur_msg].name, buf.c_str(), OPF_MESSAGE);
	update_sexp_references(m_messages[m_cur_msg].name, buf.c_str(), OPF_MESSAGE_OR_STRING);

	m_messages.erase(m_messages.begin() + m_cur_msg);

	if (m_cur_msg >= static_cast<int>(m_messages.size())) {
		m_cur_msg = static_cast<int>(m_messages.size()) - 1;
	}

	setCurrentlySelectedMessage(m_cur_msg);

	set_modified();
}

void MissionEventsDialogModel::moveMessageUp()
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg) || m_cur_msg == 0)
		return;

	std::swap(m_messages[m_cur_msg - 1], m_messages[m_cur_msg]);
	setCurrentlySelectedMessage(m_cur_msg - 1);
	set_modified();
}

void MissionEventsDialogModel::moveMessageDown()
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg) || m_cur_msg >= static_cast<int>(m_messages.size()) - 1)
		return;

	std::swap(m_messages[m_cur_msg + 1], m_messages[m_cur_msg]);
	setCurrentlySelectedMessage(m_cur_msg + 1);
	set_modified();
}


SCP_string MissionEventsDialogModel::getMessageName() const
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return "";
	}
	return m_messages[m_cur_msg].name;
}

void MissionEventsDialogModel::setMessageName(const SCP_string& name)
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	auto& msg = m_messages[m_cur_msg];

	if (!checkMessageNameConflict(name)) {
		strncpy(msg.name, name.c_str(), NAME_LENGTH - 1);
		set_modified();
	}
}

SCP_string MissionEventsDialogModel::getMessageText() const
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return "";
	}
	return m_messages[m_cur_msg].message;
}

void MissionEventsDialogModel::setMessageText(const SCP_string& text)
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	auto& msg = m_messages[m_cur_msg];

	strncpy(msg.message, text.c_str(), MESSAGE_LENGTH - 1);
	lcl_fred_replace_stuff(msg.message, MESSAGE_LENGTH - 1);

	set_modified();
}

SCP_string MissionEventsDialogModel::getMessageNote() const
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return "";
	}
	return m_messages[m_cur_msg].note;
}

void MissionEventsDialogModel::setMessageNote(const SCP_string& note)
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	auto& msg = m_messages[m_cur_msg];

	modify(msg.note, note);
}

SCP_string MissionEventsDialogModel::getMessageAni() const
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return "";
	}
	auto& msg = m_messages[m_cur_msg];
	return msg.avi_info.name ? SCP_string(msg.avi_info.name) : SCP_string("<None>");
}

void MissionEventsDialogModel::setMessageAni(const SCP_string& ani)
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	auto& msg = m_messages[m_cur_msg];
	const char* cur = msg.avi_info.name;
	const SCP_string curStr = cur ? cur : "";

	// Treat empty, "none", "<none>" as no avi and store nullptr
	const bool isNone = ani.empty() || lcase_equal(ani, "<none>") || lcase_equal(ani, "none");
	if (isNone) {
		if (cur != nullptr) { // only do work if changing something
			free(msg.avi_info.name);
			msg.avi_info.name = nullptr;
			set_modified();
		}
		return;
	}

	// No change? bail
	if (cur && curStr == ani) {
		return;
	}

	// Replace value
	if (cur)
		free(msg.avi_info.name);
	msg.avi_info.name = strdup(ani.c_str());
	set_modified();

	// Possibly add to list of known anis
	auto it = std::find_if(m_head_ani_list.begin(), m_head_ani_list.end(), [&](const SCP_string& s) {
		return lcase_equal(s, ani);
	});
	if (it == m_head_ani_list.end()) {
		m_head_ani_list.emplace_back(ani);
	}
}

SCP_string MissionEventsDialogModel::getMessageWave() const
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return "";
	}
	auto& msg = m_messages[m_cur_msg];
	return msg.wave_info.name ? SCP_string(msg.wave_info.name) : SCP_string("<None>");
}

void MissionEventsDialogModel::setMessageWave(const SCP_string& wave)
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	audiostream_close_file(m_wave_id, false);
	m_wave_id = -1;

	auto& msg = m_messages[m_cur_msg];
	const char* cur = msg.wave_info.name;
	const SCP_string curStr = cur ? cur : "";

	// Treat empty, "none", "<none>" as no avi and store nullptr
	const bool isNone = wave.empty() || lcase_equal(wave, "<none>") || lcase_equal(wave, "none");
	if (isNone) {
		if (cur != nullptr) { // only do work if changing something
			free(msg.wave_info.name);
			msg.wave_info.name = nullptr;
			set_modified();
		}
		return;
	}

	// No change? bail
	if (cur && curStr == wave) {
		return;
	}

	// Replace value
	if (cur)
		free(msg.wave_info.name);
	msg.wave_info.name = strdup(wave.c_str());
	set_modified();

	autoSelectPersona();
}

int MissionEventsDialogModel::getMessagePersona() const
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return -1;
	}
	auto& msg = m_messages[m_cur_msg];
	if (SCP_vector_inbounds(Personas, msg.persona_index)) {
		return msg.persona_index;
	}
	return -1;
}

void MissionEventsDialogModel::setMessagePersona(int persona)
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	auto& msg = m_messages[m_cur_msg];

	msg.persona_index = persona;
	set_modified();
}

int MissionEventsDialogModel::getMessageTeam() const
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return -1;
	}
	auto& msg = m_messages[m_cur_msg];
	if (msg.multi_team < 0 || msg.multi_team >= MAX_TVT_TEAMS) {
		return -1;
	}
	return msg.multi_team;
}

void MissionEventsDialogModel::setMessageTeam(int team)
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	auto& msg = m_messages[m_cur_msg];

	if (team >= MAX_TVT_TEAMS) {
		msg.multi_team = -1;
	} else {
		msg.multi_team = team;
	}
	set_modified();
}

void MissionEventsDialogModel::autoSelectPersona()
{
	// I hate everything about this function outside of retail but someone will complain
	// if I omit this "feature"...

	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	SCP_string wave_name = m_messages[m_cur_msg].wave_info.name ? m_messages[m_cur_msg].wave_info.name : "";
	SCP_string avi_name = m_messages[m_cur_msg].avi_info.name ? m_messages[m_cur_msg].avi_info.name : "";

	if ((wave_name[0] >= '1') && (wave_name[0] <= '9') && (wave_name[1] == '_')) {
		auto i = wave_name[0] - '1';
		if ((i < static_cast<int>(Personas.size())) && (Personas[i].flags & PERSONA_FLAG_WINGMAN)) {
			modify(m_messages[m_cur_msg].persona_index, i);
			if (i == 0 || i == 1) {
				avi_name = "HEAD-TP1";
			} else if (i == 2 || i == 3) {
				avi_name = "HEAD-TP2";
			} else if (i == 4) {
				avi_name = "HEAD-TP3";
			} else if (i == 5) {
				avi_name = "HEAD-VP1";
			}
		}
	} else {
		auto mask = 0;
		if (!strnicmp(wave_name.c_str(), "S_", 2)) {
			mask = PERSONA_FLAG_SUPPORT;
			avi_name = "HEAD-CM1";
		} else if (!strnicmp(wave_name.c_str(), "L_", 2)) {
			mask = PERSONA_FLAG_LARGE;
			avi_name = "HEAD-CM1";
		} else if (!strnicmp(wave_name.c_str(), "TC_", 3)) {
			mask = PERSONA_FLAG_COMMAND;
			avi_name = "HEAD-CM1";
		}

		for (auto i = 0; i < (static_cast<int>(Personas.size())); i++) {
			if (Personas[i].flags & mask) {
				modify(m_messages[m_cur_msg].persona_index, i);
			}
		}
	}

	SCP_string original_avi_name = avi_name;
	if (m_messages[m_cur_msg].avi_info.name) {
		free(m_messages[m_cur_msg].avi_info.name);
		m_messages[m_cur_msg].avi_info.name = nullptr;
	}
	m_messages[m_cur_msg].avi_info.name = strdup(avi_name.c_str());

	if (original_avi_name != avi_name) {
		set_modified();
	}
}

void MissionEventsDialogModel::playMessageWave()
{
	if (!SCP_vector_inbounds(m_messages, m_cur_msg)) {
		return;
	}

	//audiostream_close_file(m_wave_id, false);

	auto& msg = m_messages[m_cur_msg];

	if (msg.wave_info.name) {
		m_wave_id = audiostream_open(msg.wave_info.name, ASF_VOICE);
		if (m_wave_id >= 0) {
			audiostream_play(m_wave_id, 1.0f, 0);
		}
	}
}

const SCP_vector<MMessage>& MissionEventsDialogModel::getMessageList() const
{
	return m_messages;
}

bool MissionEventsDialogModel::getMissionIsMultiTeam()
{
	return The_mission.game_type & MISSION_TYPE_MULTI_TEAMS;
}

void MissionEventsDialogModel::setModified() {
	set_modified();
}

} // namespace fso::fred::dialogs