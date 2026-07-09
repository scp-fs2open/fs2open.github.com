#include "ShipAltShipClassModel.h"

#include "ship/ship.h"

namespace fso::fred::dialogs {

ShipAltShipClassModel::ShipAltShipClassModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

bool ShipAltShipClassModel::apply()
{
	// TODO: Add extra validation here
	for (auto& pool_class : _altClassPool) {
		if (pool_class.ship_class == -1 && pool_class.variable_index == -1) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Warning,
				"Warning",
				"Class Can\'t be set by both ship class and by variable simultaneously.",
				{DialogButton::Ok});
			return false;
		}
	}
	for (int i = 0; i < _numSelectedShips; i++) {
		Ships[_selectedShips[i]].s_alt_classes = _altClassPool;
	}
	return true;
}

void ShipAltShipClassModel::reject() {}

SCP_vector<alt_class> ShipAltShipClassModel::getPool() const
{
	return _altClassPool;
}

SCP_vector<std::pair<SCP_string, int>> ShipAltShipClassModel::getClasses()
{
	// Fill the ship classes combo box
	SCP_vector<std::pair<SCP_string, int>> classPool;
	std::pair<SCP_string, int> classData;
	// Add the default entry if we need one followed by all the ship classes
	classData.first = "Set From Variable";
	classData.second = -1;
	classPool.push_back(classData);
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
		if (!(it->flags[Ship::Info_Flags::Player_ship])) {
			continue;
		}
		classData.first = it->name;
		classData.second = std::distance(Ship_info.cbegin(), it);
		classPool.push_back(classData);
	}

	return classPool;
}

SCP_vector<std::pair<SCP_string, int>> ShipAltShipClassModel::getVariables()
{
	// Fill the variable combo box
	SCP_vector<std::pair<SCP_string, int>> variablePool;
	std::pair<SCP_string, int> variableData;
	variableData.first = "Set From Ship Class";
	variableData.second = -1;
	variablePool.push_back(variableData);
	for (int i = 0; i < MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_STRING) {
			std::ostringstream oss;
			SCP_string buff = Sexp_variables[i].variable_name;
			oss << buff << "[" << Sexp_variables[i].text << "]";
			buff = oss.str();
			variableData.first = buff;
			variableData.second = i;
			variablePool.push_back(variableData);
		}
	}
	return variablePool;
}

void ShipAltShipClassModel::syncData(const SCP_vector<alt_class>& newPool)
{
	if (newPool == _altClassPool) {
		return;
	} else {
		_altClassPool = newPool;
		set_modified();
	}
}

void ShipAltShipClassModel::initializeData()
{
	_numSelectedShips = 0;
	_selectedShips.clear();
	// have we got multiple selected ships?
	object* objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				_selectedShips.push_back(objp->instance);
				_numSelectedShips++;
			}
		}
		objp = GET_NEXT(objp);
	}

	Assertion(_numSelectedShips > 0, "No Ships Selected");

	_altClassPool.clear();
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				_altClassPool = Ships[objp->instance].s_alt_classes;
				break;
			}
		}
		objp = GET_NEXT(objp);
	}
	_modified = false;
}

} // namespace fso::fred::dialogs
