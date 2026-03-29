#pragma once
#include "mission/dialogs/AbstractDialogModel.h"

namespace fso::fred::dialogs {

class WaypointEditorDialogModel: public AbstractDialogModel {
 Q_OBJECT

 public:
	WaypointEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	const SCP_string& getCurrentName() const;
	void setCurrentName(const SCP_string& name);
	int getCurrentlySelectedPath() const;
	void setCurrentlySelectedPath(int elementId);

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

	bool isEnabled() const;
	const SCP_vector<std::pair<SCP_string, int>>& getWaypointPathList() const;

signals:
	void waypointPathMarkingChanged();
	

private slots:
	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void onMissionChanged();

 private: // NOLINT(readability-redundant-access-specifiers)
	void initializeData();
    void updateWaypointPathList();
	bool validateData();
	void showErrorDialogNoCancel(const SCP_string& message);

	SCP_string _currentName;
	int _currentWaypointPathSelected = -1;
	bool _enabled = false;
	SCP_vector<std::pair<SCP_string, int>> _waypointPathList;
	bool _bypass_errors = false;
	bool _noDrawLines = false;
	bool _hasCustomColor = false;
	int _colorR = 255, _colorG = 255, _colorB = 255;
};

} // namespace fso::fred::dialogs
