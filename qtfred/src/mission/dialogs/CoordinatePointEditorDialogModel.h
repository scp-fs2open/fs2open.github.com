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

	// Group (free-form string).
	const SCP_string& getGroup() const;
	bool isGroupMixed() const;
	void setGroup(const SCP_string& group);

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

	// Shape kind + per-kind parameters. Mixed-state is tracked separately per field. Setters
	// write through to every selected point's matching field; per-kind parameters are kept on
	// every point regardless of its current kind, so toggling kinds doesn't destroy the data.
	CoordinatePointShapeKind getShapeKind() const;
	int  getShapeTableIndex() const;  // valid only when kind == Tabled
	bool isShapeKindMixed() const;
	// shape_id is encoded for the combo: NGon = -2, Star = -1, Tabled = table index (>= 0).
	int  getShapeId() const;
	void setShapeId(int shape_id);

	int  getSides() const;
	bool isSidesMixed() const;
	void setSides(int v);

	int  getPoints() const;
	bool isPointsMixed() const;
	void setPoints(int v);

	float getInnerRadius() const;
	bool  isInnerRadiusMixed() const;
	void  setInnerRadius(float v);

	float getAngle() const;
	bool  isAngleMixed() const;
	void  setAngle(float v);

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
	SCP_string _group;
	int        _colorR = 255, _colorG = 255, _colorB = 255, _colorA = 255;

	CoordinatePointShapeKind _shapeKind = CoordinatePointShapeKind::NGon;
	int   _shapeTableIndex = -1;
	int   _sides         = 4;
	int   _points        = 5;
	float _innerRadius   = STAR_INNER_DEFAULT;
	float _angle         = 0.0f;

	float      _size = 1.0f;
	int        _escortPriority = 0;
	int        _multiTeam = -1;
	bool       _visibleInMission = false;

	bool _groupMixed = false;
	bool _redMixed = false, _greenMixed = false, _blueMixed = false, _alphaMixed = false;
	bool _shapeKindMixed = false;
	bool _sidesMixed = false;
	bool _pointsMixed = false;
	bool _innerRadiusMixed = false;
	bool _angleMixed = false;
	bool _sizeMixed = false;
	bool _escortPriorityMixed = false;
	bool _multiTeamMixed = false;
	bool _visibleInMissionMixed = false;

	bool _bypass_errors = false;
	bool _suppressRefresh = false;
};

} // namespace fso::fred::dialogs
