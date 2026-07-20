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
	static bool hasAnyPathsInMission();
	int getSelectionCount() const;

	const SCP_string& getCurrentName() const;
	bool setCurrentName(const SCP_string& name);

	bool getNoDrawLines() const;
	int getNoDrawLinesState() const; // Qt::CheckState as int
	void setNoDrawLines(bool val);

	bool getHasCustomColor() const;
	int getHasCustomColorState() const; // Qt::CheckState as int
	void setHasCustomColor(bool val);

	int getColorR() const;
	int getColorG() const;
	int getColorB() const;
	void setColorR(int r);
	void setColorG(int g);
	void setColorB(int b);

	bool isColorRMixed() const;
	bool isColorGMixed() const;
	bool isColorBMixed() const;
	bool hasAnyColorMixed() const;

	SCP_string getLayer() const;
	void setLayer(const SCP_string& layer);

	void selectNextPath();
	void selectPreviousPath();
	void selectWaypointPathByIndex(int idx);
	int getSelectedPathIndex() const;

signals:
	void waypointPathMarkingChanged();

private slots:
	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void onMissionChanged();

private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();
	void showErrorDialogNoCancel(const SCP_string& message);
	bool validateName(const SCP_string& name);

	SCP_vector<int> _selectedWaypointPaths; // indices into Waypoint_lists
	SCP_string _currentName;
	bool _bypass_errors = false;
	bool _noDrawLines = false;
	bool _hasCustomColor = false;
	int _colorR = 255, _colorG = 255, _colorB = 255;

	bool _noDrawLinesMixed = false;
	bool _hasCustomColorMixed = false;
	bool _redMixed = false, _greenMixed = false, _blueMixed = false;

	// Guards against re-entry into initializeData() from selection/marking/mission signals
	// while we're already mutating mission state (e.g., setLayer fans out unmarks).
	bool _suppressRefresh = false;
};

} // namespace fso::fred::dialogs
