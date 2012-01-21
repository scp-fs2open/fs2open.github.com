/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "object/object.h"
#include "weapon/weapon.h"
#include "ship/ship.h"
#include "fireball/fireballs.h"
#include "debris/debris.h"
#include "globalincs/linklist.h"
#include "freespace2/freespace.h"
#include "object/objectsnd.h"
#include "playerman/player.h"
#include "cmeasure/cmeasure.h"
#include "io/timer.h"
#include "render/3d.h"
#include "weapon/shockwave.h"
#include "ship/afterburner.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "object/objcollide.h"
#include "object/objectshield.h"
#include "lighting/lighting.h"
#include "observer/observer.h"
#include "parse/scripting.h"
#include "asteroid/asteroid.h"
#include "radar/radar.h"
#include "jumpnode/jumpnode.h"
#include "weapon/beam.h"
#include "weapon/swarm.h"
#include "radar/radarsetup.h"
#include "object/objectdock.h"
#include "mission/missionparse.h" //For 2D Mode
#include "iff_defs/iff_defs.h"



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

#ifdef OBJECT_CHECK 
typedef struct checkobject
{
	int	type;
	int	signature;
	uint	flags;
	int	parent_sig;
	int	parent_type;
} checkobject;
checkobject CheckObjects[MAX_OBJECTS];
#endif

int Num_objects=-1;
int Highest_object_index=-1;
int Highest_ever_object_index=0;
int Object_next_signature = 1;	//0 is bogus, start at 1
int Object_inited = 0;
int Show_waypoints = 0;

//WMC - Made these prettier
char *Object_type_names[MAX_OBJECT_TYPES] = {
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
//XSTR:ON
};

//-----------------------------------------------------------------------------
//	Scan the object list, freeing down to num_used objects
//	Returns number of slots freed.
int free_object_slots(int num_used)
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

	if (MAX_OBJECTS - num_already_free < num_used)
		return 0;

	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->flags & OF_SHOULD_BE_DEAD) {
			num_already_free++;
			if (MAX_OBJECTS - num_already_free < num_used)
				return num_already_free;
		} else
			switch (objp->type) {
				case OBJ_NONE:
					num_already_free++;
					if (MAX_OBJECTS - num_already_free < num_used)
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
					break;
				default:
					Int3();	//	Hey, what kind of object is this?  Unknown!
					break;
			}

	}

	num_to_free = MAX_OBJECTS - num_used - num_already_free;
	original_num_to_free = num_to_free;

	if (num_to_free > olind) {
		nprintf(("allender", "Warning: Asked to free %i objects, but can only free %i.\n", num_to_free, olind));
		num_to_free = olind;
	}

	for (i=0; i<num_to_free; i++)
		if ( (Objects[obj_list[i]].type == OBJ_DEBRIS) && (Debris[Objects[obj_list[i]].instance].flags & DEBRIS_EXPIRE) ) {
			num_to_free--;
			nprintf(("allender", "Freeing   DEBRIS object %3i\n", obj_list[i]));
			Objects[obj_list[i]].flags |= OF_SHOULD_BE_DEAD;
		}

	if (!num_to_free)
		return original_num_to_free;

//JAS - I removed this because small fireballs are now particles, which aren't objects.
//JAS	for (i=0; i<num_to_free; i++)
//JAS		if ( (Objects[obj_list[i]].type == OBJ_FIREBALL) && (Fireball_data[Objects[obj_list[i]].instance].type == FIREBALL_TYPE_SMALL) ) {
//JAS			num_to_free--;
//JAS			nprintf(("allender", "Freeing FIREBALL object %3i\n", obj_list[i]));
//JAS			Objects[obj_list[i]].flags |= OF_SHOULD_BE_DEAD;
//JAS		}
//JAS
//JAS	if (!num_to_free)
//JAS		return original_num_to_free;

	for (i=0; i<num_to_free; i++)	{
		object *tmp_obj = &Objects[obj_list[i]];
		if ( (tmp_obj->type == OBJ_FIREBALL) && (fireball_is_perishable(tmp_obj)) ) {
			num_to_free--;
			nprintf(("allender", "Freeing FIREBALL object %3i\n", obj_list[i]));
			tmp_obj->flags |= OF_SHOULD_BE_DEAD;
		}
	}

	if (!num_to_free){
		return original_num_to_free;
	}

	deleted_weapons = collide_remove_weapons();
	num_to_free -= deleted_weapons;
	if ( !num_to_free ){
		return original_num_to_free;
	}

	for (i=0; i<num_to_free; i++){
		if ( Objects[obj_list[i]].type == OBJ_WEAPON ) {
			num_to_free--;
			Objects[obj_list[i]].flags |= OF_SHOULD_BE_DEAD;
		}
	}

	if (!num_to_free){
		return original_num_to_free;
	}

	return original_num_to_free - num_to_free;
}

// Goober5000
float get_max_shield_quad(object *objp)
{
	Assert(objp);
	if(objp->type != OBJ_SHIP) {
		return 0.0f;
	}

	return Ships[objp->instance].ship_max_shield_strength / MAX_SHIELD_SECTIONS;
}

// Goober5000
float get_hull_pct(object *objp)
{
	Assert(objp);
	Assert(objp->type == OBJ_SHIP);

	float total_strength = Ships[objp->instance].ship_max_hull_strength;

	Assert(total_strength > 0.0f);	// unlike shield, no ship can have 0 hull

	if (total_strength == 0.0f)
		return 0.0f;

	if (objp->hull_strength < 0.0f)	// this sometimes happens when a ship is being destroyed
		return 0.0f;

	return objp->hull_strength / total_strength;
}

float get_sim_hull_pct(object *objp)
{
	Assert(objp);
	Assert(objp->type == OBJ_SHIP);

	float total_strength = Ships[objp->instance].ship_max_hull_strength;

	Assert(total_strength > 0.0f);	// unlike shield, no ship can have 0 hull

	if (total_strength == 0.0f)
		return 0.0f;

	if (objp->sim_hull_strength < 0.0f)	// this sometimes happens when a ship is being destroyed
		return 0.0f;

	return objp->sim_hull_strength / total_strength;
}

// Goober5000
float get_shield_pct(object *objp)
{
	Assert(objp);

	// bah - we might have asteroids
	if (objp->type != OBJ_SHIP)
		return 0.0f;

	float total_strength = Ships[objp->instance].ship_max_shield_strength;

	if (total_strength == 0.0f)
		return 0.0f;

	return shield_get_strength(objp) / total_strength;
}

//sets up the free list & init player & whatever else
void obj_init()
{
	int i, idx;
	object *objp;
	
	Object_inited = 1;
	memset( Objects, 0, sizeof(object)*MAX_OBJECTS );
	Viewer_obj = NULL;

	list_init( &obj_free_list );
	list_init( &obj_used_list );
	list_init( &obj_create_list );

	// Link all object slots into the free list
	objp = Objects;
	for (i=0; i<MAX_OBJECTS; i++)	{
		objp->type = OBJ_NONE;
		objp->signature = i + 100;
		objp->collision_group_id = 0;

		// zero all object sounds
		for(idx=0; idx<MAX_OBJECT_SOUNDS; idx++){
			objp->objsnd_num[idx] = -1;
		}
		
		list_append(&obj_free_list, objp);
		objp++;
	}

	Object_next_signature = 1;	//0 is invalid, others start at 1
	Num_objects = 0;			
	Highest_object_index = 0;

	obj_reset_pairs();
}

static int num_objects_hwm = 0;

//returns the number of a free object, updating Highest_object_index.
//Generally, obj_create() should be called to get an object, since it
//fills in important fields and does the linking.
//returns -1 if no free objects
int obj_allocate(void)
{
	int objnum;
	object *objp;

	if (!Object_inited) obj_init();

	if ( Num_objects >= MAX_OBJECTS-10 ) {
		int	num_freed;

		num_freed = free_object_slots(MAX_OBJECTS-10);
		nprintf(("warning", " *** Freed %i objects\n", num_freed));
	}

	if (Num_objects >= MAX_OBJECTS) {
		#ifndef NDEBUG
		mprintf(("Object creation failed - too many objects!\n" ));
		#endif
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
		//nprintf(("AI", "*** MAX Num Objects = %i\n", Num_objects));
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

//frees up an object.  Generally, obj_delete() should be called to get
//rid of an object.  This function deallocates the object entry after
//the object has been unlinked
void obj_free(int objnum)
{
	object *objp;

	if (!Object_inited) obj_init();

	Assert( objnum >= 0 );	// Trying to free bogus object!!!

	// get object pointer
	objp = &Objects[objnum];

	// remove objp from the used list
	list_remove( &obj_used_list, objp);

	// add objp to the end of the free
	list_append( &obj_free_list, objp );

	// decrement counter
	Num_objects--;

	Objects[objnum].type = OBJ_NONE;

	Assert(Num_objects >= 0);

	if (objnum == Highest_object_index)
		while (Objects[--Highest_object_index].type == OBJ_NONE);

}

//initialize a new object.  adds to the list for the given segment.
//returns the object number.  The object will be a non-rendering, non-physics
//object.   Pass -1 if no parent.
int obj_create(ubyte type,int parent_obj,int instance, matrix * orient, 
               vec3d * pos, float radius, uint flags )
{
	int objnum,idx;
	object *obj;

	// Find next free object
	objnum = obj_allocate();

	if (objnum == -1)		//no free objects
		return -1;

	obj = &Objects[objnum];
	Assert(obj->type == OBJ_NONE);		//make sure unused 

	// Zero out object structure to keep weird bugs from happening
	// in uninitialized fields.
//	memset( obj, 0, sizeof(object) );

	Assert(Object_next_signature > 0);	// 0 is bogus!
	obj->signature = Object_next_signature++;

	obj->type 					= type;
	obj->instance				= instance;
	obj->parent					= parent_obj;
	if (obj->parent != -1)	{
		obj->parent_sig		= Objects[parent_obj].signature;
		obj->parent_type		= Objects[parent_obj].type;
	} else {
		obj->parent_sig = obj->signature;
		obj->parent_type = obj->type;
	}

	obj->flags 					= flags | OF_NOT_IN_COLL;
	if (pos)	{
		obj->pos 				= *pos;
		obj->last_pos			= *pos;
	}

	obj->orient 				= orient?*orient:vmd_identity_matrix;
	obj->last_orient			= obj->orient;
	obj->radius 				= radius;

	obj->flags &= ~OF_INVULNERABLE;		//	Make vulnerable.
	physics_init( &obj->phys_info );

	for(idx=0; idx<MAX_OBJECT_SOUNDS; idx++){
		obj->objsnd_num[idx] = -1;
	}
	obj->num_pairs = 0;
	obj->net_signature = 0;			// be sure to reset this value so new objects don't take on old signatures.	

	obj->collision_group_id = 0;

	//WMC
	/*
	char buf[NAME_LENGTH];
	sprintf(buf, "Object %d", objnum);
	obj->core_camera = cam_create(buf, &vmd_zero_vector, &vmd_identity_matrix, obj);

	//Top-down camera
	sprintf(buf, "Object %d topdown", objnum);
	vec3d tdv = vmd_zero_vector;
	matrix tdm = vmd_identity_matrix;
	angles rot_angles = { PI_2, 0.0f, 0.0f };
	bool position_override = false;
	if(obj->type == OBJ_SHIP)
	{
		ship_info *sip = &Ship_info[Ships[Viewer_obj->instance].ship_info_index];
		if(sip->topdown_offset_def) {
			tdv.xyz.x = sip->topdown_offset.xyz.x;
			tdv.xyz.y = sip->topdown_offset.xyz.y;
			tdv.xyz.z = sip->topdown_offset.xyz.z;
			position_override = true;
		}
	}
	if(!position_override)
	{
		tdv.xyz.y = radius * 25.0f;
	}
	vm_angles_2_matrix(&tdm, &rot_angles);
	obj->topdown_camera = cam_create(buf, &tdv, &tdm, obj);
	*/


	// Goober5000
	obj->dock_list = NULL;
	obj->dead_dock_list = NULL;

	return objnum;
}

//remove object from the world
//	If Player_obj, don't remove it!
void obj_delete(int objnum)
{
	object *objp;

	Assert(objnum >= 0 && objnum < MAX_OBJECTS);
	objp = &Objects[objnum];
	Assert(objp->type != OBJ_NONE);	

	// Remove all object pairs
	obj_remove_pairs( objp );
	
	switch( objp->type )	{
	case OBJ_WEAPON:
		weapon_delete( objp );
		break;
	case OBJ_SHIP:
		if ((objp == Player_obj) && !Fred_running) {
			objp->type = OBJ_GHOST;
			objp->flags &= ~(OF_SHOULD_BE_DEAD);
			
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
		Assert(Fred_running);
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
			objp->flags &= ~OF_SHOULD_BE_DEAD;
			return;
		} else {
			// we need to be able to delete GHOST objects in multiplayer to allow for player respawns.
			nprintf(("Network","Deleting GHOST object\n"));
		}		
		break;
	case OBJ_OBSERVER:
		observer_delete(objp);
		break;	
	case OBJ_BEAM:
		break;
	case OBJ_NONE:
		Int3();
		break;
	default:
		Error( LOCATION, "Unhandled object type %d in obj_delete_all_that_should_be_dead", objp->type );
	}

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

	if (!Object_inited) obj_init();

	// Move all objects
	objp = GET_FIRST(&obj_used_list);
	while( objp !=END_OF_LIST(&obj_used_list) )	{
		// Goober5000 - HACK HACK HACK - see obj_move_all
		objp->flags &= ~OF_DOCKED_ALREADY_HANDLED;

		temp = GET_NEXT(objp);
		if ( objp->flags&OF_SHOULD_BE_DEAD )
			obj_delete( OBJ_INDEX(objp) );			// MWA says that john says that let obj_delete handle everything because of the editor
		objp = temp;
	}

}

// Add all newly created objects to the end of the used list and create their
// object pairs for collision detection
void obj_merge_created_list(void)
{
	// The old way just merged the two.   This code takes one out of the create list,
	// creates object pairs for it, and then adds it to the used list.
	//	OLD WAY: list_merge( &obj_used_list, &obj_create_list );
	object *objp = GET_FIRST(&obj_create_list);
	while( objp !=END_OF_LIST(&obj_create_list) )	{
		list_remove( obj_create_list, objp );

		// Add it to the object pairs array
		obj_add_pairs(OBJ_INDEX(objp));

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

/*
float	Last_fire_time = 0.0f;
int Avg_delay_count = 0;
float Avg_delay_total;
*/

// function to deal with firing player things like lasers, missiles, etc.
// separated out because of multiplayer issues.
void obj_player_fire_stuff( object *objp, control_info ci )
{
	ship *shipp;

	Assert( objp->flags & OF_PLAYER_SHIP);

	// try and get the ship pointer
	shipp = NULL;
	if((objp->type == OBJ_SHIP) && (objp->instance >= 0) && (objp->instance < MAX_SHIPS)){
		shipp = &Ships[objp->instance];
	}

	// single player pilots, and all players in multiplayer take care of firing their own primaries
	if(!(Game_mode & GM_MULTIPLAYER) || (objp == Player_obj))
	{
		if ( ci.fire_primary_count ) {
			// flag the ship as having the trigger down
			if(shipp != NULL){
				shipp->flags |= SF_TRIGGER_DOWN;
			}

			// fire non-streaming primaries here
			ship_fire_primary( objp, 0 );
	//			ship_stop_fire_primary(objp);	//if it hasn't fired do the "has just stoped fireing" stuff
		} else {
			// unflag the ship as having the trigger down
			if(shipp != NULL){
				shipp->flags &= ~(SF_TRIGGER_DOWN);
				ship_stop_fire_primary(objp);	//if it hasn't fired do the "has just stoped fireing" stuff
			}
		}

		if ( ci.fire_countermeasure_count ) {
			ship_launch_countermeasure( objp );
		}
	} else {
//		ship_stop_fire_primary(objp);
	}

	// single player and multiplayer masters do all of the following
	if ( !MULTIPLAYER_CLIENT ) {		
		if ( ci.fire_secondary_count ) {
			ship_fire_secondary( objp );

			// kill the secondary count
			ci.fire_secondary_count = 0;
		}
	}

	// everyone does the following for their own ships.
	if ( ci.afterburner_start ){
		if (ship_get_subsystem_strength(&Ships[objp->instance], SUBSYSTEM_ENGINE)){
			afterburners_start( objp );
		}
	}
	
	if ( ci.afterburner_stop ){
		afterburners_stop( objp, 1 );
	}
	
}

void obj_move_call_physics(object *objp, float frametime)
{
	int has_fired = -1;	//stop fireing stuff-Bobboau

	//	Do physics for objects with OF_PHYSICS flag set and with some engine strength remaining.
	if ( objp->flags & OF_PHYSICS ) {
		// only set phys info if ship is not dead
		if ((objp->type == OBJ_SHIP) && !(Ships[objp->instance].flags & SF_DYING)) {
			ship *shipp = &Ships[objp->instance];
			float	engine_strength;

			engine_strength = ship_get_subsystem_strength(shipp, SUBSYSTEM_ENGINE);
			if ( ship_subsys_disrupted(shipp, SUBSYSTEM_ENGINE) ) {
				engine_strength=0.0f;
			}

			if (engine_strength == 0.0f) {	//	All this is necessary to make ship gradually come to a stop after engines are blown.
				vm_vec_zero(&objp->phys_info.desired_vel);
				vm_vec_zero(&objp->phys_info.desired_rotvel);
				objp->phys_info.flags |= (PF_REDUCED_DAMP | PF_DEAD_DAMP);
				objp->phys_info.side_slip_time_const = Ship_info[shipp->ship_info_index].damp * 4.0f;
			} // else {
				// DA: comment out lines that resets PF_DEAD_DAMP after every frame.
				// This is now reset during engine repair.
				//	objp->phys_info.flags &= ~(PF_REDUCED_DAMP | PF_DEAD_DAMP);
				// objp->phys_info.side_slip_time_const = Ship_info[shipp->ship_info_index].damp;
			// }

			if (shipp->weapons.num_secondary_banks > 0) {
				polymodel *pm = model_get(Ship_info[shipp->ship_info_index].model_num);
				Assertion( pm != NULL, "No polymodel found for ship %s", Ship_info[shipp->ship_info_index].name );
				Assertion( pm->missile_banks != NULL, "Ship %s has %d secondary banks, but no missile banks could be found.\n", Ship_info[shipp->ship_info_index].name, shipp->weapons.num_secondary_banks );

				for (int i = 0; i < shipp->weapons.num_secondary_banks; i++) {
					//if there are no missles left don't bother
					if (shipp->weapons.secondary_bank_ammo[i] == 0)
						continue;

					int points = pm->missile_banks[i].num_slots;
					int missles_left = shipp->weapons.secondary_bank_ammo[i];
					int next_point = shipp->weapons.secondary_next_slot[i];
					float fire_wait = Weapon_info[shipp->weapons.secondary_bank_weapons[i]].fire_wait;
					float reload_time = (fire_wait == 0.0f) ? 1.0f : 1.0f / fire_wait;

					//ok so...we want to move up missles but only if there is a missle there to be moved up
					//there is a missle behind next_point, and how ever many missles there are left after that

					if (points > missles_left) {
						//there are more slots than missles left, so not all of the slots will have missles drawn on them
						for (int k = next_point; k < next_point+missles_left; k ++) {
							float &s_pct = shipp->secondary_point_reload_pct[i][k % points];
							if (s_pct < 1.0)
								s_pct += reload_time * frametime;
							if (s_pct > 1.0)
								s_pct = 1.0f;
						}
					} else {
						//we don't have to worry about such things
						for (int k = 0; k < points; k++) {
							float &s_pct = shipp->secondary_point_reload_pct[i][k];
							if (s_pct < 1.0)
								s_pct += reload_time * frametime;
							if (s_pct > 1.0)
								s_pct = 1.0f;
						}
					}
				}
			}
		}

		// if a weapon is flagged as dead, kill its engines just like a ship
		if((objp->type == OBJ_WEAPON) && (Weapons[objp->instance].weapon_flags & WF_DEAD_IN_WATER)){
			vm_vec_zero(&objp->phys_info.desired_vel);
			vm_vec_zero(&objp->phys_info.desired_rotvel);
			objp->phys_info.flags |= (PF_REDUCED_DAMP | PF_DEAD_DAMP);
			objp->phys_info.side_slip_time_const = 1.0f;	// FIXME?  originally indexed into Ship_info[], which was a bug...
		}

		if (physics_paused)	{
			if (objp==Player_obj){
				physics_sim(&objp->pos, &objp->orient, &objp->phys_info, frametime );		// simulate the physics
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
					if (ship_get_subsystem_strength(&Ships[objp->instance], SUBSYSTEM_ENGINE) > 0.0f){
						objp->phys_info.flags |= PF_USE_VEL;
					} else {
						objp->phys_info.flags &= ~PF_USE_VEL;	//	If engine blown, don't PF_USE_VEL, or ships stop immediately
					}
				} else {
					objp->phys_info.flags &= ~PF_USE_VEL;
				}
			}			

			// in multiplayer, if this object was just updatd (i.e. clients send their own positions),
			// then reset the flag and don't move the object.
			if ( MULTIPLAYER_MASTER && (objp->flags & OF_JUST_UPDATED) ) {
				objp->flags &= ~OF_JUST_UPDATED;
				goto obj_maybe_fire;
			}

	//		if ( (objp->type == OBJ_ASTEROID) && (Model_caching && (!D3D_enabled && !OGL_enabled) ) )	{
	//			// If we're doing model caching, don't rotate asteroids
	//			vec3d tmp = objp->phys_info.rotvel;
	//
	//			objp->phys_info.rotvel = vmd_zero_vector;
	//			physics_sim(&objp->pos, &objp->orient, &objp->phys_info, frametime );		// simulate the physics
	//			objp->phys_info.rotvel = tmp;
	//		} else {
				physics_sim(&objp->pos, &objp->orient, &objp->phys_info, frametime );		// simulate the physics
	//		}

			// This code seems to have no effect - DB 1/12/99
			//if ( MULTIPLAYER_CLIENT && (objp != Player_obj) ){
			//	return;
			//}

			// if the object is the player object, do things that need to be done after the ship
			// is moved (like firing weapons, etc).  This routine will get called either single
			// or multiplayer.  We must find the player object to get to the control info field
obj_maybe_fire:
			if ( (objp->flags & OF_PLAYER_SHIP) && (objp->type != OBJ_OBSERVER) && (objp == Player_obj)) {
				player *pp;
				if(Player != NULL){
//					has_fired = 1;
					pp = Player;
					obj_player_fire_stuff( objp, pp->ci );				
				}
			}

			// fire streaming weapons for ships in here - ALL PLAYERS, regardless of client, single player, server, whatever.
			// do stream weapon firing for all ships themselves. 
			if(objp->type == OBJ_SHIP){
				ship_fire_primary(objp, 1, 0);
//				if(ship_fire_primary(objp, 1, 0) < 1){
					has_fired = 1;
//				}else{
//					has_fired = -1;
//				}
			}
		}
	}
	
	if(has_fired == -1){
	//	mprintf(("stoped 1\n"));
		ship_stop_fire_primary(objp);	//if it hasn't fired do the "has just stoped fireing" stuff
	}

	//2D MODE
	//THIS IS A FREAKIN' HACK
	//Do not let ship change position on Y axis
	if(The_mission.flags & MISSION_FLAG_2D_MISSION)
	{
		angles old_angles, new_angles;
		objp->pos.xyz.y = objp->last_pos.xyz.y;
		vm_extract_angles_matrix(&old_angles, &objp->last_orient);
		vm_extract_angles_matrix(&new_angles, &objp->orient);
		new_angles.p = old_angles.p;
		new_angles.b = old_angles.b;
		//new_angles.h = old_angles.h;
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


#define IMPORTANT_FLAGS (OF_COLLIDES)

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
	if ( (CheckObjects[objnum].flags&IMPORTANT_FLAGS) != (obj->flags&IMPORTANT_FLAGS) ) {
		mprintf(( "Object flags changed!\n" ));
		Int3();
	}
	if ( CheckObjects[objnum].parent_sig != obj->parent_sig ) {
		mprintf(( "Object parent sig changed!\n" ));
		Int3();
	}
	if ( CheckObjects[objnum].parent_type != obj->parent_type ) {
		mprintf(( "Object's parent type changed!\n" ));
		Int3();
	}
}
#endif

// Call this if you want to change an object flag so that the
// object code knows what's going on.  For instance if you turn
// off OF_COLLIDES, the object code needs to know this in order to
// actually turn the object collision detection off.  By calling
// this you shouldn't get Int3's in the checkobject code.  If you
// do, then put code in here to correctly handle the case.
void obj_set_flags( object *obj, uint new_flags )
{
	int objnum = OBJ_INDEX(obj);	

	// turning collision detection off
	if ( (obj->flags & OF_COLLIDES) && (!(new_flags&OF_COLLIDES)))	{		
		// Remove all object pairs
		obj_remove_pairs( obj );

		// update object flags properly		
		obj->flags = new_flags;
		obj->flags |= OF_NOT_IN_COLL;		
#ifdef OBJECT_CHECK
		CheckObjects[objnum].flags = new_flags;
		CheckObjects[objnum].flags |= OF_NOT_IN_COLL;		
#endif		
		return;
	}
	
	
	// turning collision detection on
	if ( (!(obj->flags & OF_COLLIDES)) && (new_flags&OF_COLLIDES) )	{
		
		// observers can't collide or be hit, and they therefore have no hit or collide functions
		// So, don't allow this bit to be set
		if(obj->type == OBJ_OBSERVER){
			mprintf(("Illegal to set collision bit for OBJ_OBSERVER!!\n"));
			Int3();
		}

		obj->flags |= OF_COLLIDES;

		// Turn on collision detection
		obj_add_pairs(objnum);
				
		obj->flags = new_flags;
		obj->flags &= ~(OF_NOT_IN_COLL);		
#ifdef OBJECT_CHECK
		CheckObjects[objnum].flags = new_flags;
		CheckObjects[objnum].flags &= ~(OF_NOT_IN_COLL);		
#endif
		return;
	}

	// for a multiplayer host -- use this debug code to help trap when non-player ships are getting
	// marked as OF_COULD_BE_PLAYER
	// this code is pretty much debug code and shouldn't be relied on to always do the right thing
	// for flags other than 
	if ( MULTIPLAYER_MASTER && !(obj->flags & OF_COULD_BE_PLAYER) && (new_flags & OF_COULD_BE_PLAYER) ) {
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
	if ( (new_flags&IMPORTANT_FLAGS) != (obj->flags&IMPORTANT_FLAGS) ) {
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
			asteroid_process_pre(objp,frametime);
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
	case OBJ_NONE:
		Int3();
		break;
	default:
		Error( LOCATION, "Unhandled object type %d in obj_move_one\n", objp->type );
	}	
}

// Used to tell if a particular group of lasers has cast light yet.
ubyte Obj_weapon_group_id_used[WEAPON_MAX_GROUP_IDS];

// Called once a frame to mark all weapon groups as not having cast light yet.
void obj_clear_weapon_group_id_list()
{
	memset( Obj_weapon_group_id_used, 0, sizeof(Obj_weapon_group_id_used) );
}

int Arc_light = 1;		// If set, electrical arcs on debris cast light
DCF_BOOL(arc_light, Arc_light)	
extern fireball Fireballs[];

void obj_move_all_post(object *objp, float frametime)
{
	switch (objp->type)
	{
		case OBJ_WEAPON:
		{
			if ( !physics_paused )
				weapon_process_post( objp, frametime );

			// Cast light
			if ( Detail.lighting > 2 ) {
				// Weapons cast light

				int group_id = Weapons[objp->instance].group_id;
				int cast_light = 1;

				if ( (group_id >= 0) && (Obj_weapon_group_id_used[group_id]==0) )	{
					// Mark this group as done
					Obj_weapon_group_id_used[group_id]++;
				} else {
					// This group has already done its light casting
					cast_light = 0;
				}

				if ( cast_light )	{
					weapon_info * wi = &Weapon_info[Weapons[objp->instance].weapon_info_index];

					if ( wi->render_type == WRT_LASER )	{
						color c;
						float r,g,b;

						// get the laser color
						weapon_get_laser_color(&c, objp);

						r = i2fl(c.red)/255.0f;
						g = i2fl(c.green)/255.0f;
						b = i2fl(c.blue)/255.0f;

						light_add_point( &objp->pos, 10.0f, 20.0f, 1.0f, r, g, b, objp->parent );
						//light_add_point( &objp->pos, 10.0f, 20.0f, 1.0f, 0.0f, 0.0f, 1.0f, objp->parent );
					} else {
						light_add_point( &objp->pos, 10.0f, 20.0f, 1.0f, 1.0f, 1.0f, 1.0f, objp->parent );
					} 
				}
			}

			break;
		}

		case OBJ_SHIP:
		{
			if ( !physics_paused || (objp==Player_obj) ) {
				ship_process_post( objp, frametime );
				ship_model_update_instance(objp);
			}

			// Make any electrical arcs on ships cast light
			if (Arc_light)	{
				if ( (Detail.lighting > 2) && (objp != Viewer_obj) ) {
					int i;
					ship		*shipp;
					shipp = &Ships[objp->instance];

					for (i=0; i<MAX_SHIP_ARCS; i++ )	{
						if ( timestamp_valid( shipp->arc_timestamp[i] ) )	{
							// Move arc endpoints into world coordinates	
							vec3d tmp1, tmp2;
							vm_vec_unrotate(&tmp1,&shipp->arc_pts[i][0],&objp->orient);
							vm_vec_add2(&tmp1,&objp->pos);

							vm_vec_unrotate(&tmp2,&shipp->arc_pts[i][1],&objp->orient);
							vm_vec_add2(&tmp2,&objp->pos);

							light_add_point( &tmp1, 10.0f, 20.0f, frand(), 1.0f, 1.0f, 1.0f, -1 );
							light_add_point( &tmp2, 10.0f, 20.0f, frand(), 1.0f, 1.0f, 1.0f, -1 );
						}
					}
				}
			}		

			break;
		}

		case OBJ_FIREBALL:
		{
			if ( !physics_paused )
				fireball_process_post(objp,frametime);

			if (Detail.lighting > 3) {
				float r = 0.0f, g = 0.0f, b = 0.0f;

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
					if(fireball_is_warp(objp))
					{
						intensity = fireball_wormhole_intensity(objp); // Valathil: Get wormhole radius for lighting
						rad = objp->radius;
					}
					// P goes from 0 to 1 to 0 over the life of the explosion
					// Only do this if rad is > 0.0000001f
					if (rad > 0.0001f)
						light_add_point( &objp->pos, rad * 2.0f, rad * 5.0f, intensity, r, g, b, -1 );
				}
			}

			break;
		}

		case OBJ_SHOCKWAVE:
			// all shockwaves are moved via shockwave_move_all()
			break;

		case OBJ_DEBRIS:
		{
			if ( !physics_paused )
				debris_process_post(objp, frametime);

			// Make any electrical arcs on debris cast light
			if (Arc_light)	{
				if ( Detail.lighting > 2 ) {
					int i;
					debris		*db;
					db = &Debris[objp->instance];

					if (db->arc_frequency > 0) {
						for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
							if ( timestamp_valid( db->arc_timestamp[i] ) )	{
								// Move arc endpoints into world coordinates	
								vec3d tmp1, tmp2;
								vm_vec_unrotate(&tmp1,&db->arc_pts[i][0],&objp->orient);
								vm_vec_add2(&tmp1,&objp->pos);

								vm_vec_unrotate(&tmp2,&db->arc_pts[i][1],&objp->orient);
								vm_vec_add2(&tmp2,&objp->pos);

								light_add_point( &tmp1, 10.0f, 20.0f, frand(), 1.0f, 1.0f, 1.0f, -1 );
								light_add_point( &tmp2, 10.0f, 20.0f, frand(), 1.0f, 1.0f, 1.0f, -1 );
							}
						}
					}
				}
			}

			break;
		}

		case OBJ_ASTEROID:
		{
			if ( !physics_paused )
				asteroid_process_post(objp, frametime);

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

		case OBJ_NONE:
			Int3();
			break;

		default:
			Error( LOCATION, "Unhandled object type %d in obj_move_one\n", objp->type );
	}	
}


int Collisions_enabled = 1;

DCF_BOOL( collisions, Collisions_enabled )

MONITOR( NumObjects )

//--------------------------------------------------------------------
//move all objects for the current frame
void obj_move_all(float frametime)
{
	object *objp;	

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
		if (objp->flags & OF_SHOULD_BE_DEAD) {
			continue;
		}

		// if this is an observer object, skip it
		if (objp->type == OBJ_OBSERVER) {
			continue;
		}

		vec3d cur_pos = objp->pos;			// Save the current position

#ifdef OBJECT_CHECK 
		// if(! ((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)) ){
			obj_check_object( objp );
		// }
#endif

		// pre-move
		obj_move_all_pre(objp, frametime);

		// store last pos and orient
		objp->last_pos = cur_pos;
		objp->last_orient = objp->orient;

		// Goober5000 - skip objects which don't move, but only until they're destroyed
		if (!(objp->flags & OF_IMMOBILE && objp->hull_strength > 0.0f)) {
			// if this is an object which should be interpolated in multiplayer, do so
			if (multi_oo_is_interp_object(objp)) {
				multi_oo_interp(objp);
			} else {
				// physics
				obj_move_call_physics(objp, frametime);
			}
		}

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

			Script_system.SetHookObjects(2, "User", objp, "Target", target);
			Script_system.RunCondition(CHA_ONWPEQUIPPED, 0, NULL, objp);
		}
	}

	//	After all objects have been moved, move all docked objects.
	objp = GET_FIRST(&obj_used_list);
	while( objp !=END_OF_LIST(&obj_used_list) )	{
		dock_move_docked_objects(objp);

		//Valathil - Move the screen rotation calculation for billboards here to get the updated orientation matrices caused by docking interpolation
		angles tangles;
		vm_extract_angles_matrix(&tangles, &objp->orient);

		// If this is the viewer_object, keep track of the
		// changes in banking so that rotated bitmaps look correct.
		// This is used by the g3_draw_rotated_bitmap function.
		extern physics_info *Viewer_physics_info;
		extern int Physics_viewer_direction;
		if ( &objp->phys_info == Viewer_physics_info )	{
			switch(Physics_viewer_direction){
			case PHYSICS_VIEWER_FRONT:
				Physics_viewer_bank = -tangles.b;
				break;

			case PHYSICS_VIEWER_UP:
				Physics_viewer_bank = -tangles.h;
				break;

			case PHYSICS_VIEWER_REAR:
				Physics_viewer_bank = tangles.b;
				break;

			case PHYSICS_VIEWER_LEFT:
				Physics_viewer_bank = tangles.p;
				break;

			case PHYSICS_VIEWER_RIGHT:
				Physics_viewer_bank = -tangles.p;
				break;

			default:
				Physics_viewer_bank = -tangles.b;
				break;
			}
		}

		// unflag all objects as being updates
		objp->flags &= ~OF_JUST_UPDATED;

		objp = GET_NEXT(objp);
	}

	find_homing_object_cmeasures();	//	If any cmeasures fired, maybe steer away homing missiles	

	// do pre-collision stuff for beam weapons
	beam_move_all_pre();

	if ( Collisions_enabled ) {
		obj_check_all_collisions();		
	}

	turret_swarm_check_validity();

	// do post-collision stuff for beam weapons
	beam_move_all_post();

	// update artillery locking info now
	ship_update_artillery_lock();

//	mprintf(("moved all objects\n"));
}


MONITOR( NumObjectsRend )

// -----------------------------------------------------------------------------
//	Render an object.  Calls one of several routines based on type
extern int Cmdline_dis_weapons;
void obj_render(object *obj)
{
	SCP_list<jump_node>::iterator jnp;
	
	if ( obj->flags & OF_SHOULD_BE_DEAD ) return;

	MONITOR_INC( NumObjectsRend, 1 );	

	//WMC - By definition, override statements are executed before the actual statement
	Script_system.SetHookObject("Self", obj);
	if(!Script_system.IsConditionOverride(CHA_OBJECTRENDER, obj))
	{
		switch( obj->type )	{
		case OBJ_NONE:
			#ifndef NDEBUG
			mprintf(( "ERROR!!!! Bogus obj %d is rendering!\n", obj-Objects ));
			Int3();
			#endif
			break;
		case OBJ_WEAPON:
			if(Cmdline_dis_weapons) return;
			weapon_render(obj);
			break;
		case OBJ_SHIP:
			ship_render(obj);
			break;
		case OBJ_FIREBALL:
			fireball_render(obj);
			break;
		case OBJ_SHOCKWAVE:
			shockwave_render(obj);
			break;
		case OBJ_DEBRIS:
			debris_render(obj);
			break;
		case OBJ_ASTEROID:
			asteroid_render(obj);
			break;
	/*	case OBJ_CMEASURE:
			cmeasure_render(obj);
			break;*/
		case OBJ_JUMP_NODE:
			for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
				if(jnp->get_obj() != obj)
					continue;
				jnp->render(&obj->pos, &Eye_position);
			}
			break;
		case OBJ_WAYPOINT:
			if (Show_waypoints)	{
				gr_set_color( 128, 128, 128 );
				g3_draw_sphere_ez( &obj->pos, 5.0f );
			}
			break;
		case OBJ_GHOST:
			break;
		case OBJ_BEAM:
			break;
		default:
			Error( LOCATION, "Unhandled obj type %d in obj_render", obj->type );
		}
	}

	Script_system.RunCondition(CHA_OBJECTRENDER, '\0', NULL, obj);
	Script_system.RemHookVar("Self");
}

void obj_init_all_ships_physics()
{
	object	*objp;

	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if (objp->type == OBJ_SHIP)
			physics_ship_init(objp);
	}

}

// do client-side pre-interpolation object movement
void obj_client_pre_interpolate()
{
	object *objp;
	
	// duh
	obj_delete_all_that_should_be_dead();

	// client side processing of warping in effect stages
	multi_do_client_warp(flFrametime);     

	// client side movement of an observer
	if((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER)){
		obj_observer_move(flFrametime);   
	}
	
	// run everything except ships through physics (and ourselves of course)	
	obj_merge_created_list();						// must merge any objects created by the host!

	objp = GET_FIRST(&obj_used_list);
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )	{
		if((objp != Player_obj) && (objp->type == OBJ_SHIP)){
			continue;
		}

		// for all non-dead object which are _not_ ships
		if ( !(objp->flags&OF_SHOULD_BE_DEAD) )	{				
			// pre-move step
			obj_move_all_pre(objp, flFrametime);

			// store position and orientation
			objp->last_pos = objp->pos;
			objp->last_orient = objp->orient;

			// call physics
			obj_move_call_physics(objp, flFrametime);

			// post-move step
			obj_move_all_post(objp, flFrametime);
		}
	}
}

// do client-side post-interpolation object movement
void obj_client_post_interpolate()
{
	object *objp;

	//	After all objects have been moved, move all docked objects.
	objp = GET_FIRST(&obj_used_list);
	while( objp !=END_OF_LIST(&obj_used_list) )	{
		if ( objp != Player_obj ) {
			dock_move_docked_objects(objp);
		}
		objp = GET_NEXT(objp);
	}	

	// check collisions
	obj_check_all_collisions();		

	// do post-collision stuff for beam weapons
	beam_move_all_post();
}

#if 0
// following function is used in multiplayer only.  It deals with simulating objects on the client
// side.  Lasers will always get moved by the client (i.e. no object position info is ever sent for them).
// same for dumb missiles and possibly others.  We might move ships based on the last time their posision
// was updated
void obj_client_simulate(float frametime)
{
	object *objp;

	obj_delete_all_that_should_be_dead();

	multi_do_client_warp(frametime);     // client side processing of warping in effect stages
	
	if(Net_player->flags & NETINFO_FLAG_OBSERVER){
		obj_observer_move(frametime);   // client side movement of an observer
	}

	/*
	obj_merge_created_list();						// must merge any objects created by the host!
	objp = GET_FIRST(&obj_used_list);
	for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )	{

		if ( !(objp->flags&OF_SHOULD_BE_DEAD) )	{
			vec3d	cur_pos = objp->pos;			// Save the current position

			obj_move_all_pre(objp, frametime);

			int predict_from_server_pos = 1;

			// If not visible (or not a ship), bash position
			if ( (!obj_visible_from_eye(&cur_pos)) || (objp->type != OBJ_SHIP) ) {
				predict_from_server_pos = 0;
			}

			// If this is a player ship, don't predict from server position
			if ( objp->flags & OF_PLAYER_SHIP ) {
				predict_from_server_pos = 0;
			}

			if ( predict_from_server_pos ) {
				obj_client_predict_pos(objp, frametime);
			} else {
				obj_client_bash_pos(objp, frametime);
			}

			obj_move_all_post(objp, frametime);
		}
	}	
	*/

	//	After all objects have been moved, move all docked objects.
	objp = GET_FIRST(&obj_used_list);
	while( objp !=END_OF_LIST(&obj_used_list) )	{
		if ( objp != Player_obj ) {
			dock_move_docked_objects(objp);
		}
		objp = GET_NEXT(objp);
	}

	obj_check_all_collisions();	
}
#endif

void obj_observer_move(float frame_time)
{
	object *objp;
	float ft;

	// if i'm not in multiplayer, or not an observer, bail
	if(!(Game_mode & GM_MULTIPLAYER) || (Net_player == NULL) || !(Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type != OBJ_OBSERVER)){
		return;
	}

	objp = Player_obj;

	// obj_move_all_pre(objp, flFrametime);

	objp->last_pos = objp->pos;
	objp->last_orient = objp->orient;		// save the orientation -- useful in multiplayer.

	ft = flFrametime;
	obj_move_call_physics( objp, ft );
	obj_move_all_post(objp, frame_time);
	objp->flags &= ~OF_JUST_UPDATED;
}

// function to return a vector of the average position of all ships in the mission.
void obj_get_average_ship_pos( vec3d *pos )
{
	int count;
	object *objp;

	vm_vec_zero( pos );

   // average up all ship positions
	count = 0;
	for ( objp = GET_FIRST(&obj_used_list); objp != END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
		if ( objp->type != OBJ_SHIP )
			continue;
		vm_vec_add2( pos, &objp->pos );
		count++;
	}

	if ( count )
		vm_vec_scale( pos, 1.0f/(float)count );
}


int obj_get_SIF(object *objp)
{
	if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START))
		return Ship_info[Ships[objp->instance].ship_info_index].flags;

	Int3();
	return 0;
}

int obj_get_SIF(int obj)
{
	if ((Objects[obj].type == OBJ_SHIP) || (Objects[obj].type == OBJ_START))
		return Ship_info[Ships[Objects[obj].instance].ship_info_index].flags;

	Int3();
	return 0;
}

// Return the team for the object passed as a parameter
//
//	input:		objp => pointer to object that you want team for
//
// exit:			success => enumerated team
//					failure => -1 (for objects that don't have teams)
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
			Assert(team != -1);
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
			nprintf(("Warning","Warning => Asking for a team for object type %s\n", Object_type_names[objp->type]));
			team = -1;
			break;

		case OBJ_ASTEROID:
			team = Iff_traitor;
			break;

		default:
			Int3();	// can't happen
			break;
	} // end switch

	Assert(team != -1);
	return team;
}

// -------------------------------------------------------
// obj_add_pairs
//
// Add an element to the CheckObjects[] array, and update the 
// object pairs.  This is called from obj_create(), and the restore
// save-game code.
// 
void obj_add_pairs(int objnum)
{
	object	*objp;

	Assert(objnum != -1);
	objp = &Objects[objnum];	

	// don't do anything if its already in the object pair list
	if(!(objp->flags & OF_NOT_IN_COLL)){
		return;
	}

#ifdef OBJECT_CHECK 
	CheckObjects[objnum].type = objp->type;
	CheckObjects[objnum].signature = objp->signature;
	CheckObjects[objnum].flags = objp->flags & ~(OF_NOT_IN_COLL);
	CheckObjects[objnum].parent_sig = objp->parent_sig;
	CheckObjects[objnum].parent_type = objp->parent_type;
#endif	

	// Find all the objects that can collide with this and add 
	// it to the collision pair list. 
	object * A;
	for ( A = GET_FIRST(&obj_used_list); A !=END_OF_LIST(&obj_used_list); A = GET_NEXT(A) )	{
		obj_add_pair( objp, A );
	}
	
	objp->flags &= ~OF_NOT_IN_COLL;	
}

// Removes any occurances of object 'a' from
// the pairs list.
extern int Num_pairs;
extern obj_pair pair_used_list;
extern obj_pair pair_free_list;
void obj_remove_pairs( object * a )
{
	obj_pair *parent, *tmp;

	a->flags |= OF_NOT_IN_COLL;	
#ifdef OBJECT_CHECK 
	CheckObjects[OBJ_INDEX(a)].flags |= OF_NOT_IN_COLL;
#endif	

	if ( a->num_pairs < 1 )	{
		//mprintf(( "OBJPAIR: No need to remove pairs 1!\n" ));
		return;
	}

	Num_pairs-=a->num_pairs;
	
	parent = &pair_used_list;
	tmp = parent->next;

	while( tmp != NULL )	{
		if ( (tmp->a==a) || (tmp->b==a) )	{
			// Hmmm... a potenial compiler optimization problem here... either tmp->a or tmp->b
			// is equal to 'a' and we modify 'num_pairs' in one of these and then use the value
			// stored in 'a' later one... will the optimizer find that?  Hmmm...
			tmp->a->num_pairs--;
			Assert( tmp->a->num_pairs > -1 );
			tmp->b->num_pairs--;
			Assert( tmp->b->num_pairs > -1 );
			parent->next = tmp->next;
			tmp->a = tmp->b = NULL;
			tmp->next = pair_free_list.next;
			pair_free_list.next = tmp;
			tmp = parent->next;

			if ( a->num_pairs==0 )	{
				//mprintf(( "OBJPAIR: No need to remove pairs 2!\n" ));
				break;
			}

		} else {
			parent = tmp;
			tmp = tmp->next;
		}
	}
}

// reset all collisions
void obj_reset_all_collisions()
{
	// clear checkobjects
#ifndef NDEBUG
	memset(CheckObjects, 0, sizeof(checkobject) * MAX_OBJECTS);
#endif

	// clear object pairs
	obj_reset_pairs();

	// now add every object back into the object collision pairs
	object *moveup;
	moveup = GET_FIRST(&obj_used_list);
	while(moveup != END_OF_LIST(&obj_used_list)){
		// he's not in the collision list
		moveup->flags |= OF_NOT_IN_COLL;

		// recalc pairs for this guy
		obj_add_pairs(OBJ_INDEX(moveup));

		// next
		moveup = GET_NEXT(moveup);
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

//Makes an object start 'gliding'
//that is, it will continue on the same velocity that it was going,
//regardless of orientation -WMC
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

//Returns whether an object is gliding -WMC
bool object_get_gliding(object *objp)
{
	Assert(objp != NULL);

	return ( ((objp->phys_info.flags & PF_GLIDING) != 0) || ((objp->phys_info.flags & PF_FORCE_GLIDE) != 0));
}

bool object_glide_forced(object *objp)
{
	return (objp->phys_info.flags & PF_FORCE_GLIDE) != 0;
}

//Quickly finds an object by its signature
int obj_get_by_signature(int sig)
{
	object *objp = GET_FIRST(&obj_used_list);
	while( objp !=END_OF_LIST(&obj_used_list) )
	{
		if(objp->signature == sig)
			return OBJ_INDEX(objp);

		objp = GET_NEXT(objp);
	}
	return -1;
}

//Gets object model
int object_get_model(object *objp)
{
	switch(objp->type)
	{
		case OBJ_ASTEROID:
		{
			asteroid *asp = &Asteroids[objp->instance];
			return Asteroid_info[asp->asteroid_type].model_num[asp->asteroid_subtype];
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
		default:
			break;
	}

	return -1;
}
