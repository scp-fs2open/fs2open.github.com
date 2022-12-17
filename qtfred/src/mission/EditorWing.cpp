#include "Editor.h"
#include "object.h"

#include "mission/dialogs/FormWingDialogModel.h"

#include <globalincs/linklist.h>
#include <ship/ship.h>

namespace {

const int MULTI_WING = 999999;

}

namespace fso {
namespace fred {

int Editor::delete_wing(int wing_num, int bypass)
{
	int i, r, total;

	if (already_deleting_wing) {
		return 0;
	}

	r = check_wing_dependencies(wing_num);
	if (r) {
		return r;
	}

	already_deleting_wing = 1;
	for (i = 0; i < Num_reinforcements; i++) {
		if (!stricmp(Wings[wing_num].name, Reinforcements[i].name)) {
			delete_reinforcement(i);
			break;
		}
	}

	invalidate_references(Wings[wing_num].name, SEXP_REF_TYPE::WING);
	if (!bypass) {
		total = Wings[wing_num].wave_count;
		for (i = 0; i < total; i++) {
			delete_object(wing_objects[wing_num][i]);
		}
	}

	Wings[wing_num].wave_count = 0;
	Wings[wing_num].wing_squad_filename[0] = '\0';
	Wings[wing_num].wing_insignia_texture = -1;

	if (cur_wing == wing_num) {
		set_cur_wing(-1);
	}

	free_sexp2(Wings[wing_num].arrival_cue);
	free_sexp2(Wings[wing_num].departure_cue);

	Num_wings--;
	missionChanged();

	update_custom_wing_indexes();

	already_deleting_wing = 0;
	return 0;
}
void Editor::set_cur_wing(int wing)
{
	cur_wing = wing;
	/*	if (cur_ship != -1)
			Assert(cur_wing == Ships[cur_ship].wingnum);
		if ((cur_object_index != -1) && (Objects[cur_object_index].type == OBJ_SHIP))
			Assert(cur_wing == Ships[Objects[cur_object_index].instance].wingnum);*/
	updateAllViewports();
	// TODO: Add notification for a changed selection
}
void Editor::update_custom_wing_indexes()
{
	int i;

	for (i = 0; i < MAX_STARTING_WINGS; i++) {
		Starting_wings[i] = wing_name_lookup(Starting_wing_names[i], 1);
	}

	for (i = 0; i < MAX_SQUADRON_WINGS; i++) {
		Squadron_wings[i] = wing_name_lookup(Squadron_wing_names[i], 1);
	}

	for (i = 0; i < MAX_TVT_WINGS; i++) {
		TVT_wings[i] = wing_name_lookup(TVT_wing_names[i], 1);
	}
}

int Editor::create_wing()
{
	char msg[1024];
	int i, ship, wing = -1, waypoints = 0, count = 0, illegal_ships = 0;
	int leader, leader_team;
	object* ptr;

	if (!query_valid_object(currentObject)) {
		return -1;
	}

	leader = currentObject;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			count++;
			i = -1;
			switch (ptr->type) {
			case OBJ_SHIP:
			case OBJ_START:
				i = Ships[ptr->instance].wingnum;
				break;
			}

			if (i >= 0) {
				if (wing < 0) {
					wing = i;
				} else if (wing != i) {
					wing = MULTI_WING;
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	if (count > MAX_SHIPS_PER_WING) {
		sprintf(msg,
			"You have too many ships marked!\n"
			"A wing is limited to %d ships total",
			MAX_SHIPS_PER_WING);

		_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", msg, {DialogButton::Ok});
		return -1;
	}

	if ((wing >= 0) && (wing != MULTI_WING)) {
		sprintf(msg, "Do you want to reform wing \"%s\"?", Wings[wing].name);
		auto button = _lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Question,
			"Query",
			msg,
			{DialogButton::Yes, DialogButton::No, DialogButton::Cancel});
		if (button == DialogButton::Cancel) {
			return -1;
		} else if (button == DialogButton::No) {
			wing = -1;
		} else { // must be IDYES
			for (i = Wings[wing].wave_count - 1; i >= 0; i--) {
				ptr = &Objects[wing_objects[wing][i]];
				switch (ptr->type) {
				case OBJ_SHIP:
					remove_ship_from_wing(ptr->instance, 0);
					break;

				case OBJ_START:
					remove_player_from_wing(ptr->instance, 0);
					break;

				default:
					Int3(); // shouldn't be in a wing!
				}
			}

			Assert(!Wings[wing].wave_count);
			Num_wings--;
		}

	} else {
		wing = -1;
	}

	if (wing < 0) {
		wing = find_free_wing();

		if (wing < 0) {
			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Error",
				"Too many wings, can't create more!",
				{DialogButton::Ok});

			return -1;
		}

		Wings[wing].clear();

		auto dlg = _lastActiveViewport->dialogProvider->createFormWingDialog();

		if (!_lastActiveViewport->dialogProvider->showModalDialog(dlg.get())) {
			return -1;
		}

		strcpy_s(Wings[wing].name, dlg->getModel()->getName().c_str());
	}

	setupCurrentObjectIndices(-1);
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			//			if ((ptr->type == OBJ_START) && (ptr->instance)) {
			//				starts++;
			//				unmark_object(OBJ_INDEX(ptr));

			//			} else if (ptr->type == OBJ_WAYPOINT) {
			if (ptr->type == OBJ_WAYPOINT) {
				waypoints++;
				unmarkObject(OBJ_INDEX(ptr));

			} else if (ptr->type == OBJ_SHIP) {
				int ship_type = ship_query_general_type(ptr->instance);
				if (ship_type < 0 || !(Ship_types[ship_type].flags[Ship::Type_Info_Flags::AI_can_form_wing])) {
					illegal_ships++;
					unmarkObject(OBJ_INDEX(ptr));
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	// if this wing is a player starting wing, automatically set the hotkey for this wing
	for (i = 0; i < MAX_STARTING_WINGS; i++) {
		if (!stricmp(Wings[wing].name, Starting_wing_names[i])) {
			Wings[wing].hotkey = i;
			break;
		}
	}

	count = 0;
	if (Objects[Ships[Player_start_shipnum].objnum].flags[Object::Object_Flags::Marked]) {
		count = 1;
	}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			if ((ptr->type == OBJ_START) && (ptr->instance == Player_start_shipnum)) {
				i = 0; // player 1 start always goes to front of the wing
			} else {
				i = count++;
			}

			Assert((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START));
			ship = ptr->instance;
			if (Ships[ship].wingnum != -1) {
				if (ptr->type == OBJ_SHIP) {
					remove_ship_from_wing(ship);
				} else {
					remove_player_from_wing(ptr->instance);
				}
			}

			wing_bash_ship_name(msg, Wings[wing].name, i + 1);
			rename_ship(ship, msg);

			Wings[wing].ship_index[i] = ship;
			Ships[ship].wingnum = wing;
			if (Ships[ship].arrival_cue >= 0) {
				free_sexp2(Ships[ship].arrival_cue);
			}

			Ships[ship].arrival_cue = Locked_sexp_false;
			if (Ships[ship].departure_cue >= 0) {
				free_sexp2(Ships[ship].departure_cue);
			}

			Ships[ship].departure_cue = Locked_sexp_false;

			wing_objects[wing][i] = OBJ_INDEX(ptr);
			if (OBJ_INDEX(ptr) == leader) {
				Wings[wing].special_ship = i;
			}
		}

		ptr = GET_NEXT(ptr);
	}

	if (!count) { // this should never happen, so if it does, needs to be fixed now.
		Error(LOCATION, "No valid ships were selected to form wing from");
	}

	Wings[wing].wave_count = count;
	Num_wings++;

	//	if (starts)
	//		Fred_main_wnd->MessageBox("Multi-player starting points can't be part of a wing!\n"
	//			"All marked multi-player starting points were ignored",
	//			"Error", MB_ICONEXCLAMATION);

	if (waypoints) {
		_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Error",
			"Waypoints can't be part of a wing!\n"
			"All marked waypoints were ignored",
			{DialogButton::Ok});
	}

	if (illegal_ships) {
		_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Error,
			"Error",
			"Some ship types aren't allowed to be in a wing.\n"
			"All marked ships of these types were ignored",
			{DialogButton::Ok});
	}

	leader_team = Ships[Wings[wing].ship_index[Wings[wing].special_ship]].team;
	for (i = 0; i < Wings[wing].wave_count; i++) {
		if (Ships[Wings[wing].ship_index[i]].team != leader_team) {
			_lastActiveViewport->dialogProvider->showButtonDialog(DialogType::Warning,
				"Warning",
				"Wing contains ships on different teams",
				{DialogButton::Ok});
			break;
		}
	}

	mark_wing(wing);

	update_custom_wing_indexes();

	return 0;
}

void Editor::remove_player_from_wing(int player, int min) { remove_ship_from_wing(player, min); }
void Editor::remove_ship_from_wing(int ship, int min)
{
	char buf[256];
	int i, wing, end, obj;

	wing = Ships[ship].wingnum;
	if (wing != -1) {
		if (Wings[wing].wave_count == min) {
			Wings[wing].wave_count = 0;
			Wings[wing].wing_squad_filename[0] = '\0';
			Wings[wing].wing_insignia_texture = -1;
			delete_wing(wing);
		} else {
			i = Wings[wing].wave_count;
			end = i - 1;
			while (i--) {
				if (wing_objects[wing][i] == Ships[ship].objnum) {
					break;
				}
			}

			Assert(i != -1); // Error, object should be in wing.
			if (Wings[wing].special_ship == i) {
				Wings[wing].special_ship = 0;
			}

			// if not last element, move last element to position to fill gap
			if (i != end) {
				obj = wing_objects[wing][i] = wing_objects[wing][end];
				Wings[wing].ship_index[i] = Wings[wing].ship_index[end];
				if (Objects[obj].type == OBJ_SHIP) {
					wing_bash_ship_name(buf, Wings[wing].name, i + 1);
					rename_ship(Wings[wing].ship_index[i], buf);
				}
			}

			Wings[wing].wave_count--;
			if (Wings[wing].wave_count && (Wings[wing].threshold >= Wings[wing].wave_count)) {
				Wings[wing].threshold = Wings[wing].wave_count - 1;
			}
		}

		Ships[ship].wingnum = -1;
	}

	missionChanged();
	// reset ship name to non-wing default ship name
	sprintf(buf, "%s %d", Ship_info[Ships[ship].ship_info_index].name, ship);
	rename_ship(ship, buf);
}
int Editor::find_free_wing()
{
	int i;

	for (i = 0; i < MAX_WINGS; i++) {
		if (!Wings[i].wave_count) {
			return i;
		}
	}

	return -1;
}
void Editor::mark_wing(int wing)
{
	int i;

	unmark_all();
	Assert(Wings[wing].special_ship >= 0 && Wings[wing].special_ship < Wings[wing].wave_count);
	setupCurrentObjectIndices(wing_objects[wing][Wings[wing].special_ship]);
	for (i = 0; i < Wings[wing].wave_count; i++) {
		markObject(wing_objects[wing][i]);
	}
}

bool Editor::query_single_wing_marked()
{
	int i, obj;

	if (!query_valid_object(currentObject))
		return false;

	if (cur_wing == -1)
		return false;

	i = Wings[cur_wing].wave_count;
	if (numMarked != i) // does marked object count match number of ships in wing?
		return false;

	while (i--) {
		obj = wing_objects[cur_wing][i];
		if ((Objects[obj].type != OBJ_SHIP) && (Objects[obj].type != OBJ_START))
			Error(LOCATION, "Invalid objects detected in wing \"%s\"", Wings[cur_wing].name);

		//		if (Ships[Objects[obj].instance].wingnum != cur_wing)
		//			return false;
		Assert(Ships[Objects[obj].instance].wingnum == cur_wing);
		if (!(Objects[obj].flags[Object::Object_Flags::Marked])) // ensure all ships in wing.are marked
			return false;
	}

	return true;
}

bool Editor::wing_is_player_wing(int wing)
{
	int i;

	if (wing < 0)
		return false;

	if (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) {
		for (i = 0; i < MAX_TVT_WINGS; i++) {
			if (wing == TVT_wings[i])
				return true;
		}
	} else {
		for (i = 0; i < MAX_STARTING_WINGS; i++) {
			if (wing == Starting_wings[i])
				return true;
		}
	}

	return false;
}
} // namespace fred
} // namespace fso
