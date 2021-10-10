#include "ShipInitialStatusDialogModel.h"

#include "mission/object.h"
#include "ui/dialogs/ShipInitialStatusDialog.h"

#include <globalincs/linklist.h>

#include <QtWidgets>
#include <object/objectdock.cpp>

namespace fso {
namespace fred {
namespace dialogs {
void reset_arrival_to_false(int shipnum, bool reset_wing, EditorViewport* _viewport)
{
	char buf[256];
	ship* shipp = &Ships[shipnum];

	// falsify the ship cue
	if (set_cue_to_false(&shipp->arrival_cue)) {
		sprintf(buf, "Setting arrival cue of ship %s\nto false for initial docking purposes.", shipp->ship_name);
		// MessageBox(NULL, buf, "", MB_OK | MB_ICONEXCLAMATION);
		_viewport->dialogProvider->showButtonDialog(DialogType::Information, "Notice", buf, {DialogButton::Ok});
	}

	// falsify the wing cue and all ships in that wing
	if (reset_wing && shipp->wingnum >= 0) {
		int i;
		wing* wingp = &Wings[shipp->wingnum];

		if (set_cue_to_false(&wingp->arrival_cue)) {

			sprintf(buf, "Setting arrival cue of wing %s\nto false for initial docking purposes.", wingp->name);
			// MessageBox(NULL, buf, "", MB_OK | MB_ICONEXCLAMATION);
			_viewport->dialogProvider->showButtonDialog(DialogType::Information, "Notice", buf, {DialogButton::Ok});
		}

		for (i = 0; i < wingp->wave_count; i++)
			reset_arrival_to_false(wingp->ship_index[i], false, _viewport);
	}
}
bool set_cue_to_false(int* cue)
{
	// if the cue is not false, make it false.  Be sure to set all ship editor dialog functions
	// to update data before and after we modify the cue.
	// Comment Above Prbably not nesscary due to new model
	if (*cue != Locked_sexp_false) {

		free_sexp2(*cue);
		*cue = Locked_sexp_false;

		return true;
	} else
		return false;
}

// self-explanatory, really
void initial_status_unmark_dock_handled_flag(object* objp, dock_function_info* infop, EditorViewport* viewport)
{
	objp->flags.remove(Object::Object_Flags::Docked_already_handled);
}
void initial_status_mark_dock_leader_helper(object* objp, dock_function_info* infop, EditorViewport* viewport)
{
	ship* shipp = &Ships[objp->instance];
	int cue_to_check;

	// if this guy is part of a wing, he uses his wing's arrival cue
	if (shipp->wingnum >= 0) {
		cue_to_check = Wings[shipp->wingnum].arrival_cue;
	}
	// check the ship's arrival cue
	else {
		cue_to_check = shipp->arrival_cue;
	}

	// all ships except the leader should have a locked false arrival cue
	if (cue_to_check != Locked_sexp_false) {
		object* existing_leader;

		// increment number of leaders found
		infop->maintained_variables.int_value++;

		// see if we already found a leader
		existing_leader = infop->maintained_variables.objp_value;
		if (existing_leader != NULL) {
			ship* leader_shipp = &Ships[existing_leader->instance];

			// keep existing leader if he has a higher priority than us
			if (ship_class_compare(shipp->ship_info_index, leader_shipp->ship_info_index) >= 0) {
				// set my arrival cue to false
				reset_arrival_to_false(SHIP_INDEX(shipp), true, viewport);
				return;
			}

			// otherwise, unmark the existing leader and set his arrival cue to false
			leader_shipp->flags.remove(Ship::Ship_Flags::Dock_leader);
			reset_arrival_to_false(SHIP_INDEX(leader_shipp), true, viewport);
		}

		// mark and save me as the leader
		shipp->flags.set(Ship::Ship_Flags::Dock_leader);
		infop->maintained_variables.objp_value = objp;
	}
}
ShipInitialStatusDialogModel::ShipInitialStatusDialogModel(QObject* parent, EditorViewport* viewport, bool multi)
	: AbstractDialogModel(parent, viewport)
{
	m_multi_edit = multi;
	initializeData();
}

void ShipInitialStatusDialogModel::initializeData()
{
	int vflag, sflag, hflag;
	object* objp = nullptr;
	m_ship = _editor->cur_ship;
	if (m_ship == -1) {
		Assert(
			(Objects[_editor->currentObject].type == OBJ_SHIP) || (Objects[_editor->currentObject].type == OBJ_START));
		m_ship = get_ship_from_obj(_editor->currentObject);
		Assert(m_ship >= 0);
	}

	// initialize dockpoint stuff
	if (!m_multi_edit) {
		num_dock_points = model_get_num_dock_points(Ship_info[Ships[m_ship].ship_info_index].model_num);
		dockpoint_array = new dockpoint_information[num_dock_points];
		objp = &Objects[Ships[m_ship].objnum];
		for (int i = 0; i < num_dock_points; i++) {
			object* docked_objp = dock_find_object_at_dockpoint(objp, i);
			if (docked_objp != NULL) {
				dockpoint_array[i].dockee_shipnum = docked_objp->instance;
				dockpoint_array[i].dockee_point = dock_find_dockpoint_used_by_object(docked_objp, objp);
			} else {
				dockpoint_array[i].dockee_shipnum = -1;
				dockpoint_array[i].dockee_point = -1;
			}
		}
	}
	vflag = sflag = hflag = 0;
	m_velocity = (int)Objects[_editor->currentObject].phys_info.speed;
	m_shields = (int)Objects[_editor->currentObject].shield_quadrant[0];
	m_hull = (int)Objects[_editor->currentObject].hull_strength;

	if (Objects[_editor->currentObject].flags[Object::Object_Flags::No_shields])
		m_has_shields = 0;
	else
		m_has_shields = 1;

	if (Ships[m_ship].flags[Ship::Ship_Flags::Force_shields_on])
		m_force_shields = 1;
	else
		m_force_shields = 0;

	if (Ships[m_ship].flags[Ship::Ship_Flags::Ship_locked])
		m_ship_locked = 1;
	else
		m_ship_locked = 0;

	if (Ships[m_ship].flags[Ship::Ship_Flags::Weapons_locked])
		m_weapons_locked = 1;
	else
		m_weapons_locked = 0;
	// Lock primaries
	if (Ships[m_ship].flags[Ship::Ship_Flags::Primaries_locked]) {
		m_primaries_locked = 1;
	} else {
		m_primaries_locked = 0;
	}
	// Lock secondaries
	if (Ships[m_ship].flags[Ship::Ship_Flags::Secondaries_locked]) {
		m_secondaries_locked = 1;
	} else {
		m_secondaries_locked = 0;
	}

	// Lock turrets
	if (Ships[m_ship].flags[Ship::Ship_Flags::Lock_all_turrets_initially]) {
		m_turrets_locked = 1;
	} else {
		m_turrets_locked = 0;
	}

	if (Ships[m_ship].flags[Ship::Ship_Flags::Afterburner_locked]) {
		m_afterburner_locked = 1;
	} else {
		m_afterburner_locked = 0;
	}

	if (m_multi_edit) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				(objp->flags[Object::Object_Flags::Marked])) {
				if (objp->phys_info.speed != m_velocity)
					vflag = 1;
				if ((int)objp->shield_quadrant[0] != m_shields)
					sflag = 1;
				if ((int)objp->hull_strength != m_hull)
					hflag = 1;
				if (objp->flags[Object::Object_Flags::No_shields]) {
					if (m_has_shields)
						m_has_shields = 1;

				} else {
					if (!m_has_shields)
						m_has_shields = 1;
				}

				Assert((objp->type == OBJ_SHIP) || (objp->type == OBJ_START));

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Force_shields_on]) {
					if (!m_force_shields)
						m_force_shields = Qt::PartiallyChecked;

				} else {
					if (m_force_shields)
						m_force_shields = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Ship_locked]) {
					if (!m_ship_locked)
						m_ship_locked = Qt::PartiallyChecked;

				} else {
					if (m_ship_locked)
						m_ship_locked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Weapons_locked]) {
					if (!m_weapons_locked)
						m_weapons_locked = Qt::PartiallyChecked;

				} else {
					if (m_weapons_locked)
						m_weapons_locked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Primaries_locked]) {
					if (!m_primaries_locked)
						m_primaries_locked = Qt::PartiallyChecked;
				} else {
					if (m_primaries_locked)
						m_primaries_locked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Secondaries_locked]) {
					if (!m_secondaries_locked)
						m_secondaries_locked = Qt::PartiallyChecked;
				} else {
					if (m_secondaries_locked)
						m_secondaries_locked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Lock_all_turrets_initially]) {
					if (!m_turrets_locked)
						m_turrets_locked = Qt::PartiallyChecked;
				} else {
					if (m_turrets_locked)
						m_turrets_locked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Afterburner_locked]) {
					if (!m_afterburner_locked)
						m_afterburner_locked = Qt::PartiallyChecked;
				} else {
					if (m_afterburner_locked)
						m_afterburner_locked = Qt::PartiallyChecked;
				}
			}

			objp = GET_NEXT(objp);
		}
	}

	if (objp != NULL) {
		if (objp->type == OBJ_SHIP || objp->type == OBJ_START) {
			ship* shipp = &Ships[objp->instance];

			if (Ship_info[shipp->ship_info_index].uses_team_colors) {
				m_use_teams = true;
			}
		}
	}
	change_subsys(0);

	if (vflag) {
		m_velocity = BLANKFIELD;
	}
	if (vflag) {
		m_velocity = BLANKFIELD;
	}
	if (vflag) {
		m_velocity = BLANKFIELD;
	}
}

void ShipInitialStatusDialogModel::update_docking_info()
{
	int i;
	object* objp = &Objects[Ships[m_ship].objnum];
	for (i = 0; i < num_dock_points; i++) {
		// see if the object at this point is no longer there
		object* dockee_objp = dock_find_object_at_dockpoint(objp, i);
		if (dockee_objp != NULL) {
			// check if the dockee ship thinks that this ship is docked to this dock point
			if (objp != dock_find_object_at_dockpoint(dockee_objp, dockpoint_array[i].dockee_point)) {
				// undock it
				undock(objp, dockee_objp);
			}
		}
	}
	// add new info
	for (i = 0; i < num_dock_points; i++) {
		// see if there is an object at this point that wasn't there before
		if (dockpoint_array[i].dockee_shipnum >= 0) {
			if (dock_find_object_at_dockpoint(objp, i) == NULL) {
				object* dockee_objp = &Objects[Ships[dockpoint_array[i].dockee_shipnum].objnum];
				int dockee_point = dockpoint_array[i].dockee_point;

				// dock it
				dock(objp, i, dockee_objp, dockee_point);
			}
		}
	}

	_editor->missionChanged();
}
void ShipInitialStatusDialogModel::undock(object* objp1, object* objp2)
{
	vec3d v;
	int ship_num, other_ship_num;
	if (objp1 == nullptr || objp2 == nullptr)
		return;
	vm_vec_sub(&v, &objp2->pos, &objp1->pos);
	vm_vec_normalize(&v);
	ship_num = get_ship_from_obj(OBJ_INDEX(objp1));
	other_ship_num = get_ship_from_obj(OBJ_INDEX(objp2));

	if (_viewport->view.Move_ships_when_undocking) {
		if (ship_class_compare(Ships[ship_num].ship_info_index, Ships[other_ship_num].ship_info_index) <= 0)
			vm_vec_scale_add2(&objp2->pos,
				&v,
				ship_class_get_length(&Ship_info[Ships[objp2->instance].ship_info_index]));
		else
			vm_vec_scale_add2(&objp1->pos,
				&v,
				ship_class_get_length(&Ship_info[Ships[objp1->instance].ship_info_index]) * -1.0f);
	}

	ai_do_objects_undocked_stuff(objp1, objp2);

	// check to see if one of these ships has an arrival cue of false.  If so, then
	// reset it back to default value of true.  be sure to correctly update before
	// and after setting data.
	// Goober5000 - but don't reset it if it's part of a wing!

	if (Ships[ship_num].arrival_cue == Locked_sexp_false && Ships[ship_num].wingnum < 0) {
		Ships[ship_num].arrival_cue = Locked_sexp_true;
	} else if (Ships[other_ship_num].arrival_cue == Locked_sexp_false && Ships[other_ship_num].wingnum < 0) {
		Ships[other_ship_num].arrival_cue = Locked_sexp_true;
	}
	// if this ship is no longer docked, ensure its dock leader flag is clear
	if (!object_is_docked(&Objects[Ships[ship_num].objnum]))
		Ships[ship_num].flags.remove(Ship::Ship_Flags::Dock_leader);

	// same for the other ship
	if (!object_is_docked(&Objects[Ships[other_ship_num].objnum]))
		Ships[other_ship_num].flags.remove(Ship::Ship_Flags::Dock_leader);
}
void ShipInitialStatusDialogModel::dock(object* objp, int dockpoint, object* other_objp, int other_dockpoint)
{
	if (objp == nullptr || other_objp == nullptr) {
		return;
	}

	if (dockpoint < 0 || other_dockpoint < 0) {
		return;
	}

	dock_function_info dfi;

	// do the docking (do it in reverse so that the current object stays put)
	ai_dock_with_object(other_objp, other_dockpoint, objp, dockpoint, AIDO_DOCK_NOW);

	// unmark the handled flag in preparation for the next step
	dock_evaluate_all_docked_objects(objp, &dfi, initial_status_unmark_dock_handled_flag);

	// move all other objects to catch up with it
	dock_move_docked_objects(objp);

	// set the dock leader
	dock_evaluate_all_docked_objects(objp, &dfi, initial_status_mark_dock_leader_helper);

	// if no leader, mark me
	if (dfi.maintained_variables.int_value == 0)
		Ships[objp->instance].flags.set(Ship::Ship_Flags::Dock_leader);
}
// Duplicated from objectdock to handle scope errors
void ShipInitialStatusDialogModel::dock_evaluate_all_docked_objects(object* objp,
	dock_function_info* infop,
	void (*function)(object*, dock_function_info*, EditorViewport*))
{
	Assertion((objp != nullptr) && (infop != nullptr) && (function != nullptr),
		"dock_evaluate_all_docked_objects, invalid argument(s)");

	// not docked?
	if (!object_is_docked(objp)) {
		// call the function for just the one object
		function(objp, infop, _viewport);
		return;
	}

	// we only have two objects docked
	if (dock_check_docked_one_on_one(objp)) {
		// call the function for the first object, and return if instructed
		function(objp, infop, _viewport);
		if (infop->early_return_condition)
			return;

		// call the function for the second object, and return if instructed
		function(objp->dock_list->docked_objp, infop, _viewport);
		if (infop->early_return_condition)
			return;
	}

	// we have multiple objects docked and we're treating them as a hub
	else if (dock_check_assume_hub()) {
		// get the hub
		object* hub_objp = dock_get_hub(objp);

		// call the function for the hub, and return if instructed
		function(hub_objp, infop, _viewport);
		if (infop->early_return_condition)
			return;

		// iterate through all docked objects
		for (dock_instance* ptr = hub_objp->dock_list; ptr != NULL; ptr = ptr->next) {
			// call the function for this object, and return if instructed
			function(ptr->docked_objp, infop, _viewport);
			if (infop->early_return_condition)
				return;
		}
	}

	// we have multiple objects docked and we must treat them as a tree
	else {
		// create a bit array to mark the objects we check
		ubyte* visited_bitstring = (ubyte*)vm_malloc(calculate_num_bytes(MAX_OBJECTS));

		// clear it
		memset(visited_bitstring, 0, calculate_num_bytes(MAX_OBJECTS));

		// start evaluating the tree
		dock_evaluate_tree(objp, infop, function, visited_bitstring);

		// destroy the bit array
		vm_free(visited_bitstring);
		visited_bitstring = NULL;
	}
}

void ShipInitialStatusDialogModel::dock_evaluate_tree(object* objp,
	dock_function_info* infop,
	void (*function)(object*, dock_function_info*, EditorViewport*),
	ubyte* visited_bitstring)
{
	// make sure we haven't visited this object already
	if (get_bit(visited_bitstring, OBJ_INDEX(objp)))
		return;

	// mark as visited
	set_bit(visited_bitstring, OBJ_INDEX(objp));

	// call the function for this object, and return if instructed
	function(objp, infop, _viewport);
	if (infop->early_return_condition)
		return;

	// iterate through all docked objects
	for (dock_instance* ptr = objp->dock_list; ptr != NULL; ptr = ptr->next) {
		// start another tree with the docked object as the root, and return if instructed
		dock_evaluate_tree(ptr->docked_objp, infop, function, visited_bitstring);
		if (infop->early_return_condition)
			return;
	}
}

bool ShipInitialStatusDialogModel::apply()
{
	object* objp;

	change_subsys(0);
	if (m_multi_edit) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				(objp->flags[Object::Object_Flags::Marked])) {
				modify(objp->phys_info.speed, (float)m_velocity);

				modify(objp->shield_quadrant[0], (float)m_shields);

				modify(objp->hull_strength, (float)m_hull);

				if (m_has_shields == 1) {
					objp->flags.remove(Object::Object_Flags::No_shields);
				} else if (m_has_shields == 0) {
					objp->flags.set(Object::Object_Flags::No_shields);
				}
				auto shipp = &Ships[get_ship_from_obj(objp)];

				// We need to ensure that we handle the inconsistent "boolean" value correctly
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Force_shields_on, m_force_shields);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Ship_locked, m_ship_locked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Weapons_locked, m_weapons_locked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Primaries_locked, m_primaries_locked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Secondaries_locked, m_secondaries_locked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Lock_all_turrets_initially, m_turrets_locked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Afterburner_locked, m_afterburner_locked);
			}

			objp = GET_NEXT(objp);
		}

	} else {
		modify(Objects[_editor->currentObject].phys_info.speed, (float)m_velocity);
		modify(Objects[_editor->currentObject].shield_quadrant[0], (float)m_shields);
		modify(Objects[_editor->currentObject].hull_strength, (float)m_hull);

		Objects[_editor->currentObject].flags.set(Object::Object_Flags::No_shields, m_has_shields == 0);

		// We need to ensure that we handle the inconsistent "boolean" value correctly. Not strictly needed here but
		// just to be safe...
		handle_inconsistent_flag(Ships[m_ship].flags, Ship::Ship_Flags::Force_shields_on, m_force_shields);
		handle_inconsistent_flag(Ships[m_ship].flags, Ship::Ship_Flags::Ship_locked, m_ship_locked);
		handle_inconsistent_flag(Ships[m_ship].flags, Ship::Ship_Flags::Weapons_locked, m_weapons_locked);
		handle_inconsistent_flag(Ships[m_ship].flags, Ship::Ship_Flags::Primaries_locked, m_primaries_locked);
		handle_inconsistent_flag(Ships[m_ship].flags, Ship::Ship_Flags::Secondaries_locked, m_secondaries_locked);
		handle_inconsistent_flag(Ships[m_ship].flags, Ship::Ship_Flags::Lock_all_turrets_initially, m_turrets_locked);
		handle_inconsistent_flag(Ships[m_ship].flags, Ship::Ship_Flags::Afterburner_locked, m_afterburner_locked);
	}
	Ships[m_ship].team_name = m_team_color_setting;
	update_docking_info();
	_editor->missionChanged();
	return true;
}

void ShipInitialStatusDialogModel::reject() {}

void ShipInitialStatusDialogModel::set_modified()
{
	if (!_modified) {
		_modified = true;
	}
}

const bool ShipInitialStatusDialogModel::query_modified()
{
	return _modified;
}

void ShipInitialStatusDialogModel::setVelocity(int value)
{
	modify(m_velocity, value);
}

const int ShipInitialStatusDialogModel::getVelocity()
{
	return m_velocity;
}

void ShipInitialStatusDialogModel::setHull(int value)
{
	modify(m_hull, value);
}

const int ShipInitialStatusDialogModel::getHull()
{
	return m_hull;
}

void ShipInitialStatusDialogModel::setHasShield(int state)
{
	modify(m_has_shields, state);
}

int ShipInitialStatusDialogModel::getHasShield()
{
	return m_has_shields;
}

void ShipInitialStatusDialogModel::setShieldHull(int value)
{
	modify(m_shields, value);
}

const int ShipInitialStatusDialogModel::getShieldHull()
{
	return m_shields;
}

void ShipInitialStatusDialogModel::setForceShield(const int state)
{
	modify(m_force_shields, state);
}

const int ShipInitialStatusDialogModel::getForceShield()
{
	return m_force_shields;
}

void ShipInitialStatusDialogModel::setShipLocked(int state)
{
	modify(m_ship_locked, state);
}

const int ShipInitialStatusDialogModel::getShipLocked()
{
	return m_ship_locked;
}

void ShipInitialStatusDialogModel::setWeaponLocked(int state)
{
	modify(m_weapons_locked, state);
}

const int ShipInitialStatusDialogModel::getWeaponLocked()
{
	return m_weapons_locked;
}

void ShipInitialStatusDialogModel::setPrimariesDisabled(int state)
{
	modify(m_primaries_locked, state);
}

const int ShipInitialStatusDialogModel::getPrimariesDisabled()
{
	return m_primaries_locked;
}

void ShipInitialStatusDialogModel::setSecondariesDisabled(int state)
{
	modify(m_secondaries_locked, state);
}

const int ShipInitialStatusDialogModel::getSecondariesDisabled()
{
	return m_secondaries_locked;
}

void ShipInitialStatusDialogModel::setTurretsDisabled(int state)
{
	modify(m_turrets_locked, state);
}

const int ShipInitialStatusDialogModel::getTurretsDisabled()
{
	return m_turrets_locked;
}

void ShipInitialStatusDialogModel::setAfterburnerDisabled(int state)
{
	modify(m_afterburner_locked, state);
}

const int ShipInitialStatusDialogModel::getAfterburnerDisabled()
{
	return m_afterburner_locked;
}

void ShipInitialStatusDialogModel::setDamage(int value)
{
	modify(m_damage, value);
}

const int ShipInitialStatusDialogModel::getDamage()
{
	return m_damage;
}

SCP_string ShipInitialStatusDialogModel::getCargo()
{
	return m_cargo_name;
}

void ShipInitialStatusDialogModel::setCargo(const SCP_string& text)
{
	modify(m_cargo_name, text);
}

SCP_string ShipInitialStatusDialogModel::getColour()
{
	return m_team_color_setting;
}

void ShipInitialStatusDialogModel::setColour(const SCP_string& text)
{
	modify(m_team_color_setting, text);
}

void ShipInitialStatusDialogModel::change_subsys(int new_subsys = 0)
{
	int z, cargo_index;
	ship_subsys* ptr;
	// Goober5000
	ship_has_scannable_subsystems = Ship_info[Ships[m_ship].ship_info_index].is_huge_ship();
	if (Ships[m_ship].flags[Ship::Ship_Flags::Toggle_subsystem_scanning]) {
		ship_has_scannable_subsystems = !ship_has_scannable_subsystems;
	}

	if (cur_subsys != -1) {
		ptr = GET_FIRST(&Ships[m_ship].subsys_list);
		while (cur_subsys--) {
			Assert(ptr != END_OF_LIST(&Ships[m_ship].subsys_list));
			ptr = GET_NEXT(ptr);
		}

		modify(ptr->current_hits, 100.0f - (float)m_damage);

		// update cargo name
		if (!m_cargo_name.empty()) { //-V805
			cargo_index = string_lookup(m_cargo_name.c_str(), Cargo_names, Num_cargo);
			if (cargo_index == -1) {
				if (Num_cargo < MAX_CARGO) {
					cargo_index = Num_cargo++;
					strcpy(Cargo_names[cargo_index], m_cargo_name.c_str());
					ptr->subsys_cargo_name = cargo_index;
				} else {
					SCP_string str;
					sprintf(str, "Maximum number of cargo names %d reached.\nIgnoring new name.\n", MAX_CARGO);
					_viewport->dialogProvider->showButtonDialog(DialogType::Warning,
						"Cargo Error",
						str,
						{DialogButton::Ok});
					ptr->subsys_cargo_name = 0;
				}
			} else {
				ptr->subsys_cargo_name = cargo_index;
			}
			set_modified();
		}
	}

	cur_subsys = z = new_subsys;
	if (z == LB_ERR) {
		m_damage = 100;

	} else {
		ptr = GET_FIRST(&Ships[m_ship].subsys_list);
		while (z--) {
			Assert(ptr != END_OF_LIST(&Ships[m_ship].subsys_list));
			ptr = GET_NEXT(ptr);
		}

		m_damage = 100 - (int)ptr->current_hits;
		if (ship_has_scannable_subsystems) {
			if (ptr->subsys_cargo_name > 0) {
				m_cargo_name = Cargo_names[ptr->subsys_cargo_name];
			} else {
				m_cargo_name = "";
			}
		} else {
			m_cargo_name = "";
		}
	}
}

const int ShipInitialStatusDialogModel::getShip()
{
	return m_ship;
}

const int ShipInitialStatusDialogModel::getnum_dock_points()
{
	return num_dock_points;
}

const int ShipInitialStatusDialogModel::getShip_has_scannable_subsystems()
{
	return ship_has_scannable_subsystems;
}

const dockpoint_information* ShipInitialStatusDialogModel::getdockpoint_array()
{
	return dockpoint_array;
}

void ShipInitialStatusDialogModel::setDockee(const int point, const int ship)
{
	modify(dockpoint_array[point].dockee_shipnum, ship);
	modify(dockpoint_array[point].dockee_point, -1);
}

void ShipInitialStatusDialogModel::setDockeePoint(const int dockPoint, const int dockeePoint)
{
	modify(dockpoint_array[dockPoint].dockee_point, dockeePoint);
}

} // namespace dialogs
} // namespace fred
} // namespace fso