/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/wing.cpp $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:32 $
 * $Author: Goober5000 $
 *
 * Wing management functions for dealing with wing related operations
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.11  2006/01/14 05:24:19  Goober5000
 * first pass at converting FRED to use new IFF stuff
 * --Goober5000
 *
 * Revision 1.10  2005/12/29 08:21:00  wmcoolmon
 * No my widdle FRED, I didn't forget about you ^_^ (codebase commit)
 *
 * Revision 1.9  2005/10/29 23:02:33  Goober5000
 * partial FRED commit of changes
 * --Goober5000
 *
 * Revision 1.8  2005/09/26 07:58:27  Goober5000
 * hooray, more custom wing fixes
 * --Goober5000
 *
 * Revision 1.7  2005/08/23 07:28:02  Goober5000
 * grammar
 * --Goober5000
 *
 * Revision 1.6  2005/04/13 20:11:06  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.5  2005/03/29 03:43:11  phreak
 * ai directory fixes as well as fixes for the new jump node code
 *
 * Revision 1.4  2003/01/06 20:49:15  Goober5000
 * FRED2 support for wing squad logos - look in the wing editor
 * --Goober5000
 *
 * Revision 1.3  2002/08/15 04:35:44  penguin
 * Changes to build with fs2_open code.lib
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:03  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 4     7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 3     3/01/99 10:00a Dave
 * Fxied several dogfight related stats bugs.
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:02p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 59    2/26/98 4:59p Allender
 * groundwork for team vs team briefings.  Moved weaponry pool into the
 * Team_data structure.  Added team field into the p_info structure.
 * Allow for mutliple structures in the briefing code.
 * 
 * 58    1/06/98 2:27p Hoffoss
 * Made wing deleting free up arrival and departure cue sexp trees, which
 * wasn't happening before for some reason.
 * 
 * 57    12/29/97 4:55p Johnson
 * Added some fixes.
 * 
 * 56    11/21/97 11:46a Johnson
 * Changed code to delete reinforcement wings when the wing is deleted.
 * 
 * 55    11/13/97 4:14p Allender
 * automatic assignment of hotkeys for starting wings.  Appripriate
 * warnings when they are incorrectly used.  hotkeys correctly assigned to
 * ships/wing arriving after mission start
 * 
 * 54    10/01/97 12:37p Hoffoss
 * Changed Fred (and FreeSpace) to utilize alpha, beta and gamma as player
 * starting wing names.
 * 
 * 53    9/16/97 9:41p Hoffoss
 * Changed Fred code around to stop using Parse_player structure for
 * player information, and use actual ships instead.
 * 
 * 52    9/11/97 4:04p Hoffoss
 * Fixed bug in Fred: disband wing wasn't making sure the ship editor's
 * data was synced.
 * 
 * 51    9/06/97 2:13p Mike
 * Replace support for TEAM_NEUTRAL
 * 
 * 50    9/02/97 3:44p Johnson
 * Fixed bugs with thresholds not being lowered.
 * 
 * 49    9/02/97 3:24p Johnson
 * Fixed bug: ships removed from wings will lower threshold if required.
 * 
 * 48    8/30/97 9:52p Hoffoss
 * Implemented arrival location, distance, and anchor in Fred.
 * 
 * 47    8/25/97 5:58p Hoffoss
 * Created menu items for keypress functions in Fred, and fixed bug this
 * uncovered with wing_delete function.
 * 
 * 46    8/16/97 6:44p Hoffoss
 * Changes to allow any player to be in a wing.
 * 
 * 45    8/16/97 4:51p Hoffoss
 * Fixed bugs with wing deletion and removing ships from a wing.
 * 
 * 44    8/15/97 1:07a Hoffoss
 * Changed code to disallow some ship types from being in a wing, and
 * disallowing some ship types from being able to have initial orders.
 * 
 * 43    8/12/97 1:55a Hoffoss
 * Made extensive changes to object reference checking and handling for
 * object deletion call.
 * 
 * 42    8/10/97 4:24p Hoffoss
 * Added warning when creating a wing if wing is mixed.
 * 
 * 41    8/08/97 1:31p Hoffoss
 * Added syncronization protection to cur_object_index changes.
 * 
 * 40    8/08/97 10:07a Hoffoss
 * Fixed bug where ship references aren't updated when wing is created
 * (i.e. ships being renamed, but not in the references too).
 * 
 * 39    8/01/97 3:24p Hoffoss
 * Fixed bug where when player is removed from a wing, the ship associated
 * with the player doesn't get its wingnum variable updated.
 * 
 * 38    7/09/97 2:38p Allender
 * organized ship/wing editor dialogs.  Added protect ship and ignore
 * count checkboxes to those dialogs.  Changed flag code for
 * parse_objects.  Added unprotect sexpressions
 * 
 * 37    6/30/97 11:36a Hoffoss
 * Fixed bug with wing reforming.
 * 
 * 36    6/18/97 2:36p Hoffoss
 * Wing ship numbering starts at 1 instead of 0, and changed form wing to
 * allow reforming a wing.
 * 
 * 35    6/04/97 12:00a Allender
 * make wing goal priorities default to -1 so that their priorities get
 * set more appropriatly in the initial order dialog
 * 
 * 34    5/14/97 4:08p Lawrance
 * removing my_index from game arrays
 * 
 * 33    5/08/97 10:54a Hoffoss
 * Fixed bug in reforming wings screwing up ship names.
 * 
 * 32    4/30/97 9:17a Hoffoss
 * Hotkey for wing set to none when new wing created.
 * 
 * 31    3/28/97 3:19p Hoffoss
 * Fixed bug.
 * 
 * 30    3/20/97 3:55p Hoffoss
 * Major changes to how dialog boxes initialize (load) and update (save)
 * their internal data.  This should simplify things and create less
 * problems.
 * 
 * 29    3/17/97 4:29p Hoffoss
 * Automated player's wing flaging as a starting player wing.
 * 
 * 28    3/12/97 12:40p Hoffoss
 * Fixed bugs in wing object management functions, several small additions
 * and rearrangements.
 * 
 * 27    3/04/97 6:27p Hoffoss
 * Changes to Fred to handle new wing structure.
 * 
 * 26    2/28/97 11:31a Hoffoss
 * Implemented modeless dialog saving and restoring, and changed some
 * variables names.
 * 
 * 25    2/27/97 3:16p Allender
 * major wing structure enhancement.  simplified wing code.  All around
 * better wing support
 * 
 * 24    2/27/97 2:31p Hoffoss
 * Changed wing create code to bash ship's cues to false.
 * 
 * 23    2/24/97 5:38p Hoffoss
 * Added dialog box to name a wing at creation, code to change ship names
 * to match wing name, and code to maintain these ship names.
 * 
 * 22    2/20/97 4:03p Hoffoss
 * Several ToDo items: new reinforcement clears arrival cue, reinforcement
 * control from ship and wing dialogs, show grid toggle.
 * 
 * 21    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
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
#define new DEBUG_NEW
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
					sprintf(buf, "%s %d", Wings[wing].name, i + 1);
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
		if ((ptr->type == OBJ_SHIP) && (ptr->flags & OF_MARKED)) {
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
		Wings[wing].arrival_cue = Locked_sexp_true;
		Wings[wing].departure_cue = Locked_sexp_false;
		Wings[wing].hotkey = -1;
		Wings[wing].flags = 0;

		for (i=0; i<MAX_AI_GOALS; i++) {
			Wings[wing].ai_goals[i].ai_mode = AI_GOAL_NONE;
			Wings[wing].ai_goals[i].priority = -1;				// this sets up the priority field to be like ships
		}

		if (dlg.DoModal() == IDCANCEL)
			return -1;

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

			sprintf(msg, "%s %d", Wings[wing].name, i + 1);
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
// JEH			strcpy(Wings[wingnum].ship_names[Wings[wingnum].count++], i;
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

//	Create a wing.
//	wing_type is the type of wing from the Wing_formations array to create.
//	leader_index is the index in Objects of the leader object.  This object must
//	have a position and an orientation.
//	*wingmen is a list of indices of existing ships to be added to the wing.
//	The wingmen list is terminated by -1.
//	max_size is the maximum number of ships to add to the wing
//	fill_flag is set if more ships are to be added to fill out the wing to max_size
void create_wing(int wing_type, int leader_index, int *wingmen, int max_size, int fill_flag)
{
	int			num_placed, num_vectors, cur_vec_index;
	object		*lobjp = &Objects[leader_index];
	formation	*wingp;
	object		*parent;
	int			wing_list[MAX_OBJECTS];
	matrix		rotmat;

	initialize_wings();

	Assert((wing_type >= 0) && (wing_type < MAX_WING_FORMATIONS));
	Assert(Wing_formations[wing_type].num_vectors > 0);
	Assert(Wing_formations[wing_type].num_vectors < MAX_WING_VECTORS);

	Assert(Objects[leader_index].type != OBJ_NONE);
	Assert(max_size < MAX_SHIPS_PER_WING);

	num_placed = 0;
	wingp = &Wing_formations[wing_type];
	num_vectors = wingp->num_vectors;
	cur_vec_index = 0;
	parent = lobjp;
	vm_copy_transpose_matrix(&rotmat, &lobjp->orient);

	while (num_placed < max_size) {
		vec3d	wvec;
		int		curobj;

		if (*wingmen == -1) {
			if (!fill_flag)
				break;
			else {
				curobj = get_free_objnum();
				Assert(curobj != -1);
				Objects[curobj].type = lobjp->type;
				Assert(Wings[cur_wing].wave_count < MAX_SHIPS_PER_WING);
// JEH		Wings[cur_wing].ship_list[Wings[cur_wing].count] = curobj;
				Wings[cur_wing].wave_count++;
			}
		} else
			curobj = *wingmen++;

		Objects[curobj] = *lobjp;
		vm_vec_rotate(&wvec, &wingp->offsets[cur_vec_index], &rotmat);
		cur_vec_index = (cur_vec_index + 1) % num_vectors;
		
		if (num_placed < num_vectors)
			parent = lobjp;
		else
			parent = &Objects[wing_list[num_placed - num_vectors]];
		
		wing_list[num_placed] = curobj;

		vm_vec_add(&Objects[curobj].pos, &parent->pos, &wvec);

		num_placed++;
	}

}

//	Create a wing.
//	cur_object_index becomes the leader.
//	If count == -1, then all objects of wing cur_wing get added to the wing.
//	If count == +n, then n objects are added to the wing.
void test_form_wing(int count)
{
	int	i, wingmen[MAX_OBJECTS];
	int	j, fill_flag;

	j = 0;

	Assert(cur_object_index != -1);
	Assert(Objects[cur_object_index].type != OBJ_NONE);
	Assert(get_wingnum(cur_object_index) != -1);
	get_wingnum(cur_object_index);

	wingmen[0] = -1;

	for (i=1; i<MAX_OBJECTS; i++)
		if ((get_wingnum(i) == cur_wing) && (Objects[i].type != OBJ_NONE))
			if (i != cur_object_index)
				wingmen[j++] = i;
	
	wingmen[j] = -1;

	fill_flag = 0;

	if (count > 0) {
		fill_flag = 1;
		j += count;
	}

	set_wingnum(cur_object_index, cur_wing);
	create_wing(0, cur_object_index, wingmen, j, fill_flag);
}
