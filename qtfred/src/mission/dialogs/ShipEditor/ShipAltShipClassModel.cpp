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
	for (auto& pool_class : alt_class_pool) {
		if (pool_class.ship_class == -1 && pool_class.variable_index == -1) {
			_viewport->dialogProvider->showButtonDialog(DialogType::Warning,
				"Warning",
				"Class Can\'t be set by both ship class and by variable simultaneously.",
				{DialogButton::Ok});
			return false;
		}
	}
	for (int i = 0; i < _num_selected_ships; i++) {
		Ships[_m_selected_ships[i]].s_alt_classes = alt_class_pool;
	}
	return true;
}

void ShipAltShipClassModel::reject() {}

SCP_vector<alt_class> ShipAltShipClassModel::get_pool() const
{
	return alt_class_pool;
}

SCP_vector<std::pair<SCP_string, int>> ShipAltShipClassModel::get_classes()
{
	// Fill the ship classes combo box
	SCP_vector<std::pair<SCP_string, int>> _m_set_from_ship_class;
	std::pair<SCP_string, int> classData;
	// Add the default entry if we need one followed by all the ship classes
		classData.first = "Set From Variable";
		classData.second = -1;
		_m_set_from_ship_class.push_back(classData);
	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
			if (!(it->flags[Ship::Info_Flags::Player_ship])) {
			continue;
		}
		classData.first = it->name;
		classData.second = std::distance(Ship_info.cbegin(), it);
		_m_set_from_ship_class.push_back(classData);
	}

	return _m_set_from_ship_class;
}

SCP_vector<std::pair<SCP_string, int>> ShipAltShipClassModel::get_variables()
{
	// Fill the variable combo box
	SCP_vector<std::pair<SCP_string, int>> _m_set_from_variables;
	std::pair<SCP_string, int> variableData;
	variableData.first = "Set From Ship Class";
	variableData.second = -1;
	_m_set_from_variables.push_back(variableData);
	for (int i = 0; i < MAX_SEXP_VARIABLES; i++) {
		if (Sexp_variables[i].type & SEXP_VARIABLE_STRING) {
			std::ostringstream oss;
			SCP_string buff = Sexp_variables[i].variable_name;
			oss << buff << "[" << Sexp_variables[i].text << "]";
			buff = oss.str();
			variableData.first = buff;
			variableData.second = i;
			_m_set_from_variables.push_back(variableData);
			//_string_variables.push_back(variable);
			//			_string_variables[0].get().type = 1234;
		}
	}
	return _m_set_from_variables;
}
void ShipAltShipClassModel::sync_data(const SCP_vector<alt_class>& new_pool) {
	if (new_pool == alt_class_pool) {
		return;
	} else {
		alt_class_pool = new_pool;
		set_modified();
	}
}
void ShipAltShipClassModel::initializeData()
{
	_num_selected_ships = 0;
	_m_selected_ships.clear();
	// have we got multiple selected ships?
	object* objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				_m_selected_ships.push_back(objp->instance);
				_num_selected_ships++;
			}
		}
		objp = GET_NEXT(objp);
	}

	Assertion(_num_selected_ships > 0, "No Ships Selected");
	// Assert(Objects[cur_object_index].flags[Object::Object_Flags::Marked]);

	alt_class_pool.clear();
	objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if ((objp->type == OBJ_START) || (objp->type == OBJ_SHIP)) {
			if (objp->flags[Object::Object_Flags::Marked]) {
				alt_class_pool = Ships[objp->instance].s_alt_classes;
				break;
			}
		}
		objp = GET_NEXT(objp);
	}
}

} // namespace fso::fred::dialogs