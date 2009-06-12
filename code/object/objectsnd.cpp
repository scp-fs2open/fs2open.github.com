/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Object/ObjectSnd.cpp $
 * $Revision: 2.13 $
 * $Date: 2006-04-03 07:48:03 $
 * $Author: wmcoolmon $
 *
 * C module for managing object-linked persistant sounds
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.12  2005/10/23 19:08:01  taylor
 * fix crashes and minor slowdowns when sound is disabled (-nosound)
 *
 * Revision 2.11  2005/09/25 05:13:07  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.10  2005/07/13 02:01:30  Goober5000
 * fixed a bunch of "issues" caused by me with the species stuff
 * --Goober5000
 *
 * Revision 2.9  2005/07/13 00:44:23  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.8  2005/04/25 00:28:58  wmcoolmon
 * subsystem sounds
 *
 * Revision 2.7  2005/04/05 05:53:21  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.6  2004/07/26 20:47:45  Kazan
 * remove MCD complete
 *
 * Revision 2.5  2004/07/12 16:32:59  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.4  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/03/18 01:44:31  Goober5000
 * fixed some misspellings
 * --Goober5000
 *
 * Revision 2.2  2002/08/01 01:41:09  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 11    9/08/99 8:56a Mikek
 * Blast.  Suppress a debug warning.
 * 
 * 10    9/08/99 8:56a Mikek
 * Whoops, broke the build with debug code.
 * 
 * 9     9/08/99 8:42a Mikek
 * Make flyby sounds a lot easier to trigger.  Decrease some thresholds
 * and remove the dot product check.  This is handled by the delta
 * velocity check.
 * 
 * 8     7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 7     7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 6     6/25/99 3:08p Dave
 * Multiple flyby sounds.
 * 
 * 5     5/23/99 8:11p Alanl
 * Added support for EAX
 * 
 * 4     5/18/99 11:50a Andsager
 * Remove unused object type OBJ_GHOST_SAVE
 * 
 * 3     4/23/99 12:01p Johnson
 * Added SIF_HUGE_SHIP
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 59    5/15/98 5:16p Dave
 * Fix a standalone resetting bug.Tweaked PXO interface. Display captaincy
 * status for team vs. team. Put in asserts to check for invalid team vs.
 * team situations.
 * 
 * 58    5/07/98 12:24a Hoffoss
 * Finished up sidewinder force feedback support.
 * 
 * 57    5/06/98 1:29p Dan
 * AL: allow engine sounds to play polyphonically
 * 
 * 56    5/06/98 10:27a Dan
 * AL: Fix bug with object sounds and Aureal
 * 
 * 55    4/19/98 9:32p Lawrance
 * Add support for Shivan flyby sound
 * 
 * 54    4/18/98 9:12p Lawrance
 * Added Aureal support.
 * 
 * 53    4/12/98 5:31p Lawrance
 * use timer_get_milliseconds() instead of gettime()
 * 
 * 52    4/08/98 8:33p Lawrance
 * increase distance at which flyby sound  is heard
 * 
 * 51    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 50    3/21/98 3:36p Lawrance
 * Don't change frequency unless a threshold change has been reached.
 * 
 * 49    3/19/98 5:34p Lawrance
 * Only play flyby sound if both ships involved are above a minimum speed
 * 
 * 48    3/18/98 9:49a Lawrance
 * fix uninitialized data bug
 * 
 * 47    3/17/98 5:55p Lawrance
 * Support object-linked sounds for asteroids.
 * 
 * 46    3/14/98 4:59p Lawrance
 * Fix bug with object sounds not getting cleared
 * 
 * 45    2/22/98 2:48p John
 * More String Externalization Classification
 * 
 * 44    2/20/98 8:32p Lawrance
 * Add radius parm to sound_play_3d()
 * 
 * 43    2/12/98 11:53p Lawrance
 * move engine pos closer to center of ship, ensure flyby sounds only play
 * when flying past a small ship
 * 
 * 42    2/11/98 5:38p Dave
 * Put in a global inited flag for objsound.
 * 
 * 41    2/11/98 11:30a Dan
 * AL: fix DirectSound3D bug
 * 
 * 40    2/11/98 10:28a Lawrance
 * Fix attenuation problems related to engine position.
 * 
 * 39    2/09/98 9:09p Lawrance
 * ensure we don't get two flyby sounds too close together
 * 
 * 38    2/02/98 8:23p Lawrance
 * double distance at which a flyby sound is played
 * 
 * 37    1/29/98 10:31a Lawrance
 * Don't play doppler effect for cruisers and capital ships
 * 
 * 36    1/20/98 10:04a Lawrance
 * fix volume bug, do obj_snd_do_frame at 10Hz
 * 
 * 35    1/20/98 9:47a Mike
 * Suppress optimized compiler warnings.
 * Some secondary weapon work.
 * 
 * 34    1/10/98 1:14p John
 * Added explanation to debug console commands
 * 
 * 33    12/21/97 4:33p John
 * Made debug console functions a class that registers itself
 * automatically, so you don't need to add the function to
 * debugfunctions.cpp.  
 * 
 * 32    11/20/97 6:45p Lawrance
 * allow two looping debris sounds to play
 * 
 * 31    11/04/97 10:18a Dan
 * initialize closest_obj to NULL
 * 
 * 30    11/03/97 11:09p Lawrance
 * Only play flyby sound for ships (ie not debris).
 * 
 * 29    10/26/97 3:22p Lawrance
 * use volume and not distance when deciding which engine sounds to play
 * 
 * 28    10/22/97 10:37a Johnson
 * ALAN: ensure that object sounds are only deleted once
 * 
 * 27    10/10/97 7:43p Lawrance
 * use SF_ENGINES_ON flag
 * 
 * 26    10/06/97 4:13p Lawrance
 * use engine pos when updating volume for object sounds
 * 
 * 25    10/01/97 5:55p Lawrance
 * change call to snd_play_3d() to allow for arbitrary listening position
 * 
 * 24    9/18/97 7:58a Lawrance
 * use rear of ship when deciding where to play the engine sound
 * 
 * 23    9/11/97 5:01p Dave
 * Minor changes to handle ingame joining/dropping for multiplayer.
 * 
 * 22    9/03/97 5:02p Lawrance
 * add engine stuttering when a ship is dying
 * 
 * 21    8/29/97 5:04p Dave
 * Made sure ghost objects are handled correctly.
 * 
 * 20    7/28/97 11:38a Lawrance
 * let ship engine scale with speed for DirectSound3D engine sounds too
 * 
 * 19    7/27/97 5:14p Lawrance
 * add afterburners to the player control info
 * 
 * 18    7/14/97 12:04a Lawrance
 * make Obj_snd_enabled visible
 * 
 * 17    7/09/97 12:07a Mike
 * Changes in ship_info struct.
 * 
 * 16    7/08/97 11:38a Lawrance
 * added SIF_BIG_SHIP flag (used for object-linked engine sounds)
 * 
 * 15    6/23/97 10:15a Lawrance
 * fix bug with object linked sounds not stopping playing sounds
 * 
 * 14    6/19/97 2:05p Lawrance
 * when stopping  a linked sound, stop it right away
 * 
 * 13    6/09/97 11:50p Lawrance
 * integrating DirectSound3D
 * 
 * 12    6/08/97 5:59p Lawrance
 * comment out ds3d stuff for now
 * 
 * 11    6/06/97 4:13p Lawrance
 * use an index instead of a pointer for object-linked sounds
 * 
 * 10    6/05/97 1:37a Lawrance
 * change to support snd_get_vol_and_pan()
 * 
 * 9     6/05/97 1:07a Lawrance
 * changes to support sound interface
 * 
 * 8     6/02/97 1:50p Lawrance
 * supporting integration with Direct3D
 * 
 * 7     5/18/97 2:40p Lawrance
 * added debug function to toggle doppler effect
 * 
 * 6     5/15/97 9:05a Lawrance
 * comment out some debugging nprintf()'s
 * 
 * 5     5/14/97 10:47a Lawrance
 * only play external engine sound for loudest ship
 * 
 * 4     5/09/97 4:33p Lawrance
 * doppler effects
 * 
 * 3     5/09/97 9:41a Lawrance
 * modified comments
 * 
 * 2     5/08/97 4:30p Lawrance
 * split off object sound stuff into separate file
 *
 * $NoKeywords: $
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



//  // --mharris port hack--
//  int ds_using_ds3d();
//  int ds_get_channel(int);
//  int ds3d_update_buffer(int, float, float, vec3d *, vec3d *);
// --end hack--


// Persistant sounds for objects (pointer to obj_snd is in object struct)
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

#define VOL_PAN_UPDATE			50						// time in ms to update a persistant sound vol/pan
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
// Debug console function for object linked persistant sounds
//
//XSTR:OFF
DCF(objsnd, "Persistant sound stuff" )
{
	char		buf1[16], buf2[64];
	obj_snd	*osp;

	if ( Dc_command )	{
		dc_get_arg(ARG_STRING|ARG_NONE);

		if ( Dc_arg_type & ARG_NONE ) {
			if ( Obj_snd_enabled == TRUE ) {
				obj_snd_stop_all();
				Obj_snd_enabled = FALSE;
			}
			else {
				Obj_snd_enabled = TRUE;
			}
		}
		if ( !stricmp( Dc_arg, "list" ))	{
			for ( osp = GET_FIRST(&obj_snd_list); osp !=END_OF_LIST(&obj_snd_list); osp = GET_NEXT(osp) ) {
				Assert(osp != NULL);
				if ( osp->instance == -1 ) {
					continue;
					//sprintf(buf1,"OFF");
				} else {
					sprintf(buf1,"ON");
				}

				if ( Objects[osp->objnum].type == OBJ_SHIP ) {
					sprintf(buf2, Ships[Objects[osp->objnum].instance].ship_name);
				}
				else if ( Objects[osp->objnum].type == OBJ_DEBRIS ) {
					sprintf(buf2, "Debris");
				}
				else {
					sprintf(buf2, "Unknown");
				}

				vec3d source_pos;
				float distance;

				obj_snd_source_pos(&source_pos, osp);
				distance = vm_vec_dist_quick( &source_pos, &View_position );

				dc_printf("Object %d => name: %s vol: %.2f pan: %.2f dist: %.2f status: %s\n", osp->objnum, buf2, osp->vol, osp->pan, distance, buf1);
			} // end for
				dc_printf("Number object-linked sounds playing: %d\n", Num_obj_sounds_playing);
		}
	}

	if ( Dc_help ) {
		dc_printf ("Usage: objsnd [list]\n");
		dc_printf ("[list] --  displays status of all objects with linked sounds\n");
		dc_printf ("with no parameters, object sounds are toggled on/off\n");
		Dc_status = 0;
	}

	if ( Dc_status )	{
		dc_printf( "Object sounds are: %s\n", (Obj_snd_enabled?"ON":"OFF") );
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
// Called once at level start to initialize the persistant object sound system
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
// Stop a persistant sound from playing.
//
// parameters:  objp			=> pointer to object that sound is to be stopped for
//
//
void obj_snd_stop(object *objp, int index)
{
	obj_snd	*osp;
	int idx;

	// sanity
	if(index >= MAX_OBJECT_SOUNDS){
		Int3();
		return;
	}

	// if index is -1, kill all sounds for this guy
	if(index == -1){
		// kill all sounds for this guy
		for(idx=0; idx<MAX_OBJECT_SOUNDS; idx++){
			if ( objp->objsnd_num[idx] == -1 ){
				continue;
			}

			osp = &Objsnds[objp->objsnd_num[idx]];

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
// Stop all object-linked persistant sounds from playing
//
//
void obj_snd_stop_all()
{
	object* A;

	for ( A = GET_FIRST(&obj_used_list); A !=END_OF_LIST(&obj_used_list); A = GET_NEXT(A) ) {
		if ( A ) {
			obj_snd_stop(A, -1);
		}
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


// ---------------------------------------------------------------------------------------
// obj_snd_stop_lowest_vol()
//
//	Stop a playing object sound, if it is quieter than sound at new_distance
//
// input:		new_vol			=>	volume of requested sound to play
//
//	returns:		TRUE	=>		A sound was stopped 
//					FALSE	=>		A sound was not stopped
//
int obj_snd_stop_lowest_vol(float new_vol)
{
	obj_snd			*osp;
	object			*objp = NULL;
	obj_snd			*lowest_vol_osp = NULL;
	float				lowest_vol;
	int obj_snd_index = -1;
	int idx;
	
	lowest_vol = 1000.0f;
	for ( osp = GET_FIRST(&obj_snd_list); osp !=END_OF_LIST(&obj_snd_list); osp = GET_NEXT(osp) ) {
		Assert(osp->objnum != -1);
		objp = &Objects[osp->objnum];

		if ( (osp->instance != -1) && (osp->vol < lowest_vol) ) {
			lowest_vol = osp->vol;
			lowest_vol_osp = osp;
		}
	}

	Assert(lowest_vol_osp != NULL);
	objp = &Objects[lowest_vol_osp->objnum];

	if ( (lowest_vol < new_vol) && (objp != NULL) ) {
		// determine what index in this guy the sound is
		for(idx=0; idx<MAX_OBJECT_SOUNDS; idx++){
			if(objp->objsnd_num[idx] == (lowest_vol_osp - Objsnds)){
				obj_snd_index = idx;
				break;
			}
		}

		if((obj_snd_index == -1) || (obj_snd_index >= MAX_OBJECT_SOUNDS)){
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
// Called once per frame to process the persistant sound objects
//
void obj_snd_do_frame()
{
	float				closest_dist, distance, speed_vol_multiplier, rot_vol_mult, percent_max;
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
		if ( distance < 0 ) {
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
		if ( objp->type == OBJ_SHIP ) {
			if ( !(Ship_info[Ships[objp->instance].ship_info_index].flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
				if ( objp->phys_info.max_vel.xyz.z <= 0 ) {
					percent_max = 0.0f;
				}
				else
					percent_max = objp->phys_info.fspeed / objp->phys_info.max_vel.xyz.z;

				if ( percent_max >= 0.5 )
					speed_vol_multiplier = 1.0f;
				else {
					speed_vol_multiplier = 0.5f + (percent_max);	// linear interp: 0.5->1.0 when 0.0->0.5
				}
			}
			if (osp->ss != NULL)
			{
				if (osp->flags & OS_TURRET_BASE_ROTATION)
				{
					if (osp->ss->base_rotation_rate_pct > 0)
						rot_vol_mult = ((0.25f + (0.75f * osp->ss->base_rotation_rate_pct)) * osp->ss->system_info->turret_base_rotation_snd_mult);
					else
						rot_vol_mult = 0;
				}
				if (osp->flags & OS_TURRET_GUN_ROTATION)
				{
					if (osp->ss->gun_rotation_rate_pct > 0)
						rot_vol_mult = ((0.25f + (0.75f * osp->ss->gun_rotation_rate_pct)) * osp->ss->system_info->turret_gun_rotation_snd_mult);
					else
						rot_vol_mult = 0;
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

				if ( new_vol < 0.1 ) {
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
					if ( ds_using_ds3d() ) {
						osp->instance = snd_play_3d(gs, &source_pos, &View_position, add_distance, &objp->phys_info.vel, 1, 1.0f, SND_PRIORITY_TRIPLE_INSTANCE);
						if ( osp->instance != -1 ) {
							Num_obj_sounds_playing++;
						}
					}
					else {
						snd_get_3d_vol_and_pan(gs, &source_pos, &osp->vol, &osp->pan, add_distance);
						osp->instance = snd_play_looping( gs, osp->pan, 0, 0, (osp->vol*speed_vol_multiplier*rot_vol_mult)/gs->default_volume, SND_PRIORITY_TRIPLE_INSTANCE );
						if ( osp->instance != -1 ) {
							osp->freq =	snd_get_pitch(osp->instance);
							Num_obj_sounds_playing++;
						}
					}
				}
				Assert(Num_obj_sounds_playing <= MAX_OBJ_SOUNDS_PLAYING);

			} // 		end if ( distance < Snds[osp->id].max )
		} // 		if ( osp->instance == -1 )
		else {
			if ( distance > Snds[osp->id].max ) {
				int sound_index = -1;
				int idx;

				// determine which sound index it is for this guy
				for(idx=0; idx<MAX_OBJECT_SOUNDS; idx++){
					if(objp->objsnd_num[idx] == (osp - Objsnds)){
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


		if (ds_using_ds3d()) {
			channel = ds_get_channel(osp->instance);
			// for DirectSound3D sounds, re-establish the maximum speed based on the
			//	speed_vol_multiplier
			if ( sp == NULL || ( (sp != NULL) && (sp->flags & SF_ENGINES_ON) ) ) {
				snd_set_volume( osp->instance, gs->default_volume*speed_vol_multiplier*rot_vol_mult );
			}
			else {
				// engine sound is disabled
				snd_set_volume( osp->instance, 0.0f );
			}

			vec3d *vel=NULL;
			vel = &objp->phys_info.vel;

			// Don't play doppler effect for cruisers or capitals
			if ( sp ) {
				if ( ship_get_SIF(sp) & (SIF_BIG_SHIP | SIF_HUGE_SHIP) ) {
					vel=NULL;
				}
			}

			ds3d_update_buffer(channel, i2fl(gs->min), i2fl(gs->max), &source_pos, vel);
			snd_get_3d_vol_and_pan(gs, &source_pos, &osp->vol, &osp->pan, add_distance);
		}
		else {
			if ( sp == NULL || (sp != NULL && (sp->flags & SF_ENGINES_ON) ) ) {
				snd_get_3d_vol_and_pan(gs, &source_pos, &osp->vol, &osp->pan, add_distance);
				snd_set_volume( osp->instance, osp->vol*speed_vol_multiplier*rot_vol_mult );
				snd_set_pan( osp->instance, osp->pan );
				// Don't play doppler effect for cruisers or capitals
				if ( objp->type == OBJ_SHIP && Doppler_enabled == TRUE ) {
					if ( !(ship_get_SIF(sp) & (SIF_BIG_SHIP | SIF_HUGE_SHIP)) ) {
						int new_freq;
						// calc doppler effect
						new_freq = obj_snd_get_freq(osp->freq, objp, observer_obj, &source_pos);
						if ( abs(new_freq - osp->freq) > OBJSND_CHANGE_FREQUENCY_THRESHOLD ) {
							snd_set_pitch( osp->instance, new_freq);
						}
					}
				}
			}
			else
				snd_set_volume( osp->instance, 0.0f );
		}
	}	// end for

	// see if we want to play a flyby sound
	maybe_play_flyby_snd(closest_dist, closest_objp, observer_obj);
}

// ---------------------------------------------------------------------------------------
// obj_snd_assign()
//
// Assign a persistant sound to an object.
//
// parameters:  objnum		=> index of object that sound is being assigned to
//              i				=> Index into Snds[] array
//					 fname		=> filename of sound to play ( so DS3D can load the sound )
//
// returns:     -1			=> sound could not be assigned (possible, since only MAX_OBJECT_SOUNDS persistant
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

	obj_snd	*snd = NULL;
	object	*objp = &Objects[objnum];
	int idx, sound_index;

	// try and find a valid objsound index
	sound_index = -1;
	for(idx=0; idx<MAX_OBJECT_SOUNDS; idx++){
		if(objp->objsnd_num[idx] == -1){
			sound_index = idx;
			break;
		}
	}
	
	// no sound. doh!
	if ( sound_index == -1 ){
		return -1;
	}

	objp->objsnd_num[sound_index] = (short)obj_snd_get_slot();
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

	if ( sndnum == -1 ) {
		return -1;
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
// Remove a persistant sound that has been assigned to an object.
//
// parameters:  objnum		=> index of object that sound is being removed from.
//				index		=> index of sound in objsnd_num
//
void obj_snd_delete(int objnum, int index)
{
	if(objnum < 0 || objnum >= MAX_OBJECTS)
		return;
	if(index < 0 || index >= MAX_OBJECT_SOUNDS)
		return;

	//Sanity checking
	Assert(objnum > -1 && objnum < MAX_OBJECTS);
	Assert(index > -1 && index < MAX_OBJECT_SOUNDS);

	object *objp = &Objects[objnum];
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
// Remove every similar persistant sound that has been assigned to an object.
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
	int idx;

	if(objnum < 0 || objnum >= MAX_OBJECTS)
		return;

	objp = &Objects[objnum];

	//Go through the list and get sounds that match criteria
	for(idx=0; idx<MAX_OBJECT_SOUNDS; idx++){
		// no sound
		if ( objp->objsnd_num[idx] == -1 ){
			continue;
		}

		osp = &Objsnds[objp->objsnd_num[idx]];

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
// Remove all persistant sounds
//
void obj_snd_delete_all()
{
	/*
	obj_snd	*osp, *temp;	
	
	osp = GET_FIRST(&obj_snd_list);	
	while( (osp != NULL) && (osp !=END_OF_LIST(&obj_snd_list)) )	{
		temp = GET_NEXT(osp);
		Assert( osp->objnum != -1 );

		obj_snd_delete_type( osp->objnum );

		osp = temp;
	}
	*/

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
// Called once at game close to de-initialize the persistant object sound system
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
