/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionParse.cpp $
 * $Revision: 2.10 $
 * $Date: 2003-01-01 23:33:34 $
 * $Author: Goober5000 $
 *
 * main upper level code for parsing stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.9  2002/12/27 02:57:51  Goober5000
 * removed the existing stealth sexps and replaced them with the following...
 * ship-stealthy
 * ship-unstealthy
 * is-ship-stealthy
 * friendly-stealth-invisible
 * friendly-stealth-visible
 * is-friendly-stealth-visible
 * --Goober5000
 *
 * Revision 2.8  2002/12/23 05:18:52  Goober5000
 * Squashed some Volition bugs! :O Some of the sexps for dealing with more than
 * one ship would return after only dealing with the first ship.
 *
 * Also added the following sexps:
 * is-ship-stealthed
 * ship-force-stealth
 * ship-force-nostealth
 * ship-remove-stealth-forcing
 *
 * They toggle the stealth flag on and off.  If a ship is forced stealthy, it won't even
 * show up for friendly ships.
 * --Goober5000
 *
 * Revision 2.7  2002/12/22 22:59:04  Goober5000
 * hack: turn on ship trails if in nebula - restores backward compatibility
 * --Goober5000
 *
 * Revision 2.6  2002/12/13 08:13:29  Goober5000
 * small tweaks and bug fixes for the ballistic primary conversion
 * ~Goober5000~
 *
 * Revision 2.5  2002/12/10 05:43:34  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.4  2002/12/03 23:05:13  Goober5000
 * implemented beam-free-all-by-default mission flag
 *
 * Revision 2.3  2002/12/03 00:00:52  Goober5000
 * fixed player entry delay bug
 *
 * Revision 2.2  2002/11/14 06:15:02  bobboau
 * added nameplate code
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/10 20:42:44  mharris
 * use "ifndef NO_NETWORK" all over the place
 *
 * Revision 1.2  2002/05/03 22:07:09  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 63    9/12/99 8:09p Dave
 * Fixed problem where skip-training button would cause mission messages
 * not to get paged out for the current mission.
 * 
 * 62    9/09/99 3:53a Andsager
 * Only reposition HUGE ships to center of knossos device on warp in
 * 
 * 61    8/27/99 9:07p Dave
 * LOD explosions. Improved beam weapon accuracy.
 * 
 * 60    8/26/99 8:51p Dave
 * Gave multiplayer TvT messaging a heavy dose of sanity. Cheat codes.
 * 
 * 59    8/25/99 10:06a Jefff
 * vasudan pilots get a vasudan support ship. 
 * 
 * 58    8/24/99 5:27p Andsager
 * Make subsystems with zero strength before mission blown off.  Protect
 * red alert pilot against dying between orders and jump.
 * 
 * 57    8/18/99 10:07p Johnson
 * Fix Fred bug in positioning of Knossos device (when trying to warp in
 * through self)
 * 
 * 56    8/18/99 3:57p Andsager
 * Add warning for invalid alt_name.
 * 
 * 55    8/18/99 3:48p Andsager
 * Make support ship take default name and not 0th alt_name.
 * 
 * 54    8/16/99 3:53p Andsager
 * Add special warp in interface in Fred and saving / reading.
 * 
 * 53    8/16/99 2:01p Andsager
 * Knossos warp-in warp-out.
 * 
 * 52    8/03/99 5:35p Andsager
 * Dont draw target dot for instructor in training mission
 * 
 * 51    7/30/99 7:01p Dave
 * Dogfight escort gauge. Fixed up laser rendering in Glide.
 * 
 * 50    7/28/99 1:36p Andsager
 * Modify cargo1 to include flag CARGO_NO_DEPLETE.  Add sexp
 * cargo-no-deplete (only for BIG / HUGE).  Modify ship struct to pack
 * better.
 * 
 * 49    7/26/99 5:50p Dave
 * Revised ingame join. Better? We'll see....
 * 
 * 48    7/19/99 3:01p Dave
 * Fixed icons. Added single transport icon.
 * 
 * 47    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 46    7/13/99 5:03p Alanl
 * make sure object sounds get assigned to ships
 * 
 * 45    7/11/99 2:14p Dave
 * Added Fred names for the new icon types.
 * 
 * 44    7/02/99 4:\31p Dave
 * Much more sophisticated lightning support.
 * 
 * 43    7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 42    6/28/99 4:51p Andsager
 * Add ship-guardian sexp (does not allow ship to be killed)
 * 
 * 41    6/21/99 1:34p Alanl
 * event music tweaks
 * 
 * 40    6/16/99 10:20a Dave
 * Added send-message-list sexpression.
 * 
 * 39    6/14/99 2:06p Andsager
 * Default load brown asteroids
 * 
 * 38    6/10/99 11:06a Andsager
 * Mission designed selection of asteroid types.
 * 
 * 37    6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 36    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 35    4/26/99 8:49p Dave
 * Made all pof based nebula stuff full customizable through fred.
 * 
 * 34    4/26/99 12:49p Andsager
 * Add protect object from beam support to Fred
 * 
 * 33    4/16/99 2:34p Andsager
 * Second pass on debris fields
 * 
 * 32    4/15/99 5:00p Andsager
 * Frist pass on Debris field
 * 
 * 31    4/07/99 6:22p Dave
 * Fred and Freespace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 30    3/31/99 9:52a Andsager
 * generalization for debris field
 * 
 * 29    3/30/99 5:40p Dave
 * Fixed reinforcements for TvT in multiplayer.
 * 
 * 28    3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 27    3/24/99 6:23p Dave
 * Make sure we only apply squadron changes to the player in single-player
 * campaign mode.
 * 
 * 26    3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
 * 
 * 25    3/01/99 7:39p Dave
 * Added prioritizing ship respawns. Also fixed respawns in TvT so teams
 * don't mix respawn points.
 * 
 * 24    2/26/99 6:01p Andsager
 * Add sexp has-been-tagged-delay and cap-subsys-cargo-known-delay
 * 
 * 23    2/24/99 2:25p Dave
 * Fixed up chatbox bugs. Made squad war reporting better. Fixed a respawn
 * bug for dogfight more.
 * 
 * 22    2/23/99 8:11p Dave
 * Tidied up dogfight mode. Fixed TvT ship type problems for alpha wing.
 * Small pass over todolist items.
 * 
 * 21    2/17/99 2:10p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 20    2/11/99 3:08p Dave
 * PXO refresh button. Very preliminary squad war support.
 * 
 * 19    2/11/99 2:15p Andsager
 * Add ship explosion modification to FRED
 * 
 * 18    2/07/99 8:51p Andsager
 * Add inner bound to asteroid field.  Inner bound tries to stay astroid
 * free.  Wrap when within and don't throw at ships inside.
 * 
 * 17    2/05/99 10:38a Johnson
 * Fixed improper object array reference. 
 * 
 * 16    2/04/99 1:23p Andsager
 * Apply max spark limit to ships created in mission parse
 * 
 * 15    2/03/99 12:42p Andsager
 * Add escort priority.  Modify ship_flags_dlg to include field.  Save and
 * Load.  Add escort priority field to ship.
 * 
 * 14    1/27/99 9:56a Dave
 * Temporary checkin of beam weapons for Dan to make cool sounds.
 * 
 * 13    1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 12    1/19/99 3:57p Andsager
 * Round 2 of variables
 * 
 * 11    1/07/99 1:52p Andsager
 * Initial check in of Sexp_variables
 * 
 * 10    11/14/98 5:32p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 9     11/12/98 12:13a Dave
 * Tidied code up for multiplayer test. Put in network support for flak
 * guns.
 * 
 * 8     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 7     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 6     10/29/98 9:23p Dave
 * Removed minor bug concerning externalization of campaign files.
 * 
 * 5     10/23/98 3:51p Dave
 * Full support for tstrings.tbl and foreign languages. All that remains
 * is to make it active in Fred.
 * 
 * 4     10/20/98 10:44a Andsager
 * Add comment for creating sparks before mission starts
 * 
 * 3     10/07/98 6:27p Dave
 * Globalized mission and campaign file extensions. Removed Silent Threat
 * special code. Moved \cache \players and \multidata into the \data
 * directory.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 503   9/21/98 8:46p Dave
 * Put in special check in fred for identifying unknown ships.
 * 
 * 502   9/14/98 5:09p Dave
 * Massive hack to always ignore m-clash
 * 
 * 501   9/14/98 3:40p Allender
 * better error checking for invalid number of waves for player wings in a
 * multiplayer game.  Better popup message in FreeSpace side.
 * 
 * 500   9/11/98 2:05p Allender
 * make reinforcements work correctly in multiplayer games.  There still
 * may be a team vs team issue that I haven't thought of yet :-(
 * 
 * 499   8/07/98 9:48a Allender
 * changed how SF_FROM_PLAYER_WING is assigned for team vs. team games
 * 
 * 498   7/16/98 2:22p Allender
 * don't do wave check in single player
 * 
 * 497   7/13/98 5:19p Dave
 * 
 * 496   7/13/98 3:15p Allender
 * don't allow multiple waves for any player wings
 * 
 * 495   7/10/98 12:11a Allender
 * fixed problem where missions files of 0 length would cause game to
 * crash
 * 
 * 494   5/21/98 3:31p Allender
 * fix bug where Ship_obj_list was getting overwritten by the exited ships
 * list
 * 
 * 493   5/20/98 1:04p Hoffoss
 * Made credits screen use new artwork and removed rating field usage from
 * Fred (a goal struct member).
 * 
 * 492   5/18/98 4:41p Comet
 * allender: fix problem where ships in wings were not deleted on clients
 * when a new wing create packet came in.  A serious hack of all hacks 
 * 
 * 491   5/18/98 3:37p Jasen
 * move Int3() about too many ships in wing to before where ship actually
 * gets created
 * 
 * 490   5/18/98 1:58a Mike
 * Make Phoenix not be fired at fighters (but yes bombers).
 * Improve positioning of ships in guard mode.
 * Make turrets on player ship not fire near end of support ship docking.
 * 
 * 489   5/15/98 9:52a Allender
 * added code to give Warning when orders accepted don't match the default
 * orders
 * 
 * 488   5/13/98 4:41p Mike
 * Make big ships try a tiny bit to avoid collision with each other when
 * attacking another big ship.  Make ships a little less likely to collide
 * into player when in formation, drop off if player flying wacky.
 * 
 * 487   5/13/98 3:07p Mitri
 * fixed problem with checking current count of the mission against max
 * ships per wing
 * 
 * 486   5/11/98 4:33p Allender
 * fixed ingame join problems -- started to work on new object updating
 * code (currently ifdef'ed out)
 * 
 * 485   5/08/98 5:05p Dave
 * Go to the join game screen when quitting multiplayer. Fixed mission
 * text chat bugs. Put mission type symbols on the create game list.
 * Started updating standalone gui controls.
 *
 * $NoKeywords: $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <setjmp.h>

#include "freespace2/freespace.h"
#include "parse/parselo.h"
#include "mission/missionparse.h"
#include "mission/missiongoals.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "parse/sexp.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "ship/ai.h"
#include "ship/aigoals.h"
#include "playerman/player.h"
#include "starfield/starfield.h"
#include "bmpman/bmpman.h"
#include "lighting/lighting.h"
#include "gamesnd/eventmusic.h"
#include "mission/missionbriefcommon.h"
#include "ship/shipfx.h"
#include "debris/debris.h"
#include "cfile/cfile.h"
#include "fireball/fireballs.h"
#include "gamesnd/gamesnd.h"
#include "gamesequence/gamesequence.h"
#include "stats/medals.h"
#include "starfield/nebula.h"
#include "palman/palman.h"
#include "hud/hudets.h"
#include "mission/missionhotkey.h"
#include "hud/hudescort.h"
#include "asteroid/asteroid.h"
#include "ship/shiphit.h"
#include "math/staticrand.h"
#include "missionui/missioncmdbrief.h"
#include "missionui/redalert.h"
#include "hud/hudwingmanstatus.h"
#include "jumpnode/jumpnode.h"
#include "localization/localize.h"
#include "nebula/neb.h"
#include "demo/demo.h"
#include "nebula/neblightning.h"
#include "math/fvi.h"

#ifndef NO_NETWORK
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multi_respawn.h"
#include "network/multi_endgame.h"
#else
  // mharris FIXME: temp until I figure out if needed...
  //extern ushort netmisc_calc_checksum( void * vptr, int len );
  //extern ushort multi_assign_network_signature( int what_kind );
  #include "network/multiutil.h"
#endif

LOCAL struct {
	p_object *docker;
	char dockee[NAME_LENGTH];
	char docker_point[NAME_LENGTH];
	char dockee_point[NAME_LENGTH];
} Initially_docked[MAX_SHIPS];

int Total_initially_docked;

mission	The_mission;
char Mission_filename[80];

int Mission_palette;  // index into Nebula_palette_filenames[] of palette file to use for mission
int Nebula_index;  // index into Nebula_filenames[] of nebula to use in mission.
int Num_iff = MAX_IFF;
int Num_ai_behaviors = MAX_AI_BEHAVIORS;
int Num_cargo = 0;
int Num_status_names = MAX_STATUS_NAMES;
int Num_arrival_names = MAX_ARRIVAL_NAMES;
int Num_goal_type_names = MAX_GOAL_TYPE_NAMES;
int Num_team_names = MAX_TEAM_NAMES;
int Num_parse_goals;
int Player_starts = 1;
int Num_teams;
fix Entry_delay_time = 0;

ushort Current_file_checksum = 0;
ushort Last_file_checksum = 0;
int    Current_file_length   = 0;

// alternate ship type names
char Mission_alt_types[MAX_ALT_TYPE_NAMES][NAME_LENGTH];
int Mission_alt_type_count = 0;

#define SHIP_WARP_TIME 5.0f		// how many seconds it takes for ship to warp in

// the ship arrival list will contain a list of ships that are yet to arrive.  This
// list could also include ships that are part of wings!

p_object	ship_arrivals[MAX_SHIP_ARRIVALS], ship_arrival_list;		// for linked list of ships to arrive later
int		num_ship_arrivals;

#define MAX_SHIP_ORIGINAL			100
p_object ship_original[MAX_SHIP_ORIGINAL];
int		num_ship_original;

// list for arriving support ship
p_object	Support_ship_pobj;
p_object *Arriving_support_ship;
char Arriving_repair_targets[MAX_AI_GOALS][NAME_LENGTH];
int Num_arriving_repair_targets;

subsys_status Subsys_status[MAX_SUBSYS_STATUS];
int		Subsys_index;

char Mission_parse_storm_name[NAME_LENGTH] = "none";

team_data Team_data[MAX_TEAMS];

// variables for player start in single player
char		Player_start_shipname[NAME_LENGTH];
int		Player_start_shipnum;
p_object Player_start_pobject;

// name of all ships to use while parsing a mission (since a ship might be referenced by
// something before that ship has even been loaded yet)
char Parse_names[MAX_SHIPS + MAX_WINGS][NAME_LENGTH];
int Num_parse_names;

//XSTR:OFF

char *Nebula_filenames[NUM_NEBULAS] = {
	"Nebula01",
	"Nebula02",
	"Nebula03"	
};

char *Neb2_filenames[NUM_NEBULAS] = {
	"Nebfull01",
	"Nebfull02",
	"Nebfull03"
};

// Note: Nebula_colors[] and Nebula_palette_filenames are linked via index numbers
char *Nebula_colors[NUM_NEBULA_COLORS] = {
	"Red",
	"Blue",
	"Gold",
	"Purple",
	"Maroon",
	"Green",
	"Grey blue",
	"Violet",
	"Grey Green",
};

char *Iff_names[MAX_IFF] = { {"IFF 1"}, {"IFF 2"}, {"IFF 3"},
};

char *Ai_behavior_names[MAX_AI_BEHAVIORS] = {
	{"Chase"},
	{"Evade"},
	{"Get behind"},
	{"Stay Near"},
	{"Still"},
	{"Guard"},
	{"Avoid"},
	{"Waypoints"},
	{"Dock"},
	{"None"},
	{"Big Ship"},
	{"Path"},
	{"Be Rearmed"},
	{"Safety"},
	{"Evade Weapon"},
	{"Strafe"},
	{"Play Dead"},
	{"Bay Emerge"},
	{"Bay Depart"},
	{"Sentry Gun"},
	{"Warp Out"},
};

char *Cargo_names[MAX_CARGO];
char Cargo_names_buf[MAX_CARGO][NAME_LENGTH];

char *Ship_class_names[MAX_SHIP_TYPES];		// to be filled in from Ship_info array

char *Icon_names[MAX_BRIEF_ICONS] = {
	{"Fighter"}, {"Fighter Wing"}, {"Cargo"}, {"Cargo Wing"}, {"Largeship"},
	{"Largeship Wing"}, {"Capital"}, {"Planet"}, {"Asteroid Field"}, {"Waypoint"},
	{"Support Ship"}, {"Freighter(no cargo)"}, {"Freighter(has cargo)"},
	{"Freighter Wing(no cargo)"}, {"Freighter Wing(has cargo)"}, {"Installation"},
	{"Bomber"}, {"Bomber Wing"}, {"Cruiser"}, {"Cruiser Wing"}, {"Unknown"}, {"Unknown Wing"},
	{"Player Fighter"}, {"Player Fighter Wing"}, {"Player Bomber"}, {"Player Bomber Wing"}, 
	{"Knossos Device"}, {"Transport Wing"}, {"Corvette"}, {"Gas Miner"}, {"Awacs"}, {"Supercap"}, {"Sentry Gun"}, {"Jump Node"}, {"Transport"}
};

//	Translate team mask values like TEAM_FRIENDLY to indices in Team_names array.
//	-1 means an illegal value.
int	Team_names_index_xlate[MAX_TEAM_NAMES_INDEX+1] = {-1, 0, 1, -1, 2, -1, -1, -1, 3};

char *Team_names[MAX_TEAM_NAMES] = {
	{"Hostile"}, {"Friendly"}, {"Neutral"}, {"Unknown"},
};

char *Status_desc_names[MAX_STATUS_NAMES] = {
	{"Shields Critical"}, {"Engines Damaged"}, {"Fully Operational"},
};

char *Status_type_names[MAX_STATUS_NAMES] = {
	{"Damaged"}, {"Disabled"}, {"Corroded"},
};

char *Status_target_names[MAX_STATUS_NAMES] = {
	{"Weapons"}, {"Engines"}, {"Cable TV"},
};

// definitions for arrival locations for ships/wings
char *Arrival_location_names[MAX_ARRIVAL_NAMES] = {
	{"Hyperspace"}, {"Near Ship"}, {"In front of ship"}, {"Docking Bay"},
};

char *Special_arrival_anchor_names[MAX_SPECIAL_ARRIVAL_ANCHORS] =
{
	"<any friendly>",
	"<any enemy>",
	"<any neutral>",
	"<any friendly player>",
	"<any hostile player>",
	"<any neutral player>",
};

char *Departure_location_names[MAX_ARRIVAL_NAMES] = {
	{"Hyperspace"}, {"Docking Bay"},
};

char *Goal_type_names[MAX_GOAL_TYPE_NAMES] = {
	{"Primary"}, {"Secondary"}, {"Bonus"},
};

char *Species_names[MAX_SPECIES_NAMES] = {
	{"Terran"}, {"Vasudan"}, {"Shivan"},
};

char *Reinforcement_type_names[] = {
	"Attack/Protect",
	"Repair/Rearm",
};

char *Old_game_types[OLD_MAX_GAME_TYPES] = {
	"Single Player Only",	
	"Multiplayer Only",
	"Single/Multi Player",
	"Training mission"
};

char *Parse_object_flags[MAX_PARSE_OBJECT_FLAGS] = {
	"cargo-known",
	"ignore-count",
	"protect-ship",
	"reinforcement",
	"no-shields",
	"escort",
	"player-start",
	"no-arrival-music",
	"no-arrival-warp",
	"no-departure-warp",
	"locked",
	"invulnerable",
	"hidden-from-sensors",
	"scannable",
	"kamikaze",
	"no-dynamic",
	"red-alert-carry",
	"beam-protect-ship",
	"guardian",
	"special-warp"
};

char *Starting_wing_names[MAX_STARTING_WINGS+1] = {
	"Alpha",
	"Beta",
	"Gamma",
	"Zeta"
};

//XSTR:ON

int Num_reinforcement_type_names = sizeof(Reinforcement_type_names) / sizeof(char *);

vector Parse_viewer_pos;
matrix Parse_viewer_orient;

// definitions for timestamps for eval'ing arrival/departure cues
int Mission_arrival_timestamp;
int Mission_departure_timestamp;
fix Mission_end_time;

#define ARRIVAL_TIMESTAMP		2000		// every 2 seconds
#define DEPARTURE_TIMESTAMP	2200		// every 2.2 seconds -- just to be a little different

// calculates a "unique" file signature as a ushort (checksum) and an int (file length)
// the amount of The_mission we're going to checksum
// WARNING : do NOT call this function on the server - it will overwrite goals, etc
#define MISSION_CHECKSUM_SIZE (NAME_LENGTH + NAME_LENGTH + 4 + DATE_TIME_LENGTH + DATE_TIME_LENGTH)

// timers used to limit arrival messages and music
#define ARRIVAL_MUSIC_MIN_SEPARATION	60000
#define ARRIVAL_MESSAGE_MIN_SEPARATION 30000

#define ARRIVAL_MESSAGE_DELAY_MIN		2000
#define ARRIVAL_MESSAGE_DELAY_MAX		3000

static int Allow_arrival_music_timestamp;
static int Allow_arrival_message_timestamp;
static int Arrival_message_delay_timestamp;

// multi TvT
static int Allow_arrival_music_timestamp_m[2];
static int Allow_arrival_message_timestamp_m[2];
static int Arrival_message_delay_timestamp_m[2];

// local prototypes
void parse_player_info2(mission *pm);
void post_process_mission();
int allocate_subsys_status();
void parse_common_object_data(p_object	*objp);
void parse_asteroid_fields(mission *pm);
int mission_set_arrival_location(int anchor, int location, int distance, int objnum, vector *new_pos, matrix *new_orient);
int get_parse_name_index(char *name);
int get_anchor(char *name);
void mission_parse_do_initial_docks();
void mission_parse_set_arrival_locations();
void mission_set_wing_arrival_location( wing *wingp, int num_to_set );
int parse_lookup_alt_name(char *name);

void parse_mission_info(mission *pm)
{
	int i;
	char game_string[NAME_LENGTH];

	required_string("#Mission Info");
	
	required_string("$Version:");
	stuff_float(&pm->version);
	if (pm->version != MISSION_VERSION)
		mprintf(("Older mission, should update it (%.2f<-->%.2f)", pm->version, MISSION_VERSION));

	required_string("$Name:");
	stuff_string(pm->name, F_NAME, NULL);

	required_string("$Author:");
	stuff_string(pm->author, F_NAME, NULL);

	required_string("$Created:");
	stuff_string(pm->created, F_DATE, NULL);

	required_string("$Modified:");
	stuff_string(pm->modified, F_DATE, NULL);

	required_string("$Notes:");
	stuff_string(pm->notes, F_NOTES, NULL);

	if (optional_string("$Mission Desc:"))
		stuff_string(pm->mission_desc, F_MULTITEXT, NULL, MISSION_DESC_LENGTH);
	else
		strcpy(pm->mission_desc, NOX("No description\n"));

	pm->game_type = MISSION_TYPE_SINGLE;				// default to single player only
	if ( optional_string("+Game Type:")) {
		// HACK HACK HACK -- stuff_string was changed to *not* ignore carriage returns.  Since the
		// old style missions may have carriage returns, deal with it here.
		ignore_white_space();
		stuff_string(game_string, F_NAME, NULL);
		for ( i = 0; i < OLD_MAX_GAME_TYPES; i++ ) {
			if ( !stricmp(game_string, Old_game_types[i]) ) {

				// this block of code is now old mission compatibility code.  We specify game
				// type in a different manner than before.
				if ( i == OLD_GAME_TYPE_SINGLE_ONLY )
					pm->game_type = MISSION_TYPE_SINGLE;
				else if ( i == OLD_GAME_TYPE_MULTI_ONLY )
					pm->game_type = MISSION_TYPE_MULTI;
				else if ( i == OLD_GAME_TYPE_SINGLE_MULTI )
					pm->game_type = (MISSION_TYPE_SINGLE | MISSION_TYPE_MULTI );
				else if ( i == OLD_GAME_TYPE_TRAINING )
					pm->game_type = MISSION_TYPE_TRAINING;
				else
					Int3();

				if ( pm->game_type & MISSION_TYPE_MULTI )
					pm->game_type |= MISSION_TYPE_MULTI_COOP;

				break;
			}
		}
	}

	if ( optional_string("+Game Type Flags:") ) {
		stuff_int(&pm->game_type);
	}

	pm->flags = 0;
	if (optional_string("+Flags:")){
		stuff_int(&pm->flags);
	}

	// nebula mission stuff
	Neb2_awacs = -1.0f;
	if(optional_string("+NebAwacs:")){
		if(pm->flags & MISSION_FLAG_FULLNEB){
			stuff_float(&Neb2_awacs);
		} else {
			float temp;
			stuff_float(&temp);
		}
	}
	if(optional_string("+Storm:")){
		stuff_string(Mission_parse_storm_name, F_NAME, NULL);
		nebl_set_storm(Mission_parse_storm_name);
	}

	// hack: turn on ship trails if in nebula
	if (pm->flags & MISSION_FLAG_FULLNEB)
	{
		pm->flags |= MISSION_FLAG_SHIP_TRAILS;
	}

	// get the number of players if in a multiplayer mission
	pm->num_players = 1;
	if ( pm->game_type & MISSION_TYPE_MULTI ) {
		if ( optional_string("+Num Players:") ) {
			stuff_int( &(pm->num_players) );
		}
	}

	// get the number of respawns
	pm->num_respawns = 0;
	if ( pm->game_type & MISSION_TYPE_MULTI ) {
		if ( optional_string("+Num Respawns:") ){
			stuff_int( (int*)&(pm->num_respawns) );
		}
	}

	pm->red_alert = 0;
	if ( optional_string("+Red Alert:")) {
		stuff_int(&pm->red_alert);
	} 
	red_alert_set_status(pm->red_alert);

	pm->scramble = 0;
	if ( optional_string("+Scramble:")) {
		stuff_int(&pm->scramble);
	}

	pm->disallow_support = 0;
	if ( optional_string("+Disallow Support:")) {
		stuff_int(&pm->disallow_support);
	}

	if (optional_string("+All Teams Attack")){
		Mission_all_attack = 1;
	} else {
		Mission_all_attack = 0;
	}

	//	Maybe delay the player's entry.
	if (optional_string("+Player Entry Delay:")) {
		float	temp;
		
		stuff_float(&temp);
		Assert(temp >= 0.0f);
		Entry_delay_time = fl2f(temp);
	}
	else
	{
		Entry_delay_time = 0;
	}

	if (optional_string("+Viewer pos:")){
		stuff_vector(&Parse_viewer_pos);
	}

	if (optional_string("+Viewer orient:")){
		stuff_matrix(&Parse_viewer_orient);
	}

	// possible squadron reassignment
	strcpy(The_mission.squad_name, "");
	strcpy(The_mission.squad_filename, "");
	if(optional_string("+SquadReassignName:")){
		stuff_string(The_mission.squad_name, F_NAME, NULL);
		if(optional_string("+SquadReassignLogo:")){
			stuff_string(The_mission.squad_filename, F_NAME, NULL);
		}
	}	
	// always clear out squad reassignments if not single player
	if(Game_mode & GM_MULTIPLAYER){
		strcpy(The_mission.squad_name, "");
		strcpy(The_mission.squad_filename, "");
		mprintf(("Ignoring squadron reassignment"));
	}
	// reassign the player
	else {		
		if(!Fred_running && (Player != NULL) && (strlen(The_mission.squad_name) > 0) && (Game_mode & GM_CAMPAIGN_MODE)){
			mprintf(("Reassigning player to squadron %s\n", The_mission.squad_name));
			player_set_squad(Player, The_mission.squad_name);
			player_set_squad_bitmap(Player, The_mission.squad_filename);
		}
	}

	// set up the Num_teams variable accoriding to the game_type variable'
	Num_teams = 1;				// assume 1

	// multiplayer team v. team games have two teams.  If we have three teams, we need to use
	// a new mission mode!
	if ( (pm->game_type & MISSION_TYPE_MULTI) && (pm->game_type & MISSION_TYPE_MULTI_TEAMS) ){
		Num_teams = 2;
	}
}

void parse_player_info(mission *pm)
{
	char alt[NAME_LENGTH + 2] = "";
	Assert(pm != NULL);

// alternate type names begin here	
	mission_parse_reset_alt();
	if(optional_string("#Alternate Types:")){		
		// read them all in
		while(!optional_string("#end")){
			required_string("$Alt:");
			stuff_string(alt, F_NAME, NULL, NAME_LENGTH);

			// maybe store it
			mission_parse_add_alt(alt);			
		}
	}
	
	Player_starts = 0;
	required_string("#Players");

	while (required_string_either("#Objects", "$")){
		parse_player_info2(pm);
	}
}

void parse_player_info2(mission *pm)
{
	char str[NAME_LENGTH];
	int nt, i, total, list[MAX_SHIP_TYPES * 2], list2[MAX_WEAPON_TYPES * 2], num_starting_wings;
	team_data *ptr;
	char starting_wings[MAX_PLAYER_WINGS][NAME_LENGTH];

	// read in a ship/weapon pool for each team.
	for ( nt = 0; nt < Num_teams; nt++ ) {
		int num_ship_choices;

		ptr = &Team_data[nt];
		// get the shipname for single player missions
		// MWA -- make this required later!!!!
		if ( optional_string("$Starting Shipname:") )
			stuff_string( Player_start_shipname, F_NAME, NULL );

		required_string("$Ship Choices:");
		total = stuff_int_list(list, MAX_SHIP_TYPES * 2, SHIP_INFO_TYPE);

		Assert(!(total & 0x01));  // make sure we have an even count

		num_ship_choices = 0;
		total /= 2;							// there are only 1/2 the ships really on the list.
		for (i=0; i<total; i++) {
			// in a campaign, see if the player is allowed the ships or not.  Remove them from the
			// pool if they are not allowed
#ifndef NO_NETWORK
			if (Game_mode & GM_CAMPAIGN_MODE || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER))) {
#else
			if (Game_mode & GM_CAMPAIGN_MODE ) {
#endif
				if ( !Campaign.ships_allowed[list[i*2]] )
					continue;
			}

			ptr->ship_list[num_ship_choices] = list[i * 2];
			ptr->ship_count[num_ship_choices] = list[i * 2 + 1];
			num_ship_choices++;
		}
		ptr->number_choices = num_ship_choices;

		num_starting_wings = 0;
		if (optional_string("+Starting Wings:"))
			num_starting_wings = stuff_string_list(starting_wings, MAX_PLAYER_WINGS);

		ptr->default_ship = -1;
		if (optional_string("+Default_ship:")) {
			stuff_string(str, F_NAME, NULL);
			ptr->default_ship = ship_info_lookup(str);
			// see if the player's default ship is an allowable ship (campaign only). If not, then what
			// do we do?  choose the first allowable one?
#ifndef NO_NETWORK
			if (Game_mode & GM_CAMPAIGN_MODE || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER))) {
#else
			if (Game_mode & GM_CAMPAIGN_MODE) {
#endif
				if ( !(Campaign.ships_allowed[ptr->default_ship]) ) {
					for (i = 0; i < MAX_SHIP_TYPES; i++ ) {
						if ( Campaign.ships_allowed[ptr->default_ship] ) {
							ptr->default_ship = i;
							break;
						}
					}
					Assert( i < MAX_SHIP_TYPES );
				}
			}
		}

		if (ptr->default_ship == -1)  // invalid or not specified, make first in list
			ptr->default_ship = ptr->ship_list[0];

		for (i=0; i<MAX_WEAPON_TYPES; i++)
			ptr->weaponry_pool[i] = 0;

		if (optional_string("+Weaponry Pool:")) {
			total = stuff_int_list(list2, MAX_WEAPON_TYPES * 2, WEAPON_POOL_TYPE);

			Assert(!(total & 0x01));  // make sure we have an even count
			total /= 2;
			for (i=0; i<total; i++) {
				// in a campaign, see if the player is allowed the weapons or not.  Remove them from the
				// pool if they are not allowed
#ifndef NO_NETWORK
				if (Game_mode & GM_CAMPAIGN_MODE || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER))) {
#else
				if (Game_mode & GM_CAMPAIGN_MODE) {
#endif
					if ( !Campaign.weapons_allowed[list2[i*2]] )
						continue;
				}

				if ((list2[i * 2] >= 0) && (list2[i * 2] < MAX_WEAPON_TYPES))
					ptr->weaponry_pool[list2[i * 2]] = list2[i * 2 + 1];
			}
		}
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough ship/weapon pools for mission.  There are %d teams and only %d pools.", Num_teams, nt);
}

void parse_plot_info(mission *pm)
{
	required_string("#Plot Info");

	required_string("$Tour:");
	stuff_string(pm->tour_name, F_NAME, NULL);

	required_string("$Pre-Briefing Cutscene:");
	stuff_string(pm->pre_briefing_cutscene, F_FILESPEC, NULL);

	required_string("$Pre-Mission Cutscene:");
	stuff_string(pm->pre_mission_cutscene, F_FILESPEC, NULL);

	required_string("$Next Mission Success:");
	stuff_string(pm->next_mission_success, F_NAME, NULL);

	required_string("$Next Mission Partial:");
	stuff_string(pm->next_mission_partial, F_NAME, NULL);

	required_string("$Next Mission Failure:");
	stuff_string(pm->next_mission_failure, F_NAME, NULL);
}

void parse_briefing_info(mission *pm)
{
	char junk[4096];

	if ( !optional_string("#Briefing Info") )
		return;

	required_string("$Briefing Voice 1:");
	stuff_string(junk, F_FILESPEC, NULL);

	required_string("$Briefing Text 1:");
	stuff_string(junk, F_MULTITEXTOLD, NULL);

	required_string("$Briefing Voice 2:");
	stuff_string(junk, F_FILESPEC, NULL);

	required_string("$Briefing Text 2:");
	stuff_string(junk, F_MULTITEXTOLD, NULL);

	required_string("$Briefing Voice 3:");
	stuff_string(junk, F_FILESPEC, NULL);

	required_string("$Briefing Text 3:");
	stuff_string(junk, F_MULTITEXTOLD, NULL);

	required_string("$Debriefing Voice 1:");
	stuff_string(junk, F_FILESPEC, NULL);

	required_string("$Debriefing Text 1:");
	stuff_string(junk, F_MULTITEXTOLD, NULL);

	required_string("$Debriefing Voice 2:");
	stuff_string(junk, F_FILESPEC, NULL);

	required_string("$Debriefing Text 2:");
	stuff_string(junk, F_MULTITEXTOLD, NULL);

	required_string("$Debriefing Voice 3:");
	stuff_string(junk, F_FILESPEC, NULL);

	required_string("$Debriefing Text 3:");
	stuff_string(junk, F_MULTITEXTOLD, NULL);
}

// parse the event music and briefing music for the mission
void parse_music(mission *pm)
{
	char	name[NAME_LENGTH];

	event_music_reset_choices();

	if ( !optional_string("#Music") ) {
		return;
	}

	required_string("$Event Music:");
	stuff_string(name, F_NAME, NULL);
	event_music_set_soundtrack(name);

	required_string("$Briefing Music:");
	stuff_string(name, F_NAME, NULL);
	event_music_set_score(SCORE_BRIEFING, name);

	if ( optional_string("$Debriefing Success Music:") ) {
		stuff_string(name, F_NAME, NULL);
		event_music_set_score(SCORE_DEBRIEF_SUCCESS, name);
	}

	if ( optional_string("$Debriefing Fail Music:") ) {
		stuff_string(name, F_NAME, NULL);
		event_music_set_score(SCORE_DEBRIEF_FAIL, name);
	}
}

void parse_cmd_brief(mission *pm)
{
	int stage;

	Assert(!Cur_cmd_brief->num_stages);
	stage = 0;

	required_string("#Command Briefing");
	while (optional_string("$Stage Text:")) {
		Assert(stage < CMD_BRIEF_STAGES_MAX);
		Cur_cmd_brief->stage[stage].text = stuff_and_malloc_string(F_MULTITEXT, NULL, CMD_BRIEF_TEXT_MAX);
		Assert(Cur_cmd_brief->stage[stage].text);

		required_string("$Ani Filename:");
		stuff_string(Cur_cmd_brief->stage[stage].ani_filename, F_FILESPEC, NULL);
		if (optional_string("+Wave Filename:"))
			stuff_string(Cur_cmd_brief->stage[stage].wave_filename, F_FILESPEC, NULL);
		else
			Cur_cmd_brief->stage[stage].wave_filename[0] = 0;

		stage++;
	}

	Cur_cmd_brief->num_stages = stage;
}

void parse_cmd_briefs(mission *pm)
{
	int i;

	cmd_brief_reset();
	// a hack follows because old missions don't have a command briefing
	if (required_string_either("#Command Briefing", "#Briefing"))
		return;

	for (i=0; i<Num_teams; i++) {
		Cur_cmd_brief = &Cmd_briefs[i];
		parse_cmd_brief(pm);
	}
}

// -------------------------------------------------------------------------------------------------
// parse_briefing()
//
// Parse the data required for the mission briefing
//
// NOTE: This updates the global Briefing struct with all the data necessary to drive the briefing
//
void parse_briefing(mission *pm)
{
	int nt, i, j, stage_num = 0, icon_num = 0, team_index;
	brief_stage *bs;
	brief_icon *bi;
	briefing *bp;

	char not_used_text[MAX_ICON_TEXT_LEN];
	
	brief_reset();

	// MWA -- 2/3/98.  we can now have multiple briefing and debreifings in a mission
	for ( nt = 0; nt < Num_teams; nt++ ) {
		if ( !optional_string("#Briefing") )
			break;

		bp = &Briefings[nt];

		required_string("$start_briefing");
		required_string("$num_stages:");
		stuff_int(&bp->num_stages);
		Assert(bp->num_stages <= MAX_BRIEF_STAGES);

		stage_num = 0;
		while (required_string_either("$end_briefing", "$start_stage")) {
			required_string("$start_stage");
			Assert(stage_num < MAX_BRIEF_STAGES);
			bs = &bp->stages[stage_num++];
			required_string("$multi_text");
			if ( Fred_running )	{
				stuff_string(bs->new_text, F_MULTITEXT, NULL, MAX_BRIEF_LEN);
			} else {
				bs->new_text = stuff_and_malloc_string(F_MULTITEXT, NULL, MAX_BRIEF_LEN);
			}
			required_string("$voice:");
			stuff_string(bs->voice, F_FILESPEC, NULL);
			required_string("$camera_pos:");
			stuff_vector(&bs->camera_pos);
			required_string("$camera_orient:");
			stuff_matrix(&bs->camera_orient);
			required_string("$camera_time:");
			stuff_int(&bs->camera_time);

			if ( optional_string("$num_lines:") ) {
				stuff_int(&bs->num_lines);

				if ( Fred_running )	{
					Assert(bs->lines!=NULL);
				} else {
					if ( bs->num_lines > 0 )	{
						bs->lines = (brief_line *)malloc(sizeof(brief_line)*bs->num_lines);
						Assert(bs->lines!=NULL);
					}
				}

				for (i=0; i<bs->num_lines; i++) {
					required_string("$line_start:");
					stuff_int(&bs->lines[i].start_icon);
					required_string("$line_end:");
					stuff_int(&bs->lines[i].end_icon);
				}
			}
			else {
				bs->num_lines = 0;
			}

			required_string("$num_icons:");
			stuff_int(&bs->num_icons);

			if ( Fred_running )	{
				Assert(bs->lines!=NULL);
			} else {
				if ( bs->num_icons > 0 )	{
					bs->icons = (brief_icon *)malloc(sizeof(brief_icon)*bs->num_icons);
					Assert(bs->icons!=NULL);
				}
			}

			if ( optional_string("$flags:") )
				stuff_int(&bs->flags);
			else
				bs->flags = 0;

			if ( optional_string("$formula:") )
				bs->formula = get_sexp_main();
			else
				bs->formula = Locked_sexp_true;

			Assert(bs->num_icons <= MAX_STAGE_ICONS );

			while (required_string_either("$end_stage", "$start_icon")) {
				required_string("$start_icon");
				Assert(icon_num < MAX_STAGE_ICONS);
				bi = &bs->icons[icon_num++];

				required_string("$type:");
				stuff_int(&bi->type);

				find_and_stuff("$team:", &team_index, F_NAME, Team_names, Num_team_names, "team name");
				Assert((team_index >= 0) && (team_index < MAX_TEAM_NAMES));
				bi->team = 1 << team_index;

				find_and_stuff("$class:", &bi->ship_class, F_NAME, Ship_class_names, Num_ship_types, "ship class");

				required_string("$pos:");
				stuff_vector(&bi->pos);

				bi->label[0] = 0;
				if (optional_string("$label:"))
					stuff_string(bi->label, F_MESSAGE, NULL);

				if (optional_string("+id:")) {
					stuff_int(&bi->id);
					if (bi->id >= Cur_brief_id)
						Cur_brief_id = bi->id + 1;

				} else {
					bi->id = -1;
					for (i=0; i<stage_num-1; i++)
						for (j=0; j < bp->stages[i].num_icons; j++)
						{
							if (!stricmp(bp->stages[i].icons[j].label, bi->label))
								bi->id = bp->stages[i].icons[j].id;
						}

					if (bi->id < 0)
						bi->id = Cur_brief_id++;
				}

				required_string("$hlight:");
				int val;
				stuff_int(&val);
				if ( val>0 ) {
					bi->flags = BI_HIGHLIGHT;
				} else {
					bi->flags=0;
				}

				required_string("$multi_text");
//				stuff_string(bi->text, F_MULTITEXT, NULL, MAX_ICON_TEXT_LEN);
				stuff_string(not_used_text, F_MULTITEXT, NULL, MAX_ICON_TEXT_LEN);
				required_string("$end_icon");
			} // end while
			Assert(bs->num_icons == icon_num);
			icon_num = 0;
			required_string("$end_stage");
		}	// end while

		Assert(bp->num_stages == stage_num);
		required_string("$end_briefing");
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough briefings in mission file.  There are %d teams and only %d briefings.", Num_teams, nt );
}

// -------------------------------------------------------------------------------------------------
// parse_debriefing_old()
//
// Parse the data required for the mission debriefings
void parse_debriefing_old(mission *pm)
{
	int	junk;
	char	waste[MAX_DEBRIEF_LEN];
	
	if ( !optional_string("#Debriefing") )
		return;

	required_string("$num_debriefings:");
	stuff_int(&junk);

	while (required_string_either("#Players", "$start_debriefing")) {
		required_string("$start_debriefing");
		required_string("$formula:");
		junk = get_sexp_main();
		required_string("$num_stages:");
		stuff_int(&junk);
		while (required_string_either("$end_debriefing", "$start_stage")) {
			required_string("$start_stage");
			required_string("$multi_text");
			stuff_string(waste, F_MULTITEXT, NULL, MAX_DEBRIEF_LEN);
			required_string("$voice:");
			stuff_string(waste, F_FILESPEC, NULL);
			required_string("$end_stage");
		} // end while
		required_string("$end_debriefing");
	}	// end while
}

// -------------------------------------------------------------------------------------------------
// parse_debriefing_new()
//
// Parse the data required for the mission debriefings
void parse_debriefing_new(mission *pm)
{
	int				stage_num, nt;
	debriefing		*db;
	debrief_stage	*dbs;
	
	debrief_reset();

	// next code should be old -- hopefully not called anymore
	//if (!optional_string("#Debriefing_info")) {
	//	parse_debriefing_old(pm);
	//	return;
	//}

	// 2/3/98 -- MWA.  We can now have multiple briefings and debriefings on a team
	for ( nt = 0; nt < Num_teams; nt++ ) {

		if ( !optional_string("#Debriefing_info") )
			break;

		stage_num = 0;

		db = &Debriefings[nt];

		required_string("$Num stages:");
		stuff_int(&db->num_stages);
		Assert(db->num_stages <= MAX_DEBRIEF_STAGES);

		while (required_string_either("#", "$Formula")) {
			Assert(stage_num < MAX_DEBRIEF_STAGES);
			dbs = &db->stages[stage_num++];
			required_string("$Formula:");
			dbs->formula = get_sexp_main();
			required_string("$multi text");
			if ( Fred_running )	{
				stuff_string(dbs->new_text, F_MULTITEXT, NULL, MAX_DEBRIEF_LEN);
			} else {
				dbs->new_text = stuff_and_malloc_string(F_MULTITEXT, NULL, MAX_DEBRIEF_LEN);
			}
			required_string("$Voice:");
			stuff_string(dbs->voice, F_FILESPEC, NULL);
			required_string("$Recommendation text:");
			if ( Fred_running )	{
				stuff_string( dbs->new_recommendation_text, F_MULTITEXT, NULL, MAX_RECOMMENDATION_LEN);
			} else {
				dbs->new_recommendation_text = stuff_and_malloc_string( F_MULTITEXT, NULL, MAX_RECOMMENDATION_LEN);
			}
		} // end while

		Assert(db->num_stages == stage_num);
	}

	if ( nt != Num_teams )
		Error(LOCATION, "Not enough debriefings for mission.  There are %d teams and only %d debriefings;\n", Num_teams, nt );
}

void position_ship_for_knossos_warpin(p_object *objp, int shipnum, int objnum)
{
	// Assume no valid knossos device
	Ships[shipnum].special_warp_objnum = -1;

	// find knossos device
	int found = FALSE;
	ship_obj *so;
	int knossos_num = -1;
	for (so=GET_FIRST(&Ship_obj_list); so!=END_OF_LIST(&Ship_obj_list); so=GET_NEXT(so)) {
		knossos_num = Objects[so->objnum].instance;
		if (Ship_info[Ships[knossos_num].ship_info_index].flags & SIF_KNOSSOS_DEVICE) {
			// be close to the right device [allow multiple knossos's
			if (vm_vec_dist_quick(&Objects[knossos_num].pos, &objp->pos) < 2.0f*(Objects[knossos_num].radius + Objects[objnum].radius) ) {
				found = TRUE;
				break;
			}
		}
	}

	if (found) {
		// set ship special_warp_objnum
		Ships[shipnum].special_warp_objnum = knossos_num;

		// position self for warp on plane of device
		vector new_point;
		float dist = fvi_ray_plane(&new_point, &Objects[knossos_num].pos, &Objects[knossos_num].orient.vec.fvec, &objp->pos, &objp->orient.vec.fvec, 0.0f);
		polymodel *pm = model_get(Ship_info[Ships[shipnum].ship_info_index].modelnum);
		float desired_dist = -pm->mins.xyz.z;
		vm_vec_scale_add2(&Objects[objnum].pos, &Objects[objnum].orient.vec.fvec, (dist - desired_dist));
		// if ship is BIG or HUGE, make it go through the center of the knossos
		if (Ship_info[Ships[shipnum].ship_info_index].flags & SIF_HUGE_SHIP) {
			vector offset;
			vm_vec_sub(&offset, &Objects[knossos_num].pos, &new_point);
			vm_vec_add2(&Objects[objnum].pos, &offset);
		}
	}
}

//	Given a stuffed p_object struct, create an object and fill in the necessary fields.
//	Return object number.
int parse_create_object(p_object *objp)
{
	int	i, j, k, objnum, shipnum;
	ai_info *aip;
	ship_subsys *ptr;
	ship_subsys *temp_subsys;
	ship_info *sip;
	subsys_status *sssp;
	ship_weapon *wp;

	// base level creation
	objnum = ship_create(&objp->orient, &objp->pos, objp->ship_class);
	Assert(objnum != -1);
	shipnum = Objects[objnum].instance;

	// if arriving through knossos, adjust objpj->pos to plane of knossos and set flag
	// special warp is single player only
	if ((objp->flags & P_KNOSSOS_WARP_IN) && !(Game_mode & GM_MULTIPLAYER)) {
		if (!Fred_running) {
			position_ship_for_knossos_warpin(objp, shipnum, objnum);
		}
	}

	Ships[shipnum].group = objp->group;
	Ships[shipnum].team = objp->team;
	strcpy(Ships[shipnum].ship_name, objp->name);
	Ships[shipnum].escort_priority = objp->escort_priority;
	Ships[shipnum].special_exp_index = objp->special_exp_index;
	Ships[shipnum].respawn_priority = objp->respawn_priority;
#ifndef NO_NETWORK
	// if this is a multiplayer dogfight game, and its from a player wing, make it team traitor
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && (objp->wingnum >= 0)){
		for (i = 0; i < MAX_STARTING_WINGS; i++ ) {
			if ( !stricmp(Starting_wing_names[i], Wings[objp->wingnum].name) ) {
				Ships[shipnum].team = TEAM_TRAITOR;
			} 
		}
	}
#endif

	sip = &Ship_info[Ships[shipnum].ship_info_index];

	if ( !Fred_running ) {
		ship_assign_sound(&Ships[shipnum]);
	}

	aip = &(Ai_info[Ships[shipnum].ai_index]);
	aip->behavior = objp->behavior;
	aip->mode = aip->behavior;

	// alternate type name
	Ships[shipnum].alt_type_index = objp->alt_type_index;

	aip->ai_class = objp->ai_class;
	Ships[shipnum].weapons.ai_class = objp->ai_class;  // Fred uses this instead of above.

	// must reset the number of ai goals when the object is created
	for (i = 0; i < MAX_AI_GOALS; i++ ){
		aip->goals[i].ai_mode = AI_GOAL_NONE;
	}

	Ships[shipnum].cargo1 = objp->cargo1;

	Ships[shipnum].arrival_location = objp->arrival_location;
	Ships[shipnum].arrival_distance = objp->arrival_distance;
	Ships[shipnum].arrival_anchor = objp->arrival_anchor;
	Ships[shipnum].arrival_cue = objp->arrival_cue;
	Ships[shipnum].arrival_delay = objp->arrival_delay;
	Ships[shipnum].departure_location = objp->departure_location;
	Ships[shipnum].departure_anchor = objp->departure_anchor;
	Ships[shipnum].departure_cue = objp->departure_cue;
	Ships[shipnum].departure_delay = objp->departure_delay;
	Ships[shipnum].determination = objp->determination;
	Ships[shipnum].wingnum = objp->wingnum;
	Ships[shipnum].hotkey = objp->hotkey;
	Ships[shipnum].score = objp->score;
	Ships[shipnum].persona_index = objp->persona_index;
	Ships[shipnum].nameplate = objp->nameplate;

	// set the orders that this ship will accept.  It will have already been set to default from the
	// ship create code, so only set them if the parse object flags say they are unique
	if ( objp->flags & P_SF_USE_UNIQUE_ORDERS ) {
		Ships[shipnum].orders_accepted = objp->orders_accepted;

	// MWA  5/15/98 -- Added the following debug code because some orders that ships
	// will accept were apparently written out incorrectly with Fred.  This Int3() should
	// trap these instances.
#ifndef NDEBUG
		if ( Fred_running ) {
			int default_orders, remaining_orders;
			
			default_orders = ship_get_default_orders_accepted( &Ship_info[Ships[shipnum].ship_info_index] );
			remaining_orders = objp->orders_accepted & ~default_orders;
			if ( remaining_orders ) {
				Warning(LOCATION, "Ship %s has orders which it will accept that are\nnot part of default orders accepted.\n\nPlease reedit this ship and change the orders again\n", Ships[shipnum].ship_name);
			}
		}
#endif
	}

	// check the mission flag to possibly free all beam weapons - Goober5000, taken from SEXP.CPP
	if (The_mission.flags & MISSION_FLAG_BEAM_FREE_ALL_BY_DEFAULT)
	{
		temp_subsys = GET_FIRST(&Ships[shipnum].subsys_list);
		while(temp_subsys != END_OF_LIST(&Ships[shipnum].subsys_list))
		{
			// just mark all turrets as beam free
			if(temp_subsys->system_info->type == SUBSYSTEM_TURRET)
			{
				temp_subsys->weapons.flags |= SW_FLAG_BEAM_FREE;
				temp_subsys->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
			}

			// next item
			temp_subsys = GET_NEXT(temp_subsys);
		}
	}
	
	// check the parse object's flags for possible flags to set on this newly created ship
	if ( objp->flags & P_OF_PROTECTED ) {
		Objects[objnum].flags |= OF_PROTECTED;
	}

	if ( objp->flags & P_OF_BEAM_PROTECTED ) {
		Objects[objnum].flags |= OF_BEAM_PROTECTED;
	}

	if (objp->flags & P_OF_CARGO_KNOWN) {
		Ships[shipnum].flags |= SF_CARGO_REVEALED;
	}

	if ( objp->flags & P_SF_IGNORE_COUNT )
		Ships[shipnum].flags |= SF_IGNORE_COUNT;

	if ( objp->flags & P_SF_REINFORCEMENT )
		Ships[shipnum].flags |= SF_REINFORCEMENT;

	if (objp->flags & P_OF_NO_SHIELDS || sip->shields == 0 )
		Objects[objnum].flags |= OF_NO_SHIELDS;

	// Goober5000
	// (to avoid round-off errors, weapon reserve is not tested for zero)
	if (objp->flags & P_OF_NO_LASERS || Ship_info[objp->ship_class].max_weapon_reserve < WEAPON_RESERVE_THRESHOLD)
		Objects[objnum].flags |= OF_NO_LASERS;

	// Goober5000
	if (objp->flags & P_OF_NO_ENGINES || sip->max_speed == 0 )
		Objects[objnum].flags |= OF_NO_ENGINES;

	if (objp->flags & P_SF_ESCORT)
		Ships[shipnum].flags |= SF_ESCORT;

	if (objp->flags & P_KNOSSOS_WARP_IN) {
		Objects[objnum].flags |= OF_SPECIAL_WARP;
	}

	// don't set the flag if the mission is ongoing in a multiplayer situation. This will be set by the players in the
	// game only before the game or during respawning.
	// MWA -- changed the next line to remove the !(Game_mode & GM_MULTIPLAYER).  We shouldn't be setting
	// this flag in single player mode -- it gets set in post process mission.
	//if ((objp->flags & P_OF_PLAYER_START) && (((Game_mode & GM_MULTIPLAYER) && !(Game_mode & GM_IN_MISSION)) || !(Game_mode & GM_MULTIPLAYER)))
	if ( (objp->flags & P_OF_PLAYER_START) && (Fred_running || ((Game_mode & GM_MULTIPLAYER) && !(Game_mode & GM_IN_MISSION))) ) 
		Objects[objnum].flags |= OF_PLAYER_SHIP;

	if (objp->flags & P_SF_NO_ARRIVAL_MUSIC)
		Ships[shipnum].flags |= SF_NO_ARRIVAL_MUSIC;

	if ( objp->flags & P_SF_NO_ARRIVAL_WARP )
		Ships[shipnum].flags |= SF_NO_ARRIVAL_WARP;

	if ( objp->flags & P_SF_NO_DEPARTURE_WARP )
		Ships[shipnum].flags |= SF_NO_DEPARTURE_WARP;

	if ( objp->flags & P_SF_INITIALLY_DOCKED )
		Ships[shipnum].flags |= SF_INITIALLY_DOCKED;

	if ( objp->flags & P_SF_LOCKED )
		Ships[shipnum].flags |= SF_LOCKED;

	if ( objp->flags & P_SF_WARP_BROKEN )
		Ships[shipnum].flags |= SF_WARP_BROKEN;

	if ( objp->flags & P_SF_WARP_NEVER )
		Ships[shipnum].flags |= SF_WARP_NEVER;

	if ( objp->flags & P_SF_HIDDEN_FROM_SENSORS )
		Ships[shipnum].flags |= SF_HIDDEN_FROM_SENSORS;

	if ( objp->flags & P_SIF_STEALTH )
		Ship_info[Ships[shipnum].ship_info_index].flags |= SIF_STEALTH;

	if ( objp->flags & P_SIF2_FRIENDLY_STEALTH_INVISIBLE )
		Ship_info[Ships[shipnum].ship_info_index].flags2 |= SIF2_FRIENDLY_STEALTH_INVISIBLE;

	if ( objp->flags & P_SF_VAPORIZE )
		Ship_info[Ships[shipnum].ship_info_index].flags |= SF_VAPORIZE;

	// if ship is in a wing, and the wing's no_warp_effect flag is set, then set the equivalent
	// flag for the ship
	if ( (Ships[shipnum].wingnum != -1) && (Wings[Ships[shipnum].wingnum].flags & WF_NO_ARRIVAL_WARP) )
		Ships[shipnum].flags |= SF_NO_ARRIVAL_WARP;

	if ( (Ships[shipnum].wingnum != -1) && (Wings[Ships[shipnum].wingnum].flags & WF_NO_DEPARTURE_WARP) )
		Ships[shipnum].flags |= SF_NO_DEPARTURE_WARP;

	// mwa -- 1/30/98.  Do both flags.  Fred uses the ship flag, and FreeSpace will use the object
	// flag. I'm to lazy at this point to deal with consolidating them.
	if ( objp->flags & P_SF_INVULNERABLE ) {
		Ships[shipnum].flags |= SF_INVULNERABLE;
		Objects[objnum].flags |= OF_INVULNERABLE;
	}

	if ( objp->flags & P_SF_GUARDIAN ) {
		Objects[objnum].flags |= OF_GUARDIAN;
	}

	if ( objp->flags & P_SF_SCANNABLE )
		Ships[shipnum].flags |= SF_SCANNABLE;

	if ( objp->flags & P_SF_RED_ALERT_STORE_STATUS ){
		Assert(!(Game_mode & GM_MULTIPLAYER));
		Ships[shipnum].flags |= SF_RED_ALERT_STORE_STATUS;
	}

	// a couple of ai_info flags.  Also, do a reasonable default for the kamikaze damage regardless of
	// whether this flag is set or not
	if ( objp->flags & P_AIF_KAMIKAZE ) {
		Ai_info[Ships[shipnum].ai_index].ai_flags |= AIF_KAMIKAZE;
		Ai_info[Ships[shipnum].ai_index].kamikaze_damage = objp->kamikaze_damage;
	}

	if ( objp->flags & P_AIF_NO_DYNAMIC )
		Ai_info[Ships[shipnum].ai_index].ai_flags |= AIF_NO_DYNAMIC;

	// if the wing index and wing pos are set for this parse object, set them for the ship.  This
	// is useful in multiplayer when ships respawn
	Ships[shipnum].wing_status_wing_index = objp->wing_status_wing_index;
	Ships[shipnum].wing_status_wing_pos = objp->wing_status_wing_pos;

	// set up the ai_goals for this object -- all ships created here are AI controlled.
	if ( objp->ai_goals != -1 ) {
		int sexp;

		for ( sexp = CDR(objp->ai_goals); sexp != -1; sexp = CDR(sexp) )
			// make a call to the routine in MissionGoals.cpp to set up the ai goals for this object.
			ai_add_ship_goal_sexp( sexp, AIG_TYPE_EVENT_SHIP, aip );

		if ( objp->wingnum == -1 )			// free the sexpression nodes only for non-wing ships.  wing code will handle it's own case
			free_sexp2(objp->ai_goals);	// free up sexp nodes for reused, since they aren't needed anymore.
	}

	Assert(Ships[shipnum].modelnum != -1);

	// initialize subsystem statii here.  The subsystems are given a percentage damaged.  So a percent value
	// of 20% means that the subsystem is 20% damaged (*not* 20% of max hits).  This is opposite the way
	// that the initial velocity/hull strength/shields work
	i = objp->subsys_count;
	while (i--) {
		sssp = &Subsys_status[objp->subsys_index + i];
		if (!stricmp(sssp->name, NOX("Pilot"))) {
			wp = &Ships[shipnum].weapons;
			if (sssp->primary_banks[0] != SUBSYS_STATUS_NO_CHANGE) {
				for (j=k=0; j<MAX_PRIMARY_BANKS; j++) {
					if ( (sssp->primary_banks[j] >= 0) || Fred_running ){
						wp->primary_bank_weapons[k] = sssp->primary_banks[j];						

						// next
						k++;
					}
				}

				if (Fred_running){
					wp->num_primary_banks = sip->num_primary_banks;
				} else {
					wp->num_primary_banks = k;
				}
			}

			if (sssp->secondary_banks[0] != SUBSYS_STATUS_NO_CHANGE) {
				for (j=k=0; j<MAX_SECONDARY_BANKS; j++) {
					if ( (sssp->secondary_banks[j] >= 0) || Fred_running ){
						wp->secondary_bank_weapons[k++] = sssp->secondary_banks[j];
					}
				}

				if (Fred_running){
					wp->num_secondary_banks = sip->num_secondary_banks;
				} else {
					wp->num_secondary_banks = k;
				}
			}

			// primary weapons too - Goober5000
			for (j=0; j < wp->num_primary_banks; j++)
				if (Fred_running){
					wp->primary_bank_ammo[j] = sssp->primary_ammo[j];
				} else {
					int capacity = fl2i(sssp->primary_ammo[j]/100.0f * sip->primary_bank_ammo_capacity[j] + 0.5f );
					wp->primary_bank_ammo[j] = fl2i(capacity / Weapon_info[wp->primary_bank_weapons[j]].cargo_size + 0.5f);
				}

			for (j=0; j < wp->num_secondary_banks; j++)
				if (Fred_running){
					wp->secondary_bank_ammo[j] = sssp->secondary_ammo[j];
				} else {
					int capacity = fl2i(sssp->secondary_ammo[j]/100.0f * sip->secondary_bank_ammo_capacity[j] + 0.5f );
					wp->secondary_bank_ammo[j] = fl2i(capacity / Weapon_info[wp->secondary_bank_weapons[j]].cargo_size + 0.5f);
				}
			continue;
		}

		ptr = GET_FIRST(&Ships[shipnum].subsys_list);
		while (ptr != END_OF_LIST(&Ships[shipnum].subsys_list)) {
			if (!stricmp(ptr->system_info->subobj_name, sssp->name)) {
				if (Fred_running)
					ptr->current_hits = sssp->percent;
				else {
					float new_hits;
					new_hits = ptr->system_info->max_hits * (100.0f - sssp->percent) / 100.f;
					Ships[shipnum].subsys_info[ptr->system_info->type].current_hits -= (ptr->system_info->max_hits - new_hits);
					if ( (100.0f - sssp->percent) < 0.5) {
						ptr->current_hits = 0.0f;
						ptr->submodel_info_1.blown_off = 1;
					} else {
						ptr->current_hits = new_hits;
					}
				}

				if (sssp->primary_banks[0] != SUBSYS_STATUS_NO_CHANGE)
					for (j=0; j<MAX_PRIMARY_BANKS; j++)
						ptr->weapons.primary_bank_weapons[j] = sssp->primary_banks[j];

				if (sssp->secondary_banks[0] != SUBSYS_STATUS_NO_CHANGE)
					for (j=0; j<MAX_SECONDARY_BANKS; j++)
						ptr->weapons.secondary_bank_weapons[j] = sssp->secondary_banks[j];

				// Goober5000
				for (j=0; j<MAX_PRIMARY_BANKS; j++)
				{
					ptr->weapons.primary_bank_ammo[j] = sssp->primary_ammo[j];
				}

				for (j=0; j<MAX_SECONDARY_BANKS; j++) {
					// AL 3-5-98:  This is correct for FRED, but not for FreeSpace... but is this even used?
					//					As far as I know, turrets cannot run out of ammo
					ptr->weapons.secondary_bank_ammo[j] = sssp->secondary_ammo[j];
				}

				ptr->subsys_cargo_name = sssp->subsys_cargo_name;

				if (sssp->ai_class != SUBSYS_STATUS_NO_CHANGE)
					ptr->weapons.ai_class = sssp->ai_class;

				ai_turret_select_default_weapon(ptr);
			}

			ptr = GET_NEXT(ptr);
		}
	}

	// initial hull strength, shields, and velocity are all expressed as a percentage of the max value/
	// so a initial_hull value of 90% means 90% of max.  This way is opposite of how subsystems are dealt
	// with
	if (Fred_running) {
		Objects[objnum].phys_info.speed = (float) objp->initial_velocity;
		// Ships[shipnum].hull_hit_points_taken = (float) objp->initial_hull;
		Objects[objnum].hull_strength = (float) objp->initial_hull;
		Objects[objnum].shields[0] = (float) objp->initial_shields;

	} else {
		int max_allowed_sparks, num_sparks, i;
		polymodel *pm;

		// Ships[shipnum].hull_hit_points_taken = (float)objp->initial_hull * sip->max_hull_hit_points / 100.0f;
		Objects[objnum].hull_strength = objp->initial_hull * sip->initial_hull_strength / 100.0f;
		for (i = 0; i<MAX_SHIELD_SECTIONS; i++)
			Objects[objnum].shields[i] = (float)(objp->initial_shields * sip->shields / 100.0f) / MAX_SHIELD_SECTIONS;

		// initial velocities now do not apply to ships which warp in after mission starts
		if ( !(Game_mode & GM_IN_MISSION) ) {
			Objects[objnum].phys_info.speed = (float)objp->initial_velocity * sip->max_speed / 100.0f;
			Objects[objnum].phys_info.vel.xyz.z = Objects[objnum].phys_info.speed;
			Objects[objnum].phys_info.prev_ramp_vel = Objects[objnum].phys_info.vel;
			Objects[objnum].phys_info.desired_vel = Objects[objnum].phys_info.vel;
		}

		// recalculate damage of subsystems
		ship_recalc_subsys_strength( &Ships[shipnum] );

		// create sparks on a ship whose hull is damaged.  We will create two sparks for every 20%
		// of hull damage done.  100 means no sparks.  between 80 and 100 do two sparks.  60 and 80 is
		// four, etc.
		pm = model_get( sip->modelnum );
		max_allowed_sparks = get_max_sparks(&Objects[objnum]);
		num_sparks = (int)((100.0f - objp->initial_hull) / 5.0f);
		if (num_sparks > max_allowed_sparks) {
			num_sparks = max_allowed_sparks;
		}

		for (i = 0; i < num_sparks; i++ ) {
			vector v1, v2;

			// DA 10/20/98 - sparks must be chosen on the hull and not any submodel
			submodel_get_two_random_points(sip->modelnum, pm->detail[0], &v1, &v2);
			ship_hit_sparks_no_rotate(&Objects[objnum], &v1);
//			ship_hit_sparks_no_rotate(&Objects[objnum], &v2);
		}
	}

	// in mission, we add a log entry -- set ship positions for ships not in wings, and then do
	// warpin effect
	if ( (Game_mode & GM_IN_MISSION) && (!Fred_running) ) {
		mission_log_add_entry( LOG_SHIP_ARRIVE, Ships[shipnum].ship_name, NULL );

		// if this ship isn't in a wing, determine it's arrival location
		if ( !Game_restoring )	{
			if ( Ships[shipnum].wingnum == -1 ) {
				int location;
				// multiplayer clients set the arrival location of ships to be at location since their
				// position has already been determined.  Don't actually set the variable since we
				// don't want the warp effect to show if coming from a dock bay.
				location = objp->arrival_location;
#ifndef NO_NETWORK
				if ( MULTIPLAYER_CLIENT )
					location = ARRIVE_AT_LOCATION;
#endif
				mission_set_arrival_location(objp->arrival_anchor, location, objp->arrival_distance, objnum, NULL, NULL);
				if ( objp->arrival_location != ARRIVE_FROM_DOCK_BAY )
					shipfx_warpin_start( &Objects[objnum] );
			}
		}
		
		// possibly add this ship to a hotkey set
		if ( (Ships[shipnum].wingnum == -1) && (Ships[shipnum].hotkey != -1 ) )
			mission_hotkey_mf_add( Ships[shipnum].hotkey, Ships[shipnum].objnum, HOTKEY_MISSION_FILE_ADDED );
		else if ( (Ships[shipnum].wingnum != -1) && (Wings[Ships[shipnum].wingnum].hotkey != -1 ) )
			mission_hotkey_mf_add( Wings[Ships[shipnum].wingnum].hotkey, Ships[shipnum].objnum, HOTKEY_MISSION_FILE_ADDED );

		// possibly add this ship to the hud escort list
		if ( Ships[shipnum].flags & SF_ESCORT ){
			hud_add_remove_ship_escort( objnum, 1 );
		}
	}

#ifndef NO_NETWORK
	// for multiplayer games, make a call to the network code to assign the object signature
	// of the newly created object.  The network host of the netgame will always assign a signature
	// to a newly created object.  The network signature will get to the clients of the game in
	// different manners depending on whether or not an individual ship or a wing was created.
	if ( Game_mode & GM_MULTIPLAYER ) {
		Objects[objnum].net_signature = objp->net_signature;

		if ( (Game_mode & GM_IN_MISSION) && MULTIPLAYER_MASTER && (objp->wingnum == -1) ){
			send_ship_create_packet( &Objects[objnum], (objp==Arriving_support_ship)?1:0 );
		}
	}
#endif

	// if recording a demo, post the event
	if(Game_mode & GM_DEMO_RECORD){
		demo_POST_obj_create(objp->name, Objects[objnum].signature);
	}

	return objnum;
}

//	Mp points at the text of an object, which begins with the "$Name:" field.
//	Snags all object information and calls parse_create_object to create a ship.
//	Why create a ship?  Why not an object?  Stay tuned...
//
// flag is parameter that is used to tell what kind information we are retrieving from the mission.
// if we are just getting player starts, then don't create the objects
int parse_object(mission *pm, int flag, p_object *objp)
{
	// p_object	temp_object;
	// p_object	*objp;
	int	i, j, count, shipnum, delay, destroy_before_mission_time;
	char	name[NAME_LENGTH], flag_strings[MAX_PARSE_OBJECT_FLAGS][NAME_LENGTH];

	Assert(pm != NULL);

	// objp = &temp_object;

	required_string("$Name:");
	stuff_string(objp->name, F_NAME, NULL);
	shipnum = ship_name_lookup(objp->name);
	if (shipnum != -1)
		error_display(0, NOX("Redundant ship name: %s\n"), objp->name);


	find_and_stuff("$Class:", &objp->ship_class, F_NAME, Ship_class_names, Num_ship_types, "ship class");
	if (objp->ship_class < 0) {
		Warning(LOCATION, "Ship \"%s\" has an invalid ship type (ships.tbl probably changed).  Making it type 0", objp->name);

		// if fred is running, maybe notify the user that the mission contains MD content
		if(Fred_running){
			Fred_found_unknown_ship_during_parsing = 1;
		}

		objp->ship_class = 0;
	}

#ifndef NO_NETWORK
	// if this is a multiplayer dogfight mission, skip support ships
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && (Ship_info[objp->ship_class].flags & SIF_SUPPORT)){
		return 0;
	}		
#endif

	// optional alternate name type
	objp->alt_type_index = -1;
	if(optional_string("$Alt:")){
		// alternate name
		stuff_string(name, F_NAME, NULL, NAME_LENGTH);

		// try and find the alternate name
		objp->alt_type_index = (char)mission_parse_lookup_alt(name);
		Assert(objp->alt_type_index >= 0);
		if(objp->alt_type_index < 0){
			mprintf(("Error looking up alternate ship type name!\n"));
		} else {
			mprintf(("Using alternate ship type name : %s\n", name));
		}
	}

	int	team_index;
	find_and_stuff("$Team:", &team_index, F_NAME, Team_names, Num_team_names, "team name");
	Assert((team_index >= 0) && (team_index < MAX_TEAM_NAMES));
	objp->team = 1 << team_index;

	required_string("$Location:");
	stuff_vector(&objp->pos);

	required_string("$Orientation:");
	stuff_matrix(&objp->orient);

	find_and_stuff("$IFF:", 		&objp->iff, F_NAME, Iff_names, Num_iff, "IFF");
	find_and_stuff("$AI Behavior:",	&objp->behavior, F_NAME, Ai_behavior_names, Num_ai_behaviors, "AI behavior");
	objp->ai_goals = -1;

	if ( optional_string("+AI Class:")) {
		objp->ai_class = match_and_stuff(F_NAME, Ai_class_names, Num_ai_classes, "AI class");
		Assert(objp->ai_class > -1 );
	} else {
		objp->ai_class = Ship_info[objp->ship_class].ai_class;
	}

	if ( optional_string("$AI Goals:") ){
		objp->ai_goals = get_sexp_main();
	}

	if ( !required_string_either("$AI Goals:", "$Cargo 1:") ) {
		required_string("$AI Goals:");
		objp->ai_goals = get_sexp_main();
	}

	objp->cargo1 = -1;
	int temp;
	find_and_stuff_or_add("$Cargo 1:", &temp, F_NAME, Cargo_names, &Num_cargo, MAX_CARGO, "cargo");
	objp->cargo1 = char(temp);
	if ( optional_string("$Cargo 2:") ) {
		char buf[NAME_LENGTH];
		stuff_string(buf, F_NAME, NULL);
	}

	parse_common_object_data(objp);  // get initial conditions and subsys status
	count = 0;
	while (required_string_either("$Arrival Location:", "$Status Description:"))	{
		Assert(count < MAX_OBJECT_STATUS);

		find_and_stuff("$Status Description:", &objp->status_type[count], F_NAME, Status_desc_names, Num_status_names, "Status Description");
		find_and_stuff("$Status:", &objp->status[count], F_NAME, Status_type_names, Num_status_names, "Status Type");
		find_and_stuff("$Target:", &objp->target[count], F_NAME, Status_target_names, Num_status_names, "Target");
		count++;
	}
	objp->status_count = count;

	objp->arrival_anchor = -1;
	objp->arrival_distance = 0;
	find_and_stuff("$Arrival Location:", &objp->arrival_location, F_NAME, Arrival_location_names, Num_arrival_names, "Arrival Location");
	if ( optional_string("+Arrival Distance:") ) {
		stuff_int( &objp->arrival_distance );
		if ( objp->arrival_location != ARRIVE_AT_LOCATION ) {
			required_string("$Arrival Anchor:");
			stuff_string(name, F_NAME, NULL);
			objp->arrival_anchor = get_anchor(name);
		}
	}

	if (optional_string("+Arrival Delay:")) {
		stuff_int(&delay);
		if ( delay < 0 )
			Error(LOCATION, "Cannot have arrival delay < 0 (ship %s)", objp->name);
	} else
		delay = 0;

	if ( !Fred_running ){
		objp->arrival_delay = -delay;			// use negative numbers to mean we haven't set up a timer yet
	} else {
		objp->arrival_delay = delay;
	}

	required_string("$Arrival Cue:");
	objp->arrival_cue = get_sexp_main();
	if ( !Fred_running && (objp->arrival_cue >= 0) ) {
		// eval the arrival cue.  if the cue is true, set up the timestamp for the arrival delay
		Assert ( objp->arrival_delay <= 0 );

		// don't eval arrival_cues when just looking for player information.
		if ( eval_sexp(objp->arrival_cue) ){			// evaluate to determine if sexp is always false.
			objp->arrival_delay = timestamp( -objp->arrival_delay * 1000 );
		}
	}

	find_and_stuff("$Departure Location:", &objp->departure_location, F_NAME, Departure_location_names, Num_arrival_names, "Departure Location");
	objp->departure_anchor = -1;
	if ( objp->departure_location == DEPART_AT_DOCK_BAY ) {
		required_string("$Departure Anchor:");
		stuff_string(name, F_NAME, NULL);
		objp->departure_anchor = get_anchor(name);
	}

	if (optional_string("+Departure Delay:")) {
		stuff_int(&delay);
		if ( delay < 0 ){
			Error(LOCATION, "Cannot have departure delay < 0 (ship %s)", objp->name);
		}
	} else {
		delay = 0;
	}

	if ( !Fred_running ){
		objp->departure_delay = -delay;
	} else {
		objp->departure_delay = delay;
	}

	required_string("$Departure Cue:");
	objp->departure_cue = get_sexp_main();

	if (optional_string("$Misc Properties:"))
		stuff_string(objp->misc, F_NAME, NULL);

	required_string("$Determination:");
	stuff_int(&objp->determination);

	objp->flags = 0;
	if (optional_string("+Flags:")) {
		count = stuff_string_list(flag_strings, MAX_PARSE_OBJECT_FLAGS);
		for (i=0; i<count; i++) {
			for (j=0; j<MAX_PARSE_OBJECT_FLAGS; j++) {
				if (!stricmp(flag_strings[i], Parse_object_flags[j])) {
					objp->flags |= (1 << j);
					break;
				}
			}

			if (j == MAX_PARSE_OBJECT_FLAGS)
				Warning(LOCATION, "Unknown flag in mission file: %s\n", flag_strings[i]);
		}
	}

	// always store respawn priority, just for ease of implementation
	objp->respawn_priority = 0;
	if(optional_string("+Respawn Priority:" )){
		stuff_int(&objp->respawn_priority);	
	}

	objp->escort_priority = 0;
	if ( optional_string("+Escort Priority:" ) ) {
		Assert(objp->flags & P_SF_ESCORT);
		stuff_int(&objp->escort_priority);
	}	

	if ( objp->flags & P_OF_PLAYER_START ) {
		objp->flags |= P_OF_CARGO_KNOWN;				// make cargo known for players
		Player_starts++;
	}

	objp->special_exp_index = -1;
	if ( optional_string("+Special Exp index:" ) ) {
		stuff_int(&objp->special_exp_index);
	}

	// if the kamikaze flag is set, we should have the next flag
	if ( optional_string("+Kamikaze Damage:") ) {
		int damage;

		stuff_int(&damage);
		objp->kamikaze_damage = i2fl(damage);
	}

	objp->hotkey = -1;
	if (optional_string("+Hotkey:")) {
		stuff_int(&objp->hotkey);
		Assert((objp->hotkey >= 0) && (objp->hotkey < 10));
	}

	objp->docked_with[0] = 0;
	if (optional_string("+Docked With:")) {
		stuff_string(objp->docked_with, F_NAME, NULL);
		required_string("$Docker Point:");
		stuff_string(objp->docker_point, F_NAME, NULL);
		required_string("$Dockee Point:");
		stuff_string(objp->dockee_point, F_NAME, NULL);

		objp->flags |= P_SF_INITIALLY_DOCKED;

		// put this information into the Initially_docked array.  We will need to use this
		// informatin later since not all ships will initially get created.
		strcpy(Initially_docked[Total_initially_docked].dockee, objp->docked_with);
		strcpy(Initially_docked[Total_initially_docked].docker_point, objp->docker_point);
		strcpy(Initially_docked[Total_initially_docked].dockee_point, objp->dockee_point);
		Initially_docked[Total_initially_docked].docker = objp;
		Total_initially_docked++;
	}

	// check the optional parameter for destroying the ship before the mission starts.  If this parameter is
	// here, then we need to destroy the ship N seconds before the mission starts (for debris purposes).
	// store the time value here.  We want to create this object for sure.  Set the arrival cue and arrival
	// delay to bogus values
	destroy_before_mission_time = -1;
	if ( optional_string("+Destroy At:") ) {

		stuff_int(&destroy_before_mission_time);
		Assert ( destroy_before_mission_time >= 0 );
		objp->arrival_cue = Locked_sexp_true;
		objp->arrival_delay = timestamp(0);
	}

	// check for the optional "orders accepted" string which contains the orders from the default
	// set that this ship will actually listen to
	if ( optional_string("+Orders Accepted:") ) {
		stuff_int( &objp->orders_accepted );
		if ( objp->orders_accepted != -1 ){
			objp->flags |= P_SF_USE_UNIQUE_ORDERS;
		}
	}

	if (optional_string("+Group:")){
		stuff_int(&objp->group);
	} else {
		objp->group = 0;
	}

	if (optional_string("+Score:")){
		stuff_int(&objp->score);
	} else {
		objp->score = 0;
	}

	// parse the persona index if present
	if ( optional_string("+Persona Index:")){
		stuff_int(&objp->persona_index);
	} else {
		objp->persona_index = -1;
	}

	if ( optional_string("+nameplate:")){
		char tempname[64];
		stuff_string(tempname, F_NAME, NULL);
		mprintf(("ship %s should have nameplate texture %s",objp->name,tempname));
		objp->nameplate = bm_load(tempname);
	
		if(objp->nameplate >-1)
			mprintf((" and it does, number %d\n",objp->nameplate));
		else
			mprintf((" but it failed :(\n"));

	} else {
		objp->persona_index = -1;
	}


	objp->wingnum = -1;					// set the wing number to -1 -- possibly to be set later

#ifndef NO_NETWORK
	// for multiplayer, assign a network signature to this parse object.  Doing this here will
	// allow servers to use the signature with clients when creating new ships, instead of having
	// to pass ship names all the time
	if ( Game_mode & GM_MULTIPLAYER ){
		objp->net_signature = multi_assign_network_signature( MULTI_SIG_SHIP );
	}
#endif

	// set the wing_status position to be -1 for all objects.  This will get set to an appropriate
	// value when the wing positions are finally determined.
	objp->wing_status_wing_index = -1;
	objp->wing_status_wing_pos = -1;
	objp->respawn_count = 0;

	// if this if the starting player ship, then copy if to Starting_player_pobject (used for ingame join)
	if ( !stricmp( objp->name, Player_start_shipname) ) {
		Player_start_pobject = *objp;
		Player_start_pobject.flags |= P_SF_PLAYER_START_VALID;
	}
	

// Now create the object.
// Don't create the new ship blindly.  First, check the sexp for the arrival cue
// to determine when this ship should arrive.  If not right away, stick this ship
// onto the ship arrival list to be looked at later.  Also check to see if it should use the
// wings arrival cue.  The ship may get created later depending on whether or not the wing
// is created.
// always create ships when FRED is running

	// don't create the object if it is intially docked for either FreeSpcae or Fred.  Fred will
	// create the object later in post_process_mission
	if ( (objp->flags & P_SF_INITIALLY_DOCKED) || (!Fred_running && (!eval_sexp(objp->arrival_cue) || !timestamp_elapsed(objp->arrival_delay) || (objp->flags & P_SF_REINFORCEMENT))) ) {
		Assert ( destroy_before_mission_time == -1 );		// we can't add ships getting destroyed to the arrival list!!!
		Assert ( num_ship_arrivals < MAX_SHIP_ARRIVALS );
		memcpy( &ship_arrivals[num_ship_arrivals], objp, sizeof(p_object) );
		list_append(&ship_arrival_list, &ship_arrivals[num_ship_arrivals]);
		num_ship_arrivals++;
	}
#ifndef NO_NETWORK
	// ingame joiners bail here.
	else if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_INGAME_JOIN)){
		return 1;
	}
#endif
	else {
		int	real_objnum;

		real_objnum = parse_create_object(objp);				// this object may later get destroyed depending on wing status!!!!

		Subsys_index = objp->subsys_index;  // free elements that are no longer needed.

		// if the ship is supposed to be destroyed before the mission, then blow up the ship, mark the pieces
		// as last forever.  Only call this stuff when you are blowing up the ship
		if ( destroy_before_mission_time >= 0 ) {
			object *objp;

			objp = &Objects[real_objnum];
			if ( !Fred_running ) {
				int i;
				shipfx_blow_up_model( objp, Ships[objp->instance].modelnum, 0, 0, &objp->pos );
				objp->flags |= OF_SHOULD_BE_DEAD;

				// once the ship is exploded, find the debris pieces belonging to this object, mark them
				// as not to expire, and move them forward in time N seconds
				for (i = 0; i < MAX_DEBRIS_PIECES; i++ ) {
					debris *db;

					db = &Debris[i];
					if ( !db->flags & DEBRIS_USED )				// not used, move onto the next one.
						continue;
					if ( db->source_objnum != real_objnum )	// not from this ship, move to next one
						continue;

					debris_clear_expired_flag(db);				// mark as don't expire
					db->lifeleft = -1.0f;							// be sure that lifeleft == -1.0 so that it really doesn't expire!

					// now move the debris along it's path for N seconds
					objp = &Objects[db->objnum];
					physics_sim( &objp->pos, &objp->orient, &objp->phys_info, (float)destroy_before_mission_time );
				}
			} else  {
				// be sure to set the variable in the ships structure for the final death time!!!
				Ships[objp->instance].final_death_time = destroy_before_mission_time;
				Ships[objp->instance].flags |= SF_KILL_BEFORE_MISSION;
			}
		}
	}

	return 1;
}

void parse_common_object_data(p_object	*objp)
{
	int i;

	// set some defaults..
	objp->initial_velocity = 0;
	objp->initial_hull = 100;
	objp->initial_shields = 100;

	// now change defaults if present
	if (optional_string("+Initial Velocity:")) {
		stuff_int(&objp->initial_velocity);
	}

	if (optional_string("+Initial Hull:"))
		stuff_int(&objp->initial_hull);
	if (optional_string("+Initial Shields:"))
		stuff_int(&objp->initial_shields);

	objp->subsys_index = Subsys_index;
	objp->subsys_count = 0;
	while (optional_string("+Subsystem:")) {
		i = allocate_subsys_status();

		objp->subsys_count++;
		stuff_string(Subsys_status[i].name, F_NAME, NULL);
		
		if (optional_string("$Damage:"))
			stuff_float(&Subsys_status[i].percent);

		Subsys_status[i].subsys_cargo_name = -1;
		if (optional_string("+Cargo Name:")) {
			char cargo_name[256];
			stuff_string(cargo_name, F_NAME, NULL);
			int index = string_lookup(cargo_name, Cargo_names, Num_cargo, "cargo", 0);
			if (index == -1 && (Num_cargo < MAX_CARGO)) {
				index = Num_cargo;
				strcpy(Cargo_names[Num_cargo++], cargo_name);
			}
			Subsys_status[i].subsys_cargo_name = index;
		}

		if (optional_string("+AI Class:"))
			Subsys_status[i].ai_class = match_and_stuff(F_NAME, Ai_class_names, Num_ai_classes, "AI class");

		if (optional_string("+Primary Banks:"))
			stuff_int_list(Subsys_status[i].primary_banks, MAX_PRIMARY_BANKS, WEAPON_LIST_TYPE);

		// Goober5000
		if (optional_string("+Pbank Ammo:"))
			stuff_int_list(Subsys_status[i].primary_ammo, MAX_PRIMARY_BANKS, RAW_INTEGER_TYPE);

		if (optional_string("+Secondary Banks:"))
			stuff_int_list(Subsys_status[i].secondary_banks, MAX_SECONDARY_BANKS, WEAPON_LIST_TYPE);

		if (optional_string("+Sbank Ammo:"))
			stuff_int_list(Subsys_status[i].secondary_ammo, MAX_SECONDARY_BANKS, RAW_INTEGER_TYPE);

	}
}

void parse_objects(mission *pm, int flag)
{	
	p_object temp;

	Assert(pm != NULL);	

	required_string("#Objects");	

	// parse in objects
	num_ship_original = 0;
	while (required_string_either("#Wings", "$Name:")){
		// not all objects are always valid or legal
		if(parse_object(pm, flag, &temp)){
			// add to the default list
			if(num_ship_original < MAX_SHIP_ORIGINAL){
				memcpy(&ship_original[num_ship_original++], &temp, sizeof(p_object));
			}
		}
	}
}

p_object *mission_parse_get_original_ship( ushort net_signature )
{
	int idx;

	// look for original ships
	for(idx=0; idx<num_ship_original; idx++){
		if(ship_original[idx].net_signature == net_signature){
			return &ship_original[idx];
		}
	}

	// boo
	return NULL;
}

int find_wing_name(char *name)
{
	int	i;

	for (i=0; i<num_wings; i++){
		if (!strcmp(name, Wings[i].name)){
			return i;
		}
	}

	return -1;
}

// function to create ships in the wing that need to be created.  We psas the wing pointer, it's index
// into the Wings array
int parse_wing_create_ships( wing *wingp, int num_to_create, int force, int specific_instance )
{
	p_object *objp;
	int wingnum, objnum, num_create_save;
	int time_to_arrive;
	int pre_create_count;

	// we need to send this in multiplayer
	pre_create_count = wingp->total_arrived_count;

	// force is used to force creation of the wing -- used for multiplayer
	if ( !force ) {
		// we only want to evaluate the arrival cue of the wing if:
		// 1) single player
		// 2) multiplayer and I am the host of the game
		// can't create any ships if the arrival cue is false or the timestamp has not elapsed.

		if ( !eval_sexp(wingp->arrival_cue) ) /* || !timestamp_elapsed(wingp->arrival_delay) ) */
			return 0;

		// once the sexpressions becomes true, then check the arrival delay on the wing.  The first time, the
		// arrival delay will be <= 0 meaning that no timer object has been set yet.  Set up the timestamp
		// which should always give a number >= 0;
		if ( wingp->arrival_delay <= 0 ) {
			wingp->arrival_delay = timestamp( -wingp->arrival_delay * 1000 );
			Assert ( wingp->arrival_delay >= 0 );
		}

		if ( !timestamp_elapsed( wingp->arrival_delay ) )
			return 0;

		// if wing is coming from docking bay, then be sure that ship we are arriving from actually exists
		// (or will exist).
		if ( wingp->arrival_location == ARRIVE_FROM_DOCK_BAY ) {
			int shipnum;
			char *name;

			Assert( wingp->arrival_anchor >= 0 );
			name = Parse_names[wingp->arrival_anchor];

			// see if ship is yet to arrive.  If so, then return -1 so we can evaluate again later.
			if ( mission_parse_get_arrival_ship( name ) )
				return 0;

			// see if ship is in mission.  If not, then we can assume it was destroyed or departed since
			// it is not on the arrival list (as shown by above if statement).
			shipnum = ship_name_lookup( name );
			if ( shipnum == -1 ) {
				int num_remaining;
				// since this wing cannot arrive from this place, we need to mark the wing as destroyed and
				// set the wing variables appropriatly.  Good for directives.

				// set the gone flag
				wingp->flags |= WF_WING_GONE;

				// if the current wave is zero, it never existed
				wingp->flags |= WF_NEVER_EXISTED;

				// mark the number of waves and number of ships destroyed equal to the last wave and the number
				// of ships yet to arrive
				num_remaining = ( (wingp->num_waves - wingp->current_wave) * wingp->wave_count);
				wingp->total_arrived_count += num_remaining;
				wingp->current_wave = wingp->num_waves;

				// replaced following three lines of code with mission log call because of bug with
				// the Ships_exited list.
				//index = ship_find_exited_ship_by_name( name );
				//Assert( index != -1 );
				//if (Ships_exited[index].flags & SEF_DESTROYED ) {
				if ( mission_log_get_time(LOG_SHIP_DESTROYED, name, NULL, NULL) ) {
					wingp->total_destroyed += num_remaining;
				} else {
					wingp->total_departed += num_remaining;
				}

				Sexp_nodes[wingp->arrival_cue].value = SEXP_KNOWN_FALSE;
				return 0;
			}
		}

		if ( num_to_create == 0 )
			return 0;

		// check the wave_delay_timestamp field.  If it is not valid, make it valid (based on wave delay min
		// and max valuds).  If it is valid, and not elapsed, then return.  If it is valid and elasped, then
		// continue on.
		if ( !timestamp_valid(wingp->wave_delay_timestamp) ) {

			// if at least one of these is valid, then reset the timestamp.  If they are both zero, we will create the
			// wave
			if ( (wingp->wave_delay_min > 0) || (wingp->wave_delay_max > 0) ) {
				Assert ( wingp->wave_delay_min <= wingp->wave_delay_max );
				time_to_arrive = wingp->wave_delay_min + (int)(frand() * (wingp->wave_delay_max - wingp->wave_delay_min));

				// MWA -- 5/18/98
				// HACK HACK -- in the presense of Mike Comet and Mitri, I have introduced one of the most
				// serious breaches of coding standards.  I'm to lazy to fix this the correct way.  Insert
				// a delay before the next wave of the wing can arrive to that clients in the game have ample
				// time to kill off any ships in the wing before the next wave arrives.
				if ( Game_mode & GM_MULTIPLAYER ){
					time_to_arrive += 7;
				}
				wingp->wave_delay_timestamp = timestamp(time_to_arrive * 1000);
				return 0;
			}

			// if we get here, both min and max values are 0;  See comments above for a most serious hack
			time_to_arrive = 0;
			if ( Game_mode & GM_MULTIPLAYER )
				time_to_arrive += 7;
			time_to_arrive *= 1000;
			wingp->wave_delay_timestamp = timestamp(time_to_arrive);
		}

		// now check to see if the wave_delay_timestamp is elapsed or not
		if ( !timestamp_elapsed(wingp->wave_delay_timestamp) )
			return 0;
	}

	// finally we can create the wing.

	num_create_save = num_to_create;

	wingnum = wingp - Wings;					// get the wing number

	// if there are no ships to create, then all ships must be player start ships -- do nothing in this case.
	if ( num_to_create == 0 ){
		return 0;
	}

	wingp->current_wave++;						// we are creating new ships
	// we need to create num_to_create ships.  Since the arrival cues for ships in a wing
	// are ignored, then *all* ships must be in the ship_arrival_list.  

	objnum = -1;
	objp = GET_FIRST(&ship_arrival_list);
	while( objp != END_OF_LIST(&ship_arrival_list) )	{
		p_object *temp = GET_NEXT(objp);

		// compare the wingnums.  When they are equal, we can create the ship.  In the case of
		// wings that have multiple waves, this code implies that we essentially creating clones
		// of the ships that were created in Fred for the wing when more ships for a new wave
		// arrive.  The threshold value of a wing can also make one of the ships in a wing be "cloned"
		// more often than other ships in the wing.  I don't think this matters much.
		if ( objp->wingnum == wingnum ) {
			ai_info *aip;

			// when ingame joining, we need to create a specific ship out of the list of ships for a
			// wing.  specific_instance is a 0 based integer which specified which ship in the wing
			// to create.  So, only create the ship we actually need to.
			if ( (Game_mode & GM_MULTIPLAYER) && (specific_instance > 0) ) {
				specific_instance--;
				objp = temp;
				continue;
			}

			Assert ( !(objp->flags & P_SF_CANNOT_ARRIVE) );		// get allender

			int index;

			// if we have the maximum number of ships in the wing, we must bail as well
			if ( wingp->current_count >= MAX_SHIPS_PER_WING ) {
				Int3();					// this is bogus -- we should always allow all ships to be created
				num_to_create = 0;
				break;
			}

			// bash the ship name to be the name of the wing + sone number if there is > 1 wave in
			// this wing
			// also, if multplayer, set the parse object's net signature to be wing's net signature
			// base + total_arrived_count (before adding 1)
#ifndef NO_NETWORK
			if ( Game_mode & GM_MULTIPLAYER ){
				objp->net_signature = (ushort)(wingp->net_signature + wingp->total_arrived_count);
			}
#endif

			wingp->total_arrived_count++;
			if ( wingp->num_waves > 1 ){
				sprintf(objp->name, NOX("%s %d"), wingp->name, wingp->total_arrived_count);
			}

			objnum = parse_create_object(objp);
			aip = &Ai_info[Ships[Objects[objnum].instance].ai_index];

			// copy any goals from the wing to the newly created ship
			for (index = 0; index < MAX_AI_GOALS; index++) {
				if ( wingp->ai_goals[index].ai_mode != AI_GOAL_NONE ){
					ai_copy_mission_wing_goal( &wingp->ai_goals[index], aip );
				}
			}

			Ai_info[Ships[Objects[objnum].instance].ai_index].wing = wingnum;

			if ( wingp->flags & WF_NO_DYNAMIC ){
				aip->ai_flags |= AIF_NO_DYNAMIC;
			}

			// update housekeeping variables
			wingp->ship_index[wingp->current_count] = Objects[objnum].instance;

			// set up wingman status index
			hud_wingman_status_set_index(wingp->ship_index[wingp->current_count]);

			objp->wing_status_wing_index = Ships[Objects[objnum].instance].wing_status_wing_index;
			objp->wing_status_wing_pos = Ships[Objects[objnum].instance].wing_status_wing_pos;

			wingp->current_count++;

			// keep any player ship on the parse object list -- used for respawns
			// 5/8/98 -- MWA -- don't remove ships from the list when you are ingame joining
			if ( !(objp->flags & P_OF_PLAYER_START) ) {
#ifndef NO_NETWORK
				if ( (Game_mode & GM_NORMAL) || !(Net_player->flags & NETINFO_FLAG_INGAME_JOIN) ) {
#else
				if ( (Game_mode & GM_NORMAL) ) {
#endif
					if ( wingp->num_waves == wingp->current_wave ) {	// only remove ship if one wave in wing
						list_remove( &ship_arrival_list, objp);			// remove objp from the list
						if ( objp->ai_goals != -1 ){
							free_sexp2(objp->ai_goals);						// free up sexp nodes for reuse
						}
					}
				}
			}

#ifndef NO_NETWORK
			// flag ship with SF_FROM_PLAYER_WING if a member of player starting wings
			if ( (Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM) ) {
				// but for team vs. team games, then just check the alpha and zeta wings
				if ( !(stricmp(Starting_wing_names[STARTING_WING_ALPHA], wingp->name)) || !(stricmp(Starting_wing_names[STARTING_WING_ZETA], wingp->name)) ) {
					Ships[Objects[objnum].instance].flags |= SF_FROM_PLAYER_WING;
				}
			} 
			else
#endif
			{
				for (int i = 0; i < MAX_STARTING_WINGS; i++ ) {
					if ( !stricmp(Starting_wing_names[i], wingp->name) ) {
						Ships[Objects[objnum].instance].flags |= SF_FROM_PLAYER_WING;
					} 
				}
			}

			// keep track of how many ships to create.  Stop when we have done all that we are supposed
			// to do.
			num_to_create--;
			if ( !num_to_create ){
				break;
			}
		}
		objp = temp;
	}

	Assert ( num_to_create == 0 );		// we should always have enough ships in the list!!!

	// possibly play some event driven music here.  Send a network packet indicating the wing was
	// created.  Only do this stuff if actually in the mission.
	if ( (objnum != -1) && (Game_mode & GM_IN_MISSION) ) {		// if true, we have created at least one new ship.
		int i, ship_num;

		// see if this wing is a player starting wing, and if so, call the maybe_add_form_goal
		// function to possibly make the wing form on the player
		for (i = 0; i < MAX_STARTING_WINGS; i++ ) {
			if ( Starting_wings[i] == wingnum ){
				break;
			}
		}
		if ( i < MAX_STARTING_WINGS ){
			ai_maybe_add_form_goal( wingp );
		}

		mission_log_add_entry( LOG_WING_ARRIVE, wingp->name, NULL, wingp->current_wave );
		ship_num = wingp->ship_index[0];

		if ( !(Ships[ship_num].flags & SF_NO_ARRIVAL_MUSIC) && !(wingp->flags & WF_NO_ARRIVAL_MUSIC) ) {
			if ( timestamp_elapsed(Allow_arrival_music_timestamp) ) {
				Allow_arrival_music_timestamp = timestamp(ARRIVAL_MUSIC_MIN_SEPARATION);
				event_music_arrival(Ships[ship_num].team);	
			}
		}

		// possibly change the location where these ships arrive based on the wings arrival location
		mission_set_wing_arrival_location( wingp, num_create_save );

#ifndef NO_NETWORK
		// if in multiplayer (and I am the host) and in the mission, send a wing create command to all
		// other players
		if ( MULTIPLAYER_MASTER ){
			send_wing_create_packet( wingp, num_create_save, pre_create_count );
		}
#endif

#ifndef NDEBUG
		// test code to check to be sure that all ships in the wing are ignoring the same types
		// of orders from the player
		if ( Fred_running ) {
			Assert( wingp->ship_index[0] != -1 );
			int orders = Ships[wingp->ship_index[0]].orders_accepted;
			for (i = 1; i < wingp->current_count; i++ ) {
				if ( orders != Ships[wingp->ship_index[i]].orders_accepted ) {
					Warning(LOCATION, "ships in wing %s are ignoring different player orders.  Please find Mark A\nto talk to him about this.", wingp->name );
					break;
				}
			}
		}
#endif

	}

	wingp->wave_delay_timestamp = timestamp(-1);		// we will need to set this up properly for the next wave
	return num_create_save;
}

void parse_wing(mission *pm)
{
	int wingnum, i, wing_goals, delay;
	char name[NAME_LENGTH], ship_names[MAX_SHIPS_PER_WING][NAME_LENGTH];
	char wing_flag_strings[MAX_WING_FLAGS][NAME_LENGTH];
	wing *wingp;

	Assert(pm != NULL);
	wingp = &Wings[num_wings];

	required_string("$Name:");
	stuff_string(wingp->name, F_NAME, NULL);
	wingnum = find_wing_name(wingp->name);
	if (wingnum != -1)
		error_display(0, NOX("Redundant wing name: %s\n"), wingp->name);
	wingnum = num_wings;

	wingp->total_arrived_count = 0;
	wingp->total_destroyed = 0;
	wingp->flags = 0;

	required_string("$Waves:");
	stuff_int(&wingp->num_waves);
	Assert ( wingp->num_waves >= 1 );		// there must be at least 1 wave

	wingp->current_wave = 0;

	required_string("$Wave Threshold:");
	stuff_int(&wingp->threshold);

	required_string("$Special Ship:");
	stuff_int(&wingp->special_ship);

	wingp->arrival_anchor = -1;
	find_and_stuff("$Arrival Location:", &wingp->arrival_location, F_NAME, Arrival_location_names, Num_arrival_names, "Arrival Location");
	if ( optional_string("+Arrival Distance:") ) {
		stuff_int( &wingp->arrival_distance );
		if ( wingp->arrival_location != ARRIVE_AT_LOCATION ) {
			required_string("$Arrival Anchor:");
			stuff_string(name, F_NAME, NULL);
			wingp->arrival_anchor = get_anchor(name);
		}
	}

	if (optional_string("+Arrival delay:")) {
		stuff_int(&delay);
		if ( delay < 0 )
			Error(LOCATION, "Cannot have arrival delay < 0 on wing %s", wingp->name );
	} else
		delay = 0;

	if ( !Fred_running ){
		wingp->arrival_delay = -delay;
	} else {
		wingp->arrival_delay = delay;
	}

	required_string("$Arrival Cue:");
	wingp->arrival_cue = get_sexp_main();
	if ( !Fred_running && (wingp->arrival_cue >= 0) ) {
		if ( eval_sexp(wingp->arrival_cue) )			// evaluate to determine if sexp is always false.
			wingp->arrival_delay = timestamp( -wingp->arrival_delay * 1000 );
	}

	
	find_and_stuff("$Departure Location:", &wingp->departure_location, F_NAME, Departure_location_names, Num_arrival_names, "Departure Location");
	wingp->departure_anchor = -1;
	if ( wingp->departure_location == DEPART_AT_DOCK_BAY ) {
		required_string("$Departure Anchor:");
		stuff_string( name, F_NAME, NULL );
		wingp->departure_anchor = get_anchor(name);
	}

	if (optional_string("+Departure delay:")) {
		stuff_int(&delay);
		if ( delay < 0 )
			Error(LOCATION, "Cannot have departure delay < 0 on wing %s", wingp->name );
	} else
		delay = 0;


	if ( !Fred_running )
		wingp->departure_delay = -delay;		// use negative numbers to mean that delay timer not yet set
	else
		wingp->departure_delay = delay;

	required_string("$Departure Cue:");
	wingp->departure_cue = get_sexp_main();

	// stores a list of all names of ships in the wing
	required_string("$Ships:");
	wingp->wave_count = stuff_string_list( ship_names, MAX_SHIPS_PER_WING );
	wingp->current_count = 0;

	// get the wings goals, if any
	wing_goals = -1;
	if ( optional_string("$AI Goals:") )
		wing_goals = get_sexp_main();

	wingp->hotkey = -1;
	if (optional_string("+Hotkey:")) {
		stuff_int(&wingp->hotkey);
		Assert((wingp->hotkey >= 0) && (wingp->hotkey < 10));
	}

	if (optional_string("+Flags:")) {
		int count;

		count = stuff_string_list( wing_flag_strings, MAX_WING_FLAGS );
		for (i = 0; i < count; i++ ) {
			if ( !stricmp( wing_flag_strings[i], NOX("ignore-count")) )
				wingp->flags |= WF_IGNORE_COUNT;
			else if ( !stricmp( wing_flag_strings[i], NOX("reinforcement")) )
				wingp->flags |= WF_REINFORCEMENT;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-arrival-music")) )
				wingp->flags |= WF_NO_ARRIVAL_MUSIC;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-arrival-message")) )
				wingp->flags |= WF_NO_ARRIVAL_MESSAGE;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-arrival-warp")) )
				wingp->flags |= WF_NO_ARRIVAL_WARP;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-departure-warp")) )
				wingp->flags |= WF_NO_DEPARTURE_WARP;
			else if ( !stricmp( wing_flag_strings[i], NOX("no-dynamic")) )
				wingp->flags |= WF_NO_DYNAMIC;
			else
				Warning(LOCATION, "unknown wing flag\n%s\n\nSkipping.", wing_flag_strings[i]);
		}
	}

	// get the wave arrival delay bounds (if present).  Used as lower and upper bounds (in seconds)
	// which determine when new waves of a wing should arrive.
	wingp->wave_delay_min = 0;
	wingp->wave_delay_max = 0;
	if ( optional_string("+Wave Delay Min:") )
		stuff_int( &(wingp->wave_delay_min) );
	if ( optional_string("+Wave Delay Max:") )
		stuff_int( &(wingp->wave_delay_max) );

	// be sure to set the wave arrival timestamp of this wing to pop right away so that the
	// wing could be created if it needs to be
	wingp->wave_delay_timestamp = timestamp(0);

	// initialize wing goals
	for (i=0; i<MAX_AI_GOALS; i++) {
		wingp->ai_goals[i].ai_mode = AI_GOAL_NONE;
		wingp->ai_goals[i].priority = -1;
	}

#ifndef NO_NETWORK
	// 7/13/98 -- MWA
	// error checking against the player ship wings to be sure that wave count doesn't exceed one for
	// these wings.
	if ( Game_mode & GM_MULTIPLAYER ) {
		for (i = 0; i < MAX_STARTING_WINGS+1; i++ ) {
			if ( !stricmp(Starting_wing_names[i], wingp->name) ) {
				if ( wingp->num_waves > 1 ) {
					// only end the game if we're the server - clients will eventually find out :)
					if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
						multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_WAVE_COUNT);																
					}
					// Error(LOCATION, "Player wings Alpha, Beta, Gamma, or Zeta cannot have more than 1 wave.");
				}
			}
		}
	}

	// Get the next starting signature for this in this wing.  We want to reserve wave_count * num_waves
	// of signature.  These can be used to construct wings for ingame joiners.
	if ( Game_mode & GM_MULTIPLAYER ) {
		int next_signature;

		wingp->net_signature = multi_assign_network_signature( MULTI_SIG_SHIP );
		next_signature = wingp->net_signature + (wingp->wave_count * wingp->num_waves);
		if ( next_signature > SHIP_SIG_MAX )
			Error(LOCATION, "Too many total ships in mission (%d) for network signature assignment", SHIP_SIG_MAX);
		multi_set_network_signature( (ushort)next_signature, MULTI_SIG_SHIP );
	}
#endif

	for (i=0; i<MAX_SHIPS_PER_WING; i++)
		wingp->ship_index[i] = -1;

	// set up the ai_goals for this wing -- all ships created from this wing will inherit these goals
	// goals for the wing are stored slightly differently than for ships.  We simply store the index
	// into the sexpression array of each goal (max 10).  When a ship in this wing is created, each
	// goal in the wings goal array is given to the ship.
	if ( wing_goals != -1 ) {
		int sexp, index;

		// this will assign the goals to the wings as well as to any ships in the wing that have been
		// already created.
		index = 0;
		for ( sexp = CDR(wing_goals); sexp != -1; sexp = CDR(sexp) )
			ai_add_wing_goal_sexp(sexp, AIG_TYPE_EVENT_WING, wingnum);  // used by Fred

		//if (Fred_running)
			free_sexp2(wing_goals);  // free up sexp nodes for reused, since they aren't needed anymore.
	}

	// set the wing number for all ships in the wing
	for (i = 0; i < wingp->wave_count; i++ ) {
		//char *ship_name = wingp->ship_names[i];
		char *ship_name;
		int num, assigned = 0;
		p_object *objp;

		ship_name = ship_names[i];
		if (Fred_running) {
			num = wingp->ship_index[i] = ship_name_lookup(ship_name, 1);
			Assert ( num != -1 );

			// hack code -- REMOVE
			if ( Objects[Ships[num].objnum].flags & OF_PLAYER_SHIP )
				Ships[num].wingnum = wingnum;

		} else {
			// determine if this ship is a player ship, and deal with it appropriately.
			if ( !strnicmp(ship_name, NOX("Player "), 7) ) {
				Error(LOCATION, "Old mission file -- please convert by loading/saving in Fred -- see Allender/Hoffoss for help.");
			}

			// assign the wing number to the ship -- if the ship has arrived, doulble check that
			// there is only one wave of this wing since ships with their own arrival cue cannot be
			// in a wing with > 1 wave.  Otherwise, find the ship on the ship arrival list and set
			// their wing number
			if ( (num = ship_name_lookup(ship_name)) != -1 ) {
				Int3();					// this is impossible under the new system

			} else {
				objp = GET_FIRST(&ship_arrival_list);
				while( objp != END_OF_LIST(&ship_arrival_list) )	{
					if ( !strcmp(ship_name, objp->name) ) {
						Assert ( objp->wingnum == -1 );							// get Allender -- ship appears to be in multiple wings
						objp->wingnum = wingnum;
						assigned++;
					}
					objp = GET_NEXT(objp);
				}
			}

			if ( !assigned || (assigned > 1) )
				Error(LOCATION, "Cannot load mission -- wing %s -- ship %s not present in #Objects section (or specified multiple times in wing.\n", wingp->name, ship_name);
		}
	}

	// Fred doesn't create the wing.  otherwise, create the wing if is isn't a reinforcement.
	if ( !Fred_running && !(wingp->flags & WF_REINFORCEMENT) )
		parse_wing_create_ships( wingp, wingp->wave_count );
}

void parse_wings(mission *pm)
{
	required_string("#Wings");
	while (required_string_either("#Events", "$Name:")) {
		Assert(num_wings < MAX_WINGS);
		parse_wing(pm);
		num_wings++;
	}
}

// mission events are sexpressions which cause things to happen based on the outcome
// of other events in a mission.  Essentially scripting the different things that can happen
// in a mission

void parse_event(mission *pm)
{
	char buf[256];
	mission_event *event;

	event = &Mission_events[Num_mission_events];
	event->chain_delay = -1;

	required_string( "$Formula:" );
	event->formula = get_sexp_main();

	if (optional_string("+Name:")){
		stuff_string(event->name, F_NAME, NULL);
	} else {
		event->name[0] = 0;
	}

	if ( optional_string("+Repeat Count:")){
		stuff_int( &(event->repeat_count) );
	} else {
		event->repeat_count = 1;
	}

	event->interval = -1;
	if ( optional_string("+Interval:")){
		stuff_int( &(event->interval) );
	}

	event->score = 0;
	if ( optional_string("+Score:") ){
		stuff_int(&event->score);
	}

	if ( optional_string("+Chained:") ){
		stuff_int(&event->chain_delay);
	}

	if ( optional_string("+Objective:") ) {
		stuff_string(buf, F_NAME, NULL);
		event->objective_text = strdup(buf);
	} else {
		event->objective_text = NULL;
	}

	if ( optional_string("+Objective key:") ) {
		stuff_string(buf, F_NAME, NULL);
		event->objective_key_text = strdup(buf);
	} else {
		event->objective_key_text = NULL;
	}

	event->team = -1;
	if( optional_string("+Team:") ) {
		stuff_int(&event->team);
	}

	event->timestamp = timestamp(-1);

	// sanity check on the repeat count variable
	if ( event->repeat_count <= 0 ){
		Error (LOCATION, "Repeat count for mission event %s is <=0.\nMust be >= 1!", event->name );
	}
}

void parse_events(mission *pm)
{
	required_string("#Events");

	while (required_string_either( "#Goals", "$Formula:")) {
		Assert( Num_mission_events < MAX_MISSION_EVENTS );
		parse_event(pm);
		Num_mission_events++;
	}
}

void parse_goal(mission *pm)
{
	int dummy;

	mission_goal	*goalp;

	goalp = &Mission_goals[Num_goals++];

	Assert(Num_goals < MAX_GOALS);
	Assert(pm != NULL);

	find_and_stuff("$Type:", &goalp->type, F_NAME, Goal_type_names, Num_goal_type_names, "goal type");

	required_string("+Name:");
	stuff_string(goalp->name, F_NAME, NULL);

	// backwards compatibility for old Fred missions - all new missions should use $MessageNew
	if(optional_string("$Message:")){
		stuff_string(goalp->message, F_NAME, NULL, MAX_GOAL_TEXT);
	} else {
		required_string("$MessageNew:");
		stuff_string(goalp->message, F_MULTITEXT, NULL, MAX_GOAL_TEXT);
	}

	if (optional_string("$Rating:")){
		stuff_int(&dummy);  // not used
	}

	required_string("$Formula:");
	goalp->formula = get_sexp_main();

	goalp->flags = 0;
	if ( optional_string("+Invalid:") )
		goalp->type |= INVALID_GOAL;
	if ( optional_string("+Invalid") )
		goalp->type |= INVALID_GOAL;
	if ( optional_string("+No music") )
		goalp->flags |= MGF_NO_MUSIC;

	goalp->score = 0;
	if ( optional_string("+Score:") ){
		stuff_int(&goalp->score);
	}

	goalp->team = 0;
	if ( optional_string("+Team:") ){
		stuff_int( &goalp->team );
	}
}

void parse_goals(mission *pm)
{
	required_string("#Goals");

	while (required_string_either("#Waypoints", "$Type:")){
		parse_goal(pm);
	}
}

void parse_waypoint_list(mission *pm)
{
	waypoint_list	*wpl;


	Assert(Num_waypoint_lists < MAX_WAYPOINT_LISTS);
	Assert(pm != NULL);
	wpl = &Waypoint_lists[Num_waypoint_lists];

	required_string("$Name:");
	stuff_string(wpl->name, F_NAME, NULL);

	required_string("$List:");
	wpl->count = stuff_vector_list(wpl->waypoints, MAX_WAYPOINTS_PER_LIST);

	Num_waypoint_lists++;
}

void parse_waypoints(mission *pm)
{
	int z;
	vector pos;

	required_string("#Waypoints");

	Num_jump_nodes = 0;
	while (optional_string("$Jump Node:")) {
		Assert(Num_jump_nodes < MAX_JUMP_NODES);
		stuff_vector(&pos);
		z = jumpnode_create(&pos);
		Assert(z >= 0);

		if (optional_string("$Jump Node Name:")) {
			stuff_string(Jump_nodes[Num_jump_nodes - 1].name, F_NAME, NULL);
		}

		// If no name exists, then use a standard name
		if ( Jump_nodes[Num_jump_nodes - 1].name[0] == 0 ) {
			sprintf(Jump_nodes[Num_jump_nodes - 1].name, "Jump Node %d", Num_jump_nodes);
		}
	}

	while (required_string_either("#Messages", "$Name:"))
		parse_waypoint_list(pm);
}

void parse_messages(mission *pm)
{
	required_string("#Messages");

	mprintf(("Starting mission message count : %d\n", Num_message_waves));

	// the message_parse function can be found in MissionMessage.h.  The format in the
	// mission file takes the same format as the messages in messages,tbl.  Make parsing
	// a whole lot easier!!!
	while ( required_string_either("#Reinforcements", "$Name")){
		message_parse();		// call the message parsing system
	}

	mprintf(("Ending mission message count : %d\n", Num_message_waves));
}

void parse_reinforcement(mission *pm)
{
	reinforcements *ptr;
	int instance;

	Assert(Num_reinforcements < MAX_REINFORCEMENTS);
	Assert(pm != NULL);
	ptr = &Reinforcements[Num_reinforcements];

	required_string("$Name:");
	stuff_string(ptr->name, F_NAME, NULL);	

	find_and_stuff("$Type:", &ptr->type, F_NAME, Reinforcement_type_names, Num_reinforcement_type_names, "reinforcement type");

	required_string("$Num times:");
	stuff_int(&ptr->uses);
	ptr->num_uses = 0;

	// reset the flags to 0
	ptr->flags = 0;

	if ( optional_string("+Arrival delay:") ){
		stuff_int( &(ptr->arrival_delay) );
	}

	if ( optional_string("+No Messages:") ){
		stuff_string_list( ptr->no_messages, MAX_REINFORCEMENT_MESSAGES );
	}

	if ( optional_string("+Yes Messages:") ){
		stuff_string_list( ptr->yes_messages, MAX_REINFORCEMENT_MESSAGES );
	}	

	// sanity check on the names of reinforcements -- must either be wings/ships/arrival list.
	if ( ship_name_lookup(ptr->name) == -1 ) {
		if ( wing_name_lookup(ptr->name, 1) == -1 ) {
			p_object *p_objp;

			for ( p_objp = GET_FIRST(&ship_arrival_list); p_objp != END_OF_LIST(&ship_arrival_list); p_objp = GET_NEXT(p_objp) ) {
				if ( !stricmp(ptr->name, p_objp->name) ){
					break;
				}
			}

			if ( p_objp == END_OF_LIST(&ship_arrival_list) ) {
				Warning(LOCATION, "Reinforcement %s not found as ship or wing", ptr->name);
				return;
			}
		}
	}

	// now, if the reinforcement is a wing, then set the number of waves of the wing == number of
	// uses of the reinforcement
	instance = wing_name_lookup(ptr->name, 1);
	if ( instance != -1 )
		Wings[instance].num_waves = ptr->uses;

	Num_reinforcements++;
}

void parse_reinforcements(mission *pm)
{
	Num_reinforcements = 0;
	required_string("#Reinforcements");

	while (required_string_either("#Background bitmaps", "$Name:"))
		parse_reinforcement(pm);
}

void parse_bitmap(mission *pm)
{
	/*
	char name[NAME_LENGTH];
	int z;
	starfield_bitmaps *ptr;

	Assert(Num_starfield_bitmaps < MAX_STARFIELD_BITMAPS);
	Assert(pm != NULL);
	ptr = &Starfield_bitmaps[Num_starfield_bitmaps];

	required_string("$Bitmap:");
	stuff_string(name, F_NAME, NULL);
	for (z=0; z<Num_starfield_bitmap_lists; z++)	{
		if (!stricmp(name, Starfield_bitmap_list[z].name)){
			break;
		}
	}

	if ( z >= Num_starfield_bitmap_lists )	{
		Warning( LOCATION, "Bitmap specified in mission not in game!\n" );
		z = 0;
	}
	
	ptr->bitmap_index = z;
	required_string("$Orientation:");
	stuff_matrix(&ptr->m);

	required_string("$Rotation rate:");
	stuff_float(&ptr->rot);

	required_string("$Distance:");
	stuff_float(&ptr->dist);

	required_string("$Light:");
	stuff_int(&ptr->light);
	Num_starfield_bitmaps++;
	calculate_bitmap_points(ptr);
	*/
	Int3();
}

void parse_bitmaps(mission *pm)
{
	char str[MAX_FILENAME_LEN+1] = "";
	starfield_bitmap_instance b;
	int z;

	Num_starfield_bitmaps = 0;
	required_string("#Background bitmaps");

	required_string("$Num stars:");
	stuff_int(&Num_stars);
	if (Num_stars >= MAX_STARS)
		Num_stars = MAX_STARS;

	int Ambient_light_level;
	required_string("$Ambient light level:");
	stuff_int(&Ambient_light_level);

	// This should call light_set_ambient() to
	// set the ambient light

	Nebula_index = -1;
	Mission_palette = 1;

	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		// no regular nebula stuff
		nebula_close();

		// neb2 info
		strcpy(Neb2_texture_name, "Eraseme3");
		Neb2_poof_flags = ((1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5));
		if(optional_string("+Neb2:")){
			stuff_string(Neb2_texture_name, F_NAME, NULL);

			required_string("+Neb2Flags:");			
			stuff_int(&Neb2_poof_flags);

			// initialize neb effect. its gross to do this here, but Fred is dumb so I have no choice ... :(
			if(Fred_running){
				neb2_level_init();
			}
		}
	} else {
		if (optional_string("+Nebula:")) {
			stuff_string(str, F_NAME, NULL, MAX_FILENAME_LEN);
			
			// parse the proper nebula type (full or not)	
			for (z=0; z<NUM_NEBULAS; z++){
				if(The_mission.flags & MISSION_FLAG_FULLNEB){
					if (!stricmp(str, Neb2_filenames[z])) {
						Nebula_index = z;
						break;
					}
				} else {
					if (!stricmp(str, Nebula_filenames[z])) {
						Nebula_index = z;
						break;
					}
				}
			}

			if (optional_string("+Color:")) {
				stuff_string(str, F_NAME, NULL, MAX_FILENAME_LEN);
				for (z=0; z<NUM_NEBULA_COLORS; z++){
					if (!stricmp(str, Nebula_colors[z])) {
						Mission_palette = z;
						break;
					}
				}
			}

			if (optional_string("+Pitch:")){
				stuff_int(&Nebula_pitch);
			} else {
				Nebula_pitch = 0;
			}

			if (optional_string("+Bank:")){
				stuff_int(&Nebula_bank);
			} else {
				Nebula_bank = 0;
			}

			if (optional_string("+Heading:")){
				stuff_int(&Nebula_heading);
			} else {
				Nebula_heading = 0;
			}						
		}

		if (Nebula_index >= 0){		
			nebula_init(Nebula_filenames[Nebula_index], Nebula_pitch, Nebula_bank, Nebula_heading);
		} else {
			nebula_close();		
		}
	}	
	
	// parse suns
	Num_suns = 0;
	while(optional_string("$Sun:")){
		// filename
		stuff_string(b.filename, F_NAME, NULL);
			
		// angles
		required_string("+Angles:");
		stuff_float(&b.ang.p);
		stuff_float(&b.ang.b);
		stuff_float(&b.ang.h);		

		// scale
		required_string("+Scale:");
		stuff_float(&b.scale_x);		
		b.scale_y = b.scale_x;
		b.div_x = 1;
		b.div_y = 1;
		
		// if we have room, store it
		if(Num_suns < MAX_STARFIELD_BITMAPS){
			Suns[Num_suns] = b;
			strcpy(Suns[Num_suns].filename, b.filename);
			Num_suns++;
		}
	}

	// parse background bitmaps
	Num_starfield_bitmaps = 0;
	while(optional_string("$Starbitmap:")){
		// filename
		stuff_string(b.filename, F_NAME, NULL);
			
		// angles
		required_string("+Angles:");
		stuff_float(&b.ang.p);
		stuff_float(&b.ang.b);
		stuff_float(&b.ang.h);		

		// scale
		// scale
		if(optional_string("+Scale:")){
			stuff_float(&b.scale_x);
			b.scale_y = b.scale_x;
			b.div_x = 1;
			b.div_y = 1;
		} else {
			required_string("+ScaleX:");
			stuff_float(&b.scale_x);

			required_string("+ScaleY:");
			stuff_float(&b.scale_y);

			required_string("+DivX:");
			stuff_int(&b.div_x);

			required_string("+DivY:");
			stuff_int(&b.div_y);
		}		
		
		// if we have room, store it
		if(Num_starfield_bitmaps < MAX_STARFIELD_BITMAPS){
			Starfield_bitmap_instance[Num_starfield_bitmaps] = b;
			strcpy(Starfield_bitmap_instance[Num_starfield_bitmaps].filename, b.filename);
			Num_starfield_bitmaps++;
		}
	}

	if ( optional_string("#Asteroid Fields") ){
		parse_asteroid_fields(pm);
	}
}

void parse_asteroid_fields(mission *pm)
{
#ifndef FS2_DEMO

	int i, count, subtype;

	Assert(pm != NULL);
	for (i=0; i<MAX_ASTEROID_FIELDS; i++)
		Asteroid_field.num_initial_asteroids = 0;

	i = 0;
	count = 0;
//	required_string("#Asteroid Fields");
	while (required_string_either("#", "$density:")) {
		float speed, density;


		Assert(i < 1);
		required_string("$Density:");
		stuff_float(&density);

		Asteroid_field.num_initial_asteroids = (int) density;

		Asteroid_field.field_type = FT_ACTIVE;
		if (optional_string("+Field Type:")) {
			stuff_int((int*)&Asteroid_field.field_type);
		}

		Asteroid_field.debris_genre = DG_ASTEROID;
		if (optional_string("+Debris Genre:")) {
			stuff_int((int*)&Asteroid_field.debris_genre);
		}

		Asteroid_field.field_debris_type[0] = -1;
		Asteroid_field.field_debris_type[1] = -1;
		Asteroid_field.field_debris_type[2] = -1;
		if (Asteroid_field.debris_genre == DG_SHIP) {
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&Asteroid_field.field_debris_type[0]);
			}
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&Asteroid_field.field_debris_type[1]);
			}
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&Asteroid_field.field_debris_type[2]);
			}
		} else {
			// debris asteroid
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&subtype);
				Asteroid_field.field_debris_type[subtype] = 1;
				count++;
			}
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&subtype);
				Asteroid_field.field_debris_type[subtype] = 1;
				count++;
			}
			if (optional_string("+Field Debris Type:")) {
				stuff_int(&subtype);
				Asteroid_field.field_debris_type[subtype] = 1;
				count++;
			}
		}

		// backward compatibility
		if ( (Asteroid_field.debris_genre == DG_ASTEROID) && (count == 0) ) {
			Asteroid_field.field_debris_type[0] = 0;
		}

		required_string("$Average Speed:");
		stuff_float(&speed);

		vm_vec_rand_vec_quick(&Asteroid_field.vel);
		vm_vec_scale(&Asteroid_field.vel, speed);

		Asteroid_field.speed = speed;

		required_string("$Minimum:");
		stuff_vector(&Asteroid_field.min_bound);

		required_string("$Maximum:");
		stuff_vector(&Asteroid_field.max_bound);

		if (optional_string("+Inner Bound:")) {
			Asteroid_field.has_inner_bound = 1;

			required_string("$Minimum:");
			stuff_vector(&Asteroid_field.inner_min_bound);

			required_string("$Maximum:");
			stuff_vector(&Asteroid_field.inner_max_bound);
		} else {
			Asteroid_field.has_inner_bound = 0;
		}
		i++;
	}
#endif // DEMO
}

void parse_variables()
{
	if (! optional_string("#Sexp_variables") ) {
		return;
	} else {
		int num_variables;
		num_variables = stuff_sexp_variable_list();
	}
}


void parse_mission(mission *pm, int flag)
{
	int i;

	Player_starts = Num_cargo = Num_waypoint_lists = Num_goals = num_wings = num_ship_arrivals = 0;
	Player_start_shipnum = -1;
	*Player_start_shipname = 0;		// make the string 0 length for checking later
	memset( &Player_start_pobject, 0, sizeof(Player_start_pobject) );

	// initialize the initially_docked array.
	for ( i = 0; i < MAX_SHIPS; i++ ) {
		Initially_docked[i].docker = NULL;
		Initially_docked[i].dockee[0] = '\0';
		Initially_docked[i].docker_point[0] = '\0';
		Initially_docked[i].dockee_point[0] = '\0';
	}
	Total_initially_docked = 0;

	list_init( &ship_arrival_list );		// init lists for arrival objects and wings

	init_parse();
	Subsys_index = 0;

	parse_mission_info(pm);	
	Current_file_checksum = netmisc_calc_checksum(pm,MISSION_CHECKSUM_SIZE);
	if ( flag == MISSION_PARSE_MISSION_INFO )
		return;
	parse_plot_info(pm);
	parse_variables();
	parse_briefing_info(pm);	// TODO: obsolete code, keeping so we don't obsolete existing mission files
	parse_cmd_briefs(pm);
	parse_briefing(pm);
	parse_debriefing_new(pm);
	parse_player_info(pm);
	parse_objects(pm, flag);
	parse_wings(pm);
	parse_events(pm);
	parse_goals(pm);
	parse_waypoints(pm);
	parse_messages(pm);
	parse_reinforcements(pm);
	parse_bitmaps(pm);
	parse_music(pm);

	post_process_mission();
}

void post_process_mission()
{
	int			i;
	int			indices[MAX_SHIPS], objnum;
	p_object		*p_objp;
	ship_weapon	*swp;
	ship_obj *so;

	// the player_start_shipname had better exist at this point!
	Player_start_shipnum = ship_name_lookup( Player_start_shipname );
	Assert ( Player_start_shipnum != -1 );
	Assert ( Player_start_pobject.flags & P_SF_PLAYER_START_VALID );

	// Assign objnum, shipnum, etc. to the player structure
	objnum = Ships[Player_start_shipnum].objnum;
	Player_obj = &Objects[objnum];
	if (!Fred_running){
		Player->objnum = objnum;
	}

	Player_obj->flags |= OF_PLAYER_SHIP;			// make this object a player controlled ship.
	Player_ship = &Ships[Player_start_shipnum];
	Player_ai = &Ai_info[Player_ship->ai_index];

	Player_ai->targeted_subsys = NULL;
	Player_ai->targeted_subsys_parent = -1;

	// determine if player start has initial velocity and set forward cruise percent to relect this
	if ( Player_obj->phys_info.vel.xyz.z > 0.0f )
		Player->ci.forward_cruise_percent = Player_obj->phys_info.vel.xyz.z / Player_ship->current_max_speed * 100.0f;

#ifndef NO_NETWORK
	// put in hard coded starting wing names.
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)){
		Starting_wings[0] = wing_name_lookup(Starting_wing_names[0],1);
		Starting_wings[1] = wing_name_lookup(Starting_wing_names[MAX_STARTING_WINGS],1);
	}
	else
#endif
	{
		for (i = 0; i < MAX_STARTING_WINGS; i++ ) {
			Starting_wings[i] = wing_name_lookup(Starting_wing_names[i], 1);
		}
	}

	init_ai_system();

	// call a function to deal with intially docked ships
	mission_parse_do_initial_docks();

	// deal with setting up arrival location for all ships.  Must do this now after all ships are created
	mission_parse_set_arrival_locations();

	// clear out information about arriving support ships
	Arriving_support_ship = NULL;
	Num_arriving_repair_targets = 0;

	// convert all ship name indices to ship indices now that mission has been loaded
	if (Fred_running) {
		for (i=0; i<Num_parse_names; i++) {
			indices[i] = ship_name_lookup(Parse_names[i], 1);
			if (indices[i] < 0)
				Warning(LOCATION, "Ship name \"%s\" referenced, but this ship doesn't exist", Parse_names[i]);
		}

		for (i=0; i<MAX_SHIPS; i++) {
			if ((Ships[i].objnum >= 0) && (Ships[i].arrival_anchor >= 0) && (Ships[i].arrival_anchor < SPECIAL_ARRIVAL_ANCHORS_OFFSET))
				Ships[i].arrival_anchor = indices[Ships[i].arrival_anchor];

			if ( (Ships[i].objnum >= 0) && (Ships[i].departure_anchor >= 0) )
				Ships[i].departure_anchor = indices[Ships[i].departure_anchor];
		}

		for (i=0; i<MAX_WINGS; i++) {
			if (Wings[i].wave_count  && (Wings[i].arrival_anchor >= 0) && (Wings[i].arrival_anchor < SPECIAL_ARRIVAL_ANCHORS_OFFSET))
				Wings[i].arrival_anchor = indices[Wings[i].arrival_anchor];

			if (Wings[i].wave_count  && (Wings[i].departure_anchor >= 0) )
				Wings[i].departure_anchor = indices[Wings[i].departure_anchor];
		}

	}

	// before doing anything else, we must validate all of the sexpressions that were loaded into the mission.
	// Loop through the Sexp_nodes array and send the top level functions to the check_sexp_syntax parser

	for (i = 0; i < MAX_SEXP_NODES; i++) {
		if ( is_sexp_top_level(i) && (!Fred_running || (i != Sexp_clipboard))) {
			int result, bindex, op;

			op = identify_operator(CTEXT(i));
			Assert(op != -1);  // need to make sure it is an operator before we treat it like one..
			result = check_sexp_syntax( i, query_operator_return_type(op), 1, &bindex);

			// entering this if statement will result in program termination!!!!!
			// print out an error based on the return value from check_sexp_syntax()
			if ( result ) {
				char sexp_str[8192], text[8192];

				convert_sexp_to_string( i, sexp_str, SEXP_ERROR_CHECK_MODE);
				sprintf(text, "%s.\n\nIn sexpression: %s\n(Error appears to be: %s)",
					sexp_error_message(result), sexp_str, Sexp_nodes[bindex].text);

				if (!Fred_running)
					Error( LOCATION, text );
				else
					Warning( LOCATION, text );
			}
		}
	}

	ai_post_process_mission();


	/*
	for (i=0; i<Total_initially_docked; i++) {
		z = ship_name_lookup(Initially_docked[i].dockee);
		if (z >= 0) {
			Assert(Initially_docked[i].docker->type == OBJ_SHIP);
			p1 = model_find_dock_name_index(Ships[Initially_docked[i].docker->instance].modelnum,
				Initially_docked[i].docker_point);
			Assert(Objects[z].type == OBJ_SHIP);
			p2 = model_find_dock_name_index(Ships[Objects[z].instance].modelnum,
				Initially_docked[i].dockee_point);

			if ((p1 >= 0) && (p2 >= 0)) {
				nprintf(("AI", "Initially Docked: %s with %s\n", Ships[Initially_docked[i].docker->instance].ship_name, Ships[Objects[z].instance].ship_name));
				if (ship_docking_valid(Initially_docked[i].docker->instance, Objects[z].instance))  // only dock if they are allowed to be docked.
					ai_dock_with_object(Initially_docked[i].docker, &Objects[z], 89, AIDO_DOCK_NOW, p1, p2);
					
			} else
				Int3();		//	Curious.  Two ships told to dock, but one of the dock points is bogus.  
								// Get Allender or Hoffoss, one of whom probably wrote the above if ()
		}
	}
	*/

	// we must also count all of the ships of particular types.  We count all of the ships that do not have
	// their SF_IGNORE_COUNT flag set.  We don't count ships in wings when the equivalent wing flag is set.
	// in counting ships in wings, we increment the count by the wing's wave count to account for everyone.
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {
		int siflags, num, shipnum;

		shipnum = Objects[so->objnum].instance;
		// pass over non-ship objects and player ship objects
		if ( Ships[shipnum].objnum == -1 || (Objects[Ships[shipnum].objnum].flags & OF_PLAYER_SHIP) )
			continue;
		if ( Ships[shipnum].flags & SF_IGNORE_COUNT )
			continue;
		if ( (Ships[shipnum].wingnum != -1) && (Wings[Ships[shipnum].wingnum].flags & WF_IGNORE_COUNT) )
			continue;

		siflags = Ship_info[Ships[shipnum].ship_info_index].flags;
		
		// determine the number of times we need to add this ship into the count
//		if ( Ships[i].wingnum == -1 )
			num = 1;
//		else
//			num = Wings[Ships[i].wingnum].num_waves;

		ship_add_ship_type_count( siflags, num );
	}
	// now go through the list of ships yet to arrive
	for ( p_objp = GET_FIRST(&ship_arrival_list); p_objp != END_OF_LIST(&ship_arrival_list); p_objp = GET_NEXT(p_objp) ) {
		int siflags, num;

		// go through similar motions as above
		if ( p_objp->flags & P_SF_IGNORE_COUNT )
			continue;
		if ( (p_objp->wingnum != -1) && (Wings[p_objp->wingnum].flags & WF_IGNORE_COUNT) )
			continue;

		siflags = Ship_info[p_objp->ship_class].flags;

		if ( p_objp->wingnum == -1 )
			num = 1;
		else
			num = Wings[p_objp->wingnum].num_waves - 1;			// subtract one since we already counted the first wave
		
		ship_add_ship_type_count( siflags, num );
	}

	// set player weapons that are selected by default
	// AL 09/17/97: I added this code to select the first primary/secondary weapons, 
	// since I noticed the player ship sometimes doesn't get default weapons selected		

	// DB: modified 4/23/98 to take multiplayer into account. Under certain circumstances, multiplayer netplayer ships
	//     had their current_primary_bank and current_secondary_bank set to -1 (from ship_set()) and left there since
	//     Player_ship is not the only one we need to need about.
	for ( so = GET_FIRST(&Ship_obj_list); so != END_OF_LIST(&Ship_obj_list); so = GET_NEXT(so) ) {		
		ship *shipp = &Ships[Objects[so->objnum].instance];

		// don't process non player wing ships
		if ( !(shipp->flags & SF_FROM_PLAYER_WING ) )
			continue;			

		swp = &shipp->weapons;
	
		// swp = &Player_ship->weapons;
		if ( swp->num_primary_banks > 0 ) {
			swp->current_primary_bank = 0;			// currently selected primary bank
		}

		if ( swp->num_secondary_banks > 0 ) {
			swp->current_secondary_bank = 0;			// currently selected secondary bank
		}
	}

	ets_init_ship(Player_obj);	// init ETS data for the player

	// put the timestamp stuff here for now
	Mission_arrival_timestamp = timestamp( ARRIVAL_TIMESTAMP );
	Mission_departure_timestamp = timestamp( DEPARTURE_TIMESTAMP );
	Mission_end_time = -1;

#ifndef NO_NETWORK
	if(Game_mode & GM_MULTIPLAYER){ 
		multi_respawn_build_points();
	}	
#endif

	// maybe reset hotkey defaults when loading new mission
	if ( Last_file_checksum != Current_file_checksum ){
		mission_hotkey_reset_saved();
	}

	Allow_arrival_music_timestamp=timestamp(0);
	Allow_arrival_message_timestamp=timestamp(0);
	Arrival_message_delay_timestamp = timestamp(-1);

	int idx;
	for(idx=0; idx<2; idx++){
		Allow_arrival_music_timestamp_m[idx]=timestamp(0);
		Allow_arrival_message_timestamp_m[idx]=timestamp(0);
		Arrival_message_delay_timestamp_m[idx] = timestamp(-1);
	}	

	Last_file_checksum = Current_file_checksum;
}

int get_mission_info(char *filename, mission *mission_p)
{
	int rval;

	// if mission_p is NULL, make it point to The_mission
	if ( mission_p == NULL )
		mission_p = &The_mission;

	if ((rval = setjmp(parse_abort)) != 0) {
		nprintf(("Error", "Error abort!  Code = %d", rval));
		return rval;
	
	} else {
		int filelength;

		// open localization
		lcl_ext_open();

		CFILE *ftemp = cfopen(filename, "rt");
		if (!ftemp){
			// close localization
			lcl_ext_close();

			return -1;
		}

		// 7/9/98 -- MWA -- check for 0 length file.
		filelength = cfilelength(ftemp);
		cfclose(ftemp);
		if ( filelength == 0 ){
			// close localization
			lcl_ext_close();	

			return -1;
		}

		read_file_text(filename, CF_TYPE_MISSIONS);
		memset( mission_p, 0, sizeof(mission) );
		init_parse();
		parse_mission_info(mission_p);

		// close localization
		lcl_ext_close();
	}

	return 0;
}

// mai parse routine for parsing a mission.  The default parameter flags tells us which information
// to get when parsing the mission.  0 means get everything (default).  Other flags just gets us basic
// info such as game type, number of players etc.
int parse_main(char *mission_name, int flags)
{
	int rval, i;

	// fill in Ship_class_names array with the names from the ship_info struct;
	Num_parse_names = 0;
	Mission_all_attack = 0;	//	Might get set in mission load.
	Assert(Num_ship_types < MAX_SHIP_TYPES);

	for (i = 0; i < Num_ship_types; i++)
		Ship_class_names[i] = Ship_info[i].name;

	if ((rval = setjmp(parse_abort)) != 0) {
		nprintf(("Error", "Error abort!  Code = %i.", rval));
		return rval;
	
	} else {
		// open localization
		lcl_ext_open();

		CFILE *ftemp = cfopen(mission_name, "rt", CFILE_NORMAL, CF_TYPE_MISSIONS);
		// fail situation.
		if (!ftemp) {
			if (!Fred_running)
				Error( LOCATION, "Couldn't open mission '%s'\n", mission_name );

			Current_file_length = -1;
			Current_file_checksum = 0;

			// close localization
			lcl_ext_close();

			return -1;
		}

		Current_file_length = cfilelength(ftemp);
		cfclose(ftemp);

		read_file_text(mission_name, CF_TYPE_MISSIONS);
		memset(&The_mission, 0, sizeof(The_mission));
		parse_mission(&The_mission, flags);
		display_parse_diagnostics();

		// close localization
		lcl_ext_close();
	}

	if (!Fred_running)
		strcpy(Mission_filename, mission_name);

	return 0;
}

// sets the arrival lcoation of the ships in wingp.  pass num_to_set since the threshold value
// for wings may have us create more ships in the wing when there are still some remaining
void mission_set_wing_arrival_location( wing *wingp, int num_to_set )
{
	int index;

	// get the starting index into the ship_index array of the first ship whose location we need set.

	index = wingp->current_count - num_to_set;
	if ( (wingp->arrival_location == ARRIVE_FROM_DOCK_BAY) || (wingp->arrival_location == ARRIVE_AT_LOCATION) ) {
		while ( index < wingp->current_count ) {
			object *objp;

			objp = &Objects[Ships[wingp->ship_index[index]].objnum];
			mission_set_arrival_location(wingp->arrival_anchor, wingp->arrival_location, wingp->arrival_distance, OBJ_INDEX(objp), NULL, NULL);

			index++;
		}
	} else {
		object *leader_objp;
		vector pos;
		matrix orient;
		int wing_index;

		// wing is not arriving from a docking bay -- possibly move them based on arriving near
		// or in front of some other ship.
		index = wingp->current_count - num_to_set;
		leader_objp = &Objects[Ships[wingp->ship_index[index]].objnum];
		if (mission_set_arrival_location(wingp->arrival_anchor, wingp->arrival_location, wingp->arrival_distance, OBJ_INDEX(leader_objp), &pos, &orient)) {
			// modify the remaining ships created
			index++;
			wing_index = 1;
			while ( index < wingp->current_count ) {
				object *objp;

				objp = &Objects[Ships[wingp->ship_index[index]].objnum];

				// change the position of the next ships in the wing.  Use the cool function in AiCode.cpp which
				// Mike K wrote to give new positions to the wing members.
				get_absolute_wing_pos( &objp->pos, leader_objp, wing_index++, 0);
				memcpy( &objp->orient, &orient, sizeof(matrix) );

				index++;
			}
		}
	}

	// create warp effect if in mission and not arriving from docking bay
	if ( (Game_mode & GM_IN_MISSION) && (wingp->arrival_location != ARRIVE_FROM_DOCK_BAY) ) {
		for ( index = wingp->current_count - num_to_set; index < wingp->current_count; index ++ ) {
			shipfx_warpin_start( &Objects[Ships[wingp->ship_index[index]].objnum] );
		}
	}
}

// this function is called after a mission is parsed to set the arrival locations of all ships in the
// mission to the apprioriate spot.  Mainly needed because ships might be in dock bays to start
// the mission, so their AI mode must be set appropriately.
void mission_parse_set_arrival_locations()
{
	int i;
	object *objp;

	if ( Fred_running )
		return;

	obj_merge_created_list();
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		ship *shipp;

		if ( objp->type != OBJ_SHIP ) 
			continue;

		shipp = &Ships[objp->instance];
		// if the ship is in a wing -- ignore the info and let the wing info handle it
		if ( shipp->wingnum != -1 )
			continue;

		// call function to set arrival location for this ship.
		mission_set_arrival_location( shipp->arrival_anchor, shipp->arrival_location, shipp->arrival_distance, OBJ_INDEX(objp), NULL, NULL);
	}

	// do the wings
	for ( i = 0; i < num_wings; i++ ) {

		// if wing has no ships, then don't process it.
		if ( Wings[i].current_count == 0 )
			continue;

		mission_set_wing_arrival_location( &Wings[i], Wings[i].current_count );
	}
}


// function which iterates through the ship_arrival_list and creates any ship which
// should be intially docked with a ship which currently exists in the mission
void mission_parse_do_initial_docks()
{
	p_object *pobjp, *tmp;

	pobjp = GET_FIRST( &ship_arrival_list );
	while ( pobjp != END_OF_LIST(&ship_arrival_list) ) {
		int shipnum;

		tmp = GET_NEXT(pobjp);

		// see if the flag for initial docked is set
		if ( pobjp->flags & P_SF_INITIALLY_DOCKED ) {
			// see if who this parse object is supposed to be docked with is in the mission
			shipnum = ship_name_lookup( pobjp->docked_with );
			if ( shipnum != -1 ) {
				int objnum, p1, p2;

				// the ship exists, so create this object, then dock the two.
				objnum = parse_create_object( pobjp );
				Assert ( objnum != -1 );

				list_remove( &ship_arrival_list, pobjp);

				// p1 is the parse object's docking point.
				// p2 is the existing objects docking point.
				p1 = model_find_dock_name_index(Ships[shipnum].modelnum, pobjp->docker_point);
				p2 = model_find_dock_name_index(Ships[Objects[objnum].instance].modelnum, pobjp->dockee_point);

				if ((p1 >= 0) && (p2 >= 0)) {
					nprintf(("AI", "Initially Docked: %s with %s\n", Ships[shipnum].ship_name, Ships[Objects[objnum].instance].ship_name));
					if (ship_docking_valid(shipnum, Objects[objnum].instance))  // only dock if they are allowed to be docked.
						ai_dock_with_object(&Objects[Ships[shipnum].objnum], &Objects[objnum], 89, AIDO_DOCK_NOW, p1, p2);
					else
						ai_dock_with_object(&Objects[objnum], &Objects[Ships[shipnum].objnum], 89, AIDO_DOCK_NOW, p2, p1);
						
				} else
					Int3();		//	Curious.  Two ships told to dock, but one of the dock points is bogus.  
									// Get Allender or Hoffoss, one of whom probably wrote the above if ()
			}
		}

		pobjp = tmp;
	}
}

// function which returns true or false if the given mission support multiplayers
int mission_parse_is_multi(char *filename, char *mission_name)
{
	int rval, game_type;
	int filelength;
	CFILE *ftemp;

	// new way of getting information.  Open the file, and just get the name and the game_type flags.
	// return the flags if a multiplayer mission

	game_type = 0;

	ftemp = cfopen(filename, "rt");
	if (!ftemp)
		return 0;

	// 7/9/98 -- MWA -- check for 0 length file.
	filelength = cfilelength(ftemp);
	cfclose(ftemp);
	if ( filelength == 0 )
		return 0;

	// open localization
	lcl_ext_open();

	if ((rval = setjmp(parse_abort)) != 0) {
		Error(LOCATION, "Bogus!  Trying to get multi game type on mission %s returned as a mission from cf_get_filelist\n");
	} else	{
		read_file_text(filename, CF_TYPE_MISSIONS);
		reset_parse();
		if ( skip_to_string("$Name:") != 1 ) {
			nprintf(("Network", "Unable to process %s because we couldn't find $Name:", filename));

			// close localization
			lcl_ext_close();

			return 0;
		}
		stuff_string( mission_name, F_NAME, NULL );
		if ( skip_to_string("+Game Type Flags:") != 1 ) {
			nprintf(("Network", "Unable to process %s because we couldn't find +Game Type Flags:\n", filename));

			// close localization
			lcl_ext_close();

			return 0;
		}
		stuff_int(&game_type);
	}
	if ( game_type & MISSION_TYPE_MULTI ){
		// close localization
		lcl_ext_close();

		return game_type;
	}

	// close localization
	lcl_ext_close();
	
	return 0;
}

// function which gets called to retrieve useful information about a mission.  We will get the
// name, description, and number of players for a mission.  Probably used for multiplayer only?
// The calling function can use the information in The_mission to get the name/description of the mission
// if needed.

int mission_parse_get_multi_mission_info( char *filename )
{
	if ( parse_main(filename, MISSION_PARSE_MISSION_INFO) ){
		return -1;
	}

	Assert( The_mission.game_type & MISSION_TYPE_MULTI );		// assume multiplayer only for now?

	// return the number of parse_players.  later, we might want to include (optionally?) the number
	// of other ships in the main players wing (usually wing 'alpha') for inclusion of number of
	// players allowed.

	return The_mission.num_players;
}

// returns true or false if this is on the yet to arrive list
int mission_parse_ship_arrived( char *shipname )
{
	p_object *objp;

	for ( objp = GET_FIRST(&ship_arrival_list); objp !=END_OF_LIST(&ship_arrival_list); objp = GET_NEXT(objp) )	{
		if ( !stricmp( objp->name, shipname) )
			return 0;			// still on the arrival list
	}
	return 1;
}

// return the parse object on the ship arrival list associated with the given name
p_object *mission_parse_get_arrival_ship( char *name )
{
	p_object *objp;

	for ( objp = GET_FIRST(&ship_arrival_list); objp !=END_OF_LIST(&ship_arrival_list); objp = GET_NEXT(objp) )	{
		if ( !stricmp( objp->name, name) )
			return objp;			// still on the arrival list
	}

	return NULL;
}

// return the parse object on the ship arrival list associated with the given signature
p_object *mission_parse_get_arrival_ship( ushort net_signature )
{
	p_object *objp;

	for ( objp = GET_FIRST(&ship_arrival_list); objp !=END_OF_LIST(&ship_arrival_list); objp = GET_NEXT(objp) )	{
		if ( objp->net_signature == net_signature )
			return objp;			// still on the arrival list
	}

	return NULL;
}

// mission_set_arrival_location() sets the arrival location of a parse object according to the arrival location
// of the object.  Returns true if object set to new position, false if not.
int mission_set_arrival_location(int anchor, int location, int dist, int objnum, vector *new_pos, matrix *new_orient)
{
	int shipnum, anchor_objnum;
	vector anchor_pos, rand_vec, new_fvec;
	matrix orient;

	if ( location == ARRIVE_AT_LOCATION )
		return 0;

	Assert(anchor >= 0);

	// this ship might possibly arrive at another location.  The location is based on the
	// proximity of some ship (and some other special tokens)

	// if we didn't find the arrival anchor in the list of special nodes, then do a
	// ship name lookup on the anchor
	if (anchor < SPECIAL_ARRIVAL_ANCHORS_OFFSET) {
		shipnum = ship_name_lookup(Parse_names[anchor]);
		if ( shipnum == -1 ) {
			Assert ( location != ARRIVE_FROM_DOCK_BAY );		// bogus data somewhere!!!  get mwa
			nprintf (("allender", "couldn't find ship for arrival anchor -- using location ship created at"));
			return 0;
		}

	} else {
		// come up with a position based on the special token names
		shipnum = -1;

		if (anchor == ANY_FRIENDLY) {
			shipnum = ship_get_random_team_ship( TEAM_FRIENDLY, SHIP_GET_ANY_SHIP );
		} else if (anchor == ANY_HOSTILE) {
			shipnum = ship_get_random_team_ship( opposing_team_mask(Player_ship->team), SHIP_GET_ANY_SHIP );
		} else if (anchor == ANY_FRIENDLY_PLAYER) {
			shipnum = ship_get_random_team_ship( TEAM_FRIENDLY, SHIP_GET_ONLY_PLAYERS );
		} else if (anchor == ANY_HOSTILE_PLAYER) {
			shipnum = ship_get_random_team_ship( opposing_team_mask(Player_ship->team), SHIP_GET_ONLY_PLAYERS );
		} else
			Int3();		// get allender -- unknown special arrival instructions

		// if we didn't get an object from one of the above functions, then make the object
		// arrive at it's placed location
		if ( shipnum == -1 ) {
			nprintf (("Allender", "Couldn't find random ship for arrival anchor -- using default location\n"));
			return 0;
		}
	}

	// take the shipnum and get the position.  once we have positions, we can determine where
	// to make this ship appear
	Assert ( shipnum != -1 );
	anchor_objnum = Ships[shipnum].objnum;
	anchor_pos = Objects[anchor_objnum].pos;

	// if arriving from docking bay, then set ai mode and call function as per AL's instructions.
	if ( location == ARRIVE_FROM_DOCK_BAY ) {
		vector pos, fvec;

		// if we get an error, just let the ship arrive(?)
		if ( ai_acquire_emerge_path(&Objects[objnum], anchor_objnum, &pos, &fvec) == -1 ) {
			Int3();			// get MWA or AL -- not sure what to do here when we cannot acquire a path
			return 0;
		}
		Objects[objnum].pos = pos;
		Objects[objnum].orient.vec.fvec = fvec;
	} else {

		// AL: ensure dist > 0 (otherwise get errors in vecmat)
		// TODO: maybe set distance to 2x ship radius of ship appearing in front of?
		if ( dist <= 0 ) {
			Error(LOCATION, "Distance of %d is invalid in mission_set_arrival_location\n", dist);
			return 0;
		}
		
		// get a vector which is the ships arrival position based on the type of arrival
		// this ship should have.  Arriving near a ship we use a random normalized vector
		// scaled by the distance given by the designer.  Arriving in front of a ship means
		// entering the battle in the view cone.
		if ( location == ARRIVE_NEAR_SHIP ) {
			// get a random vector -- use static randvec if in multiplayer
			if ( Game_mode & GM_NORMAL )
				vm_vec_rand_vec_quick(&rand_vec);
			else
				static_randvec( Objects[objnum].net_signature, &rand_vec );
		} else if ( location == ARRIVE_IN_FRONT_OF_SHIP ) {
			vector t1, t2, t3;
			int r1, r2;
			float x;

			// cool function by MK to give a reasonable random vector "in front" of a ship
			// rvec and uvec are the right and up vectors.
			// If these are not available, this would be an expensive method.
			//x = cos(angle)
			x = (float)cos(ANG_TO_RAD(45));
			if ( Game_mode & GM_NORMAL ) {
				r1 = rand() < RAND_MAX/2 ? -1 : 1;
				r2 = rand() < RAND_MAX/2 ? -1 : 1;
			} else {
				// in multiplayer, use the static rand functions so that all clients can get the
				// same information.
				r1 = static_rand(Objects[objnum].net_signature) < RAND_MAX/2 ? -1 : 1;
				r2 = static_rand(Objects[objnum].net_signature+1) < RAND_MAX/2 ? -1 : 1;
			}

			vm_vec_copy_scale(&t1, &(Objects[anchor_objnum].orient.vec.fvec), x);
			vm_vec_copy_scale(&t2, &(Objects[anchor_objnum].orient.vec.rvec), (1.0f - x) * r1);
			vm_vec_copy_scale(&t3, &(Objects[anchor_objnum].orient.vec.uvec), (1.0f - x) * r2);

			vm_vec_add(&rand_vec, &t1, &t2);
			vm_vec_add2(&rand_vec, &t3);
			vm_vec_normalize(&rand_vec);
		}

		// add in the radius of the two ships involved.  This will make the ship arrive further than
		// specified, but will appear more accurate since we are pushing the edge of the model to the
		// specified distance.  large objects appears to be a lot closer without the following line because
		// the object centers were at the correct distance, but the model itself was much closer to the
		// target ship.
		dist += (int)Objects[objnum].radius + (int)Objects[anchor_objnum].radius;
		vm_vec_scale_add(&Objects[objnum].pos, &anchor_pos, &rand_vec, (float)dist);

		// I think that we will always want to orient the ship that is arriving to face towards
		// the ship it is arriving near/in front of.  The effect will be cool!
		//
		// calculate the new fvec of the ship arriving and use only that to get the matrix.  isn't a big
		// deal not getting bank.
		vm_vec_sub(&new_fvec, &anchor_pos, &Objects[objnum].pos );
		vm_vector_2_matrix( &orient, &new_fvec, NULL, NULL );
		Objects[objnum].orient = orient;
	}

	// set the new_pos parameter since it might be used outside the function (i.e. when dealing with wings).
	if ( new_pos )
		memcpy(new_pos, &Objects[objnum].pos, sizeof(vector) );

	if ( new_orient )
		memcpy( new_orient, &Objects[objnum].orient, sizeof(matrix) );

	return 1;
}

// mark a reinforcement as available
void mission_parse_mark_reinforcement_available(char *name)
{
	int i;
	reinforcements *rp;

	for (i = 0; i < Num_reinforcements; i++) {
		rp = &Reinforcements[i];
		if ( !stricmp(rp->name, name) ) {
			if ( !(rp->flags & RF_IS_AVAILABLE) ) {
				rp->flags |= RF_IS_AVAILABLE;

#ifndef NO_NETWORK
				// tell all of the clients.
				if ( MULTIPLAYER_MASTER ) {
					send_reinforcement_avail( i );
				}
#endif
			}
			return;
		}
	}

	Assert ( i < Num_reinforcements );
}

// mission_did_ship_arrive takes a parse object and checked the arrival cue and delay and
// creates the object if necessary.  Returns -1 if not created.  objnum of created ship otherwise
int mission_did_ship_arrive(p_object *objp)
{
	int did_arrive;

	// find out in the arrival cue became true
	did_arrive = eval_sexp(objp->arrival_cue);

	// we must first check to see if this ship is a reinforcement or not.  If so, then don't
	// process
	if ( objp->flags & P_SF_REINFORCEMENT ) {

		// if this ship did arrive, mark the reinforcement as available, and tell clients if in multiplayer
		// mode
		if ( did_arrive ) {
			mission_parse_mark_reinforcement_available(objp->name);
		}
		return -1;
	}

	if ( did_arrive ) { 		// has the arrival criteria been met?
		int object_num;		

		Assert ( !(objp->flags & P_SF_CANNOT_ARRIVE) );		// get allender

		// check to see if the delay field <= 0.  if so, then create a timestamp and then maybe
		// create the object
		if ( objp->arrival_delay <= 0 ) {
			objp->arrival_delay = timestamp( -objp->arrival_delay * 1000 );
			Assert( objp->arrival_delay >= 0 );
		}
		
		// if the timestamp hasn't elapsed, move onto the next ship.
		if ( !timestamp_elapsed(objp->arrival_delay) )
			return -1;

		// check to see if this ship is to arrive via a docking bay.  If so, and the ship to arrive from
		// doesn't exist, don't create.
		if ( objp->arrival_location == ARRIVE_FROM_DOCK_BAY ) {
			int shipnum;
			char *name;

			Assert( objp->arrival_anchor >= 0 );
			name = Parse_names[objp->arrival_anchor];
	
			// see if ship is yet to arrive.  If so, then return -1 so we can evaluate again later.
			if ( mission_parse_get_arrival_ship( name ) )
				return -1;

			// see if ship is in mission.  If not, then we can assume it was destroyed or departed since
			// it is not on the arrival list (as shown by above if statement).
			shipnum = ship_name_lookup( name );
			if ( shipnum == -1 ) {
				Sexp_nodes[objp->arrival_cue].value = SEXP_KNOWN_FALSE;
				return -1;
			}
		}

		object_num = parse_create_object(objp);							// create the ship

		// since this ship is not in a wing, create a SHIP_ARRIVE entry
		//mission_log_add_entry( LOG_SHIP_ARRIVE, objp->name, NULL );
		Assert(object_num >= 0 && object_num < MAX_OBJECTS);
		
		// Play the music track for an arrival
		if ( !(Ships[Objects[object_num].instance].flags & SF_NO_ARRIVAL_MUSIC) )
			if ( timestamp_elapsed(Allow_arrival_music_timestamp) ) {
				Allow_arrival_music_timestamp = timestamp(ARRIVAL_MUSIC_MIN_SEPARATION);
				event_music_arrival(Ships[Objects[object_num].instance].team);
			}
		return object_num;
	} else {
		// check to see if the arrival cue of this ship is known false -- if so, then remove
		// the parse object from the ship
		if ( Sexp_nodes[objp->arrival_cue].value == SEXP_KNOWN_FALSE )
			objp->flags |= P_SF_CANNOT_ARRIVE;
	}

	return -1;

}

// funciton to set a flag on all parse objects on ship arrival list which cannot
// arrive in the mission
void mission_parse_mark_non_arrivals()
{
	p_object *pobjp;

	for ( pobjp = GET_FIRST(&ship_arrival_list); pobjp != END_OF_LIST(&ship_arrival_list); pobjp = GET_NEXT(pobjp) ) {
		if ( pobjp->wingnum != -1 ) {
			if ( Sexp_nodes[Wings[pobjp->wingnum].arrival_cue].value == SEXP_KNOWN_FALSE )
				pobjp->flags |= P_SF_CANNOT_ARRIVE;
		} else {
			if ( Sexp_nodes[pobjp->arrival_cue].value == SEXP_KNOWN_FALSE )
				pobjp->flags |= P_SF_CANNOT_ARRIVE;
		}
	}
}

// function to deal with support ship arrival.  objnum is the object number of the arriving support
// ship.  This function can get called from either single or multiplayer.  Needed to that clients
// can know when to abort rearm.
void mission_parse_support_arrived( int objnum )
{
	int i;

	// when the support ship arrives, the shipname it is supposed to repair is in the 'misc'
	// field of the parse_object.  If the ship still exists, call ai function which actually
	// issues the goal for the repair
	for ( i = 0; i < Num_arriving_repair_targets; i++ ) {
		int shipnum;

		shipnum = ship_name_lookup( Arriving_repair_targets[i] );

		if ( shipnum != -1 ) {
			object *requester_objp, *support_objp;

			support_objp = &Objects[objnum];
			requester_objp = &Objects[Ships[shipnum].objnum];
			ai_add_rearm_goal( requester_objp, support_objp );
		}
	}

	//	MK: A bit of a hack.  If on player's team and player isn't allowed shields, don't give this ship shields.
	if ((Player_obj->flags & OF_NO_SHIELDS) && (Player_ship->team == Ships[Objects[objnum].instance].team))
		Objects[objnum].flags |= OF_NO_SHIELDS;

	Ships[Objects[objnum].instance].flags |= SF_WARPED_SUPPORT;

	Arriving_support_ship = NULL;
	Num_arriving_repair_targets = 0;
}

MONITOR(NumShipArrivals);

// mission_parse_arrivals will parse the lists of arriving ships and
// wings -- creating new ships/wings if the arrival criteria have been
// met.
void mission_eval_arrivals()
{
	p_object *objp;
	wing *wingp;
	int i, objnum;

	// before checking arrivals, check to see if we should play a message concerning arrivals
	// of other wings.  We use the timestamps to delay the arrival message slightly for
	// better effect
#ifndef NO_NETWORK
	if ( timestamp_valid(Arrival_message_delay_timestamp) && timestamp_elapsed(Arrival_message_delay_timestamp) && !((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)) ){
#else
	if ( timestamp_valid(Arrival_message_delay_timestamp) && timestamp_elapsed(Arrival_message_delay_timestamp) ) {
#endif
		int rship, use_terran;

		// use terran command 25% of time
		use_terran = ((frand() - 0.75) > 0.0f)?1:0;

		rship = ship_get_random_player_wing_ship( SHIP_GET_NO_PLAYERS );
		if ( (rship == -1) || use_terran ){
			message_send_builtin_to_player( MESSAGE_ARRIVE_ENEMY, NULL, MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1 );
		} else if ( rship != -1 ) {
			message_send_builtin_to_player( MESSAGE_ARRIVE_ENEMY, &Ships[rship], MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1 );
		}

		Arrival_message_delay_timestamp = timestamp(-1);		// make the stamp invalid
	}

//	if ( !timestamp_elapsed(Mission_arrival_timestamp) )
//		return;

	// check the ship_arrival_list
	objnum = -1;
	objp = GET_FIRST(&ship_arrival_list);
	while( objp !=END_OF_LIST(&ship_arrival_list) )	{
		p_object *temp = GET_NEXT(objp);
		if ( objp->wingnum == -1 )	{								// if this object has a wing -- let code for wings determine if it should be created

			objnum = mission_did_ship_arrive( objp );
			if ( objnum != -1 )	{
				list_remove( &ship_arrival_list, objp);
				MONITOR_INC(NumShipArrivals,1);
			}

		}
		objp = temp;
	}

	// check for any initially docked ships.  Do it after all are created since the next function
	// messes with the ship_arrival_list
	mission_parse_do_initial_docks();			// maybe create it's docked counterpart

	mission_parse_mark_non_arrivals();			// mark parse objects which can no longer arrive

	// check the support ship arrival list
	if ( Arriving_support_ship )	{
		int objnum;

		objnum = mission_did_ship_arrive( Arriving_support_ship );

		if ( objnum != -1 ) {
			MONITOR_INC(NumShipArrivals,1);
			mission_parse_support_arrived( objnum );
		}
	}

	// we must also check to see if there are waves of a wing that must
	// reappear if all the ships of the current wing have been destroyed or
	// have departed. If this is the case, then create the next wave.

	for ( i = 0; i < num_wings; i++ ) {
		wingp = &Wings[i];

		// should we process this wing anymore
		if ( wingp->flags & WF_WING_GONE )
			continue;

		// if we have a reinforcement wing, then don't try to create new ships automatically.
		if ( wingp->flags & WF_REINFORCEMENT ) {

			// check to see in the wings arrival cue is true, and if so, then mark the reinforcement
			// as available
			if ( eval_sexp(wingp->arrival_cue) ) {
				mission_parse_mark_reinforcement_available(wingp->name);
			}
			continue;
		}
		
		// don't do evaluations for departing wings
		if ( wingp->flags & WF_WING_DEPARTING ){
			continue;
		}

		// must check to see if we are at the last wave.  Code above to determine when a wing is gone only
		// gets run when a ship is destroyed (not every N seconds like it used to).  Do a quick check
		// here.
		if ( wingp->current_wave == wingp->num_waves ){
			continue;
		}

		// if the current wave of this wing is 0, then we haven't created the ships in the wing yet.
		// call parse_wing_create_ships to try and create it.  That function will eval the arrival
		// cue of the wing and create the ships if necessary, or if the threshold of the wing has
		// been reached, then try and create more ships
		if ( (wingp->current_wave == 0) || (wingp->current_count <= wingp->threshold) ) {
			int created;

			created = parse_wing_create_ships( wingp, wingp->wave_count );

			// if we created ships in this wing, check to see if the wings was int the reinforcements
			// array.  If so, then if we have more uses, then reset the reinforcement flag for the wing
			// so the user can call in another set if need be.
			if ( created > 0 ) {
				int rship;

				mission_parse_do_initial_docks();		// maybe create other initially docked ships
				if ( Wings[i].flags & WF_RESET_REINFORCEMENT ) {
					Wings[i].flags &= ~WF_RESET_REINFORCEMENT;
					Wings[i].flags |= WF_REINFORCEMENT;
				}

				// possibly send a message to the player when this wing arrives.
				if ( wingp->flags & WF_NO_ARRIVAL_MESSAGE ){
					continue;
				}

#ifndef NO_NETWORK
				// multiplayer team vs. team
				if((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_TEAM)){
					// send a hostile wing arrived message
					rship = Wings[i].ship_index[0];

					int multi_team_filter = Ships[rship].team == TEAM_FRIENDLY ? 1 : 0;

					// there are two timestamps at work here.  One to control how often the player receives
					// messages about incoming hostile waves, and the other to control how long after
					// the wing arrives does the player actually get the message.
					if ( timestamp_elapsed(Allow_arrival_message_timestamp_m[multi_team_filter]) ) {
						if ( !timestamp_valid(Arrival_message_delay_timestamp_m[multi_team_filter]) ){
							Arrival_message_delay_timestamp_m[multi_team_filter] = timestamp_rand(ARRIVAL_MESSAGE_DELAY_MIN, ARRIVAL_MESSAGE_DELAY_MAX );
						}
						Allow_arrival_message_timestamp_m[multi_team_filter] = timestamp(ARRIVAL_MESSAGE_MIN_SEPARATION);					
						
						// send to the proper team
						message_send_builtin_to_player( MESSAGE_ARRIVE_ENEMY, NULL, MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, multi_team_filter );
					}
				} 
				else
#endif
				{
					// everything else
					// see if this is a starting player wing
					if ( i == Starting_wings[STARTING_WING_BETA] ) {					// this is the beta wing
						rship = ship_get_random_ship_in_wing( i, SHIP_GET_NO_PLAYERS );
						if ( rship != -1 ){
							message_send_builtin_to_player( MESSAGE_BETA_ARRIVED, &Ships[rship], MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1 );
						}
					} else if ( i == Starting_wings[STARTING_WING_GAMMA] ) {			// this is the gamma wing
						rship = ship_get_random_ship_in_wing( i, SHIP_GET_NO_PLAYERS );
						if ( rship != -1 ) {
							message_send_builtin_to_player( MESSAGE_GAMMA_ARRIVED, &Ships[rship], MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1 );
						}
					} else if ( !stricmp( wingp->name, "delta") ) {
						rship = ship_get_random_ship_in_wing( i, SHIP_GET_NO_PLAYERS );
						if ( rship != -1 ) {
							message_send_builtin_to_player( MESSAGE_DELTA_ARRIVED, &Ships[rship], MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1 );
						}
					} else if ( !stricmp(wingp->name, "epsilon") ) {
						rship = ship_get_random_ship_in_wing( i, SHIP_GET_NO_PLAYERS );
						if ( rship != -1 ) {
							message_send_builtin_to_player( MESSAGE_EPSILON_ARRIVED, &Ships[rship], MESSAGE_PRIORITY_LOW, MESSAGE_TIME_SOON, 0, 0, -1, -1 );
						}
					} else {
						// see if we have a hostile wing that arrived
						rship = Wings[i].ship_index[0];
						if ( Ships[rship].team != TEAM_FRIENDLY ) {

							// there are two timestamps at work here.  One to control how often the player receives
							// messages about incoming hostile waves, and the other to control how long after
							// the wing arrives does the player actually get the message.
							if ( timestamp_elapsed(Allow_arrival_message_timestamp) ) {
								if ( !timestamp_valid(Arrival_message_delay_timestamp) ){
									Arrival_message_delay_timestamp = timestamp_rand(ARRIVAL_MESSAGE_DELAY_MIN, ARRIVAL_MESSAGE_DELAY_MAX );
								}
								Allow_arrival_message_timestamp = timestamp(ARRIVAL_MESSAGE_MIN_SEPARATION);
							}
						}
					}
				}
			}
		}
	}
	Mission_arrival_timestamp = timestamp(ARRIVAL_TIMESTAMP);
}

MONITOR(NumShipDepartures);

// called to make object objp depart.
void mission_do_departure( object *objp )
{
	ship *shipp;
//	vector v;

	MONITOR_INC(NumShipDepartures,1);

	Assert ( objp->type == OBJ_SHIP );
	shipp = &Ships[objp->instance];

	// if departing to a docking bay, try to find the anchor ship to depart to.  If not found, then
	// just make it warp out like anything else.
	if ( shipp->departure_location == DEPART_AT_DOCK_BAY ) {
		int anchor_shipnum;
		char *name;

		Assert( shipp->departure_anchor >= 0 );
		name = Parse_names[shipp->departure_anchor];

		// see if ship is yet to arrive.  If so, then return -1 so we can evaluate again later.
		if ( mission_parse_get_arrival_ship( name ) )
			goto do_departure_warp;

		// see if ship is in mission.  If not, then we can assume it was destroyed or departed since
		// it is not on the arrival list (as shown by above if statement).
		anchor_shipnum = ship_name_lookup( name );
		if ( anchor_shipnum == -1 )
			goto do_departure_warp;

		ai_acquire_depart_path(objp, Ships[anchor_shipnum].objnum);
		return;
	}

do_departure_warp:
	ai_set_mode_warp_out( objp, &Ai_info[Ships[objp->instance].ai_index] );

}

// put here because mission_eval_arrivals is here.  Might move these to a better location
// later -- MWA
void mission_eval_departures()
{
	int i, j;
	object *objp;
	wing *wingp;

//	if ( !timestamp_elapsed(Mission_departure_timestamp) )
//		return;

	// scan through the active ships an evaluate their departure cues.  For those
	// ships whose time has come, set their departing flag.

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->type == OBJ_SHIP) {
			ship	*shipp;

			Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));

			shipp = &Ships[objp->instance];
			
			// don't process a ship that is already departing or dying or disabled
			// AL 12-30-97: Added SF_CANNOT_WARP to check
			if ( (shipp->flags & (SF_DEPARTING | SF_DYING | SF_CANNOT_WARP )) || ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE) ) {
				continue;
			}

			// don't process ships that are part of a wing -- handled in seperate case
			if ( shipp->wingnum != -1 )
				continue;

//				&& (!timestamp_valid(shipp->departure_delay) || timestamp_elapsed(shipp->departure_delay)) )
			// when the departure cue becomes true, set off the departure delay timer.  We store the
			// timer as -seconds in Freespace which indicates that the timer has not been set.  If the timer
			// is not set, then turn it into a valid timer and keep evaluating the timer until it is elapsed
			if ( eval_sexp(shipp->departure_cue) ) {
				if ( shipp->departure_delay <= 0 )
					shipp->departure_delay = timestamp(-shipp->departure_delay * 1000 );
				if ( timestamp_elapsed(shipp->departure_delay) )
					mission_do_departure( objp );
			}
		}
	}

	// now scan through the list of wings and check their departure cues.  For wings with
	// that cue being true, we must update internal variables to indicate that the wing is
	// departed and that no further waves of this wing will appear

	for ( i = 0; i < num_wings; i++ ) {
		wingp = &Wings[i];

		// should we process this wing anymore
		if ( wingp->flags & WF_WING_DEPARTING )
			continue;

		// evaluate the sexpression.  If true, mark all the ships in this wing as departing and increment
		// the num departed in the wing structure.  Then add number of remaining waves * ships/wave to
		// departed count to get total count of ships in the wing which departed.  (We are counting ships
		// that have not yet arrived as departed if they never arrive -- this may be bad, but for some reason
		// seems like the right thing to do).
 //&& (!timestamp_valid(wingp->departure_delay) || timestamp_elapsed(wingp->departure_delay)) ) {

		if ( eval_sexp(wingp->departure_cue) ) {
			// if we haven't set up the departure timer yet (would be <= 0) setup the timer to pop N seconds
			// later
			if ( wingp->departure_delay <= 0 )
				wingp->departure_delay = timestamp( -wingp->departure_delay * 1000 );
			if ( !timestamp_elapsed(wingp->departure_delay) )
				continue;

			wingp->flags |= WF_WING_DEPARTING;
			for ( j = 0; j < wingp->current_count; j++ ) {
				ship *shipp;

				shipp = &Ships[wingp->ship_index[j]];
				if ( (shipp->flags & SF_DEPARTING) || (shipp->flags & SF_DYING) )
					continue;

//				shipp->flags |= SF_DEPARTING;
//				shipp->final_depart_time = timestamp(3*1000);

				Assert ( shipp->objnum != -1 );
				objp = &Objects[shipp->objnum];

				// copy the wing's depature information to the ship
				shipp->departure_location = wingp->departure_location;
				shipp->departure_anchor = wingp->departure_anchor;

				mission_do_departure( objp );
				// don't add to wingp->total_departed here -- this is taken care of in ship code.
			}

			// MWA 2/25/98 -- don't do the follwoing wing member updates.  It makes the accurate counts
			// sort of messed up and causes problems for the event log.  The code in ship_wing_cleanup()
			// now keys off of the WF_WING_DEPARTING flag instead of the counts below.

			/*
			// now be sure that we update wing structure members if there are any remaining waves left
			if ( wingp->current_wave < wingp->num_waves ) {
				int num_remaining;

				num_remaining = ( (wingp->num_waves - wingp->current_wave) * wingp->wave_count);
				wingp->total_departed += num_remaining;
				wingp->total_arrived_count += num_remaining;
				wingp->current_wave = wingp->num_waves;
			}
			*/

		}
	}
	Mission_departure_timestamp = timestamp(DEPARTURE_TIMESTAMP);
}

// function called from high level game loop to do mission evaluation stuff
void mission_parse_eval_stuff()
{
	mission_eval_arrivals();
	mission_eval_departures();
}

int allocate_subsys_status()
{
	int i;
	// set primary weapon ammunition here, but does it actually matter? - Goober5000

	Assert(Subsys_index < MAX_SUBSYS_STATUS);
	Subsys_status[Subsys_index].percent = 0.0f;

	Subsys_status[Subsys_index].primary_banks[0] = SUBSYS_STATUS_NO_CHANGE;
	Subsys_status[Subsys_index].primary_ammo[0] = 100; // *
	
	for (i=1; i<MAX_PRIMARY_BANKS; i++)
	{
		Subsys_status[Subsys_index].primary_banks[i] = -1;  // none
		Subsys_status[Subsys_index].primary_ammo[i] = 100;	// *
	}

	Subsys_status[Subsys_index].secondary_banks[0] = SUBSYS_STATUS_NO_CHANGE;
	Subsys_status[Subsys_index].secondary_ammo[0] = 100;
	
	for (i=1; i<MAX_SECONDARY_BANKS; i++)
	{
		Subsys_status[Subsys_index].secondary_banks[i] = -1;
		Subsys_status[Subsys_index].secondary_ammo[i] = 100;
	}

	Subsys_status[Subsys_index].ai_class = SUBSYS_STATUS_NO_CHANGE;
	return Subsys_index++;
}

// find (or add) the name in the list and return an index to it.
int get_parse_name_index(char *name)
{
	int i;

	for (i=0; i<Num_parse_names; i++)
		if (!stricmp(name, Parse_names[i]))
			return i;

	Assert(i < MAX_SHIPS + MAX_WINGS);
	Assert(strlen(name) < NAME_LENGTH);
	strcpy(Parse_names[i], name);
	return Num_parse_names++;
}

int get_anchor(char *name)
{
	int i;

	for (i=0; i<MAX_SPECIAL_ARRIVAL_ANCHORS; i++)
		if (!stricmp(name, Special_arrival_anchor_names[i]))
			return SPECIAL_ARRIVAL_ANCHORS_OFFSET + i;

	return get_parse_name_index(name);
}

// function to fixup the goals/ai references for player objects in the mission
void mission_parse_fixup_players()
{
	object *objp;

	// merge created list to have all objects on used list
	obj_merge_created_list();
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( (objp->type == OBJ_SHIP) && (objp->flags & OF_PLAYER_SHIP) ) {
			ai_clear_ship_goals( &Ai_info[Ships[objp->instance].ai_index] );
			init_ai_object( OBJ_INDEX(objp) );
		}
	}
}

// code to warp in a new support ship.  It works by finding the average position of all ships
// in the mission, creating a vector from that position to the player, and scaling out behind the
// player some distance.  Should be sufficient.

#define WARP_IN_MIN_DISTANCE	1000.0f
#define WARP_IN_TIME_MIN		3000				// warps in min 3 seconds later
#define WARP_IN_TIME_MAX		6000				// warps in max 6 seconds later

// function which adds requester_objp onto the queue of ships for the arriving support ship to service
void mission_add_to_arriving_support( object *requester_objp )
{
	int i;
	ship *shipp;

	Assert ( Arriving_support_ship );

	if ( Num_arriving_repair_targets == MAX_AI_GOALS ) {
		// Int3();			// get allender -- ship isn't going to get repair, but I hope they never queue up this far!!!
		mprintf(("Reached MAX_AI_GOALS trying to add repair request!\n"));
		return;
	}

	shipp = &Ships[requester_objp->instance];
	// check for duplicates before adding
	for (i = 0; i < Num_arriving_repair_targets; i++ ) {
		if ( !stricmp(Arriving_repair_targets[i], shipp->ship_name) ){
			break;
		}
	}
	if ( i != Num_arriving_repair_targets ){		// found the ship before reaching the end -- ignore it!
		return;
	}

	strcpy( Arriving_repair_targets[Num_arriving_repair_targets], Ships[requester_objp->instance].ship_name );
	Num_arriving_repair_targets++;

#ifndef NO_NETWORK
	if ( MULTIPLAYER_MASTER ){
		multi_maybe_send_repair_info( requester_objp, NULL, REPAIR_INFO_WARP_ADD );
	}	
#endif
}

extern int pp_collide_any(vector *curpos, vector *goalpos, float radius, object *ignore_objp1, object *ignore_objp2, int big_only_flag);

//	Set the warp in position for a support ship relative to an object.
//	Caller tries several positions, passing vector in x, y, z.
int get_warp_in_pos(vector *pos, object *objp, float x, float y, float z)
{
	float	rand_val;

	if ( Game_mode & GM_NORMAL )
		rand_val = frand();
	else
		rand_val = static_randf(objp->net_signature);

	rand_val = 1.0f + (rand_val - 0.5f)*0.2f;

	*pos = objp->pos;

	vm_vec_scale_add2( pos, &objp->orient.vec.rvec, x*rand_val*800.0f);
	vm_vec_scale_add2( pos, &objp->orient.vec.uvec, y*rand_val*800.0f);
	vm_vec_scale_add2( pos, &objp->orient.vec.fvec, z*rand_val*800.0f);

	return pp_collide_any(&objp->pos, pos, objp->radius, objp, NULL, 1);
}

void mission_warp_in_support_ship( object *requester_objp )
{
	vector center, warp_in_pos;
	//float mag;
	p_object *pobj;
	int i, requester_species;
	ship *requester_shipp;

	Assert ( requester_objp->type == OBJ_SHIP );
	requester_shipp = &Ships[requester_objp->instance];	//	MK, 10/23/97, used to be ->type, bogus, no?

	// if the support ship is already arriving, add the requester to the list
	if ( Arriving_support_ship ) {
		mission_add_to_arriving_support( requester_objp );
		return;
	}
	
	// get average position of all ships
	obj_get_average_ship_pos( &center );
	vm_vec_sub( &warp_in_pos, &center, &(requester_objp->pos) );

	// be sure to account for case as player being only ship left in mission
	/*
	if ( !(IS_VEC_NULL( warp_in_pos)) ) {
		mag = vm_vec_mag( &warp_in_pos );
		if ( mag < WARP_IN_MIN_DISTANCE )
			vm_vec_scale( &warp_in_pos, WARP_IN_MIN_DISTANCE/mag);
		else
			vm_vec_scale( &warp
	} else {
		// take -player_pos.vec.fvec scaled by 1000.0f;
		warp_in_pos = Player_obj->orient.vec.fvec;
		vm_vec_scale( &warp_in_pos, -1000.0f );
	}
	*/

	//	Choose position to warp in ship.
	//	Temporary, but changed by MK because it used to be exactly behind the player.
	//	This could cause an Assert if the player immediately targeted it (before moving).
	//	Tend to put in front of the player to aid him in flying towards the ship.

	if (!get_warp_in_pos(&warp_in_pos, requester_objp, 1.0f, 0.1f, 1.0f))
		if (!get_warp_in_pos(&warp_in_pos, requester_objp, 1.0f, 0.2f, -1.0f))
			if (!get_warp_in_pos(&warp_in_pos, requester_objp, -1.0f, -0.2f, -1.0f))
				if (!get_warp_in_pos(&warp_in_pos, requester_objp, -1.0f, -0.1f, 1.0f))
					get_warp_in_pos(&warp_in_pos, requester_objp, 0.1f, 1.0f, 0.2f);

	// create a parse object, and put it onto the ship_arrival_list.  This whole thing kind of sucks.
	// I want to put it into a parse object since it needs to arrive just a little later than
	// this function is called.  I have to make some assumptions in the code about values for the parse
	// object since I'm no longer working with a mission file.  These exceptions will be noted with
	// comments

	Arriving_support_ship = &Support_ship_pobj;
	pobj = Arriving_support_ship;

	// create a name for the ship.  use "Support #".  look for collisions until one isn't found anymore
	i = 1;
	do {
		sprintf(pobj->name, NOX("Support %d"), i);
		if ( (ship_name_lookup(pobj->name) == -1) && (ship_find_exited_ship_by_name(pobj->name) == -1) )
			break;
		i++;
	} while(1);

	pobj->pos = warp_in_pos;
	vm_set_identity( &(pobj->orient) );

	// *sigh*.  Gotta get the ship class.  For now, this will amount to finding a ship in the ship_info
	// array with the same team as the requester of type SIF_SUPPORT.  Might need to be changed, but who knows
	// vasudans use the terran support ship.
	requester_species = Ship_info[requester_shipp->ship_info_index].species;

	// 5/6/98 -- MWA  Don't need to do anything for multiplayer.  I think that we always want to use
	// the species of the caller ship.
	Assert( (requester_species == SPECIES_TERRAN) || (requester_species == SPECIES_VASUDAN) );
//	if ( (Game_mode & GM_NORMAL) && (requester_species == SPECIES_VASUDAN) )	{	// make vasundan's use the terran support ship
//		requester_species = SPECIES_TERRAN;
//	}

	// get index of correct species support ship
	for (i=0; i < Num_ship_types; i++) {
		if ( (Ship_info[i].species == requester_species) && (Ship_info[i].flags & SIF_SUPPORT) )
			break;
	}

	if ( i < Num_ship_types )
		pobj->ship_class = i;
	else
		Int3();				// BOGUS!!!!  gotta figure something out here

	pobj->team = requester_shipp->team;

	pobj->behavior = AIM_NONE;		// ASSUMPTION:  the mission file has the string "None" which maps to AIM_NONE

	// set the ai_goals to -1.  We will put the requester object shipname in repair target array and then take
	// care of setting up the goal when creating the ship!!!!
	pobj->ai_goals = -1;
	Num_arriving_repair_targets = 0;
	mission_add_to_arriving_support( requester_objp );

	// need to set ship's cargo to nothing.  scan the cargo_names array looking for the string nothing.
	// add it if not found
	for (i = 0; i < Num_cargo; i++ )
		if ( !stricmp(Cargo_names[i], NOX("nothing")) )
			break;

	if ( i == Num_cargo ) {
		strcpy(Cargo_names[i], NOX("Nothing"));
		Num_cargo++;
	}
	pobj->cargo1 = char(i);

	pobj->status_count = 0;

	pobj->arrival_location = 0;			// ASSUMPTION: this is index to arrival_lcation string array for hyperspace!!!!
	pobj->arrival_distance = 0;
	pobj->arrival_anchor = -1;
	pobj->arrival_cue = Locked_sexp_true;
	pobj->arrival_delay = timestamp_rand(WARP_IN_TIME_MIN, WARP_IN_TIME_MAX);

	pobj->subsys_count = 0;				// number of elements used in subsys_status array
	pobj->initial_velocity = 100;		// start at 100% velocity
	pobj->initial_hull = 100;			// start at 100% hull	
	pobj->initial_shields = 100;		// and 100% shields

	pobj->departure_location = 0;		// ASSUMPTION: this is index to departure_lcation string array for hyperspace!!!!
	pobj->departure_anchor = -1;
	pobj->departure_cue = Locked_sexp_false;
	pobj->departure_delay= 0;

	pobj->determination = 10;			// ASSUMPTION:  mission file always had this number written out
	pobj->wingnum = -1;
	if ( Player_obj->flags & P_OF_NO_SHIELDS )
		pobj->flags = P_OF_NO_SHIELDS;	// support ships have no shields when player has not shields

	pobj->ai_class = Ship_info[pobj->ship_class].ai_class;
	pobj->hotkey = -1;
	pobj->score = 0;

	pobj->docked_with[0] = '\0';
	pobj->group = -1;
	pobj->persona_index = -1;
	pobj->net_signature = multi_assign_network_signature(MULTI_SIG_SHIP);
	pobj->wing_status_wing_index = -1;
	pobj->wing_status_wing_pos = -1;
	pobj->respawn_count = 0;
	pobj->alt_type_index = -1;
	pobj->nameplate = -1;

}

// returns true if a support ship is currently in the process of warping in.
int mission_is_support_ship_arriving()
{
	if ( Arriving_support_ship )
		return 1;
	else
		return 0;
}

// returns true if the given ship is scheduled to be repaired by the arriving support ship
int mission_is_repair_scheduled( object *objp )
{
	char *name;
	int i;

	if ( !Arriving_support_ship )
		return 0;

	Assert ( objp->type == OBJ_SHIP );
	name = Ships[objp->instance].ship_name;
	for (i = 0; i < Num_arriving_repair_targets; i++ ) {
		if ( !strcmp( name, Arriving_repair_targets[i]) )
			return 1;
	}

	return 0;
}

// function which removed the given ship from the list of ships that are to get repair
// by arriving support ship
int mission_remove_scheduled_repair( object *objp )
{
	char *name;
	int i, index;

	if ( !Arriving_support_ship )
		return 0;

	// itereate through the target list looking for this ship name.  If not found, we
	// can simply return.
	Assert ( objp->type == OBJ_SHIP );
	name = Ships[objp->instance].ship_name;
	for (index = 0; index < Num_arriving_repair_targets; index++ ) {
		if ( !strcmp( name, Arriving_repair_targets[index]) )
			break;
	}
	if ( index == Num_arriving_repair_targets )
		return 0;

	// ship is found -- compress the array
	for ( i = index; i < Num_arriving_repair_targets - 1; i++ )
		strcpy( Arriving_repair_targets[i], Arriving_repair_targets[i+1] );

	Num_arriving_repair_targets--;

#ifndef NO_NETWORK
	if ( MULTIPLAYER_MASTER )
		multi_maybe_send_repair_info( objp, NULL, REPAIR_INFO_WARP_REMOVE );
#endif

	return 1;
}

// alternate name stuff
int mission_parse_lookup_alt(char *name)
{
	int idx;

	// sanity
	if(name == NULL){
		return -1;
	}

	// lookup
	for(idx=0; idx<Mission_alt_type_count; idx++){
		if(!strcmp(Mission_alt_types[idx], name)){
			return idx;
		}
	}

	// could not find
	return -1;
}

static int mission_parse_lookup_alt_index_warn = 1;
void mission_parse_lookup_alt_index(int index, char *out)
{
	// sanity
	if(out == NULL){
		return;
	}
	if((index < 0) || (index > Mission_alt_type_count)){
		if (mission_parse_lookup_alt_index_warn) {
			Warning(LOCATION, "Ship with invalid alt_name.  Get a programmer");
			mission_parse_lookup_alt_index_warn = 0;
		}
		return;
	}

	// stuff it
	strcpy(out, Mission_alt_types[index]);
}

int mission_parse_add_alt(char *name)
{
	// sanity
	if(name == NULL){
		return -1;
	}

	// maybe add
	if(Mission_alt_type_count < MAX_ALT_TYPE_NAMES){
		// stuff the name
		strncpy(Mission_alt_types[Mission_alt_type_count++], name, NAME_LENGTH);

		// done
		return Mission_alt_type_count - 1;
	}

	return -1;
}

void mission_parse_reset_alt()
{
	Mission_alt_type_count = 0;
}

int is_training_mission()
{
	return (The_mission.game_type & MISSION_TYPE_TRAINING);
}
