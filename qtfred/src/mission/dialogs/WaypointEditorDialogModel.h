#pragma once

#include "mission/dialogs/AbstractDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

class WaypointEditorDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	struct PointListElement {
		SCP_string name;
		int id = -1;

		PointListElement(const SCP_string& name, int id);
	};

	WaypointEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;

	void reject() override;

	static const int ID_JUMP_NODE_MENU = 8000;
	static const int ID_WAYPOINT_MENU = 9000;

	const SCP_string& getCurrentName() const;
	int getCurrentElementId() const;
	bool isEnabled() const;
	const SCP_vector<WaypointEditorDialogModel::PointListElement>& getElements() const;

	void idSelected(int elementId);
	void setNameEditText(const SCP_string& name);
 private:
	bool showErrorDialog(const SCP_string& message, const SCP_string title);

	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void missionChanged();

	void updateElementList();

	void initializeData();

	SCP_string _currentName;
	int _currentElementId = -1;
	bool _enabled = false;
	SCP_vector<PointListElement> _elements;

	bool bypass_errors = false;
};

}
}
}
