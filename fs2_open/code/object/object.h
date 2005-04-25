/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Object/Object.h $
 * $Revision: 2.14 $
 * $Date: 2005-04-25 00:28:58 $
 * $Author: wmcoolmon $
 *
 * <insert description of file here>
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.13  2005/04/05 05:53:21  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.12  2005/03/27 12:28:32  Goober5000
 * clarified max hull/shield strength names and added ship guardian thresholds
 * --Goober5000
 *
 * Revision 2.11  2005/03/25 06:57:36  wmcoolmon
 * Big, massive, codebase commit. I have not removed the old ai files as the ones I uploaded aren't up-to-date (But should work with the rest of the codebase)
 *
 * Revision 2.10  2005/03/03 06:05:30  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.9  2005/02/04 10:12:32  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.8  2005/01/11 21:38:49  Goober5000
 * multiple ship docking :)
 * don't tell anyone yet... check the SCP internal
 * --Goober500
 *
 * Revision 2.7  2004/08/11 05:06:29  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.6  2004/05/10 08:03:30  Goober5000
 * fixored the handling of no lasers and no engines... the tests should check the ship,
 * not the object
 * --Goober5000
 *
 * Revision 2.5  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.4  2004/02/04 08:41:02  Goober5000
 * made code more uniform and simplified some things,
 * specifically shield percentage and quadrant stuff
 * --Goober5000
 *
 * Revision 2.3  2003/04/29 01:03:22  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.2  2002/12/10 05:43:33  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.1  2002/08/01 01:41:08  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 17    8/16/99 3:53p Andsager
 * Add special warp in interface in Fred and saving / reading.
 * 
 * 16    7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 15    7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 14    6/28/99 4:51p Andsager
 * Add ship-guardian sexp (does not allow ship to be killed)
 * 
 * 13    5/18/99 11:50a Andsager
 * Remove unused object type OBJ_GHOST_SAVE
 * 
 * 12    4/26/99 10:58a Andsager
 * Add OF_BEAM_PROTECTED flag to keep object from being targeted for zing.
 * 
 * 11    4/23/99 5:53p Dave
 * Started putting in new pof nebula support into Fred.
 * 
 * 10    4/21/99 6:15p Dave
 * Did some serious housecleaning in the beam code. Made it ready to go
 * for anti-fighter "pulse" weapons. Fixed collision pair creation. Added
 * a handy macro for recalculating collision pairs for a given object.
 * 
 * 9     3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 8     1/25/99 5:03a Dave
 * First run of stealth, AWACS and TAG missile support. New mission type
 * :)
 * 
 * 7     1/24/99 11:37p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 6     1/12/99 12:53a Dave
 * More work on beam weapons - made collision detection very efficient -
 * collide against all object types properly - made 3 movement types
 * smooth. Put in test code to check for possible non-darkening pixels on
 * object textures.
 * 
 * 5     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 4     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 3     10/16/98 3:42p Andsager
 * increase MAX_WEAPONS and MAX_SHIPS and som header files
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 96    6/30/98 2:25p Dave
 * Revamped object update system
 * 
 * 95    6/30/98 2:23p Dave
 * Revised object update system. Removed updates for all weapons. Put
 * button info back into control info packet.
 * 
 * 94    5/11/98 4:33p Allender
 * fixed ingame join problems -- started to work on new object updating
 * code (currently ifdef'ed out)
 * 
 * 93    5/01/98 12:59a Dave
 * Put in some test code for a new object update system. Found the problem
 * with the current system (low-level packet buffering). Gonna fix it :)
 * 
 * 92    4/02/98 11:40a Lawrance
 * check for #ifdef DEMO instead of #ifdef DEMO_RELEASE
 * 
 * 91    4/01/98 9:20a Mike
 * Reduce MAX_SHIPS, MAX_OBJECTS and make MAX_AI_INFO same as MAX_SHIPS
 * 
 * 90    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 89    3/26/98 5:43p Lawrance
 * rename ship_team_from_obj(), obj_team() and move to object lib
 * 
 * 88    3/12/98 1:24p Mike
 * When weapons linked, increase firing delay.
 * Free up weapon slots for AI ships, if necessary.
 * Backside render shield effect.
 * Remove shield hit triangle if offscreen.
 * 
 * 87    3/09/98 10:56a Hoffoss
 * Added jump node objects to Fred.
 * 
 * 86    3/07/98 2:34p Allender
 * more ingame join stuff.  Works in a basic fashion in this new system.
 * 
 * 85    2/27/98 4:48p John
 * Made objects keep track of number of pairs they have associated with
 * them.  Then, I can early out of the obj_remove_all which was 2.5% of
 * frametime at beginning of sm2-2 which then is 0% after this.
 * 
 * 84    2/26/98 3:46p Hoffoss
 * Externed object_inited variable for use in state restore.
 * 
 * 83    2/23/98 5:08p Allender
 * made net_signature an unsigned short.  Now using permanent and
 * non-permanent object "pools".
 * 
 * 82    2/05/98 12:51a Mike
 * Early asteroid stuff.
 * 
 * 81    1/18/98 5:09p Lawrance
 * Added support for TEAM_TRAITOR
 * 
 * 80    12/12/97 5:23p Lawrance
 * add TEAM_ANY #define... used for some targeting functions
 * 
 * 79    12/11/97 5:46p Hoffoss
 * Changed Fred to not display weapons that are not available to various
 * ships.
 * 
 * 78    11/03/97 5:38p Dave
 * Cleaned up more multiplayer sequencing. Added OBJ_OBSERVER module/type.
 * Restructured HUD_config structs/flags.
 * 
 * 77    10/23/97 4:41p Allender
 * lots of new rearm/repair code.  Rearm requests now queue as goals for
 * support ship.  Warp in of new support ships functional.  Support for
 * stay-still and play-dead.  
 * 
 * 76    10/16/97 4:40p Allender
 * removed object structure member (for multiplayer) -- moved to ship
 * structure
 * 
 * 75    9/30/97 5:05p Dave
 * Added OF_COULD_BE_PLAYER flag for objects which ingame joiners can
 * grab.
 * 
 * 74    9/25/97 4:50p Mike
 * Support ignoring wings.
 * 
 * 73    9/17/97 5:12p John
 * Restructured collision routines.  Probably broke a lot of stuff.
 * 
 * 72    9/15/97 4:43p Dave
 * Got basic observer mode working. Seems bug free so far.
 * 
 * 71    9/11/97 5:01p Dave
 * Minor changes to handle ingame joining/dropping for multiplayer.
 * 
 * 70    9/09/97 12:01a Mike
 * Further support for new team numbering, switched from 0, 1, 2, 3 to 1,
 * 2, 4, 8 to allow attacking of two simultaneous teams.
 * 
 * 69    9/08/97 10:24p Mike
 * Working on attacking multiple simultaneous teams.
 * 
 * 68    9/08/97 5:20p Dave
 * Added OF_IS_ORIGINAL flag
 * 
 * 67    9/06/97 2:13p Mike
 * Replace support for TEAM_NEUTRAL
 * 
 * 66    9/05/97 5:02p Lawrance
 * save/restore object pairs
 * 
 * 65    8/29/97 10:13a Allender
 * work on server/client prediction code -- doesn't work too well.  Made
 * all clients simulate their own orientation with the server giving
 * corrections every so often.
 * 
 * 64    8/20/97 7:54p Lawrance
 * make obj_reset_pairs() external
 * 
 * 63    8/16/97 3:53p Hoffoss
 * Added OF_NO_SHIELDS define and support in Fred and mission load/save.
 * 
 * 62    8/11/97 8:46p Hoffoss
 * Added a new object type for Fred usage.
 * 
 * 61    8/11/97 10:36a John
 * added new function that you should call when changing object flags.
 * 
 * 60    8/04/97 11:45a Lawrance
 * split off initialization of CheckObject[] elements into separate
 * function
 * 
 * 59    7/31/97 5:55p John
 * made so you pass flags to obj_create.
 * Added new collision code that ignores any pairs that will never
 * collide.
 * 
 * 58    7/30/97 9:40a Allender
 * added last_orient to object structure in anticipation of use for
 * multiplayer
 * 
 * 57    7/28/97 5:10p Hoffoss
 * Removed all occurances of neutral team from Fred.
 * 
 * 56    7/24/97 10:24a Mike
 * Restore support for Unknown team
 * 
 * 55    7/16/97 2:52p Lawrance
 * make shockwaves objects
 * 
 * 54    6/24/97 10:04a Allender
 * major multiplayer improvements.  Better sequencing before game.
 * Dealing with weapon/fireball/counter measure objects between
 * client/host.  
 * 
 * 53    6/23/97 10:12a Hoffoss
 * Added new object type for Fred.
 * 
 * 52    6/19/97 1:43p Allender
 * basic object syncing when a mission loads.  basic object update packets
 * every frame
 * 
 * 51    6/13/97 1:15p Allender
 * use player positions for multiplayer games.  No real error checking
 * yet, but you can get people into a mutiplayer mission now.
 * 
 * 50    6/06/97 4:13p Lawrance
 * use an index instead of a pointer for object-linked sounds
 * 
 * 49    6/05/97 6:08p Hoffoss
 * Added an object flag, and rearranged them to make it more organized.
 * 
 * 48    5/19/97 1:07p Mike
 * Add OBJ_GHOST and deal with a dead player better.
 * 
 * 47    5/14/97 4:08p Lawrance
 * removing my_index from game arrays
 * 
 * 46    5/12/97 6:00p Mike
 * Add countermeasures.
 * 
 * 45    5/09/97 4:34p Lawrance
 * move out obj_snd typedef
 * 
 * 44    5/08/97 4:30p Lawrance
 * split off object sound stuff into separate file
 * 
 * 43    5/06/97 9:36a Lawrance
 * added object-linked persistant sounds
 * 
 * 42    4/25/97 3:29p Mike
 * Making shield multi-part.
 * 
 * 41    4/24/97 4:39p Hoffoss
 * Waypoint navbouys are now objects of type OBJ_WAYPOINT and displaying
 * of them can be toggled on and off through DCF.
 * 
 * 40    4/10/97 3:20p Mike
 * Change hull damage to be like shields.
 * 
 * 39    3/07/97 4:37p Mike
 * Make rockeye missile home.
 * Remove UNKNOWN and NEUTRAL teams.
 * 
 * 38    3/05/97 12:49p John
 * added Viewer_obj.  Took out the interp_??? variables for turning
 * outline,etc on and put them in flags you pass to model_render.
 * Cleaned up model_interp code to fit new coding styles.
 * 
 * 37    2/07/97 9:09a John
 * Added new debris objects
 * 
 * 36    2/04/97 8:31a Adam
 * 
 * 35    1/21/97 1:37p Hoffoss
 * Added an object flag for Fred.
 * 
 * 34    1/20/97 7:58p John
 * Fixed some link errors with testcode.
 * 
 * 33    1/10/97 5:15p Mike
 * Moved ship-specific parameters from obj_subsystem to ship_subsys.
 * 
 * Added turret code to AI system.
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _OBJECT_H
#define _OBJECT_H

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "math/vecmat.h"
#include "physics/physics.h"

/*
 *		CONSTANTS
 */

#define MAX_SHIELD_SECTIONS	4					//	Number of sections in shield.

#ifndef NDEBUG
#define OBJECT_CHECK 
#endif

//Object types
#define OBJ_NONE				0		//unused object
#define OBJ_SHIP				1		//a ship
#define OBJ_WEAPON			2		//a laser, missile, etc
#define OBJ_FIREBALL			3		//an explosion
#define OBJ_START				4		//a starting point marker (player start, etc)
#define OBJ_WAYPOINT			5		//a waypoint object, maybe only ever used by Fred
#define OBJ_DEBRIS			6		//a flying piece of ship debris
#define OBJ_CMEASURE			7		//a countermeasure, such as chaff
#define OBJ_GHOST				8		//so far, just a placeholder for when a player dies.
#define OBJ_POINT				9		//generic object type to display a point in Fred.
#define OBJ_SHOCKWAVE		10		// a shockwave
#define OBJ_WING				11		// not really a type used anywhere, but I need it for Fred.
#define OBJ_OBSERVER       12    // used for multiplayer observers (possibly single player later)
#define OBJ_ASTEROID			13		//	An asteroid, you know, a big rock, like debris, sort of.
#define OBJ_JUMP_NODE		14		// A jump node object, used only in Fred.
#define OBJ_BEAM				15		// beam weapons. we have to roll them into the object system to get the benefits of the collision pairs

//Make sure to change Object_type_names in Object.c when adding another type!
#define MAX_OBJECT_TYPES	16

#define UNUSED_OBJNUM		(-MAX_OBJECTS*2)	//	Newer systems use this instead of -1 for invalid object.

#ifndef NDEBUG
extern char	*Object_type_names[MAX_OBJECT_TYPES];
#endif

// each object type should have these functions:  (I will use weapon as example)
//
// int weapon_create( weapon specific parameters )
// {
//    ...
//		objnum = obj_create();
//		... Do some check to correctly handle obj_create returning  which
//        means that that object couldn't be created
//    ... Initialize the weapon-specific info in Objects[objnum]
//    return objnum;
// }
//
// void weapon_delete( object * obj )
// {
//    {Put a call to this in OBJECT.C, function obj_delete_all_that_should_be_dead }
//    WARNING: To kill an object, set it's OF_SHOULD_BE_DEAD flag.  Then,
//    this function will get called when it's time to clean up the data.
//    Assert( obj->flags & OF_SHOULD_BE_DEAD );
//    ...
//    ... Free up all weapon-specfic data
//    obj_delete(objnum);
// }
// 
// void weapon_move( object * obj )
// {
//    {Put a call to this in ??? }
//    ... Do whatever needs to be done each frame.  Usually this amounts
//        to setting the thrust, seeing if we've died, etc.
// }
//
// int weapon_check_collision( object * obj, object * other_obj, vec3d * hitpos )
// {
//    this should check if a vector from 
//		other_obj->last_pos to other_obj->pos with a radius of other_obj->radius
//    collides with object obj.   If it does, then fill in hitpos with the point
//    of impact and return non-zero, otherwise return 0 if no impact.   Note that
//    this shouldn't take any action... that happens in weapon_hit.
// }

// 
// void weapon_hit( object * obj, object * other_obj, vec3d * hitpos )
// {
//    {Put a call to this in COLLIDE.C}
//    ... Do what needs to be done when this object gets hit
//    ... Reducing shields, etc
// }

//Misc object flags
#define OF_RENDERS					(1<<0)	// It renders as something ( objtype_render gets called)
#define OF_COLLIDES					(1<<1)	// It collides with stuff (objtype_check_impact & objtype_hit gets called)
#define OF_PHYSICS					(1<<2)	// It moves with standard physics.
#define OF_SHOULD_BE_DEAD			(1<<3)	// this object should be dead, so next time we can, we should delete this object.
#define OF_INVULNERABLE				(1<<4)	// invulnerable
#define OF_PROTECTED				(1<<5)	// Don't kill this object, probably mission-critical.
#define OF_PLAYER_SHIP				(1<<6)	// this object under control of some player -- don't do ai stuff on it!!!
#define OF_NO_SHIELDS				(1<<7)	// object has no shield generator system (i.e. no shields)
#define OF_JUST_UPDATED				(1<<8)	// for multiplayer -- indicates that we received object update this frame
#define OF_COULD_BE_PLAYER			(1<<9)	// for multiplayer -- indicates that it is selectable ingame joiners as their ship
#define OF_WAS_RENDERED				(1<<10)	// Set if this object was rendered this frame.  Only gets set if OF_RENDERS set.  Gets cleared or set in obj_render_all().
#define OF_NOT_IN_COLL				(1<<11)	// object has not been added to collision list
#define OF_BEAM_PROTECTED			(1<<12)	// don't fire beam weapons at this type of object, probably mission critical.
#define OF_SPECIAL_WARP				(1<<13)	// Object has special warp-in enabled.
#define OF_DOCKED_ALREADY_HANDLED	(1<<14)	// Goober5000 - a docked object that we already moved

// Flags used by Fred
#define OF_MARKED			(1<<17)	// Object is marked (Fred).  Can be reused in Freespace for anything that won't be used by Fred.
#define OF_TEMP_MARKED		(1<<18)	// Temporarily marked (Fred).
#define OF_REFERENCED		(1<<19)	// (Fred) Object is referenced by something somewhere
#define OF_HIDDEN			(1<<20)	// Object is hidden (not shown) and can't be manipulated


// max # of object sounds per object
//WMC - bumped this to 32 :D
#define MAX_OBJECT_SOUNDS	32

struct dock_instance;

typedef struct object {
	struct object	*next, *prev;	// for linked lists of objects
	int				signature;		// Every object ever has a unique signature...
	char				type;				// what type of object this is... robot, weapon, hostage, powerup, fireball
	int				parent;			// This object's parent.
	int				parent_sig;		// This object's parent's signature
	char				parent_type;	// This object's parent's type
	int				instance;		// which instance.  ie.. if type is Robot, then this indexes into the Robots array
	uint				flags;			// misc flags.  Call obj_set_flags to change this.
	vec3d			pos;				// absolute x,y,z coordinate of center of object
	matrix			orient;			// orientation of object in world
	float				radius;			// 3d size of object - for collision detection
	vec3d			last_pos;		// where object was last frame
	matrix			last_orient;	// how the object was oriented last frame
	physics_info	phys_info;		// a physics object
	float				shield_quadrant[MAX_SHIELD_SECTIONS];	//	Shield is broken into components.  Quadrants on 4/24/97.
	float				hull_strength;	//	Remaining hull strength.
	short				objsnd_num[MAX_OBJECT_SOUNDS];		// Index of persistant sound struct.  -1 if no persistant sound assigned.
	ushort			net_signature;
	int				num_pairs;		// How many object pairs this is associated with.  When 0 then there are no more.

	union {
		class jump_node *jnp;		// WMC - Direct pointer to the object. Used only for jump nodes as of now
	};
	dock_instance	*dock_list;		// Goober5000 - objects this object is docked to
} object;

// object backup struct used by Fred.
typedef struct object_orient_pos {
	vec3d pos;
	matrix orient;
} object_orient_pos;

/*
 *		VARIABLES
 */

extern int Object_inited;
extern int Show_waypoints;

// The next signature for the next newly created object. Zero is bogus
#define OBJECT_SIG_SHIP_START					300000;				// ships start at this signature
extern int Object_next_ship_signature;
extern int Object_next_signature;		
extern int num_objects;

extern object Objects[];
extern int Highest_object_index;		//highest objnum
extern int Highest_ever_object_index;
extern object obj_free_list;
extern object obj_used_list;
extern object obj_create_list;

extern int render_total;
extern int render_order[MAX_OBJECTS];

extern object *Viewer_obj;	// Which object is the viewer. Can be NULL.
extern object *Player_obj;	// Which object is the player. Has to be valid.

// Use this instead of "objp - Objects" to get an object number
// given it's pointer.  This way, we can replace it will a macro
// to check that the pointer is valid for debugging.
#define OBJ_INDEX(objp) (objp-Objects)

/*
 *		FUNCTIONS
 */

//do whatever setup needs to be done
void obj_init();

//initialize a new object.  adds to the list for the given segment.
//returns the object number.  The object will be a non-rendering, non-physics
//object.  Returns 0 if failed, otherwise object index.
//You can pass 0 for parent if you don't care about that.
//You can pass null for orient and/or pos if you don't care.
int obj_create(ubyte type,int parent_obj, int instance, matrix * orient, vec3d * pos, float radius, uint flags );

//Render an object.  Calls one of several routines based on type
void obj_render(object *obj);

//Sorts and renders all the ojbects
void obj_render_all(void (*render_function)(object *objp), bool* render_viewer_last );

//move all objects for the current frame
void obj_move_all(float frametime);		// moves all objects

//move an object for the current frame
void obj_move_one(object * obj, float frametime);

// function to delete an object -- should probably only be called directly from editor code
void obj_delete(int objnum);

// should only be used by the editor!
void obj_merge_created_list(void);

// recalculate object pairs for an object
#define OBJ_RECALC_PAIRS(obj_to_reset)		do {	obj_set_flags(obj_to_reset, obj_to_reset->flags & ~(OF_COLLIDES)); obj_set_flags(obj_to_reset, obj_to_reset->flags | OF_COLLIDES); } while(0);

// Removes any occurances of object 'a' from the pairs list.
void obj_remove_pairs( object * a );

// add an object to the pairs list
void obj_add_pairs(int objnum);

//	Returns true if objects A and B are expected to collide in next duration seconds.
//	For purposes of this check, the first object moves from current location to predicted
//	location.  The second object is assumed to be where it will be at time duration, NOT
//	where it currently is.
//	radius_scale: 0.0f means use polygon models, else scale sphere size by radius_scale
//	radius_scale == 1.0f means Descent style collisions.
int objects_will_collide(object *A, object *B, float duration, float radius_scale);

// Used for pausing.  Seems hacked.  Was in PHYSICS, but that broke the TestCode program,
// so I moved it into the object lib.  -John
void obj_init_all_ships_physics();

float get_shield_strength(object *objp);
void set_shield_strength(object *objp, float strength);
void add_shield_strength(object *objp, float delta);

// Goober5000
float get_hull_pct(object *objp);
float get_shield_pct(object *objp);
float get_max_shield_quad(object *objp);

// returns the average 3-space position of all ships.  useful to find "center" of battle (sort of)
void obj_get_average_ship_pos(vec3d *pos);

// function to deal with firing player things like lasers, missiles, etc.
// separated out because of multiplayer issues.
void obj_player_fire_stuff( object *objp, control_info ci );

// Call this if you want to change an object flag so that the
// object code knows what's going on.  For instance if you turn
// off OF_COLLIDES, the object code needs to know this in order to
// actually turn the object collision detection off.  By calling
// this you shouldn't get Int3's in the checkobject code.  If you
// do, then put code in here to correctly handle the case.
void obj_set_flags(object *obj, uint new_flags);

// get the Ship_info flags for a given ship (if you have the object)
int obj_get_SIF(object *objp);
int obj_get_SIF(int obj);

// get the team for any object
int obj_team(object *objp);

void obj_move_all_pre(object *objp, float frametime);
void obj_move_all_post(object *objp, float frametime);

void obj_move_call_physics(object *objp, float frametime);

// multiplayer object update stuff begins -------------------------------------------

// do client-side pre-interpolation object movement
void obj_client_pre_interpolate();

// do client-side post-interpolation object movement
void obj_client_post_interpolate();

// move an observer object in multiplayer
void obj_observer_move(float frame_time);

// Goober5000
int object_is_docked(object *objp);
void obj_move_one_docked_object(object *objp, object *parent_objp);


#endif
