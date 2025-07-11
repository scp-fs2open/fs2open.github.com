/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "asteroid/asteroid.h"
#include "cmeasure/cmeasure.h"
#include "debris/debris.h"
#include "debugconsole/console.h"
#include "fireball/fireballs.h"
#include "freespace.h"
#include "globalincs/linklist.h"
#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "iff_defs/iff_defs.h"
#include "io/timer.h"
#include "jumpnode/jumpnode.h"
#include "lighting/lighting.h"
#include "lighting/lighting_profiles.h"
#include "mission/missionparse.h" //For 2D Mode
#include "network/multi.h"
#include "network/multiutil.h"
#include "network//multi_obj.h"
#include "object/deadobjectdock.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "object/objectdock.h"
#include "object/objectshield.h"
#include "object/objectsnd.h"
#include "observer/observer.h"
#include "scripting/global_hooks.h"
#include "scripting/api/libs/graphics.h"
#include "scripting/scripting.h"
#include "playerman/player.h"
#include "radar/radar.h"
#include "radar/radarsetup.h"
#include "render/3d.h"
#include "ship/afterburner.h"
#include "ship/ship.h"
#include "starfield/starfield.h"
#include "tracing/tracing.h"
#include "weapon/beam.h"
#include "weapon/shockwave.h"
#include "weapon/swarm.h"
#include "weapon/weapon.h"
#include "tracing/Monitor.h"
#include "graphics/light.h"
#include "graphics/color.h"
#include "math/curve.h"

extern void ship_reset_disabled_physics(object *objp, int ship_class);

/*
 *  Global variables
 */

object obj_free_list;
object obj_used_list;
object obj_create_list;	

object *Player_obj = NULL;
object *Viewer_obj = NULL;

//Data for objects
object Objects[MAX_OBJECTS];
SCP_map<int, raw_pof_obj> Pof_objects;

#ifdef OBJECT_CHECK 
checkobject CheckObjects[MAX_OBJECTS];
#endif

int Num_objects=-1;
int Highest_object_index=-1;
int Highest_ever_object_index=0;
int Object_next_signature = 1;	//0 is bogus, start at 1
int Object_inited = 0;
int Show_waypoints = 0;

object_h::object_h(int in_objnum)
	: objnum(in_objnum)
{
	if (objnum >= 0 && objnum < MAX_OBJECTS)
		sig = Objects[objnum].signature;
	else
		objnum = -1;
}

object_h::object_h(const object* in_objp)
{
	if (in_objp)
	{
		objnum = OBJ_INDEX(in_objp);
		sig = in_objp->signature;
	}
}

object_h::object_h()
{}

bool object_h::isValid() const
{
	// a signature of 0 is invalid, per obj_init()
	if (objnum < 0 || sig <= 0 || objnum >= MAX_OBJECTS)
		return false;
	return Objects[objnum].signature == sig;
}

object* object_h::objp() const
{
	return &Objects[objnum];
}

object* object_h::objp_or_null() const
{
	return isValid() ? &Objects[objnum] : nullptr;
}

//WMC - Made these prettier
const char *Object_type_names[MAX_OBJECT_TYPES] = {
//XSTR:OFF
	"None",
	"Ship",
	"Weapon",
	"Fireball",
	"Start",
	"Waypoint",
	"Debris",
	"Countermeasure",
	"Ghost",
	"Point",
	"Shockwave",
	"Wing",
	"Observer",
	"Asteroid",
	"Jump Node",
	"Beam",
	"Raw Pof"
//XSTR:ON
};

obj_flag_name Object_flag_names[] = {
    { Object::Object_Flags::Invulnerable,			"invulnerable",						},
	{ Object::Object_Flags::Protected,				"protect-ship",						},
	{ Object::Object_Flags::Beam_protected,			"beam-protect-ship",				},
	{ Object::Object_Flags::No_shields,				"no-shields",						},
	{ Object::Object_Flags::Targetable_as_bomb,		"targetable-as-bomb",				},
	{ Object::Object_Flags::Flak_protected,			"flak-protect-ship",				},
	{ Object::Object_Flags::Laser_protected,		"laser-protect-ship",				},
	{ Object::Object_Flags::Missile_protected,		"missile-protect-ship",				},
	{ Object::Object_Flags::Immobile,				"immobile",							},
	{ Object::Object_Flags::Dont_change_position,	"don't-change-position",			},
	{ Object::Object_Flags::Dont_change_orientation,	"don't-change-orientation",		},
	{ Object::Object_Flags::Collides,				"collides",							},
	{ Object::Object_Flags::Attackable_if_no_collide, "ai-attackable-if-no-collide",	},
};

obj_flag_description Object_flag_descriptions[] = {
    { Object::Object_Flags::Invulnerable,				"Stops this object from taking any damage."},
	{ Object::Object_Flags::Protected,					"Ship and Turret AI will ignore and not attack this object."},
	{ Object::Object_Flags::Beam_protected,				"Turrets with beam weapons will ignore and not attack this object."},
	{ Object::Object_Flags::No_shields,					"This object will have no shields.  (If this object can otherwise have shields, its shield energy will be fully reallocated to other ETS systems.)"},
	{ Object::Object_Flags::Targetable_as_bomb,			"Allows this object to be targeted with the bomb targeting key."},
	{ Object::Object_Flags::Flak_protected,				"Turrets with flak weapons will ignore and not attack this object."},
	{ Object::Object_Flags::Laser_protected,			"Turrets with laser weapons will ignore and not attack this object."},
	{ Object::Object_Flags::Missile_protected,			"Turrets with missile weapons will ignore and not attack this object."},
	{ Object::Object_Flags::Immobile,					"Will not let an object change position or orientation.  Upon destruction it will still do the death roll and explosion."},
	{ Object::Object_Flags::Dont_change_position,		"Will not let an object change position.  Upon destruction it will still do the death roll and explosion."},
	{ Object::Object_Flags::Dont_change_orientation,	"Will not let an object change orientation.  Upon destruction it will still do the death roll and explosion."},
	{ Object::Object_Flags::Collides,					"This object will collide with other objects."},
	{ Object::Object_Flags::Attackable_if_no_collide,	"Allows the AI to attack this object, even if no-collide is set.  (Normally an object that does not collide is also not attacked.)"},
};

extern const int Num_object_flag_names = sizeof(Object_flag_names) / sizeof(obj_flag_name);

#ifdef OBJECT_CHECK
checkobject::checkobject() 
    : type(0), signature(0), parent_sig(0) 
{
    flags.reset();
}
#endif

// all we need to set are the pointers, but type, parent, and instance are useful to set as well
object::object()
	: next(nullptr), prev(nullptr), signature(0), type(OBJ_NONE), parent(-1), parent_sig(0), instance(-1), pos(vmd_zero_vector), orient(vmd_identity_matrix),
	radius(0.0f), last_pos(vmd_zero_vector), last_orient(vmd_identity_matrix), hull_strength(0.0f), sim_hull_strength(0.0f), net_signature(0), num_pairs(0),
	dock_list(nullptr), dead_dock_list(nullptr), collision_group_id(0)
{
	memset(&(this->phys_info), 0, sizeof(physics_info));
}

object::~object()
{
	dock_free_dock_list(this);
	dock_free_dead_dock_list(this);
}

// DO NOT set next and prev to NULL because they keep the object on the free and used lists
void object::clear()
{
	signature = num_pairs = collision_group_id = 0;
	parent = parent_sig = instance = -1;
	type = OBJ_NONE;
    flags.reset();
	pos = last_pos = vmd_zero_vector;
	orient = last_orient = vmd_identity_matrix;
	radius = hull_strength = sim_hull_strength = 0.0f;
	physics_init( &phys_info );
	shield_quadrant.clear();
	objsnd_num.clear();
	net_signature = 0;

	pre_move_event.clear();
	post_move_event.clear();

	// just in case nobody called obj_delete last mission
	dock_free_dock_list(this);
	dock_free_dead_dock_list(this);
}

/**
 * Scan the object list, freeing down to num_used objects
 *
 * @param  target_num_used Number of used objects to free down to
 * @return Returns number of slots freed
 */
int free_object_slots(int target_num_used)
{
	int	i, olind, deleted_weapons;
	int	obj_list[MAX_OBJECTS];
	int	num_already_free, num_to_free, original_num_to_free;
	object *objp;

	olind = 0;

	// calc num_already_free by walking the obj_free_list
	num_already_free = 0;
	for ( objp = GET_FIRST(&obj_free_list); objp != END_OF_LIST(&obj_free_list); objp = GET_NEXT(objp) )
		num_already_free++;

	if (MAX_OBJECTS - num_already_free < target_num_used)
		return 0;

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->flags[Object::Object_Flags::Should_be_dead]) {
			num_already_free++;
			if (MAX_OBJECTS - num_already_free < target_num_used)
				return num_already_free;
		} else
			switch (objp->type) {
				case OBJ_NONE:
					num_already_free++;
					if (MAX_OBJECTS - num_already_free < target_num_used)
						return 0;
					break;
				case OBJ_FIREBALL:
				case OBJ_WEAPON:
				case OBJ_DEBRIS:
//				case OBJ_CMEASURE:
					obj_list[olind++] = OBJ_INDEX(objp);
					break;

				case OBJ_GHOST:
				case OBJ_SHIP:
				case OBJ_START:
				case OBJ_WAYPOINT:
				case OBJ_POINT:
				case OBJ_SHOCKWAVE:
				case OBJ_WING:
				case OBJ_OBSERVER:
				case OBJ_ASTEROID:
				case OBJ_JUMP_NODE:				
				case OBJ_BEAM:
				case OBJ_RAW_POF:
					break;
				default:
					Int3();	//	Hey, what kind of object is this?  Unknown!
					break;
			}

	}

	num_to_free = MAX_OBJECTS - target_num_used - num_already_free;
	original_num_to_free = num_to_free;

	if (num_to_free > olind) {
		nprintf(("allender", "Warning: Asked to free %i objects, but can only free %i.\n", num_to_free, olind));
		num_to_free = olind;
	}

	for (i=0; i<num_to_free; i++)
		if ( (Objects[obj_list[i]].type == OBJ_DEBRIS) && (!Debris[Objects[obj_list[i]].instance].flags[Debris_Flags::DoNotExpire]) ) {
			num_to_free--;
			nprintf(("allender", "Freeing   DEBRIS object %3i\n", obj_list[i]));
			Objects[obj_list[i]].flags.set(Object::Object_Flags::Should_be_dead);
		}

	if (num_to_free <= 0) {
		return original_num_to_free;
	}

	for (i=0; i<num_to_free; i++)	{
		object *tmp_obj = &Objects[obj_list[i]];
		if ( (tmp_obj->type == OBJ_FIREBALL) && (fireball_is_perishable(tmp_obj)) ) {
			num_to_free--;
			if (num_to_free <= 0) {
				return original_num_to_free;
			}
			nprintf(("allender", "Freeing FIREBALL object %3i\n", obj_list[i]));
			tmp_obj->flags.set(Object::Object_Flags::Should_be_dead);
		}
	}

	deleted_weapons = collide_remove_weapons();

	num_to_free -= deleted_weapons;
	if ( num_to_free <= 0){
		return original_num_to_free;
	}

	for (i=0; i<num_to_free; i++){
		if ( Objects[obj_list[i]].type == OBJ_WEAPON ) {
			num_to_free--;
			Objects[obj_list[i]].flags.set(Object::Object_Flags::Should_be_dead);
		}
	}

	if (!num_to_free){
		return original_num_to_free;
	}

	return original_num_to_free - num_to_free;
}

// Goober5000
// This helper function does not check the object type and is not intended as a public API.
float get_hull_or_sim_hull_pct_helper(const object *objp, const float object::*strength_field, const ship *shipp, bool allow_negative)
{
	Assert(objp && shipp);
	if (!objp || !shipp)
		return 0.0f;

	float total_strength = shipp->ship_max_hull_strength;
	Assert(total_strength > 0.0f);	// unlike shield, no ship can have 0 hull
	if (total_strength == 0.0f)
		return 0.0f;

	if (!allow_negative && objp->*strength_field < 0.0f)	// this sometimes happens when a ship is being destroyed
		return 0.0f;

	return objp->*strength_field / total_strength;
}

float get_hull_pct(const object *objp, bool allow_negative)
{
	Assert(objp && objp->type == OBJ_SHIP);
	if (!objp || objp->type != OBJ_SHIP)
		return 0.0f;
	return get_hull_or_sim_hull_pct_helper(objp, &object::hull_strength, &Ships[objp->instance], allow_negative);
}

float get_hull_pct(const ship_registry_entry *ship_entry, bool allow_negative)
{
	return get_hull_or_sim_hull_pct_helper(ship_entry->objp(), &object::hull_strength, ship_entry->shipp(), allow_negative);
}

float get_sim_hull_pct(const object *objp, bool allow_negative)
{
	Assert(objp && objp->type == OBJ_SHIP);
	if (!objp || objp->type != OBJ_SHIP)
		return 0.0f;
	return get_hull_or_sim_hull_pct_helper(objp, &object::sim_hull_strength, &Ships[objp->instance], allow_negative);
}

float get_sim_hull_pct(const ship_registry_entry *ship_entry, bool allow_negative)
{
	return get_hull_or_sim_hull_pct_helper(ship_entry->objp(), &object::sim_hull_strength, ship_entry->shipp(), allow_negative);
}

// Goober5000
// This helper function does not check the object type and is not intended as a public API.
float get_shield_pct_helper(const object *objp, const ship *shipp)
{
	Assert(objp && shipp);
	if (!objp || !shipp)
		return 0.0f;

	float total_strength = shield_get_max_strength(shipp);
	if (total_strength == 0.0f)
		return 0.0f;

	return shield_get_strength(objp) / total_strength;
}

float get_shield_pct(const object *objp)
{
	Assert(objp);
	if (!objp)
		return 0.0f;

	// bah - we might have asteroids
	if (objp->type != OBJ_SHIP)
		return 0.0f;

	return get_shield_pct_helper(objp, &Ships[objp->instance]);
}

float get_shield_pct(const ship_registry_entry *ship_entry)
{
	return get_shield_pct_helper(ship_entry->objp(), ship_entry->shipp());
}

static void on_script_state_destroy(lua_State*) {
	// Since events are mostly used for scripting, we clear the event handlers when the Lua state is destroyed
	for (auto& obj : Objects) {
		obj.pre_move_event.clear();
		obj.post_move_event.clear();
	}
}

/**
 * Sets up the free list & init player & whatever else
 */
void obj_init()
{
	int i;
	object *objp;
	
	Object_inited = 1;
	for (i = 0; i < MAX_OBJECTS; ++i)
		Objects[i].clear();
	Viewer_obj = NULL;

	list_init( &obj_free_list );
	list_init( &obj_used_list );
	list_init( &obj_create_list );

	// Link all object slots into the free list
	objp = Objects;
	for (i=0; i<MAX_OBJECTS; i++)	{
		list_append(&obj_free_list, objp);
		objp++;
	}

	Object_next_signature = 1;	//0 is invalid, others start at 1
	Num_objects = 0;
	Highest_object_index = 0;

	obj_reset_colliders();

	Script_system.OnStateDestroy.add(on_script_state_destroy);
}

void obj_shutdown()
{
	for (auto& obj : Objects) {
		obj.clear();
	}
}

static int num_objects_hwm = 0;

/** 
 * Allocates an object
 *
 * Generally, obj_create() should be called to get an object, since it
 * fills in important fields and does the linking.
 *
 * @return the number of a free object, updating Highest_object_index
 * @return -1 if no free objects
 */
int obj_allocate(bool essential)
{
	int objnum;
	object *objp;

	if (!Object_inited) {
		mprintf(("Why hasn't obj_init() been called yet?\n"));
		obj_init();
	}

	if ( (Num_objects >= MAX_OBJECTS-10) && essential ) {
		int	num_freed;

		num_freed = free_object_slots(MAX_OBJECTS-10);
		nprintf(("warning", " *** Freed %i objects\n", num_freed));
	}

	if (Num_objects >= MAX_OBJECTS) {
		mprintf(("Object creation failed - too many objects!\n" ));
		return -1;
	}

	// Find next available object
	objp = GET_FIRST(&obj_free_list);
	Assert ( objp != &obj_free_list );		// shouldn't have the dummy element

	// remove objp from the free list
	list_remove( &obj_free_list, objp );
	
	// insert objp onto the end of create list
	list_append( &obj_create_list, objp );

	// increment counter
	Num_objects++;

	if (Num_objects > num_objects_hwm) {
		num_objects_hwm = Num_objects;
	}

	// get objnum
	objnum = OBJ_INDEX(objp);

	if (objnum > Highest_object_index) {
		Highest_object_index = objnum;
		if (Highest_object_index > Highest_ever_object_index)
			Highest_ever_object_index = Highest_object_index;
	}

	return objnum;
}

/**
 * Frees up an object  
 *
 * Generally, obj_delete() should be called to get rid of an object.
 * This function deallocates the object entry after the object has been unlinked
 */
void obj_free(int objnum)
{
	object *objp;

	if (!Object_inited) {
		mprintf(("Why hasn't obj_init() been called yet?\n"));
		obj_init();
	}

	Assert( objnum >= 0 );	// Trying to free bogus object!!!

	// get object pointer
	objp = &Objects[objnum];

	// remove objp from the used list
	list_remove( &obj_used_list, objp );

	// add objp to the end of the free
	list_append( &obj_free_list, objp );

	// decrement counter
	Num_objects--;

	Objects[objnum].type = OBJ_NONE;

	Assert(Num_objects >= 0);

	if (objnum == Highest_object_index) {
		while (Highest_object_index >= 0 && Objects[Highest_object_index].type == OBJ_NONE) {
			--Highest_object_index;
		}
	}
}

/**
 * Create a raw pof item
 * @return the objnum of this raw POF
 */
int obj_raw_pof_create(const char* pof_filename, const matrix* orient, const vec3d* pos)
{
	static int next_raw_pof_id = 0;

	// Unlikely this would ever be hit.. but just in case
	if (next_raw_pof_id >= INT_MAX || next_raw_pof_id < 0) {
		Error(LOCATION, "Too many RAW_POF objects created!");
		return -1;
	}

	if (!VALID_FNAME(pof_filename)) {
		Warning(LOCATION, "Invalid tech model POF: %s", pof_filename);
		return -1;
	}

	int model_num = model_load(pof_filename);
	if (model_num < 0) {
		Warning(LOCATION, "Failed to load tech model: %s", pof_filename);
		return -1;
	}

	int id = next_raw_pof_id++;
	Pof_objects[id] = {model_num, -1, {}};

	flagset<Object::Object_Flags> flags;
	flags.set(Object::Object_Flags::Renders);

	int objnum = obj_create(OBJ_RAW_POF, -1, id, orient, pos, model_get_radius(model_num), flags);

	Pof_objects[id].model_instance = model_create_instance(objnum, model_num);

	return objnum;
}

/**
 * Initialize a new object. Adds to the list for the given segment.
 *
 * The object will be a non-rendering, non-physics object.   Pass -1 if no parent.
 * @return the object number 
 */
int obj_create(ubyte type, int parent_obj, int instance, const matrix *orient,
               const vec3d *pos, float radius, const flagset<Object::Object_Flags> &flags, bool essential)
{
	int objnum;
	object *obj;

	// Find next free object
	objnum = obj_allocate(essential);

	if (objnum == -1)		//no free objects
		return -1;

	obj = &Objects[objnum];
	Assert(obj->type == OBJ_NONE);		//make sure unused 

	// clear object in preparation for setting of custom values
	obj->clear();

	Assert(Object_next_signature > 0);	// 0 is bogus!
	obj->signature = Object_next_signature++;

	obj->type 					= type;
	obj->instance				= instance;
	obj->parent					= parent_obj;
	if (obj->parent != -1)	{
		obj->parent_sig		= Objects[parent_obj].signature;
	} else {
		obj->parent_sig = obj->signature;
	}

	obj->flags 					= flags;
    obj->flags.set(Object::Object_Flags::Not_in_coll);
	if (pos)	{
		obj->pos 				= *pos;
		obj->last_pos			= *pos;
	}

	if (orient)	{
		obj->orient 			= *orient;
		obj->last_orient		= *orient;
	}
	obj->radius 				= radius;

	obj->shield_quadrant.resize(DEFAULT_SHIELD_SECTIONS);	// Might be changed by the ship creation code

	return objnum;
}

void obj_delete_all() 
{
	int counter = 0;
	for (int i = 0; i < MAX_OBJECTS; ++i) 
	{
		if (Objects[i].type == OBJ_NONE)
			continue;
		++counter;
		obj_delete(i);
	}

	mprintf(("Cleanup: Deleted %i objects\n", counter));
}

/**
 * Remove object from the world
 * If Player_obj, don't remove it!
 * 
 * @param objnum Object number to remove
 */
void obj_delete(int objnum)
{
	object *objp;

	Assert(objnum >= 0 && objnum < MAX_OBJECTS);
	objp = &Objects[objnum];
	if (objp->type == OBJ_NONE) {
		mprintf(("obj_delete() called for already deleted object %d.\n", objnum));
		return;
	};	

	// Remove all object pairs
	obj_remove_collider(objnum);

	switch( objp->type )	{
	case OBJ_WEAPON:
		weapon_delete( objp );
		break;
	case OBJ_SHIP:
		if ((objp == Player_obj) && !Fred_running) {
			objp->type = OBJ_GHOST;
            objp->flags.remove(Object::Object_Flags::Should_be_dead);
			
			// we have to traverse the ship_obj list and remove this guy from it as well
			ship_obj *moveup = GET_FIRST(&Ship_obj_list);
			while(moveup != END_OF_LIST(&Ship_obj_list)){
				if(OBJ_INDEX(objp) == moveup->objnum){
					list_remove(&Ship_obj_list,moveup);
					break;
				}
				moveup = GET_NEXT(moveup);
			}

			physics_init(&objp->phys_info);
			obj_snd_delete_type(OBJ_INDEX(objp));

			return;
		} else
			ship_delete( objp );
		break;
	case OBJ_FIREBALL:
		fireball_delete( objp );
		break;
	case OBJ_SHOCKWAVE:
		shockwave_delete( objp );
		break;
	case OBJ_START:
	case OBJ_WAYPOINT:
	case OBJ_POINT:
		break;  // requires no action, handled by the Fred code.
	case OBJ_JUMP_NODE:
		break;  // requires no further action, handled by jumpnode deconstructor.
	case OBJ_DEBRIS:
		debris_delete( objp );
		break;
	case OBJ_ASTEROID:
		asteroid_delete(objp);
		break;
/*	case OBJ_CMEASURE:
		cmeasure_delete( objp );
		break;*/
	case OBJ_GHOST:
		if(!(Game_mode & GM_MULTIPLAYER)){
			mprintf(("Warning: Tried to delete a ghost!\n"));
			objp->flags.remove(Object::Object_Flags::Should_be_dead);
			return;
		} else {
			// we need to be able to delete GHOST objects in multiplayer to allow for player respawns.
			nprintf(("Network","Deleting GHOST object\n"));
			objp->net_signature = 0;
		}		
		break;
	case OBJ_OBSERVER:
		observer_delete(objp);
		break;	
	case OBJ_BEAM:
		break;
	case OBJ_RAW_POF:
		model_delete_instance(Pof_objects[objp->instance].model_instance);
		Pof_objects.erase(objp->instance);
		break;
	case OBJ_NONE:
		Int3();
		break;
	default:
		Error( LOCATION, "Unhandled object type %d in obj_delete_all_that_should_be_dead", objp->type );
	}

	// this avoids include issues from physics state, multi interpolate and object code
	extern void multi_interpolate_clear_helper(int objnum);

	// clean up interpolation info
	multi_interpolate_clear_helper(objnum);

	// delete any dock information we still have
	dock_free_dock_list(objp);
	dock_free_dead_dock_list(objp);

	// if a persistant sound has been created, delete it
	obj_snd_delete_type(OBJ_INDEX(objp));		

	objp->type = OBJ_NONE;		//unused!
	objp->signature = 0;

	obj_free(objnum);
}


//	------------------------------------------------------------------------------------------------------------------
void obj_delete_all_that_should_be_dead()
{
	object *objp, *temp;

	if (!Object_inited) {
		mprintf(("Why hasn't obj_init() been called yet?\n"));
		obj_init();
	}

	// Move all objects
	objp = GET_FIRST(&obj_used_list);
	while( objp !=END_OF_LIST(&obj_used_list) )	{
		// Goober5000 - HACK HACK HACK - see obj_move_all
		objp->flags.remove(Object::Object_Flags::Docked_already_handled);

		temp = GET_NEXT(objp);
		if ( objp->flags[Object::Object_Flags::Should_be_dead] )
			obj_delete( OBJ_INDEX(objp) );			// MWA says that john says that let obj_delete handle everything because of the editor
		objp = temp;
	}

}

/**
 * Add all newly created objects to the end of the used list and create their
 * object pairs for collision detection
 */
void obj_merge_created_list(void)
{
	// The old way just merged the two.   This code takes one out of the create list,
	// creates object pairs for it, and then adds it to the used list.
	//	OLD WAY: list_merge( &obj_used_list, &obj_create_list );
	object *objp = GET_FIRST(&obj_create_list);
	while( objp !=END_OF_LIST(&obj_create_list) )	{
		list_remove( obj_create_list, objp );

		// Add it to the object pairs array
		obj_add_collider(OBJ_INDEX(objp));

		// Then add it to the object used list
		list_append( &obj_used_list, objp );

		objp = GET_FIRST(&obj_create_list);
	}

	// Make sure the create list is empty.
	list_init(&obj_create_list);
}

int physics_paused = 0, ai_paused = 0;


// Goober5000
extern void call_doa(object *child, object *parent);
void obj_move_one_docked_object(object *objp, object *parent_objp)
{
	// in FRED, just move and return
	if (Fred_running)
	{
		call_doa(objp, parent_objp);
		return;
	}

	// support ships (and anyone else, for that matter) don't keep up if they're undocking
	ai_info *aip = &Ai_info[Ships[objp->instance].ai_index];
	if ( (aip->mode == AIM_DOCK) && (aip->submode >= AIS_UNDOCK_1) )
	{
		if (aip->goal_objnum == OBJ_INDEX(parent_objp))
		{
			return;
		}
	}

	// check the guy that I'm docked with and don't move if he's undocking from me
	ai_info *other_aip = &Ai_info[Ships[parent_objp->instance].ai_index];
	if ( (other_aip->mode == AIM_DOCK) && (other_aip->submode >= AIS_UNDOCK_1) )
	{
		if (other_aip->goal_objnum == OBJ_INDEX(objp))
		{
			return;
		}
	}

	// we're here, so we move with our parent object
	call_doa(objp, parent_objp);
}

/**
 * Deals with firing player things like lasers, missiles, etc.
 *
 * Separated out because of multiplayer issues.
*/
void obj_player_fire_stuff( object *objp, control_info ci )
{
	ship *shipp;

	Assert( objp->flags[Object::Object_Flags::Player_ship]);

	// try and get the ship pointer
	shipp = NULL;
	if((objp->type == OBJ_SHIP) && (objp->instance >= 0) && (objp->instance < MAX_SHIPS)){
		shipp = &Ships[objp->instance];
	} else {
		return;
	}

	ship_weapon* swp = &shipp->weapons;

	// single player pilots, and all players in multiplayer take care of firing their own primaries
	if(!(Game_mode & GM_MULTIPLAYER) || (objp == Player_obj))
	{
		if ( ci.fire_primary_count ) {
			// flag the ship as having the trigger down
			if(shipp != NULL){
				shipp->flags.set(Ship::Ship_Flags::Trigger_down);
				swp->flags.set(Ship::Weapon_Flags::Primary_trigger_down);
			}

			// fire non-streaming primaries here
			// Cyborg17, this is where the inaccurate multi shots are being shot... 
			// so let's let the new system take over instead by excluding client player shots
			// on the server.
			if (!(MULTIPLAYER_MASTER) || (objp == Player_obj)) {
				ship_fire_primary(objp);
			}
			
		} else {
			// unflag the ship as having the trigger down
			if(shipp != NULL){
                shipp->flags.remove(Ship::Ship_Flags::Trigger_down);
				swp->flags.remove(Ship::Weapon_Flags::Primary_trigger_down);
				ship_stop_fire_primary(objp);	//if it hasn't fired do the "has just stoped fireing" stuff
			}
		}

		if ( ci.fire_countermeasure_count ) {
			ship_launch_countermeasure( objp );
		}
	}

	// single player and multiplayer masters do all of the following
	if ( !MULTIPLAYER_CLIENT 
		// Cyborg17 - except clients now fire dumbfires for rollback on the server
		|| !(Weapon_info[swp->secondary_bank_weapons[shipp->weapons.current_secondary_bank]].is_homing())) {
		if (ci.fire_secondary_count) {
   			if ( !ship_start_secondary_fire(objp) ) {
				ship_fire_secondary( objp );
			}

			// kill the secondary count
			ci.fire_secondary_count = 0;
		} else {
			if ( ship_stop_secondary_fire(objp) ) {
				ship_fire_secondary( objp );
			}
		}
	}

	if ( MULTIPLAYER_CLIENT && objp == Player_obj ) {
		if (Weapon_info[swp->secondary_bank_weapons[shipp->weapons.current_secondary_bank]].trigger_lock) {
			if (ci.fire_secondary_count) {
				ship_start_secondary_fire(objp);
			} else {
				ship_stop_secondary_fire(objp);
			}
		}	
	}

	// everyone does the following for their own ships.
	if ( ci.afterburner_start ){
		if (Ships[objp->instance].flags[Ship::Ship_Flags::Maneuver_despite_engines] || !ship_subsystems_blown(&Ships[objp->instance], SUBSYSTEM_ENGINE)) {
			afterburners_start( objp );
		}
	}
	
	if ( ci.afterburner_stop ){
		afterburners_stop( objp, 1 );
	}
	
}

void obj_move_call_physics(object *objp, float frametime)
{
	TRACE_SCOPE(tracing::Physics);

	//	Do physics for objects with OF_PHYSICS flag set and with some engine strength remaining.
	if ( objp->flags[Object::Object_Flags::Physics] ) {
		// only set phys info if ship is not dead
		if ((objp->type == OBJ_SHIP) && !(Ships[objp->instance].flags[Ship::Ship_Flags::Dying])) {
			ship *shipp = &Ships[objp->instance];

			if (!shipp->flags[Ship::Ship_Flags::Maneuver_despite_engines]) {
				bool engines_blown = ship_subsystems_blown(shipp, SUBSYSTEM_ENGINE);
				if ( ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE) ) {
					engines_blown = true;
				}

				if (engines_blown) {	//	All this is necessary to make ship gradually come to a stop after engines are blown.
					vm_vec_zero(&objp->phys_info.desired_vel);
					vm_vec_zero(&objp->phys_info.desired_rotvel);
					vm_mat_zero(&objp->phys_info.ai_desired_orient);
					objp->phys_info.flags |= (PF_REDUCED_DAMP | PF_DEAD_DAMP);
					objp->phys_info.side_slip_time_const = Ship_info[shipp->ship_info_index].damp * 4.0f;
				}
			}
			// recover if we *are* maneuvering but the flag was added
			else if ((objp->phys_info.flags & PF_DEAD_DAMP) && !shipp->flags[Ship::Ship_Flags::Dying]) {
				ship_reset_disabled_physics(objp, shipp->ship_info_index);
			}
		}

		// if a weapon is flagged as dead, kill its engines just like a ship
		if((objp->type == OBJ_WEAPON) && (Weapons[objp->instance].weapon_flags[Weapon::Weapon_Flags::Dead_in_water])){
			vm_vec_zero(&objp->phys_info.desired_vel);
			vm_vec_zero(&objp->phys_info.desired_rotvel);
			objp->phys_info.flags |= (PF_REDUCED_DAMP | PF_DEAD_DAMP);
			objp->phys_info.side_slip_time_const = 1.0f;	// FIXME?  originally indexed into Ship_info[], which was a bug...
		}

		if (physics_paused)	{
			if (objp==Player_obj){
				physics_sim(&objp->pos, &objp->orient, &objp->phys_info, &The_mission.gravity, frametime );		// simulate the physics
			}
		} else {
			//	Hack for dock mode.
			//	If docking with a ship, we don't obey the normal ship physics, we can slew about.
			if (objp->type == OBJ_SHIP) {
				ai_info	*aip = &Ai_info[Ships[objp->instance].ai_index];

				//	Note: This conditional for using PF_USE_VEL (instantaneous acceleration) is probably too loose.
				//	A ships awaiting support will fly towards the support ship with instantaneous acceleration.
				//	But we want to have ships in the process of docking have quick acceleration, or they overshoot their goals.
				//	Probably can not key off objnum_I_am_docked_or_docking_with, but then need to add some other condition.  Live with it for now. -- MK, 2/19/98

				// Goober5000 - no need to key off objnum; other conditions get it just fine

				if (/* (objnum_I_am_docked_or_docking_with != -1) || */
					((aip->mode == AIM_DOCK) && ((aip->submode == AIS_DOCK_2) || (aip->submode == AIS_DOCK_3) || (aip->submode == AIS_UNDOCK_0))) ||
					((aip->mode == AIM_WARP_OUT) && (aip->submode >= AIS_WARP_3))) {
					if (Ships[objp->instance].flags[Ship::Ship_Flags::Maneuver_despite_engines] || !ship_subsystems_blown(&Ships[objp->instance], SUBSYSTEM_ENGINE)){
						objp->phys_info.flags |= PF_USE_VEL;
					} else {
						objp->phys_info.flags &= ~PF_USE_VEL;	//	If engine blown, don't PF_USE_VEL, or ships stop immediately
					}
				} else {
					objp->phys_info.flags &= ~PF_USE_VEL;
				}
			}			

			// simulate the physics
			physics_sim(&objp->pos, &objp->orient, &objp->phys_info, &The_mission.gravity,  frametime);

			// if the object is the player object, do things that need to be done after the ship
			// is moved (like firing weapons, etc).  This routine will get called either single
			// or multiplayer.  We must find the player object to get to the control info field
			if ( (objp->flags[Object::Object_Flags::Player_ship]) && (objp->type != OBJ_OBSERVER) && (objp == Player_obj)) {
				player *pp;
				if(Player != NULL){
					pp = Player;
					obj_player_fire_stuff( objp, pp->ci );				
				}
			}
		}
	}

	//2D MODE
	//THIS IS A FREAKIN' HACK
	//Do not let ship change position on Y axis
	if(The_mission.flags[Mission::Mission_Flags::Mission_2d])
	{
		angles old_angles, new_angles;
		objp->pos.xyz.y = objp->last_pos.xyz.y;
		vm_extract_angles_matrix(&old_angles, &objp->last_orient);
		vm_extract_angles_matrix(&new_angles, &objp->orient);
		new_angles.p = old_angles.p;
		new_angles.b = old_angles.b;
		vm_angles_2_matrix(&objp->orient, &new_angles);

		//Phys stuff hack
		new_angles.h = old_angles.h;
		vm_angles_2_matrix(&objp->phys_info.last_rotmat, &new_angles);
		objp->phys_info.vel.xyz.y = 0.0f;
		objp->phys_info.desired_rotvel.xyz.x = 0;
		objp->phys_info.desired_rotvel.xyz.z = 0;
		objp->phys_info.desired_vel.xyz.y = 0.0f;
	}
}


#ifdef OBJECT_CHECK 

void obj_check_object( object *obj )
{
	int objnum = OBJ_INDEX(obj);

	// PROGRAMMERS: If one of these Int3() gets hit, then someone
	// is changing a value in the object structure that might cause
	// collision detection to not work.  See John for more info if
	// you are hitting one of these.

	if ( CheckObjects[objnum].type != obj->type )	{
		if ( (obj->type==OBJ_WAYPOINT) && (CheckObjects[objnum].type==OBJ_SHIP) )	{
			// We know about ships changing into waypoints and that is
			// ok.
			CheckObjects[objnum].type = OBJ_WAYPOINT;
		 } else if ( (obj->type==OBJ_SHIP) && (CheckObjects[objnum].type==OBJ_GHOST) )	{
			// We know about player changing into a ghost after dying and that is
			// ok.
			CheckObjects[objnum].type = OBJ_GHOST;
		} else if ( (obj->type==OBJ_GHOST) && (CheckObjects[objnum].type==OBJ_SHIP) )	{
			// We know about player changing into a ghost after dying and that is
			// ok.
			CheckObjects[objnum].type = OBJ_SHIP;
		} else {
			mprintf(( "Object type changed! Old: %i, Current: %i\n", CheckObjects[objnum].type, obj->type ));
			Int3();
		}
	}
	if ( CheckObjects[objnum].signature != obj->signature ) {
		mprintf(( "Object signature changed!\n" ));
		Int3();
	}
	if ( (CheckObjects[objnum].flags[Object::Object_Flags::Collides]) != (obj->flags[Object::Object_Flags::Collides]) ) {
		mprintf(( "Object flags changed!\n" ));
		Int3();
	}
	if ( CheckObjects[objnum].parent_sig != obj->parent_sig ) {
		mprintf(( "Object parent sig changed!\n" ));
		Int3();
	}
}
#endif

/**
 * Call this if you want to change an object flag so that the
 * object code knows what's going on.  For instance if you turn
 * off OF_COLLIDES, the object code needs to know this in order to
 * actually turn the object collision detection off.  By calling
 * this you shouldn't get Int3's in the checkobject code.  If you
 * do, then put code in here to correctly handle the case.
 */
void obj_set_flags( object *obj, const flagset<Object::Object_Flags>& new_flags )
{
	int objnum = OBJ_INDEX(obj);	

	// turning collision detection off
	if ( (obj->flags[Object::Object_Flags::Collides]) && (!(new_flags[Object::Object_Flags::Collides])))	{
		// Remove all object pairs
		obj_remove_collider(objnum);

		// update object flags properly		
		obj->flags = new_flags;
        obj->flags.set(Object::Object_Flags::Not_in_coll);
#ifdef OBJECT_CHECK
		CheckObjects[objnum].flags = new_flags;
        CheckObjects[objnum].flags.set(Object::Object_Flags::Not_in_coll);
#endif		
		return;
	}
	
	
	// turning collision detection on
	if ( (!(obj->flags[Object::Object_Flags::Collides])) && (new_flags[Object::Object_Flags::Collides]) )	{
		
		// observers can't collide or be hit, and they therefore have no hit or collide functions
		// So, don't allow this bit to be set
		if(obj->type == OBJ_OBSERVER){
			mprintf(("Illegal to set collision bit for OBJ_OBSERVER!!\n"));
			Int3();
		}

		obj->flags.set(Object::Object_Flags::Collides);

		// Turn on collision detection
		obj_add_collider(objnum);

		obj->flags = new_flags;
        obj->flags.remove(Object::Object_Flags::Not_in_coll);
#ifdef OBJECT_CHECK
		CheckObjects[objnum].flags = new_flags;
		CheckObjects[objnum].flags.remove(Object::Object_Flags::Not_in_coll);
#endif
		return;
	}

	// for a multiplayer host -- use this debug code to help trap when non-player ships are getting
	// marked as OF_COULD_BE_PLAYER
	// this code is pretty much debug code and shouldn't be relied on to always do the right thing
	// for flags other than 
	if ( MULTIPLAYER_MASTER && !(obj->flags[Object::Object_Flags::Could_be_player]) && (new_flags[Object::Object_Flags::Could_be_player]) ) {
		ship *shipp;
		int team, slot;

		// this flag sometimes gets set for observers.
		if ( obj->type == OBJ_OBSERVER ) {
			return;
		}

		// sanity checks
		if ( (obj->type != OBJ_SHIP) || (obj->instance < 0) ) {
			return;				// return because we really don't want to set the flag
		}

		// see if this ship is really a player ship (or should be)
		shipp = &Ships[obj->instance];
		extern void multi_ts_get_team_and_slot(char *, int *, int *);
		multi_ts_get_team_and_slot(shipp->ship_name,&team,&slot);
		if ( (shipp->wingnum == -1) || (team == -1) || (slot==-1) ) {
			Int3();
			return;
		}

		// set the flag
		obj->flags = new_flags;
#ifdef OBJECT_CHECK
		CheckObjects[objnum].flags = new_flags;
#endif

		return;
	}

	// Check for unhandled flag changing
	if ( (new_flags[Object::Object_Flags::Collides]) != (obj->flags[Object::Object_Flags::Collides]) ) {
		mprintf(( "Unhandled flag changing in obj_set_flags!!\n" ));
		mprintf(( "Add code to support it, see John for questions!!\n" ));
		Int3();
	} else {
		// Since it wasn't an important flag, just bash it.
		obj->flags = new_flags;
		#ifdef OBJECT_CHECK 
		CheckObjects[objnum].flags = new_flags;
		#endif
	}	
}


void obj_move_all_pre(object *objp, float frametime)
{
	TRACE_SCOPE(tracing::PreMove);

	switch( objp->type )	{
	case OBJ_WEAPON:
		if (!physics_paused){
 			weapon_process_pre( objp, frametime );
		}
		break;	
	case OBJ_SHIP:
		if (!physics_paused || (objp==Player_obj )){
			ship_process_pre( objp, frametime );
		}
		break;
	case OBJ_FIREBALL:
		// all fireballs are moved via fireball_process_post()
		break;
	case OBJ_SHOCKWAVE:
		// all shockwaves are moved via shockwave_move_all()
		break;
	case OBJ_DEBRIS:
		// all debris are moved via debris_process_post()
		break;
	case OBJ_ASTEROID:
		if (!physics_paused){
			asteroid_process_pre(objp);
		}
		break;
/*	case OBJ_CMEASURE:
		if (!physics_paused){
			cmeasure_process_pre(objp, frametime);
		}
		break;*/
	case OBJ_WAYPOINT:
		break;  // waypoints don't move..
	case OBJ_GHOST:
		break;
	case OBJ_OBSERVER:
	case OBJ_JUMP_NODE:	
		break;	
	case OBJ_BEAM:		
		break;
	case OBJ_RAW_POF:
		break;
	case OBJ_NONE:
		Int3();
		break;
	default:
		Error(LOCATION, "Unhandled object type %d in obj_move_all_pre\n", objp->type);
	}

	objp->pre_move_event(objp);
}

// Used to tell if a particular group of lasers has cast light yet.
ubyte Obj_weapon_group_id_used[WEAPON_MAX_GROUP_IDS];

/**
 * Called once a frame to mark all weapon groups as not having cast light yet.
 */
void obj_clear_weapon_group_id_list()
{
	memset( Obj_weapon_group_id_used, 0, sizeof(Obj_weapon_group_id_used) );
}

int Arc_light = 1;		// If set, electrical arcs on debris cast light
DCF_BOOL(arc_light, Arc_light)	

void obj_move_all_post(object *objp, float frametime)
{
	switch (objp->type)
	{
		case OBJ_WEAPON:
		{
			TRACE_SCOPE(tracing::WeaponPostMove);

			if ( !physics_paused )
				weapon_process_post( objp, frametime );

			// Cast light
			if ( Detail.lighting > 3 ) {
				// Weapons cast light

				int group_id = Weapons[objp->instance].group_id;
				int cast_light = 1;

				if (group_id >= 0) {
					if (Obj_weapon_group_id_used[group_id] == 0) {
						// Mark this group as done
						Obj_weapon_group_id_used[group_id]++;
					}
					else {
						// This group has already done its light casting
						cast_light = 0;
					}
				}
				if (cast_light) {
					weapon* wp = &Weapons[objp->instance];
					weapon_info* wi = &Weapon_info[wp->weapon_info_index];
					auto lp = lighting_profiles::current();
					hdr_color light_color;

					float intensity_mult = wi->weapon_curves.get_output(weapon_info::WeaponCurveOutputs::LIGHT_INTENSITY_MULT, *wp, &wp->modular_curves_instance);
					float radius_mult = wi->weapon_curves.get_output(weapon_info::WeaponCurveOutputs::LIGHT_RADIUS_MULT, *wp, &wp->modular_curves_instance);
					float r_mult = wi->weapon_curves.get_output(weapon_info::WeaponCurveOutputs::LIGHT_R_MULT, *wp, &wp->modular_curves_instance);
					float g_mult = wi->weapon_curves.get_output(weapon_info::WeaponCurveOutputs::LIGHT_G_MULT, *wp, &wp->modular_curves_instance);
					float b_mult = wi->weapon_curves.get_output(weapon_info::WeaponCurveOutputs::LIGHT_B_MULT, *wp, &wp->modular_curves_instance);

					float source_radius = objp->radius;
					float light_radius;
					float light_brightness;

					// Handle differing adjustments depending on weapon type.
					if (wi->render_type == WRT_LASER) {
						light_radius = lp->laser_light_radius.handle(wi->light_radius) * radius_mult;
						// intensity is stored in the light color even if no user setting is done.
						light_brightness = lp->laser_light_brightness.handle(wi->light_color.i());
					} else {
						// Missiles should typically not be treated as lights for their whole radius. TODO: make configurable.
						source_radius *= 0.05f;
						light_radius = lp->missile_light_radius.handle(wi->light_radius) * radius_mult;
						light_brightness = lp->missile_light_brightness.handle(wi->light_color.i());
					}

					// If there is no specific color set in the table, laser render weapons have a dynamic color.
					if (!wi->light_color_set && wi->render_type == WRT_LASER) {
						// Classic dynamic laser color is handled with an old color object
						color c;
						weapon_get_laser_color(&c, objp);
						light_color.set_rgbai((c.red/255.f), (c.green/255.f), (c.blue/255.f), 1.f, light_brightness);
					} else {
						// If not a laser then all default information needed is stored in the weapon light color
						light_color.set_rgbai(wi->light_color.r(), wi->light_color.g(), wi->light_color.b(), 1.f, light_brightness);
					}

					light_color.multiply_rgbai(r_mult, g_mult, b_mult, 1.f, intensity_mult);

					if(light_radius > 0.0f && intensity_mult > 0.0f && light_color.i() > 0.0f)
						light_add_point(&objp->pos, light_radius, light_radius, &light_color, source_radius);
				}
			}

			break;
		}

		case OBJ_SHIP:
		{
			TRACE_SCOPE(tracing::ShipPostMove);

			if ( !physics_paused || (objp==Player_obj) ) {
				ship_process_post( objp, frametime );
			}

			// Make any electrical arcs on ships cast light
			if (Arc_light)	{
				if ( (Detail.lighting > 3) && (objp != Viewer_obj) ) {
					auto shipp = &Ships[objp->instance];

					for (auto &arc: shipp->electrical_arcs) {
						if ( arc.timestamp.isValid() )	{
							// Move arc endpoints into world coordinates	
							vec3d tmp1, tmp2;
							vm_vec_unrotate(&tmp1,&arc.endpoint_1,&objp->orient);
							vm_vec_add2(&tmp1,&objp->pos);

							vm_vec_unrotate(&tmp2,&arc.endpoint_2,&objp->orient);
							vm_vec_add2(&tmp2,&objp->pos);

							light_add_point( &tmp1, 10.0f, 20.0f, frand(), 1.0f, 1.0f, 1.0f);
							light_add_point( &tmp2, 10.0f, 20.0f, frand(), 1.0f, 1.0f, 1.0f);
						}
					}
				}
			}	

			//Check for changing team colors
			ship* shipp = &Ships[objp->instance];
			if (Ship_info[shipp->ship_info_index].uses_team_colors && stricmp(shipp->secondary_team_name.c_str(), "none") != 0) {
				if (f2fl(Missiontime) * 1000 > f2fl(shipp->team_change_timestamp) * 1000 + shipp->team_change_time) {
					shipp->team_name = shipp->secondary_team_name;
					shipp->team_change_timestamp = 0;
					shipp->team_change_time = 0;
					shipp->secondary_team_name = "none";
				}
			}

			break;
		}

		case OBJ_FIREBALL:
		{
			TRACE_SCOPE(tracing::FireballPostMove);

			if ( !physics_paused )
				fireball_process_post(objp,frametime);

			if (Detail.lighting > 2) {
				float r = 0.0f, g = 0.0f, b = 0.0f;
				// Make sure the new system works fine.
				Assert(objp->instance > -1);
				Assert(static_cast<int>(Fireballs.size()) > objp->instance);

				fireball_get_color(Fireballs[objp->instance].fireball_info_index, &r, &g, &b);

				// we don't cast black light, so just bail in that case
				if ( (r == 0.0f) && (g == 0.0f) && (b == 0.0f) )
					break;

				// Make explosions cast light
				float p = fireball_lifeleft_percent(objp);
				if (p > 0.0f) {
					if (p > 0.5f)
						p = 1.0f - p;

					p *= 2.0f;
					float rad = p * (1.0f + frand() * 0.05f) * objp->radius;
					
					float intensity = 1.0f;
					if (fireball_is_warp(objp))
					{
						// Make sure the new system works fine.
						Assert(static_cast<int>(Fireballs.size()) > objp->instance);
						Assert(objp->instance > -1);
						intensity = fireball_wormhole_intensity(&Fireballs[objp->instance]); // Valathil: Get wormhole radius for lighting
						rad = objp->radius;
					}
					// P goes from 0 to 1 to 0 over the life of the explosion
					// Only do this if rad is > 0.0000001f
					// TODO: Make fireball source radius configurable, currently sized based on modern subspace portal textures as that will be a very prominent case of it
					if (rad > 0.0001f)
						light_add_point( &objp->pos, rad * 2.0f, rad * 5.0f, intensity, r, g, b,rad * 0.3f);
				}
			}

			break;
		}

		case OBJ_SHOCKWAVE:
			// all shockwaves are moved via shockwave_move_all()
			break;

		case OBJ_DEBRIS:
		{
			TRACE_SCOPE(tracing::DebrisPostMove);

			if ( !physics_paused )
				debris_process_post(objp, frametime);

			// Make any electrical arcs on debris cast light
			if (Arc_light)	{
				if ( Detail.lighting > 3 ) {
					auto db = &Debris[objp->instance];

					if (db->arc_frequency > 0) {
						for (auto &arc: db->electrical_arcs) {
							if ( arc.timestamp.isValid() )	{
								// Move arc endpoints into world coordinates	
								vec3d tmp1, tmp2;
								vm_vec_unrotate(&tmp1,&arc.endpoint_1,&objp->orient);
								vm_vec_add2(&tmp1,&objp->pos);

								vm_vec_unrotate(&tmp2,&arc.endpoint_2,&objp->orient);
								vm_vec_add2(&tmp2,&objp->pos);

								light_add_point( &tmp1, 10.0f, 20.0f, frand(), 1.0f, 1.0f, 1.0f );
								light_add_point( &tmp2, 10.0f, 20.0f, frand(), 1.0f, 1.0f, 1.0f );
							}
						}
					}
				}
			}

			break;
		}

		case OBJ_ASTEROID:
		{
			TRACE_SCOPE(tracing::AsteroidPostMove);

			if ( !physics_paused )
				asteroid_process_post(objp);

			break;
		}

		case OBJ_WAYPOINT:
			break;  // waypoints don't move..

		case OBJ_GHOST:
			break;

		case OBJ_OBSERVER:
			void observer_process_post(object *objp);
			observer_process_post(objp);
			break;

		case OBJ_JUMP_NODE:
			radar_plot_object(objp);
			break;	

		case OBJ_BEAM:		
			break;

		case OBJ_RAW_POF:
			break;

		case OBJ_NONE:
			Int3();
			break;

		default:
		    Error(LOCATION, "Unhandled object type %d in obj_move_all_post\n", objp->type);
	    }

	    objp->post_move_event(objp);
}


int Collisions_enabled = 1;

DCF_BOOL( collisions, Collisions_enabled )

MONITOR( NumObjects )

/**
 * Move all objects for the current frame
 */
void obj_move_all(float frametime)
{
	TRACE_SCOPE(tracing::MoveObjects);

	object *objp;	
	SCP_vector<object*> cmeasure_list;
	const bool global_cmeasure_timer = (Cmeasures_homing_check > 0);

	Assertion(Cmeasures_homing_check >= 0, "Cmeasures_homing_check is %d in obj_move_all(); it should never be negative. Get a coder!\n", Cmeasures_homing_check);

	if (global_cmeasure_timer)
		Cmeasures_homing_check--;

	// Goober5000 - HACK HACK HACK
	// this function also resets the OF_DOCKED_ALREADY_HANDLED flag, to save trips
	// through the used object list
	obj_delete_all_that_should_be_dead();

	obj_merge_created_list();

	// Clear the table that tells which groups of weapons have cast light so far.
	if(!(Game_mode & GM_MULTIPLAYER) || (MULTIPLAYER_MASTER)) {
		obj_clear_weapon_group_id_list();
	}

	MONITOR_INC( NumObjects, Num_objects );	

	for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		// skip objects which should be dead
		if (objp->flags[Object::Object_Flags::Should_be_dead]) {
			continue;
		}

		// if this is an observer object, skip it
		if (objp->type == OBJ_OBSERVER) {
			continue;
		}

		// Compile a list of active countermeasures during an existing traversal of obj_used_list
		if (objp->type == OBJ_WEAPON) {
			weapon *wp = &Weapons[objp->instance];
			weapon_info *wip = &Weapon_info[wp->weapon_info_index];

			if (wip->wi_flags[Weapon::Info_Flags::Cmeasure]) {
				if ((wip->cmeasure_timer_interval > 0 && timestamp_elapsed(wp->cmeasure_timer))	// If it's timer-based and ready to pulse...
					|| (wip->cmeasure_timer_interval <= 0 && global_cmeasure_timer)) {	// ...or it's not and the global counter is active...
					// ...then it's actively pulsing and we need to add objp to cmeasure_list.
					cmeasure_list.push_back(objp);
					if (wip->cmeasure_timer_interval > 0) {
						// Reset the timer
						wp->cmeasure_timer = timestamp(wip->cmeasure_timer_interval);
					}
				}
			}
		}

		vec3d cur_pos = objp->pos;			// Save the current position

#ifdef OBJECT_CHECK 
			obj_check_object( objp );
#endif

		// pre-move
		obj_move_all_pre(objp, frametime);

		bool interpolation_object = multi_oo_is_interp_object(objp);

		// store last pos and orient, but only for non-interpolation objects
		// interpolation objects will need to to work backwards from the last good position
		// to prevent collision issues
		if (!interpolation_object){
			objp->last_pos = cur_pos;
			objp->last_orient = objp->orient;
		}

		// Goober5000 - accommodate objects that aren't supposed to move in some way (at least until they're destroyed)
		bool dont_change_position = objp->flags[Object::Object_Flags::Dont_change_position, Object::Object_Flags::Immobile] && objp->hull_strength > 0.0f;
		bool dont_change_orientation = objp->flags[Object::Object_Flags::Dont_change_orientation, Object::Object_Flags::Immobile] && objp->hull_strength > 0.0f;

		// skip the physics if we're totally immobile
		if (!dont_change_position || !dont_change_orientation) {
			// if this is an object which should be interpolated in multiplayer, do so
			if (interpolation_object) {
				extern void interpolate_main_helper(int objnum, vec3d* pos, matrix* ori, physics_info* pip, vec3d* last_pos, matrix* last_orient, vec3d* gravity, bool player_ship);

				interpolate_main_helper(OBJ_INDEX(objp), &objp->pos, &objp->orient, &objp->phys_info, &objp->last_pos, &objp->last_orient, &The_mission.gravity, objp->flags[Object::Object_Flags::Player_ship]);
			} else {
				// physics
				obj_move_call_physics(objp, frametime);
			}
		}

		// If the object isn't supposed to move, roll back any movement that occurred.  Most of the movement should already have been skipped, but this ensures complete immobility.
		if (dont_change_position) {
			objp->pos = objp->last_pos;

			// make sure velocity is always 0
			vm_vec_zero(&objp->phys_info.vel);
			vm_vec_zero(&objp->phys_info.desired_vel);
			objp->phys_info.speed = 0.0f;
			objp->phys_info.fspeed = 0.0f;
		}
		if (dont_change_orientation) {
			objp->orient = objp->last_orient;

			// make sure velocity is always 0
			vm_vec_zero(&objp->phys_info.rotvel);
			vm_vec_zero(&objp->phys_info.desired_rotvel);
		}

		// Submodel movement now happens here, right after physics movement.  It's not excluded by the "immobile", "don't-change-position", or "don't-change-orientation" flags.
		
		// this flag only affects ship subsystems, not any other type of submodel movement
		if (objp->type == OBJ_SHIP && !Ships[objp->instance].flags[Ship::Ship_Flags::Subsystem_movement_locked])
			ship_move_subsystems(objp);

		// do animation on this object
		int model_instance_num = object_get_model_instance_num(objp);
		if (model_instance_num >= 0) {
			polymodel_instance* pmi = model_get_instance(model_instance_num);
			animation::ModelAnimation::stepAnimations(frametime, pmi);
		}

		// finally, do intrinsic motion on this object
		// (this happens last because look_at is a type of intrinsic rotation,
		// and look_at needs to happen last or the angle may be off by a frame)
		model_do_intrinsic_motions(objp);

		// For ships, we now have to make sure that all the submodel detail levels remain consistent.
		if (objp->type == OBJ_SHIP)
			ship_model_replicate_submodels(objp);

		// move post
		obj_move_all_post(objp, frametime);

		// Equipment script processing
		if (objp->type == OBJ_SHIP) {
			ship* shipp = &Ships[objp->instance];
			object* target;

			if (Ai_info[shipp->ai_index].target_objnum != -1)
				target = &Objects[Ai_info[shipp->ai_index].target_objnum];
			else
				target = NULL;
			if (objp == Player_obj && Player_ai->target_objnum != -1)
				target = &Objects[Player_ai->target_objnum];

			if (scripting::hooks::OnWeaponEquipped->isActive()) {
				scripting::hooks::OnWeaponEquipped->run(scripting::hooks::WeaponEquippedConditions{ shipp, target },
					scripting::hook_param_list(
						scripting::hook_param("User", 'o', objp),
						scripting::hook_param("Target", 'o', target)
					));
			}
		}
	}

	// Now apply intrinsic motion to things that aren't objects (like skyboxes).  This technically doesn't belong in the object code,
	// but there isn't really a good place to put this, it doesn't hurt to have this here, and it's conceptually related to what's here.
	model_do_intrinsic_motions(nullptr);

	//	After all objects have been moved, move all docked objects.
	for (objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp)) {
		// skip objects which should be dead
		if (objp->flags[Object::Object_Flags::Should_be_dead]) {
			continue;
		}

		dock_move_docked_objects(objp);

		//Valathil - Move the screen rotation calculation for billboards here to get the updated orientation matrices caused by docking interpolation
		vec3d tangles;

		tangles.xyz.x = -objp->phys_info.rotvel.xyz.x*frametime;
		tangles.xyz.y = -objp->phys_info.rotvel.xyz.y*frametime;
		tangles.xyz.z = objp->phys_info.rotvel.xyz.z*frametime;

		// If this is the viewer_object, keep track of the
		// changes in banking so that rotated bitmaps look correct.
		// This is used by the g3_draw_rotated_bitmap function.
		extern physics_info *Viewer_physics_info;
		if ( &objp->phys_info == Viewer_physics_info )	{
			vec3d tangles_r;
			vm_vec_unrotate(&tangles_r, &tangles, &Eye_matrix);
			vm_vec_rotate(&tangles, &tangles_r, &objp->orient);

			if(objp->dock_list && objp->dock_list->docked_objp->type == OBJ_SHIP && Ai_info[Ships[objp->dock_list->docked_objp->instance].ai_index].submode == AIS_DOCK_4) {
				Physics_viewer_bank -= tangles.xyz.z*0.65f;
			} else {
				Physics_viewer_bank -= tangles.xyz.z;
			}

			if ( Physics_viewer_bank < 0.0f ){
				Physics_viewer_bank += 2.0f * PI; 	 
			} 	 

			if ( Physics_viewer_bank > 2.0f * PI ){ 	 
				Physics_viewer_bank -= 2.0f * PI; 	 
			}
		}
	}

	if (!cmeasure_list.empty())
		find_homing_object_cmeasures(cmeasure_list);	//	If any cmeasures are active, maybe steer away homing missiles

	// do pre-collision stuff for beam weapons
	beam_move_all_pre();

	if ( Collisions_enabled ) {
		TRACE_SCOPE(tracing::CollisionDetection);
		obj_sort_and_collide();
	}

	turret_swarm_check_validity();

	// do post-collision stuff for beam weapons
	beam_move_all_post();

	// Cyborg17 - Update the multi record on multi with these new positions. Clients need to get updated, too.
	if (MULTIPLAYER_MASTER) {
		multi_ship_record_update_all();
	}

	// update artillery locking info now
	ship_update_artillery_lock();

	if (Nmodel_instance_num >= 0) {
		animation::ModelAnimation::stepAnimations(frametime, model_get_instance(Nmodel_instance_num));
	}

	if (Viewer_obj && Viewer_obj->type == OBJ_SHIP && Viewer_obj->instance >= 0) {
		ship* shipp = &Ships[Viewer_obj->instance];
		if (shipp->cockpit_model_instance >= 0) {
			animation::ModelAnimation::stepAnimations(frametime, model_get_instance(shipp->cockpit_model_instance));
		}
	}

//	mprintf(("moved all objects\n"));
}


MONITOR( NumObjectsRend )

/**
 * Render an object.  Calls one of several routines based on type
 */
extern int Cmdline_dis_weapons;

void obj_render(object *obj)
{
	model_draw_list render_list;

	obj_queue_render(obj, &render_list);

	render_list.init_render();
	render_list.render_all();

	gr_zbias(0);
	gr_set_cull(0);
	gr_zbuffer_set(ZBUFFER_TYPE_READ);
	gr_set_fill_mode(GR_FILL_MODE_SOLID);

	gr_clear_states();

	gr_reset_lighting();
}

void raw_pof_render(object* obj, model_draw_list* scene) {
	model_render_params render_info;

	auto pof_obj = Pof_objects[obj->instance];

	uint64_t render_flags = MR_NORMAL | MR_IS_MISSILE | MR_NO_BATCH;

	if (pof_obj.flags[Object::Raw_Pof_Flags::Render_without_light])
		render_flags |= MR_NO_LIGHTING;

	if (pof_obj.flags[Object::Raw_Pof_Flags::Glowmaps_disabled]) {
		render_flags |= MR_NO_GLOWMAPS;
	}

	if (pof_obj.flags[Object::Raw_Pof_Flags::Draw_as_wireframe]) {
		render_flags |= MR_SHOW_OUTLINE_HTL | MR_NO_POLYS | MR_NO_TEXTURING;
		render_info.set_color(Wireframe_color);
	}

	if (pof_obj.flags[Object::Raw_Pof_Flags::Render_full_detail]) {
		render_flags |= MR_FULL_DETAIL;
	}

	uint debug_flags = render_info.get_debug_flags();

	if (pof_obj.flags[Object::Raw_Pof_Flags::Render_without_diffuse]) {
		debug_flags |= MR_DEBUG_NO_DIFFUSE;
	}

	if (pof_obj.flags[Object::Raw_Pof_Flags::Render_without_glowmap]) {
		debug_flags |= MR_DEBUG_NO_GLOW;
	}

	if (pof_obj.flags[Object::Raw_Pof_Flags::Render_without_normalmap]) {
		debug_flags |= MR_DEBUG_NO_NORMAL;
	}

	if (pof_obj.flags[Object::Raw_Pof_Flags::Render_without_ambientmap]) {
		debug_flags |= MR_DEBUG_NO_AMBIENT;
	}

	if (pof_obj.flags[Object::Raw_Pof_Flags::Render_without_specmap]) {
		debug_flags |= MR_DEBUG_NO_SPEC;
	}

	if (pof_obj.flags[Object::Raw_Pof_Flags::Render_without_reflectmap]) {
		debug_flags |= MR_DEBUG_NO_REFLECT;
	}

	render_info.set_object_number(OBJ_INDEX(obj));

	render_info.set_flags(render_flags);
	render_info.set_debug_flags(debug_flags);

	model_render_queue(&render_info, scene, Pof_objects[obj->instance].model_num, &obj->orient, &obj->pos);
}

void obj_queue_render(object* obj, model_draw_list* scene)
{
	TRACE_SCOPE(tracing::QueueRender);

	if ( obj->flags[Object::Object_Flags::Should_be_dead] ) return;

	if (scripting::hooks::OnObjectRender->isActive()) {
		scripting::api::Current_scene = scene;

		auto param_list = scripting::hook_param_list(scripting::hook_param("Self", 'o', obj));

		// Always execute the hook content
		bool skip_render = scripting::hooks::OnObjectRender->isOverride(scripting::hooks::ObjectDrawConditions{ obj }, param_list);
		scripting::hooks::OnObjectRender->run(scripting::hooks::ObjectDrawConditions{ obj }, param_list);

		// Clear the render scene context
		scripting::api::Current_scene = nullptr;

		if (skip_render) {
			// Script said that it want's to skip rendering
			return;
		}
	}

	switch ( obj->type ) {
	case OBJ_NONE:
#ifndef NDEBUG
		mprintf(( "ERROR!!!! Bogus obj " PTRDIFF_T_ARG " is rendering!\n", obj-Objects ));
		Int3();
#endif
		break;
	case OBJ_WEAPON:
		if ( Cmdline_dis_weapons ) return;
		weapon_render(obj, scene);
		break;
	case OBJ_SHIP:
		ship_render(obj, scene);
		break;
	case OBJ_FIREBALL:
		fireball_render(obj, scene);
		break;
	case OBJ_SHOCKWAVE:
		shockwave_render(obj, scene);
		break;
	case OBJ_DEBRIS:
		debris_render(obj, scene);
		break;
	case OBJ_ASTEROID:
		asteroid_render(obj, scene);
		break;
	case OBJ_JUMP_NODE:
		for ( SCP_list<CJumpNode>::iterator jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp ) {
			if ( jnp->GetSCPObject() != obj ) {
				continue;
			}

			jnp->Render(scene, &obj->pos, &Eye_position);
		}
		break;
	case OBJ_WAYPOINT:
		// 		if (Show_waypoints)	{
		// 			gr_set_color( 128, 128, 128 );
		// 			g3_draw_sphere_ez( &obj->pos, 5.0f );
		// 		}
		break;
	case OBJ_GHOST:
		break;
	case OBJ_BEAM:
		break;
	case OBJ_RAW_POF:
		raw_pof_render(obj, scene);
		break;
	default:
		Error( LOCATION, "Unhandled obj type %d in obj_render", obj->type );
	}
}

void obj_init_all_ships_physics()
{
	object	*objp;

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->type == OBJ_SHIP)
			physics_ship_init(objp);
	}

}

void obj_observer_move(float frame_time)
{
	object *objp;
	float ft;

	// if i'm not in multiplayer, or not an observer, bail
	if(!(Game_mode & GM_MULTIPLAYER) || (Net_player == NULL) || !(Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type != OBJ_OBSERVER)){
		return;
	}

	objp = Player_obj;

	objp->last_pos = objp->pos;
	objp->last_orient = objp->orient;		// save the orientation -- useful in multiplayer.

	ft = flFrametime;
	obj_move_call_physics( objp, ft );
	obj_move_all_post(objp, frame_time);
}

/**
 * Returns a vector of the average position of all ships in the mission.
 */
void obj_get_average_ship_pos( vec3d *pos )
{
	int count;

	vm_vec_zero( pos );

   // average up all ship positions
	count = 0;
	for (auto so: list_range(&Ship_obj_list)) {
		auto objp = &Objects[so->objnum];
		if (objp->flags[Object::Object_Flags::Should_be_dead])
			continue;

		vm_vec_add2( pos, &objp->pos );
		count++;
	}

	if ( count )
		vm_vec_scale( pos, 1.0f/(float)count );
}

/**
 * Return the team for the object passed as a parameter
 *
 * @param objp Pointer to object that you want team for
 *
 * @return enumerated team on success
 * @return -1 on failure (for objects that don't have teams)
 */
int obj_team(object *objp)
{
	Assert( objp != NULL );
	int team = -1;

	switch ( objp->type ) {
		case OBJ_SHIP:
			Assert( objp->instance >= 0 && objp->instance < MAX_SHIPS );
			team = Ships[objp->instance].team;
			break;

		case OBJ_DEBRIS:
			team = debris_get_team(objp);
			Assertion(team != -1, "Obj_team called for a debris object with no team.");
			break;

/*		case OBJ_CMEASURE:
			Assert( objp->instance >= 0 && objp->instance < MAX_CMEASURES);
			team = Cmeasures[objp->instance].team;
			break;
*/
		case OBJ_WEAPON:
			Assert( objp->instance >= 0 && objp->instance < MAX_WEAPONS );
			team = Weapons[objp->instance].team;
			break;

		case OBJ_JUMP_NODE:
			team = Player_ship->team;
			break;
					
		case OBJ_FIREBALL:
		case OBJ_WAYPOINT:
		case OBJ_START:
		case OBJ_NONE:
		case OBJ_GHOST:
		case OBJ_SHOCKWAVE:		
		case OBJ_BEAM:
		case OBJ_RAW_POF:
			team = -1;
			break;

		case OBJ_ASTEROID:
			team = Iff_traitor;
			break;

		default:
			Int3();	// can't happen
			break;
	} // end switch

	Assertion(team != -1, "Obj_team called for a object of type %s with no team.",  Object_type_names[objp->type]);
	return team;
}

/**
 * Removes any occurances of object 'a' from
 * the pairs list.
 */
extern int Num_pairs;

/**
 * Reset all collisions
 */
void obj_reset_all_collisions()
{
	// clear checkobjects
#ifndef NDEBUG
    for (int i = 0; i < MAX_OBJECTS; ++i) {
        CheckObjects[i] = checkobject();
    }
#endif

	// clear object pairs
	obj_reset_colliders();

	// now add every object back into the object collision pairs
	for (auto moveup: list_range(&obj_used_list)) {
		if (moveup->flags[Object::Object_Flags::Should_be_dead])
			continue;

		// he's not in the collision list
		moveup->flags.set(Object::Object_Flags::Not_in_coll);

		// recalc pairs for this guy
		obj_add_collider(OBJ_INDEX(moveup));
	}
}

// Goober5000
int object_is_docked(object *objp)
{
	return (objp->dock_list != NULL);
}

// Goober5000
int object_is_dead_docked(object *objp)
{
	return (objp->dead_dock_list != NULL);
}

/**
 * Makes an object start 'gliding'
 *
 * It will continue on the same velocity that it was going,
 * regardless of orientation -WMC
 */
void object_set_gliding(object *objp, bool enable, bool force)
{
	Assert(objp != NULL);

	if(enable) {
		if (!force) {
			objp->phys_info.flags |= PF_GLIDING;
		} else {
			objp->phys_info.flags |= PF_FORCE_GLIDE;
		}
	} else {
		if (!force) {
			objp->phys_info.flags &= ~PF_GLIDING;
		} else {
			objp->phys_info.flags &= ~PF_FORCE_GLIDE;
		}
		vm_vec_rotate(&objp->phys_info.prev_ramp_vel, &objp->phys_info.vel, &objp->orient);	//Backslash
	}
}

/**
 * @return whether an object is gliding -WMC
 */
bool object_get_gliding(object *objp)
{
	Assert(objp != NULL);

	return ( ((objp->phys_info.flags & PF_GLIDING) != 0) || ((objp->phys_info.flags & PF_FORCE_GLIDE) != 0));
}

bool object_glide_forced(object *objp)
{
	return (objp->phys_info.flags & PF_FORCE_GLIDE) != 0;
}

/**
 * Quickly finds an object by its signature
 */
int obj_get_by_signature(int sig)
{
	Assert(sig > 0);

	for (auto objp: list_range(&obj_used_list))
	{
		// don't skip over should-be-dead objects, since we assume we know what we're doing
		if (objp->signature == sig)
			return OBJ_INDEX(objp);
	}

	return -1;
}

/**
 * Gets the model number for this object, or -1 if none
 */
int object_get_model_num(const object *objp)
{
	switch(objp->type)
	{
		case OBJ_ASTEROID:
		{
			asteroid *asp = &Asteroids[objp->instance];
			return Asteroid_info[asp->asteroid_type].subtypes[asp->asteroid_subtype].model_number;
		}
		case OBJ_DEBRIS:
		{
			debris *debrisp = &Debris[objp->instance];
			return debrisp->model_num;
		}
		case OBJ_SHIP:
		{
			ship *shipp = &Ships[objp->instance];
			return Ship_info[shipp->ship_info_index].model_num;
		}
		case OBJ_WEAPON:
		{
			weapon *wp = &Weapons[objp->instance];
			return Weapon_info[wp->weapon_info_index].model_num;
		}
		case OBJ_RAW_POF:
			return Pof_objects[objp->instance].model_num;
		default:
			break;
	}

	return -1;
}

/**
 * Gets the model for this object, or nullptr if none
 */
polymodel *object_get_model(const object *objp)
{
	int model_num = object_get_model_num(objp);
	return model_num >= 0 ? model_get(model_num) : nullptr;
}

/**
 * Gets the model instance number for this object, or -1 if none
 */
int object_get_model_instance_num(const object *objp)
{
	if (objp == nullptr)
		return -1;

	switch (objp->type)
	{
		case OBJ_ASTEROID:
		{
			asteroid *asp = &Asteroids[objp->instance];
			return asp->model_instance_num;
		}
		case OBJ_DEBRIS:
		{
			debris *debrisp = &Debris[objp->instance];
			return debrisp->model_instance_num;
		}
		case OBJ_SHIP:
		{
			ship *shipp = &Ships[objp->instance];
			return shipp->model_instance_num;
		}
		case OBJ_WEAPON:
		{
			weapon *wp = &Weapons[objp->instance];
			return wp->model_instance_num;
		}
		case OBJ_JUMP_NODE:
		{
			CJumpNode* jnp = jumpnode_get_by_objnum(OBJ_INDEX(objp));
			Assertion(jnp != nullptr, "Could not find jump node!");
			return jnp->GetPolymodelInstanceNum();
		}
		case OBJ_RAW_POF:
			return Pof_objects[objp->instance].model_instance;
		default:
			break;
	}

	return -1;
}

/**
 * Gets the model instance for this object, or nullptr if none
 */
polymodel_instance *object_get_model_instance(const object *objp)
{
	int model_instance_num = object_get_model_instance_num(objp);
	return model_instance_num >= 0 ? model_get_instance(model_instance_num) : nullptr;
}

bool obj_compare(object* left, object* right) {
	if (left == right) {
		// Same pointer
		return true;
	}
	if (left == nullptr || right == nullptr) {
		// Only one is nullptr and the other is not (since they are not equal)
		return false;
	}

	return OBJ_INDEX(left) == OBJ_INDEX(right);
}

void physics_populate_snapshot(physics_snapshot& snapshot, const object* objp)
{
    Assertion(objp != nullptr, "Bad object (nullptr) passed to physics_overwrite_snapshot, please report to the SCP!");

    snapshot.position = objp->pos;
    snapshot.orientation = objp->orient;
    snapshot.velocity = objp->phys_info.vel;
    snapshot.desired_velocity = objp->phys_info.desired_vel;
    snapshot.rotational_velocity = objp->phys_info.rotvel;
    snapshot.desired_rotational_velocity = objp->phys_info.desired_rotvel;
}

void physics_apply_pstate_to_object(object* objp, const physics_snapshot& source)
{
    Assertion(objp != nullptr, "Bad object passed to phsyics snapshot application code.  This is a coder mistake, please report!");

    objp->pos = source.position;
    objp->orient = source.orientation;
    objp->phys_info.vel = source.velocity;
    objp->phys_info.desired_vel = source.desired_velocity;
    objp->phys_info.rotvel = source.rotational_velocity;
    objp->phys_info.desired_rotvel = source.desired_rotational_velocity;
}

