#include "WaypointPathGeneratorDialogModel.h"

#include <globalincs/linklist.h>
#include <globalincs/pstypes.h>
#include <iff_defs/iff_defs.h>
#include <jumpnode/jumpnode.h>
#include <math/floating.h>
#include <mission/object.h>
#include <object/waypoint.h>
#include <ship/ship.h>

namespace fso::fred::dialogs {

WaypointPathGeneratorDialogModel::WaypointPathGeneratorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	// Generate a default unique path name
	char name_buf[NAME_LENGTH];
	waypoint_find_unique_name(name_buf, 1);
	_pathName = name_buf;

	// Populate scene objects list
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		int objnum = OBJ_INDEX(ptr);
		if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
			_sceneObjects.emplace_back(Ships[ptr->instance].ship_name, objnum);
		} else if (ptr->type == OBJ_WAYPOINT) {
			SCP_string text;
			waypoint_stuff_name(text, ptr->instance);
			_sceneObjects.emplace_back(text, objnum);
		} else if (ptr->type == OBJ_JUMP_NODE) {
			for (auto& jn : Jump_nodes) {
				if (jn.GetSCPObjectNumber() == objnum) {
					_sceneObjects.emplace_back(jn.GetName(), objnum);
					break;
				}
			}
		}
	}
}

bool WaypointPathGeneratorDialogModel::apply()
{
	_bypass_errors = false;

	if (!validateData()) {
		return false;
	}

	// Determine center position
	vec3d center;
	if (_useObjectCenter && _centerObjectObjnum >= 0 && query_valid_object(_centerObjectObjnum)) {
		center = Objects[_centerObjectObjnum].pos;
	} else {
		center = { { { _centerX, _centerY, _centerZ } } };
	}

	int totalPoints = _numPoints * _loops;
	if (totalPoints <= 0) {
		showErrorDialog("Total waypoints value is invalid.");
		return false;
	}

	// Generate positions
	SCP_vector<vec3d> positions;
	positions.reserve(totalPoints);

	for (int i = 0; i < totalPoints; ++i) {
		// _numPoints >= 3 is guaranteed by validateData(), so no division by zero
		float angle = (2.0f * PI * i) / static_cast<float>(_numPoints);

		float driftOffset = 0.0f;
		if (totalPoints > 1) {
			driftOffset = _drift * (i / static_cast<float>(totalPoints - 1)) - _drift * 0.5f;
		}

		auto var = [](float v) -> float {
			return (v != 0.0f) ? frand_range(-v, v) : 0.0f;
		};

		vec3d pos;
		switch (_axis) {
		case GeneratorAxis::XZ:
			pos.xyz.x = center.xyz.x + _radius * cosf(angle) + var(_varianceX);
			pos.xyz.y = center.xyz.y + driftOffset + var(_varianceY);
			pos.xyz.z = center.xyz.z + _radius * sinf(angle) + var(_varianceZ);
			break;
		case GeneratorAxis::XY:
			pos.xyz.x = center.xyz.x + _radius * cosf(angle) + var(_varianceX);
			pos.xyz.y = center.xyz.y + _radius * sinf(angle) + var(_varianceY);
			pos.xyz.z = center.xyz.z + driftOffset + var(_varianceZ);
			break;
		case GeneratorAxis::ZY:
			pos.xyz.x = center.xyz.x + driftOffset + var(_varianceX);
			pos.xyz.y = center.xyz.y + _radius * sinf(angle) + var(_varianceY);
			pos.xyz.z = center.xyz.z + _radius * cosf(angle) + var(_varianceZ);
			break;
		}

		positions.push_back(pos);
	}

	// Create the first waypoint, this creates a new list with an auto-generated name.
	// Capture the index before the call since waypoint_add always appends.
	int listIndex = static_cast<int>(Waypoint_lists.size());
	waypoint_add(positions.data(), -1);
	Assertion(listIndex < static_cast<int>(Waypoint_lists.size()), "waypoint_add() failed to append a new waypoint list!");

	// Rename the list to the user specified name
	Waypoint_lists[listIndex].set_name(_pathName.c_str());

	// Add remaining waypoints to the same list
	for (int i = 1; i < static_cast<int>(positions.size()); ++i) {
		waypoint_add(positions.data() + i, calc_waypoint_instance(listIndex, i - 1));
	}

	// Select all waypoints in the new path
	_editor->unmark_all();
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_WAYPOINT && calc_waypoint_list_index(ptr->instance) == listIndex) {
			_editor->markObject(OBJ_INDEX(ptr));
		}
	}

	_editor->missionChanged();

	return true;
}

void WaypointPathGeneratorDialogModel::reject()
{
	// One shot generator, nothing to undo
}

bool WaypointPathGeneratorDialogModel::validateData()
{
	if (_pathName.empty()) {
		showErrorDialog("Please enter a name for the waypoint path.");
		return false;
	}

	if (_pathName[0] == '<') {
		showErrorDialog("Waypoint path names may not begin with '<'.");
		return false;
	}

	// Wing name collision
	for (auto& wing : Wings) {
		if (!stricmp(wing.name, _pathName.c_str())) {
			showErrorDialog("This name is already used by a wing.");
			return false;
		}
	}

	// Ship name collision
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
			if (!stricmp(_pathName.c_str(), Ships[ptr->instance].ship_name)) {
				showErrorDialog("This name is already used by a ship.");
				return false;
			}
		}
	}

	// Target priority group collision
	for (auto& ai : Ai_tp_list) {
		if (!stricmp(_pathName.c_str(), ai.name)) {
			showErrorDialog("This name is already used by a target priority group.");
			return false;
		}
	}

	// Waypoint path name collision
	for (const auto& wpl : Waypoint_lists) {
		if (!stricmp(wpl.get_name(), _pathName.c_str())) {
			showErrorDialog("This name is already used by another waypoint path.");
			return false;
		}
	}

	// Jump node name collision
	if (jumpnode_get_by_name(_pathName.c_str()) != nullptr) {
		showErrorDialog("This name is already used by a jump node.");
		return false;
	}

	if (_radius <= 0.0f) {
		showErrorDialog("Radius must be greater than zero.");
		return false;
	}

	if (_numPoints < 3) {
		showErrorDialog("Number of points must be at least 3.");
		return false;
	}

	if (_loops < 1) {
		showErrorDialog("Loops must be at least 1.");
		return false;
	}

	return true;
}

void WaypointPathGeneratorDialogModel::showErrorDialog(const SCP_string& message)
{
	if (_bypass_errors) {
		return;
	}
	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", message, {DialogButton::Ok});
	_bypass_errors = false;
}

// --- Getters/Setters ---

const SCP_string& WaypointPathGeneratorDialogModel::getPathName() const { return _pathName; }
void WaypointPathGeneratorDialogModel::setPathName(const SCP_string& v) { _pathName = v; }

bool WaypointPathGeneratorDialogModel::getUseObjectCenter() const { return _useObjectCenter; }
void WaypointPathGeneratorDialogModel::setUseObjectCenter(bool v) { _useObjectCenter = v; }

int WaypointPathGeneratorDialogModel::getCenterObjectObjnum() const { return _centerObjectObjnum; }
void WaypointPathGeneratorDialogModel::setCenterObjectObjnum(int objnum) { _centerObjectObjnum = objnum; }

float WaypointPathGeneratorDialogModel::getCenterX() const { return _centerX; }
void WaypointPathGeneratorDialogModel::setCenterX(float v) { _centerX = v; }
float WaypointPathGeneratorDialogModel::getCenterY() const { return _centerY; }
void WaypointPathGeneratorDialogModel::setCenterY(float v) { _centerY = v; }
float WaypointPathGeneratorDialogModel::getCenterZ() const { return _centerZ; }
void WaypointPathGeneratorDialogModel::setCenterZ(float v) { _centerZ = v; }

GeneratorAxis WaypointPathGeneratorDialogModel::getAxis() const { return _axis; }
void WaypointPathGeneratorDialogModel::setAxis(GeneratorAxis v) { _axis = v; }

int WaypointPathGeneratorDialogModel::getNumPoints() const { return _numPoints; }
void WaypointPathGeneratorDialogModel::setNumPoints(int v) { _numPoints = v; }

int WaypointPathGeneratorDialogModel::getLoops() const { return _loops; }
void WaypointPathGeneratorDialogModel::setLoops(int v) { _loops = v; }

float WaypointPathGeneratorDialogModel::getRadius() const { return _radius; }
void WaypointPathGeneratorDialogModel::setRadius(float v) { _radius = v; }

float WaypointPathGeneratorDialogModel::getDrift() const { return _drift; }
void WaypointPathGeneratorDialogModel::setDrift(float v) { _drift = v; }

float WaypointPathGeneratorDialogModel::getVarianceX() const { return _varianceX; }
void WaypointPathGeneratorDialogModel::setVarianceX(float v) { _varianceX = v; }
float WaypointPathGeneratorDialogModel::getVarianceY() const { return _varianceY; }
void WaypointPathGeneratorDialogModel::setVarianceY(float v) { _varianceY = v; }
float WaypointPathGeneratorDialogModel::getVarianceZ() const { return _varianceZ; }
void WaypointPathGeneratorDialogModel::setVarianceZ(float v) { _varianceZ = v; }

const SCP_vector<std::pair<SCP_string, int>>& WaypointPathGeneratorDialogModel::getSceneObjects() const
{
	return _sceneObjects;
}

} // namespace fso::fred::dialogs
