/*
 * SexpAnnotationModel — shared annotation data model for event editors.
 * See sexp_annotation_model.h for class documentation.
 */

#include "missioneditor/sexp_annotation_model.h"
#include "missioneditor/sexp_tree_model.h"

#include <algorithm>
#include <vector>

// -----------------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------------

void SexpAnnotationModel::loadFromGlobal(const SCP_vector<sexp_tree_item>& tree_nodes,
                                         const SCP_vector<mission_event>& events,
                                         const SCP_vector<int>& sig)
{
	m_annotations = Event_annotations;

	for (auto& ea : m_annotations) {
		ea.node_index = -1;

		if (ea.path.empty())
			continue;

		int key = resolveFromPath(ea.path, tree_nodes, events, sig);
		if (key != -1) {
			ea.node_index = key;
		} else {
			// Path could not be resolved (event was probably deleted).
			// Reset to default so it will be pruned on save.
			ea.comment.clear();
			ea.r = ea.g = ea.b = 255;
			ea.node_index = -1;
		}
	}
}

void SexpAnnotationModel::saveToGlobal(const SCP_vector<sexp_tree_item>& tree_nodes,
                                       const SCP_vector<mission_event>& events,
                                       const SCP_vector<int>& sig)
{
	for (auto& ea : m_annotations) {
		int key = ea.node_index;
		SCP_list<int> old_path = ea.path;
		ea.path.clear();

		if (key >= 0 && key < static_cast<int>(tree_nodes.size()) &&
		    tree_nodes[key].type != SEXPT_UNUSED) {
			ea.path = buildPath(key, tree_nodes, events);
		} else if (isRootKey(key)) {
			ea.path = buildPath(key, tree_nodes, events);
		} else {
			// Node lost — try to resolve from the old path as a fallback.
			// (The old path may use original event indices, so we need sig.)
			int resolved = resolveFromPath(old_path, tree_nodes, events, sig);
			if (resolved >= 0 || isRootKey(resolved)) {
				ea.path = buildPath(resolved, tree_nodes, events);
			} else {
				// Truly gone; mark default for pruning.
				ea.comment.clear();
				ea.r = ea.g = ea.b = 255;
			}
		}

		// Reset transient field.
		ea.node_index = -1;
	}

	prune();
	Event_annotations = m_annotations;
}

// -----------------------------------------------------------------------
// Lookup
// -----------------------------------------------------------------------

int SexpAnnotationModel::findByKey(int key) const
{
	for (size_t i = 0; i < m_annotations.size(); ++i) {
		if (m_annotations[i].node_index == key)
			return static_cast<int>(i);
	}
	return -1;
}

const event_annotation* SexpAnnotationModel::getByKey(int key) const
{
	int idx = findByKey(key);
	return (idx >= 0) ? &m_annotations[idx] : nullptr;
}

event_annotation* SexpAnnotationModel::getByKey(int key)
{
	int idx = findByKey(key);
	return (idx >= 0) ? &m_annotations[idx] : nullptr;
}

event_annotation& SexpAnnotationModel::ensureByKey(int key)
{
	auto* existing = getByKey(key);
	if (existing)
		return *existing;

	m_annotations.emplace_back();
	auto& ea = m_annotations.back();
	ea.node_index = key;
	return ea;
}

// -----------------------------------------------------------------------
// Predicates
// -----------------------------------------------------------------------

bool SexpAnnotationModel::isDefault(const event_annotation& ea)
{
	return ea.comment.empty() && ea.r == 255 && ea.g == 255 && ea.b == 255;
}

// -----------------------------------------------------------------------
// Mutation
// -----------------------------------------------------------------------

void SexpAnnotationModel::prune()
{
	m_annotations.erase(
		std::remove_if(m_annotations.begin(), m_annotations.end(),
		               [](const event_annotation& ea) { return isDefault(ea); }),
		m_annotations.end());
}

void SexpAnnotationModel::clear()
{
	m_annotations.clear();
}

// -----------------------------------------------------------------------
// Path building (key → path)
// -----------------------------------------------------------------------

SCP_list<int> SexpAnnotationModel::buildPath(int key,
                                             const SCP_vector<sexp_tree_item>& tree_nodes,
                                             const SCP_vector<mission_event>& events) const
{
	SCP_list<int> path;

	// --- Root key: path is just [event_index] ---
	if (isRootKey(key)) {
		int formula = formulaFromRootKey(key);
		for (int i = 0; i < static_cast<int>(events.size()); ++i) {
			if (events[i].formula == formula) {
				path.push_back(i);
				return path;
			}
		}
		return path; // empty = formula not found
	}

	// --- Regular node ---
	if (key < 0 || key >= static_cast<int>(tree_nodes.size()))
		return path;

	// Walk up to find the root (a node with no parent).
	int root = key;
	while (tree_nodes[root].parent >= 0)
		root = tree_nodes[root].parent;

	// Find the event index whose formula matches this root.
	int event_idx = -1;
	for (int i = 0; i < static_cast<int>(events.size()); ++i) {
		if (events[i].formula == root) {
			event_idx = i;
			break;
		}
	}
	if (event_idx < 0)
		return path; // root not found in events

	path.push_back(event_idx);

	// Collect child-position indices from the target node up to the root,
	// then reverse them so the path reads top-down.
	std::vector<int> positions;
	int cur = key;
	for (;;) {
		int parent = tree_nodes[cur].parent;
		if (parent < 0)
			break;

		int pos = 0;
		int sibling = tree_nodes[parent].child;
		while (sibling >= 0 && sibling != cur) {
			++pos;
			sibling = tree_nodes[sibling].next;
		}
		positions.push_back(pos);
		cur = parent;
	}

	for (auto rit = positions.rbegin(); rit != positions.rend(); ++rit)
		path.push_back(*rit);

	return path;
}

// -----------------------------------------------------------------------
// Path resolution (path → key)
// -----------------------------------------------------------------------

int SexpAnnotationModel::resolveFromPath(const SCP_list<int>& path,
                                         const SCP_vector<sexp_tree_item>& tree_nodes,
                                         const SCP_vector<mission_event>& events,
                                         const SCP_vector<int>& sig) const
{
	if (path.empty())
		return -1;

	int orig_event_idx = path.front();

	// Map the original event index (from the saved path) to a formula
	// using the sig table (which maps current dialog index → original index).
	int formula = -1;
	for (int i = 0; i < static_cast<int>(sig.size()); ++i) {
		if (sig[i] == orig_event_idx) {
			if (i < static_cast<int>(events.size())) {
				formula = events[i].formula;
			}
			break;
		}
	}
	if (formula < 0)
		return -1;

	// Path of length 1 = root label annotation.
	if (path.size() == 1)
		return rootKey(formula);

	// Walk down the tree from the formula node.
	int node = formula;
	auto it = path.begin();
	++it; // skip event index
	for (; it != path.end() && node >= 0; ++it) {
		int target = *it;
		if (node < 0 || node >= static_cast<int>(tree_nodes.size()))
			return -1;
		int child = tree_nodes[node].child;
		for (int c = 0; c < target && child >= 0; ++c) {
			child = tree_nodes[child].next;
		}
		node = child;
	}

	return node; // >= 0 if resolution succeeded, -1 if not
}
