#include "missioneditor/sexp_annotation_model.h"
#include "missioneditor/sexp_tree_model.h"

SCP_vector<event_annotation> Event_annotations;

// -----------------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------------

// Copy the global Event_annotations into our working set and resolve each
// stored path to a live annotation key (tree_nodes[] index or root key).
// Annotations whose paths can't be resolved (e.g. event was deleted) are
// reset to default values so they'll be pruned on save.
void SexpAnnotationModel::loadFromGlobal(const SCP_vector<sexp_tree_item>& tree_nodes, const SCP_vector<mission_event>& events, const SCP_vector<int>& sig)
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

// Rebuild persistable paths from annotation keys, attempt fallback resolution
// for any lost nodes, prune default-valued annotations, and write the result
// back to the global Event_annotations.
void SexpAnnotationModel::saveToGlobal(const SCP_vector<sexp_tree_item>& tree_nodes, const SCP_vector<mission_event>& events, const SCP_vector<int>& sig)
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
			}
		}

		// If no path could be built, the annotated node no longer exists (e.g. its
		// event was deleted); mark default so prune() removes it rather than letting
		// an unattachable annotation survive in the global list.
		if (ea.path.empty()) {
			ea.comment.clear();
			ea.r = ea.g = ea.b = 255;
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

// Return the vector index of the annotation with the given key, or -1 if not found.
int SexpAnnotationModel::findByKey(int key) const
{
	// -1 is the default/unresolved sentinel, not a real key; matching it would
	// surface an unresolved annotation on any item that has no key of its own
	if (key == -1)
		return -1;

	for (size_t i = 0; i < m_annotations.size(); ++i) {
		if (m_annotations[i].node_index == key)
			return static_cast<int>(i);
	}
	return -1;
}

// Return a const pointer to the annotation with the given key, or nullptr.
const event_annotation* SexpAnnotationModel::getByKey(int key) const
{
	int idx = findByKey(key);
	return (idx >= 0) ? &m_annotations[idx] : nullptr;
}

// Return a mutable pointer to the annotation with the given key, or nullptr.
event_annotation* SexpAnnotationModel::getByKey(int key)
{
	int idx = findByKey(key);
	return (idx >= 0) ? &m_annotations[idx] : nullptr;
}

// Get or create an annotation for the given key. If one already exists it is
// returned; otherwise a new default-valued annotation is appended.
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

// True if the annotation has default values (empty comment, white color).
bool SexpAnnotationModel::isDefault(const event_annotation& ea)
{
	return ea.comment.empty() && ea.r == 255 && ea.g == 255 && ea.b == 255;
}

// -----------------------------------------------------------------------
// Mutation
// -----------------------------------------------------------------------

// Remove the annotation with the given key, if one exists.
void SexpAnnotationModel::removeByKey(int key)
{
	int idx = findByKey(key);
	if (idx >= 0) {
		m_annotations.erase(m_annotations.begin() + idx);
	}
}

// Remove all annotations that have default values (no useful data).
void SexpAnnotationModel::prune()
{
	m_annotations.erase(
		std::remove_if(m_annotations.begin(), m_annotations.end(),
		               [](const event_annotation& ea) { return isDefault(ea); }),
		m_annotations.end());
}

// Discard all annotations from the working set.
void SexpAnnotationModel::clear()
{
	m_annotations.clear();
}

// -----------------------------------------------------------------------
// Path building (key -> path)
// -----------------------------------------------------------------------

// Build a persistable path from an annotation key to identify the node across
// save/load cycles. The path is a list of integers: [event_index, child_pos, ...].
// For root keys the path is just [event_index]. For regular nodes the path walks
// from the root down, recording the child position at each level.
SCP_list<int> SexpAnnotationModel::buildPath(int key, const SCP_vector<sexp_tree_item>& tree_nodes, const SCP_vector<mission_event>& events)
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
	// Guard against cycles with a depth limit based on the tree size.
	int root = key;
	int depth = 0;
	const int max_depth = static_cast<int>(tree_nodes.size());
	while (tree_nodes[root].parent >= 0 && depth < max_depth) {
		root = tree_nodes[root].parent;
		++depth;
	}
	if (depth >= max_depth)
		return path; // cycle detected

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
	path.push_back(0);	// child position under the event label: the top operator is its only child

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
// Path resolution (path -> key)
// -----------------------------------------------------------------------

// Resolve a stored path back to an annotation key by mapping the original event
// index through the sig table, then walking child positions down the tree.
// Returns a tree_nodes[] index (>= 0), a root key (<= -2), or -1 on failure.
int SexpAnnotationModel::resolveFromPath(const SCP_list<int>& path, const SCP_vector<sexp_tree_item>& tree_nodes, const SCP_vector<mission_event>& events, const SCP_vector<int>& sig)
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

	auto it = path.begin();
	++it; // skip event index

	// A label has exactly one child; any other index fails.
	if (*it != 0)
		return -1;
	++it;

	// Walk down the tree from the formula node.
	int node = formula;
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
