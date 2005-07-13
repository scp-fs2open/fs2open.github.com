/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/SystemVars.h $
 * $Revision: 2.10 $
 * $Date: 2005-07-13 02:50:48 $
 * $Author: Goober5000 $
 *
 * Variables and constants common to FreeSpace and Fred.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.9  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.8  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.7  2005/01/16 22:39:09  wmcoolmon
 * Added VM_TOPDOWN view; Added 2D mission mode, add 16384 to mission +Flags to use.
 *
 * Revision 2.6  2004/10/31 21:33:05  taylor
 * rename OGL_inited to OGL_enabled to match D3D version
 *
 * Revision 2.5  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.4  2004/02/20 04:29:54  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.3  2003/09/26 14:37:14  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.2  2002/10/05 16:46:09  randomtiger
 * Added us fs2_open people to the credits. Worth looking at just for that.
 * Added timer bar code, by default its not compiled in.
 * Use TIMEBAR_ACTIVE in project and dependancy code settings to activate.
 * Added the new timebar files with the new code.
 *
 * Revision 2.1  2002/08/15 04:41:17  penguin
 * Added #include, needed for FRED
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 11    6/22/99 7:03p Dave
 * New detail options screen.
 * 
 * 10    6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 9     6/14/99 10:45a Dave
 * Made beam weapons specify accuracy by skill level in the weapons.tbl
 * 
 * 8     5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 7     3/29/99 6:17p Dave
 * More work on demo system. Got just about everything in except for
 * blowing ships up, secondary weapons and player death/warpout.
 * 
 * 6     3/28/99 5:58p Dave
 * Added early demo code. Make objects move. Nice and framerate
 * independant, but not much else. Don't use yet unless you're me :)
 * 
 * 5     3/28/99 12:37p Dave
 * Tentative beginnings to warpin effect.
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
 * 37    9/21/98 8:46p Dave
 * Put in special check in fred for identifying unknown ships.
 * 
 * 36    8/17/98 5:07p Dave
 * First rev of corkscrewing missiles.
 * 
 * 35    5/13/98 11:34p Mike
 * Model caching system.
 * 
 * 34    5/09/98 4:52p Lawrance
 * Implement padlock view (up/rear/left/right)
 * 
 * 33    4/01/98 5:34p John
 * Made only the used POFs page in for a level.   Reduced some interp
 * arrays.    Made custom detail level work differently.
 * 
 * 32    3/31/98 5:18p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 *  
 * 
 * 31    3/30/98 2:38p Mike
 * Add asteroid_density to detail level support.
 * No force explosion damage in training missions.
 * Make cargo deathrolls shorter.
 * 
 * 30    3/23/98 5:19p John
 * Upped number of default detail levels to 4
 * 
 * 29    3/22/98 2:41p John
 * Added lighting into the detail structure.
 * 
 * 28    3/22/98 11:06a John
 * Changed default detail levels
 * 
 * 27    3/22/98 11:02a John
 * Made a bunch of the detail levels actually do something
 * 
 * 26    3/22/98 9:53a John
 * Added in first stage of new detail level stuff
 * 
 * 25    3/17/98 3:53p Lawrance
 * Fix warpout in chase view
 * 
 * 24    2/28/98 7:03p Lawrance
 * get slew working in any view
 * 
 * 23    2/26/98 12:33a Lawrance
 * Added back slew mode,  lots of changes to external and chase views.
 * 
 * 22    2/10/98 9:06a John
 * Added variables for restoring
 * 
 * 21    2/03/98 9:25p John
 * Upgraded Direct3D code to new version 5.0 code.  Separated the D3D code
 * more.  Added a global variable D3D_enabled flag.
 * 
 * 20    1/13/98 2:20p John
 * Added code to load palette based on hud color.  Added code to turn off
 * nebulas using detail.  Added code in WinMain to time out after waiting
 * too long for window creation.    
 * 
 * 19    12/19/97 12:03p Allender
 * Added GM_CAMPAIGN_MODE to indicate when player is in a campaign or not.
 * Started adding FreeSpace support for carrying of ship/weapon types
 * across missions in a campaign.
 * 
 * 18    12/16/97 6:20p Hoffoss
 * Added more debugging code for demos, and fixed a bug in demo
 * recording/playback.
 * 
 * 17    12/05/97 3:46p John
 * made ship thruster glow scale instead of being an animation.
 * 
 * 16    9/22/97 5:12p Dave
 * Added stats transfer game _mode_. Started work on multiplayer chat
 * screens for weapon and ship select
 * 
 * 15    8/04/97 5:28p Dave
 * Moved Game bit GM_GAME_HOST to a netinfo flag NETINFO_FLAG_GAME_HOST
 * 
 * 14    8/04/97 4:37p Dave
 * Added GM_STANDALONE_SERVER and GM_GAME_HOST game mode bits
 * 
 * 13    8/04/97 10:21a Dave
 * Added Is_standalone global var
 * 
 * 12    7/29/97 10:59a Dave
 * Added demo recording and demo playback bit flags
 * 
 * 11    6/24/97 6:21p John
 * added detail flags.
 * sped up motion debris system a bit.
 * 
 * 10    6/09/97 10:38a Allender
 * added GAME_MODE_IN_GAME flag.  Probably to be used only for multiplayer
 * 
 * 9     5/07/97 1:44p Mike
 * Add viewing from other ships.
 * 
 * 8     4/16/97 3:24p Mike
 * New Camera system.
 * 
 * 7     4/15/97 11:28p Mike
 * External view system
 * 
 * 6     4/15/97 4:00p Mike
 * Intermediate checkin caused by getting other files.  Working on camera
 * slewing system.
 * 
 * 5     4/11/97 5:02p Mike
 * Improve death sequence, clean up sequencing.
 * 
 * 4     4/08/97 8:47a John
 * Added a global varible for detail level
 * 
 * 3     4/02/97 6:03p Mike
 * Make dying work through event driven code.
 * 
 * 2     4/01/97 11:07p Mike
 * Clean up game sequencing functions.  Get rid of Multiplayer and add
 * Game_mode.  Add SystemVars.cpp
 * 
 * 1     4/01/97 10:59p Mike
 *
 * $NoKeywords: $
 */

#ifndef _SYSTEMVARS_H
#define _SYSTEMVARS_H

#include "globalincs/pstypes.h"
#include "math.h"

#define	GM_MULTIPLAYER					(1 << 0)
#define	GM_NORMAL						(1 << 1)
#define	GM_DEAD_DIED					(1 << 2)				//	Died, waiting to blow up.
#define	GM_DEAD_BLEW_UP				(1 << 3)				//	Blew up.
#define	GM_DEAD_ABORTED				(1 << 4)				//	Player pressed a key, aborting death sequence.
#define	GM_IN_MISSION					(1 << 5)				// Player is actually in the mission -- not at a pre-mission menu

#define	GM_DEAD							(GM_DEAD_DIED | GM_DEAD_BLEW_UP | GM_DEAD_ABORTED)

#define  GM_STANDALONE_SERVER			(1 << 8)
#define	GM_STATS_TRANSFER				(1 << 9)				// in the process of stats transfer
#define	GM_CAMPAIGN_MODE				(1 << 10)			// are we currently in a campaign.

#define	GM_DEMO_RECORD					(1 << 11)			// recording a demo
#define	GM_DEMO_PLAYBACK				(1 << 12)			// playing a demo back
#define	GM_DEMO							(GM_DEMO_RECORD | GM_DEMO_PLAYBACK)			// true whenever a demo is being recorded or played back

#define	VM_EXTERNAL						(1 << 0)				//	Set if not viewing from player position.
#define	VM_SLEWED						(1 << 1)				//	Set if viewer orientation is slewed.
#define	VM_DEAD_VIEW					(1 << 2)				//	Set if viewer is watching from dead view.
#define	VM_CHASE							(1 << 3)				//	Chase view.
#define	VM_OTHER_SHIP					(1 << 4)				//	View from another ship.
#define	VM_EXTERNAL_CAMERA_LOCKED	(1 << 5)				// External camera is locked in place (ie controls move ship not camera)
#define	VM_WARP_CHASE					(1	<< 6)				// View while warping out (form normal view mode)
#define	VM_PADLOCK_UP					(1 << 7)
#define	VM_PADLOCK_REAR				(1 << 8)
#define	VM_PADLOCK_LEFT				(1 << 9)
#define	VM_PADLOCK_RIGHT				(1 << 10)
#define	VM_WARPIN_ANCHOR				(1 << 11)			// special warpin camera mode
#define VM_TOPDOWN					(1 << 12)				//Camera is looking down on ship
#define VM_FREECAMERA				(1 << 13)				//Camera is not attached to any particular object, probably under SEXP control

#define	VM_PADLOCK_ANY (VM_PADLOCK_UP|VM_PADLOCK_REAR|VM_PADLOCK_LEFT|VM_PADLOCK_RIGHT)

//-----Cutscene stuff
//No bars
#define CUB_NONE			0
//List of types of bars
#define CUB_CUTSCENE		(1<<0)
//Styles to get to bars
#define CUB_GRADUAL			(1<<15)
extern float Cutscene_bars_progress, Cutscene_delta_time;
extern int Cutscene_bar_flags;

//-----Fadein stuff
struct shader;
extern shader Viewer_shader;
#define FI_NONE					0
#define FI_FADEIN				1
#define FI_FADEOUT				2
extern float Fade_delta_time;
extern int Fade_type;


typedef struct vei {
	angles_t	angles;			//	Angles defining viewer location.
	float		distance;		//	Distance from which to view, plus 2x radius.
} vei;

typedef struct vci {	
	angles_t	angles;			
	float		distance;		// Distance from which to view, plus 3x radius
} vci;

extern fix Missiontime;
extern fix Frametime;
extern int Framecount;

extern int Game_mode;

extern int Viewer_mode;
extern int Rand_count;

extern int Game_restoring;		// If set, this means we are restoring data from disk

// The detail level.  Anything below zero draws simple models earlier than it
// should.   Anything above zero draws higher detail models longer than it should.
// -2=lowest
// -1=low
// 0=normal (medium)	
// 1=high
// 2=extra high
extern int Game_detail_level;

#define DETAIL_DEFAULT (0xFFFFFFFF)

#define DETAIL_FLAG_STARS			(1<<0)	// draw the stars
#define DETAIL_FLAG_NEBULAS		(1<<1)	// draw the motion debris
#define DETAIL_FLAG_MOTION			(1<<2)	// draw the motion debris
#define DETAIL_FLAG_PLANETS		(1<<3)	// draw planets
#define DETAIL_FLAG_MODELS			(1<<4)	// draw models not as blobs
#define DETAIL_FLAG_LASERS			(1<<5)	// draw lasers not as pixels
#define DETAIL_FLAG_CLEAR			(1<<6)	// clear screen background after each frame
#define DETAIL_FLAG_HUD				(1<<7)	// draw hud stuff
#define DETAIL_FLAG_FIREBALLS		(1<<8)	// draw fireballs
#define DETAIL_FLAG_COLLISION		(1<<9)	// use good collision detection


extern uint Game_detail_flags;

extern angles	Viewer_slew_angles;
extern vei		Viewer_external_info;
extern vci		Viewer_chase_info;

extern int Is_standalone;
extern int Interface_framerate;				// show interface framerate during flips
extern int Interface_last_tick;				// last timer tick on flip

// for notifying players of unknown ship types
extern int Fred_found_unknown_ship_during_parsing;

#define NOISE_NUM_FRAMES 15

// Noise numbers go from 0 to 1.0
extern float Noise[NOISE_NUM_FRAMES];


// If true, then we are using Direct3D hardware.  This is used for game type stuff
// that changes when you're using hardware.
extern int D3D_enabled;
// for OGL
extern int OGL_enabled;

// game skill levels 
#define	NUM_SKILL_LEVELS	5

//====================================================================================
// DETAIL LEVEL STUFF
// If you change any of this, be sure to increment the player file version 
// in Freespace\ManagePilot.cpp and change Detail_defaults in SystemVars.cpp 
// or bad things will happen, I promise.
//====================================================================================

#define MAX_DETAIL_LEVEL 4			// The highest valid value for the "analog" detail level settings

// If you change this, update player file in ManagePilot.cpp 
typedef struct detail_levels {

	int		setting;						// Which default setting this was created from.   0=lowest... NUM_DEFAULT_DETAIL_LEVELS-1, -1=Custom

	// "Analogs"
	int		nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
	int		detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest	
	int		hardware_textures;		// 0=max culling, MAX_DETAIL_LEVEL=no culling
	int		num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
	int		num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
	int		num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
	int		shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
	int		lighting;					// 0=min, MAX_DETAIL_LEVEL=max	

	// Booleans
	int		targetview_model;			// 0=off, 1=on	
	int		planets_suns;				// 0=off, 1=on			
	int		weapon_extras;				// extra weapon details. trails, glows
} detail_levels;

// Global values used to access detail levels in game and libs
extern detail_levels Detail;

#define NUM_DEFAULT_DETAIL_LEVELS	4	// How many "predefined" detail levels there are

// Call this with:
// 0 - lowest
// NUM_DEFAULT_DETAIL_LEVELS - highest
// To set the parameters in Detail to some set of defaults
void detail_level_set(int level);

// Returns the current detail level or -1 if custom.
int current_detail_level();

//====================================================================================
// Memory stuff from WinDebug.cpp
extern int TotalRam;
void windebug_memwatch_init();

enum
{
	TIMERBAR_DEFAULT,
	TIMERBAR_D3DCODE,
};

#define MAX_LIGHTS 256
#define MAX_LIGHT_LEVELS 16

#define safe_kill(a) if(a)vm_free(a)


#endif
