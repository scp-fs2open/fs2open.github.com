// Autopilot.cpp
// Derek Meek
// 4-30-2004

/*
 * $Logfile: /Freespace2/code/Autopilot/Autopilot.cpp $
 * $Revision: 1.20 $
 * $Date: 2005-07-13 02:30:52 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.19  2005/04/19 23:03:42  wmcoolmon
 * Lock time compression when autopilot is engaged
 *
 * Revision 1.18  2005/04/05 05:53:14  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 1.17  2005/03/25 06:57:32  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 1.16  2005/03/03 06:05:26  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 1.15  2005/01/31 23:27:51  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 1.14  2004/10/03 21:41:10  Kazan
 * Autopilot convergence collision fix for ai_fly_to_ship() and ai_waypoints() -- mathematically expensive, only usable by autopilot
 *
 * Revision 1.13  2004/09/28 22:51:41  Kazan
 * fix autopilot formation bug
 *
 * Revision 1.12  2004/09/28 19:54:31  Kazan
 * | is binary or, || is boolean or - please use the right one
 * autopilot velocity ramping biasing
 * made debugged+k kill guardianed ships
 *
 * Revision 1.11  2004/07/29 23:41:21  Kazan
 * bugfixes
 *
 * Revision 1.10  2004/07/27 18:52:10  Kazan
 * squished another
 *
 * Revision 1.9  2004/07/27 18:04:09  Kazan
 * i love it when bugs go crunch (autopilot ai fixup)
 *
 * Revision 1.8  2004/07/26 20:47:24  Kazan
 * remove MCD complete
 *
 * Revision 1.7  2004/07/26 17:54:04  Kazan
 * Autopilot system completed -- i am dropping plans for GUI nav map
 * Fixed FPS counter during time compression
 *
 * Revision 1.6  2004/07/25 19:27:51  Kazan
 * only disable afterburning during AIM_WAYPOINTS and AIM_FLY_TO_SHIP while AutoPilotEngaged
 *
 * Revision 1.5  2004/07/25 18:46:28  Kazan
 * -fred_no_warn has become -no_warn and applies to both fred and fs2
 * added new ai directive (last commit) and disabled afterburners while performing AIM_WAYPOINTS or AIM_FLY_TO_SHIP
 * fixed player ship speed bug w/ player-use-ai, now stays in formation correctly and manages speed
 * made -radar_reduce ignore itself if no parameter is given (ignoring launcher bug)
 *
 * Revision 1.4  2004/07/25 00:31:27  Kazan
 * i have absolutely nothing to say about that subject
 *
 * Revision 1.3  2004/07/12 16:32:42  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.2  2004/07/01 16:38:18  Kazan
 * working on autonav
 *
 * Revision 1.1  2004/05/07 23:50:14  Kazan
 * Sorry Guys!
 *
 *
 *
 *
 */


#include "autopilot/autopilot.h"
#include "ai/ai.h"
#include "ai/aigoals.h"
#include "ship/ship.h"
#include "object/object.h"
#include "parse/sexp.h"
#include "freespace2/freespace.h"



// Extern functions/variables
extern int		Player_use_ai;
extern int sexp_distance2(int obj1, char *subj);
extern int ai_goal_find_empty_slot( ai_goal *goals );


// Module variables
bool AutoPilotEngaged;
int CurrentNav;
float ramp_bias;
NavPoint Navs[MAX_NAVPOINTS];

// used for ramping time compression;
int start_dist;

// ********************************************************************************************

void autopilot_ai_waypoint_goal_fixup(ai_goal* aigp)
{
	// this function sets wp_index properly;
	for (int i = 0; i < Num_waypoint_lists; i++)
	{
		if (!stricmp(aigp->ship_name, Waypoint_lists[i].name))
		{
			aigp->wp_index = i;
			return;
		}
	}
}


// ********************************************************************************************
bool Sel_NextNav()
{
	if (AutoPilotEngaged)
		return false;

	int i;
	if (CurrentNav == -1)
	{
		for (i = 0; i < MAX_NAVPOINTS; i++)
		{
			if (Navs[i].flags & NP_VALIDTYPE && !(Navs[i].flags & NP_NOSELECT))
			{
				CurrentNav=i;
				return true;
			}
		}		
	}
	else
	{
		for (i = CurrentNav+1; i < MAX_NAVPOINTS+CurrentNav; i++)
		{
			if (Navs[i%MAX_NAVPOINTS].flags & NP_VALIDTYPE && !(Navs[i%MAX_NAVPOINTS].flags & NP_NOSELECT))
			{
				if (i != CurrentNav)
				{
					CurrentNav=i%MAX_NAVPOINTS;
					return true;
				}
			}
		}
	}
	return false;
}


// ********************************************************************************************
vec3d NavPoint::GetPosition()
{
	vec3d position;

	if (flags & NP_WAYPOINT)
	{
		position = ((waypoint_list*)target_obj)->waypoints[waypoint_num-1];
	}
	else
	{
		position = Objects[((ship*)target_obj)->objnum].pos;
	}

	return position;
}

char* NavPoint::GetInteralName()
{
	char *NavName;
	char strtmp[3];

	if (flags & NP_WAYPOINT)
	{
		NavName = new char[strlen(((waypoint_list*)target_obj)->name)+5];
		memset(NavName, 0, strlen(((waypoint_list*)target_obj)->name)+5);
		strcpy(NavName, ((waypoint_list*)target_obj)->name);

		strcat(NavName, ":");
		strcat(NavName, itoa(waypoint_num, strtmp, 10));
	}
	else
	{		
		NavName = new char[strlen(((ship*)target_obj)->ship_name)+1];
		memset(NavName, 0, strlen(((ship*)target_obj)->ship_name)+1);
		strcpy(NavName, ((ship*)target_obj)->ship_name);
	}

	return NavName;
}

// ********************************************************************************************
// Tell us is autopilot is allow
// This needs:
//        * Nav point selected
//        * No enemies within X distance
//        * Destination > 1,000 meters away
bool CanAutopilot()
{
	bool CanAuto = true;
	CanAuto = CanAuto && CurrentNav != -1;

	int dist = sexp_distance2(Player_ship->objnum, "<any hostile>");

	CanAuto = CanAuto && (5000 <= dist || dist == SEXP_NAN);
	

	if (CanAuto) // protect against no currentNav
	{
		char *name = Navs[CurrentNav].GetInteralName();
		CanAuto = CanAuto && (1000 <= sexp_distance2(Player_ship->objnum, name));
		delete[] name;
	}

	return CanAuto;
}

// ********************************************************************************************
// Engages autopilot
// This does:
//        * Control switched from player to AI
//        * Time compression to 32x
//        * Lock time compression -WMC
//        * Tell AI to fly to targetted Nav Point (for all nav-status wings/ships)
//		  * Sets max waypoint speed to the best-speed of the slowest ship tagged
void StartAutopilot()
{
	if (!CanAutopilot())
		return;

	AutoPilotEngaged = true;

	Player_use_ai = 1;
	set_time_compression(1);
	lock_time_compression(true);

	// determine speed cap
	int i,j;
	float speed_cap = 1000000.0; // 1m is a safe starting point

	for (i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && 
				(Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
					(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY)
				)
			)
		{
			if (speed_cap > Ship_info[Ships[i].ship_info_index].max_vel.xyz.z)
				speed_cap = Ship_info[Ships[i].ship_info_index].max_vel.xyz.z;
		}
	}


	// damp speed_cap to 90% of actual -- to make sure ships stay in formation
	speed_cap = 0.90f * speed_cap;
	ramp_bias = speed_cap/50.0f;

	// assign ship goals
	// when assigning goals to individual ships only do so if Ships[shipnum].wingnum != -1 
	// we will assign wing goals below
	

	for (i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && 
				(Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
					(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY)
				)
			)
		{

			// clear the ship goals and cap the waypoint speed
			ai_clear_ship_goals(&Ai_info[Ships[i].ai_index]);
			Ai_info[Ships[i].ai_index].waypoint_speed_cap = (int)speed_cap;

			
			// if they're not part of a wing set their goal
			if (Ships[i].wingnum == -1)
			{
				if (Navs[CurrentNav].flags & NP_WAYPOINT)
				{
					
					ai_add_ship_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_WAYPOINTS_ONCE, -1, ((waypoint_list*)Navs[CurrentNav].target_obj)->name, &Ai_info[Ships[i].ai_index] );
					
					//fixup has to wait until after wing goals
				}
				else
				{
					ai_add_ship_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_FLY_TO_SHIP, -1, ((ship*)Navs[CurrentNav].target_obj)->ship_name, &Ai_info[Ships[i].ai_index] );
				}

			}
		}
	}

	// assign wing goals
	for (i = 0; i < MAX_WINGS; i++)
	{
		if (Wings[i].flags & WF_NAV_CARRY )
		{	
			//ai_add_ship_goal_player( int type, int mode, int submode, char *shipname, ai_info *aip );

			//ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_STAY_NEAR_SHIP, 0, target_shipname, wingnum );
			//ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_WAYPOINTS_ONCE, 0, target_shipname, wingnum );
			//ai_clear_ship_goals( &(Ai_info[Ships[num].ai_index]) );
			
			ai_clear_wing_goals( i );
			if (Navs[CurrentNav].flags & NP_WAYPOINT)
			{
				
				ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_WAYPOINTS_ONCE, AIF_FORMATION_WING, ((waypoint_list*)Navs[CurrentNav].target_obj)->name, i );
				

				// "fix up" the goal
				for (j = 0; j < MAX_AI_GOALS; j++)
				{
					if (Wings[i].ai_goals[j].ai_mode == AI_GOAL_WAYPOINTS_ONCE ||
						Wings[i].ai_goals[j].ai_mode == AIM_WAYPOINTS )
					{
						autopilot_ai_waypoint_goal_fixup(&(Wings[i].ai_goals[j]));
					}
				}
			}
			else
			{
				ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_FLY_TO_SHIP, AIF_FORMATION_WING, ((ship*)Navs[CurrentNav].target_obj)->ship_name, i );

			}
		}
	}

	// fixup has to go down here because ships are assigned goals during wing goals as well
	for (i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1)
		{
			if (Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
				(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY))
				for (j = 0; j < MAX_AI_GOALS; j++)
				{
					if (Ai_info[Ships[i].ai_index].goals[j].ai_mode == AI_GOAL_WAYPOINTS_ONCE ||
						Ai_info[Ships[i].ai_index].goals[j].ai_mode == AIM_WAYPOINTS)
					{
						autopilot_ai_waypoint_goal_fixup( &(Ai_info[Ships[i].ai_index].goals[j]) );

						
						// formation fixup
						//ai_form_on_wing(&Objects[Ships[i].objnum], &Objects[Player_ship->objnum]);
					}
				}
		}
	}

	start_dist = DistanceTo(CurrentNav);

}

// ********************************************************************************************
// Checks if autopilot should automatically die
// Returns true if:
//         * Targetted waypoint < 10,000 meters away
//         * Enemy < 10,000 meters
bool Autopilot_AutoDiable()
{
	return !CanAutopilot();
}

// ********************************************************************************************
// Disengages autopilot
// this does:
//         * Time compression to 1x
//         * Delete AI nav goal
//         * Control switched from AI to player
void EndAutoPilot()
{
	AutoPilotEngaged = false;

	set_time_compression(1);
	lock_time_compression(false);
	Player_use_ai = 0;
	//Clear AI Goals

	// assign ship goals
	// when assigning goals to individual ships only do so if Ships[shipnum].wingnum != -1 
	// we will assign wing goals below
	int i,j;

	for (i = 0; i < MAX_SHIPS; i++)

	{
		if (Ships[i].objnum != -1 && 
			(
				Ships[i].flags2 & SF2_NAVPOINT_CARRY || 
				(Ships[i].wingnum != -1 && Wings[Ships[i].wingnum].flags & WF_NAV_CARRY )
			 )
		   )
		{
			Ai_info[Ships[i].ai_index].waypoint_speed_cap = -1; // uncap their speed

			// old "dumb" routine
			//ai_clear_ship_goals( &(Ai_info[Ships[i].ai_index]) );
			for (j = 0; j < MAX_AI_GOALS; j++)
			{
				if (Ai_info[Ships[i].ai_index].goals[j].ai_mode == AI_GOAL_WAYPOINTS_ONCE ||
					Ai_info[Ships[i].ai_index].goals[j].ai_mode == AI_GOAL_FLY_TO_SHIP ||
					Ai_info[Ships[i].ai_index].goals[j].ai_mode == AIM_WAYPOINTS ||
					Ai_info[Ships[i].ai_index].goals[j].ai_mode == AIM_FLY_TO_SHIP)
				{
					ai_remove_ship_goal( &(Ai_info[Ships[i].ai_index]), j );
				}
			}
		}
	}

	// assign wing goals
	for (i = 0; i < MAX_WINGS; i++)
	{
		if (Wings[i].flags & WF_NAV_CARRY )
		{
			// old "dumb" routine
			//ai_clear_wing_goals( i );
			for (j = 0; j < MAX_AI_GOALS; j++)
			{
				if (Wings[i].ai_goals[j].ai_mode == AI_GOAL_WAYPOINTS_ONCE ||
					Wings[i].ai_goals[j].ai_mode == AI_GOAL_FLY_TO_SHIP ||
					Wings[i].ai_goals[j].ai_mode == AIM_WAYPOINTS ||
					Wings[i].ai_goals[j].ai_mode == AIM_FLY_TO_SHIP)
				{
					Wings[i].ai_goals[j].ai_mode = AI_GOAL_NONE;
					Wings[i].ai_goals[j].signature = -1;
					Wings[i].ai_goals[j].priority = -1;
				}
			}
		}
	}
}

// ********************************************************************************************
// Checks for changes every NPS_TICKRATE milliseconds
// Checks:
//			* if we've gotten close enough to a nav point for it to be counted as "Visited"
//			* If we're current AutoNavigating it checks if we need to autodisengage
void NavSystem_Do()
{
	static unsigned int last_update = 0;

	if (clock () - last_update > NPS_TICKRATE)
	{
		if (AutoPilotEngaged)
			if (Autopilot_AutoDiable())
				EndAutoPilot();

		// check if a NavPoints target has left, delete it if so
		int i;

		for (i = 0; i < MAX_NAVPOINTS; i++)
		{
			if ((Navs[i].flags & NP_SHIP) && (Navs[i].target_obj != NULL))
			{
				if (((ship*)Navs[i].target_obj)->objnum == -1)
					DelNavPoint(i);
			}
		}
		
		// check if we're reached a Node
		for (i = 0; i < MAX_NAVPOINTS; i++)
		{
			if (Navs[i].target_obj != NULL)
			{
				if (Navs[i].flags & NP_VALIDTYPE && DistanceTo(i) < 1000)
					Navs[i].flags |= NP_VISITED;
			}
		}
	}

	// ramp time compression
	if (AutoPilotEngaged)
	{
		int dstfrm_start = start_dist - DistanceTo(CurrentNav);

		// Ramp UP time compression
		if (dstfrm_start < (3500*ramp_bias))
		{

			if (dstfrm_start >= (3000*ramp_bias) && DistanceTo(CurrentNav) > 30000)
				set_time_compression(64);
			else if (dstfrm_start >= (2000*ramp_bias))
				set_time_compression(32);
			else if (dstfrm_start >= (1600*ramp_bias))
				set_time_compression(16);
			else if (dstfrm_start >= (1200*ramp_bias))
				set_time_compression(8);
			else if (dstfrm_start >= (800*ramp_bias))
				set_time_compression(4);
			else if (dstfrm_start >= (400*ramp_bias))
				set_time_compression(2);
		}

		// Ramp DOWN time compression
		if (DistanceTo(CurrentNav) <= (7000*ramp_bias))
		{
			int dist = DistanceTo(CurrentNav);
			if (dist >= (5000*ramp_bias))
				set_time_compression(32);
			else if (dist >= (4000*ramp_bias))
				set_time_compression(16);
			else if (dist >= (3000*ramp_bias))
				set_time_compression(8);
			else if (dist >= (2000*ramp_bias))
				set_time_compression(4);
			else if (dist >= (1000*ramp_bias))
				set_time_compression(2);
		}
	}
}


// ********************************************************************************************
// Inits the Nav System
void NavSystem_Init()
{
	memset((char *)&Navs, 0, sizeof(Navs));
	AutoPilotEngaged = false;
	CurrentNav = -1;
}

// ********************************************************************************************
// Finds a Nav point by name
int FindNav(char *Nav)
{
	for (int i = 0; i < MAX_NAVPOINTS; i++)
	{
		if (!stricmp(Navs[i].NavName, Nav))
			return i;
	}

	return -1;
}

// ********************************************************************************************
// Set A Nav point to "ZERO"
void ZeroNav(int i)
{
	memset((char *)&Navs[i], 0, sizeof(NavPoint));
}

// ********************************************************************************************
// Removes a Nav
bool DelNavPoint(char *Nav)
{
	int n = FindNav(Nav);

	return DelNavPoint(n);

}


bool DelNavPoint(int nav)
{	
	if (nav != -1)
	{
		if (nav != MAX_NAVPOINTS-1)
		{
			for (int i = nav; i < MAX_NAVPOINTS-1; i++)
			{
				Navs[nav] = Navs[nav+1];
			}

		}
		ZeroNav(MAX_NAVPOINTS-1);
		return true;
	}

	return false;
}

// ********************************************************************************************
// adds a Nav
bool AddNav_Ship(char *Nav, char *TargetName, int flags)
{
	// find an empty nav - should be the end

	int empty = -1;
	int i;

	for (i = 0; i < MAX_NAVPOINTS && empty == -1; i++)
	{
		if (Navs[i].flags == 0)
			empty = i;
	}

	if (empty == -1) // no empty navpoint slots
		return false;

	// Create the NavPoint struct
	NavPoint tnav;

	strncpy(tnav.NavName, Nav, 32);
	tnav.flags = NP_SHIP | flags;

	Assert(!(tnav.flags & NP_WAYPOINT));


	for (i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].objnum != -1 && !stricmp(TargetName, Ships[i].ship_name))
		{
			tnav.target_obj = (void *)&Ships[i];		
		}
	}

	tnav.waypoint_num = 0; // unused for NP_SHIP


	// copy it into it's location
	Navs[empty] = tnav;

	return true;
}


bool AddNav_Waypoint(char *Nav, char *WP_Path, int node, int flags)
{
	// find an empty nav - should be the end

	int empty = -1;
	int i;

	if (node == 0)
		node = 1;

	for (i = 0; i < MAX_NAVPOINTS && empty == -1; i++)
	{
		if (Navs[i].flags == 0)
			empty = i;
	}

	if (empty == -1) // no empty navpoint slots
		return false;

	// Create the NavPoint struct
	NavPoint tnav;

	strncpy(tnav.NavName, Nav, 32);
	tnav.flags = NP_WAYPOINT | flags;

	Assert(!(tnav.flags & NP_SHIP));


	for (i = 0; i < Num_waypoint_lists; i++)
	{
		if (!stricmp(WP_Path, Waypoint_lists[i].name))
		{
			tnav.target_obj = (void *)&Waypoint_lists[i];		
		}
	}

	tnav.waypoint_num = node;

	// copy it into it's location
	Navs[empty] = tnav;

	return true;
}

// ********************************************************************************************
//Change Flags
bool Nav_Alt_Flags(char *Nav, int flags)
{
	flags &= ~NP_VALIDTYPE; //clear the NP_SHIP and NP_WAYPOINT bits, make sure they haven't been set

	int nav = FindNav(Nav);

	if (nav != -1)
	{
		Navs[nav].flags &= NP_VALIDTYPE; // Clear all bits BUT NP_SHIP or NO_WAYPOINT
		Navs[nav].flags |= flags; // merge
	}

	return (nav != -1);
}

//Get Flags
int Nav_Get_Flags(char *Nav)
{
	int flags = 0;

	int nav = FindNav(Nav);

	if (nav != -1)
		flags = Navs[nav].flags;

	return flags;
}

// ********************************************************************************************
// Sexp Accessors

//Generic
bool Nav_Set_Flag(char *Nav, int flag)
{
	Assert(!(flag & NP_VALIDTYPE));
	int nav = FindNav(Nav);

	if (nav != -1)
	{
		Navs[nav].flags |= flag;
		return true;
	}

	return false;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_UnSet_Flag(char *Nav, int flag)
{
	Assert(!(flag & NP_VALIDTYPE));

	int nav = FindNav(Nav);

	if (nav != -1)
	{
		Navs[nav].flags &= ~flag;
		return true;
	}

	return false;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//Named
bool Nav_Set_Hidden(char *Nav)
{
	return Nav_Set_Flag(Nav, NP_HIDDEN);

}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_Set_NoAccess(char *Nav)
{
	return Nav_Set_Flag(Nav, NP_NOACCESS);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_Set_Visited(char *Nav)
{
	
	return Nav_Set_Flag(Nav, NP_VISITED);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_UnSet_Hidden(char *Nav)
{
	return Nav_UnSet_Flag(Nav, NP_HIDDEN);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_UnSet_NoAccess(char *Nav)
{
	return Nav_UnSet_Flag(Nav, NP_NOACCESS);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool Nav_UnSet_Visited(char *Nav)
{
	return Nav_UnSet_Flag(Nav, NP_VISITED);
}



//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


unsigned int DistanceTo(char *nav)
{
	int n = FindNav(nav);

	return DistanceTo(n);
}

unsigned int DistanceTo(int nav)
{
	if (nav > MAX_NAVPOINTS && nav < 0)
		return 0xFFFFFFFF;

	char *name = Navs[nav].GetInteralName();

	if (name == NULL)
		return 0xFFFFFFFF;

	int distance = sexp_distance2(Player_ship->objnum, name);
	delete[] name;

	if (distance == SEXP_NAN)
		return 0xFFFFFFFF;
	return distance;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

bool IsVisited(char *nav)
{
	int n = FindNav(nav);

	return IsVisited(n);
}

bool IsVisited(int nav)
{
	if (nav > MAX_NAVPOINTS && nav < 0)
		return 0;

	if (Navs[nav].flags & NP_VISITED)
		return 1;
	return 0;
}

