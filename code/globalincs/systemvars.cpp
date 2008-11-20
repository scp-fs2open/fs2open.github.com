/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/SystemVars.cpp $
 * $Revision: 2.11.2.2 $
 * $Date: 2008-01-17 07:43:23 $
 * $Author: Goober5000 $
 *
 * Variables and constants common to FreeSpace and Fred.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.11.2.1  2006/09/11 01:00:27  taylor
 * various small compiler warning and strict compiling fixes
 *
 * Revision 2.11  2006/04/20 06:32:01  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.10  2006/01/27 20:29:30  Goober5000
 * some tweaks for insertion sort
 * --Goober5000
 *
 * Revision 2.9  2006/01/27 06:03:41  Goober5000
 * add an insertion sort function for small or almost-sorted arrays; it has the same prototype as stdlib's quick sort
 * --Goober5000
 *
 * Revision 2.8  2005/05/26 04:27:10  taylor
 * timer changes to fix game_busy() on the loading screen and 100% incorrect values on some machines,
 *   don't know if this is going to work that great but it's more like the Linux SDL code now which
 *   has worked for years without major problems
 *
 * Revision 2.7  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.6  2005/02/04 10:12:29  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.5  2004/10/31 21:33:05  taylor
 * rename OGL_inited to OGL_enabled to match D3D version
 *
 * Revision 2.4  2004/07/26 20:47:31  Kazan
 * remove MCD complete
 *
 * Revision 2.3  2004/07/12 16:32:47  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.2  2002/11/22 20:56:15  phreak
 * added OGL_inited global.  Sets to 0 even if OGL isn't defined by preproccessor
 * -phreak
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    9/09/99 8:53p Dave
 * Fixed multiplayer degenerate orientation case problem. Make sure warp
 * effect never goes lower than LOD 1. 
 * 
 * 11    9/06/99 11:25a Mikek
 * Decrease some settings for High detail level.
 * 
 * 10    8/05/99 2:05a Dave
 * Whee.
 * 
 * 9     7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 8     7/29/99 12:05a Dave
 * Nebula speed optimizations.
 * 
 * 7     6/22/99 7:03p Dave
 * New detail options screen.
 * 
 * 6     6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 5     5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 4     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 3     10/07/98 6:27p Dave
 * Globalized mission and campaign file extensions. Removed Silent Threat
 * special code. Moved \cache \players and \multidata into the \data
 * directory.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 38    9/21/98 8:46p Dave
 * Put in special check in fred for identifying unknown ships.
 * 
 * 37    8/17/98 5:07p Dave
 * First rev of corkscrewing missiles.
 * 
 * 36    5/13/98 11:34p Mike
 * Model caching system.
 * 
 * 35    4/25/98 4:06p John
 * Made defaults make a little more sense
 * 
 * 34    4/20/98 8:41p John
 * Made debris culling actually reduce Glide texture resolution.
 * 
 * 33    4/12/98 9:56a John
 * Made lighting detail flags work.   Made explosions cast light on
 * highest.
 * 
 * 32    4/08/98 8:34p Lawrance
 * Fix up how custom button works on the detail tab.
 * 
 * 31    4/01/98 5:34p John
 * Made only the used POFs page in for a level.   Reduced some interp
 * arrays.    Made custom detail level work differently.
 * 
 * 30    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 29    3/30/98 2:38p Mike
 * Add asteroid_density to detail level support.
 * No force explosion damage in training missions.
 * Make cargo deathrolls shorter.
 * 
 * 28    3/24/98 8:16a John
 * made highest detail different than next one down
 * 
 * 27    3/23/98 5:19p John
 * Upped number of default detail levels to 4
 * 
 * 26    3/22/98 4:11p Andsager
 * Remove global FreeSpace_running
 * 
 * 25    3/22/98 3:28p John
 * Added in stippled alpha for lower details.  Made medium detail use
 * engine glow.
 * 
 * 24    3/22/98 2:41p John
 * Added lighting into the detail structure.
 * 
 * 23    3/22/98 11:06a John
 * Changed default detail levels
 * 
 * 22    3/22/98 11:02a John
 * Made a bunch of the detail levels actually do something
 * 
 * 21    3/22/98 9:53a John
 * Added in first stage of new detail level stuff
 * 
 * 20    3/09/98 12:13a Andsager
 * Add code to help find jumps in position.
 * 
 * 19    2/28/98 7:03p Lawrance
 * get slew working in any view
 * 
 * 18    2/16/98 4:16p John
 * Added loading animation
 * 
 * 17    2/10/98 9:06a John
 * Added variables for restoring
 * 
 * 16    2/05/98 9:27p John
 * took out ending min/max etc on monitor
 * 
 * 15    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 14    2/03/98 9:25p John
 * Upgraded Direct3D code to new version 5.0 code.  Separated the D3D code
 * more.  Added a global variable D3D_enabled flag.
 * 
 * 13    1/17/98 12:17p John
 * fixed erroneous print
 * 
 * 12    1/17/98 12:14p John
 * Added loading... bar to freespace.
 * 
 * 11    1/11/98 2:15p John
 * Changed a lot of stuff that had to do with bitmap loading.   Made cfile
 * not do callbacks, I put that in global code.   Made only bitmaps that
 * need to load for a level load.
 * 
 * 10    12/16/97 6:20p Hoffoss
 * Added more debugging code for demos, and fixed a bug in demo
 * recording/playback.
 * 
 * 9     12/05/97 3:46p John
 * made ship thruster glow scale instead of being an animation.
 * 
 * 8     8/05/97 10:18a Lawrance
 * my_rand() being used temporarily instead of rand()
 * 
 * 7     8/04/97 10:21a Dave
 * Added Is_standalone global var
 * 
 * 6     6/24/97 6:21p John
 * added detail flags.
 * sped up motion debris system a bit.
 * 
 * 5     4/15/97 11:28p Mike
 * External view system
 * 
 * 4     4/15/97 4:00p Mike
 * Intermediate checkin caused by getting other files.  Working on camera
 * slewing system.
 * 
 * 3     4/08/97 8:47a John
 * Added a global varible for detail level
 * 
 * 2     4/01/97 11:07p Mike
 * Clean up game sequencing functions.  Get rid of Multiplayer and add
 * Game_mode.  Add SystemVars.cpp
 * 
 * 1     4/01/97 10:59p Mike
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "io/timer.h"
#include "nebula/neb.h"
#include "graphics/2d.h"


fix Missiontime;
fix Frametime;
int	Framecount=0;

int Game_mode;

int Game_restoring = 0;		// If set, this means we are restoring data from disk

int	Viewer_mode;		//	Viewer's mode, see VM_xxxx flags.

//CUTSCENE STUFF
//Cutscene flags
int Cutscene_bar_flags = CUB_NONE;
//Time for gradual change in seconds
float Cutscene_delta_time = 1.0f;
//How far along a change is (0 to 1)
float Cutscene_bars_progress = 1.0f;

//FADEIN STUFF
shader Viewer_shader;
int Fade_type = FI_NONE;
float Fade_delta_time = 1.0f;

// The detail level.  Anything below zero draws simple models earlier than it
// should.   Anything above zero draws higher detail models longer than it should.
// -2=lowest
// -1=low
// 0=normal (medium)	
// 1=high
// 2=extra high
int Game_detail_level = 0;
uint Game_detail_flags = DETAIL_DEFAULT;	// see systemvars.h for explanation

angles	Viewer_slew_angles;			//	Angles of viewer relative to forward.
vei		Viewer_external_info;		//	Viewer angles to ship in external view.
vci		Viewer_chase_info;			// View chase camera information
vec3d leaning_position;

int Is_standalone;
int Rand_count;

int Interface_last_tick = -1;			// last timer tick on flip

#ifndef NDEBUG
// for debugging, used to print the currently processing filename on the loading screen
char Processing_filename[MAX_PATH_LEN];
#endif

// override states to skip rendering of certain elements, but without disabling them completely
bool Basemap_override = false;
bool Envmap_override = false;
bool Specmap_override = false;
bool Normalmap_override = false;
bool Heightmap_override = false;
bool Glowpoint_override = false;
bool GLSL_override = false;

// Values used for noise for thruster animations
float Noise[NOISE_NUM_FRAMES] = { 
	0.468225f,
	0.168765f,
	0.318945f,
	0.292866f,
	0.553357f,
	0.468225f,
	0.180456f,
	0.418465f,
	0.489958f,
	1.000000f,
	0.468225f,
	0.599820f,
	0.664718f,
	0.294215f,
	0.000000f
};


int myrand()
{
	int rval;
	rval = rand();
	Rand_count++;
//	nprintf(("Alan","RAND: %d\n", rval));
	return rval;
}

// returns a random number between 0 and 0x7fffffff
int rand32()
{
	if (RAND_MAX == 0x7fff) {
		int random32;
		// this gets two random 16 bit numbers and stuffs them into a 32 bit number
		random32 = (rand() << 16) | rand();
		//since rand() returns between 0 and 0x7fff, there needs to be a thing to randomly generate the 15th bit
		random32 |= ((rand() & 1) << 15);
		
		return random32;
	}
	else {
		// karajorma - can't imagine what RAND_MAX could be if it's not 0x7fffffff but let's be careful
		Assert(RAND_MAX == 0x7fffffff);
		return rand();
	}
}

// Variables for the loading callback hooks
static int cf_timestamp = -1;
static void (*cf_callback)(int count) = NULL;
static int cf_in_callback = 0;	
static int cb_counter = 0;
static int cb_last_counter = 0;
static int cb_delta_step = -1;

// Call this with the name of a function.  That function will
// then get called around 10x per second.  The callback function
// gets passed a 'count' which is how many times game_busy has
// been called since the callback was set.   It gets called
// one last time with count=-1 when you turn off the callback
// by calling game_busy_callback(NULL).   Game_busy_callback
// returns the current count, so you can tell how many times
// game_busy got called.
int game_busy_callback( void (*callback)(int count), int delta_step )
{
	if ( !callback ) {

		// Call it once more to finalize things
		cf_in_callback++;
		(*cf_callback)(-1);
		cf_in_callback--;

		cf_timestamp = -1;
		cf_callback = NULL;
	} else {
		cb_counter = 0;
		cb_last_counter = 0;
		cb_delta_step = delta_step;
		cf_timestamp = timer_get_milliseconds()+(1000/10);
		cf_callback = callback;

		// Call it once
		cf_in_callback++;
		(*cf_callback)(0);		// pass 0 first time!
		cf_in_callback--;
	
	}

	return cb_counter;
}

// Call whenever loading to display cursor
void game_busy(char *filename)
{
	if ( cf_in_callback != 0 ) return;	// don't call callback if we're already in it.
	if ( cf_timestamp < 0 ) return;
	if ( !cf_callback ) return;

	cb_counter++;

//	mprintf(( "CB_COUNTER=%d\n", cb_counter ));

#ifndef NDEBUG
	if (filename != NULL) 
		strncpy(Processing_filename, filename, MAX_PATH_LEN);
#endif

	int t1 = timer_get_milliseconds();

	if ( (t1 > cf_timestamp) || ((cb_counter > cb_last_counter+155) && (cb_delta_step > 0)) )	{
		cb_last_counter = cb_counter;
		cf_in_callback++;
		(*cf_callback)(cb_counter);
		cf_in_callback--;
		cf_timestamp = t1 + (1000/10);
	}
}

//======================== CODE TO MONITOR EVENTS ======================

#ifndef NDEBUG

#define MAX_MONITORS 64

static int Num_monitors = 0;
static monitor *Monitor[MAX_MONITORS];

monitor::monitor( char *_name )
{
	int i;

	if ( Num_monitors >= MAX_MONITORS )	{
		Int3();			// Too many monitor variables!! Increase MAX_MONITORS!!
		return;
	}

	for (i=0; i<Num_monitors; i++ )	{
		int ret  = stricmp( Monitor[i]->name, _name );

		if ( ret == 0)	{
			Int3();		// This monitor variable already exists!!!! 
			return;
		} else if ( ret > 0 )	{
			break;		// Insert it here

		} else if ( ret < 0 )	{
			// do nothing
		}
	}

	if ( i < Num_monitors )	{
		// Insert it at element i
		int j;
		for (j=Num_monitors; j>i; j-- )	{
			Monitor[j] = Monitor[j-1];
		}
		Monitor[i] = this;		
		Num_monitors++;
	} else {
		Monitor[Num_monitors] = this;		
		Num_monitors++;
	}

	name = _name;
	value = 0;
}


int Monitor_inited = 0;
char Monitor_filename[128];
fix monitor_last_time = -1;

DCF(monitor,"Monitors game performace")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING|ARG_NONE);
		if ( Dc_arg_type == ARG_NONE )	{
			if ( Monitor_inited )	{
				Monitor_inited = 0;

/*
				FILE *fp = fopen( Monitor_filename, "at" );
				if ( fp )	{
					fprintf( fp, "\n\n" );
					fprintf( fp, "Name\tMin\tMax\tAvg\n" );
					for (int i=0; i<Num_monitors; i++ )	{
						if ( Monitor[i]->cnt > 0 )	{
							fprintf( fp, "%s\t%d\t%d\t%d\n", Monitor[i]->name, Monitor[i]->min, Monitor[i]->max, Monitor[i]->sum / Monitor[i]->cnt  );
						} else {
							fprintf( fp, "%s\t%d\t%d\t?\n", Monitor[i]->name, Monitor[i]->min, Monitor[i]->max );
						}
					}
					fclose(fp);
				}
*/

				dc_printf( "Monitor to file '%s' turned off\n", Monitor_filename );
			} else {
				dc_printf( "Monitor isn't on\n" );
			}
		} else {
			if ( Monitor_inited )	{
				dc_printf( "Monitor already on\n" );
			} else {
				Monitor_inited = 1;

				strcpy( Monitor_filename, Dc_arg );

				// Reset them all
				int i;
				for (i=0; i<Num_monitors; i++ )	{
					Monitor[i]->value = 0;
					Monitor[i]->sum = 0;
					Monitor[i]->cnt = 0;
					Monitor[i]->min = 0;
					Monitor[i]->max = 0;
				}

				FILE *fp = fopen( Monitor_filename, "wt" );
				if ( fp )	{
					for (i=0; i<Num_monitors; i++ )	{
						if ( i > 0 )	{
							fprintf( fp, "\t" );
						}
						fprintf( fp, "%s", Monitor[i]->name );
					
					}
					fprintf( fp, "\n" );
					fclose(fp);
				}
				dc_printf( "Monitor outputting to file '%s'\n", Monitor_filename );
				monitor_last_time = -1;
			}
		}
	}
	if ( Dc_help )	{
		dc_printf( "Usage: monitor filename\nOutputs monitoring info to filename. No filename turns it off\n" );
	}
	
}


MONITOR(FrameRateX100)

void monitor_update()
{
	int i;
	FILE * fp;

	fix this_time = timer_get_fixed_seconds();
	fix frametime;

	if ( monitor_last_time != -1 )	{
		frametime = this_time - monitor_last_time;
	} else {
		frametime = 0;
	}

	if ( frametime > 0 )	{
		MONITOR_INC(FrameRateX100, (F1_0*100) / frametime );
	} else {
		MONITOR_INC(FrameRateX100, 0 );
	}

		
	if ( !Monitor_inited )	{
		return;
	}

	if ( frametime != 0 )	{
		fp = fopen( Monitor_filename, "at" );
		if ( fp )	{

			for (i=0; i<Num_monitors; i++ )	{
				if (i>0) fprintf( fp, "\t" );
				fprintf( fp, "%d", Monitor[i]->value );
			}
			fprintf( fp, "\n" );
			fclose(fp);
		}

		for (i=0; i<Num_monitors; i++ )	{

			// Record stats
			Monitor[i]->sum += Monitor[i]->value;

			if ( (Monitor[i]->cnt < 1)  || (Monitor[i]->value < Monitor[i]->min ))	{
				Monitor[i]->min = Monitor[i]->value;
			}

			if ( (Monitor[i]->cnt < 1)  || (Monitor[i]->value > Monitor[i]->max ))	{
				Monitor[i]->max = Monitor[i]->value;
			}

			Monitor[i]->cnt++;

			//	Reset the value
			Monitor[i]->value = 0;
		}
	} else {
		for (i=0; i<Num_monitors; i++ )	{
			//	Reset the value
			Monitor[i]->value = 0;
		}
	}

	monitor_last_time = timer_get_fixed_seconds();

}
#endif	//NDEBUG


#if MAX_DETAIL_LEVEL != 4
#error MAX_DETAIL_LEVEL is assumed to be 4 in SystemVars.cpp
#endif

#if NUM_DEFAULT_DETAIL_LEVELS != 4
#error NUM_DEFAULT_DETAIL_LEVELS is assumed to be 4 in SystemVars.cpp
#endif

// Detail level stuff
detail_levels Detail_defaults[NUM_DEFAULT_DETAIL_LEVELS] = {
	{				// Low
		0,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		0,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		0,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		0,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		0,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		2,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

					// ====  Booleans ====
		0,			//	targetview_model;			// 0=off, 1=on		
		0,			//	planets_suns;				// 0=off, 1=on		
		0,			// weapon_extras
	},
	{				// Medium
		1,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		1,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		1,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		1,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		2,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		2,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		2,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		1,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		3,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

		// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon extras				
	},
	{				// High level
		2,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		2,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		3,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		3,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		3,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		3,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		3,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		4,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

										// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon_extras
	},
	{				// Highest level
		3,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		3,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		3,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		4,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		4,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		3,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		4,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

										// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon_extras
	},
};


// Global used to access detail levels in game and libs
detail_levels Detail = Detail_defaults[NUM_DEFAULT_DETAIL_LEVELS-1];

// Call this with:
// 0 - lowest
// NUM_DETAIL_LEVELS - highest
// To set the parameters in Detail to some set of defaults
void detail_level_set(int level)
{
	if ( level < 0 )	{
		Detail.setting = -1;
		return;
	}
	Assert( level >= 0 );
	Assert( level < NUM_DEFAULT_DETAIL_LEVELS );

	Detail = Detail_defaults[level];

	// reset nebula stuff
	neb2_set_detail_level(level);
}

// Returns the current detail level or -1 if custom.
int current_detail_level()
{
//	return Detail.setting;
	int i;

	for (i=0; i<NUM_DEFAULT_DETAIL_LEVELS; i++ )	{
		if ( memcmp( &Detail, &Detail_defaults[i], sizeof(detail_levels) )==0 )	{
			return i;
		}
	}
	return -1;
}

#ifndef NDEBUG
DCF(detail_level,"Change the detail level")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_INT|ARG_NONE);
		if ( Dc_arg_type & ARG_NONE )	{
			Game_detail_level = 0;
			dc_printf( "Detail level reset\n" );
		}
		if ( Dc_arg_type & ARG_INT )	{
			Game_detail_level = Dc_arg_int;
		}
	}

	if ( Dc_help )	
		dc_printf( "Usage: detail_level [n]\nn is detail level. 0 normal, - lower, + higher, -2 to 2 usually\nNo parameter resets it to default.\n" );

	if ( Dc_status )				
		dc_printf("Detail level set to %d\n", Game_detail_level);
}

DCF(detail, "Turns on/off parts of the game for speed testing" )
{
	if ( Dc_command )	{
		dc_get_arg(ARG_INT|ARG_NONE);
		if ( Dc_arg_type & ARG_NONE )	{
			if ( Game_detail_flags == DETAIL_DEFAULT )	{
				Game_detail_flags = DETAIL_FLAG_CLEAR;
				dc_printf( "Detail flags set lowest (except has screen clear)\n" );
			} else {
				Game_detail_flags = DETAIL_DEFAULT;
				dc_printf( "Detail flags set highest\n" );
			}
		}
		if ( Dc_arg_type & ARG_INT )	{
			Game_detail_flags ^= Dc_arg_int;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: detail [n]\nn is detail bit to toggle.\n" );
		dc_printf( "   1: draw the stars\n" );
		dc_printf( "   2: draw the nebulas\n" );
		dc_printf( "   4: draw the motion debris\n" );
		dc_printf( "   8: draw planets\n" );
		dc_printf( "  16: draw models not as blobs\n" );
		dc_printf( "  32: draw lasers not as pixels\n" );
		dc_printf( "  64: clear screen background after each frame\n" );
		dc_printf( " 128: draw hud stuff\n" );
		dc_printf( " 256: draw fireballs\n" );
		dc_printf( " 512: do collision detection\n" );
	}

	if ( Dc_status )	{
		dc_printf("Detail flags set to 0x%08x\n", Game_detail_flags);
		dc_printf( "   1: draw the stars: %s\n", (Game_detail_flags&1?"on":"off") );
		dc_printf( "   2: draw the nebulas: %s\n", (Game_detail_flags&2?"on":"off") );
		dc_printf( "   4: draw the motion debris: %s\n", (Game_detail_flags&4?"on":"off")  );
		dc_printf( "   8: draw planets: %s\n", (Game_detail_flags&8?"on":"off")  );
		dc_printf( "  16: draw models not as blobs: %s\n", (Game_detail_flags&16?"on":"off")  );
		dc_printf( "  32: draw lasers not as pixels: %s\n", (Game_detail_flags&32?"on":"off")  );
		dc_printf( "  64: clear screen background after each frame: %s\n", (Game_detail_flags&64?"on":"off")  );
		dc_printf( " 128: draw hud stuff: %s\n", (Game_detail_flags&128?"on":"off")  );
		dc_printf( " 256: draw fireballs: %s\n", (Game_detail_flags&256?"on":"off")  );
		dc_printf( " 512: do collision detection: %s\n", (Game_detail_flags&512?"on":"off")  );
	}
}
#endif

// Goober5000
// (Taylor says that for optimization purposes malloc/free should be used rather than vm_malloc/vm_free here)
void insertion_sort(void *array_base, size_t array_size, size_t element_size, int (*fncompare)(const void *, const void *))
{
	int i, j;
	void *current;
	char *array_byte_base;
	
	// this is used to avoid having to cast array_base to (char *) all the time
	array_byte_base = (char *) array_base;

	// allocate space for the element being moved
	current = malloc(element_size);
	if (current == NULL)
	{
		Int3();
		return;
	}

	// loop
	for (i = 1; (unsigned) i < array_size; i++)
	{	
		// grab the current element
		memcpy(current, array_byte_base + (i * element_size), element_size);

		// bump other elements toward the end of the array
		for (j = i - 1; (j >= 0) && (fncompare(array_byte_base + (j * element_size), current) > 0); j--)
		{
			memcpy(array_byte_base + ((j + 1) * element_size), array_byte_base + (j * element_size), element_size);
		}

		// insert the current element at the correct place
		memcpy(array_byte_base + ((j + 1) * element_size), current, element_size);
	}

	// free the allocated space
	free(current);
}
