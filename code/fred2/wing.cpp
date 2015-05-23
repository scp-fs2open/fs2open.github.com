/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "FREDView.h"
#include "render/3d.h"
#include "physics/physics.h"
#include "object/object.h"
#include "editor.h"
#include "ai/ailocal.h"
#include "ship/ship.h"
#include "math/vecmat.h"
#include "Management.h"
#include "globalincs/linklist.h"
#include "MainFrm.h"
#include "wing.h"
#include "CreateWingDlg.h"
#include "Management.h"

#define MULTI_WING	999999

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int already_deleting_wing = 0;

void remove_player_from_wing(int player, int min = 1);

// Finds a free wing slot (i.e. unused)
int find_free_wing()
{
	int i;

	for (i=0; i<MAX_WINGS; i++)
		if (!Wings[i].wave_count)
			return i;

	return -1;
}

int check_wing_dependencies(int wing_num)
{
	char *name;

	name = Wings[wing_num].name;
	return reference_handler(name, REF_TYPE_WING, -1);
}

void mark_wing(int wing)
{
	int i;

	unmark_all();
	Assert(Wings[wing].special_ship >= 0 && Wings[wing].special_ship < Wings[wing].wave_count);
	set_cur_object_index(wing_objects[wing][Wings[wing].special_ship]);
	for (i=0; i<Wings[wing].wave_count; i++)
		mark_object(wing_objects[wing][i]);
}

// delete a whole wing, also deleting its ships if necessary.
int delete_wing(int wing_num, int bypass)
{
	int i, r, total;

	if (already_deleting_wing)
		return 0;

	r = check_wing_dependencies(wing_num);
	if (r)
		return r;

	already_deleting_wing = 1;
	for (i=0; i<Num_reinforcements; i++)
		if (!stricmp(Wings[wing_num].name, Reinforcements[i].name)) {
			delete_reinforcement(i);
			break;
		}

	invalidate_references(Wings[wing_num].name, REF_TYPE_WING);
	if (!bypass) {
		total = Wings[wing_num].wave_count;
		for (i=0; i<total; i++)
			delete_object(wing_objects[wing_num][i]);
	}

	Wings[wing_num].wave_count = 0;
	Wings[wing_num].wing_squad_filename[0] = '\0';
	Wings[wing_num].wing_insignia_texture = -1;

	if (cur_wing == wing_num)
		set_cur_wing(cur_wing = -1);  // yes, one '=' is correct.

	free_sexp2(Wings[wing_num].arrival_cue);
	free_sexp2(Wings[wing_num].departure_cue);

	Num_wings--;
	set_modified();

	update_custom_wing_indexes();

	already_deleting_wing = 0;
	return 0;
}

// delete a whole wing, leaving ships intact but wingless.
void remove_wing(int wing_num)
{
	int i, total;
	object *ptr;

	if (check_wing_dependencies(wing_num))
		return;

	Ship_editor_dialog.bypass_errors = Wing_editor_dialog.bypass_errors = 1;
	Ship_editor_dialog.update_data(0);
	total = Wings[wing_num].wave_count;
	for (i=0; i<total; i++) {
		ptr = &Objects[wing_objects[wing_num][i]];
		if (ptr->type == OBJ_SHIP)
			remove_ship_from_wing(ptr->instance, 0);
		else if (ptr->type == OBJ_START)
			remove_player_from_wing(ptr->instance, 0);
	}

	Assert(!Wings[wing_num].wave_count);

	Wings[wing_num].wave_count = 0;
	Wings[wing_num].wing_squad_filename[0] = '\0';
	Wings[wing_num].wing_insignia_texture = -1;

	Ship_editor_dialog.initialize_data(1);
	Ship_editor_dialog.bypass_errors = Wing_editor_dialog.bypass_errors = 0;

	if (cur_wing == wing_num) {
		set_cur_wing(cur_wing = -1);  // yes, one '=' is correct.
	}

	free_sexp2(Wings[wing_num].arrival_cue);
	free_sexp2(Wings[wing_num].departure_cue);

	Num_wings--;

	update_custom_wing_indexes();

	set_modified();
}

// Takes a ship out of a wing, deleting wing if that was the only ship in it.
void remove_ship_from_wing(int ship, int min)
{
	char buf[256];
	int i, wing, end, obj;

	wing = Ships[ship].wingnum;
	if (wing != -1) {
		if (Wings[wing].wave_count == min)
		{
			Wings[wing].wave_count = 0;
			Wings[wing].wing_squad_filename[0] = '\0';
			Wings[wing].wing_insignia_texture = -1;
			delete_wing(wing);
		}
		else
		{
			i = Wings[wing].wave_count;
			end = i - 1;
			while (i--)
				if (wing_objects[wing][i] == Ships[ship].objnum)
					break;

			Assert(i != -1);  // Error, object should be in wing.
			if (Wings[wing].special_ship == i)
				Wings[wing].special_ship = 0;

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
			if (Wings[wing].wave_count && (Wings[wing].threshold >= Wings[wing].wave_count))
				Wings[wing].threshold = Wings[wing].wave_count - 1;
		}

		Ships[ship].wingnum = -1;
	}

	set_modified();
	// reset ship name to non-wing default ship name
	sprintf(buf, "%s %d", Ship_info[Ships[ship].ship_info_index].name, ship);
	rename_ship(ship, buf);
}

// Takes a player out of a wing, deleting wing if that was the only ship in it.
void remove_player_from_wing(int player, int min)
{
	remove_ship_from_wing(player, min);
}

// Forms a wing from marked objects
int create_wing()
{
	char msg[1024];
	int i, ship, wing = -1, waypoints = 0, count = 0, illegal_ships = 0;
	int leader, leader_team;
	object *ptr;
	create_wing_dlg dlg;

	if (!query_valid_object())
		return -1;

	leader = cur_object_index;
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (( (ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START) ) && (ptr->flags & OF_MARKED)) {
			count++;
			i = -1;
			switch (ptr->type) {
				case OBJ_SHIP:
				case OBJ_START:
					i = Ships[ptr->instance].wingnum;
					break;
			}

			if (i >= 0) {
				if (wing < 0)
					wing = i;
				else if (wing != i)
					wing = MULTI_WING;
			}
		}

		ptr = GET_NEXT(ptr);
	}

	if (count > MAX_SHIPS_PER_WING) {
		sprintf(msg, "You have too many ships marked!\n"
			"A wing is limited to %d ships total", MAX_SHIPS_PER_WING);

		Fred_main_wnd->MessageBox(msg, "Error", MB_ICONEXCLAMATION);
		return -1;
	}

	if ((wing >= 0) && (wing != MULTI_WING)) {
		sprintf(msg, "Do you want to reform wing \"%s\"?", Wings[wing].name);
		i = Fred_main_wnd->MessageBox(msg, "Query", MB_YESNOCANCEL);
		if (i == IDCANCEL)
			return -1;

		else if (i == IDNO)
			wing = -1;

		else {  // must be IDYES
			for (i=Wings[wing].wave_count-1; i>=0; i--) {
				ptr = &Objects[wing_objects[wing][i]];
				switch (ptr->type) {
					case OBJ_SHIP:
						remove_ship_from_wing(ptr->instance, 0);
						break;

					case OBJ_START:
						remove_player_from_wing(ptr->instance, 0);
						break;

					default:
						Int3();  // shouldn't be in a wing!
				}
			}

			Assert(!Wings[wing].wave_count);
			Num_wings--;
		}

	} else
		wing = -1;

	if (wing < 0) {
		wing = find_free_wing();

		if (wing < 0) {
			Fred_main_wnd->MessageBox("Too many wings, can't create more!",
				"Error", MB_ICONEXCLAMATION);

			return -1;
		}

		Wings[wing].num_waves = 1;
		Wings[wing].threshold = 0;
		Wings[wing].arrival_location = Wings[wing].departure_location = 0;
		Wings[wing].arrival_distance = 0;
		Wings[wing].arrival_anchor = -1;
		Wings[wing].arrival_delay = 0;
		Wings[wing].arrival_cue = Locked_sexp_true;
		Wings[wing].departure_delay = 0;
		Wings[wing].departure_cue = Locked_sexp_false;
		Wings[wing].hotkey = -1;
		Wings[wing].flags = 0;
		Wings[wing].wave_delay_min = 0;
		Wings[wing].wave_delay_max = 0;

		for (i=0; i<MAX_AI_GOALS; i++) {
			Wings[wing].ai_goals[i].ai_mode = AI_GOAL_NONE;
			Wings[wing].ai_goals[i].priority = -1;				// this sets up the priority field to be like ships
		}

		if (dlg.DoModal() == IDCANCEL)
			return -1;

		dlg.m_name.TrimLeft();
		dlg.m_name.TrimRight();
		string_copy(Wings[wing].name, dlg.m_name, NAME_LENGTH - 1);
	}

	set_cur_indices(-1);
	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags & OF_MARKED) {
//			if ((ptr->type == OBJ_START) && (ptr->instance)) {
//				starts++;
//				unmark_object(OBJ_INDEX(ptr));

//			} else if (ptr->type == OBJ_WAYPOINT) {
			if (ptr->type == OBJ_WAYPOINT) {
				waypoints++;
				unmark_object(OBJ_INDEX(ptr));

			} else if (ptr->type == OBJ_SHIP) {
				int ship_type = ship_query_general_type(ptr->instance);
				if(ship_type < 0 || !(Ship_types[ship_type].ai_bools & STI_AI_CAN_FORM_WING))
				{
					illegal_ships++;
					unmark_object(OBJ_INDEX(ptr));
				}
			}
		}

		ptr = GET_NEXT(ptr);
	}

	// if this wing is a player starting wing, automatically set the hotkey for this wing
	for (i = 0; i < MAX_STARTING_WINGS; i++ ) {
		if ( !stricmp(Wings[wing].name, Starting_wing_names[i]) ) {
			Wings[wing].hotkey = i;
			break;
		}
	}

	count = 0;
	if (Objects[Ships[Player_start_shipnum].objnum].flags & OF_MARKED)
		count = 1;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags & OF_MARKED) {
			if ((ptr->type == OBJ_START) && (ptr->instance == Player_start_shipnum))
				i = 0;  // player 1 start always goes to front of the wing
			else
				i = count++;

			Assert((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START));
			ship = ptr->instance;
			if (Ships[ship].wingnum != -1) {
				if (ptr->type == OBJ_SHIP)
					remove_ship_from_wing(ship);
				else
					remove_player_from_wing(ptr->instance);
			}

			wing_bash_ship_name(msg, Wings[wing].name, i + 1);
			rename_ship(ship, msg);

			Wings[wing].ship_index[i] = ship;
			Ships[ship].wingnum = wing;
			if (Ships[ship].arrival_cue >= 0)
				free_sexp2(Ships[ship].arrival_cue);

			Ships[ship].arrival_cue = Locked_sexp_false;
			if (Ships[ship].departure_cue >= 0)
				free_sexp2(Ships[ship].departure_cue);

			Ships[ship].departure_cue = Locked_sexp_false;

			wing_objects[wing][i] = OBJ_INDEX(ptr);
			if (OBJ_INDEX(ptr) == leader)
				Wings[wing].special_ship = i;
		}

		ptr = GET_NEXT(ptr);
	}

	if (!count)  // this should never happen, so if it does, needs to be fixed now.
		Error(LOCATION, "No valid ships were selected to form wing from");

	Wings[wing].wave_count = count;
	Num_wings++;

//	if (starts)
//		Fred_main_wnd->MessageBox("Multi-player starting points can't be part of a wing!\n"
//			"All marked multi-player starting points were ignored",
//			"Error", MB_ICONEXCLAMATION);

	if (waypoints)
		Fred_main_wnd->MessageBox("Waypoints can't be part of a wing!\n"
			"All marked waypoints were ignored",
			"Error", MB_ICONEXCLAMATION);

	if (illegal_ships)
		Fred_main_wnd->MessageBox("Some ship types aren't allowed to be in a wing.\n"
			"All marked ships of these types were ignored",
			"Error", MB_ICONEXCLAMATION);


	leader_team = Ships[Wings[wing].ship_index[Wings[wing].special_ship]].team;
	for (i = 0; i < Wings[wing].wave_count; i++)
	{
		if (Ships[Wings[wing].ship_index[i]].team != leader_team)
		{
			Fred_main_wnd->MessageBox("Wing contains ships on different teams", "Warning");
			break;
		}
	}

	mark_wing(wing);

	update_custom_wing_indexes();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// Old stuff down there..

#define	MAX_WING_VECTORS	8	//	8 vectors per wing formation.  Possible to have more
								//	than 8 ships.  A vector is where a ship is located relative
								//	to the leader.  Other ships can be located at the same
								//	vector relative to another member.  So wing formation
								//	size is not limited by this constant.
#define	MAX_WING_FORMATIONS	8	//	8 different kinds of wing formations

typedef struct formation {
	int		num_vectors;
	vec3d	offsets[MAX_WING_VECTORS];
} formation;

formation Wing_formations[MAX_WING_FORMATIONS];

//wing	Wings[MAX_WINGS];

int	Wings_initialized = 0;

void initialize_wings(void)
{
	if (Wings_initialized)
		return;

	Wings_initialized = 1;

	Wing_formations[0].num_vectors = 2;
	
	Wing_formations[0].offsets[0].xyz.x = -5.0f;
	Wing_formations[0].offsets[0].xyz.y = +1.0f;
	Wing_formations[0].offsets[0].xyz.z = -5.0f;

	Wing_formations[0].offsets[1].xyz.x = +5.0f;
	Wing_formations[0].offsets[1].xyz.y = +1.0f;
	Wing_formations[0].offsets[1].xyz.z = -5.0f;
}

void create_wings_from_objects(void)
{
	int	i;

	for (i=0; i<MAX_WINGS; i++)
		Wings[i].wave_count= 0;

	for (i=0; i<MAX_OBJECTS; i++)
		if (Objects[i].type != OBJ_NONE)
			if (get_wingnum(i) != -1) {
				int	wingnum = get_wingnum(i);
				
				Assert((wingnum >= 0) && (wingnum < MAX_WINGS));
				Assert(Wings[wingnum].wave_count < MAX_SHIPS_PER_WING);
// JEH			strcpy_s(Wings[wingnum].ship_names[Wings[wingnum].count++], i;
			}

}

int get_free_objnum(void)
{
	int	i;

	for (i=1; i<MAX_OBJECTS; i++)
		if (Objects[i].type == OBJ_NONE)
			return i;

	return -1;
}
