// Autopilot.h
// Derek Meek
// 4-30-2004

/*
 * $Logfile: /Freespace2/code/Autopilot/Autopilot.h $
 * $Revision: 1.7 $
 * $Date: 2005-04-05 05:53:14 $
 * $Author: taylor $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2004/08/11 05:06:18  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.5  2004/07/27 18:52:10  Kazan
 * squished another
 *
 * Revision 1.4  2004/07/25 19:27:51  Kazan
 * only disable afterburning during AIM_WAYPOINTS and AIM_FLY_TO_SHIP while AutoPilotEngaged
 *
 * Revision 1.3  2004/07/25 00:31:27  Kazan
 * i have absolutely nothing to say about that subject
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

#include "PreProcDefines.h"
#if !defined(_AUTOPILOT_H_)
#define _AUTOPILOT_H_

#include "globalincs/pstypes.h"

// milliseconds between updates
#define NPS_TICKRATE	250

#define MAX_NAVPOINTS	8

#define NP_WAYPOINT		0x0001 // Nav Point is bound to the poistion of a single node of a waypoint path
#define NP_SHIP			0x0002 // Nav Point is bound to the poisiton of a certain ship 
#define NP_HIDDEN		0x0004 // Nav Point doesn't show on map and isn't selectable
#define NP_NOACCESS		0x0008 // Nav Point isn't selectable
#define NP_VISITED		0x0100 // Whether we've been within 1,000 meters of this waypoint

#define NP_NOSELECT		( NP_HIDDEN | NP_NOACCESS )
#define NP_VALIDTYPE	( NP_WAYPOINT | NP_SHIP )

struct NavPoint 
{
	char NavName[32];
	int flags;

	void *target_obj;
	int waypoint_num; //only used when flags & NP_WAYPOINT

	vec3d GetPosition();
	char* GetInteralName();
};

extern bool AutoPilotEngaged;
extern int CurrentNav;
extern NavPoint Navs[MAX_NAVPOINTS];

// Cycles through the NavPoint List
bool Sel_NextNav();


// Tell us is autopilot is allow
// This needs:
//        * Nav point selected
//        * No enemies within 5,000 meters
//        * Destination > 1,000 meters away
bool CanAutopilot();

// Engages autopilot
// This does:
//        * Control switched from player to AI
//        * Time compression to 32x
//        * Tell AI to fly to targetted Nav Point (for all nav-status wings/ships)
//		  * Sets max waypoint speed to the best-speed of the slowest ship tagged
void StartAutopilot();

// Checks if autopilot should automatically die
// Returns true if:
//         * Targetted waypoint < 1,000 meters away
//         * Enemy < 5,000 meters
bool Autopilot_AutoDiable();

// Disengages autopilot
// this does:
//         * Time compression to 1x
//         * Delete AI nav goal
//         * Control switched from AI to player
void EndAutoPilot();


// Checks for changes every NPS_TICKRATE milliseconds
// Checks:
//			* if we've gotten close enough to a nav point for it to be counted as "Visited"
//			* If we're current AutoNavigating it checks if we need to autodisengage
void NavSystem_Do();


// Inits the Nav System
void NavSystem_Init();

// Finds a Nav point by name
int FindNav(char *Nav);

// Set A Nav point to "ZERO"
void ZeroNav(int i);

// Removes a Nav
bool DelNavPoint(char *Nav);
bool DelNavPoint(int nav);

// adds a Nav
bool AddNav_Ship(char *Nav, char *TargetName, int flags); 
bool AddNav_Waypoint(char *Nav, char *WP_Path, int node, int flags);

//Change Flags
bool Nav_Alt_Flags(char *Nav, int flags);

// Sexp Accessors
bool Nav_Set_Flag(char *Nav, int flag);
bool Nav_UnSet_Flag(char *Nav, int flag);

bool Nav_Set_Hidden(char *Nav);
bool Nav_Set_NoAccess(char *Nav);
bool Nav_Set_Visited(char *Nav);

bool Nav_UnSet_Hidden(char *Nav);
bool Nav_UnSet_NoAccess(char *Nav);
bool Nav_UnSet_Visited(char *Nav);

// Useful functions
unsigned int DistanceTo(char *nav);
unsigned int DistanceTo(int nav);

bool IsVisited(char *nav);
bool IsVisited(int nav);
#endif

