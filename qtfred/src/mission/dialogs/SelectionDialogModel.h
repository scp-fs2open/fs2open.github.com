#pragma once

#include <QtCore/QObject>

#include <mission/EditorViewport.h>

#include "AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class SelectionDialogModel : public AbstractDialogModel {
	Q_OBJECT
 public:
	struct ListEntry {
		SCP_string name;
		int id = -1;
		bool selected = false;
	};

 private:
	void initializeData();

	void updateObjectList();

	void updateWingListSelection();

	void updateShipListSelection();

	void updateStatus(bool first_time);

	SCP_vector<ListEntry> _obj_list;

	SCP_vector<ListEntry> _wing_list;

	SCP_vector<ListEntry> _waypoint_list;

	bool _filter_ships = true;
	bool _filter_starts = true;
	bool _filter_waypoints = true;
	SCP_vector<bool> _filter_iff;

 public:
	bool apply() override;
	void reject() override;

signals:
	void selectionUpdated();

 public:
	SelectionDialogModel(QObject* parent, EditorViewport* viewport);

	void clearSelection();

	void selectAll();

	void invertSelection();

	const SCP_vector<ListEntry>& getObjectList() const;

	const SCP_vector<ListEntry>& getWingList() const;

	const SCP_vector<ListEntry>& getWaypointList() const;

	bool isFilterShips() const;
	void setFilterShips(bool filter_ships);

	bool isFilterStarts() const;
	void setFilterStarts(bool filter_starts);

	bool isFilterWaypoints() const;
	void setFilterWaypoints(bool filter_waypoints);

	bool isFilterIFFTeam(int team) const;
	void setFilterIFFTeam(int team, bool filter);

	/**
	 * @brief Updates the selection status of the objects in the list
	 *
	 * The list must be the same as returned by getObjectList with only the selection status changed.
	 *
	 * @param newSelection The new selection list
	 */
	void updateObjectSelection(const SCP_vector<ListEntry>& newSelection);

	void selectWing(int wing_id);

	void selectWaypointPath(int wp_id);
};

}
}
}


