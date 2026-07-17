#include "mission/dialogs/CoordinatePointEditorDialogModel.h"

#include <coordinate_points/coordinate_shapes.h>
#include <globalincs/globals.h>
#include <globalincs/linklist.h>
#include <iff_defs/iff_defs.h>
#include <jumpnode/jumpnode.h>
#include <math/floating.h>
#include <mission/missionparse.h>
#include <mission/object.h>
#include <object/object.h>
#include <object/waypoint.h>
#include <ship/ship.h>

namespace fso::fred::dialogs {

CoordinatePointEditorDialogModel::CoordinatePointEditorDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport)
{
	connect(viewport->editor, &Editor::currentObjectChanged, this, &CoordinatePointEditorDialogModel::onSelectedObjectChanged);
	connect(viewport->editor, &Editor::objectMarkingChanged, this, &CoordinatePointEditorDialogModel::onSelectedObjectMarkingChanged);
	connect(viewport->editor, &Editor::missionChanged, this, &CoordinatePointEditorDialogModel::onMissionChanged);

	initializeData();
}

bool CoordinatePointEditorDialogModel::apply() { return true; }
void CoordinatePointEditorDialogModel::reject() {}

mission_coordinate_point* CoordinatePointEditorDialogModel::getSelected(int objnum) const
{
	return find_coordinate_point_by_objnum(objnum);
}

void CoordinatePointEditorDialogModel::initializeData()
{
	_selectedObjnums.clear();
	_groupMixed = false;
	_redMixed = _greenMixed = _blueMixed = _alphaMixed = false;
	_shapeKindMixed = false;
	_sidesMixed = false;
	_pointsMixed = false;
	_innerRadiusMixed = false;
	_angleMixed = false;
	_sizeMixed = false;
	_escortPriorityMixed = false;
	_multiTeamMixed = false;
	_visibleInMissionMixed = false;

	// Collect every marked OBJ_COORDINATE_POINT object.
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_COORDINATE_POINT && ptr->flags[Object::Object_Flags::Marked]) {
			_selectedObjnums.push_back(OBJ_INDEX(ptr));
		}
	}

	if (!_selectedObjnums.empty()) {
		const mission_coordinate_point* first = getSelected(_selectedObjnums.front());
		if (first != nullptr) {
			_currentName      = first->name;
			_group         = first->group;
			_colorR           = first->display_color.red;
			_colorG           = first->display_color.green;
			_colorB           = first->display_color.blue;
			_colorA           = first->display_color.alpha;
			_shapeKind        = first->shape_kind;
			_shapeTableIndex  = first->shape_table_index;
			_sides            = first->shape_sides;
			_points           = first->shape_points;
			_innerRadius      = first->shape_inner_radius;
			_angle            = first->shape_angle_deg;
			_size             = first->size_scale;
			_escortPriority   = first->escort_priority;
			_multiTeam        = first->multi_team;
			_visibleInMission = first->flags[CoordinatePoint::Flags::Visible_in_mission];

			for (size_t i = 1; i < _selectedObjnums.size(); ++i) {
				const auto* other = getSelected(_selectedObjnums[i]);
				if (other == nullptr) continue;
				if (other->group          != _group)         _groupMixed = true;
				if (other->display_color.red   != _colorR)         _redMixed = true;
				if (other->display_color.green != _colorG)         _greenMixed = true;
				if (other->display_color.blue  != _colorB)         _blueMixed = true;
				if (other->display_color.alpha != _colorA)         _alphaMixed = true;
				// Shape kind is mixed if either the kind itself differs OR (for Tabled) the
				// resolved table index differs -- two points both Tabled but pointing at
				// different shapes are still "mixed."
				if (other->shape_kind        != _shapeKind)        _shapeKindMixed = true;
				else if (_shapeKind == CoordinatePointShapeKind::Tabled &&
						 other->shape_table_index != _shapeTableIndex)
					_shapeKindMixed = true;
				if (other->shape_sides       != _sides)            _sidesMixed = true;
				if (other->shape_points      != _points)           _pointsMixed = true;
				if (other->shape_inner_radius != _innerRadius)     _innerRadiusMixed = true;
				if (other->shape_angle_deg   != _angle)            _angleMixed = true;
				if (other->size_scale        != _size)             _sizeMixed = true;
				if (other->escort_priority   != _escortPriority)   _escortPriorityMixed = true;
				if (other->multi_team        != _multiTeam)        _multiTeamMixed = true;
				if (other->flags[CoordinatePoint::Flags::Visible_in_mission] != _visibleInMission)
					_visibleInMissionMixed = true;
			}
		}
	} else {
		_currentName.clear();
		_group.clear();
		_colorR = _colorG = _colorB = _colorA = 255;
		_shapeKind = CoordinatePointShapeKind::NGon;
		_shapeTableIndex = -1;
		_sides = 4;
		_points = 5;
		_innerRadius = STAR_INNER_DEFAULT;
		_angle = 0.0f;
		_size = 1.0f;
		_escortPriority = 0;
		_multiTeam = -1;
		_visibleInMission = false;
	}

	Q_EMIT coordinatePointMarkingChanged();
	_modified = false;
}

bool CoordinatePointEditorDialogModel::hasValidSelection() const { return !_selectedObjnums.empty(); }
bool CoordinatePointEditorDialogModel::hasMultipleSelection() const { return _selectedObjnums.size() > 1; }
bool CoordinatePointEditorDialogModel::hasAnyCoordinatePointsInMission() { return !Coordinate_points.empty(); }
int  CoordinatePointEditorDialogModel::getSelectionCount() const { return static_cast<int>(_selectedObjnums.size()); }

void CoordinatePointEditorDialogModel::showErrorDialogNoCancel(const SCP_string& message)
{
	if (_bypass_errors) return;
	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", message, {DialogButton::Ok});
}

bool CoordinatePointEditorDialogModel::validateName(const SCP_string& name)
{
	if (name.empty()) {
		showErrorDialogNoCancel("Coordinate point name cannot be empty.");
		return false;
	}

	// Collision with another coordinate point (excluding the one we're editing).
	const auto* current = getSelected(_selectedObjnums.front());
	for (const auto& other : Coordinate_points) {
		if (&other == current) continue;
		if (!stricmp(other.name.c_str(), name.c_str())) {
			showErrorDialogNoCancel("This name is already being used by another coordinate point.");
			return false;
		}
	}

	// Collision with a ship.
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			if (!stricmp(name.c_str(), Ships[ptr->instance].ship_name)) {
				showErrorDialogNoCancel("This name is already being used by a ship.");
				return false;
			}
		}
	}

	// Collision with a wing.
	for (auto& wing : Wings) {
		if (!stricmp(wing.name, name.c_str())) {
			showErrorDialogNoCancel("This name is already being used by a wing.");
			return false;
		}
	}

	// Collision with a waypoint list.
	for (const auto& wl : Waypoint_lists) {
		if (!stricmp(wl.get_name(), name.c_str())) {
			showErrorDialogNoCancel("This name is already being used by a waypoint path.");
			return false;
		}
	}

	// Collision with a jump node.
	if (jumpnode_get_by_name(name.c_str()) != nullptr) {
		showErrorDialogNoCancel("This name is already being used by a jump node.");
		return false;
	}

	if (name[0] == '<') {
		showErrorDialogNoCancel("Coordinate point names cannot begin with '<'.");
		return false;
	}

	return true;
}

const SCP_string& CoordinatePointEditorDialogModel::getCurrentName() const { return _currentName; }

bool CoordinatePointEditorDialogModel::setCurrentName(const SCP_string& name)
{
	if (hasMultipleSelection() || _selectedObjnums.empty()) {
		return false;
	}

	_bypass_errors = false;
	SCP_string trimmed = name;
	SCP_trim(trimmed);

	if (!validateName(trimmed)) {
		return false;
	}

	auto* cp = getSelected(_selectedObjnums.front());
	if (cp == nullptr) return false;

	cp->name = trimmed;
	_currentName = trimmed;
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
	return true;
}

const SCP_string& CoordinatePointEditorDialogModel::getGroup() const { return _group; }
bool CoordinatePointEditorDialogModel::isGroupMixed() const { return _groupMixed; }

void CoordinatePointEditorDialogModel::setGroup(const SCP_string& group)
{
	_group = group;
	_groupMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->group = group;
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

int CoordinatePointEditorDialogModel::getColorR() const { return _colorR; }
int CoordinatePointEditorDialogModel::getColorG() const { return _colorG; }
int CoordinatePointEditorDialogModel::getColorB() const { return _colorB; }
int CoordinatePointEditorDialogModel::getColorA() const { return _colorA; }
bool CoordinatePointEditorDialogModel::isColorRMixed() const { return _redMixed; }
bool CoordinatePointEditorDialogModel::isColorGMixed() const { return _greenMixed; }
bool CoordinatePointEditorDialogModel::isColorBMixed() const { return _blueMixed; }
bool CoordinatePointEditorDialogModel::isColorAMixed() const { return _alphaMixed; }
bool CoordinatePointEditorDialogModel::hasAnyColorMixed() const {
	return _redMixed || _greenMixed || _blueMixed || _alphaMixed;
}

namespace {
void applyColorChannel(SCP_vector<int>& selected,
	int r, int g, int b, int a,
	bool rMixed, bool gMixed, bool bMixed, bool aMixed)
{
	for (int objnum : selected) {
		auto* cp = find_coordinate_point_by_objnum(objnum);
		if (cp == nullptr) continue;
		int outR = rMixed ? cp->display_color.red   : r;
		int outG = gMixed ? cp->display_color.green : g;
		int outB = bMixed ? cp->display_color.blue  : b;
		int outA = aMixed ? cp->display_color.alpha : a;
		gr_init_alphacolor(&cp->display_color, outR, outG, outB, outA);
	}
}
}

void CoordinatePointEditorDialogModel::setColorR(int r)
{
	// -1 = spinbox "mixed" sentinel; leave _redMixed untouched.
	if (r < 0) return;
	CLAMP(r, 0, 255);
	_colorR = r;
	_redMixed = false;
	applyColorChannel(_selectedObjnums, _colorR, _colorG, _colorB, _colorA,
		_redMixed, _greenMixed, _blueMixed, _alphaMixed);
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

void CoordinatePointEditorDialogModel::setColorG(int g)
{
	if (g < 0) return;
	CLAMP(g, 0, 255);
	_colorG = g;
	_greenMixed = false;
	applyColorChannel(_selectedObjnums, _colorR, _colorG, _colorB, _colorA,
		_redMixed, _greenMixed, _blueMixed, _alphaMixed);
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

void CoordinatePointEditorDialogModel::setColorB(int b)
{
	if (b < 0) return;
	CLAMP(b, 0, 255);
	_colorB = b;
	_blueMixed = false;
	applyColorChannel(_selectedObjnums, _colorR, _colorG, _colorB, _colorA,
		_redMixed, _greenMixed, _blueMixed, _alphaMixed);
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

void CoordinatePointEditorDialogModel::setColorA(int a)
{
	if (a < 0) return;
	CLAMP(a, 0, 255);
	_colorA = a;
	_alphaMixed = false;
	applyColorChannel(_selectedObjnums, _colorR, _colorG, _colorB, _colorA,
		_redMixed, _greenMixed, _blueMixed, _alphaMixed);
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

CoordinatePointShapeKind CoordinatePointEditorDialogModel::getShapeKind() const { return _shapeKind; }
int CoordinatePointEditorDialogModel::getShapeTableIndex() const { return _shapeTableIndex; }
bool CoordinatePointEditorDialogModel::isShapeKindMixed() const { return _shapeKindMixed; }

int CoordinatePointEditorDialogModel::getShapeId() const
{
	// Combo encoding: NGon=-2, Star=-1, Tabled=tableIndex.
	switch (_shapeKind) {
		case CoordinatePointShapeKind::NGon:   return -2;
		case CoordinatePointShapeKind::Star:   return -1;
		case CoordinatePointShapeKind::Tabled: return _shapeTableIndex;
	}
	return -2;
}

void CoordinatePointEditorDialogModel::setShapeId(int shape_id)
{
	CoordinatePointShapeKind kind;
	int tableIdx = -1;
	if (shape_id == -2) {
		kind = CoordinatePointShapeKind::NGon;
	} else if (shape_id == -1) {
		kind = CoordinatePointShapeKind::Star;
	} else if (shape_id >= 0 && shape_id < static_cast<int>(Coordinate_shapes.size())) {
		kind = CoordinatePointShapeKind::Tabled;
		tableIdx = shape_id;
	} else {
		return;  // unknown id (mixed-state sentinel or stale)
	}

	_shapeKind       = kind;
	_shapeTableIndex = tableIdx;
	_shapeKindMixed  = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp == nullptr) continue;
		cp->shape_kind = kind;
		// Only overwrite shape_table_index for Tabled. For NGon/Star leave any prior index
		// alone (defensively; it shouldn't be used anyway when kind != Tabled).
		if (kind == CoordinatePointShapeKind::Tabled) {
			cp->shape_table_index = tableIdx;
		}
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

int  CoordinatePointEditorDialogModel::getSides() const { return _sides; }
bool CoordinatePointEditorDialogModel::isSidesMixed() const { return _sidesMixed; }

void CoordinatePointEditorDialogModel::setSides(int v)
{
	// Below NGON_SIDES_MIN means "mixed" sentinel from the spinbox; ignore.
	if (v < NGON_SIDES_MIN) return;
	if (v > NGON_SIDES_MAX) v = NGON_SIDES_MAX;
	_sides = v;
	_sidesMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->shape_sides = v;
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

int  CoordinatePointEditorDialogModel::getPoints() const { return _points; }
bool CoordinatePointEditorDialogModel::isPointsMixed() const { return _pointsMixed; }

void CoordinatePointEditorDialogModel::setPoints(int v)
{
	if (v < STAR_POINTS_MIN) return;
	if (v > STAR_POINTS_MAX) v = STAR_POINTS_MAX;
	_points = v;
	_pointsMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->shape_points = v;
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

float CoordinatePointEditorDialogModel::getInnerRadius() const { return _innerRadius; }
bool  CoordinatePointEditorDialogModel::isInnerRadiusMixed() const { return _innerRadiusMixed; }

void CoordinatePointEditorDialogModel::setInnerRadius(float v)
{
	// Negative = "mixed" sentinel.
	if (v < 0.0f) return;
	CLAMP(v, STAR_INNER_MIN, STAR_INNER_MAX);
	_innerRadius = v;
	_innerRadiusMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->shape_inner_radius = v;
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

float CoordinatePointEditorDialogModel::getAngle() const { return _angle; }
bool  CoordinatePointEditorDialogModel::isAngleMixed() const { return _angleMixed; }

void CoordinatePointEditorDialogModel::setAngle(float v)
{
	// The dialog uses a value one below the real -360 floor as the "mixed" blank sentinel; ignore
	// it so a user landing on the blank value doesn't commit an out-of-range angle.
	if (v < -360.0f) return;
	_angle = v;
	_angleMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->shape_angle_deg = v;
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

float CoordinatePointEditorDialogModel::getSize() const { return _size; }
bool CoordinatePointEditorDialogModel::isSizeMixed() const { return _sizeMixed; }

void CoordinatePointEditorDialogModel::setSize(float v)
{
	// Negative is the "mixed" sentinel for the dialog's double spinbox.
	if (v < 0.0f) return;
	CLAMP(v, COORDINATE_POINT_SIZE_MIN, COORDINATE_POINT_SIZE_MAX);
	_size = v;
	_sizeMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->size_scale = v;
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

int CoordinatePointEditorDialogModel::getEscortPriority() const { return _escortPriority; }
bool CoordinatePointEditorDialogModel::isEscortPriorityMixed() const { return _escortPriorityMixed; }

void CoordinatePointEditorDialogModel::setEscortPriority(int v)
{
	// -1 = "mixed" sentinel.
	if (v < 0) return;
	_escortPriority = v;
	_escortPriorityMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->escort_priority = v;
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

int CoordinatePointEditorDialogModel::getMultiTeam() const { return _multiTeam; }
bool CoordinatePointEditorDialogModel::isMultiTeamMixed() const { return _multiTeamMixed; }

bool CoordinatePointEditorDialogModel::missionIsMultiTeam()
{
	return (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) != 0;
}

SCP_string CoordinatePointEditorDialogModel::getLayer() const
{
	// Returns empty string when selected points span different layers; the dialog's
	// findData("") then yields -1 and the combo renders blank.
	SCP_string result;
	bool first = true;
	for (int objnum : _selectedObjnums) {
		SCP_string layer = _viewport->getObjectLayerName(objnum);
		if (first) {
			result = layer;
			first = false;
		} else if (result != layer) {
			return "";
		}
	}
	return result;
}

void CoordinatePointEditorDialogModel::setLayer(const SCP_string& layer)
{
	// moveObjectToLayer may unmark objects (when moving to a hidden layer) and fires
	// notifyLayerStructureChanged; both can reach back into our refresh slots. Suppress
	// re-entry during the batch, then run a single initializeData() at the end.
	_suppressRefresh = true;
	for (int objnum : _selectedObjnums) {
		_viewport->moveObjectToLayer(objnum, layer);
	}
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
	initializeData();
}

void CoordinatePointEditorDialogModel::setMultiTeam(int v)
{
	// Sentinel: a value below -1 is used by the dialog to mean "mixed; leave untouched".
	if (v < -1) return;
	if (v >= MAX_TVT_TEAMS) v = -1;
	_multiTeam = v;
	_multiTeamMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->multi_team = v;
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

bool CoordinatePointEditorDialogModel::getVisibleInMission() const { return _visibleInMission; }

int CoordinatePointEditorDialogModel::getVisibleInMissionState() const
{
	if (_visibleInMissionMixed) return Qt::PartiallyChecked;
	return _visibleInMission ? Qt::Checked : Qt::Unchecked;
}

void CoordinatePointEditorDialogModel::setVisibleInMission(bool v)
{
	_visibleInMission = v;
	_visibleInMissionMixed = false;
	for (int objnum : _selectedObjnums) {
		auto* cp = getSelected(objnum);
		if (cp != nullptr) cp->flags.set(CoordinatePoint::Flags::Visible_in_mission, v);
	}
	_suppressRefresh = true;
	set_modified();
	_editor->missionChanged();
	_suppressRefresh = false;
}

void CoordinatePointEditorDialogModel::selectCoordinatePointByObjnum(int objnum)
{
	if (objnum < 0) return;

	_suppressRefresh = true;
	_editor->unmark_all();
	_editor->markObject(objnum);
	_suppressRefresh = false;
	initializeData();
}

void CoordinatePointEditorDialogModel::selectNextPoint()
{
	if (Coordinate_points.empty()) return;

	// Walk the list and find the entry after the currently-selected one.
	int current = _selectedObjnums.empty() ? -1 : _selectedObjnums.front();
	int first_objnum = -1;
	bool found_current = false;
	for (const auto& cp : Coordinate_points) {
		if (cp.objnum < 0) continue;
		if (first_objnum < 0) first_objnum = cp.objnum;
		if (found_current) {
			selectCoordinatePointByObjnum(cp.objnum);
			return;
		}
		if (cp.objnum == current) {
			found_current = true;
		}
	}
	// Either no current selection or current was the last one: wrap to first.
	if (first_objnum >= 0) {
		selectCoordinatePointByObjnum(first_objnum);
	}
}

void CoordinatePointEditorDialogModel::selectPreviousPoint()
{
	if (Coordinate_points.empty()) return;

	int current = _selectedObjnums.empty() ? -1 : _selectedObjnums.front();
	int prev_objnum = -1;
	int last_objnum = -1;
	for (const auto& cp : Coordinate_points) {
		if (cp.objnum < 0) continue;
		if (cp.objnum == current) {
			if (prev_objnum >= 0) {
				selectCoordinatePointByObjnum(prev_objnum);
				return;
			}
			// Current is the first entry: wrap to last (computed after loop).
			break;
		}
		prev_objnum = cp.objnum;
		last_objnum = cp.objnum;
	}
	// Walk the rest to find the last live entry for wrap.
	for (const auto& cp : Coordinate_points) {
		if (cp.objnum >= 0) last_objnum = cp.objnum;
	}
	if (last_objnum >= 0) {
		selectCoordinatePointByObjnum(last_objnum);
	}
}

void CoordinatePointEditorDialogModel::onSelectedObjectChanged(int)
{
	if (_suppressRefresh) return;
	initializeData();
}

void CoordinatePointEditorDialogModel::onSelectedObjectMarkingChanged(int, bool)
{
	if (_suppressRefresh) return;
	initializeData();
}

void CoordinatePointEditorDialogModel::onMissionChanged()
{
	if (_suppressRefresh) return;
	initializeData();
}

} // namespace fso::fred::dialogs
