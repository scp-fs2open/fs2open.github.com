#pragma once

#include "mission/missiongoals.h"

struct event_annotation {
	int node_index = -1; // index into sexp tree_nodes[] (-1 if unresolved); transient, not persisted
	int item_image = -1; // the previous image of the tree node (replaced by a comment icon when there is a comment)
	SCP_list<int> path;  // a way to find the node that the annotation represents:
						 // the first number is the event, the second number is the node on the first layer, etc.
	SCP_string comment;
	ubyte r = 255;
	ubyte g = 255;
	ubyte b = 255;
};
extern SCP_vector<event_annotation> Event_annotations;

struct sexp_tree_item;

class SexpAnnotationModel {
public:
	// ---------------------------------------------------------------
	// Lifecycle
	// ---------------------------------------------------------------

	// Copy Event_annotations into the local working set, resolving each
	// stored path to an annotation key (node index or root key).
	// `sig` maps current dialog event index → original Mission_events index.
	void loadFromGlobal(const SCP_vector<sexp_tree_item>& tree_nodes, const SCP_vector<mission_event>& events, const SCP_vector<int>& sig);

	// Rebuild paths from annotation keys, prune defaults, and write the
	// result back to the global Event_annotations.
	void saveToGlobal(const SCP_vector<sexp_tree_item>& tree_nodes, const SCP_vector<mission_event>& events, const SCP_vector<int>& sig);

	// ---------------------------------------------------------------
	// Lookup
	// ---------------------------------------------------------------

	// Find the vector index of the annotation with the given key, or -1.
	int findByKey(int key) const;

	// Get a pointer to the annotation with the given key, or nullptr.
	const event_annotation* getByKey(int key) const;
	event_annotation* getByKey(int key);

	// Get or create an annotation for the given key.
	event_annotation& ensureByKey(int key);

	// ---------------------------------------------------------------
	// Predicates
	// ---------------------------------------------------------------

	// True if the annotation has default values (empty comment, white color).
	static bool isDefault(const event_annotation& ea);

	// ---------------------------------------------------------------
	// Mutation
	// ---------------------------------------------------------------

	// Remove the annotation with the given key, if one exists.
	void removeByKey(int key);

	// Remove all annotations that are at default values.
	void prune();

	// Discard all annotations.
	void clear();

	// ---------------------------------------------------------------
	// Direct access
	// ---------------------------------------------------------------

	SCP_vector<event_annotation>& annotations() { return m_annotations; }
	const SCP_vector<event_annotation>& annotations() const { return m_annotations; }

	// ---------------------------------------------------------------
	// Root key encoding helpers
	// ---------------------------------------------------------------
	// Root labels (event names) don't have a tree_nodes[] entry.
	// We encode them as -(formula + 2), which is always <= -2,
	// avoiding collision with -1 (the default/unresolved sentinel).

	static int rootKey(int formula) { return -(formula + 2); }
	static bool isRootKey(int key) { return key <= -2; }
	static int formulaFromRootKey(int key) { return -(key + 2); }

	// ---------------------------------------------------------------
	// Path conversion
	// ---------------------------------------------------------------
	// Public so editors can serialize annotations in a rebuild-stable form
	// (annotation keys are tree_nodes[] indices, which do not survive a
	// tree reload).

	// Build a persistable path from an annotation key.
	static SCP_list<int> buildPath(int key, const SCP_vector<sexp_tree_item>& tree_nodes, const SCP_vector<mission_event>& events);

	// Resolve a stored path back to an annotation key.
	// Returns the key (>= 0 for regular nodes, <= -2 for root keys) or -1 on failure.
	static int resolveFromPath(const SCP_list<int>& path, const SCP_vector<sexp_tree_item>& tree_nodes, const SCP_vector<mission_event>& events, const SCP_vector<int>& sig);

private:
	SCP_vector<event_annotation> m_annotations;
};
