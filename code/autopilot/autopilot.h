// Autopilot.h
// Derek Meek
// 4-30-2004



#if !defined(_AUTOPILOT_H_)
#define _AUTOPILOT_H_

#include "globalincs/pstypes.h"
#include "object/object.h"

#include <map>

// milliseconds between updates
#define NPS_TICKRATE	125

#define MAX_NAVPOINTS	8

#define NP_WAYPOINT		0x0001 // Nav Point is bound to the position of a single node of a waypoint path
#define NP_SHIP			0x0002 // Nav Point is bound to the position of a certain ship 
#define NP_HIDDEN		0x0004 // Nav Point doesn't show on map and isn't selectable
#define NP_NOACCESS		0x0008 // Nav Point isn't selectable
#define NP_VISITED		0x0100 // Whether we've been within 1,000 meters of this waypoint

#define NP_NOSELECT		( NP_HIDDEN | NP_NOACCESS )
#define NP_VALIDTYPE	( NP_WAYPOINT | NP_SHIP )

class NavPoint 
{
public:
	char m_NavName[32] = { 0 };
	int flags = 0;
	ubyte normal_color[3] = { 0x80, 0x80, 0xFF };
	ubyte visited_color[3] = { 0xFF, 0xFF, 0x00 };

	const void *target_obj = nullptr;
	int waypoint_num = -1; //only used when flags & NP_WAYPOINT

	const vec3d *GetPosition();

	// these assignments should match the initialization
	void clear()
	{
		m_NavName[0] = 0;
		flags = 0;

		normal_color[0] = 0x80;
		normal_color[1] = 0x80;
		normal_color[2] = 0xFF;

		visited_color[0] = 0xFF;
		visited_color[1] = 0xFF;
		visited_color[2] = 0x00;

		target_obj = nullptr;
		waypoint_num = -1;
	}
};


#define NP_MSG_FAIL_NOSEL		0
#define NP_MSG_FAIL_GLIDING		1
#define NP_MSG_FAIL_TOCLOSE		2
#define NP_MSG_FAIL_HOSTILES	3
#define NP_MSG_MISC_LINKED		4
#define NP_MSG_FAIL_HAZARD		5
#define NP_MSG_FAIL_SUPPORT_PRESENT	6
#define NP_MSG_FAIL_SUPPORT_WORKING 7
#define NP_NUM_MESSAGES 8

struct NavMessage
{
	char message[256];
	char filename[256]; // can be ""
};

extern bool AutoPilotEngaged;
extern int CurrentNav;
extern NavPoint Navs[MAX_NAVPOINTS];
extern NavMessage NavMsgs[NP_NUM_MESSAGES];
extern TIMESTAMP LockAPConv;
extern SCP_map<int,int> autopilot_wings;

// Cycles through the NavPoint List
bool Sel_NextNav();


// Tell us if autopilot is allowed
// This needs:
//        * Nav point selected
//        * No enemies within AutopilotMinEnemyDistance meters
//        * No asteroids within AutopilotMinAsteroidDistance meters
//        * Destination > 1,000 meters away
//        * Support ship not present or is actively leaving
bool CanAutopilot(const vec3d *targetPos, bool send_msg=false);

// Check if autopilot is allowed at player's current position
// See CanAutopilot(vec3d, bool) for more information
inline bool CanAutopilot(bool send_msg=false) { return CanAutopilot(&Player_obj->pos, send_msg); }

// Engages autopilot
// This does:
//        * Checks if Autopilot is allowed.  See CanAutopilot() for conditions.
//        * Control switched from player to AI
//        * Time compression to 32x
//        * Tell AI to fly to targeted Nav Point (for all nav-status wings/ships)
//        * Sets max waypoint speed to the best-speed of the slowest ship tagged
// Returns false if autopilot cannot be started. True otherwise.
bool StartAutopilot();

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

// parse autopilot.tbl
void parse_autopilot_table(const char *filename);

// Finds a Nav point by name
int FindNav(const char *Nav);

// Selects a Nav point by name
void SelectNav(const char *Nav);

// Deselects any navpoint selected.
void DeselectNav();

// Removes a Nav
bool DelNavPoint(const char *Nav);
bool DelNavPoint(int nav);

// adds a Nav
bool AddNav_Ship(const char *Nav, const char *TargetName, int flags);
bool AddNav_Waypoint(const char *Nav, const char *WP_Path, int node, int flags);

// Sexp Accessors
bool Nav_Set_Flag(const char *Nav, int flag);
bool Nav_UnSet_Flag(const char *Nav, int flag);

bool Nav_Set_Hidden(const char *Nav);
bool Nav_Set_NoAccess(const char *Nav);
bool Nav_Set_Visited(const char *Nav);

bool Nav_UnSet_Hidden(const char *Nav);
bool Nav_UnSet_NoAccess(const char *Nav);
bool Nav_UnSet_Visited(const char *Nav);

// Useful functions
unsigned int DistanceTo(const char *nav);
unsigned int DistanceTo(int nav);

bool IsVisited(const char *nav);
bool IsVisited(int nav);

void Nav_SetColor(const char *nav, bool visited, ubyte r, ubyte g, ubyte b);

void send_autopilot_msg(const char *msg, const char *snd=nullptr);
void send_autopilot_msgID(int msgid);
#endif

