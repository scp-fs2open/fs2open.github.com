#include "ShipAltShipClassModel.h"

#include "ship/ship.h"
namespace fso::fred::dialogs {
ShipAltShipClassModel::ShipAltShipClassModel(QObject* parent, EditorViewport* viewport, bool is_several_ships)
	: AbstractDialogModel(parent, viewport), _multi_edit(is_several_ships)
{
	initializeData();
}

bool ShipAltShipClassModel::apply()
{
	// TODO: Add extra validation here
	for (int i = 0; i < _num_selected_ships; i++) {
		Ships[_m_selected_ships[i]].s_alt_classes = alt_class_pool;
	}
	return true;
}

void ShipAltShipClassModel::reject() {}

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

	Assert(_num_selected_ships > 0);
	// Assert(Objects[cur_object_index].flags[Object::Object_Flags::Marked]);

	// Fill the variable combo box
	_m_set_from_variables.clear();
	_string_variables.clear();
	_m_set_from_variables.push_back("Set From Ship Class");
	for (auto& variable : Sexp_variables) {
		if (variable.type & SEXP_VARIABLE_STRING) {
			SCP_string buff = variable.variable_name;
			buff = buff + "[" + variable.text + "]";
			_m_set_from_variables.push_back(buff);
			_string_variables.push_back(variable);
			//			_string_variables[0].get().type = 1234;
		}
	}

	/* int index = std::distance(std::begin(Sexp_variables),
		std::find_if(std::begin(Sexp_variables), std::end(Sexp_variables), [this](sexp_variable value) {
			return value.variable_name == _string_variables[0].get().variable_name;
		}));*/

	// Fill the ship classes combo box
	_m_set_from_ship_class.clear();
	// Add the default entry if we need one followed by all the ship classes
	if (_m_set_from_variables.size() > 0) {
		_m_set_from_ship_class.push_back("Set From Variable");
	}

	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
		if (_player_flyable_ships_only && !(it->flags[Ship::Info_Flags::Player_ship])) {
			continue;
		}

		_ship_class_indices.push_back(std::distance(Ship_info.cbegin(), it));
		_m_set_from_ship_class.push_back(it->name);
	}
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