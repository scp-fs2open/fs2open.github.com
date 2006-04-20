/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FREESPACE2/FreeSpace.h $
 * $Revision: 2.12 $
 * $Date: 2006-04-20 06:32:01 $
 * $Author: Goober5000 $
 *
 * FreeSpace, the game, not the project, header information.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.11  2006/01/16 11:02:23  wmcoolmon
 * Various warning fixes, scripting globals fix; added "plr" and "slf" global variables for in-game hooks; various lua functions; GCC fixes for scripting.
 *
 * Revision 2.10  2006/01/14 19:54:55  wmcoolmon
 * Special shockwave and moving capship bugfix, (even more) scripting stuff, slight rearrangement of level management functions to facilitate scripting access.
 *
 * Revision 2.9  2005/12/06 03:13:49  taylor
 * fix quite a few CFILE issues:
 *   use #define's for path lengths when possible so it's easier to move between functions
 *   fix huge Cfile_stack[] issue (how the hell did that get through :v: QA?)
 *   add Int3() check on cfopen() so it's easier to know if it get's called before cfile is ready to use
 *   move path separators to pstypes.h
 *   fix possible string overruns when setting up CFILE roots
 *   make sure we don't try to init current directory again thinking it's a CD-ROM
 *   add the list of VP roots to debug log, this will undoubtedly be useful
 * when -nosound is use go ahead and set -nomusic too to both checks are correct
 * add list of cmdline options to debug log
 * fix possible overwrite issues with get_version_string() and remove '(fs2_open)' from string plus change OGL->OpenGL, D3D->Direct3D
 *
 * Revision 2.8  2005/10/10 17:16:22  taylor
 * remove NO_NETWORK
 * whether multi is disabled or not is now determined at runtime
 * clean out some crap and old debug messages that were littered about
 *
 * Revision 2.7  2005/07/13 02:50:52  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.6  2005/06/03 06:39:26  taylor
 * better audio pause/unpause support when game window loses focus or is minimized
 *
 * Revision 2.5  2005/03/03 06:05:27  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.4  2004/10/31 21:31:34  taylor
 * bump COUNT_ESTIMATE, reset time compression at the start of a mission, new pilot file support, add feature_disabled popup
 *
 * Revision 2.3  2004/08/11 05:06:22  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/04/07 03:31:53  righteous1
 * Updated to add alt_tab_pause() function to draw pause screen and discontinue sounds when the game is minimized. -R1
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
 * 24    10/06/99 10:32a Jefff
 * oem updates
 * 
 * 23    9/30/99 6:04p Jefff
 * OEM changes
 * 
 * 22    9/08/99 3:22p Dave
 * Updated builtin mission list.
 * 
 * 21    9/06/99 6:38p Dave
 * Improved CD detection code.
 * 
 * 20    9/06/99 1:30a Dave
 * Intermediate checkin. Started on enforcing CD-in-drive to play the
 * game.
 * 
 * 19    9/06/99 1:16a Dave
 * Make sure the user sees the intro movie.
 * 
 * 18    9/05/99 11:19p Dave
 * Made d3d texture cache much more safe. Fixed training scoring bug where
 * it would backout scores without ever having applied them in the first
 * place.
 * 
 * 17    9/03/99 1:32a Dave
 * CD checking by act. Added support to play 2 cutscenes in a row
 * seamlessly. Fixed super low level cfile bug related to files in the
 * root directory of a CD. Added cheat code to set campaign mission # in
 * main hall.
 * 
 * 16    8/19/99 10:12a Alanl
 * preload mission-specific messages on machines greater than 48MB
 * 
 * 15    5/19/99 4:07p Dave
 * Moved versioning code into a nice isolated common place. Fixed up
 * updating code on the pxo screen. Fixed several stub problems.
 * 
 * 14    5/09/99 8:57p Dave
 * Final E3 build preparations.
 * 
 * 13    5/05/99 10:06a Dave
 * Upped beta version to 0.04
 * 
 * 12    4/29/99 3:02p Dave
 * New beta version.
 * 
 * 11    4/25/99 3:02p Dave
 * Build defines for the E3 build.
 * 
 * 10    4/09/99 2:27p Dave
 * Upped version #
 * 
 * 9     4/09/99 2:21p Dave
 * Multiplayer beta stuff. CD checking.
 * 
 * 8     4/08/99 2:10a Dave
 * Numerous bug fixes for the beta. Added builtin mission info for the
 * beta.
 * 
 * 7     3/19/99 9:52a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 6     12/03/98 5:22p Dave
 * Ported over FreeSpace 1 multiplayer ships.tbl and weapons.tbl
 * checksumming.
 * 
 * 5     10/13/98 9:26a Dave
 * Began neatening up freespace.h.
 * 
 * 4     10/09/98 1:35p Dave
 * Split off registry stuff into seperate file.
 * 
 * 3     10/07/98 6:27p Dave
 * Globalized mission and campaign file extensions. Removed Silent Threat
 * special code. Moved \cache \players and \multidata into the \data
 * directory.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin. 
 * 
 * 79    9/13/98 10:51p Dave
 * Put in newfangled icons for mission simulator room. New mdisk.vp
 * checksum and file length.
 * 
 * 78    9/11/98 6:59p Dave
 * First beta/rev thingie to interplay.
 * 
 * 77    9/11/98 4:14p Dave
 * Fixed file checksumming of < file_size. Put in more verbose kicking and
 * PXO stats store reporting.
 * 
 * 76    9/04/98 3:51p Dave
 * Put in validated mission updating and application during stats
 * updating.
 * 
 * 75    5/23/98 2:41p Mike
 * Make Easy the default skill level and prevent old pilot's skill level
 * from carrying into new pilot.
 * 
 * 74    5/19/98 1:19p Allender
 * new low level reliable socket reading code.  Make all missions/campaign
 * load/save to data missions folder (i.e. we are rid of the player
 * missions folder)
 * 
 * 73    5/13/98 11:34p Mike
 * Model caching system.
 * 
 * 72    5/10/98 10:05p Allender
 * only show cutscenes which have been seen before.  Made Fred able to
 * write missions anywhere, defaulting to player misison folder, not data
 * mission folder.  Fix FreeSpace code to properly read missions from
 * correct locations
 * 
 * 71    5/08/98 7:08p Dave
 * Lots of UI tweaking.
 * 
 * 70    5/08/98 5:31p Lawrance
 * extern cd checking routines 
 *
 * $NoKeywords: $
 */

#ifndef _FREESPACE_H
#define _FREESPACE_H
#ifndef STAMPER_PROGRAM							// because of all the dependancies, I have to do this...yuck!!!  MWA 7/21/97

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "graphics/2d.h"

// --------------------------------------------------------------------------------------------------------
// FREESPACE DEFINES/VARS
//
														
// filename extensions
#define FS_CAMPAIGN_FILE_EXT				NOX(".fc2")

// CDROM volume names
#ifdef MULTIPLAYER_BETA_BUILD
	#define FS_CDROM_VOLUME_1					NOX("FS2_BETA")
	#define FS_CDROM_VOLUME_2					NOX("FS2_BETA")
#elif defined(E3_BUILD)
	#define FS_CDROM_VOLUME_1					NOX("FS2_E3DEMO")
	#define FS_CDROM_VOLUME_2					NOX("FS2_E3DEMO")
#elif defined(OEM_BUILD)
	#define FS_CDROM_VOLUME_1					NOX("FS2_OEM")
	#define FS_CDROM_VOLUME_2					NOX("FS2_OEM")
	#define FS_CDROM_VOLUME_3					NOX("FS2_OEM")
#else
	#define FS_CDROM_VOLUME_1					NOX("FREESPACE2_1")
	#define FS_CDROM_VOLUME_2					NOX("FREESPACE2_2")
	#define FS_CDROM_VOLUME_3					NOX("FREESPACE2_3")

	// old volume names
	// #define FS_CDROM_VOLUME_1					NOX("FREESPACE_1")
	// #define FS_CDROM_VOLUME_2					NOX("FREESPACE_2")
	// #define FS_CDROM_VOLUME_3					NOX("FREESPACE_3")
#endif

// frametime/missiontime variables
extern fix Frametime;
extern float flRealframetime;
extern float flFrametime;
extern fix Missiontime;

// 0 - 4
extern int Game_skill_level;

// see GM_* defines in systemvars.h
extern int Game_mode;

// if this value is set anywhere within the game_do_state_common() function, the normal do_state() will not be called
// for this frame. Useful for getting out of sticky sequencing situations.
extern int Game_do_state_should_skip;

// time compression
extern bool Time_compression_locked;
extern fix Game_time_compression;

// Set if subspace is active this level
extern int Game_subspace_effect;		

// The current mission being played.
extern char Game_current_mission_filename[MAX_FILENAME_LEN];

// game's CDROM directory
extern char Game_CDROM_dir[MAX_PATH_LEN];

// if the ships.tbl the player has is valid
extern int Game_ships_tbl_valid;

// if the weapons.tbl the player has is valid
extern int Game_weapons_tbl_valid;

// to disable networking at runtime
extern int Networking_disabled;

// this is a mission actually designed at Volition
#define MAX_BUILTIN_MISSIONS					100
#define FSB_FROM_VOLITION						(1<<0)			// we made it in-house
#define FSB_MULTI									(1<<1)			// is a multiplayer mission
#define FSB_TRAINING								(1<<2)			// is a training mission
#define FSB_CAMPAIGN								(1<<3)			// is a campaign mission
#define FSB_CAMPAIGN_FILE						(1<<4)			// is actually a campaign file

typedef struct fs_builtin_mission {
	char filename[MAX_FILENAME_LEN];
	int flags;															// see FSB_* defines above
	char cd_volume[MAX_FILENAME_LEN];							// cd volume which this needs
} fs_builtin_mission;


// --------------------------------------------------------------------------------------------------------
// FREESPACE FUNCTIONS
//

// mission management -------------------------------------------------

// loads in the currently selected mission
int game_start_mission();		

// shutdown a mission
void game_level_close();


// gameplay stuff -----------------------------------------------------

// stop the game (mission) timer
void game_stop_time();

// start the game (mission) timer
void game_start_time();

// call whenever in a loop or if you need to get a keypress
int game_check_key();

// poll for keypresses
int game_poll();

// function to read keyboard stuff
void game_process_keys();

// call this when you don't want the user changing time compression
void lock_time_compression(bool is_locked);

// call this to set time compression properly
void set_time_compression(float multiplier, float change_time = 0);

//call this to change the relative time compression (ie double it)
void change_time_compression(float multiplier);

// call this to set frametime properly (once per frame)
void game_set_frametime(int state);

// Used to halt all looping game sounds
void game_stop_looped_sounds();

// do stuff that may need to be done regardless of state
void game_do_state_common(int state,int no_networking = 0);


// skill level --------------------------------------------------------

// increase the skill level (will wrap around to min skill level)
void game_increase_skill_level();

// get the default game skill level
int game_get_default_skill_level();

// a keypress.  See CPP file for more info.
void game_flush();

// running with low-memory (less than 48MB)
bool game_using_low_mem();

// misc ---------------------------------------------------------------

// lookup the specified filename. return an fs_builtin_mission* if found, NULL otherwise
fs_builtin_mission *game_find_builtin_mission(char *filename);



//================================================================
// GAME FLASH STUFF  - code in FreeSpace.cpp

// Resets the flash
void game_flash_reset();

// Adds a flash effect.  These can be positive or negative.
// The range will get capped at around -1 to 1, so stick 
// with a range like that.
void game_flash( float r, float g, float b );

// Adds a flash for Big Ship explosions
// cap range from 0 to 1
void big_explosion_flash(float flash);

// Call once a frame to diminish the
// flash effect to 0.
void game_flash_diminish();

// Loads the best palette for this level, based
// on nebula color and hud color.  You could just call palette_load_table with
// the appropriate filename, but who wants to do that.
void game_load_palette();

//================================================================

// Call at the beginning of each frame
void game_whack_reset();

// Call to apply a whack to a the ship. Used for force feedback
void game_whack_apply( float x, float y );

// call to apply a "shudder"
void game_shudder_apply(int time, float intensity);

//===================================================================

// make sure a CD is in the drive before continuing (returns 1 to continue, otherwise 0).
int game_do_cd_check(char *volume_name=NULL);
int game_do_cd_check_specific(char *volume_name, int cdnum);
int find_freespace_cd(char *volume_name=NULL);
int set_cdrom_path(int drive_num);
int game_do_cd_mission_check(char *filename);

// Used to tell the player that a feature isn't available in the demo version of FreeSpace
void game_feature_not_in_demo_popup();

// Used to tell the player that a feature is disabled by build settings
void game_feature_disabled_popup();

//	Return version string for demo or full version, depending on build.
void get_version_string(char *str, int max_size);

// format the specified time (fixed point) into a nice string
void game_format_time(fix m_time,char *time_str);

// if the game is running using hacked data
int game_hacked_data();

// show the oem upsell screens (end of campaign, or close of game
void oem_upsell_show_screens();

// calls to be executed when the game is put in or restored from minimized or inactive state
void game_pause();
void game_unpause();

//WMC - Stuff for scripting, these make the game go
extern void game_level_init(int seed = -1);
extern void game_post_level_init();
extern void game_render_frame_setup(vec3d *eye_pos, matrix *eye_orient);
extern void game_render_frame(vec3d *eye_pos, matrix *eye_orient);
extern void game_simulation_frame();
extern void game_update_missiontime();
extern void game_render_post_frame();

#endif			// endif of #ifndef STAMPER_PROGRAM
#endif 
