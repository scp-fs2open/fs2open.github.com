/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _OBJECT_H
#define _OBJECT_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "math/vecmat.h"
#include "object/object_flags.h"
#include "physics/physics.h"
#include "physics/physics_state.h"
#include "io/timer.h"					// prevents some include issues with files in the actions folder
#include "utils/event.h"

#include <functional>

/*
 *		CONSTANTS
 */

#define DEFAULT_SHIELD_SECTIONS	4	//	Number of sections in standard shields.

#ifndef NDEBUG
#define OBJECT_CHECK 
#endif

//Object types
#define OBJ_NONE            0	//unused object
#define OBJ_SHIP            1	//a ship
#define OBJ_WEAPON          2	//a laser, missile, etc
#define OBJ_FIREBALL        3	//an explosion
#define OBJ_START           4	//a starting point marker (player start, etc)
#define OBJ_WAYPOINT        5	//a waypoint object, maybe only ever used by Fred
#define OBJ_DEBRIS          6	//a flying piece of ship debris
//#define OBJ_CMEASURE      7	//a countermeasure, such as chaff
#define OBJ_GHOST           8	//so far, just a placeholder for when a player dies.
#define OBJ_POINT           9	//generic object type to display a point in Fred.
#define OBJ_SHOCKWAVE       10	// a shockwave
#define OBJ_WING            11	// not really a type used anywhere, but I need it for Fred.
#define OBJ_OBSERVER        12	// used for multiplayer observers (possibly single player later)
#define OBJ_ASTEROID        13	//	An asteroid, you know, a big rock, like debris, sort of.
#define OBJ_JUMP_NODE       14	// A jump node object, used only in Fred.
#define OBJ_BEAM            15	// beam weapons. we have to roll them into the object system to get the benefits of the collision pairs
#define OBJ_RAW_POF         16	// A raw pof file. has no physics, ai or anything. Currently only used in the Lab to render tech models

//Make sure to change Object_type_names in Object.c when adding another type!
#define MAX_OBJECT_TYPES	17

#define UNUSED_OBJNUM		(-MAX_OBJECTS*2)	//	Newer systems use this instead of -1 for invalid object.

extern const char	*Object_type_names[MAX_OBJECT_TYPES];

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
//    Assert( obj->flags[Object::Object_Flags::Should_be_dead] );
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

typedef struct obj_flag_name {
	Object::Object_Flags flag;
	char flag_name[TOKEN_LENGTH];
} obj_flag_name;

typedef struct obj_flag_description {
	Object::Object_Flags flag;
	const char *flag_desc;
} obj_flag_description;

extern obj_flag_name Object_flag_names[];
extern obj_flag_description Object_flag_descriptions[];
extern const int Num_object_flag_names;

struct dock_instance;
class model_draw_list;
class polymodel;
struct polymodel_instance;

typedef struct raw_pof_obj {
	  int                            model_num;      // The model number of the loaded POF
	  int                            model_instance; // The model instance
	  flagset<Object::Raw_Pof_Flags> flags;          // Render flags
} raw_pof_obj;

extern SCP_map<int, raw_pof_obj> Pof_objects;

class object
{
public:
	class object	*next, *prev;	// for linked lists of objects
	int				signature;		// Every object ever has a unique signature...
	char			type;			// what type of object this is... ship, weapon, debris, asteroid, fireball, see OBJ_* defines above
	int				parent;			// This object's parent.
	int				parent_sig;		// This object's parent's signature
	int				instance;		// index into the corresponding type array, i.e. if type == OBJ_SHIP then instance indexes the Ships array
	flagset<Object::Object_Flags> flags;			// misc flags.  Call obj_set_flags to change this.
	vec3d			pos;				// absolute x,y,z coordinate of center of object
	matrix			orient;			// orientation of object in world
	float			radius;			// 3d size of object - for collision detection
	vec3d			last_pos;		// where object was last frame
	matrix			last_orient;	// how the object was oriented last frame
	physics_info	phys_info;		// a physics object
	SCP_vector<float>	shield_quadrant;	//	Shield is broken into components, quadrants by default.
	float			hull_strength;	//	Remaining hull strength.
	float			sim_hull_strength;	// Simulated hull strength - used with training weapons.
	SCP_vector<int> objsnd_num;		// Index of persistant sound struct.
	ushort			net_signature;
	int				num_pairs;		// How many object pairs this is associated with.  When 0 then there are no more.

	dock_instance	*dock_list;			// Goober5000 - objects this object is docked to
	dock_instance	*dead_dock_list;	// Goober5000 - objects this object was docked to when destroyed; replaces dock_objnum_when_dead

	int				collision_group_id; // This is a bitfield. Collision checks will be skipped if A->collision_group_id & B->collision_group_id returns nonzero

	util::event<void, object*> pre_move_event;
	util::event<void, object*> post_move_event;

	void clear();


	object();
	~object();

	// An object should never be copied; there are allocated pointers, and linked list shenanigans.
	object(const object& other) = delete;
	object& operator=(const object& other) = delete;

	// Eventually we want to allow an object to be moved, especially when we get around to making Objects[] dynamic
	object(object&& other) noexcept = delete;
	object& operator=(object&& other) noexcept = delete;
};

struct lua_State;
namespace scripting {
	class ade_table_entry;
}
namespace luacpp {
	class LuaValue;
}

extern int Num_objects;
extern object Objects[];

struct object_h final	// prevent subclassing because classes which might use this should have their own isValid member function
{
	int objnum = -1;
	int sig = -1;

	object_h(const object* in_objp);
	object_h(int in_objnum);
	object_h();

	bool isValid() const;
	object* objp() const;
	object* objp_or_null() const;
};

// object backup struct used by Fred.
typedef struct object_orient_pos {
	vec3d pos;
	matrix orient;
} object_orient_pos;

#ifdef OBJECT_CHECK
class checkobject
{
public:
	int	type;
	int	signature;
	flagset<Object::Object_Flags>	flags;
	int	parent_sig;

    checkobject();
};
#endif

/*
 *		VARIABLES
 */

extern int Object_inited;
extern int Show_waypoints;

extern int Object_next_signature;		// The next signature for the next newly created object. Zero is bogus
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
// given it's pointer.  This way, we can replace it with a macro
// to check that the pointer is valid for debugging.
// This code will break in 64 bit builds when we have more than 2^31 objects but that will probably never happen
#define OBJ_INDEX(objp) static_cast<int>(objp-Objects)

/*
 *		FUNCTIONS
 */

//do whatever setup needs to be done
void obj_init();

void obj_shutdown();

//initialize a new object.  adds to the list for the given segment.
//returns the object number.  The object will be a non-rendering, non-physics
//object.  Returns 0 if failed, otherwise object index.
//You can pass 0 for parent if you don't care about that.
//You can pass null for orient and/or pos if you don't care.
int obj_create(ubyte type, int parent_obj, int instance, const matrix *orient, const vec3d *pos, float radius, const flagset<Object::Object_Flags> &flags, bool essential = true);

void obj_render(object* obj);

void obj_queue_render(object* obj, model_draw_list* scene);

//Sorts and renders all the ojbects
void obj_render_all(const std::function<void(object*)>& render_function, bool* render_viewer_last );

//move all objects for the current frame
void obj_move_all(float frametime);		// moves all objects

// function to delete an object -- should probably only be called directly from editor code
void obj_delete(int objnum);

void obj_delete_all();

void obj_delete_all_that_should_be_dead();

// should only be used by the editor!
void obj_merge_created_list(void);

// recalculate object pairs for an object
#define OBJ_RECALC_PAIRS(obj_to_reset)		do {	obj_set_flags(obj_to_reset, obj_to_reset->flags - Object::Object_Flags::Collides); obj_set_flags(obj_to_reset, obj_to_reset->flags + Object::Object_Flags::Collides); } while(false);

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

// Goober5000
float get_hull_pct(const object *objp, bool allow_negative = false);
float get_sim_hull_pct(const object *objp, bool allow_negative = false);
float get_shield_pct(const object *objp);

struct ship_registry_entry;

// SEXPs evaluated during debriefing are susceptible to a "use-after-free" problem where the object and ship have been deleted but still
// contain information useful for debriefing.  Since the ship registry still contains object and ship indexes, use this rather than the
// instance and objnum indexes in the object and ship structures themselves.
float get_hull_pct(const ship_registry_entry *ship_entry, bool allow_negative = false);
float get_sim_hull_pct(const ship_registry_entry *ship_entry, bool allow_negative = false);
float get_shield_pct(const ship_registry_entry *ship_entry);

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
void obj_set_flags(object *obj, const flagset<Object::Object_Flags>& new_flags);

// get the team for any object
int obj_team(object *objp);

void obj_move_all_pre(object *objp, float frametime);
void obj_move_all_post(object *objp, float frametime);

void obj_move_call_physics(object *objp, float frametime);

// move an observer object in multiplayer
void obj_observer_move(float frame_time);

/**
 * @brief Checks if the given object is docked with anyone.
 *
 * @returns Nonzero if docked, or
 * @returns 0 if not docked
 *
 * @author Goober5000
 */
int object_is_docked(object *objp);

/**
 * @brief Checks if the given object is dead-docked with anyone.
 *
 * @returns Nonzero if docked, or
 * @returns 0 if not docked
 *
 * @details An object is "dead-docked" when it is dying and still has objects docked to it. The dead_dock list is
 *   populated when the object dies, and is used later on to jettison and maybe damage the docked objects.
 *
 * @author Goober5000
 */
int object_is_dead_docked(object *objp);

/**
 * @brief Moves a docked object to keep up with the parent object as it moves
 *
 * @param[in,out] objp The docked object
 * @param[in]     parent_objp The object that it's docked to
 *
 * @author Goober5000
 */
void obj_move_one_docked_object(object *objp, object *parent_objp);

//WMC
void object_set_gliding(object *objp, bool enable=true, bool force = false);
bool object_get_gliding(object *objp);
bool object_glide_forced(object* objp);
int obj_get_by_signature(int sig);

int object_get_model_num(const object *objp);
polymodel *object_get_model(const object *objp);
int object_get_model_instance_num(const object *objp);
polymodel_instance *object_get_model_instance(const object *objp);

void obj_render_queue_all();

/**
 * @brief Compares two object pointers and determines if they refer to the same object
 *
 * @note Two @c nullptr parameters are considered equal
 *
 * @param left The first object pointer, may be @c nullptr
 * @param right The second object pointer, may be @c nullptr
 * @return @c true if the two pointers refer to the same object
 */
bool obj_compare(object *left, object *right);

////////////////////////////////////////////////////////////
// physics_state api functions that require the object type

/**
 * @brief Populate a physics snapshot directly from the info in an object
 *
 * @param[in,out] snapshot Destination physics snapshot
 * @param[in]     objp The object pointer that we are pulling information from
 *
 * @author J Fernandez
 */
void physics_populate_snapshot(physics_snapshot& snapshot, const object* objp);

/**
 * @brief Change the object's physics info to match the info contained in a snapshot.
 *
 * @param[in,out] objp Destination object pointer
 * @param[in]     source The physics snapshot we are pulling information from
 *
 * @details To be used when interpolating or restoring a game state.
 * 
 * @author J Fernandez
 */
void physics_apply_pstate_to_object(object* objp, const physics_snapshot& source);

/**
 *@brief create a raw pof instance
 *
 * @author Mike Nelson
 */
int obj_raw_pof_create(const char* pof_filename, const matrix* orient, const vec3d* pos);


#endif
