#pragma once
#include "mission/dialogs/AbstractDialogModel.h"

namespace fso::fred::dialogs {

class WaypointEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT

public:
	WaypointEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	bool hasValidSelection() const;
	bool hasMultipleSelection() const;
	bool hasAnyPathsInMission() const;
	int getSelectionCount() const;

	const SCP_string& getCurrentName() const;
	bool setCurrentName(const SCP_string& name);

	bool getNoDrawLines() const;
	void setNoDrawLines(bool val);
	bool getHasCustomColor() const;
	void setHasCustomColor(bool val);
	int getColorR() const;
	int getColorG() const;
	int getColorB() const;
	void setColorR(int r);
	void setColorG(int g);
	void setColorB(int b);

	SCP_string getLayer() const;
	void setLayer(const SCP_string& layer);

	void selectNextPath();
	void selectPreviousPath();

signals:
	void waypointPathMarkingChanged();

private slots:
	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void onMissionChanged();

private:
	void initializeData();
	void showErrorDialogNoCancel(const SCP_string& message);
	bool validateName(const SCP_string& name);
	void selectWaypointPathByIndex(int idx);

	SCP_vector<int> _selectedWaypointPaths; // indices into Waypoint_lists
	SCP_string _currentName;
	bool _bypass_errors = false;
	bool _noDrawLines = false;
	bool _hasCustomColor = false;
	int _colorR = 255, _colorG = 255, _colorB = 255;
};

} // namespace fso::fred::dialogs
