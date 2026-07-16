#pragma once
#include "mission/dialogs/AbstractDialogModel.h"

namespace fso::fred::dialogs {

// Model for the Reorder dialog.  A direct-edit model: each move is applied to the
// mission immediately, reordering how the object type is written to the mission file 
// and shown in Scene Browser.
class ReorderDialogModel : public AbstractDialogModel {
	Q_OBJECT

public:
	enum class Type {
		Ships,
		Wings,
		Props,
		WaypointLists,
		JumpNodes,
	};

	ReorderDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	// Display names for the given type, in current storage (mission-file) order.
	SCP_vector<SCP_string> getItemNames(Type type) const;

	// Move the item at display position from_pos to to_pos for the given type,
	// applying the reorder to the mission immediately.  No-op if from_pos == to_pos
	// or either position is out of range.
	void moveItem(Type type, int from_pos, int to_pos);

private: // NOLINT(readability-redundant-access-specifiers)
	// The occupied storage indices for the given type, in display order.  For
	// ships/wings/props/jump nodes these are the live Ships[]/Wings[]/Props[]/
	// Jump_nodes[] slots; for waypoint lists they are simply 0..N-1.
	SCP_vector<int> getSlots(Type type) const;
};

} // namespace fso::fred::dialogs
