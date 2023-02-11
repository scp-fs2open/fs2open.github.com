/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/linklist.h"
#include "io/joy_ff.h"
#include "io/timer.h"
#include "object/object.h"
#include "object/objectsnd.h"
#include "render/3d.h"
#include "ship/ship.h"
#include "sound/ds.h"
#include "sound/ds3d.h"
#include "species_defs/species_defs.h"


//  // --mharris port hack--
//  int ds_using_ds3d();
//  int ds_get_channel(int);
//  int ds3d_update_buffer(int, float, float, vec3d *, vec3d *);
// --end hack--


// Persistent sounds for objects (pointer to obj_snd is in object struct)
typedef struct _obj_snd {
	_obj_snd	*next, *prev;
	int		objnum;			// object index of object that contains this sound
	gamesnd_id	id;				// Index into Snds[] array
	sound_handle instance;      // handle of currently playing sound (a ds3d handle if USES_DS3D flag set)
	int		next_update;	// timestamp that marks next allowed vol/pan change
	float		vol;				// volume of sound (range: 0.0 -> 1.0)
	float		pan;				// pan of sound (range: -1.0 -> 1.0)
	int		freq;				// valid range: 100 -> 100000 Hz
	int		flags;			
	vec3d	offset;			// offset from the center of the object where the sound lives
	const ship_subsys *ss;		//Associated subsystem
} obj_snd;

#define SPEED_SOUND				600.0f				// speed of sound in FreeSpace

static int MAX_OBJ_SOUNDS_PLAYING = -1; // initialized in obj_snd_level_init()
static int Num_obj_sounds_playing;

static	obj_snd	obj_snd_list;						// head of linked list of object sound structs
static	int		Doppler_enabled = TRUE;

#define	MAX_OBJ_SNDS	256
obj_snd	Objsnds[MAX_OBJ_SNDS];

int		Obj_snd_enabled = TRUE;
UI_TIMESTAMP	Obj_snd_last_update;							// timer used to run object sound updates at fixed time intervals
int		Obj_snd_level_inited=0;

// ship flyby data
#define	FLYBY_MIN_DISTANCE				90
#define	FLYBY_MIN_RELATIVE_SPEED		100
#define	FLYBY_MIN_NEXT_TIME				1000	// in ms
#define	FLYBY_MIN_REPEAT_TIME			4000	// in ms
int		Flyby_next_sound;
int		Flyby_next_repeat;
object	*Flyby_last_objp;

// return the world pos of the sound source on a ship.  
void obj_snd_source_pos(vec3d *sound_pos, obj_snd *osp)
{
	vec3d offset_world;
	object *objp = &Objects[osp->objnum];

	// get sound pos in world coords
	vm_vec_unrotate(&offset_world, &osp->offset, &objp->orient);
	vm_vec_add(sound_pos, &objp->pos, &offset_world);
}

// determine what index in this guy the sound is
int obj_snd_find(object *objp, obj_snd *osp)
{
	int idx = 0;
	for (auto iter = objp->objsnd_num.begin(); iter != objp->objsnd_num.end(); ++iter, ++idx) {
		if (*iter == (osp - Objsnds)) {
			return idx;
		}
	}
	return -1;
}

// ---------------------------------------------------------------------------------------
// dcf_objsnd()
//
// Debug console function for object linked persistent sounds
//
//XSTR:OFF
DCF(objsnd, "Persistent sound stuff" )
{
	char buf1[4];
	char buf2[MAX_NAME_LEN];
	obj_snd *osp;
	SCP_string arg;

	if (dc_optional_string_either("help", "--help")) {
		dc_printf ("Usage: objsnd [-list]\n");
		dc_printf ("[-list] --  displays status of all objects with linked sounds\n");
		dc_printf ("with no parameters, object sounds are toggled on/off\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf( "Object sounds are: %s\n", (Obj_snd_enabled?"ON":"OFF") );
	}
	
	if (dc_optional_string("-list")) {
		for ( osp = GET_FIRST(&obj_snd_list); osp !=END_OF_LIST(&obj_snd_list); osp = GET_NEXT(osp) ) {
			vec3d source_pos;
			float distance;

			Assert(osp != NULL);
			if (!osp->instance.isValid()) {
				continue;
				//sprintf(buf1,"OFF");
			} else {
				sprintf(buf1,"ON");
			}

			if ( Objects[osp->objnum].type == OBJ_SHIP ) {
				strcpy_s(buf2, Ships[Objects[osp->objnum].instance].ship_name);
			}
			else if ( Objects[osp->objnum].type == OBJ_DEBRIS ) {
				sprintf(buf2, "Debris");
			}
			else {
				sprintf(buf2, "Unknown");
			}

			obj_snd_source_pos(&source_pos, osp);
			distance = vm_vec_dist_quick( &source_pos, &View_position );

			dc_printf("Object %d => name: %s vol: %.2f pan: %.2f dist: %.2f status: %s\n", osp->objnum, buf2, osp->vol, osp->pan, distance, buf1);
		} // end for

		dc_printf("Number object-linked sounds playing: %d\n", Num_obj_sounds_playing);
		return;
	}

	if (!dc_maybe_stuff_string_white(arg)) {
		// No arguments, toggle snd on/off
		if ( Obj_snd_enabled == TRUE ) {
				obj_snd_stop_all();
				Obj_snd_enabled = FALSE;
			} else {
				Obj_snd_enabled = TRUE;
		}
	} else {
		dc_printf("Unknown argument '%s'\n", arg.c_str());
	}
}
//XSTR:ON

// ---------------------------------------------------------------------------------------
// Debug console function for toggling doppler effection on/off
//
DCF_BOOL( doppler, Doppler_enabled )


// ---------------------------------------------------------------------------------------
// obj_snd_get_slot()
//
// Get a free slot in the Objsnds[] array
//
//	returns -1 if no slot is available
int obj_snd_get_slot()
{
	int i;

	for ( i = 0; i < MAX_OBJ_SNDS; i++ ) {
		if ( !(Objsnds[i].flags & OS_USED) ) 
			return i;
	}

	return -1;
}

// ---------------------------------------------------------------------------------------
// obj_snd_init()
//
// Called once at level start to initialize the persistent object sound system
//
void obj_snd_level_init()
{
	int i;

	if (MAX_OBJ_SOUNDS_PLAYING < 0)
	{
		MAX_OBJ_SOUNDS_PLAYING = Cmdline_no_enhanced_sound ? 5 : 12;
	}

	list_init(&obj_snd_list);
	for ( i = 0; i < MAX_OBJ_SNDS; i++ ) {
		Objsnds[i].flags = 0;
	}

	Num_obj_sounds_playing = 0;
	Flyby_next_sound = 1;
	Flyby_next_repeat = 1;
	Flyby_last_objp = NULL;
	Obj_snd_last_update = ui_timestamp();

	if ( !snd_is_inited() ) {
		Obj_snd_enabled = FALSE;
		return;
	}

	Obj_snd_level_inited=1;
}


// ---------------------------------------------------------------------------------------
// obj_snd_stop()
//
// Stop a persistent sound from playing.
//
// parameters:  objp			=> pointer to object that sound is to be stopped for
//
//
void obj_snd_stop(object *objp, int index)
{
	obj_snd	*osp;

	// sanity
	if(index >= (int) objp->objsnd_num.size()){
		Error(LOCATION, "Object sound index %d is bigger than the actual size %d!", index, (int) objp->objsnd_num.size());
		return;
	}

	// if index is -1, kill all sounds for this guy
	if(index == -1){
		// kill all sounds for this guy
		for(SCP_vector<int>::iterator iter = objp->objsnd_num.begin(); iter != objp->objsnd_num.end(); ++iter){
			if ( *iter == -1 ){
				continue;
			}

			osp = &Objsnds[*iter];

			if (osp->instance.isValid()) {
				snd_stop(osp->instance);
				osp->instance = sound_handle::invalid();
				switch(objp->type) {
					case OBJ_SHIP:
					case OBJ_GHOST:
					case OBJ_DEBRIS:
					case OBJ_ASTEROID:
					case OBJ_WEAPON:
						Num_obj_sounds_playing--;
						Assert(Num_obj_sounds_playing >= 0);					
						break;

					default:
						Int3();	// get Alan
						break;
				}				
			}		
		}
	} else {		
		if ( objp->objsnd_num[index] == -1 ){
			return;
		}

		osp = &Objsnds[objp->objsnd_num[index]];

		if (osp->instance.isValid()) {
			snd_stop(osp->instance);
			osp->instance = sound_handle::invalid();
			switch(objp->type) {
			case OBJ_SHIP:
			case OBJ_GHOST:
			case OBJ_DEBRIS:
			case OBJ_ASTEROID:
			case OBJ_WEAPON:
				Num_obj_sounds_playing--;
				Assert(Num_obj_sounds_playing >= 0);					
				break;

			default:
				Int3();	// get Alan
				break;
			}				
		}		
	}
}

// ---------------------------------------------------------------------------------------
// obj_snd_stop_all()
//
// Stop all object-linked persistent sounds from playing
//
//
void obj_snd_stop_all()
{
	object* A;

	for ( A = GET_FIRST(&obj_used_list); A !=END_OF_LIST(&obj_used_list); A = GET_NEXT(A) ) {
		// we probably don't want to skip should-be-dead objects here
		obj_snd_stop(A, -1);
	}
}

/**
 * Stop a playing object sound, if it is quieter than sound at new_distance
 *
 * @param new_vol Volume of requested sound to play
 *
 * @return true A sound was stopped
 * @return false A sound was not stopped
 */
bool obj_snd_stop_lowest_vol(float new_vol)
{
	obj_snd			*osp;
	object			*objp = NULL;
	obj_snd			*lowest_vol_osp = NULL;
	float				lowest_vol;
	int obj_snd_index = -1;
	
	lowest_vol = 1000.0f;
	for ( osp = GET_FIRST(&obj_snd_list); osp !=END_OF_LIST(&obj_snd_list); osp = GET_NEXT(osp) ) {
		Assert(osp->objnum != -1);
		objp = &Objects[osp->objnum];

		if ((osp->instance.isValid()) && (osp->vol < lowest_vol)) {
			lowest_vol = osp->vol;
			lowest_vol_osp = osp;
		}
	}

	if (lowest_vol_osp != NULL)
        objp = &Objects[lowest_vol_osp->objnum];

	if ( (lowest_vol < new_vol) && (objp != NULL) ) {
		obj_snd_index = obj_snd_find(objp, lowest_vol_osp);

		if((obj_snd_index == -1) || (obj_snd_index >= (int) objp->objsnd_num.size())){
			Int3();		// get dave
		} else {
			obj_snd_stop(objp, obj_snd_index);
		}

		return true;
	}
	
	return false;
}

//int Debug_1 = 0, Debug_2 = 0;

// ---------------------------------------------------------------------------------------
// maybe_play_flyby_snd()
//
// Based on how close the object is to the camera (and relative speed), maybe
// play a flyby sound.  Only play flyby sound for OBJ_SHIP objects.
//
// NOTE: global data Flyby_last_objp, Flyby_next_sound, Flyby_next_repeat are 
//			used.
//
void maybe_play_flyby_snd(float closest_dist, object *closest_objp, object *listener_objp)
{
	if ( closest_objp == NULL || closest_objp->type != OBJ_SHIP || closest_objp == listener_objp ) {
		return;
	}

	if ( closest_dist < FLYBY_MIN_DISTANCE ) {
		float relative_speed;
		vec3d diff;
		vm_vec_sub(&diff, &listener_objp->phys_info.vel, &closest_objp->phys_info.vel);


		relative_speed = vm_vec_mag_quick(&diff);
		if ( relative_speed > FLYBY_MIN_RELATIVE_SPEED ) {
			if ( timestamp_elapsed(Flyby_next_sound) ) {

				if ( closest_objp == Flyby_last_objp ) {
					if ( timestamp_elapsed(Flyby_next_repeat) ) {
						Flyby_next_repeat = timestamp(FLYBY_MIN_REPEAT_TIME);
					}
					else 
						return;
				}				

				Assert(closest_objp->type == OBJ_SHIP);
				if(closest_objp->type != OBJ_SHIP){
					return;
				}

				// check to see if a flyby sound is specified in ship class
				// if not, then pick a random species-based sound

				ship_info* sip = &Ship_info[Ships[closest_objp->instance].ship_info_index];
				game_snd *snd;

				if (sip->flyby_snd.isValid()) {
					snd = gamesnd_get_game_sound(sip->flyby_snd);
				} else {
					if (sip->flags[Ship::Info_Flags::Bomber])
						snd = &Species_info[sip->species].snd_flyby_bomber;
					else
						snd = &Species_info[sip->species].snd_flyby_fighter;
				}

				if (snd->sound_entries.empty())
					return; // This class or species does not define any relevant flyby sounds

				// play da sound
				snd_play_3d(snd, &closest_objp->pos, &View_position);

				//float dist = vm_vec_dist(&closest_objp->pos, &View_position);
				//nprintf(("AI", "Frame %i: Playing flyby sound, species = %i, size = %i, dist = %7.3f\n", Framecount, species, ship_size, dist));
//				nprintf(("AI", "Frame %i: Playing flyby sound, species = %i, size = %i, dist = %7.3f\n", Framecount, Debug_1, Debug_2, dist));
//Debug_1 = (Debug_1+1)%3;
//Debug_2 = (Debug_2+1)%2;

				joy_ff_fly_by(100 - (int) (100.0f * closest_dist / FLYBY_MIN_DISTANCE));

				Flyby_next_sound = timestamp(FLYBY_MIN_NEXT_TIME);
				Flyby_last_objp = closest_objp;
			}
		}
	}
}

// ---------------------------------------------------------------------------------------
// obj_snd_do_frame()
//
// Called once per frame to process the persistent sound objects
//
void obj_snd_do_frame()
{
	float				closest_dist, distance, speed_vol_multiplier, rot_vol_mult, percent_max, alive_vol_mult;
	obj_snd			*osp;
	object			*objp, *closest_objp;
	game_snd			*gs;
	ship				*sp;
	int				channel;
	vec3d			source_pos;
	float				add_distance;

	if ( Obj_snd_enabled == FALSE )
		return;

	if ( ui_timestamp_since(Obj_snd_last_update) > 100 ) {
		Obj_snd_last_update = ui_timestamp();
	} else {
		return;
	}

	closest_dist = 1000000.0f;
	closest_objp = nullptr;

	object *observer_obj = nullptr;
	if (Viewer_obj != nullptr) {
		observer_obj = Viewer_obj;
	} else if (Viewer_mode & VM_OTHER_SHIP && Player_ai->target_objnum != -1) {
		// apparently Viewer_obj is still null when Viewing Other Ship externally
		observer_obj = &Objects[Player_ai->target_objnum];
	} else {
		observer_obj = Player_obj;
	}

	for ( osp = GET_FIRST(&obj_snd_list); osp !=END_OF_LIST(&obj_snd_list); osp = GET_NEXT(osp) ) {
		Assert(osp != nullptr);
		objp = &Objects[osp->objnum];
		if ((Player_obj == objp) && (observer_obj == Player_obj) && !(osp->flags & OS_PLAY_ON_PLAYER)) {
			// we don't play sounds if the view is from the player
			// unless OS_PLAY_ON_PLAYER was set manually
			continue;
		}

		gs = gamesnd_get_game_sound(osp->id);
		if (gs->flags & GAME_SND_NOT_VALID) {
			continue;
		}

		obj_snd_source_pos(&source_pos, osp);
		distance = vm_vec_dist_quick( &source_pos, &View_position );

		// how much extra distance do we add before attentuation?
		add_distance = 0.0f;
		if(osp->flags & OS_MAIN){
			add_distance = objp->radius;
		}

		distance -= add_distance;
		if ( distance < 0.0f ) {
			distance = 0.0f;
		}

		// save closest distance (used for flyby sound) if this is a small ship (and not the observer)
		if ( (objp->type == OBJ_SHIP) && (distance < closest_dist) && (objp != observer_obj) ) {
			if ( Ship_info[Ships[objp->instance].ship_info_index].is_small_ship() ) {
				closest_dist = distance;
				closest_objp = objp;
			}
		}

		speed_vol_multiplier = 1.0f;
		rot_vol_mult = 1.0f;
		alive_vol_mult = 1.0f;
		if ( objp->type == OBJ_SHIP ) {
			ship_info *sip = &Ship_info[Ships[objp->instance].ship_info_index];

			// we don't want to start the engine sound unless the ship is
			// moving (unless flag SIF_BIG_SHIP is set)
			if ( (osp->flags & OS_ENGINE) && !(sip->is_big_or_huge()) ) {
				if ( objp->phys_info.max_vel.xyz.z <= 0.0f ) {
					percent_max = 0.0f;
				}
				else
					percent_max = objp->phys_info.fspeed / objp->phys_info.max_vel.xyz.z;

				if ( sip->min_engine_vol == -1.0f) {
					// Retail behavior: volume ramps from 0.5 (when stationary) to 1.0 (when at half speed)
					if ( percent_max >= 0.5f ) {
						speed_vol_multiplier = 1.0f;
					} else {
						speed_vol_multiplier = 0.5f + (percent_max);	// linear interp: 0.5->1.0 when 0.0->0.5
					}
				} else {
					// Volume ramps from min_engine_vol (when stationary) to 1.0 (when at full speed)
					speed_vol_multiplier = sip->min_engine_vol + ((1.0f - sip->min_engine_vol) * percent_max);
				}
			}

			// check conditions for subsystem sounds
			if (osp->ss != nullptr)
			{
				if (osp->flags & OS_TURRET_BASE_ROTATION)
				{
					if (osp->ss->base_rotation_rate_pct > 0.0f)
						rot_vol_mult = ((0.25f + (0.75f * osp->ss->base_rotation_rate_pct)) * osp->ss->system_info->turret_base_rotation_snd_mult);
					else
						rot_vol_mult = 0.0f;
				}
				if (osp->flags & OS_TURRET_GUN_ROTATION)
				{
					if (osp->ss->gun_rotation_rate_pct > 0.0f)
						rot_vol_mult = ((0.25f + (0.75f * osp->ss->gun_rotation_rate_pct)) * osp->ss->system_info->turret_gun_rotation_snd_mult);
					else
						rot_vol_mult = 0.0f;
				}
				if (osp->flags & OS_SUBSYS_ROTATION )
				{
					if (osp->ss->flags[Ship::Subsystem_Flags::Rotates]) {
						rot_vol_mult = 1.0f;
					} else {
						rot_vol_mult = 0.0f;
					}
				}
				if (osp->flags & OS_SUBSYS_ALIVE)
				{
					if (osp->ss->current_hits > 0.0f) {
						alive_vol_mult = 1.0f;
					} else {
						alive_vol_mult = 0.0f;
					}
				}
				if (osp->flags & OS_SUBSYS_DEAD)
				{
					if (osp->ss->current_hits <= 0.0f) {
						alive_vol_mult = 1.0f;
					} else {
						alive_vol_mult = 0.0f;
					}
				}
				if (osp->flags & OS_SUBSYS_DAMAGED)
				{
					alive_vol_mult = osp->ss->current_hits / osp->ss->max_hits;
					CLAMP(alive_vol_mult, 0.0f, 1.0f);
				}

			}
		}

		if ( !osp->instance.isValid() ) {
			bool go_ahead_flag = true;
			float new_vol;

			// if this is a 3D sound, check distance
			// (since non-3D sounds have gs->max set to 0, they will never be played)
			if ( distance < gs->max ) {
				float max_vol = gs->volume_range.max();
				if ( distance <= gs->min ) {
					new_vol = max_vol;
				}
				else {
					new_vol = max_vol - (distance - gs->min) * max_vol
						/ (gs->max - gs->min);
				}

				if ( new_vol < 0.1f ) {
					continue;
				}

				switch( objp->type ) {
					case OBJ_SHIP:
					case OBJ_DEBRIS:
					case OBJ_ASTEROID:
					case OBJ_WEAPON:
						if ( Num_obj_sounds_playing >= MAX_OBJ_SOUNDS_PLAYING ) {
							go_ahead_flag = obj_snd_stop_lowest_vol(new_vol);
						}
						break;

					default:
						UNREACHABLE("Unhandled object type %d for persistent sound; get a coder!", objp->type);
						break;
				} // end switch

				if ( go_ahead_flag ) {
					int is_looping = (osp->flags & OS_LOOPING_DISABLED) ? 0 : 1;
					osp->instance = snd_play_3d(gs, &source_pos, &View_position, add_distance, &objp->phys_info.vel, is_looping, 1.0f, SND_PRIORITY_TRIPLE_INSTANCE, nullptr, 1.0f, 0, true);
					if (osp->instance.isValid()) {
						Num_obj_sounds_playing++;
					}
				}
				Assert(Num_obj_sounds_playing <= MAX_OBJ_SOUNDS_PLAYING);
			}
		}
		else {
			// sound has finished playing and won't be played again
			if ((osp->flags & OS_LOOPING_DISABLED) && !snd_is_playing(osp->instance)) {
				auto osp_prev = GET_PREV(osp);

				// non-looping sounds that have already played once need to be removed from the object sound list
				int sound_index = obj_snd_find(objp, osp);
				obj_snd_delete(objp, sound_index, false);

				// don't corrupt the iterating loop (next iteration will move to the deleted osp's next sibling)
				osp = osp_prev;
				continue;
			}

			// currently playing sound has gone past maximum
			if ( distance > gamesnd_get_game_sound(osp->id)->max ) {
				int sound_index = obj_snd_find(objp, osp);

				Assert(sound_index != -1);
				obj_snd_stop(objp, sound_index);
			}
		}

		if (!osp->instance.isValid())
			continue;

		sp = nullptr;
		if ( objp->type == OBJ_SHIP )
			sp = &Ships[objp->instance];


		channel = ds_get_channel(osp->instance);
		// for DirectSound3D sounds, re-establish the maximum speed based on the
		//	speed_vol_multiplier
		if ( (sp == nullptr) || !(osp->flags & OS_ENGINE) || (sp->flags[Ship::Ship_Flags::Engines_on]) ) {
			snd_set_volume( osp->instance, gs->volume_range.next() *speed_vol_multiplier*rot_vol_mult*alive_vol_mult );
		}
		else {
			// engine sound is disabled
			snd_set_volume( osp->instance, 0.0f );
		}

		vec3d vel = objp->phys_info.vel;

		// Don't play doppler effect for cruisers or capitals
		if ( sp ) {
			if ( Ship_info[sp->ship_info_index].is_big_or_huge() ) {
				vel = vmd_zero_vector;
			}
		}

		ds3d_update_buffer(channel, i2fl(gs->min), i2fl(gs->max), &source_pos, &vel);
		snd_get_3d_vol_and_pan(gs, &source_pos, &osp->vol, &osp->pan, add_distance);
	}	// end for

	// see if we want to play a flyby sound
	maybe_play_flyby_snd(closest_dist, closest_objp, observer_obj);
}

// ---------------------------------------------------------------------------------------
// obj_snd_assign()
//
// Assign a persistent sound to an object.
//
// parameters:  objnum		=> index of object that sound is being assigned to
//              i				=> Index into Snds[] array
//					 fname		=> filename of sound to play ( so DS3D can load the sound )
//
// returns:     -1			=> sound could not be assigned (possible, since only MAX_OBJECT_SOUNDS persistent
//										sound can be assigned per object).  
//               >= 0			=> sound was successfully assigned
//
int obj_snd_assign(int objnum, gamesnd_id sndnum, const vec3d *pos, int flags, const ship_subsys *associated_sub)
{
	Assertion(pos != nullptr, "Sound position must not be null!");

	if(objnum < 0 || objnum >= MAX_OBJECTS)
		return -1;

	if(!sndnum.isValid())
		return -1;

	if ( Obj_snd_enabled == FALSE )
		return -1;

	//Initialize object sound list, if not initialized
	if (!Obj_snd_level_inited)
		obj_snd_level_init();

	obj_snd	*snd = NULL;
	object	*objp = &Objects[objnum];
	int sound_index;
	int idx = 0;

	// try and find a valid objsound index
	sound_index = -1;
	for(SCP_vector<int>::iterator iter = objp->objsnd_num.begin(); iter != objp->objsnd_num.end(); ++iter, ++idx){
		if(*iter == -1){
			sound_index = idx;
			break;
		}
	}
	
	// no sound slot free, make a new one!
	if ( sound_index == -1 ){
		sound_index = (int) objp->objsnd_num.size();
		objp->objsnd_num.push_back(-1);
	}

	objp->objsnd_num[sound_index] = obj_snd_get_slot();
	if ( objp->objsnd_num[sound_index] == -1 ) {
		nprintf(("Sound", "SOUND ==> No free object-linked sounds left\n"));
		return -1;
	}
	snd = &Objsnds[objp->objsnd_num[sound_index]];
	snd->flags = OS_USED;

	if(flags > 0){
		snd->flags |= flags;
	}

	snd->id = sndnum;

	snd->instance    = sound_handle::invalid();
	snd->vol = 0.0f;
	snd->objnum = OBJ_INDEX(objp);
	snd->next_update = 1;
	snd->offset = *pos;
	snd->ss = associated_sub;
	// vm_vec_sub(&snd->offset, pos, &objp->pos);	

	// add objp to the obj_snd_list
	list_append( &obj_snd_list, snd );

	return sound_index;
}

// ---------------------------------------------------------------------------------------
// obj_snd_delete()
//
// Remove a persistent sound that has been assigned to an object.
//
// parameters:  objnum		=> index of object that sound is being removed from.
//				index		=> index of sound in objsnd_num
//				stop_sound	=> whether we stop it (defaults to true)
//
void obj_snd_delete(object *objp, int index, bool stop_sound)
{
	Assert(index > -1 && index < (int) objp->objsnd_num.size());

	obj_snd *osp = &Objsnds[objp->objsnd_num[index]];

	//Stop the sound
	if (stop_sound)
		obj_snd_stop(objp, index);

	// remove objp from the obj_snd_list
	list_remove( &obj_snd_list, osp );
	osp->objnum = -1;
	osp->flags = 0;
	osp = NULL;
	objp->objsnd_num[index] = -1;
}

// ---------------------------------------------------------------------------------------
// obj_snd_delete_type()
//
// Remove every similar persistent sound that has been assigned to an object.
//
// parameters:  objnum		=> index of object that sound is being removed from.
//				sndnum		=> index of sound that we're trying to completely get rid of
//								-1 to delete all persistent sounds on ship.
//
//
void	obj_snd_delete_type(int objnum, gamesnd_id sndnum, ship_subsys *ss)
{
	object	*objp;
	obj_snd	*osp;

	if(objnum < 0 || objnum >= MAX_OBJECTS)
		return;

	objp = &Objects[objnum];

	int idx = 0;
	//Go through the list and get sounds that match criteria
	for(SCP_vector<int>::iterator iter = objp->objsnd_num.begin(); iter != objp->objsnd_num.end(); ++iter, ++idx){
		// no sound
		if ( *iter == -1 ){
			continue;
		}

		osp = &Objsnds[*iter];

		// if we're just deleting a specific sound type
		// and this is not one of them. skip it.
		//Also check if this is assigned to the right subsystem, if one has been given.
		if(((sndnum.isValid()) && (osp->id != sndnum))
			|| ((ss != NULL) && (osp->ss != ss))){
			continue;
		}

		obj_snd_delete(objp, idx);
	}
}

// ---------------------------------------------------------------------------------------
// obj_snd_delete_all()
//
// Remove all persistent sounds
//
void obj_snd_delete_all()
{
	int idx;
	for(idx=0; idx<MAX_OBJ_SNDS; idx++){
		if(Objsnds[idx].flags & OS_USED){
			obj_snd_delete_type(Objsnds[idx].objnum);
		}
	}
}

// ---------------------------------------------------------------------------------------
// obj_snd_close()
//
// Called once at game close to de-initialize the persistent object sound system
//
void obj_snd_level_close()
{
	if ( !Obj_snd_level_inited ) {
		return;
	}
	obj_snd_delete_all();
	Obj_snd_level_inited=0;
}
