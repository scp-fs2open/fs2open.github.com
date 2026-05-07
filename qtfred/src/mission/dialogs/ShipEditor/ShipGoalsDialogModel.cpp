#include "ShipGoalsDialogModel.h"
#include "ui/util/ErrorChecker.h"
#include <globalincs/linklist.h>
#include <mission/object.h>
#include <model/model.h>

namespace fso::fred::dialogs {

ShipGoalsDialogModel::ShipGoalsDialogModel(QObject* parent, EditorViewport* viewport, bool multi, int selfShip, int selfWing)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(multi, selfShip, selfWing);
}

void ShipGoalsDialogModel::initComboData()
{
	// don't add more than one of the same string (case-insensitive)
	SCP_unordered_map<SCP_string, size_t, SCP_string_lcase_hash, SCP_string_lcase_equal_to> strings_to_indexes;

	// start by adding "None"
	auto none_str = "None";
	strings_to_indexes.emplace(none_str, 0);
	SCP_set<ai_goal_mode> none_set{ AI_GOAL_NONE };
	_aiGoalComboData.clear();
	_aiGoalComboData.emplace_back(none_str, std::move(none_set));

	// initialize the data used in the combo boxes in the Initial Orders dialog
	for (int i = 0; i < _aiGoalListSize; ++i) {
		if (!_valid[i])
			continue;
		auto& entry = Editor::getAi_goal_list()[i];

		// see if we already added the string
		auto ii = strings_to_indexes.find(entry.name);
		if (ii != strings_to_indexes.end()) {
			// skip adding the string, but add the entry's goal definition to the combo box data at the existing index
			_aiGoalComboData[ii->second].second.insert(entry.def);
		} else {
			// this string will correspond to the index that is about to be created
			strings_to_indexes[entry.name] = _aiGoalComboData.size();

			// add the entry's goal definition as the first (maybe only) member of the set
			SCP_set<ai_goal_mode> new_set{ entry.def };
			_aiGoalComboData.emplace_back(entry.name, std::move(new_set));
		}
	}
}

const SCP_vector<std::pair<const char*, SCP_set<ai_goal_mode>>>& ShipGoalsDialogModel::getAiGoalComboData()
{
	return _aiGoalComboData;
}

ai_goal_mode ShipGoalsDialogModel::getFirstModeFromComboBox(int whichItem)
{
	// whichItem indicates initial goal 1 through MAX_AI_GOALS, so find that behavior...
	int behavior_index = _behavior[whichItem];

	// if we have a superposition of behaviors, bail here
	if (behavior_index < 0)
		return ai_goal_mode::AI_GOAL_SCHROEDINGER;

	// the behavior is the index into the combo box that contains a subset of goals from Ai_goal_list
	const auto& set = _aiGoalComboData[behavior_index].second;

	// just get the first mode in the set, since chase/chase-wing and guard/guard-wing are handled respectively together
	return *(set.begin());
}

bool ShipGoalsDialogModel::apply()
{
	int i;

	if (_goalp) {
		for (i = 0; i < ED_MAX_GOALS; i++)
			updateItem(i);

		verifyOrders();
	} else {
		object* ptr;

		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				_goalp = Ai_info[Ships[ptr->instance].ai_index].goals;
				for (i = 0; i < ED_MAX_GOALS; i++) {
					updateItem(i);
				}
			}

			ptr = GET_NEXT(ptr);
		}

		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				_selfShip = ptr->instance;
				_goalp = Ai_info[Ships[_selfShip].ai_index].goals;
				verifyOrders();
			}

			ptr = GET_NEXT(ptr);
		}
	}

	_editor->missionChanged();
	return true;
}

int ShipGoalsDialogModel::verifyOrders()
{
	ErrorChecker checker(_viewport);
	if (!checker.runCheck(ErrorCheckType::InitialOrders, {_goalp, _selfShip, _selfWing}))
		return 0;

	SCP_string message;
	for (const auto& entry : checker.getErrors()) {
		if (!message.empty())
			message += "\n\n";
		message += entry.message;
	}

	auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
		"Order Error",
		message,
		{DialogButton::Ok, DialogButton::Cancel});
	return (button == DialogButton::Ok) ? 0 : 1;
}

void ShipGoalsDialogModel::updateItem(int item)
{
	ai_goal_mode mode;
	char save[80]{};
	SCP_string error_message;
	waypoint_list* wp_list;

	if (item >= MAX_AI_GOALS)
		return;

	if (!_multiEdit || _priority[item] >= 0)
		_goalp[item].priority = _priority[item];

	mode = getFirstModeFromComboBox(item);
	switch (mode) {
	case AI_GOAL_NONE:
	case AI_GOAL_CHASE_ANY:
	case AI_GOAL_UNDOCK:
	case AI_GOAL_KEEP_SAFE_DISTANCE:
	case AI_GOAL_PLAY_DEAD:
	case AI_GOAL_PLAY_DEAD_PERSISTENT:
	case AI_GOAL_WARP:
		// these goals do not have a target in the dialog box, so let's set the goal and return immediately
		// so that we don't run afoul of the "doesn't have a valid target" code at the bottom of the function
		modify(_goalp[item].ai_mode, mode);
		return;

	case AI_GOAL_SCHROEDINGER:
		// return, but don't set the goal
		return;

	case AI_GOAL_WAYPOINTS:
	case AI_GOAL_WAYPOINTS_ONCE:
	case AI_GOAL_DISABLE_SHIP:
	case AI_GOAL_DISABLE_SHIP_TACTICAL:
	case AI_GOAL_DISARM_SHIP:
	case AI_GOAL_DISARM_SHIP_TACTICAL:
	case AI_GOAL_IGNORE:
	case AI_GOAL_IGNORE_NEW:
	case AI_GOAL_EVADE_SHIP:
	case AI_GOAL_STAY_NEAR_SHIP:
	case AI_GOAL_FORM_ON_WING:
	case AI_GOAL_STAY_STILL:
	case AI_GOAL_CHASE_SHIP_CLASS:
		break;

	case AI_GOAL_DESTROY_SUBSYSTEM: {
		if (_subsys[item].empty()) {
			sprintf(error_message, "Order #%d doesn't have valid subsystem name.  Order will be removed", item + 1);
			_viewport->dialogProvider->showButtonDialog(DialogType::Information,
				"Order Error",
				error_message,
				{ DialogButton::Ok });
			modify(_goalp[item].ai_mode, AI_GOAL_NONE);
			return;
		}

		// Look up the subsystem in the target ship to get a persistent name pointer.
		// Storing _subsys[item].c_str() directly would dangle after the model is destroyed.
		const char* persistent_name = nullptr;
		int target_ship_idx = _object[item] & DATA_MASK;
		if (target_ship_idx >= 0 && target_ship_idx < MAX_SHIPS) {
			ship_subsys* cur_ss = GET_FIRST(&Ships[target_ship_idx].subsys_list);
			while (cur_ss != END_OF_LIST(&Ships[target_ship_idx].subsys_list)) {
				if (!stricmp(cur_ss->system_info->subobj_name, _subsys[item].c_str())) {
					persistent_name = cur_ss->system_info->subobj_name;
					break;
				}
				cur_ss = GET_NEXT(cur_ss);
			}
		}

		if (!persistent_name) {
			sprintf(error_message, "Order #%d doesn't have valid subsystem name.  Order will be removed", item + 1);
			_viewport->dialogProvider->showButtonDialog(DialogType::Information,
				"Order Error",
				error_message,
				{ DialogButton::Ok });
			modify(_goalp[item].ai_mode, AI_GOAL_NONE);
			return;
		}

		if (!_goalp[item].docker.name || stricmp(_goalp[item].docker.name, persistent_name) != 0)
			set_modified();
		_goalp[item].docker.name = persistent_name;
		break;
	}

	case AI_GOAL_CHASE:
	case AI_GOAL_CHASE_WING:
		switch (_object[item] & TYPE_MASK) {
		case TYPE_SHIP:
		case TYPE_PLAYER:
			mode = AI_GOAL_CHASE;
			break;

		case TYPE_WING:
			mode = AI_GOAL_CHASE_WING;
			break;
		}

		break;

	case AI_GOAL_DOCK: {
		// Resolve persistent dock bay name pointers from the polymodel.
		// Storing _subsys[item].c_str() directly would dangle after the model is destroyed.
		char* docker = nullptr;
		char* dockee = nullptr;

		if (!_multiEdit || (_object[item] && !_subsys[item].empty())) {
			if (_selfShip >= 0) {
				int model_num = Ship_info[Ships[_selfShip].ship_info_index].model_num;
				polymodel* pm = model_get(model_num);
				if (pm) {
					for (int b = 0; b < pm->n_docks; b++) {
						if (!stricmp(pm->docking_bays[b].name, _subsys[item].c_str())) {
							docker = pm->docking_bays[b].name;
							break;
						}
					}
				}
			}
		}

		if (!_multiEdit || (_object[item] && (_dock2[item] >= 0))) {
			int dockee_ship = _object[item] & DATA_MASK;
			if (dockee_ship >= 0 && dockee_ship < MAX_SHIPS && _dock2[item] >= 0) {
				int model_num = Ship_info[Ships[dockee_ship].ship_info_index].model_num;
				polymodel* pm = model_get(model_num);
				if (pm && _dock2[item] < pm->n_docks) {
					dockee = pm->docking_bays[_dock2[item]].name;
				}
			}
		}

		if (!docker || !dockee) {
			sprintf(error_message, "Order #%d doesn't have valid docking points.  Order will be removed", item + 1);
			_viewport->dialogProvider->showButtonDialog(DialogType::Information,
				"Order Error",
				error_message,
				{ DialogButton::Ok });
			modify(_goalp[item].ai_mode, AI_GOAL_NONE);
			return;
		} else {
			if (!_goalp[item].docker.name || stricmp(_goalp[item].docker.name, docker) != 0)
				set_modified();
			if (!_goalp[item].dockee.name || stricmp(_goalp[item].dockee.name, dockee) != 0)
				set_modified();

			_goalp[item].docker.name = docker;
			_goalp[item].dockee.name = dockee;
		}

		break;
	}

	case AI_GOAL_GUARD:
	case AI_GOAL_GUARD_WING:
		switch (_object[item] & TYPE_MASK) {
		case TYPE_SHIP:
		case TYPE_PLAYER:
			mode = AI_GOAL_GUARD;
			break;

		case TYPE_WING:
			mode = AI_GOAL_GUARD_WING;
			break;
		}

		break;

	default:
		Warning(LOCATION, "Unknown AI_GOAL type 0x%x", mode);
		modify(_goalp[item].ai_mode, AI_GOAL_NONE);
		return;
	}
	modify(_goalp[item].ai_mode, mode);

	*save = 0;
	if (_goalp[item].target_name)
		strcpy_s(save, _goalp[item].target_name);

	switch (_object[item] & TYPE_MASK) {
		int not_used;

	case TYPE_SHIP:
	case TYPE_PLAYER:
		_goalp[item].target_name = ai_get_goal_target_name(Ships[_object[item] & DATA_MASK].ship_name, &not_used);
		break;

	case TYPE_WING:
		_goalp[item].target_name = ai_get_goal_target_name(Wings[_object[item] & DATA_MASK].name, &not_used);
		break;

	case TYPE_PATH:
		wp_list = find_waypoint_list_at_index(_object[item] & DATA_MASK);
		Assert(wp_list != nullptr);
		_goalp[item].target_name = ai_get_goal_target_name(wp_list->get_name(), &not_used);
		break;

	case TYPE_WAYPOINT:
		_goalp[item].target_name = ai_get_goal_target_name(object_name(_object[item] & DATA_MASK), &not_used);
		break;

	case TYPE_SHIP_CLASS:
		_goalp[item].target_name = ai_get_goal_target_name(Ship_info[_object[item] & DATA_MASK].name, &not_used);
		break;

	case 0:
	case (-1 & TYPE_MASK):
		if (_multiEdit)
			return;

		sprintf(error_message, "Order #%d doesn't have a valid target.  Order will be removed", item + 1);
		_viewport->dialogProvider->showButtonDialog(DialogType::Information,
			"Order Error",
			error_message,
			{ DialogButton::Ok });
		modify(_goalp[item].ai_mode, AI_GOAL_NONE);
		return;

	default:
		Error(LOCATION, "Unhandled TYPE_X #define %d in ship goals dialog box", _object[item] & TYPE_MASK);
	}

	if (stricmp(save, _goalp[item].target_name))
		set_modified();
}

void ShipGoalsDialogModel::reject() {}

void ShipGoalsDialogModel::initializeData(bool multi, int selfShip, int selfWing)
{
	int i, j, z;
	object* ptr;
	for (i = 0; i < ED_MAX_GOALS; i++) {
		_behavior[i] = -1;
		_object[i] = -1;
		_priority[i] = 0;
		_subsys[i] = "";
		_dock2[i] = -1;
	}
	_goalp = nullptr;
	_multiEdit = multi;
	_selfShip = selfShip;
	_selfWing = selfWing;
	Assert(_aiGoalListSize <= MAX_VALID);

	// start off with all goals available
	for (i = 0; i < _aiGoalListSize; i++) {
		_valid[i] = 1;
	}

	if (_selfShip >= 0) { // editing orders for just one ship
		for (i = 0; i < _aiGoalListSize; i++) {
			if (!(ai_query_goal_valid(_selfShip, Editor::getAi_goal_list()[i].def))) {
				_valid[i] = 0;
			}
		}
	} else if (_selfWing >= 0) { // editing orders for just one wing
		for (i = 0; i < Wings[_selfWing].wave_count; i++) {
			for (j = 0; j < _aiGoalListSize; j++) {
				if (!ai_query_goal_valid(Wings[_selfWing].ship_index[i], Editor::getAi_goal_list()[j].def)) {
					_valid[j] = 0;
				}
			}
		}
		for (i = 0; i < _aiGoalListSize; i++) {
			if (Editor::getAi_goal_list()[i].def == AI_GOAL_DOCK) { // a whole wing can't dock with one object..
				_valid[i] = 0;
			}
		}
	} else { // editing orders for all marked ships
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
				for (i = 0; i < _aiGoalListSize; i++) {
					if (!ai_query_goal_valid(ptr->instance, Editor::getAi_goal_list()[i].def)) {
						_valid[i] = 0;
					}
				}
			}

			ptr = GET_NEXT(ptr);
		}
	}
	if (Waypoint_lists.empty()) {
		for (i = 0; i < _aiGoalListSize; i++) {
			switch (Editor::getAi_goal_list()[i].def) {
			case AI_GOAL_WAYPOINTS:
			case AI_GOAL_WAYPOINTS_ONCE:
				_valid[i] = 0;
				break;
			default:
				break;
			}
		}
	}

	z = 0;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
			i = ptr->instance;

			if ((_selfShip > 0) && (_selfShip != i) && ship_docking_valid(_selfShip, i)) {
				z = 1;
			}
		}
		ptr = GET_NEXT(ptr);
	}

	if (!z) {
		for (i = 0; i < _aiGoalListSize; i++) {
			if (Editor::getAi_goal_list()[i].def == AI_GOAL_DOCK) {
				_valid[i] = 0;
			}
		}
	}

	// initialize the data for the behavior boxes (they remain constant while the dialog is open)
	initComboData();

	if (_selfShip >= 0) {
		initialize(Ai_info[Ships[_selfShip].ai_index].goals);
	} else if (_selfWing >= 0) {
		initialize(Wings[_selfWing].ai_goals);
	} else {
		initializeMulti();
	}
	modelChanged();
	_modified = false;
}

void ShipGoalsDialogModel::initialize(ai_goal* goals)
{
	int i, item, inst, flag;
	ai_goal_mode mode;
	object* ptr;
	SCP_vector<SCP_string> docks;

	// note that the flag variable is a bitfield:
	// 1 = ships
	// 2 = wings
	// 4 = waypoint paths
	// 8 = individual waypoints
	// 16 = ship classes

	_goalp = goals;
	for (item = 0; item < ED_MAX_GOALS; item++) {
		flag = 1;
		_priority[item] = 0;
		mode = AI_GOAL_NONE;

		if (item < MAX_AI_GOALS) {
			_priority[item] = _goalp[item].priority;
			mode = _goalp[item].ai_mode;
		}

		if (_priority[item] < 0 || _priority[item] > MAX_EDITOR_GOAL_PRIORITY) {
			_priority[item] = 50;
		}

		_behavior[item] = 0;
		if (mode != AI_GOAL_NONE) {
			i = static_cast<int>(_aiGoalComboData.size());
			while (i-- > 0) {
				const auto& set = _aiGoalComboData[i].second;
				if (set.find(mode) != set.end()) {
					_behavior[item] = i;
					break;
				}
			}
		}

		switch (mode) {
		case AI_GOAL_NONE:
		case AI_GOAL_CHASE_ANY:
		case AI_GOAL_UNDOCK:
		case AI_GOAL_KEEP_SAFE_DISTANCE:
		case AI_GOAL_PLAY_DEAD:
		case AI_GOAL_PLAY_DEAD_PERSISTENT:
		case AI_GOAL_WARP:
			continue;

		case AI_GOAL_CHASE_SHIP_CLASS:
			flag = 16; // target is a ship class
			break;

		case AI_GOAL_STAY_STILL:
			flag = 9; // target is a ship or a waypoint
			break;

		case AI_GOAL_CHASE:
		case AI_GOAL_GUARD:
		case AI_GOAL_DISABLE_SHIP:
		case AI_GOAL_DISABLE_SHIP_TACTICAL:
		case AI_GOAL_DISARM_SHIP:
		case AI_GOAL_DISARM_SHIP_TACTICAL:
		case AI_GOAL_IGNORE:
		case AI_GOAL_IGNORE_NEW:
		case AI_GOAL_EVADE_SHIP:
		case AI_GOAL_STAY_NEAR_SHIP:
		case AI_GOAL_FORM_ON_WING:
			break;

		case AI_GOAL_WAYPOINTS:
		case AI_GOAL_WAYPOINTS_ONCE:
			flag = 4; // target is a waypoint
			break;

		case AI_GOAL_DESTROY_SUBSYSTEM:
			// docker.name already holds the subsystem name string... copy it directly.
			// (ship_find_subsys returns an int index, not the name, so don't use it here.)
			if (_goalp[item].docker.name != nullptr)
				_subsys[item] = _goalp[item].docker.name;
			break;

		case AI_GOAL_DOCK:
			// Store the docker bay name string directly; the persistent pointer lives in the polymodel.
			if (_goalp[item].docker.name != nullptr)
				_subsys[item] = _goalp[item].docker.name;
			break;

		case AI_GOAL_CHASE_WING:
		case AI_GOAL_GUARD_WING:
			flag = 2; // target is a wing
			break;

		default:
			Error(LOCATION, "Unhandled AI_GOAL_X #define %d in ship goals dialog box", mode);
		}

		if (flag & 0x1) {
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
					inst = ptr->instance;
					if (ptr->type == OBJ_SHIP) {
						Assert(inst >= 0 && inst < MAX_SHIPS);
						if (!stricmp(_goalp[item].target_name, Ships[inst].ship_name)) {
							_object[item] = inst | TYPE_SHIP;
							break;
						}
					} else {
						Assert(inst >= 0 && inst < MAX_SHIPS);
						if (!stricmp(_goalp[item].target_name, Ships[inst].ship_name)) {
							_object[item] = inst | TYPE_PLAYER;
							break;
						}
					}
				}

				ptr = GET_NEXT(ptr);
			}
		}

		if (flag & 0x2) {
			for (i = 0; i < MAX_WINGS; i++) {
				if (Wings[i].wave_count) {
					if (!stricmp(_goalp[item].target_name, Wings[i].name)) {
						_object[item] = i | TYPE_WING;
						break;
					}
				}
			}
		}

		if (flag & 0x4) { // data is a waypoint path name
			SCP_vector<waypoint_list>::iterator ii;
			for (i = 0, ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++i, ++ii) {
				if (!stricmp(_goalp[item].target_name, ii->get_name())) {
					_object[item] = i | TYPE_PATH;
					break;
				}
			}
		}

		if (flag & 0x8) { // data is a waypoint name
			waypoint* wpt = find_matching_waypoint(_goalp[item].target_name);
			if (wpt != nullptr)
				_object[item] = wpt->get_objnum() | TYPE_WAYPOINT;
		}

		if (flag & 0x10) { // data is a ship class
			for (i = 0; i < ship_info_size(); i++) {
				if (!stricmp(_goalp[item].target_name, Ship_info[i].name)) {
					_object[item] = i | TYPE_SHIP_CLASS;
					break;
				}
			}
		}

		switch (mode) {
		case AI_GOAL_DOCK:
			_dock2[item] = -1;
			if (_object[item]) {
				docks =
					_editor->get_docking_list(Ship_info[Ships[_object[item] & DATA_MASK].ship_info_index].model_num);
				for (i = 0; unsigned(i) < docks.size(); i++) {
					Assert(_goalp[item].dockee.name);
					Assert(_goalp[item].dockee.index != -1);
					if (!stricmp(_goalp[item].dockee.name, docks[i].c_str())) {
						_dock2[item] = i;
						break;
					}
				}
			}
			break;
		default:
			break;
		}
	}
}

void ShipGoalsDialogModel::initializeMulti()
{
	int i, flag = 0;
	object* ptr;
	int behavior[ED_MAX_GOALS]{};
	int priority[ED_MAX_GOALS]{};
	SCP_string subsys[ED_MAX_GOALS]{};
	int dock2[ED_MAX_GOALS]{};
	int data[ED_MAX_GOALS]{};

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			initialize(Ai_info[Ships[ptr->instance].ai_index].goals);
			if (!flag) {
				flag = 1;
				for (i = 0; i < ED_MAX_GOALS; i++) {
					behavior[i] = _behavior[i];
					priority[i] = _priority[i];
					subsys[i] = _subsys[i];
					dock2[i] = _dock2[i];
					data[i] = _object[i];
				}
			} else {
				for (i = 0; i < ED_MAX_GOALS; i++) {
					if (behavior[i] != _behavior[i]) {
						behavior[i] = -1;
						data[i] = -1;
					}

					if (data[i] != _object[i]) {
						data[i] = -1;
						subsys[i] = -1;
						dock2[i] = -1;
					}

					if (priority[i] != _priority[i]) {
						priority[i] = -1;
					}
					if (subsys[i] != _subsys[i]) {
						subsys[i] = -1;
					}
					if (dock2[i] != _dock2[i]) {
						dock2[i] = -1;
					}
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	_goalp = nullptr;
	for (i = 0; i < ED_MAX_GOALS; i++) {
		_behavior[i] = behavior[i];
		_priority[i] = priority[i];
		_subsys[i] = subsys[i];
		_dock2[i] = dock2[i];
		_object[i] = data[i];
	}
}

void ShipGoalsDialogModel::setShip(int shipNum)
{
	_selfShip = shipNum;
}

int ShipGoalsDialogModel::getShip() const
{
	return _selfShip;
}

void ShipGoalsDialogModel::setWing(int wingNum)
{
	modify(_selfWing, wingNum);
}

int ShipGoalsDialogModel::getWing() const
{
	return _selfWing;
}

ai_goal* ShipGoalsDialogModel::getGoal() const
{
	return _goalp;
}

int ShipGoalsDialogModel::getValid(int pos) const
{
	return _valid[pos];
}

const ai_goal_list* ShipGoalsDialogModel::getGoalTypes()
{
	return Editor::getAi_goal_list();
}

int ShipGoalsDialogModel::getGoalsSize() const
{
	return _aiGoalListSize;
}

void ShipGoalsDialogModel::setBehavior(int index, int behavior)
{
	modify(_behavior[index], behavior);
}

int ShipGoalsDialogModel::getBehavior(int index) const
{
	return _behavior[index];
}

void ShipGoalsDialogModel::setObject(int index, int objNum)
{
	modify(_object[index], objNum);
}

int ShipGoalsDialogModel::getObject(int index) const
{
	return _object[index];
}

void ShipGoalsDialogModel::setSubsys(int index, const SCP_string& subsys)
{
	modify(_subsys[index], subsys);
}

SCP_string ShipGoalsDialogModel::getSubsys(int index) const
{
	return _subsys[index];
}

void ShipGoalsDialogModel::setDock(int index, long long dock)
{
	modify(_dock2[index], dock);
}

int ShipGoalsDialogModel::getDock(int index) const
{
	return _dock2[index];
}

void ShipGoalsDialogModel::setPriority(int index, int priority)
{
	modify(_priority[index], priority);
}

int ShipGoalsDialogModel::getPriority(int index) const
{
	return _priority[index];
}

} // namespace fso::fred::dialogs
