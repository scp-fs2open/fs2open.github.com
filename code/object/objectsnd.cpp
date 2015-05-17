/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "globalincs/pstypes.h"
#include "object/object.h"
#include "object/objectsnd.h"
#include "globalincs/linklist.h"
#include "ship/ship.h"
#include "gamesnd/gamesnd.h"
#include "sound/ds.h"
#include "sound/ds3d.h"
#include "io/timer.h"
#include "render/3d.h"
#include "io/joy_ff.h"
#include "species_defs/species_defs.h"
#include "debugconsole/console.h"


//  // --mharris port hack--
//  int ds_using_ds3d();
//  int ds_get_channel(int);
//  int ds3d_update_buffer(int, float, float, vec3d *, vec3d *);
// --end hack--


// Persistent sounds for objects (pointer to obj_snd is in object struct)
typedef struct _obj_snd {
	_obj_snd	*next, *prev;
	int		objnum;			// object index of object that contains this sound
	int		id;				// Index into Snds[] array
	int		instance;		// handle of currently playing sound (a ds3d handle if USES_DS3D flag set)
	int		next_update;	// timestamp that marks next allowed vol/pan change
	float		vol;				// volume of sound (range: 0.0 -> 1.0)
	float		pan;				// pan of sound (range: -1.0 -> 1.0)
	int		freq;				// valid range: 100 -> 100000 Hz
	int		flags;			
	vec3d	offset;			// offset from the center of the object where the sound lives
	ship_subsys *ss;		//Associated subsystem
} obj_snd;

#define VOL_PAN_UPDATE			50						// time in ms to update a persistent sound vol/pan
#define MIN_PERSISTANT_VOL		0.10f
#define MIN_FORWARD_SPEED		5
#define SPEED_SOUND				600.0f				// speed of sound in FreeSpace

#define MAX_OBJ_SOUNDS_PLAYING						5
static	int Num_obj_sounds_playing;

#define OBJSND_CHANGE_FREQUENCY_THRESHOLD			10

static	obj_snd	obj_snd_list;						// head of linked list of object sound structs
static	int		Doppler_enabled = TRUE;

#define	MAX_OBJ_SNDS	256
obj_snd	Objsnds[MAX_OBJ_SNDS];

int		Obj_snd_enabled = TRUE;
int		Obj_snd_last_update;							// timer used to run object sound updates at fixed time intervals
int		Obj_snd_level_inited=0;

// ship flyby data
#define	FLYBY_MIN_DISTANCE				90
#define	FLYBY_MIN_SPEED					50
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
			if ( osp->instance == -1 ) {
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

	list_init(&obj_snd_list);
	for ( i = 0; i < MAX_OBJ_SNDS; i++ ) {
		Objsnds[i].flags = 0;
	}

	Num_obj_sounds_playing = 0;
	Flyby_next_sound = 1;
	Flyby_next_repeat = 1;
	Flyby_last_objp = NULL;
	Obj_snd_last_update=0;

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

			if ( osp->instance != -1 ) {
				snd_stop(osp->instance);
				osp->instance = -1;
				switch(objp->type) {
					case OBJ_SHIP:
					case OBJ_GHOST:
					case OBJ_DEBRIS:
					case OBJ_ASTEROID:
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

		if ( osp->instance != -1 ) {
			snd_stop(osp->instance);
			osp->instance = -1;
			switch(objp->type) {
			case OBJ_SHIP:
			case OBJ_GHOST:
			case OBJ_DEBRIS:
			case OBJ_ASTEROID:
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
		obj_snd_stop(A, -1);
	}
}

// ---------------------------------------------------------------------------------------
// obj_snd_get_freq()
//
// Calculate the frequency of a sound to be played, based on the relative velocities
// of the source and observor
//
//	returns:		frequency of the sound
//
int obj_snd_get_freq(int source_freq, object* source, object* observor, vec3d *source_pos)
{
	vec3d	v_os, v_so;	// os == observor to source, so == source to observor
	float		vo, vs, freq;

	vm_vec_normalized_dir(&v_os, source_pos, &observor->pos);
	vm_vec_normalized_dir(&v_so, &observor->pos, source_pos);
	
	vo = vm_vec_dotprod(&v_os, &observor->phys_info.vel);
	vs = vm_vec_dotprod(&v_so, &source->phys_info.vel);

	freq = source_freq * ( (SPEED_SOUND + vo) / (SPEED_SOUND - vs) );
	return fl2i(freq);
}


/**
 * Stop a playing object sound, if it is quieter than sound at new_distance
 *
 * @param new_vol Volume of requested sound to play
 *
 * @return 1 A sound was stopped 
 * @return 0 A sound was not stopped
 */
int obj_snd_stop_lowest_vol(float new_vol)
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

		if ( (osp->instance != -1) && (osp->vol < lowest_vol) ) {
			lowest_vol = osp->vol;
			lowest_vol_osp = osp;
		}
	}

	if (lowest_vol_osp != NULL)
        objp = &Objects[lowest_vol_osp->objnum];

	if ( (lowest_vol < new_vol) && (objp != NULL) ) {
		int idx = 0;
		// determine what index in this guy the sound is
		for(SCP_vector<int>::iterator iter = objp->objsnd_num.begin(); iter != objp->objsnd_num.end(); ++iter, ++idx){
			if(*iter == (lowest_vol_osp - Objsnds)){
				obj_snd_index = idx;
				break;
			}
		}

		if((obj_snd_index == -1) || (obj_snd_index >= (int) objp->objsnd_num.size())){
			Int3();		// get dave
		} else {
			obj_snd_stop(objp, obj_snd_index);
		}

		return TRUE;
	}
	
	return FALSE;
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

				// pick a random species-based sound

				ship_info *sip = &Ship_info[Ships[closest_objp->instance].ship_info_index];
				game_snd *snd;

				if (sip->flags & SIF_BOMBER)
					snd = &Species_info[sip->species].snd_flyby_bomber;
				else
					snd = &Species_info[sip->species].snd_flyby_fighter;

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
	int				channel, go_ahead_flag;
	vec3d			source_pos;
	float				add_distance;

	if ( Obj_snd_enabled == FALSE )
		return;

	int now = timer_get_milliseconds();
	if ( (now - Obj_snd_last_update) > 100 ) {
		Obj_snd_last_update=now;
	} else {
		return;
	}

	closest_dist = 1000000.0f;
	closest_objp = NULL;

	object *observer_obj = NULL;
	if (Viewer_obj != NULL) {
		observer_obj = Viewer_obj;
	} else if (Viewer_mode & VM_OTHER_SHIP && Player_ai->target_objnum != -1) {
		// apparently Viewer_obj is still null when Viewing Other Ship externally
		observer_obj = &Objects[Player_ai->target_objnum];
	} else {
		observer_obj = Player_obj;
	}

	for ( osp = GET_FIRST(&obj_snd_list); osp !=END_OF_LIST(&obj_snd_list); osp = GET_NEXT(osp) ) {
		Assert(osp != NULL);
		objp = &Objects[osp->objnum];
		if ( Player_obj == objp && observer_obj == Player_obj ) {
			// we don't play the engine sound if the view is from the player
			continue;
		}
		
		gs = &Snds[osp->id];

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
			if ( Ship_info[Ships[objp->instance].ship_info_index].flags & SIF_SMALL_SHIP ) {
				closest_dist = distance;
				closest_objp = objp;
			}
		}

		// If the object is a ship, we don't want to start the engine sound unless the ship is
		// moving (unless flag SIF_BIG_SHIP is set)
		speed_vol_multiplier = 1.0f;
		rot_vol_mult = 1.0f;
		alive_vol_mult = 1.0f;
		if ( objp->type == OBJ_SHIP ) {
			if ( !(Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
				if ( objp->phys_info.max_vel.xyz.z <= 0.0f ) {
					percent_max = 0.0f;
				}
				else
					percent_max = objp->phys_info.fspeed / objp->phys_info.max_vel.xyz.z;

				if ( percent_max >= 0.5f )
					speed_vol_multiplier = 1.0f;
				else {
					speed_vol_multiplier = 0.5f + (percent_max);	// linear interp: 0.5->1.0 when 0.0->0.5
				}
			}
			if (osp->ss != NULL)
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
					if (osp->ss->flags & SSF_ROTATES) {
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
	
		go_ahead_flag = TRUE;
		float max_vol,new_vol;
		if ( osp->instance == -1 ) {
			if ( distance < Snds[osp->id].max ) {
				max_vol = Snds[osp->id].default_volume;
				if ( distance <= Snds[osp->id].min ) {
					new_vol = max_vol;
				}
				else {
					new_vol = max_vol - (distance - Snds[osp->id].min) * max_vol / (Snds[osp->id].max - Snds[osp->id].min);
				}

				if ( new_vol < 0.1f ) {
					continue;
				}

				switch( objp->type ) {
					case OBJ_SHIP:
					case OBJ_DEBRIS:
					case OBJ_ASTEROID:
						if ( Num_obj_sounds_playing >= MAX_OBJ_SOUNDS_PLAYING ) {
							go_ahead_flag = obj_snd_stop_lowest_vol(new_vol);
						}
						break;

					default:
						Int3();	// get Alan
						break;
				} // end switch

				if ( go_ahead_flag ) {
					osp->instance = snd_play_3d(gs, &source_pos, &View_position, add_distance, &objp->phys_info.vel, 1, 1.0f, SND_PRIORITY_TRIPLE_INSTANCE);
					if ( osp->instance != -1 ) {
						Num_obj_sounds_playing++;
					}
				}
				Assert(Num_obj_sounds_playing <= MAX_OBJ_SOUNDS_PLAYING);

			} // 		end if ( distance < Snds[osp->id].max )
		} // 		if ( osp->instance == -1 )
		else {
			if ( distance > Snds[osp->id].max ) {
				int sound_index = -1;
				int idx = 0;

				// determine which sound index it is for this guy
				for(SCP_vector<int>::iterator iter = objp->objsnd_num.begin(); iter != objp->objsnd_num.end(); ++iter, ++idx){
					if(*iter == (osp - Objsnds)){
						sound_index = idx;
						break;
					}
				}

				Assert(sound_index != -1);
				obj_snd_stop(objp, sound_index);						// currently playing sound has gone past maximum
			}
		}

		if ( osp->instance == -1 )
			continue;

		sp = NULL;
		if ( objp->type == OBJ_SHIP )
			sp = &Ships[objp->instance];


		channel = ds_get_channel(osp->instance);
		// for DirectSound3D sounds, re-establish the maximum speed based on the
		//	speed_vol_multiplier
		if ( sp == NULL || ( (sp != NULL) && (sp->flags & SF_ENGINES_ON) ) ) {
			snd_set_volume( osp->instance, gs->default_volume*speed_vol_multiplier*rot_vol_mult*alive_vol_mult );
		}
		else {
			// engine sound is disabled
			snd_set_volume( osp->instance, 0.0f );
		}

		vec3d vel = objp->phys_info.vel;

		// Don't play doppler effect for cruisers or capitals
		if ( sp ) {
			if ( ship_get_SIF(sp) & (SIF_BIG_SHIP | SIF_HUGE_SHIP) ) {
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
int obj_snd_assign(int objnum, int sndnum, vec3d *pos, int main, int flags, ship_subsys *associated_sub)
{
	if(objnum < 0 || objnum > MAX_OBJECTS)
		return -1;

	if(sndnum < 0)
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

	if(main){
		snd->flags |= OS_MAIN;
	}

	if(flags > 0){
		snd->flags |= flags;
	}

	snd->id = sndnum;

	snd->instance = -1;
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
//
void obj_snd_delete(int objnum, int index)
{
	//Sanity checking
	Assert(objnum > -1 && objnum < MAX_OBJECTS);

	object *objp = &Objects[objnum];
	
	Assert(index > -1 && index < (int) objp->objsnd_num.size());

	obj_snd *osp = &Objsnds[objp->objsnd_num[index]];

	//Stop the sound
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
void	obj_snd_delete_type(int objnum, int sndnum, ship_subsys *ss)
{
	object	*objp;
	obj_snd	*osp;

	if(objnum < 0 || objnum >= MAX_OBJECTS)
		return;

	objp = &Objects[objnum];

	size_t idx = 0;
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
		if(((sndnum != -1) && (osp->id != sndnum))
			|| ((ss != NULL) && (osp->ss != ss))){
			continue;
		}

		obj_snd_delete(objnum, idx);
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

// ---------------------------------------------------------------------------------------
// obj_snd_is_playing()
//
// Determines if a given object-linked sound is currently playing
//
int obj_snd_is_playing(int object, int index)
{
	if ( obj_snd_return_instance(object, index) < 0 ) 
		return 0;

	return 1;
}

// ---------------------------------------------------------------------------------------
// obj_snd_return_instance()
//
// Returns the sound instance for a given object-linked sound
//
int obj_snd_return_instance(int objnum, int index)
{
	if ( objnum < 0 || objnum >= MAX_OBJECTS)
		return -1;

	if ( index < 0 || index >= MAX_OBJ_SNDS )
		return -1;

	object *objp = &Objects[objnum];
	obj_snd *osp = &Objsnds[objp->objsnd_num[index]];

	return osp->instance;
}

// ---------------------------------------------------------------------------------------
// obj_snd_update_offset()
//
// Updates offset of the given object sound
//
int obj_snd_update_offset(int objnum, int index, vec3d *new_offset)
{
	if ( objnum < 0 || objnum >= MAX_OBJECTS)
		return 0;

	if ( index < 0 || index >= MAX_OBJ_SNDS )
		return 0;

	object *objp = &Objects[objnum];
	obj_snd *osp = &Objsnds[objp->objsnd_num[index]];
	osp->offset = *new_offset;

	return 1;
}
