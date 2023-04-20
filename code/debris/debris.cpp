/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "cmdline/cmdline.h"
#include "debris/debris.h"
#include "fireball/fireballs.h"
#include "freespace.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "io/timer.h"
#include "network/multi.h"
#include "network/multimsgs.h"
#include "network/multiutil.h"
#include "object/objcollide.h"
#include "object/objectsnd.h"
#include "particle/particle.h"
#include "radar/radar.h"
#include "radar/radarsetup.h"
#include "render/3d.h"
#include "scripting/global_hooks.h"
#include "scripting/hook_api.h"
#include "scripting/scripting.h"
#include "scripting/api/objs/vecmath.h"
#include "ship/ship.h"
#include "ship/shipfx.h"
#include "species_defs/species_defs.h"
#include "tracing/Monitor.h"
#include "utils/Random.h"
#include "weapon/weapon.h"

constexpr int MIN_RADIUS_FOR_PERSISTENT_DEBRIS = 50;	// ship radius at which debris from it becomes persistant
constexpr int DEBRIS_SOUND_DELAY = 2000;				// time to start debris sound after created

int Num_hull_pieces;	// number of hull pieces in existence
						// we now maintain a kind of "virtual" Hull_debris_list,
						// but all we really need to know is how many hull pieces there are and whether a debris piece is on it

						// This list holds debris pieces that are set to expire when their lifetime runs out;
						// pieces that were placed by FREDers (i.e. that have the DoNotExpire flag) should not be on it.

SCP_vector<debris> Debris;

int Debris_inited = 0;

int Debris_model = -1;
int Debris_vaporize_model = -1;
int Debris_num_submodels = 0;

#define	DEBRIS_INDEX(dp) (int)(dp-Debris.data())


/**
 * Start the sequence of a piece of debris writhing in unholy agony!!!
 */
static void debris_start_death_roll(object *debris_obj, debris *debris_p, vec3d *hitpos = nullptr)
{
	if (debris_p->is_hull)	{
		// tell everyone else to blow up the piece of debris
		if( MULTIPLAYER_MASTER )
			send_debris_update_packet(debris_obj,DEBRIS_UPDATE_NUKE);

		int fireball_type = fireball_ship_explosion_type(&Ship_info[debris_p->ship_info_index]);
		if(fireball_type < 0) {
			fireball_type = FIREBALL_EXPLOSION_LARGE1 + Random::next(FIREBALL_NUM_LARGE_EXPLOSIONS);
		}
		fireball_create( &debris_obj->pos, fireball_type, FIREBALL_LARGE_EXPLOSION, OBJ_INDEX(debris_obj), debris_obj->radius*1.75f);

		// only play debris destroy sound if hull piece and it has been around for at least 2 seconds
		if ( Missiontime > debris_p->time_started + 2*F1_0 ) {
			auto snd_id = Ship_info[debris_p->ship_info_index].debris_explosion_sound;
			if (snd_id.isValid()) {
				snd_play_3d( gamesnd_get_game_sound(snd_id), &debris_obj->pos, &View_position, debris_obj->radius );
			}
		}
	}

	if (scripting::hooks::OnDebrisDeath->isActive()) {
		scripting::hooks::OnDebrisDeath->run(scripting::hook_param_list(
			scripting::hook_param("Debris", 'o', debris_obj),
			scripting::hook_param("Hitpos",
				'o',
				scripting::api::l_Vector.Set(hitpos ? *hitpos : vmd_zero_vector),
				hitpos != nullptr)));
	}

    debris_obj->flags.set(Object::Object_Flags::Should_be_dead);
}

/**
 * This will get called at the start of each level.
 */
void debris_init()
{
	if ( !Debris_inited ) {
		Debris_inited = 1;
	}

	Debris_model = -1;
	Debris_vaporize_model = -1;
	Debris_num_submodels = 0;
		
	// Reset everything between levels
	Debris.clear();
	Debris.reserve(SOFT_LIMIT_DEBRIS_PIECES);

	Num_hull_pieces = 0;
}

/**
 * Page in debris bitmaps at level load
 */
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

/**
 * Add item to [virtual] Hull_debris_list
 */
void debris_add_to_hull_list(debris *db)
{
	if ( db->is_hull && !db->flags[Debris_Flags::OnHullDebrisList] ) {
		Num_hull_pieces++;
		db->flags.set(Debris_Flags::OnHullDebrisList);
	}
}

/**
 * Remove item from [virtual] Hull_debris_list
 */
void debris_remove_from_hull_list(debris *db)
{
	if ( db->is_hull && db->flags[Debris_Flags::OnHullDebrisList] ) {
		Num_hull_pieces--;
		db->flags.remove(Debris_Flags::OnHullDebrisList);
	}
}

/**
 * Delete the debris object.  
 * This is only ever called via obj_delete().  Do not call directly.
 * Use debris_start_death_roll() if you want to force a debris piece to die.
 */
void debris_delete( object * obj )
{
	int		num;
	debris	*db;

	num = obj->instance;

	Assert(num >= 0 && num < (int)Debris.size());
	Assert(Debris[num].objnum == OBJ_INDEX(obj));

	db = &Debris[num];

	if (db->model_instance_num >= 0) {
		model_delete_instance(db->model_instance_num);
	}

	if ( db->is_hull ) {
		debris_remove_from_hull_list(db);
	}

	db->flags.reset();
	db->objnum = -1;
}


/**
 * Do various updates to debris:  check if time to die, start fireballs
 * Maybe delete debris if it's very far away from player.
 *
 * @param obj			pointer to debris object
 * @param frame_time	time elapsed since last debris_move() called
 */
void debris_process_post(object * obj, float frame_time)
{
	int num = obj->instance;
	int objnum = OBJ_INDEX(obj);

	Assert(num >= 0 && num < (int)Debris.size());
	Assert(Debris[num].objnum == objnum);

	debris *db = &Debris[num];

	if ( db->is_hull ) {
		radar_plot_object( obj );

		if ( timestamp_elapsed(db->sound_delay) ) {
			obj_snd_assign(objnum, db->ambient_sound, &vmd_zero_vector);
			db->sound_delay = TIMESTAMP::invalid();
		}
	}

	if (!db->flags[Debris_Flags::DoNotExpire]) {
		Assertion(db->lifeleft >= 0.0f, "A ship with a negative lifeleft should also have the DoNotExpire flag!");

		db->lifeleft -= frame_time;
		if (db->lifeleft < 0.0f) {
			debris_start_death_roll(obj, db);
		}
	}

	// ================== DO THE ELECTRIC ARCING STUFF =====================
	if ( db->arc_frequency <= 0 )	{
		return;			// If arc_frequency <= 0, this piece has no arcs on it
	}

	if ( !timestamp_elapsed(db->fire_timeout) && timestamp_elapsed(db->next_fireball))	{		

		db->next_fireball = _timestamp_rand(db->arc_frequency,db->arc_frequency*2 );
		db->arc_frequency += 100;	

		if (db->is_hull)	{

			int n, n_arcs = Random::next(1, 3);		// Create 1-3 sparks

			vec3d v1 = submodel_get_random_point(db->model_num, db->submodel_num);
			vec3d v2 = submodel_get_random_point(db->model_num, db->submodel_num);
			vec3d v3 = submodel_get_random_point(db->model_num, db->submodel_num);
			vec3d v4 = submodel_get_random_point(db->model_num, db->submodel_num);

			n = 0;

			int lifetime = Random::next(100, 1000);

			// Create the spark effects
			for (int i=0; i<MAX_DEBRIS_ARCS; ++i)	{
				if ( !db->arc_timestamp[i].isValid() )	{

					db->arc_timestamp[i] = _timestamp(lifetime);	// live up to a second

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
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_05), &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  500 )	{
				// 0.75 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_04), &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  250 )	{
				// 0.50 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_03), &snd_pos, &View_position, obj->radius );
			} else if ( lifetime >  100 )	{
				// 0.25 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_02), &snd_pos, &View_position, obj->radius );
			} else {
				// 0.10 second effect
				snd_play_3d( gamesnd_get_game_sound(GameSounds::DEBRIS_ARC_01), &snd_pos, &View_position, obj->radius );
			}
		}
	}

	for (int i=0; i<MAX_DEBRIS_ARCS; ++i)	{
		if ( db->arc_timestamp[i].isValid() )	{
			if ( timestamp_elapsed( db->arc_timestamp[i] ) )	{
				// Kill off the spark
				db->arc_timestamp[i] = TIMESTAMP::invalid();
			} else {
				// Maybe move a vertex....  20% of the time maybe?
				int mr = Random::next();
				if ( mr < Random::MAX_VALUE/5 )	{
					db->arc_pts[i][mr % 2] = submodel_get_random_point(db->model_num, db->submodel_num);
				}
			}
		}
	}
}

/**
 * Locate the oldest hull debris chunk.  Search through the Hull_debris_list, which is a list
 * of all the hull debris chunks.
 */
int debris_find_oldest()
{
	int		oldest_index;
	fix		oldest_time;

	oldest_index = -1;
	oldest_time = 0x7fffffff;

	for (auto &db: Debris) {
		if (db.flags[Debris_Flags::OnHullDebrisList]) {
			if ((db.time_started < oldest_time) && !(Objects[db.objnum].flags[Object::Object_Flags::Should_be_dead])) {
				oldest_index = DEBRIS_INDEX(&db);
				oldest_time = db.time_started;
			}
		}
	}

	return oldest_index;
}

#define	DEBRIS_ROTVEL_SCALE	5.0f
void calc_debris_physics_properties( physics_info *pi, vec3d *min, vec3d *max, float density );

MONITOR(NumSmallDebris)
MONITOR(NumHullDebris)

/**
 * Create debris from an object
 *
 * @param source_obj	Source object
 * @param model_num		Model number
 * @param submodel_num	Sub-model number
 * @param pos			Position in global coordinates
 * @param exp_center	Explosion center in global coordinates
 * @param hull_flag		Whether this debris is a chunk of a ship's hull (as opposed to shrapnel)
 * @param exp_force		Explosion force, used to assign velocity to pieces. 1.0f assigns velocity like before. 2.0f assigns twice as much to non-inherited part of velocity
 * @param source_subsys	The subsystem this debris came from, if any
 */
object *debris_create(object *source_obj, int model_num, int submodel_num, vec3d *pos, vec3d *exp_center, bool hull_flag, float exp_force, ship_subsys* source_subsys)
{
	int             parent_objnum;
	object  *obj;
	ship            *shipp;
	bool vaporize;

	parent_objnum = OBJ_INDEX(source_obj);

	Assert( (source_obj->type == OBJ_SHIP ) || (source_obj->type == OBJ_GHOST));
	Assert( source_obj->instance >= 0 && source_obj->instance < MAX_SHIPS );	
	shipp = &Ships[source_obj->instance];

	vaporize = (shipp->flags[Ship::Ship_Flags::Vaporize]);

	obj = debris_create_only(parent_objnum, shipp->ship_info_index, shipp->alt_type_index, shipp->team, -1.0f, -1, model_num, submodel_num, pos, nullptr, hull_flag, vaporize, shipp->debris_damage_type_idx);
	if (obj != nullptr)
	{
		debris_create_set_velocity(&Debris[obj->instance], shipp, exp_center, exp_force, source_subsys);
		debris_create_fire_hook(obj, source_obj);
	}

	return obj;
}

object *debris_create_only(int parent_objnum, int parent_ship_class, int alt_type_index, int team, float hull_strength, int spark_timeout, int model_num, int submodel_num, vec3d *pos, matrix *orient, bool hull_flag, bool vaporize, int damage_type_idx)
{
	int		objnum;
	object	*obj;
	debris	*db;
	ship_info *sip;
	matrix orient_buf;

	// try to maintain our soft limit
	if (hull_flag && (Num_hull_pieces >= SOFT_LIMIT_DEBRIS_PIECES)) {
		// cause oldest hull debris chunk to blow up
		int oldest_n = debris_find_oldest();
		if (oldest_n >= 0)
			debris_start_death_roll(&Objects[Debris[oldest_n].objnum], &Debris[oldest_n]);
	}

	int n = 0;
	for (auto &db_temp: Debris) {
		if ( !(db_temp.flags[Debris_Flags::Used]) )
			break;
		++n;
	}

	// we might have to create a new slot
	if (n == (int)Debris.size()) {
		Debris.emplace_back();
	}

	db = &Debris[n];


	// validate or deduce some parameters

	if ( pos == nullptr ) {
		if (parent_objnum >= 0) {
			pos = &Objects[parent_objnum].pos;
		} else {
			pos = &vmd_zero_vector;
		}
	}

	if ( orient == nullptr ) {
		if (parent_objnum >= 0 && hull_flag) {
			orient = &Objects[parent_objnum].orient;
		} else {
			// non-hull debris has no relation to its parent orientation
			vec3d rand;
			vm_vec_rand_vec(&rand);
			vm_vector_2_matrix(&orient_buf, &rand);
			orient = &orient_buf;
		}
	}

	sip = (parent_ship_class < 0) ? nullptr : &Ship_info[parent_ship_class];
	if (hull_flag && !sip)
	{
		Warning(LOCATION, "Cannot create hull debris without a ship class!");
		return nullptr;
	}

	if (hull_strength < 0.0f)
	{
		if (parent_objnum >= 0 && Objects[parent_objnum].type == OBJ_SHIP)
			hull_strength = Ships[Objects[parent_objnum].instance].ship_max_hull_strength / 8.0f;
		else
			hull_strength = 10.0f;
	}

	if (hull_flag && (model_num < 0 || submodel_num < 0))
	{
		Warning(LOCATION, "Model and submodel numbers must be specified for hull debris!");
		return nullptr;
	}

	if (team < 0 && hull_flag)
	{
		if (parent_objnum >= 0 && Objects[parent_objnum].type == OBJ_SHIP)
			team = Ships[Objects[parent_objnum].instance].team;
		else if (Player_ship)
			team = Player_ship->team;
		else
			team = 0;
	}

	if (damage_type_idx < 0)
	{
		if (parent_objnum >= 0 && Objects[parent_objnum].type == OBJ_SHIP)
			damage_type_idx = Ships[Objects[parent_objnum].instance].debris_damage_type_idx;
	}

	// done validation


	if (!hull_flag) {
		if (model_num >= 0) {
			db->model_num = model_num;
			db->submodel_num = (sip == nullptr || sip->generic_debris_num_submodels <= 0) ? 0 : Random::next(sip->generic_debris_num_submodels);
		}
		else if (vaporize) {
			db->model_num = Debris_vaporize_model;
			db->submodel_num = Random::next(Debris_num_submodels);
		}
		else {
			db->model_num = Debris_model;
			db->submodel_num = Random::next(Debris_num_submodels);
		}
	}
	else {
		db->model_num = model_num;
		db->submodel_num = submodel_num;
	}

	float radius = submodel_get_radius(db->model_num, db->submodel_num);

	// if its generic, maybe cull it if its too small and far
	if (!hull_flag) {
		float dist = vm_vec_dist_quick(pos, &Eye_position);
		// Make vaporize debris seen from farther away
		if (vaporize) {
			dist /= 2.0f;
		}
		if (dist > radius * 200.0f) {
			return nullptr;
		}
	}

	// Create Debris piece n!
	if ( hull_flag ) {
		if (Random::next() < (Random::MAX_VALUE/6))	// Make some pieces blow up shortly after explosion.
			db->lifeleft = 2.0f * (frand()) + 0.5f;
		else {
			db->flags.set(Debris_Flags::DoNotExpire);
			db->lifeleft = -1.0f;		// large hull pieces stay around forever
		}
	} else {
		// small non-hull pieces should stick around longer the larger they are
		// sqrtf should make sure its never too crazy long
		db->lifeleft = (frand() * 2.0f + 0.1f) * sqrtf(radius);
	}

	//WMC - Oh noes, we may need to change lifeleft
	if(hull_flag)
	{
		if(sip->debris_min_lifetime >= 0.0f && sip->debris_max_lifetime >= 0.0f)
		{
			db->lifeleft = (( sip->debris_max_lifetime - sip->debris_min_lifetime ) * frand()) + sip->debris_min_lifetime;
		}
		else if(sip->debris_min_lifetime >= 0.0f)
		{
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
	db->flags.set(Debris_Flags::Used);
	db->is_hull = hull_flag;
	db->source_objnum = parent_objnum;
	db->damage_type_idx = damage_type_idx;
	db->ship_info_index = parent_ship_class;
	db->team = team;
	db->ambient_sound = (sip == nullptr) ? gamesnd_id(-1) : sip->debris_ambient_sound;
	db->fire_timeout = TIMESTAMP::never();	// if not changed, timestamp_elapsed() will return false
	db->time_started = Missiontime;
	db->species = (sip == nullptr) ? -1 : sip->species;
	db->parent_alt_name = alt_type_index;
	db->damage_mult = 1.0f;

	for (int i=0; i<MAX_DEBRIS_ARCS; ++i)	{	// NOLINT
		db->arc_timestamp[i] = TIMESTAMP::invalid();
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

	db->next_fireball = _timestamp_rand(500,2000);	//start one 1/2 - 2 secs later

	flagset<Object::Object_Flags> default_flags;
	default_flags.set(Object::Object_Flags::Renders);
	default_flags.set(Object::Object_Flags::Physics);
	default_flags.set(Object::Object_Flags::Collides, hull_flag != 0);

	objnum = obj_create(OBJ_DEBRIS, parent_objnum, n, orient, pos, radius, default_flags, false);
	if ( objnum == -1 ) {
		mprintf(("Couldn't create debris object -- out of object slots\n"));
		return nullptr;
	}

	db->objnum = objnum;
	obj = &Objects[objnum];

	db->model_instance_num = hull_flag ? model_create_instance(objnum, db->model_num) : -1;

	// assign the network signature.  The signature will be 0 for non-hull pieces, but since that
	// is our invalid signature, it should be okay.
	obj->net_signature = 0;

	if ( (Game_mode & GM_MULTIPLAYER) && hull_flag ) {
		obj->net_signature = multi_get_next_network_signature( MULTI_SIG_DEBRIS );
	}

	obj->hull_strength = hull_strength;

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

		db->sound_delay = _timestamp(DEBRIS_SOUND_DELAY);

		// limit the amount of time that fireballs appear
		// let fireball length be linked to radius of ship.  Range is .33 radius => 3.33 radius seconds.
		if (spark_timeout >= 0) {
			db->fire_timeout = _timestamp(spark_timeout);
		} else if (parent_objnum >= 0) {
			float t = 1000*Objects[parent_objnum].radius/3 + Random::next(fl2i(1000*3*Objects[parent_objnum].radius));
			db->fire_timeout = _timestamp(fl2i(t));		// fireballs last from 5 - 30 seconds
		} else {
			db->fire_timeout = TIMESTAMP::immediate();
		}

		if (parent_objnum >= 0 && Objects[parent_objnum].radius >= MIN_RADIUS_FOR_PERSISTENT_DEBRIS) {
			db->flags.set(Debris_Flags::DoNotExpire);
		} else {
			debris_add_to_hull_list(db);
		}
	}

	if (hull_flag) {
		MONITOR_INC(NumHullDebris,1);
	} else {
		MONITOR_INC(NumSmallDebris,1);
	}

	return obj;
}

void debris_create_set_velocity(debris *db, ship *source_shipp, vec3d *exp_center, float exp_force, ship_subsys* source_subsys)
{
	auto obj = &Objects[db->objnum];
	auto source_obj = (source_shipp == nullptr) ? nullptr : &Objects[source_shipp->objnum];
	physics_info *pi = &obj->phys_info;
	vec3d rotvel, radial_vel, to_center;

	if ( exp_center )
		vm_vec_sub( &to_center, &obj->pos, exp_center );
	else
		vm_vec_zero(&to_center);

	float scale;
	if ( db->is_hull )	{
		scale = exp_force * i2fl(Random::next(10, 29));	// for radial_vel away from location of blast center

		// set up physics mass and I_inv for hull debris pieces
		auto pm = model_get(db->model_num);
		vec3d *min, *max;
		min = &pm->submodel[db->submodel_num].min;
		max = &pm->submodel[db->submodel_num].max;
		float density = source_subsys != nullptr ? source_subsys->system_info->density : Ship_info[source_shipp->ship_info_index].debris_density;
		calc_debris_physics_properties( &obj->phys_info, min, max, density);
	}
	else {
		scale = exp_force * i2fl(Random::next(10, 29));	// for radial_vel away from blast center (non-hull)
	}

	if ( vm_vec_mag_squared( &to_center ) < 0.1f )	{
		vm_vec_rand_vec_quick(&radial_vel);
		vm_vec_scale(&radial_vel, scale );
	}
	else {
		vm_vec_normalize(&to_center);
		vm_vec_copy_scale(&radial_vel, &to_center, scale );
	}

	if (source_obj)
	{
		// DA: here we need to vel_from_rot = w x to_center, where w is world is unrotated to world coords and offset is the 
		// displacement fromt the center of the parent object to the center of the debris piece
		vec3d world_rotvel, vel_from_rotvel;
		vm_vec_unrotate ( &world_rotvel, &source_obj->phys_info.rotvel, &source_obj->orient );
		vm_vec_cross ( &vel_from_rotvel, &world_rotvel, &to_center );
		vm_vec_scale ( &vel_from_rotvel, DEBRIS_ROTVEL_SCALE);

		vm_vec_add (&obj->phys_info.vel, &radial_vel, &source_obj->phys_info.vel);
		vm_vec_add2(&obj->phys_info.vel, &vel_from_rotvel);
	}


	float radius = submodel_get_radius(db->model_num, db->submodel_num);

	// make sure rotational velocity does not get too high
	if (radius < 1.0) {
		radius = 1.0f;
	}

	scale = (6.0f + i2fl(Random::next(4))) / radius;

	vm_vec_rand_vec_quick(&rotvel);
	vm_vec_scale(&rotvel, scale);

	pi->rotvel = rotvel;
	pi->flags |= (PF_DEAD_DAMP | PF_BALLISTIC);
	check_rotvel_limit( &obj->phys_info );

	// check that debris is not created with too high a velocity
	if (db->is_hull)
	{
		shipfx_debris_limit_speed(db, source_shipp);
	}

	// blow out his reverse thrusters. Or drag, same thing.
	pi->rotdamp = 10000.0f;
	if (source_shipp != nullptr) {
		pi->gravity_const = Ship_info[source_shipp->ship_info_index].debris_gravity_const;
	} else {
		pi->gravity_const = 1.0f;
	}
	

	vm_vec_zero(&pi->max_vel);		// make so he can't turn on his own VOLITION anymore.
	vm_vec_zero(&pi->max_rotvel);	// make so he can't change speed on his own VOLITION anymore.

	// ensure vel is valid
	Assert( !vm_is_vec_nan(&obj->phys_info.vel) );
}

void debris_create_fire_hook(object *obj, object *source_obj)
{
	if (scripting::hooks::OnDebrisCreated->isActive()) {
		scripting::hooks::ShipSourceConditions conditions;
		conditions.source_shipp = (source_obj != nullptr && source_obj->type == OBJ_SHIP) ? &Ships[source_obj->instance] : nullptr;
		scripting::hooks::OnDebrisCreated->run(std::move(conditions),
			scripting::hook_param_list(
				scripting::hook_param("Debris", 'o', obj),
				scripting::hook_param("Source", 'o', source_obj)));
	}
}

/**
 * Alas, poor debris_obj got whacked.  Fortunately, we know who did it, where and how hard, so we
 * can do something about it.
 */
void debris_hit(object *debris_obj, object * /*other_obj*/, vec3d *hitpos, float damage, vec3d* force)
{
	debris	*debris_p = &Debris[debris_obj->instance];

	// Do a little particle spark shower to show we hit
	{
		particle::particle_emitter pe;

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
		particle::emit( &pe, particle::PARTICLE_FIRE, 0 );
	}

	// multiplayer clients bail here
	if(MULTIPLAYER_CLIENT){
		return;
	}

	if ( damage < 0.0f ) {
		damage = 0.0f;
	}

	if (hitpos && force && The_mission.ai_profile->flags[AI::Profile_Flags::Whackable_debris]) {
		vec3d rel_hit_pos = *hitpos - debris_obj->pos;
		physics_calculate_and_apply_whack(force, &rel_hit_pos, &debris_obj->phys_info, &debris_obj->orient, &debris_obj->phys_info.I_body_inv);
	}

	debris_obj->hull_strength -= damage;

	if (debris_obj->hull_strength < 0.0f) {
		debris_start_death_roll(debris_obj, debris_p, hitpos);
	} else {
		// otherwise, give all the other players an update on the debris
		if(MULTIPLAYER_MASTER){
			send_debris_update_packet(debris_obj,DEBRIS_UPDATE_UPDATE);
		}
	}
}

/**
 * See if poor debris object *obj got whacked by evil *other_obj at point *hitpos.
 * NOTE: debris_hit_info pointer NULL for debris:weapon collision, otherwise debris:ship collision.
 * @return true if hit, else return false.
 */
int debris_check_collision(object *pdebris, object *other_obj, vec3d *hitpos, collision_info_struct *debris_hit_info, vec3d* hitNormal)
{
	mc_info	mc;

	Assert( pdebris->type == OBJ_DEBRIS );

	int num = pdebris->instance;

	Assert( num >= 0 && num < (int)Debris.size() );
	Assert( Debris[num].objnum == OBJ_INDEX(pdebris));	

	// debris_hit_info NULL - so debris-weapon collision
	if ( debris_hit_info == NULL ) {
		// debris weapon collision
		Assert( other_obj->type == OBJ_WEAPON );
		mc.model_instance_num = -1;
		mc.model_num = Debris[num].model_num;	// Fill in the model to check
		mc.submodel_num = Debris[num].submodel_num;
		mc.orient = &pdebris->orient;					// The object's orient
		mc.pos = &pdebris->pos;							// The object's position
		mc.p0 = &other_obj->last_pos;				// Point 1 of ray to check
		mc.p1 = &other_obj->pos;					// Point 2 of ray to check
		mc.flags = (MC_CHECK_MODEL | MC_SUBMODEL);

		if (model_collide(&mc)) {
			*hitpos = mc.hit_point_world;

			if (hitNormal)
			{
				vec3d normal;

				if (mc.model_instance_num >= 0)
					model_instance_local_to_global_dir(&normal, &mc.hit_normal, mc.model_instance_num, mc.hit_submodel, mc.orient);
				else
					model_local_to_global_dir(&normal, &mc.hit_normal, mc.model_num, mc.hit_submodel, mc.orient);

				*hitNormal = normal;
			}
		}

		weapon *wp = &Weapons[other_obj->instance];
		wp->collisionInfo = new mc_info;	// The weapon will free this memory later
		*wp->collisionInfo = mc;

		return mc.num_hits;
	}
	
	// debris ship collision -- use debris_hit_info to calculate physics
	object *pship_obj = other_obj;
	Assert( pship_obj->type == OBJ_SHIP );

	object *heavy = debris_hit_info->heavy;
	object *lighter = debris_hit_info->light;
	object *heavy_obj = heavy;
	object *light_obj = lighter;

	vec3d zero, p0, p1;
	vm_vec_zero(&zero);
	vm_vec_sub(&p0, &lighter->last_pos, &heavy->last_pos);
	vm_vec_sub(&p1, &lighter->pos, &heavy->pos);

	mc.pos = &zero;								// The object's position
	mc.p0 = &p0;									// Point 1 of ray to check
	mc.p1 = &p1;									// Point 2 of ray to check

	// find the light object's position in the heavy object's reference frame at last frame and also in this frame.
	vec3d p0_temp, p0_rotated;
		
	// Collision detection from rotation enabled if at rotation is less than 30 degree in frame
	// This should account for all ships
	if ( (vm_vec_mag_squared(&heavy->phys_info.rotvel) * flFrametime*flFrametime) < (PI*PI/36) ) {
		// collide_rotate calculate (1) start position and (2) relative velocity
		debris_hit_info->collide_rotate = true;
		vm_vec_rotate(&p0_temp, &p0, &heavy->last_orient);
		vm_vec_unrotate(&p0_rotated, &p0_temp, &heavy->orient);
		mc.p0 = &p0_rotated;				// Point 1 of ray to check
		vm_vec_sub(&debris_hit_info->light_rel_vel, &p1, &p0_rotated);
		vm_vec_scale(&debris_hit_info->light_rel_vel, 1/flFrametime);
	} else {
		debris_hit_info->collide_rotate = false;
		vm_vec_sub(&debris_hit_info->light_rel_vel, &lighter->phys_info.vel, &heavy->phys_info.vel);
	}

	int mc_ret_val = 0;

	if ( debris_hit_info->heavy == pship_obj ) {	// ship is heavier, so debris is sphere. Check sphere collision against ship poly model
		mc.model_instance_num = Ships[pship_obj->instance].model_instance_num;
		mc.model_num = Ship_info[Ships[pship_obj->instance].ship_info_index].model_num;	// Fill in the model to check
		mc.orient = &pship_obj->orient;								// The object's orient
		mc.radius = pdebris->radius;
		mc.flags = (MC_CHECK_MODEL | MC_CHECK_SPHERELINE);

		// copy important data
		int copy_flags = mc.flags;  // make a copy of start end positions of sphere in  big ship RF
		vec3d copy_p0, copy_p1;
		copy_p0 = *mc.p0;
		copy_p1 = *mc.p1;

		// first test against the sphere - if this fails then don't do any submodel tests
		mc.flags = MC_ONLY_SPHERE | MC_CHECK_SPHERELINE;

		if (model_collide(&mc)) {

			// Set earliest hit time
			debris_hit_info->hit_time = FLT_MAX;

			auto pmi = model_get_instance(Ships[heavy_obj->instance].model_instance_num);
			auto pm = model_get(pmi->model_num);

			// Do collision the cool new way
			if ( debris_hit_info->collide_rotate ) {
				// We collide with the sphere, find the list of moving submodels and test one at a time
				SCP_vector<int> submodel_vector;
				model_get_moving_submodel_list(submodel_vector, heavy_obj);

				// turn off all moving submodels, collide against only 1 at a time.
				// turn off collision detection for all moving submodels
				for (auto submodel : submodel_vector) {
					pmi->submodel[submodel].collision_checked = true;
				}

				// Only check single submodel now, since children of moving submodels are handled as moving as well
				mc.flags = copy_flags | MC_SUBMODEL;

				if (Ship_info[Ships[pship_obj->instance].ship_info_index].collision_lod > -1) {
					mc.lod = Ship_info[Ships[pship_obj->instance].ship_info_index].collision_lod;
				}

				// check each submodel in turn
				for (auto submodel: submodel_vector) {
					auto smi = &pmi->submodel[submodel];

					// turn on just one submodel for collision test
					smi->collision_checked = false;

					// find the start and end positions of the sphere in submodel RF
					model_instance_global_to_local_point(&p0, &light_obj->last_pos, pm, pmi, submodel, &heavy_obj->last_orient, &heavy_obj->last_pos, true);
					model_instance_global_to_local_point(&p1, &light_obj->pos, pm, pmi, submodel, &heavy_obj->orient, &heavy_obj->pos);

					mc.p0 = &p0;
					mc.p1 = &p1;

					mc.orient = &vmd_identity_matrix;
					mc.submodel_num = submodel;

					if ( model_collide(&mc) ) {
						if ( mc.hit_dist < debris_hit_info->hit_time ) {
							mc_ret_val = 1;

							// set up debris_hit_info common
							set_hit_struct_info(debris_hit_info, &mc, true);
							model_instance_local_to_global_point(&debris_hit_info->hit_pos, &mc.hit_point, pm, pmi, mc.hit_submodel, &heavy_obj->orient, &zero);

							// set up debris_hit_info for rotating submodel
							if (!debris_hit_info->edge_hit) {
								model_instance_local_to_global_dir(&debris_hit_info->collision_normal, &mc.hit_normal, pm, pmi, mc.hit_submodel, &heavy_obj->orient);
							}

							// find position in submodel RF of light object at collison
							vec3d int_light_pos, diff;
							vm_vec_sub(&diff, mc.p1, mc.p0);
							vm_vec_scale_add(&int_light_pos, mc.p0, &diff, mc.hit_dist);
							model_instance_local_to_global_point(&debris_hit_info->light_collision_cm_pos, &int_light_pos, pm, pmi, mc.hit_submodel, &heavy_obj->orient, &zero);
						}
					}

					// Don't look at this submodel again
					smi->collision_checked = true;
				}
			}

			// Now complete base model collision checks that do not take into account rotating submodels.
			mc.flags = copy_flags;
			*mc.p0 = copy_p0;
			*mc.p1 = copy_p1;
			mc.orient = &heavy_obj->orient;

			// usual ship_ship collision test
			if ( model_collide(&mc) )	{
				// check if this is the earliest hit
				if (mc.hit_dist < debris_hit_info->hit_time) {
					mc_ret_val = 1;

					set_hit_struct_info(debris_hit_info, &mc, false);

					// get collision normal if not edge hit
					if (!debris_hit_info->edge_hit) {
						model_instance_local_to_global_dir(&debris_hit_info->collision_normal, &mc.hit_normal, pm, pmi, mc.hit_submodel, &heavy_obj->orient);
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
		mc.orient = &pdebris->orient;				// The object's orient
		mc.radius = model_get_core_radius(Ship_info[Ships[pship_obj->instance].ship_info_index].model_num);

		// check for collision between debris model and ship sphere
		mc.flags = (MC_CHECK_MODEL | MC_SUBMODEL | MC_CHECK_SPHERELINE);

		mc_ret_val = model_collide(&mc);

		if (mc_ret_val) {
			set_hit_struct_info(debris_hit_info, &mc, false);

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

	if (mc_ret_val) {
		WarpEffect* warp_effect = nullptr;
		ship* shipp = &Ships[pship_obj->instance];

		// this is extremely confusing but mc.hit_point_world isn't actually in world coords
		// everything above was calculated relative to the heavy's position
		vec3d actual_world_hit_pos = mc.hit_point_world + heavy_obj->pos;
		if ((shipp->is_arriving()) && (shipp->warpin_effect != nullptr))
			warp_effect = shipp->warpin_effect;
		else if ((shipp->flags[Ship::Ship_Flags::Depart_warp]) && (shipp->warpout_effect != nullptr))
			warp_effect = shipp->warpout_effect;

		if (warp_effect != nullptr && point_is_clipped_by_warp(&actual_world_hit_pos, warp_effect))
			mc_ret_val = 0;
	}


	if ( mc_ret_val )	{

		// SET PHYSICS PARAMETERS
		// already have (hitpos - heavy) and light_cm_pos

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

/**
 * Return the team field for a debris object
 */
int debris_get_team(object *objp)
{
	Assert( objp->type == OBJ_DEBRIS );
	Assert( objp->instance >= 0 && objp->instance < (int)Debris.size() );

	return Debris[objp->instance].team;
}

/**
 * Fills in debris physics properties when created, specifically mass and moment of inertia
 */
void calc_debris_physics_properties( physics_info *pi, vec3d *mins, vec3d *maxs, float density )
{
	float dx, dy, dz, mass;
	dx = maxs->xyz.x - mins->xyz.x;
	dy = maxs->xyz.y - mins->xyz.y;
	dz = maxs->xyz.z - mins->xyz.z;

	// John, with new bspgen, just set pi->mass = mass
	mass = 0.12f * dx * dy * dz;
	pi->mass = (float) pow(mass, 0.6666667f) * 4.65f * density;

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

/**
* Renders debris
*/
void debris_render(object * obj, model_draw_list *scene)
{
	int			i, num, swapped;
	debris		*db;

	swapped = -1;
	num = obj->instance;

	Assert(num >= 0 && num < (int)Debris.size());
	db = &Debris[num];

	Assert(db->flags[Debris_Flags::Used]);

	texture_info *tbase = nullptr;

	auto pm = model_get(db->model_num);
	model_clear_instance( db->model_num );

	// Swap in a different texture depending on the species
	if (db->species >= 0)
	{
		//WMC - Someday, we should have glowing debris.
		if ( pm->n_textures == 1 ) {
			tbase = &pm->maps[0].textures[TM_BASE_TYPE];
			swapped = tbase->GetTexture();
			tbase->SetTexture(Species_info[db->species].debris_texture.bitmap_id);
		}
	}

	polymodel_instance *pmi = nullptr;
	if (db->model_instance_num >= 0)
	{
		pmi = model_get_instance(db->model_instance_num);
		model_instance_clear_arcs(pm, pmi);

		// Only render electrical arcs if within 500m of the eye (for a 10m piece)
		if ( vm_vec_dist_quick( &obj->pos, &Eye_position ) < obj->radius*50.0f )	{
			for (i=0; i<MAX_DEBRIS_ARCS; i++ )	{
				if ( db->arc_timestamp[i].isValid() )	{
					model_instance_add_arc( pm, pmi, db->submodel_num, &db->arc_pts[i][0], &db->arc_pts[i][1], MARC_TYPE_DAMAGED );
				}
			}
		}
	}

	model_render_params render_info;

	if ( db->is_hull ) {
		MONITOR_INC(NumHullDebrisRend, 1);
	} else {
		MONITOR_INC(NumSmallDebrisRend, 1);
		// render_info.set_flags(MR_NO_LIGHTING);
	}

	submodel_render_queue( &render_info, scene, pm, pmi, db->submodel_num, &obj->orient, &obj->pos );

	if (tbase != NULL && (swapped!=-1) && pm)	{
		tbase->SetTexture(swapped);
	}
}

bool debris_is_generic(debris *db)
{
	return db->model_num == Debris_model;
}

bool debris_is_vaporized(debris *db)
{
	return db->model_num == Debris_vaporize_model;
}

void create_generic_debris(object* ship_objp, vec3d* pos, float min_num_debris, float max_num_debris, float speed_mult, bool use_ship_debris) {
	Assertion(ship_objp->type == OBJ_SHIP, "create_generic_debris called for a non-ship, only ships can spew debris!");
	if (ship_objp->type != OBJ_SHIP)
		return;

	float num_debris = frand_range(min_num_debris, max_num_debris);

	num_debris *= (Detail.num_small_debris + 0.5f) / 4.5f;

	vec3d create_pos = *pos;
	for (int i = 0; i < num_debris; i++) {
		int model_num = use_ship_debris ? Ship_info[Ships[ship_objp->instance].ship_info_index].generic_debris_model_num : -1;
		debris_create(ship_objp, model_num, -1, &create_pos, pos, 0, speed_mult);
	}
}
