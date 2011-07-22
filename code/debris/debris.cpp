/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "debris/debris.h"
#include "render/3d.h"
#include "fireball/fireballs.h"
#include "radar/radar.h"
#include "gamesnd/gamesnd.h"
#include "object/objectsnd.h"
#include "globalincs/linklist.h"
#include "particle/particle.h"
#include "freespace2/freespace.h"
#include "object/objcollide.h"
#include "io/timer.h"
#include "species_defs/species_defs.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "radar/radarsetup.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"




#define MAX_LIFE									10.0f
#define MIN_RADIUS_FOR_PERSISTANT_DEBRIS	50		// ship radius at which debris from it becomes persistant
#define DEBRIS_SOUND_DELAY						2000	// time to start debris sound after created

// limit the number of hull debris chunks that can exist.  
//#define	MAX_HULL_PIECES		10
#define MAX_HULL_PIECES			MAX_DEBRIS_PIECES //-WMCD
int		Num_hull_pieces;		// number of hull pieces in existance
debris	Hull_debris_list;		// head of linked list for hull debris chunks, for quick search

debris Debris[MAX_DEBRIS_PIECES];

int Num_debris_pieces = 0;
int Debris_inited = 0;

int Debris_model = -1;
int Debris_vaporize_model = -1;
int Debris_num_submodels = 0;

#define	MAX_DEBRIS_DIST					10000.0f			//	Debris goes away if it's this far away.
#define	DEBRIS_DISTANCE_CHECK_TIME		(10*1000)		//	Check every 10 seconds.
#define	DEBRIS_INDEX(dp) (dp-Debris)

#define	MAX_SPEED_SMALL_DEBRIS		200					// maximum velocity of small debris piece
#define	MAX_SPEED_BIG_DEBRIS			150					// maximum velocity of big debris piece
#define	MAX_SPEED_CAPITAL_DEBRIS	100					// maximum velocity of capital debris piece
//#define	DEBRIS_SPEED_DEBUG

// ---------------------------------------------------------------------------------------
// debris_start_death_roll()
//
//	Start the sequence of a piece of debris writhing in unholy agony!!!
//
static void debris_start_death_roll(object *debris_obj, debris *debris_p)
{
	if (debris_p->is_hull)	{
		// tell everyone else to blow up the piece of debris
		if( MULTIPLAYER_MASTER )
			send_debris_update_packet(debris_obj,DEBRIS_UPDATE_NUKE);

		int fireball_type = fireball_ship_explosion_type(&Ship_info[debris_p->ship_info_index]);
		if(fireball_type < 0) {
			fireball_type = FIREBALL_EXPLOSION_LARGE1 + rand()%FIREBALL_NUM_LARGE_EXPLOSIONS;
		}
		fireball_create( &debris_obj->pos, fireball_type, FIREBALL_LARGE_EXPLOSION, OBJ_INDEX(debris_obj), debris_obj->radius*1.75f);

		// only play debris destroy sound if hull piece and it has been around for at least 2 seconds
		if ( Missiontime > debris_p->time_started + 2*F1_0 ) {
			snd_play_3d( &Snds[SND_MISSILE_IMPACT1], &debris_obj->pos, &View_position, debris_obj->radius );
			
		}
	}

  	debris_obj->flags |= OF_SHOULD_BE_DEAD;
//	demo_do_flag_dead(OBJ_INDEX(debris_obj));
}

// ---------------------------------------------------------------------------------------
// debris_init()
//
// This will get called at the start of each level.
//
void debris_init()
{
	int i;

	if ( !Debris_inited ) {
		Debris_inited = 1;
	}

	Debris_model = -1;
	Debris_vaporize_model = -1;
	Debris_num_submodels = 0;
		
	// Reset everything between levels
	Num_debris_pieces = 0;
	for (i=0; i<MAX_DEBRIS_PIECES; i++ )	{
		Debris[i].flags = 0;
		Debris[i].sound_delay = 0;
	}
		
	Num_hull_pieces = 0;
	list_init(&Hull_debris_list);
}

// Page in debris bitmaps at level load
void debris_page_in()
{
	uint i;

	Debris_model = model_load( NOX("debris01.pof"), 0, NULL );
	if (Debris_model >= 0)	{
		polymodel * pm;
		pm = model_get(Debris_model);
		Debris_num_submodels = pm->n_models;
	}

	Debris_vaporize_model = model_load( NOX("debris02.pof"), 0, NULL );

	for (i=0; i<Species_info.size(); i++ )
	{
		species_info *species = &Species_info[i];

		nprintf(( "Paging", "Paging in debris texture '%s'\n", species->debris_texture.filename));

		species->debris_texture.bitmap_id = bm_load(species->debris_texture.filename);
		if (species->debris_texture.bitmap_id < 0)
		{
			Warning( LOCATION, "Couldn't load species %s debris\ntexture, '%s'\n", species->species_name, species->debris_texture.filename);
		}

		bm_page_in_texture(species->debris_texture.bitmap_id);
	}
	
}

MONITOR(NumSmallDebrisRend)
MONITOR(NumHullDebrisRend)

// ---------------------------------------------------------------------------------------
// debris_render()
//
//
void debris_render(object * obj)
{
	int			i, num, swapped;
	polymodel	*pm;
	debris		*db;


	swapped = -1;
	pm = NULL;	
	num = obj->instance;

	Assert(num >= 0 && num < MAX_DEBRIS_PIECES);
	db = &Debris[num];

	Assert(db->flags & DEBRIS_USED);

	texture_info *tbase = NULL;

	// Swap in a different texture depending on the species
	if (db->species >= 0)
	{
		pm = model_get( db->model_num );

		//WMC - Someday, we should have glowing debris.
		if ( pm != NULL && (pm->n_textures == 1) ) {
			tbase = &pm->maps[0].textures[TM_BASE_TYPE];
			swapped = tbase->GetTexture();
			tbase->SetTexture(Species_info[db->species].debris_texture.bitmap_id);
		}
	}

	model_clear_instance( db->model_num );

	// Only render electrical arcs if within 500m of the eye (for a 10m piece)
	if ( vm_vec_dist_quick( &obj->pos, &Eye_position ) < obj->radius*50.0f )	{
		for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
			if ( timestamp_valid( db->arc_timestamp[i] ) )	{
				model_add_arc( db->model_num, db->submodel_num, &db->arc_pts[i][0], &db->arc_pts[i][1], MARC_TYPE_NORMAL );
			}
		}
	}

	if ( db->is_hull )	{
		MONITOR_INC(NumHullDebrisRend,1);
		submodel_render( db->model_num, db->submodel_num, &obj->orient, &obj->pos );
	} else {
		MONITOR_INC(NumSmallDebrisRend,1);
		submodel_render( db->model_num, db->submodel_num, &obj->orient, &obj->pos, MR_NO_LIGHTING );
	}

	if (tbase != NULL && (swapped!=-1) && pm)	{
		tbase->SetTexture(swapped);
	}
}

// Removed the DEBRIS_EXPIRE flag, and remove item from Hull_debris_list
void debris_clear_expired_flag(debris *db)
{
	if ( db->flags & DEBRIS_EXPIRE ) {
		db->flags &= ~DEBRIS_EXPIRE;
		if ( db->is_hull ) {
			Num_hull_pieces--;
			list_remove(Hull_debris_list, db);
			Assert( Num_hull_pieces >= 0 );
		}
	}
}

// ---------------------------------------------------------------------------------------
// debris_delete()
//
// Delete the debris object.  This is only ever called via obj_delete().  Do not call directly.
// Use debris_start_death_roll() if you want to force a debris piece to die.
//
void debris_delete( object * obj )
{
	int		num;
	debris	*db;

	num = obj->instance;
	Assert( Debris[num].objnum == OBJ_INDEX(obj));

	db = &Debris[num];

	Assert( Num_debris_pieces >= 0 );
	if ( db->is_hull && (db->flags & DEBRIS_EXPIRE) ) {
		debris_clear_expired_flag(db);
	}

	db->flags = 0;
	Num_debris_pieces--;
}

//	If debris piece *db is far away from all players, make it go away very soon.
//	In single player game, delete if MAX_DEBRIS_DIST from player.
//	In multiplayer game, delete if MAX_DEBRIS_DIST from all players.
void maybe_delete_debris(debris *db)
{
	object	*objp;

	if (timestamp_elapsed(db->next_distance_check) && timestamp_elapsed(db->must_survive_until)) {
		if (!(Game_mode & GM_MULTIPLAYER)) {		//	In single player game, just check against player.
			if (vm_vec_dist_quick(&Player_obj->pos, &Objects[db->objnum].pos) > MAX_DEBRIS_DIST)
				db->lifeleft = 0.1f;
			else
				db->next_distance_check = timestamp(DEBRIS_DISTANCE_CHECK_TIME);
		} else {
			for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) ) {
				if (objp->flags & OF_PLAYER_SHIP) {
					if (vm_vec_dist_quick(&objp->pos, &Objects[db->objnum].pos) < MAX_DEBRIS_DIST) {
						db->next_distance_check = timestamp(DEBRIS_DISTANCE_CHECK_TIME);
						return;
					}
				}
			}
			db->lifeleft = 0.1f;
		}
	}
}

// broke debris_move into debris_process_pre and debris_process_post as was done with all
// *_move functions on 8/13 by MK and MA.
void debris_process_pre( object *objp, float frame_time)
{
}

MONITOR(NumSmallDebris)
MONITOR(NumHullDebris)

// ---------------------------------------------------------------------------------------
// debris_process_post()
//
// Do various updates to debris:  check if time to die, start fireballs
//
// parameters:		obj			=>		pointer to debris object
//						frame_time	=>		time elapsed since last debris_move() called
//
//	Maybe delete debris if it's very far away from player.
void debris_process_post(object * obj, float frame_time)
{
	int i, num;
	num = obj->instance;

	int objnum = OBJ_INDEX(obj);
	Assert( Debris[num].objnum == objnum );
	debris *db = &Debris[num];

	if ( db->is_hull ) {
		MONITOR_INC(NumHullDebris,1);
		radar_plot_object( obj );

		if ( timestamp_elapsed(db->sound_delay) ) {
			obj_snd_assign(objnum, SND_DEBRIS, &vmd_zero_vector, 0);
			db->sound_delay = 0;
		}
	} else {
		MONITOR_INC(NumSmallDebris,1);
	}

	if ( db->lifeleft >= 0.0f) {
		db->lifeleft -= frame_time;
		if ( db->lifeleft < 0.0f )	{
			debris_start_death_roll(obj, db);
		}
	}

	maybe_delete_debris(db);	//	Make this debris go away if it's very far away.

	// ================== DO THE ELECTRIC ARCING STUFF =====================
	if ( db->arc_frequency <= 0 )	{
		return;			// If arc_frequency <= 0, this piece has no arcs on it
	}

	if ( !timestamp_elapsed(db->fire_timeout) && timestamp_elapsed(db->next_fireball))	{

		// start the next fireball up in the next 50 - 100 ms
		//db->next_fireball = timestamp_rand(60,80);		

		db->next_fireball = timestamp_rand(db->arc_frequency,db->arc_frequency*2 );
		db->arc_frequency += 100;	

		if (db->is_hull)	{

			int n, n_arcs = ((rand()>>5) % 3)+1;		// Create 1-3 sparks

			vec3d v1, v2, v3, v4;
			submodel_get_two_random_points( db->model_num, db->submodel_num, &v1, &v2 );
			submodel_get_two_random_points( db->model_num, db->submodel_num, &v3, &v4 );

			n = 0;

			int a = 100, b = 1000;
			int lifetime = (myrand()%((b)-(a)+1))+(a);

			// Create the spark effects
			for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
				if ( !timestamp_valid( db->arc_timestamp[i] ) )	{
					//db->arc_timestamp[i] = timestamp_rand(400,1000);	// live up to a second
					db->arc_timestamp[i] = timestamp(lifetime);	// live up to a second

					switch( n )	{
					case 0:
						db->arc_pts[i][0] = v1;
						db->arc_pts[i][1] = v2;
						break;
					case 1:
						db->arc_pts[i][0] = v2;
						db->arc_pts[i][1] = v3;
						break;

					case 2:
						db->arc_pts[i][0] = v2;
						db->arc_pts[i][1] = v4;
						break;

					default:
						Int3();
					}
						
					n++;
					if ( n == n_arcs )
						break;	// Don't need to create anymore
				}
			}

	
			// rotate v2 out of local coordinates into world.
			// Use v2 since it is used in every bolt.  See above switch().
			vec3d snd_pos;
			vm_vec_unrotate(&snd_pos, &v2, &obj->orient);
			vm_vec_add2(&snd_pos, &obj->pos );

			//Play a sound effect
			if ( lifetime > 750 )	{
				// 1.00 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_05], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  500 )	{
				// 0.75 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_04], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  250 )	{
				// 0.50 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_03], &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  100 )	{
				// 0.25 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_02], &snd_pos, &View_position, obj->radius );
			} else {
				// 0.10 second effect
				snd_play_3d( &Snds[SND_DEBRIS_ARC_01], &snd_pos, &View_position, obj->radius );
			}

		}



	}

	for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
		if ( timestamp_valid( db->arc_timestamp[i] ) )	{
			if ( timestamp_elapsed( db->arc_timestamp[i] ) )	{
				// Kill off the spark
				db->arc_timestamp[i] = timestamp(-1);
			} else {
				// Maybe move a vertex....  20% of the time maybe?
				int mr = myrand();
				if ( mr < RAND_MAX/5 )	{
					vec3d v1, v2;
					submodel_get_two_random_points( db->model_num, db->submodel_num, &v1, &v2 );
					db->arc_pts[i][mr % 2] = v1;
				}
			}
		}
	}

}

// ---------------------------------------------------------------------------------------
// debris_find_oldest()
//
// Locate the oldest hull debris chunk.  Search through the Hull_debris_list, which is a list
// of all the hull debris chunks.
//
int debris_find_oldest()
{
	int		oldest_index;
	fix		oldest_time;
	debris	*db;

	oldest_index = -1;
	oldest_time = 0x7fffffff;

	for ( db = GET_FIRST(&Hull_debris_list); db != END_OF_LIST(&Hull_debris_list); db = GET_NEXT(db) ) {
		if ( (db->time_started < oldest_time) && !(Objects[db->objnum].flags & OF_SHOULD_BE_DEAD) ) {
			oldest_index = DEBRIS_INDEX(db);
			oldest_time = db->time_started;
		}
	}

	return oldest_index;
}

#define	DEBRIS_ROTVEL_SCALE	5.0f
void calc_debris_physics_properties( physics_info *pi, vec3d *min, vec3d *max );
// ---------------------------------------------------------------------------------------
// debris_create()
//
// Create debris from an object
//
//	exp_force:	Explosion force, used to assign velocity to pieces.
//					1.0f assigns velocity like before.  2.0f assigns twice as much to non-inherited part of velocity
object *debris_create(object *source_obj, int model_num, int submodel_num, vec3d *pos, vec3d *exp_center, int hull_flag, float exp_force)
{
	int		i, n, objnum, parent_objnum;
	object	*obj;
	ship		*shipp;
	debris	*db;	
	polymodel *pm;
	int vaporize;
	physics_info *pi = NULL;
	ship_info *sip = NULL;

	parent_objnum = OBJ_INDEX(source_obj);

	Assert( (source_obj->type == OBJ_SHIP ) || (source_obj->type == OBJ_GHOST));
	Assert( source_obj->instance >= 0 && source_obj->instance < MAX_SHIPS );	
	shipp = &Ships[source_obj->instance];
	sip = &Ship_info[shipp->ship_info_index];
	vaporize = (shipp->flags &SF_VAPORIZE);

	if ( !hull_flag )	{
		// Make vaporize debris seen from farther away
		float dist = vm_vec_dist_quick( pos, &Eye_position );
		if (vaporize) {
			dist /= 2.0f;
		}
		if ( dist > 200.0f ) {
			//mprintf(( "Not creating debris that is %.1f m away\n", dist ));
			return NULL;
		}
	}

	if ( hull_flag && (Num_hull_pieces >= MAX_HULL_PIECES ) ) {
		// cause oldest hull debris chunk to blow up
		n = debris_find_oldest();
		if ( n >= 0 ) {
			debris_start_death_roll(&Objects[Debris[n].objnum], &Debris[n] );
		}
	}

	for (n=0; n<MAX_DEBRIS_PIECES; n++ ) {
		if ( !(Debris[n].flags & DEBRIS_USED) )
			break;
	}

	if (n == MAX_DEBRIS_PIECES) {
		n = debris_find_oldest();

		if (n >= 0)
			debris_start_death_roll(&Objects[Debris[n].objnum], &Debris[n]);

		nprintf(("Warning","Frame %i: Could not create debris, no more slots left\n", Framecount));
		return NULL;
	}

	db = &Debris[n];

	//WMC - We must survive until now, at least.
	db->must_survive_until = timestamp();

	if(hull_flag && sip->debris_min_lifetime >= 0.0f && sip->debris_max_lifetime >= 0.0f)
	{
		db->lifeleft = (( sip->debris_max_lifetime - sip->debris_min_lifetime ) * frand()) + sip->debris_min_lifetime;
	}
	else
	{
		// Create Debris piece n!
		if ( hull_flag ) {
			if (rand() < RAND_MAX/6)	// Make some pieces blow up shortly after explosion.
				db->lifeleft = 2.0f * ((float) myrand()/(float) RAND_MAX) + 0.5f;
			else
				db->lifeleft = -1.0f;		// large hull pieces stay around forever
		} else {
			db->lifeleft = (i2fl(myrand())/i2fl(RAND_MAX))*2.0f+0.1f;
		}
	}

	//WMC - Oh noes, we may need to change lifeleft
	if(hull_flag)
	{
		if(sip->debris_min_lifetime >= 0.0f && sip->debris_max_lifetime >= 0.0f)
		{
			db->must_survive_until = timestamp(fl2i(sip->debris_min_lifetime * 1000.0f));
			db->lifeleft = (( sip->debris_max_lifetime - sip->debris_min_lifetime ) * frand()) + sip->debris_min_lifetime;
		}
		else if(sip->debris_min_lifetime >= 0.0f)
		{
			db->must_survive_until = timestamp(fl2i(sip->debris_min_lifetime * 1000.0f));
			if(db->lifeleft < sip->debris_min_lifetime)
				db->lifeleft = sip->debris_min_lifetime;
		}
		else if(sip->debris_max_lifetime >= 0.0f)
		{
			if(db->lifeleft > sip->debris_max_lifetime)
				db->lifeleft = sip->debris_max_lifetime;
		}
	}

	// increase lifetime for vaporized debris
	if (vaporize) {
		db->lifeleft *= 3.0f;
	}
	db->flags |= DEBRIS_USED;
	db->is_hull = hull_flag;
	db->source_objnum = parent_objnum;
	db->source_sig = source_obj->signature;
	db->ship_info_index = shipp->ship_info_index;
	db->team = shipp->team;
	db->fire_timeout = 0;	// if not changed, timestamp_elapsed() will return false
	db->time_started = Missiontime;
	db->species = Ship_info[shipp->ship_info_index].species;
	db->next_distance_check = (myrand() % 2000) + 4*DEBRIS_DISTANCE_CHECK_TIME;
	db->parent_alt_name = shipp->alt_type_index;
	db->damage_mult = 1.0f;

	for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
		db->arc_timestamp[i] = timestamp(-1);
		//	vec3d	arc_pts[MAX_DEBRIS_ARCS][2];		// The endpoints of each arc
	}

	if ( db->is_hull )	{
		// Percent of debris pieces with arcs controlled via table (default 50%)
		if (frand() < sip->debris_arc_percent) {
			db->arc_frequency = 1000;
		} else {
			db->arc_frequency = 0;
		}
	} else {
		db->arc_frequency = 0;
	}

	if ( model_num < 0 )	{
		if (vaporize) {
			db->model_num = Debris_vaporize_model;
		} else {
			db->model_num = Debris_model;
		}
		db->submodel_num = (myrand()>>4) % Debris_num_submodels;
	} else {
		db->model_num = model_num;
		db->submodel_num = submodel_num;
	}
	float radius = submodel_get_radius( db->model_num, db->submodel_num );

	db->next_fireball = timestamp_rand(500,2000);	//start one 1/2 - 2 secs later

	if ( pos == NULL )
		pos = &source_obj->pos;

	uint flags = OF_RENDERS | OF_PHYSICS;
	if ( hull_flag )	
		flags |= OF_COLLIDES;
	objnum = obj_create( OBJ_DEBRIS, parent_objnum, n, &source_obj->orient, pos, radius, flags );
	if ( objnum == -1 ) {
		mprintf(("Couldn't create debris object -- out of object slots\n"));
		return NULL;
	}

	db->objnum = objnum;
	
	obj = &Objects[objnum];
	pi = &obj->phys_info;

	// assign the network signature.  The signature will be 0 for non-hull pieces, but since that
	// is our invalid signature, it should be okay.
	obj->net_signature = 0;

	if ( (Game_mode & GM_MULTIPLAYER) && hull_flag ) {
		obj->net_signature = multi_get_next_network_signature( MULTI_SIG_DEBRIS );
	}

	// -- No long need shield: bset_shield_strength(obj, 100.0f);		//	Hey!  Set to some meaningful value!

	if (source_obj->type == OBJ_SHIP) {
		obj->hull_strength = Ships[source_obj->instance].ship_max_hull_strength/8.0f;
	} else
		obj->hull_strength = 10.0f;

	if (hull_flag) {
		if(sip->debris_min_hitpoints >= 0.0f && sip->debris_max_hitpoints >= 0.0f)
		{
			obj->hull_strength = (( sip->debris_max_hitpoints - sip->debris_min_hitpoints ) * frand()) + sip->debris_min_hitpoints;
		}
		else if(sip->debris_min_hitpoints >= 0.0f)
		{
			if(obj->hull_strength < sip->debris_min_hitpoints)
				obj->hull_strength = sip->debris_min_hitpoints;
		}
		else if(sip->debris_max_hitpoints >= 0.0f)
		{
			if(obj->hull_strength > sip->debris_max_hitpoints)
				obj->hull_strength = sip->debris_max_hitpoints;
		}
		db->damage_mult = sip->debris_damage_mult;
	}

	Num_debris_pieces++;

	vec3d rotvel, radial_vel, to_center;

	if ( exp_center )
		vm_vec_sub( &to_center,pos, exp_center );
	else
		vm_vec_zero(&to_center);

	float scale;

	if ( hull_flag )	{
		float t;
		scale = exp_force * i2fl((myrand()%20) + 10);	// for radial_vel away from location of blast center
		db->sound_delay = timestamp(DEBRIS_SOUND_DELAY);

		// set up physics mass and I_inv for hull debris pieces
		pm = model_get(model_num);
		vec3d *min, *max;
		min = &pm->submodel[submodel_num].min;
		max = &pm->submodel[submodel_num].max;
		calc_debris_physics_properties( &obj->phys_info, min, max );

		// limit the amount of time that fireballs appear
		// let fireball length be linked to radius of ship.  Range is .33 radius => 3.33 radius seconds.
		t = 1000*Objects[db->source_objnum].radius/3 + myrand()%(fl2i(1000*3*Objects[db->source_objnum].radius));
		db->fire_timeout = timestamp(fl2i(t));		// fireballs last from 5 - 30 seconds
		
		if ( Objects[db->source_objnum].radius < MIN_RADIUS_FOR_PERSISTANT_DEBRIS ) {
			db->flags |= DEBRIS_EXPIRE;	// debris can expire
			Num_hull_pieces++;
			list_append(&Hull_debris_list, db);
		} else {
			nprintf(("Alan","A forever chunk of debris was created from ship with radius %f\n",Objects[db->source_objnum].radius));
		}
	}
	else {
		scale = exp_force * i2fl((myrand()%20) + 10);	// for radial_vel away from blast center (non-hull)
	}

	if ( vm_vec_mag_squared( &to_center ) < 0.1f )	{
		vm_vec_rand_vec_quick(&radial_vel);
		vm_vec_scale(&radial_vel, scale );
	}
	else {
		vm_vec_normalize(&to_center);
		vm_vec_copy_scale(&radial_vel, &to_center, scale );
	}

	//	MK: This next line causes debris pieces to get between 50% and 100% of the parent ship's
	//	velocity.  What would be very cool is if the rotational velocity of the parent would become 
	// translational velocity of the debris piece.  This would be based on the location of the debris
	//	piece in the parent object.

	// DA: here we need to vel_from_rot = w x to_center, where w is world is unrotated to world coords and offset is the 
	// displacement fromt the center of the parent object to the center of the debris piece

	vec3d world_rotvel, vel_from_rotvel;
	vm_vec_unrotate ( &world_rotvel, &source_obj->phys_info.rotvel, &source_obj->orient );
	vm_vec_crossprod ( &vel_from_rotvel, &world_rotvel, &to_center );
	vm_vec_scale ( &vel_from_rotvel, DEBRIS_ROTVEL_SCALE);

	vm_vec_add (&obj->phys_info.vel, &radial_vel, &source_obj->phys_info.vel);
	vm_vec_add2(&obj->phys_info.vel, &vel_from_rotvel);

//	vm_vec_scale_add(&obj->phys_info.vel, &radial_vel, &source_obj->phys_info.vel, frand()/2.0f + 0.5f);
//	nprintf(("Andsager","object vel from rotvel: %0.2f, %0.2f, %0.2f\n",vel_from_rotvel.x, vel_from_rotvel.y, vel_from_rotvel.z));

// make sure rotational velocity does not get too high
	if (radius < 1.0) {
		radius = 1.0f;
	}

	scale = ( 6.0f + i2fl(myrand()%4) ) / radius;

	vm_vec_rand_vec_quick(&rotvel);
	vm_vec_scale(&rotvel, scale);

	pi->flags |= PF_DEAD_DAMP;
	pi->rotvel = rotvel;
	check_rotvel_limit( &obj->phys_info );

	// check that debris is not created with too high a velocity
	if (hull_flag)
	{
		shipfx_debris_limit_speed(db, shipp);
	}

	// blow out his reverse thrusters. Or drag, same thing.
	pi->rotdamp = 10000.0f;
	pi->side_slip_time_const = 10000.0f;
	pi->flags |= (PF_REDUCED_DAMP | PF_DEAD_DAMP);	// set damping equal for all axis and not changable

	vm_vec_zero(&pi->max_vel);		// make so he can't turn on his own VOLITION anymore.
	vm_vec_zero(&pi->max_rotvel);	// make so he can't change speed on his own VOLITION anymore.


	// ensure vel is valid
	Assert( !vm_is_vec_nan(&obj->phys_info.vel) );

//	if ( hull_flag )	{
//		vm_vec_zero(&pi->vel);
//		vm_vec_zero(&pi->rotvel);
//	}

	return obj;
}

// ---------------------------------------------------------------------------------------
// debris_hit()
//
//	Alas, poor debris_obj got whacked.  Fortunately, we know who did it, where and how hard, so we
//	can do something about it.
//
void debris_hit(object *debris_obj, object *other_obj, vec3d *hitpos, float damage)
{
	debris	*debris_p = &Debris[debris_obj->instance];


	// Do a little particle spark shower to show we hit
	{
		particle_emitter	pe;

		pe.pos = *hitpos;								// Where the particles emit from
		pe.vel = debris_obj->phys_info.vel;		// Initial velocity of all the particles

		vec3d tmp_norm;
		vm_vec_sub( &tmp_norm, hitpos, &debris_obj->pos );
		vm_vec_normalize_safe(&tmp_norm);
			
		pe.normal = tmp_norm;			// What normal the particle emit around
		pe.normal_variance = 0.3f;		//	How close they stick to that normal 0=good, 1=360 degree
		pe.min_rad = 0.20f;				// Min radius
		pe.max_rad = 0.40f;				// Max radius

		// Sparks for first time at this spot
		pe.num_low = 10;				// Lowest number of particles to create
		pe.num_high = 10;			// Highest number of particles to create
		pe.normal_variance = 0.3f;		//	How close they stick to that normal 0=good, 1=360 degree
		pe.min_vel = 0.0f;				// How fast the slowest particle can move
		pe.max_vel = 10.0f;				// How fast the fastest particle can move
		pe.min_life = 0.25f;			// How long the particles live
		pe.max_life = 0.75f;			// How long the particles live
		particle_emit( &pe, PARTICLE_FIRE, 0 );
	}

	// multiplayer clients bail here
	if(MULTIPLAYER_CLIENT){
		return;
	}

	if ( damage < 0.0f ) {
		damage = 0.0f;
	}

	debris_obj->hull_strength -= damage;

	if (debris_obj->hull_strength < 0.0f) {
		debris_start_death_roll(debris_obj, debris_p );
	} else {
		// otherwise, give all the other players an update on the debris
		if(MULTIPLAYER_MASTER){
			send_debris_update_packet(debris_obj,DEBRIS_UPDATE_UPDATE);
		}
	}
}

// ---------------------------------------------------------------------------------------
// debris_check_collision()
//
//	See if poor debris object *obj got whacked by evil *other_obj at point *hitpos.
// NOTE: debris_hit_info pointer NULL for debris:weapon collision, otherwise debris:ship collision.
//	Return true if hit, else return false.
//
int debris_check_collision(object *pdebris, object *other_obj, vec3d *hitpos, collision_info_struct *debris_hit_info)
{
	mc_info	mc;
	int		num;

	Assert( pdebris->type == OBJ_DEBRIS );

	num = pdebris->instance;
	Assert( num >= 0 );

	Assert( Debris[num].objnum == OBJ_INDEX(pdebris));	

	// debris_hit_info NULL - so debris-weapon collision
	if ( debris_hit_info == NULL ) {
		// debris weapon collision
		Assert( other_obj->type == OBJ_WEAPON );
		mc.model_instance_num = -1;
		mc.model_num = Debris[num].model_num;	// Fill in the model to check
		mc.submodel_num = Debris[num].submodel_num;
		model_clear_instance( mc.model_num );
		mc.orient = &pdebris->orient;					// The object's orient
		mc.pos = &pdebris->pos;							// The object's position
		mc.p0 = &other_obj->last_pos;				// Point 1 of ray to check
		mc.p1 = &other_obj->pos;					// Point 2 of ray to check
		mc.flags = (MC_CHECK_MODEL | MC_SUBMODEL);

		if (model_collide(&mc)) {
			*hitpos = mc.hit_point_world;
		}

		return mc.num_hits;
	}
	
	// debris ship collision -- use debris_hit_info to calculate physics
	object *ship_obj = other_obj;
	Assert( ship_obj->type == OBJ_SHIP );

	object *heavy = debris_hit_info->heavy;
	object *light = debris_hit_info->light;
	object *heavy_obj = heavy;
	object *light_obj = light;

	vec3d zero, p0, p1;
	vm_vec_zero(&zero);
	vm_vec_sub(&p0, &light->last_pos, &heavy->last_pos);
	vm_vec_sub(&p1, &light->pos, &heavy->pos);

	mc.pos = &zero;								// The object's position
	mc.p0 = &p0;									// Point 1 of ray to check
	mc.p1 = &p1;									// Point 2 of ray to check

	// find the light object's position in the heavy object's reference frame at last frame and also in this frame.
	vec3d p0_temp, p0_rotated;
		
	// Collision detection from rotation enabled if at rotaion is less than 30 degree in frame
	// This should account for all ships
	if ( (vm_vec_mag_squared(&heavy->phys_info.rotvel) * flFrametime*flFrametime) < (PI*PI/36) ) {
		// collide_rotate calculate (1) start position and (2) relative velocity
		debris_hit_info->collide_rotate = 1;
		vm_vec_rotate(&p0_temp, &p0, &heavy->last_orient);
		vm_vec_unrotate(&p0_rotated, &p0_temp, &heavy->orient);
		mc.p0 = &p0_rotated;				// Point 1 of ray to check
		vm_vec_sub(&debris_hit_info->light_rel_vel, &p1, &p0_rotated);
		vm_vec_scale(&debris_hit_info->light_rel_vel, 1/flFrametime);
	} else {
		debris_hit_info->collide_rotate = 0;
		vm_vec_sub(&debris_hit_info->light_rel_vel, &light->phys_info.vel, &heavy->phys_info.vel);
	}

	int mc_ret_val = 0;

	if ( debris_hit_info->heavy == ship_obj ) {	// ship is heavier, so debris is sphere. Check sphere collision against ship poly model
		mc.model_instance_num = Ships[ship_obj->instance].model_instance_num;
		mc.model_num = Ship_info[Ships[ship_obj->instance].ship_info_index].model_num;	// Fill in the model to check
		mc.orient = &ship_obj->orient;								// The object's orient
		mc.radius = pdebris->radius;
		mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);

		// copy important data
		int copy_flags = mc.flags;  // make a copy of start end positions of sphere in  big ship RF
		vec3d copy_p0, copy_p1;
		copy_p0 = *mc.p0;
		copy_p1 = *mc.p1;

		// first test against the sphere - if this fails then don't do any submodel tests
		mc.flags = MC_ONLY_SPHERE | MC_CHECK_SPHERELINE;

		SCP_vector<int> submodel_vector;
		polymodel *pm;
		polymodel_instance *pmi;

		ship_model_start(ship_obj);

		if (model_collide(&mc)) {

			// Set earliest hit time
			debris_hit_info->hit_time = FLT_MAX;

			// Do collision the cool new way
			if ( debris_hit_info->collide_rotate ) {
				SCP_vector<int>::iterator smv;

				// We collide with the sphere, find the list of rotating submodels and test one at a time
				model_get_rotating_submodel_list(&submodel_vector, heavy_obj);

				// Get polymodel and turn off all rotating submodels, collide against only 1 at a time.
				pm = model_get(Ship_info[Ships[heavy_obj->instance].ship_info_index].model_num);
				pmi = model_get_instance(Ships[heavy_obj->instance].model_instance_num);

				// turn off all rotating submodels and test for collision
				for (smv = submodel_vector.begin(); smv != submodel_vector.end(); smv++) {
					pmi->submodel[*smv].collision_checked = true;
				}

				// reset flags to check MC_CHECK_MODEL | MC_CHECK_SPHERELINE and maybe MC_CHECK_INVISIBLE_FACES and MC_SUBMODEL_INSTANCE
				mc.flags = copy_flags | MC_SUBMODEL_INSTANCE;

				// check each submodel in turn
				for (smv = submodel_vector.begin(); smv != submodel_vector.end(); smv++) {
					// turn on submodel for collision test
					pmi->submodel[*smv].collision_checked = false;

					// set angles for last frame (need to set to prev to get p0)
					angles copy_angles = pmi->submodel[*smv].angs;

					// find the start and end positions of the sphere in submodel RF
					pmi->submodel[*smv].angs = pmi->submodel[*smv].prev_angs;
					world_find_model_instance_point(&p0, &light_obj->last_pos, pm, pmi, *smv, &heavy_obj->last_orient, &heavy_obj->last_pos);

					pmi->submodel[*smv].angs = copy_angles;
					world_find_model_instance_point(&p1, &light_obj->pos, pm, pmi, *smv, &heavy_obj->orient, &heavy_obj->pos);

					mc.p0 = &p0;
					mc.p1 = &p1;
					// mc.pos = zero	// in submodel RF

					mc.orient = &vmd_identity_matrix;
					mc.submodel_num = *smv;

					if ( model_collide(&mc) ) {
						if ( mc.hit_dist < debris_hit_info->hit_time ) {
							mc_ret_val = 1;

							// set up debris_hit_info common
							set_hit_struct_info(debris_hit_info, &mc, SUBMODEL_ROT_HIT);
							model_instance_find_world_point(&debris_hit_info->hit_pos, &mc.hit_point, mc.model_num, mc.model_instance_num, mc.hit_submodel, &heavy_obj->orient, &zero);

							// set up debris_hit_info for rotating submodel
							if (debris_hit_info->edge_hit == 0) {
								model_instance_find_obj_dir(&debris_hit_info->collision_normal, &mc.hit_normal, heavy_obj, mc.hit_submodel);
							}

							// find position in submodel RF of light object at collison
							vec3d int_light_pos, diff;
							vm_vec_sub(&diff, mc.p1, mc.p0);
							vm_vec_scale_add(&int_light_pos, mc.p0, &diff, mc.hit_dist);
							model_instance_find_world_point(&debris_hit_info->light_collision_cm_pos, &int_light_pos, mc.model_num, mc.model_instance_num, mc.hit_submodel, &heavy_obj->orient, &zero);
						}
					}
					// Don't look at this submodel again
					pmi->submodel[*smv].collision_checked = true;
				}

			}

			// Recover and do usual ship_ship collision, but without rotating submodels
			mc.flags = copy_flags;
			*mc.p0 = copy_p0;
			*mc.p1 = copy_p1;
			mc.orient = &heavy_obj->orient;

			// usual ship_ship collision test
			if ( model_collide(&mc) )	{
				// check if this is the earliest hit
				if (mc.hit_dist < debris_hit_info->hit_time) {
					mc_ret_val = 1;

					set_hit_struct_info(debris_hit_info, &mc, SUBMODEL_NO_ROT_HIT);

					// get collision normal if not edge hit
					if (debris_hit_info->edge_hit == 0) {
						model_instance_find_obj_dir(&debris_hit_info->collision_normal, &mc.hit_normal, heavy_obj, mc.hit_submodel);
					}

					// find position in submodel RF of light object at collison
					vec3d diff;
					vm_vec_sub(&diff, mc.p1, mc.p0);
					vm_vec_scale_add(&debris_hit_info->light_collision_cm_pos, mc.p0, &diff, mc.hit_dist);

				}
			}
		}

	} else {
		// Debris is heavier obj
		mc.model_instance_num = -1;
		mc.model_num = Debris[num].model_num;		// Fill in the model to check
		mc.submodel_num = Debris[num].submodel_num;
		model_clear_instance( mc.model_num );
		mc.orient = &pdebris->orient;				// The object's orient
		mc.radius = model_get_core_radius(Ship_info[Ships[ship_obj->instance].ship_info_index].model_num);

		// check for collision between debris model and ship sphere
		mc.flags = (MC_CHECK_MODEL | MC_SUBMODEL | MC_CHECK_SPHERELINE);

		mc_ret_val = model_collide(&mc);

		if (mc_ret_val) {
			set_hit_struct_info(debris_hit_info, &mc, SUBMODEL_NO_ROT_HIT);

			// set normal if not edge hit
			if ( !debris_hit_info->edge_hit ) {
				vm_vec_unrotate(&debris_hit_info->collision_normal, &mc.hit_normal, &heavy->orient);
			}

			// find position in submodel RF of light object at collison
			vec3d diff;
			vm_vec_sub(&diff, mc.p1, mc.p0);
			vm_vec_scale_add(&debris_hit_info->light_collision_cm_pos, mc.p0, &diff, mc.hit_dist);

		}
	}


	if ( mc_ret_val )	{

		// SET PHYSICS PARAMETERS
		// already have (hitpos - heavy) and light_cm_pos
		// get heavy cm pos - already have light_cm_pos
		debris_hit_info->heavy_collision_cm_pos = zero;

		// get r_heavy and r_light
		debris_hit_info->r_heavy = debris_hit_info->hit_pos;
		vm_vec_sub(&debris_hit_info->r_light, &debris_hit_info->hit_pos, &debris_hit_info->light_collision_cm_pos);

		// set normal for edge hit
		if ( debris_hit_info->edge_hit ) {
			vm_vec_copy_normalize(&debris_hit_info->collision_normal, &debris_hit_info->r_light);
			vm_vec_negate(&debris_hit_info->collision_normal);
		}

		// get world hitpos
		vm_vec_add(hitpos, &debris_hit_info->heavy->pos, &debris_hit_info->r_heavy);

		return 1;
	} else {
		// no hit
		return 0;
	}
}

// ---------------------------------------------------------------------------------------
// debris_get_team()
//
//	Return the team field for a debris object
//
int debris_get_team(object *objp)
{
	Assert( objp->type == OBJ_DEBRIS );
	Assert( objp->instance >= 0 && objp->instance < MAX_DEBRIS_PIECES );
	return Debris[objp->instance].team;
}

// fills in debris physics properties when created, specifically mass and moment of inertia
void calc_debris_physics_properties( physics_info *pi, vec3d *mins, vec3d *maxs )
{
	float dx, dy, dz, mass;
	dx = maxs->xyz.x - mins->xyz.x;
	dy = maxs->xyz.y - mins->xyz.y;
	dz = maxs->xyz.z - mins->xyz.z;

	// John, with new bspgen, just set pi->mass = mass
	mass = 0.12f * dx * dy * dz;
	pi->mass = (float) pow(mass, 0.6666667f) * 4.65f;

	pi->I_body_inv.vec.rvec.xyz.x = 12.0f / (pi->mass *  (dy*dy + dz*dz));
	pi->I_body_inv.vec.rvec.xyz.y = 0.0f;
	pi->I_body_inv.vec.rvec.xyz.z = 0.0f;

	pi->I_body_inv.vec.uvec.xyz.x = 0.0f;
	pi->I_body_inv.vec.uvec.xyz.y = 12.0f / (pi->mass *  (dx*dx + dz*dz));
	pi->I_body_inv.vec.uvec.xyz.z = 0.0f;

	pi->I_body_inv.vec.fvec.xyz.x = 0.0f;
	pi->I_body_inv.vec.fvec.xyz.y = 0.0f;
	pi->I_body_inv.vec.fvec.xyz.z = 12.0f / (pi->mass *  (dx*dx + dy*dy));
}
