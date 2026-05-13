#pragma once
#include "mission/dialogs/AbstractDialogModel.h"

#include "coordinate_points/coordinate_point.h"

namespace fso::fred::dialogs {

class CoordinatePointEditorDialogModel : public AbstractDialogModel {
	Q_OBJECT

public:
	CoordinatePointEditorDialogModel(QObject* parent, EditorViewport* viewport);

	bool apply() override;
	void reject() override;

	bool hasValidSelection() const;
	bool hasMultipleSelection() const;
	static bool hasAnyCoordinatePointsInMission();
	int getSelectionCount() const;

	// Name (disabled in multi-select). Returns false on validation failure.
	const SCP_string& getCurrentName() const;
	bool setCurrentName(const SCP_string& name);

	// Category (free-form string).
	const SCP_string& getCategory() const;
	bool isCategoryMixed() const;
	void setCategory(const SCP_string& category);

	// Color, per-channel.
	int getColorR() const;
	int getColorG() const;
	int getColorB() const;
	int getColorA() const;
	void setColorR(int r);
	void setColorG(int g);
	void setColorB(int b);
	void setColorA(int a);
	bool isColorRMixed() const;
	bool isColorGMixed() const;
	bool isColorBMixed() const;
	bool isColorAMixed() const;
	bool hasAnyColorMixed() const;

	// Shape.
	CoordinatePointShape getShape() const;
	bool isShapeMixed() const;
	void setShape(CoordinatePointShape shape);

	// Size scale (clamped to [SIZE_MIN, SIZE_MAX] on set).
	float getSize() const;
	bool  isSizeMixed() const;
	void  setSize(float v);

	// Escort priority (0 = not on list; clamped to >= 0 on set).
	int getEscortPriority() const;
	bool isEscortPriorityMixed() const;
	void setEscortPriority(int v);

	// Multiplayer team affiliation. -1 = visible to all teams (default).
	// 0..MAX_TVT_TEAMS-1 = visible only to that team in multiplayer.
	int getMultiTeam() const;
	bool isMultiTeamMixed() const;
	void setMultiTeam(int v);
	static bool missionIsMultiTeam();

	// Visible-in-mission flag (tristate when mixed).
	bool getVisibleInMission() const;
	int  getVisibleInMissionState() const; // Qt::CheckState as int
	void setVisibleInMission(bool v);

	// Layer assignment for the editor's view-layer system. Empty / "<mixed>" handling is by
	// the dialog; the model writes through to every selected point.
	SCP_string getLayer() const;
	void setLayer(const SCP_string& layer);

	void selectNextPoint();
	void selectPreviousPoint();

signals:
	void coordinatePointMarkingChanged();

private slots:
	void onSelectedObjectChanged(int);
	void onSelectedObjectMarkingChanged(int, bool);
	void onMissionChanged();

private:
	void initializeData();
	void showErrorDialogNoCancel(const SCP_string& message);
	bool validateName(const SCP_string& name);
	void selectCoordinatePointByObjnum(int objnum);
	mission_coordinate_point* getSelected(int objnum) const;

	SCP_vector<int> _selectedObjnums;

	SCP_string _currentName;
	SCP_string _category;
	int        _colorR = 255, _colorG = 255, _colorB = 255, _colorA = 255;
	CoordinatePointShape _shape = CoordinatePointShape::Diamond;
	float      _size = 1.0f;
	int        _escortPriority = 0;
	int        _multiTeam = -1;
	bool       _visibleInMission = false;

	bool _categoryMixed = false;
	bool _redMixed = false, _greenMixed = false, _blueMixed = false, _alphaMixed = false;
	bool _shapeMixed = false;
	bool _sizeMixed = false;
	bool _escortPriorityMixed = false;
	bool _multiTeamMixed = false;
	bool _visibleInMissionMixed = false;

	bool _bypass_errors = false;
	bool _suppressRefresh = false;
};

} // namespace fso::fred::dialogs
