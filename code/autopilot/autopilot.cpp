// Autopilot.cpp
// Derek Meek
// 4-30-2004

/*
 * $Logfile: /Freespace2/code/Autopilot/Autopilot.cpp $
 * $Revision: 1.2 $
 * $Date: 2004-07-01 16:38:18 $
 * $Author: Kazan $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/05/07 23:50:14  Kazan
 * Sorry Guys!
 *
 *
 *
 *
 */


#include "Autopilot.h"
#include "ship/ai.h"
#include "ship/aigoals.h"
#include "ship/ship.h"
#include "object/object.h"
#include "parse/sexp.h"

// Extern functions/variables
extern void sexp_player_use_ai(int use_ai);
extern int sexp_distance2(int obj1, char *subj);
extern int ai_goal_find_empty_slot( ai_goal *goals );

extern fix Game_time_compression;


#if defined(ENABLE_AUTO_PILOT)
// Module variables
bool AutoPilotEngaged;
int CurrentNav;
NavPoint Navs[MAX_NAVPOINTS];

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
vector NavPoint::GetPosition()
{
	vector position;

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
		NavName = strdup(((waypoint_list*)target_obj)->name);
		strcat(NavName, ":");
		strcat(NavName, itoa(waypoint_num, strtmp, 10));
	}
	else
	{
		NavName = strdup(((ship*)target_obj)->ship_name);
	}

	return NavName;
}

// ********************************************************************************************
// Tell us is autopilot is allow
// This needs:
//        * Nav point selected
//        * No enemies within X distance
//        * Destination > 10,000 meters away
bool CanAutopilot()
{
	bool CanAuto = true;
	CanAuto = CanAuto && CurrentNav != -1;

	int dist = sexp_distance2(Player_ship->objnum, "<any hostile>");

	CanAuto = CanAuto && (10000 <= dist || dist == SEXP_NAN);
	

	if (CanAuto) // protect against no currentNav
	{
		CanAuto = CanAuto && (10000 <= sexp_distance2(Player_ship->objnum, Navs[CurrentNav].GetInteralName()));
	}

	return CanAuto;
}

// ********************************************************************************************
// Engages autopilot
// This does:
//        * Control switched from player to AI
//        * Time compression to 32x
//        * Tell AI to fly to targetted Nav Point (for all nav-status wings/ships)
//		  * Sets max waypoint speed to the best-speed of the slowest ship tagged
void StartAutopilot()
{
	if (!CanAutopilot())
		return;

	AutoPilotEngaged = true;

	sexp_player_use_ai(1);
	Game_time_compression = F1_0*32;

	// determine speed cap
	int i;
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
			Ai_info[Ships[i].ai_index].waypoint_speed_cap = speed_cap;

			
			// if they're not part of a wing set their goal
			if (Ships[i].wingnum == -1)
			{
				if (Navs[CurrentNav].flags & NP_WAYPOINT)
				{
					
					ai_add_ship_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_WAYPOINTS_ONCE, 0, ((waypoint_list*)Navs[CurrentNav].target_obj)->name, &Ai_info[Ships[i].ai_index] );
					

				}
				else
				{
					ai_add_ship_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_CHASE, 0, ((ship*)Navs[CurrentNav].target_obj)->ship_name, &Ai_info[Ships[i].ai_index] );
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
				
				ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_WAYPOINTS_ONCE, 0, ((waypoint_list*)Navs[CurrentNav].target_obj)->name, i );
				

			}
			else
			{
				ai_add_wing_goal_player( AIG_TYPE_PLAYER_WING, AI_GOAL_CHASE, 0, ((ship*)Navs[CurrentNav].target_obj)->ship_name, i );
			}
		}
	}
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

	Game_time_compression = F1_0;
	sexp_player_use_ai(0);

	//Clear AI Goals

	// assign ship goals
	// when assigning goals to individual ships only do so if Ships[shipnum].wingnum != -1 
	// we will assign wing goals below
	int i;

	for (i = 0; i < MAX_SHIPS; i++)

	{
		if (Ships[i].objnum != -1 && Ships[i].flags2 & SF2_NAVPOINT_CARRY && Ships[i].wingnum != -1 )
		{
			ai_clear_ship_goals( &(Ai_info[Ships[i].ai_index]) );
		}
	}

	// assign wing goals
	for (i = 0; i < MAX_WINGS; i++)
	{
		if (Wings[i].flags & WF_NAV_CARRY )
		{
			ai_clear_wing_goals( i );
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
			if (Navs[i].flags & NP_SHIP)
			{
				if (((ship*)Navs[i].target_obj)->objnum == -1)
					DelNavPoint(i);
			}
		}
		
		// check if we're reached a Node
		for (i = 0; i < MAX_NAVPOINTS; i++)
		{
			if (Navs[i].flags & NP_VALIDTYPE && DistanceTo(i) < 1000)
				Navs[i].flags |= NP_VISITED;
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

	for (int i = 0; i < MAX_NAVPOINTS && empty == -1; i++)
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

	if (node == 0)
		node = 1;

	for (int i = 0; i < MAX_NAVPOINTS && empty == -1; i++)
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
	return Nav_UnSet_Flag(Nav, NP_HIDDEN);
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


int DistanceTo(char *nav)
{
	int n = FindNav(nav);

	return DistanceTo(n);
}

int DistanceTo(int nav)
{
	if (nav > MAX_NAVPOINTS && nav < 0)
		return 0xFFFFFFFF;

	int distance = sexp_distance2(Player_ship->objnum, Navs[nav].GetInteralName());

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

#endif