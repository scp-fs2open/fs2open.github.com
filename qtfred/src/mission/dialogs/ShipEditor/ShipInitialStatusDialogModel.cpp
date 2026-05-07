#include "ShipInitialStatusDialogModel.h"

#include "mission/object.h"

#include <globalincs/linklist.h>
#include <localization/localize.h>
#include <mod_table/mod_table.h>

#include <QtWidgets>
#include <object/objectdock.cpp>

namespace fso::fred::dialogs {

void resetArrivalToFalse(int shipnum, bool resetWing, EditorViewport* _viewport)
{
	char buf[256];
	ship* shipp = &Ships[shipnum];

	// falsify the ship cue
	if (setCueToFalse(&shipp->arrival_cue)) {
		sprintf(buf, "Setting arrival cue of ship %s\nto false for initial docking purposes.", shipp->ship_name);
		_viewport->dialogProvider->showButtonDialog(DialogType::Information, "Notice", buf, {DialogButton::Ok});
	}

	// falsify the wing cue and all ships in that wing
	if (resetWing && shipp->wingnum >= 0) {
		int i;
		wing* wingp = &Wings[shipp->wingnum];

		if (setCueToFalse(&wingp->arrival_cue)) {
			sprintf(buf, "Setting arrival cue of wing %s\nto false for initial docking purposes.", wingp->name);
			_viewport->dialogProvider->showButtonDialog(DialogType::Information, "Notice", buf, {DialogButton::Ok});
		}

		for (i = 0; i < wingp->wave_count; i++)
			resetArrivalToFalse(wingp->ship_index[i], false, _viewport);
	}
}

bool setCueToFalse(int* cue)
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
void initialStatusUnmarkDockHandledFlag(object* objp)
{
	objp->flags.remove(Object::Object_Flags::Docked_already_handled);
}

void initialStatusMarkDockLeaderHelper(object* objp, dock_function_info* infop, EditorViewport* viewport)
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
		if (existing_leader != nullptr) {
			ship* leader_shipp = &Ships[existing_leader->instance];

			// keep existing leader if he has a higher priority than us
			if (ship_class_compare(shipp->ship_info_index, leader_shipp->ship_info_index) >= 0) {
				// set my arrival cue to false
				resetArrivalToFalse(SHIP_INDEX(shipp), true, viewport);
				return;
			}

			// otherwise, unmark the existing leader and set his arrival cue to false
			leader_shipp->flags.remove(Ship::Ship_Flags::Dock_leader);
			resetArrivalToFalse(SHIP_INDEX(leader_shipp), true, viewport);
		}

		// mark and save me as the leader
		shipp->flags.set(Ship::Ship_Flags::Dock_leader);
		infop->maintained_variables.objp_value = objp;
	}
}

ShipInitialStatusDialogModel::ShipInitialStatusDialogModel(QObject* parent, EditorViewport* viewport, bool multi)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(multi);
}

void ShipInitialStatusDialogModel::initializeData(bool multi)
{
	_multiEdit = multi;
	int vflag, sflag, hflag;
	object* objp = nullptr;
	_ship = _editor->cur_ship;
	if (_ship == -1) {
		Assert(
			(Objects[_editor->currentObject].type == OBJ_SHIP) || (Objects[_editor->currentObject].type == OBJ_START));
		_ship = get_ship_from_obj(_editor->currentObject);
		Assert(_ship >= 0);
	}

	_moveShipsWhenUndocking = _viewport->Move_ships_when_undocking;

	// initialize dockpoint stuff
	if (!_multiEdit) {
		_numDockPoints = model_get_num_dock_points(Ship_info[Ships[_ship].ship_info_index].model_num);
		_dockpointArray = new dockpoint_information[_numDockPoints];
		objp = &Objects[Ships[_ship].objnum];
		for (int i = 0; i < _numDockPoints; i++) {
			object* docked_objp = dock_find_object_at_dockpoint(objp, i);
			if (docked_objp != nullptr) {
				_dockpointArray[i].dockee_shipnum = docked_objp->instance;
				_dockpointArray[i].dockee_point = dock_find_dockpoint_used_by_object(docked_objp, objp);
			} else {
				_dockpointArray[i].dockee_shipnum = -1;
				_dockpointArray[i].dockee_point = -1;
			}
		}
	}
	vflag = sflag = hflag = 0;
	_velocity = static_cast<int>(Objects[_editor->currentObject].phys_info.speed);
	_shields = static_cast<int>(Objects[_editor->currentObject].shield_quadrant[0]);
	_hull = static_cast<int>(Objects[_editor->currentObject].hull_strength);
	_guardianThreshold = Ships[_ship].ship_guardian_threshold;
	if (Objects[_editor->currentObject].flags[Object::Object_Flags::No_shields])
		_hasShields = Qt::Unchecked;
	else
		_hasShields = Qt::Checked;

	if (Ships[_ship].flags[Ship::Ship_Flags::Force_shields_on])
		_forceShields = Qt::Checked;
	else
		_forceShields = Qt::Unchecked;

	if (Ships[_ship].flags[Ship::Ship_Flags::Ship_locked])
		_shipLocked = Qt::Checked;
	else
		_shipLocked = Qt::Unchecked;

	if (Ships[_ship].flags[Ship::Ship_Flags::Weapons_locked])
		_weaponsLocked = Qt::Checked;
	else
		_weaponsLocked = Qt::Unchecked;
	// Lock primaries
	if (Ships[_ship].flags[Ship::Ship_Flags::Primaries_locked]) {
		_primariesLocked = Qt::Checked;
	} else {
		_primariesLocked = Qt::Unchecked;
	}
	// Lock secondaries
	if (Ships[_ship].flags[Ship::Ship_Flags::Secondaries_locked]) {
		_secondariesLocked = Qt::Checked;
	} else {
		_secondariesLocked = Qt::Unchecked;
	}

	// Lock turrets
	if (Ships[_ship].flags[Ship::Ship_Flags::Lock_all_turrets_initially]) {
		_turretsLocked = Qt::Checked;
	} else {
		_turretsLocked = Qt::Unchecked;
	}

	if (Ships[_ship].flags[Ship::Ship_Flags::Afterburner_locked]) {
		_afterburnerLocked = Qt::Checked;
	} else {
		_afterburnerLocked = Qt::Unchecked;
	}

	if (_multiEdit) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				(objp->flags[Object::Object_Flags::Marked])) {
				if (objp->phys_info.speed != _velocity)
					vflag = 1;
				if (static_cast<int>(objp->shield_quadrant[0]) != _shields)
					sflag = 1;
				if (static_cast<int>(objp->hull_strength) != _hull)
					hflag = 1;
				if (objp->flags[Object::Object_Flags::No_shields]) {
					if (_hasShields)
						_hasShields = Qt::PartiallyChecked;

				} else {
					if (!_hasShields)
						_hasShields = Qt::PartiallyChecked;
				}

				Assert((objp->type == OBJ_SHIP) || (objp->type == OBJ_START));

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Force_shields_on]) {
					if (!_forceShields)
						_forceShields = Qt::PartiallyChecked;

				} else {
					if (_forceShields)
						_forceShields = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Ship_locked]) {
					if (!_shipLocked)
						_shipLocked = Qt::PartiallyChecked;

				} else {
					if (_shipLocked)
						_shipLocked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Weapons_locked]) {
					if (!_weaponsLocked)
						_weaponsLocked = Qt::PartiallyChecked;

				} else {
					if (_weaponsLocked)
						_weaponsLocked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Primaries_locked]) {
					if (!_primariesLocked)
						_primariesLocked = Qt::PartiallyChecked;
				} else {
					if (_primariesLocked)
						_primariesLocked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Secondaries_locked]) {
					if (!_secondariesLocked)
						_secondariesLocked = Qt::PartiallyChecked;
				} else {
					if (_secondariesLocked)
						_secondariesLocked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Lock_all_turrets_initially]) {
					if (!_turretsLocked)
						_turretsLocked = Qt::PartiallyChecked;
				} else {
					if (_turretsLocked)
						_turretsLocked = Qt::PartiallyChecked;
				}

				if (Ships[get_ship_from_obj(objp)].flags[Ship::Ship_Flags::Afterburner_locked]) {
					if (!_afterburnerLocked)
						_afterburnerLocked = Qt::PartiallyChecked;
				} else {
					if (_afterburnerLocked)
						_afterburnerLocked = Qt::PartiallyChecked;
				}
			}

			objp = GET_NEXT(objp);
		}
	}

	if (Ship_info[Ships[_ship].ship_info_index].uses_team_colors) {
		_useTeams = true;
		_teamColorSetting = Ships[_ship].team_name;
	}
	changeSubsys(0);

	if (vflag) {
		_velocity = BLANKFIELD;
	}
	if (vflag) {
		_velocity = BLANKFIELD;
	}
	if (vflag) {
		_velocity = BLANKFIELD;
	}
	_modified = false;
	modelChanged();
	_modified = false;
}

void ShipInitialStatusDialogModel::updateDockingInfo()
{
	int i;
	object* objp = &Objects[Ships[_ship].objnum];
	for (i = 0; i < _numDockPoints; i++) {
		// see if the object at this point is no longer there
		object* dockee_objp = dock_find_object_at_dockpoint(objp, i);
		if (dockee_objp != nullptr) {
			// check if the dockee ship thinks that this ship is docked to this dock point
			if (objp != dock_find_object_at_dockpoint(dockee_objp, _dockpointArray[i].dockee_point)) {
				// undock it
				undock(objp, dockee_objp);
			}
		}
	}
	// add new info
	for (i = 0; i < _numDockPoints; i++) {
		// see if there is an object at this point that wasn't there before
		if (_dockpointArray[i].dockee_shipnum >= 0) {
			if (dock_find_object_at_dockpoint(objp, i) == nullptr) {
				object* dockee_objp = &Objects[Ships[_dockpointArray[i].dockee_shipnum].objnum];
				int dockee_point = _dockpointArray[i].dockee_point;

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

	if (_moveShipsWhenUndocking) {
		if (ship_class_compare(Ships[ship_num].ship_info_index, Ships[other_ship_num].ship_info_index) <= 0) {
			vm_vec_scale_add2(&objp2->pos,
				&v,
				ship_class_get_length(&Ship_info[Ships[objp2->instance].ship_info_index]));
		} else {
			vm_vec_scale_add2(&objp1->pos,
				&v,
				ship_class_get_length(&Ship_info[Ships[objp1->instance].ship_info_index]) * -1.0f);
		}
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

void ShipInitialStatusDialogModel::dock(object* objp, int dockpoint, object* otherObjp, int otherDockpoint)
{
	if (objp == nullptr || otherObjp == nullptr) {
		return;
	}

	if (dockpoint < 0 || otherDockpoint < 0) {
		return;
	}

	dock_function_info dfi;

	// do the docking (do it in reverse so that the current object stays put)
	ai_dock_with_object(otherObjp, otherDockpoint, objp, dockpoint, AIDO_DOCK_NOW);

	// unmark the handled flag in preparation for the next step
	dockEvaluateAllDockedObjects(objp, &dfi, initialStatusUnmarkDockHandledFlag);

	// move all other objects to catch up with it
	dock_move_docked_objects(objp);

	// set the dock leader
	dockEvaluateAllDockedObjects(objp, &dfi, initialStatusMarkDockLeaderHelper);

	// if no leader, mark me
	if (dfi.maintained_variables.int_value == 0)
		Ships[objp->instance].flags.set(Ship::Ship_Flags::Dock_leader);
}

// Duplicated from objectdock to handle scope errors
void ShipInitialStatusDialogModel::dockEvaluateAllDockedObjects(object* objp,
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
		for (dock_instance* ptr = hub_objp->dock_list; ptr != nullptr; ptr = ptr->next) {
			// call the function for this object, and return if instructed
			function(ptr->docked_objp, infop, _viewport);
			if (infop->early_return_condition)
				return;
		}
	}

	// we have multiple objects docked and we must treat them as a tree
	else {
		// create a bit array to mark the objects we check
		auto visited_bitstring = (ubyte*)vm_malloc(calculate_num_bytes(MAX_OBJECTS));

		// clear it
		memset(visited_bitstring, 0, calculate_num_bytes(MAX_OBJECTS));

		// start evaluating the tree
		dockEvaluateTree(objp, infop, function, visited_bitstring);

		// destroy the bit array
		vm_free(visited_bitstring);
		visited_bitstring = nullptr;
	}
}

void ShipInitialStatusDialogModel::dockEvaluateAllDockedObjects(object* objp,
	dock_function_info* infop,
	void (*function)(object*))
{
	Assertion((objp != nullptr) && (infop != nullptr) && (function != nullptr),
		"dock_evaluate_all_docked_objects, invalid argument(s)");

	// not docked?
	if (!object_is_docked(objp)) {
		// call the function for just the one object
		function(objp);
		return;
	}

	// we only have two objects docked
	if (dock_check_docked_one_on_one(objp)) {
		// call the function for the first object, and return if instructed
		function(objp);
		if (infop->early_return_condition)
			return;

		// call the function for the second object, and return if instructed
		function(objp->dock_list->docked_objp);
		if (infop->early_return_condition)
			return;
	}

	// we have multiple objects docked and we're treating them as a hub
	else if (dock_check_assume_hub()) {
		// get the hub
		object* hub_objp = dock_get_hub(objp);

		// call the function for the hub, and return if instructed
		function(hub_objp);
		if (infop->early_return_condition)
			return;

		// iterate through all docked objects
		for (dock_instance* ptr = hub_objp->dock_list; ptr != nullptr; ptr = ptr->next) {
			// call the function for this object, and return if instructed
			function(ptr->docked_objp);
			if (infop->early_return_condition)
				return;
		}
	}

	// we have multiple objects docked and we must treat them as a tree
	else {
		// create a bit array to mark the objects we check
		auto visited_bitstring = (ubyte*)vm_malloc(calculate_num_bytes(MAX_OBJECTS));

		// clear it
		memset(visited_bitstring, 0, calculate_num_bytes(MAX_OBJECTS));

		// start evaluating the tree
		dockEvaluateTree(objp, infop, function, visited_bitstring);

		// destroy the bit array
		vm_free(visited_bitstring);
		visited_bitstring = nullptr;
	}
}

void ShipInitialStatusDialogModel::dockEvaluateTree(object* objp,
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
	for (dock_instance* ptr = objp->dock_list; ptr != nullptr; ptr = ptr->next) {
		// start another tree with the docked object as the root, and return if instructed
		dockEvaluateTree(ptr->docked_objp, infop, function, visited_bitstring);
		if (infop->early_return_condition)
			return;
	}
}

void ShipInitialStatusDialogModel::dockEvaluateTree(object* objp,
	dock_function_info* infop,
	void (*function)(object*),
	ubyte* visited_bitstring)
{
	// make sure we haven't visited this object already
	if (get_bit(visited_bitstring, OBJ_INDEX(objp)))
		return;

	// mark as visited
	set_bit(visited_bitstring, OBJ_INDEX(objp));

	// call the function for this object, and return if instructed
	function(objp);
	if (infop->early_return_condition)
		return;

	// iterate through all docked objects
	for (dock_instance* ptr = objp->dock_list; ptr != nullptr; ptr = ptr->next) {
		// start another tree with the docked object as the root, and return if instructed
		dockEvaluateTree(ptr->docked_objp, infop, function, visited_bitstring);
		if (infop->early_return_condition)
			return;
	}
}

bool ShipInitialStatusDialogModel::apply()
{
	object* objp;

	changeSubsys(0);
	if (_multiEdit) {
		objp = GET_FIRST(&obj_used_list);
		while (objp != END_OF_LIST(&obj_used_list)) {
			if (((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) &&
				(objp->flags[Object::Object_Flags::Marked])) {
				modify(objp->phys_info.speed, (float)_velocity);

				modify(objp->shield_quadrant[0], (float)_shields);

				modify(objp->hull_strength, (float)_hull);

				if (_hasShields == Qt::Checked) {
					objp->flags.remove(Object::Object_Flags::No_shields);
				} else if (_hasShields == Qt::Unchecked) {
					objp->flags.set(Object::Object_Flags::No_shields);
				}
				auto shipp = &Ships[get_ship_from_obj(objp)];
				shipp->ship_guardian_threshold = _guardianThreshold;
				// We need to ensure that we handle the inconsistent "boolean" value correctly
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Force_shields_on, _forceShields);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Ship_locked, _shipLocked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Weapons_locked, _weaponsLocked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Primaries_locked, _primariesLocked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Secondaries_locked, _secondariesLocked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Lock_all_turrets_initially, _turretsLocked);
				handle_inconsistent_flag(shipp->flags, Ship::Ship_Flags::Afterburner_locked, _afterburnerLocked);
				shipp->team_name = _teamColorSetting;
			}

			objp = GET_NEXT(objp);
		}

	} else {
		modify(Objects[_editor->currentObject].phys_info.speed, (float)_velocity);
		modify(Objects[_editor->currentObject].shield_quadrant[0], (float)_shields);
		modify(Objects[_editor->currentObject].hull_strength, (float)_hull);

		Objects[_editor->currentObject].flags.set(Object::Object_Flags::No_shields, _hasShields == 0);
		Ships[_ship].ship_guardian_threshold = _guardianThreshold;
		// We need to ensure that we handle the inconsistent "boolean" value correctly. Not strictly needed here but
		// just to be safe...
		handle_inconsistent_flag(Ships[_ship].flags, Ship::Ship_Flags::Force_shields_on, _forceShields);
		handle_inconsistent_flag(Ships[_ship].flags, Ship::Ship_Flags::Ship_locked, _shipLocked);
		handle_inconsistent_flag(Ships[_ship].flags, Ship::Ship_Flags::Weapons_locked, _weaponsLocked);
		handle_inconsistent_flag(Ships[_ship].flags, Ship::Ship_Flags::Primaries_locked, _primariesLocked);
		handle_inconsistent_flag(Ships[_ship].flags, Ship::Ship_Flags::Secondaries_locked, _secondariesLocked);
		handle_inconsistent_flag(Ships[_ship].flags, Ship::Ship_Flags::Lock_all_turrets_initially, _turretsLocked);
		handle_inconsistent_flag(Ships[_ship].flags, Ship::Ship_Flags::Afterburner_locked, _afterburnerLocked);
		Ships[_ship].team_name = _teamColorSetting;
	}
	updateDockingInfo();
	_editor->missionChanged();
	return true;
}

void ShipInitialStatusDialogModel::reject() {}

bool ShipInitialStatusDialogModel::getMoveShipsWhenUndocking() const
{
	return _moveShipsWhenUndocking;
}

void ShipInitialStatusDialogModel::setMoveShipsWhenUndocking(bool moveShips)
{
	modify(_moveShipsWhenUndocking, moveShips);
}

void ShipInitialStatusDialogModel::setVelocity(int velocity)
{
	modify(_velocity, velocity);
}

int ShipInitialStatusDialogModel::getVelocity() const
{
	return _velocity;
}

void ShipInitialStatusDialogModel::setHull(int hull)
{
	modify(_hull, hull);
}

int ShipInitialStatusDialogModel::getHull() const
{
	return _hull;
}

void ShipInitialStatusDialogModel::setHasShield(int hasShield)
{
	modify(_hasShields, hasShield);
}

int ShipInitialStatusDialogModel::getHasShield() const
{
	return _hasShields;
}

void ShipInitialStatusDialogModel::setShieldHull(int shieldHull)
{
	modify(_shields, shieldHull);
}

int ShipInitialStatusDialogModel::getShieldHull() const
{
	return _shields;
}

void ShipInitialStatusDialogModel::setForceShield(int forceShield)
{
	modify(_forceShields, forceShield);
}

int ShipInitialStatusDialogModel::getForceShield() const
{
	return _forceShields;
}

void ShipInitialStatusDialogModel::setShipLocked(int locked)
{
	modify(_shipLocked, locked);
}

int ShipInitialStatusDialogModel::getShipLocked() const
{
	return _shipLocked;
}

void ShipInitialStatusDialogModel::setWeaponLocked(int locked)
{
	modify(_weaponsLocked, locked);
}

int ShipInitialStatusDialogModel::getWeaponLocked() const
{
	return _weaponsLocked;
}

void ShipInitialStatusDialogModel::setPrimariesDisabled(int disabled)
{
	modify(_primariesLocked, disabled);
}

int ShipInitialStatusDialogModel::getPrimariesDisabled() const
{
	return _primariesLocked;
}

void ShipInitialStatusDialogModel::setSecondariesDisabled(int disabled)
{
	modify(_secondariesLocked, disabled);
}

int ShipInitialStatusDialogModel::getSecondariesDisabled() const
{
	return _secondariesLocked;
}

void ShipInitialStatusDialogModel::setTurretsDisabled(int disabled)
{
	modify(_turretsLocked, disabled);
}

int ShipInitialStatusDialogModel::getTurretsDisabled() const
{
	return _turretsLocked;
}

void ShipInitialStatusDialogModel::setAfterburnerDisabled(int disabled)
{
	modify(_afterburnerLocked, disabled);
}

int ShipInitialStatusDialogModel::getAfterburnerDisabled() const
{
	return _afterburnerLocked;
}

void ShipInitialStatusDialogModel::setDamage(int damage)
{
	modify(_damage, damage);
}

int ShipInitialStatusDialogModel::getDamage() const
{
	return _damage;
}

SCP_string ShipInitialStatusDialogModel::getCargo() const
{
	return _cargoName;
}

void ShipInitialStatusDialogModel::setCargo(const SCP_string& cargo)
{
	modify(_cargoName, cargo);
}

SCP_string ShipInitialStatusDialogModel::getCargoTitle() const
{
	return _cargoTitle;
}

void ShipInitialStatusDialogModel::setCargoTitle(const SCP_string& cargoTitle)
{
	modify(_cargoTitle, cargoTitle);
}

SCP_string ShipInitialStatusDialogModel::getColor() const
{
	return _teamColorSetting;
}

void ShipInitialStatusDialogModel::setColor(const SCP_string& color)
{
	modify(_teamColorSetting, color);
}

void ShipInitialStatusDialogModel::changeSubsys(int subsysIndex)
{
	int z, cargo_index;
	ship_subsys* ptr;
	// Goober5000
	_shipHasScannableSubsystems = Ship_info[Ships[_ship].ship_info_index].is_huge_ship();
	if (Ships[_ship].flags[Ship::Ship_Flags::Toggle_subsystem_scanning]) {
		_shipHasScannableSubsystems = ~_shipHasScannableSubsystems;
	}

	if (_curSubsys != -1) {
		ptr = GET_FIRST(&Ships[_ship].subsys_list);
		while (_curSubsys--) {
			Assert(ptr != END_OF_LIST(&Ships[_ship].subsys_list));
			ptr = GET_NEXT(ptr);
		}

		ptr->current_hits = 100.0f - (float)_damage;

		// update cargo name
		if (!_cargoName.empty()) { //-V805
			lcl_fred_replace_stuff(_cargoName);
			cargo_index = string_lookup(_cargoName.c_str(), Cargo_names, Num_cargo);
			if (cargo_index == -1) {
				if (Num_cargo < MAX_CARGO) {
					cargo_index = Num_cargo++;
					strcpy(Cargo_names[cargo_index], _cargoName.c_str());
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
		}

		// update cargo title
		strcpy_s(ptr->subsys_cargo_title, _cargoTitle.c_str());
	}

	_curSubsys = z = subsysIndex;
	if (z == -1) {
		_damage = 100;

	} else {
		ptr = GET_FIRST(&Ships[_ship].subsys_list);
		while (z--) {
			Assert(ptr != END_OF_LIST(&Ships[_ship].subsys_list));
			ptr = GET_NEXT(ptr);
		}

		_damage = 100 - static_cast<int>(ptr->current_hits);
		if (ptr->subsys_cargo_name > 0) {
			_cargoName = Cargo_names[ptr->subsys_cargo_name];
		} else {
			_cargoName = "";
		}
		_cargoTitle = ptr->subsys_cargo_title;
	}
	set_modified();
	modelChanged();
}

int ShipInitialStatusDialogModel::getShip() const
{
	return _ship;
}

int ShipInitialStatusDialogModel::getNumDockPoints() const
{
	return _numDockPoints;
}

int ShipInitialStatusDialogModel::getShipHasScannableSubsystems() const
{
	return _shipHasScannableSubsystems;
}

dockpoint_information* ShipInitialStatusDialogModel::getDockpointArray() const
{
	return _dockpointArray;
}

void ShipInitialStatusDialogModel::setDockee(int dockPointIndex, int dockeeShipnum)
{
	modify(_dockpointArray[dockPointIndex].dockee_shipnum, dockeeShipnum);
	modify(_dockpointArray[dockPointIndex].dockee_point, -1);
}

void ShipInitialStatusDialogModel::setDockeePoint(int dockPointIndex, int dockeePoint)
{
	modify(_dockpointArray[dockPointIndex].dockee_point, dockeePoint);
}

bool ShipInitialStatusDialogModel::getUseTeamcolours() const
{
	return _useTeams;
}

bool ShipInitialStatusDialogModel::getIfMultipleShips() const
{
	return _multiEdit;
}

int ShipInitialStatusDialogModel::getGuardian() const
{
	return _guardianThreshold;
}

void ShipInitialStatusDialogModel::setGuardian(int guardian)
{
	modify(_guardianThreshold, guardian);
}

bool ShipInitialStatusDialogModel::getToggleSubsystemScanning() const
{
	return Ships[_ship].flags[Ship::Ship_Flags::Toggle_subsystem_scanning];
}

bool ShipInitialStatusDialogModel::getUseNewScanningBehavior()
{
	return Use_new_scanning_behavior;
}

} // namespace fso::fred::dialogs
