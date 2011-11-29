/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifdef _WIN32
 #include <direct.h>
 #include <io.h>
 #include <windows.h>
#ifndef _MINGW
 #include <crtdbg.h>
#endif // !_MINGW
#else
 #include <unistd.h>
 #include <sys/stat.h>
#endif

#include "anim/animplay.h"
#include "asteroid/asteroid.h"
#include "autopilot/autopilot.h"
#include "bmpman/bmpman.h"
#include "camera/camera.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "cmeasure/cmeasure.h"
#include "cutscene/cutscenes.h"
#include "cutscene/movie.h"
#include "debris/debris.h"
#include "exceptionhandler/exceptionhandler.h"
#include "fireball/fireballs.h"
#include "freespace2/freespace.h"
#include "freespace2/freespaceresource.h"
#include "freespace2/levelpaging.h"
#include "gamehelp/contexthelp.h"
#include "gamehelp/gameplayhelp.h"
#include "gamesequence/gamesequence.h"
#include "gamesnd/eventmusic.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/linklist.h"
#include "globalincs/version.h"
#include "globalincs/mspdb_callstack.h"
#include "graphics/font.h"
#include "hud/hud.h"
#include "hud/hudconfig.h"
#include "hud/hudescort.h"
#include "hud/hudlock.h"
#include "hud/hudmessage.h"
#include "hud/hudshield.h"
#include "hud/hudtargetbox.h"
#include "hud/hudparse.h"
#include "hud/hudsquadmsg.h"
#include "iff_defs/iff_defs.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "io/key.h"
#include "io/mouse.h"
#include "io/timer.h"
#include "external_dll/trackirpublic.h" // header file for the TrackIR routines (Swifty)
#include "jumpnode/jumpnode.h"
#include "lab/lab.h"
#include "lab/wmcgui.h"	//So that GUI_System can be initialized
#include "lighting/lighting.h"
#include "localization/localize.h"
#include "math/staticrand.h"
#include "menuui/barracks.h"
#include "menuui/credits.h"
#include "menuui/mainhallmenu.h"
#include "menuui/optionsmenu.h"
#include "menuui/playermenu.h"
#include "menuui/readyroom.h"
#include "menuui/snazzyui.h"
#include "menuui/techmenu.h"
#include "menuui/trainingmenu.h"
#include "mission/missionbriefcommon.h"
#include "mission/missioncampaign.h"
#include "mission/missiongoals.h"
#include "mission/missionhotkey.h"
#include "mission/missionload.h"
#include "mission/missionlog.h"
#include "mission/missionmessage.h"
#include "mission/missionparse.h"
#include "mission/missiontraining.h"
#include "missionui/fictionviewer.h"
#include "missionui/missionbrief.h"
#include "missionui/missioncmdbrief.h"
#include "missionui/missiondebrief.h"
#include "missionui/missionloopbrief.h"
#include "missionui/missionpause.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionshipchoice.h"
#include "missionui/missionweaponchoice.h"
#include "missionui/redalert.h"
#include "nebula/neb.h"
#include "nebula/neblightning.h"
#include "network/multi.h"
#include "network/multi_dogfight.h"
#include "network/multi_endgame.h"
#include "network/multi_ingame.h"
#include "network/multi_log.h"
#include "network/multi_pause.h"
#include "network/multi_pxo.h"
#include "network/multi_rate.h"
#include "network/multi_respawn.h"
#include "network/multi_voice.h"
#include "network/multimsgs.h"
#include "network/multiteamselect.h"
#include "network/multiui.h"
#include "network/multiutil.h"
#include "network/stand_gui.h"
#include "object/objcollide.h"
#include "object/objectsnd.h"
#include "object/waypoint.h"
#include "observer/observer.h"
#include "osapi/osapi.h"
#include "osapi/osregistry.h"
#include "parse/encrypt.h"
#include "parse/lua.h"
#include "parse/parselo.h"
#include "parse/scripting.h"
#include "parse/sexp.h"
#include "particle/particle.h"
#include "playerman/managepilot.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "popup/popupdead.h"
#include "radar/radar.h"
#include "radar/radarsetup.h"
#include "render/3d.h"
#include "ship/afterburner.h"
#include "ship/awacs.h"
#include "ship/ship.h"
#include "ship/shipcontrails.h"
#include "ship/shipfx.h"
#include "ship/shiphit.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "sound/fsspeech.h"
#include "sound/sound.h"
#include "sound/voicerec.h"
#include "starfield/starfield.h"
#include "starfield/supernova.h"
#include "stats/medals.h"
#include "stats/stats.h"
#include "weapon/beam.h"
#include "weapon/emp.h"
#include "weapon/flak.h"
#include "weapon/muzzleflash.h"
#include "weapon/shockwave.h"
#include "weapon/weapon.h"
#include "fs2netd/fs2netd_client.h"

#include "globalincs/pstypes.h"

extern int Om_tracker_flag; // needed for FS2OpenPXO config



#ifdef NDEBUG
#ifdef FRED
#error macro FRED is defined when trying to build release FreeSpace.  Please undefine FRED macro in build settings
#endif
#endif


//	Revision history.
//	Full version:
//    1.00.04	5/26/98	MWA -- going final (12 pm)
//    1.00.03	5/26/98	MWA -- going final (3 am)
//    1.00.02	5/25/98	MWA -- going final
//    1.00.01	5/25/98	MWA -- going final
//		0.90		5/21/98	MWA -- getting ready for final.
//		0.10		4/9/98.  Set by MK.
//
//	Demo version: (obsolete since DEMO codebase split from tree)
//		0.03		4/10/98	AL.	Interplay rev
//		0.02		4/8/98	MK.	Increased when this system was modified.
//		0.01		4/7/98?	AL.	First release to Interplay QA.
//
//	OEM version:
//		1.00		5/28/98	AL.	First release to Interplay QA.


//  This function is defined in code\network\multiutil.cpp so will be linked from multiutil.obj
//  it's required fro the -missioncrcs command line option - Kazan
void multi_spew_pxo_checksums(int max_files, char *outfile);
void fs2netd_spew_table_checksums(char *outfile);

extern bool frame_rate_display;

bool Env_cubemap_drawn = false;

void game_reset_view_clip();
void game_reset_shade_frame();
void game_post_level_init();
void game_do_frame();
void game_update_missiontime();	// called from game_do_frame() and navmap_do_frame()
void game_reset_time();
void game_show_framerate();			// draws framerate in lower right corner

int Game_no_clear = 0;

typedef struct big_expl_flash {
	float max_flash_intensity;	// max intensity
	float cur_flash_intensity;	// cur intensity
	int	flash_start;		// start time
} big_expl_flash;

#define FRAME_FILTER 16

#define DEFAULT_SKILL_LEVEL	1
int	Game_skill_level = DEFAULT_SKILL_LEVEL;

#define EXE_FNAME			("fs2.exe")

#define LAUNCHER_FNAME	("Launcher.exe")

// JAS: Code for warphole camera.
// Needs to be cleaned up.
float Warpout_time = 0.0f;
int Warpout_forced = 0;		// Set if this is a forced warpout that cannot be cancelled.
int Warpout_sound = -1;
int Use_joy_mouse = 0;
int Use_palette_flash = 1;
#ifndef NDEBUG
int Use_fullscreen_at_startup = 0;
#endif
int Show_area_effect = 0;
object	*Last_view_target = NULL;

int dogfight_blown = 0;

int	frame_int = -1;
float frametimes[FRAME_FILTER];
float frametotal = 0.0f;
float flRealframetime;
float flFrametime;
fix FrametimeOverall = 0;

#ifndef NDEBUG
	int	Show_framerate = 1;
	int	Show_mem = 1;
#else 
	int	Show_framerate = 0;
	int	Show_mem = 0;
#endif

int	Framerate_cap = 120;

// to determine if networking should be disabled, needs to be done first thing
int Networking_disabled = 0;

// for the model page in system
extern void model_page_in_start();

int	Show_cpu = 0;
int	Show_target_debug_info = 0;
int	Show_target_weapons = 0;
int	Game_font = -1;
#ifndef NDEBUG
static int Show_player_pos = 0;		// debug console command to show player world pos on HUD
#endif

int Debug_octant = -1;

fix Game_time_compression = F1_0;
fix Desired_time_compression = Game_time_compression;
fix Time_compression_change_rate = 0;
bool Time_compression_locked = false; //Can the user change time with shift- controls?

// auto-lang stuff
int detect_lang();

// table checksums that will be used for pilot files
uint Weapon_tbl_checksum = 0;
uint Ships_tbl_checksum = 0;

// if the ships.tbl the player has is valid
int Game_ships_tbl_valid = 0;

// if the weapons.tbl the player has is valid
int Game_weapons_tbl_valid = 0;

int Test_begin = 0;
extern int	Player_attacking_enabled;
int Show_net_stats;

int Pre_player_entry;

int	Fred_running = 0;

// required for hudtarget... kinda dumb, but meh
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH+1];
char Fred_callsigns[MAX_SHIPS][NAME_LENGTH+1];

char Game_current_mission_filename[MAX_FILENAME_LEN];
int game_single_step = 0;
int last_single_step=0;

int game_zbuffer = 1;
static int Game_paused;

int Game_level_seed;

#define EXPIRE_BAD_CHECKSUM			1
#define EXPIRE_BAD_TIME					2

extern void ssm_init();
extern void ssm_level_init();
extern void ssm_process();

// static variable to contain the time this version was built
// commented out for now until
// I figure out how to get the username into the file
//LOCAL char freespace_build_time[] = "Compiled on:"__DATE__" "__TIME__" by "__USER__;

// defines and variables used for dumping frame for making trailers.
#ifndef NDEBUG
int Debug_dump_frames = 0;			// Set to 0 to not dump frames, else equal hz to dump. (15 or 30 probably)
int Debug_dump_trigger = 0;
int Debug_dump_frame_count;
int Debug_dump_frame_num = 0;
#define DUMP_BUFFER_NUM_FRAMES	1			// store every 15 frames
#endif

// amount of time to wait after the player has died before we display the death died popup
#define PLAYER_DIED_POPUP_WAIT		2500
int Player_died_popup_wait = -1;

int Multi_ping_timestamp = -1;

int Default_env_map = -1;

// builtin mission list stuff
int Game_builtin_mission_count = 92;
fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
	// single player campaign
	{ "freespace2.fc2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN_FILE),					"" },
			
	// act 1
	{ "sm1-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "sm1-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "loop1-1.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "loop1-2.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "loop1-3.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "training-1.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "training-2.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "training-3.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "tsm-104.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "tsm-105.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
	{ "tsm-106.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},

	// act 2
	{ "sm2-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm2-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},

	// act 3
	{ "sm3-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "sm3-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
	{ "loop2-1.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},		
	{ "loop2-2.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},

	// multiplayer missions

	// gauntlet
	{ "g-shi.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "g-ter.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "g-vas.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},

	// coop
	{ "m-01.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "m-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "m-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "m-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},		

	// dogfight
	{ "mdh-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdh-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdh-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdh-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdh-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdh-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdh-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdh-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdh-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdl-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "mdm-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
	{ "osdog.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},

	// TvT		
	{ "mt-01.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-05.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-06.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-07.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-08.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-09.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
	{ "mt-10.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},				

	// campaign
	{ "templar.fc2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN_FILE),					"" },
	{ "templar-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
	{ "templar-02.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
	{ "templar-03.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
	{ "templar-04.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
};


// Internal function prototypes
void game_maybe_draw_mouse(float frametime);
void init_animating_pointer();
void load_animating_pointer(char *filename, int dx, int dy);
void unload_animating_pointer();
void game_do_training_checks();
void game_shutdown(void);
void game_show_event_debug(float frametime);
void game_event_debug_init();
void game_frame(int paused = false);
void demo_upsell_show_screens();
void game_start_subspace_ambient_sound();
void game_stop_subspace_ambient_sound();
void verify_ships_tbl();
void verify_weapons_tbl();
void game_title_screen_display();
void game_title_screen_close();

// loading background filenames
static char *Game_loading_bground_fname[GR_NUM_RESOLUTIONS] = {
	"LoadingBG",		// GR_640
	"2_LoadingBG"		// GR_1024
};


static char *Game_loading_ani_fname[GR_NUM_RESOLUTIONS] = {
	"Loading",		// GR_640
	"2_Loading"		// GR_1024
};

static char *Game_title_screen_fname[GR_NUM_RESOLUTIONS] = {
	"PreLoad",
	"2_PreLoad"
};

static char *Game_logo_screen_fname[GR_NUM_RESOLUTIONS] = {
	"PreLoadLogo",
	"2_PreLoadLogo"
};

// for title screens
static int Game_title_bitmap = -1;
static int Game_title_logo = -1;

// cdrom stuff
char Game_CDROM_dir[MAX_PATH_LEN];
int init_cdrom();

// How much RAM is on this machine. Set in WinMain
uint FreeSpace_total_ram = 0;

// game flash stuff
float Game_flash_red = 0.0f;
float Game_flash_green = 0.0f;
float Game_flash_blue = 0.0f;
float Sun_spot = 0.0f;
big_expl_flash Big_expl_flash = {0.0f, 0.0f, 0};

// game shudder stuff (in ms)
int Game_shudder_time = -1;
int Game_shudder_total = 0;
float Game_shudder_intensity = 0.0f;			// should be between 0.0 and 100.0

// EAX stuff
sound_env Game_sound_env;
sound_env Game_default_sound_env = { EAX_ENVIRONMENT_BATHROOM, 0.2f, 0.2f, 1.0f };
int Game_sound_env_update_timestamp;


fs_builtin_mission *game_find_builtin_mission(char *filename)
{
	int idx;

	// look through all existing builtin missions
	for(idx=0; idx<Game_builtin_mission_count; idx++){
		if(!stricmp(Game_builtin_mission_list[idx].filename, filename)){
			return &Game_builtin_mission_list[idx];
		}
	}

	// didn't find it
	return NULL;
}

int game_get_default_skill_level()
{
	return DEFAULT_SKILL_LEVEL;
}

// Resets the flash
void game_flash_reset()
{
	Game_flash_red = 0.0f;
	Game_flash_green = 0.0f;
	Game_flash_blue = 0.0f;
	Sun_spot = 0.0f;
	Big_expl_flash.max_flash_intensity = 0.0f;
	Big_expl_flash.cur_flash_intensity = 0.0f;
	Big_expl_flash.flash_start = 0;
}

float Gf_critical = -1.0f;					// framerate we should be above on the average for this mission
float Gf_critical_time = 0.0f;			// how much time we've been at the critical framerate

void game_framerate_check_init()
{
	// zero critical time
	Gf_critical_time = 0.0f;
		
	// nebula missions
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		Gf_critical = 15.0f;			
	} else {
		Gf_critical = 25.0f;
	}
}

extern float Framerate;
void game_framerate_check()
{
	int y_start = 100;
	
	// if the current framerate is above the critical level, add frametime
	if(Framerate >= Gf_critical){
		Gf_critical_time += flFrametime;
	}	

	if (!Show_framerate) {
		return;
	}

	// display if we're above the critical framerate
	if(Framerate < Gf_critical){
		gr_set_color_fast(&Color_bright_red);
		gr_string(200, y_start, "Framerate warning");

		y_start += 10;
	}

	// display our current pct of good frametime
	if(f2fl(Missiontime) >= 0.0f){
		float pct = (Gf_critical_time / f2fl(Missiontime)) * 100.0f;

		if(pct >= 85.0f){
			gr_set_color_fast(&Color_bright_green);
		} else {
			gr_set_color_fast(&Color_bright_red);
		}

		gr_printf(200, y_start, "%d%%", (int)pct);

		y_start += 10;
	}
}


/**
 * Adds a flash effect.  
 *
 * These can be positive or negative. The range will get capped at around -1 to 1, so stick 
 * with a range like that.
 *
 * @param r red colour value
 * @param g green colour value
 * @param b blue colour value
 */
void game_flash( float r, float g, float b )
{
	Game_flash_red += r;
	Game_flash_green += g;
	Game_flash_blue += b;

	if ( Game_flash_red < -1.0f )	{
		Game_flash_red = -1.0f;
	} else if ( Game_flash_red > 1.0f )	{
		Game_flash_red = 1.0f;
	}

	if ( Game_flash_green < -1.0f )	{
		Game_flash_green = -1.0f;
	} else if ( Game_flash_green > 1.0f )	{
		Game_flash_green = 1.0f;
	}

	if ( Game_flash_blue < -1.0f )	{
		Game_flash_blue = -1.0f;
	} else if ( Game_flash_blue > 1.0f )	{
		Game_flash_blue = 1.0f;
	}

}

/**
 * Adds a flash for Big Ship explosions
 * @param flash flash intensity. Range capped from 0 to 1.
 */
void big_explosion_flash(float flash)
{
	CLAMP(flash, 0.0f, 1.0f);

	Big_expl_flash.flash_start = timestamp(1);
	Big_expl_flash.max_flash_intensity = flash;
	Big_expl_flash.cur_flash_intensity = 0.0f;
}

//	Amount to diminish palette towards normal, per second.
#define	DIMINISH_RATE	0.75f
#define	SUN_DIMINISH_RATE	6.00f

int Sun_drew = 0;

float sn_glare_scale = 1.7f;
DCF(sn_glare, "")
{
	dc_get_arg(ARG_FLOAT);
	sn_glare_scale = Dc_arg_float;
}

float Supernova_last_glare = 0.0f;
bool stars_sun_has_glare(int index);
void game_sunspot_process(float frametime)
{
	int n_lights, idx;
	int sn_stage;
	float Sun_spot_goal = 0.0f;

	// supernova
	sn_stage = supernova_active();
	if(sn_stage){		
		// sunspot differently based on supernova stage
		switch(sn_stage){
		// approaching. player still in control
		case 1:			
			float pct;
			pct = (1.0f - (supernova_time_left() / SUPERNOVA_CUT_TIME));

			vec3d light_dir;				
			light_get_global_dir(&light_dir, 0);
			float dot;
			dot = vm_vec_dot( &light_dir, &Eye_matrix.vec.fvec );
			
			if(dot >= 0.0f){
				// scale it some more
				dot = dot * (0.5f + (pct * 0.5f));
				dot += 0.05f;					

				Sun_spot_goal += (dot * sn_glare_scale);
			}

			// draw the sun glow
			if ( !shipfx_eye_in_shadow( &Eye_position, Viewer_obj, 0 ) )	{
				// draw the glow for this sun
				stars_draw_sun_glow(0);	
			}

			Supernova_last_glare = Sun_spot_goal;
			break;

		// camera cut. player not in control. note : at this point camera starts out facing the sun. so we can go nice and bright
		case 2: 					
		case 3:
			Sun_spot_goal = 0.9f;
			Sun_spot_goal += (1.0f - (supernova_time_left() / SUPERNOVA_CUT_TIME)) * 0.1f;

			if(Sun_spot_goal > 1.0f){
				Sun_spot_goal = 1.0f;
			}

			Sun_spot_goal *= sn_glare_scale;
			Supernova_last_glare = Sun_spot_goal;
			break;		

		// fade to white. display dead popup
		case 4:
		case 5:
			Supernova_last_glare += (2.0f * flFrametime);
			if(Supernova_last_glare > 2.0f){
				Supernova_last_glare = 2.0f;
			}

			Sun_spot_goal = Supernova_last_glare;
			break;
		}
	
		Sun_drew = 0;				
	} else {
		Sun_spot_goal = 0.0f;
		if ( Sun_drew )	{
			// check sunspots for all suns
			n_lights = light_get_global_count();

			// check
			for(idx=0; idx<n_lights; idx++){
				if ( !shipfx_eye_in_shadow( &Eye_position, Viewer_obj, idx ) )	{

					vec3d light_dir;				
					light_get_global_dir(&light_dir, idx);

					//only do sunglare stuff if this sun has one
					if (stars_sun_has_glare(idx))
					{
						float dot = vm_vec_dot( &light_dir, &Eye_matrix.vec.fvec )*0.5f+0.5f;

						Sun_spot_goal += (float)pow(dot,85.0f);
					}

					// draw the glow for this sun
					stars_draw_sun_glow(idx);				
				}
			}

			Sun_drew = 0;
		}
	}

	float dec_amount = frametime*SUN_DIMINISH_RATE;

	if ( Sun_spot < Sun_spot_goal )	{
		Sun_spot += dec_amount;
		if ( Sun_spot > Sun_spot_goal )	{
			Sun_spot = Sun_spot_goal;
		}
	} else if ( Sun_spot > Sun_spot_goal )	{
		Sun_spot -= dec_amount;
		if ( Sun_spot < Sun_spot_goal )	{
			Sun_spot = Sun_spot_goal;
		}
	}
}


/**
 * Call once a frame to diminish the flash effect to 0.
 * @param frametime Period over which to dimish at ::DIMINISH_RATE
 */
void game_flash_diminish(float frametime)
{
	float dec_amount = frametime*DIMINISH_RATE;

	if ( Game_flash_red > 0.0f ) {
		Game_flash_red -= dec_amount;		
		if ( Game_flash_red < 0.0f )
			Game_flash_red = 0.0f;
	} else {
		Game_flash_red += dec_amount;		
		if ( Game_flash_red > 0.0f )
			Game_flash_red = 0.0f;
	} 

	if ( Game_flash_green > 0.0f ) {
		Game_flash_green -= dec_amount;		
		if ( Game_flash_green < 0.0f )
			Game_flash_green = 0.0f;
	} else {
		Game_flash_green += dec_amount;		
		if ( Game_flash_green > 0.0f )
			Game_flash_green = 0.0f;
	} 

	if ( Game_flash_blue > 0.0f ) {
		Game_flash_blue -= dec_amount;		
		if ( Game_flash_blue < 0.0f )
			Game_flash_blue = 0.0f;
	} else {
		Game_flash_blue += dec_amount;		
		if ( Game_flash_blue > 0.0f )
			Game_flash_blue = 0.0f;
	} 

	// update big_explosion_cur_flash
#define	TIME_UP		1500
#define	TIME_DOWN	2500
	int duration = TIME_UP + TIME_DOWN;
	int time = timestamp_until(Big_expl_flash.flash_start);
	if (time > -duration) {
		time = -time;
		if (time < TIME_UP) {
			Big_expl_flash.cur_flash_intensity = Big_expl_flash.max_flash_intensity * time / (float) TIME_UP;
		} else {
			time -= TIME_UP;
			Big_expl_flash.cur_flash_intensity = Big_expl_flash.max_flash_intensity * ((float) TIME_DOWN - time) / (float) TIME_DOWN;
		}
	}
	
	if ( Use_palette_flash )	{
		int r,g,b;
		static int o_r=0, o_g=0, o_b=0;

		// Change the 200 to change the color range of colors.
		r = fl2i( Game_flash_red*128.0f );  
		g = fl2i( Game_flash_green*128.0f );   
		b = fl2i( Game_flash_blue*128.0f );  

		if ( Sun_spot > 0.0f )	{
			r += fl2i(Sun_spot*128.0f);
			g += fl2i(Sun_spot*128.0f);
			b += fl2i(Sun_spot*128.0f);
		}

		if ( Big_expl_flash.cur_flash_intensity  > 0.0f ) {
			r += fl2i(Big_expl_flash.cur_flash_intensity*128.0f);
			g += fl2i(Big_expl_flash.cur_flash_intensity*128.0f);
			b += fl2i(Big_expl_flash.cur_flash_intensity*128.0f);
		}

		if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
		if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
		if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;

		if ( (r!=0) || (g!=0) || (b!=0) ) {
			gr_flash( r, g, b );

			o_r = r;
			o_g = g;
			o_b = b;
		}
	}
	
}


void game_level_close()
{
	//WMC - this is actually pretty damn dangerous, but I don't want a modder
	//to accidentally use an override here without realizing it.
	if(!Script_system.IsConditionOverride(CHA_MISSIONEND))
	{
		// save player-persistent variables
		mission_campaign_save_player_persistent_variables();	// Goober5000

		// De-Initialize the game subsystems
		sexp_music_close();	// Goober5000
		event_music_level_close();
		game_stop_looped_sounds();
		snd_stop_all();
		obj_snd_level_close();					// uninit object-linked persistant sounds
		gamesnd_unload_gameplay_sounds();	// unload gameplay sounds from memory
		anim_level_close();						// stop and clean up any anim instances
		message_mission_shutdown();			// called after anim_level_close() to make sure instances are clear
		shockwave_level_close();
		fireball_close();	
		shield_hit_close();
		mission_event_shutdown();
		asteroid_level_close();
		jumpnode_level_close();
		waypoint_level_close();
		flak_level_close();						// unload flak stuff
		neb2_level_close();						// shutdown gaseous nebula stuff
		ct_level_close();
		beam_level_close();
		mflash_level_close();
		mission_brief_common_reset();		// close out parsed briefing/mission stuff
		cam_close();
		subtitles_close();
		trail_level_close();
		ship_clear_cockpit_displays();
		hud_level_close();
		model_instance_free_all();

		// be sure to not only reset the time but the lock as well
		set_time_compression(1.0f, 0.0f);
		lock_time_compression(false);

		audiostream_unpause_all();
		Game_paused = 0;

		if (gr_screen.envmap_render_target >= 0) {
			if ( bm_release(gr_screen.envmap_render_target, 1) ) {
				gr_screen.envmap_render_target = -1;
			}
		}

		gr_set_ambient_light(120, 120, 120);

		ENVMAP = Default_env_map;
	}
	else
	{
		Error(LOCATION, "Scripting Mission End override is not fully supported yet.");
	}

	Script_system.RunCondition(CHA_MISSIONEND);
}

uint load_gl_init;
uint load_mission_load;
uint load_post_level_init;

/**
 * Intializes game stuff.  
 *
 * @param seed Only set by demo playback code.
 * @return 0 on failure, 1 on success
 */
void game_level_init(int seed)
{
	game_busy( NOX("** starting game_level_init() **") );
	load_gl_init = (uint) time(NULL);
	// seed the random number generator
	if ( seed == -1 ) {
		// if no seed was passed, seed the generator either from the time value, or from the
		// netgame security flags -- ensures that all players in multiplayer game will have the
		// same randon number sequence (with static rand functions)
		if ( Game_mode & GM_NORMAL ) {
			Game_level_seed = (int) time(NULL);
		} else {
			Game_level_seed = Netgame.security;
		}
	} else {
		Assert( !(Game_mode & GM_MULTIPLAYER) );
		Game_level_seed = seed;
	}
	srand( Game_level_seed );

	// semirand function needs to get re-initted every time in multiplayer
	if ( Game_mode & GM_MULTIPLAYER ){
		init_semirand();
	}

	Framecount = 0;
	game_reset_view_clip();
	game_reset_shade_frame();

	Key_normal_game = (Game_mode & GM_NORMAL);
	Cheats_enabled = 0;

	Game_shudder_time = -1;

	Perspective_locked = false;

	// reset the geometry map and distortion map batcher, this should to be done pretty soon in this mission load process (though it's not required)
	batch_reset();

	// Initialize the game subsystems
	if(!Is_standalone){
		game_reset_time();			// resets time, and resets saved time too
	}

	Multi_ping_timestamp = -1;

	obj_init();						// Must be inited before the other systems

	if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
		model_page_in_start();		// mark any existing models as unused but don't unload them yet
		mprintf(( "Beginning level bitmap paging...\n" ));
		bm_page_in_start();
	} else {
		model_free_all();			// Free all existing models if standalone server
	}

	mission_brief_common_init();		// Free all existing briefing/debriefing text
	weapon_level_init();

	NavSystem_Init();				// zero out the nav system

	ai_level_init();				//	Call this before ship_init() because it reads ai.tbl.
	ship_level_init();
	player_level_init();
	shipfx_flash_init();			// Init the ship gun flash system.
	game_flash_reset();			// Reset the flash effect
	particle_init();				// Reset the particle system
	fireball_init();
	debris_init();
	shield_hit_init();				//	Initialize system for showing shield hits

	mission_init_goals();
	mission_log_init();
	messages_init();
	obj_snd_level_init();					// init object-linked persistant sounds
	anim_level_init();
	shockwave_level_init();
	afterburner_level_init();
	scoring_level_init( &Player->stats );
	key_level_init();
	asteroid_level_init();
	control_config_clear_used_status();
	collide_ship_ship_sounds_init();
	Missiontime = 0;
	Pre_player_entry = 1;			//	Means the player has not yet entered.
	Entry_delay_time = 0;			//	Could get overwritten in mission read.
	observer_init();
	flak_level_init();				// initialize flak - bitmaps, etc
	ct_level_init();				// initialize ships contrails, etc
	awacs_level_init();				// initialize AWACS
	beam_level_init();				// initialize beam weapons
	mflash_level_init();
	ssm_level_init();	
	supernova_level_init();
	cam_init();
	snd_aav_init();

	// multiplayer dogfight hack
	dogfight_blown = 0;

	shipfx_engine_wash_level_init();

	stars_pre_level_init();
	neb2_level_init();
	nebl_level_init();

	Last_view_target = NULL;
	Game_paused = 0;

	Game_no_clear = 0;

	// campaign wasn't ended
	Campaign_ended_in_mission = 0;

	Env_cubemap_drawn = false;

	load_gl_init = (uint) (time(NULL) - load_gl_init);

	//WMC - Init multi players for level
	if (Game_mode & GM_MULTIPLAYER && Player != NULL) {
		Player->flags |= PLAYER_FLAGS_IS_MULTI;

		// clear multiplayer stats
		init_multiplayer_stats();
	}
}

/**
 * Called when a mission is over -- does server specific stuff.
 */
void freespace_stop_mission()
{	
	game_level_close();
	Game_mode &= ~GM_IN_MISSION;
}

/**
 * Called at frame interval to process networking stuff
 */
void game_do_networking()
{
	Assert( Net_player != NULL );
	if (!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// see if this player should be reading/writing data.  Bit is set when at join
	// screen onward until quits back to main menu.
	if ( !(Net_player->flags & NETINFO_FLAG_DO_NETWORKING) ){
		return;
	}

	if(gameseq_get_state()!=GS_STATE_MULTI_PAUSED){
		multi_do_frame();
	} else {
		multi_pause_do_frame();
	}	
}

// An estimate as to how high the count passed to game_loading_callback will go.
// This is just a guess, it seems to always be about the same.   The count is
// proportional to the code being executed, not the time, so this works good
// for a bar, assuming the code does about the same thing each time you
// load a level.   You can find this value by looking at the return value
// of game_busy_callback(NULL), which I conveniently print out to the
// debug output window with the '=== ENDING LOAD ==' stuff.   
#define COUNT_ESTIMATE 425

int Game_loading_callback_inited = 0;
int Game_loading_background = -1;
generic_anim Game_loading_ani;

static int Game_loading_ani_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		63, 316  // GR_640
	},
	{
		101, 505	// GR_1024
	}
};

#ifndef NDEBUG
extern char Processing_filename[MAX_PATH_LEN];
static int busy_shader_created = 0;
shader busy_shader;
#endif
static int framenum;

/**
 * This gets called 10x per second and count is the number of times ::game_busy() has been 
 * called since the current callback function was set.
 */
void game_loading_callback(int count)
{	
	int new_framenum;
	game_do_networking();

	Assert( Game_loading_callback_inited==1 );
	Assertion( Game_loading_ani.num_frames > 0, "Load Screen animation %s not found, or corrupted. Needs to be an animation with at least 1 frame.", Game_loading_ani.filename );

	int do_flip = 0;

	new_framenum = ((Game_loading_ani.num_frames*count) / COUNT_ESTIMATE)+1;
	if ( new_framenum > Game_loading_ani.num_frames-1 )	{
		new_framenum = Game_loading_ani.num_frames-1;
	} else if ( new_framenum < 0 )	{
		new_framenum = 0;
	}
	//make sure we always run forwards - graphical hack
	if(new_framenum > framenum)
		framenum = new_framenum;

	if ( Game_loading_ani.num_frames > 0 )	{
		if ( Game_loading_background > -1 )	{
			gr_set_bitmap( Game_loading_background );
			gr_bitmap(0,0);
		}

		gr_set_bitmap( Game_loading_ani.first_frame + framenum );
		gr_bitmap(Game_loading_ani_coords[gr_screen.res][0],Game_loading_ani_coords[gr_screen.res][1]);

		do_flip = 1;
	}

#ifndef NDEBUG
	// print the current filename being processed by game_busy(), the shader here is a quick hack
	// since the background isn't always drawn so we can't clear the text away from the previous
	// filename. the shader is completely opaque to hide the old text. must easier and faster than
	// redrawing the entire screen every flip - taylor
	if (!busy_shader_created) {
		gr_create_shader(&busy_shader, 5, 5, 5, 255);
		busy_shader_created = 1;
	}

	if (Processing_filename[0] != '\0') {
		gr_set_shader(&busy_shader);
		gr_shade(0, 0, gr_screen.clip_width_unscaled, 17); // make sure it goes across the entire width

		gr_set_color_fast(&Color_white);
		gr_string(5, 5, Processing_filename);

		do_flip = 1;
		memset( Processing_filename, 0, MAX_PATH_LEN );
	}
#endif

#ifndef NDEBUG
	if(Cmdline_show_mem_usage)
	{
#ifdef _WIN32
		void memblockinfo_sort();
		void memblockinfo_sort_get_entry(int index, char *filename, int *size);

		char mem_buffer[1000];
		char filename[35];
		int size;
		int i;
	  	memblockinfo_sort();
		for(i = 0; i < 30; i++)
		{
			memblockinfo_sort_get_entry(i, filename, &size);

			size /= 1024;

			if(size == 0)
				break;

			char *short_name = strrchr(filename, '\\');
			if(short_name == NULL)
				short_name = filename;
			else
				short_name++;

			sprintf(mem_buffer,"%s:\t%d K", short_name, size);
			gr_string( 20, 220 + (i*10), mem_buffer);
		}
		sprintf(mem_buffer,"Total RAM:\t%d K", TotalRam / 1024);
		gr_string( 20, 230 + (i*10), mem_buffer);
#endif	// _WIN32
	}
#endif	// !NDEBUG

	if (do_flip)
		gr_flip();
}

void game_loading_callback_init()
{
	Assert( Game_loading_callback_inited==0 );

	Game_loading_background = bm_load(The_mission.loading_screen[gr_screen.res]);
	
	if (Game_loading_background < 0)
		Game_loading_background = bm_load(Game_loading_bground_fname[gr_screen.res]);

	strcpy_s(Game_loading_ani.filename, Game_loading_ani_fname[gr_screen.res]);
	generic_anim_init(&Game_loading_ani, Game_loading_ani.filename);
	generic_anim_load(&Game_loading_ani);
	Assertion( Game_loading_ani.num_frames > 0, "Load Screen animation %s not found, or corrupted. Needs to be an animation with at least 1 frame.", Game_loading_ani.filename );

	Game_loading_callback_inited = 1;
	Mouse_hidden = 1;
	framenum = 0;
	game_busy_callback( game_loading_callback, (COUNT_ESTIMATE/Game_loading_ani.num_frames)+1 );


}

void game_loading_callback_close()
{
	Assert( Game_loading_callback_inited==1 );

	// Make sure bar shows all the way over.
	game_loading_callback(COUNT_ESTIMATE);
	
	int real_count = game_busy_callback( NULL );
 	Mouse_hidden = 0;

	Game_loading_callback_inited = 0;
	
#ifndef NDEBUG
	mprintf(( "=================== ENDING LOAD ================\n" ));
	mprintf(( "Real count = %d,  Estimated count = %d\n", real_count, COUNT_ESTIMATE ));
	mprintf(( "================================================\n" ));
#else
	// to remove warnings in release build
	real_count = 0;
#endif

	generic_anim_unload(&Game_loading_ani);

	bm_release( Game_loading_background );
	common_free_interface_palette();		// restore game palette
	Game_loading_background = -1;

	gr_set_font( FONT1 );
}

/**
 * Update the sound environment (ie change EAX settings based on proximity to large ships)
 */
void game_maybe_update_sound_environment()
{
	// do nothing for now
}

/**
 * Assign the sound environment for the game, based on the current mission
 */
void game_assign_sound_environment()
{
	if (The_mission.sound_environment.id >= 0) {
		Game_sound_env = The_mission.sound_environment;
	} else if (SND_ENV_DEFAULT > 0) {
		sound_env_get(&Game_sound_env, SND_ENV_DEFAULT);
	} else {
		Game_sound_env = Game_default_sound_env;
	}

	Game_sound_env_update_timestamp = timestamp(1);
}

/**
 * Function which gets called before actually entering the mission.
 */
void freespace_mission_load_stuff()
{
	// called if we're not on a freespace dedicated (non rendering, no pilot) server
	// IE : we _don't_ want to load any sounds or bitmap/texture info on this machine.
	if(!(Game_mode & GM_STANDALONE_SERVER)){	
	
		mprintf(( "=================== STARTING LEVEL DATA LOAD ==================\n" ));

		game_busy( NOX("** setting up event music **") );
		event_music_level_init(-1);	// preloads the first 2 seconds for each event music track

		game_busy( NOX("** unloading interface sounds **") );
		gamesnd_unload_interface_sounds();		// unload interface sounds from memory

		game_busy( NOX("** preloading common game sounds **") );
		gamesnd_preload_common_sounds();			// load in sounds that are expected to play

		if (Cmdline_snd_preload) {
			game_busy( NOX("** preloading gameplay sounds **") );
			gamesnd_load_gameplay_sounds();			// preload in gameplay sounds if wanted
		}

		game_busy( NOX("** assigning sound environment for mission **") );
		ship_assign_sound_all();	// assign engine sounds to ships
		game_assign_sound_environment();	 // assign the sound environment for this mission

		obj_merge_created_list();

		if (!(Game_mode & GM_MULTIPLAYER)) {
			// call function in missionparse.cpp to fixup player/ai stuff.
			game_busy( NOX("** fixing up player/ai stuff **") );
			mission_parse_fixup_players();
		}

		// Load in all the bitmaps for this level
		level_page_in();

		game_busy( NOX("** finished with level_page_in() **") );

		if(Game_loading_callback_inited) {
			game_loading_callback_close();
		}
	} 
	// the only thing we need to call on the standalone for now.
	else {
		obj_merge_created_list();

		// Load in all the bitmaps for this level
		level_page_in();
	}
}

/**
 * Called after mission is loaded.
 *
 * Because player isn't created until after mission loads, some things must get initted after the level loads
 */
void game_post_level_init()
{
	extern void game_environment_map_gen();
	game_environment_map_gen();

 	HUD_init();
	hud_setup_escort_list();
	mission_hotkey_set_defaults();	// set up the default hotkeys (from mission file)

	stars_post_level_init();	

	// While trying to track down the nebula bug I encountered a cool effect -
	// comment this out to fly a mission in a void. Maybe we should develop this
	// into a full effect or something, because it is seriously cool.
	neb2_post_level_init();		

#ifndef NDEBUG
	game_event_debug_init();
#endif

	training_mission_init();
	asteroid_create_all();

	// set ambient light for level
	gr_set_ambient_light(The_mission.ambient_light_level & 0xff, 
							(The_mission.ambient_light_level >> 8) & 0xff,
							(The_mission.ambient_light_level >> 16) & 0xff);

	game_framerate_check_init();

	// If this is a red alert mission in campaign mode, bash wingman status
	if ( (Game_mode & GM_CAMPAIGN_MODE) && red_alert_mission() ) {
		red_alert_bash_wingman_status();
	}

	freespace_mission_load_stuff();

	// m!m Make hv.Player available in "On Mission Start" hook
	if(Player_obj)
		Script_system.SetHookObject("Player", Player_obj);

	// HACK: That scripting hook should be in mission so GM_IN_MISSION has to be set
	Game_mode |= GM_IN_MISSION;
	Script_system.RunCondition(CHA_MISSIONSTART);
	Game_mode &= ~GM_IN_MISSION;

	if (Player_obj)
		Script_system.RemHookVar("Player");
}

/**
 * Tells the server to load the mission and initialize structures
 */
int game_start_mission()
{
	mprintf(( "=================== STARTING LEVEL LOAD ==================\n" ));

	// clear post processing settings
	gr_post_process_set_defaults();

	get_mission_info(Game_current_mission_filename, &The_mission, false);

	if ( !(Game_mode & GM_STANDALONE_SERVER) )
		game_loading_callback_init();

	game_level_init();
	
	if (Game_mode & GM_MULTIPLAYER) {
		Player->flags |= PLAYER_FLAGS_IS_MULTI;

		// clear multiplayer stats
		init_multiplayer_stats();
	}

	game_busy( NOX("** starting mission_load() **") );
	load_mission_load = (uint) time(NULL);
	if (mission_load(Game_current_mission_filename)) {
		if ( !(Game_mode & GM_MULTIPLAYER) ) {
			popup(PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, XSTR( "Attempt to load the mission failed", 169));
			gameseq_post_event(GS_EVENT_MAIN_MENU);
		} else {
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_LOAD_FAIL);
		}

		if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
			game_loading_callback_close();
		}

		game_level_close();

		return 0;
	}
	load_mission_load = (uint) (time(NULL) - load_mission_load);

	// free up memory from parsing the mission
	extern void stop_parse();
	stop_parse();

	game_busy( NOX("** starting game_post_level_init() **") );
	load_post_level_init = (uint) time(NULL);
	game_post_level_init();
	load_post_level_init = (uint) (time(NULL) - load_post_level_init);

#ifndef NDEBUG
	{
		void Do_model_timings_test();
		Do_model_timings_test();	
	}
#endif

	bm_print_bitmaps();
	return 1;
}

int Interface_framerate = 0;
#ifndef NDEBUG

DCF_BOOL( mouse_control, Use_mouse_to_fly )
DCF_BOOL( show_framerate, Show_framerate )
DCF_BOOL( show_target_debug_info, Show_target_debug_info )
DCF_BOOL( show_target_weapons, Show_target_weapons )
DCF_BOOL( lead_target_cheat, Players[Player_num].lead_target_cheat )
DCF_BOOL( sound, Sound_enabled )
DCF_BOOL( zbuffer, game_zbuffer )
DCF_BOOL( show_shield_mesh, Show_shield_mesh)
DCF_BOOL( player_attacking, Player_attacking_enabled )
DCF_BOOL( show_waypoints, Show_waypoints )
DCF_BOOL( show_area_effect, Show_area_effect )
DCF_BOOL( show_net_stats, Show_net_stats )
DCF_BOOL( log, Log_debug_output_to_file )
extern int Training_message_method;
DCF_BOOL( training_msg_method, Training_message_method )
DCF_BOOL( show_player_pos, Show_player_pos )
DCF_BOOL(i_framerate, Interface_framerate )

DCF(warp, "Tests warpin effect")
{
	if ( Dc_command )	{
		bool warpin = true;
		int idx = -1;

		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);
		if( Dc_arg_type & ARG_TRUE) warpin = true;
		else if(Dc_arg_type & ARG_FALSE) warpin = false;

		if(!(Dc_arg_type & ARG_NONE))
		{
			dc_get_arg(ARG_STRING|ARG_NONE);
			if(Dc_arg_type & ARG_STRING)
			{
				idx = ship_name_lookup(Dc_arg);
				if(idx > -1)
				{
					if(warpin)
						shipfx_warpin_start(&Objects[Ships[idx].objnum]);
					else
						shipfx_warpout_start(&Objects[Ships[idx].objnum]);
				}
			}
		}
		
		if(idx < 0)
		{
			if(Player_ai->target_objnum > -1)
			{
				if(warpin)
					shipfx_warpin_start(&Objects[Player_ai->target_objnum]);
				else
					shipfx_warpout_start(&Objects[Player_ai->target_objnum]);
			}
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: Show_mem\nWarps in if true, out if false, player target unless specific ship is specified\n" );	
}

DCF(show_mem,"Toggles showing mem usage")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Show_mem = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Show_mem = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Show_mem ^= 1;	

		if ( Show_mem )	{
			Show_cpu = 0;
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: Show_mem\nSets show_mem to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	{
		dc_printf( "Show_mem is %s\n", (Show_mem?"TRUE":"FALSE") );	
		dc_printf( "Show_cpu is %s\n", (Show_cpu?"TRUE":"FALSE") );	
	}
}

DCF(show_cpu,"Toggles showing cpu usage")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Show_cpu = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Show_cpu = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Show_cpu ^= 1;	

		if ( Show_cpu )	{
			Show_mem = 0;
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: Show_cpu\nSets show_cpu to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	{
		dc_printf( "Show_mem is %s\n", (Show_mem?"TRUE":"FALSE") );	
		dc_printf( "Show_cpu is %s\n", (Show_cpu?"TRUE":"FALSE") );	

	}
}

#endif

			int Game_init_seed;

DCF(use_joy_mouse,"Makes joystick move mouse cursor")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_joy_mouse = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_joy_mouse = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_joy_mouse ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: use_joy_mouse [bool]\nSets use_joy_mouse to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "use_joy_mouse is %s\n", (Use_joy_mouse?"TRUE":"FALSE") );	

	os_config_write_uint( NULL, NOX("JoystickMovesCursor"), Use_joy_mouse );
}

DCF(palette_flash,"Toggles palette flash effect on/off")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_palette_flash = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_palette_flash = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_palette_flash ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: palette_flash [bool]\nSets palette_flash to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "palette_flash is %s\n", (Use_palette_flash?"TRUE":"FALSE") );	
}

int Use_low_mem = 0;

DCF(low_mem,"Uses low memory settings regardless of RAM")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_low_mem = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_low_mem = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_low_mem ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: low_mem [bool]\nSets low_mem to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "low_mem is %s\n", (Use_low_mem?"TRUE":"FALSE") );	

	os_config_write_uint( NULL, NOX("LowMem"), Use_low_mem );
}


#ifndef NDEBUG

DCF(force_fullscreen, "Forces game to startup in fullscreen mode")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_fullscreen_at_startup = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_fullscreen_at_startup = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_fullscreen_at_startup ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: force_fullscreen [bool]\nSets force_fullscreen to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "force_fullscreen is %s\n", (Use_fullscreen_at_startup?"TRUE":"FALSE") );	
	os_config_write_uint( NULL, NOX("ForceFullscreen"), Use_fullscreen_at_startup );
}
#endif

int	Framerate_delay = 0;

float FreeSpace_gamma = 1.0f;

DCF(gamma,"Sets Gamma factor")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);
		if ( Dc_arg_type & ARG_FLOAT )	{
			FreeSpace_gamma = Dc_arg_float;
		} else {
			dc_printf( "Gamma reset to 1.0f\n" );
			FreeSpace_gamma = 1.0f;
		}
		if ( FreeSpace_gamma < 0.1f )	{
			FreeSpace_gamma = 0.1f;
		} else if ( FreeSpace_gamma > 5.0f )	{
			FreeSpace_gamma = 5.0f;
		}
		gr_set_gamma(FreeSpace_gamma);

		char tmp_gamma_string[32];
		sprintf( tmp_gamma_string, NOX("%.2f"), FreeSpace_gamma );
		os_config_write_string( NULL, NOX("Gamma"), tmp_gamma_string );
	}

	if ( Dc_help )	{
		dc_printf( "Usage: gamma <float>\n" );
		dc_printf( "Sets gamma in range 1-3, no argument resets to default 1.2\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Gamma = %.2f\n", FreeSpace_gamma );
	}
}

void run_launcher()
{
#ifdef _WIN32
	const char *launcher_link = "explorer.exe \"http://www.randomtiger.pwp.blueyonder.co.uk/freespace/Launcher5.rar\"";

	int download = MessageBox((HWND)os_get_window(), 
		"Run the fs2_open launcher to fix your problem. "
		"Would you like to download the latest version of the launcher? "
		"You must have at least version 5.0 to run fs2_open versions above 3.6.", 
		"Question", MB_YESNO | MB_ICONQUESTION);

	if(download == IDYES)
	{
		// Someone should change this to the offical link
		WinExec(launcher_link, SW_SHOW);
		return;
	}

	// This now crashes the launcher since fs2_open is still open
	return;
#endif
}

#ifdef APPLE_APP
char full_path[1024];
#endif

/**
 * Game initialisation
 */
void game_init()
{
	int s1, e1;
	char *ptr;
	char whee[MAX_PATH_LEN];

	Game_current_mission_filename[0] = 0;

	// seed the random number generator
	Game_init_seed = (int) time(NULL);
	srand( Game_init_seed );

	Framerate_delay = 0;

#ifndef NDEBUG
	load_filter_info();
#endif

	// encrypt stuff
	encrypt_init();

	// Initialize the timer before the os
	timer_init();

	// init os stuff next
	if ( !Is_standalone ) {		
		os_init( Osreg_class_name, Osreg_app_name );
	}

#ifndef NDEBUG
	#if FS_VERSION_REVIS == 0
		mprintf(("FreeSpace 2 Open version: %i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD));
	#else
		mprintf(("FreeSpace 2 Open version: %i.%i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS));
	#endif

	extern void cmdline_debug_print_cmdline();
	cmdline_debug_print_cmdline();
#endif

#ifdef APPLE_APP
	// some OSX hackery to drop us out of the APP the binary is run from
	char *c = NULL;
	c = strstr(full_path, ".app");
	if ( c != NULL ) {
		while (c && (*c != '/'))
			c--;

		*c = '\0';
	}
	strncpy(whee, full_path, MAX_PATH_LEN-1);
#else
	GetCurrentDirectory(MAX_PATH_LEN-1, whee);
#endif
	strcat_s(whee, DIR_SEPARATOR_STR);
	strcat_s(whee, EXE_FNAME);

	//Initialize the libraries
	s1 = timer_get_milliseconds();

	if ( cfile_init(whee, strlen(Game_CDROM_dir) ? Game_CDROM_dir : NULL) ) {			// initialize before calling any cfopen stuff!!!
		exit(1);
	}

	e1 = timer_get_milliseconds();

	// initialize localization module. Make sure this is done AFTER initialzing OS.
	lcl_init( detect_lang() );	
	lcl_xstr_init();

	if (Is_standalone) {
		// force off some cmdlines if they are on
		Cmdline_spec = 0;
		Cmdline_glow = 0;
		Cmdline_env = 0;
		Cmdline_3dwarp = 0;
		Cmdline_normal = 0;

		// now init the standalone server code
		std_init_standalone();
	}

	// verify that he has a valid ships.tbl (will Game_ships_tbl_valid if so)
	verify_ships_tbl();

	// verify that he has a valid weapons.tbl
	verify_weapons_tbl();


	Use_joy_mouse = 0;
	Use_low_mem = os_config_read_uint( NULL, NOX("LowMem"), 0 );

#ifndef NDEBUG
	Use_fullscreen_at_startup = os_config_read_uint( NULL, NOX("ForceFullscreen"), 1 );
#endif

	// change FPS cap if told to do so (for those who can't use vsync or where vsync isn't enough)
	uint max_fps = 0;
	if ( (max_fps = os_config_read_uint(NULL, NOX("MaxFPS"), 0)) != 0 ) {
		if ( (max_fps > 15) && (max_fps < 120) ) {
			Framerate_cap = (int)max_fps;
		}
	}

	Asteroids_enabled = 1;		

/////////////////////////////
// SOUND INIT START
/////////////////////////////

	if ( !Is_standalone ) {
		snd_init();
	}

	if(fsspeech_init() == false) {
		mprintf(("Failed to init speech\n"));

		if(Cmdline_query_speech)
		{
			if(!fsspeech_was_compiled()) 
				MessageBox((HWND)os_get_window(), "Speech is not compiled in this build in code.lib", "FS2_Open Warning", MB_ICONWARNING);
			else
				MessageBox((HWND)os_get_window(), "Speech is compiled, but failed to init", "FS2_Open Warning", MB_ICONWARNING);	
		}
	} else if(Cmdline_query_speech) {
		// Its bad practice to use a negative type, this is an exceptional case
		fsspeech_play(-1,"Welcome to FS2 open");
		MessageBox((HWND)os_get_window(), "Speech is compiled and initialised and should be working", "FS2_Open Info", MB_OK);
	}

/////////////////////////////
// SOUND INIT END
/////////////////////////////

	if ( gr_init() == false ) {
#ifdef _WIN32
		ClipCursor(NULL);
		ShowCursor(TRUE);
		ShowWindow((HWND)os_get_window(),SW_MINIMIZE);
		MessageBox( NULL, "Error intializing graphics!", "Error", MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
		run_launcher();
#elif defined(SCP_UNIX)
		fprintf(stderr, "Error initializing graphics!");

		// the default entry should have been created already if it didn't exist, so if we're here then
		// the current value is invalid and we need to replace it
		os_config_write_string(NULL, NOX("VideocardFs2open"), NOX("OGL -(1024x768)x16 bit"));

		// courtesy
		fprintf(stderr, "The default video entry is now in place.  Please try running the game again...\n");
		fprintf(stderr, "(edit ~/.fs2_open/fs2_open.ini to change from default resolution)\n");
#endif
		exit(1);
		return;
	}

// Karajorma - Moved here from the sound init code cause otherwise windows complains
#ifdef FS2_VOICER
	if(Cmdline_voice_recognition)
	{
		bool voiceRectOn = VOICEREC_init((HWND)os_get_window(), WM_RECOEVENT, GRAMMARID1, IDR_CMD_CFG);
	
		if(voiceRectOn == false)
		{
			MessageBox((HWND)os_get_window(), "Failed to init voice rec", "Error", MB_OK);
		}
	}

#endif

	// D3D's gamma system now works differently. 1.0 is the default value
	ptr = os_config_read_string(NULL, NOX("GammaD3D"), NOX("1.0"));
	FreeSpace_gamma = (float)atof(ptr);

	script_init();			//WMC

	gr_font_init();					// loads up all fonts
	
	// add title screen
	if(!Is_standalone){
		// #Kazan# - moved this down - WATCH THESE calls - anything that shares code between standalone and normal
		// cannot make gr_* calls in standalone mode because all gr_ calls are NULL pointers
		gr_set_gamma(FreeSpace_gamma);
		game_title_screen_display();
	}
	
	// attempt to load up master tracker registry info (login and password)
	Multi_tracker_id = -1;		

	// should we be using this or not?
	Om_tracker_flag = os_config_read_uint( "PXO", "FS2OpenPXO" , 0 );
	// pxo login and password
	ptr = os_config_read_string(NOX("PXO"),NOX("Login"),NULL);
	if(ptr == NULL){
		nprintf(("Network","Error reading in PXO login data\n"));
		strcpy_s(Multi_tracker_login,"");
	} else {		
		strcpy_s(Multi_tracker_login,ptr);
	}
	ptr = os_config_read_string(NOX("PXO"),NOX("Password"),NULL);
	if(ptr == NULL){		
		nprintf(("Network","Error reading PXO password\n"));
		strcpy_s(Multi_tracker_passwd,"");
	} else {		
		strcpy_s(Multi_tracker_passwd,ptr);
	}	

	// pxo squad name and password
	ptr = os_config_read_string(NOX("PXO"),NOX("SquadName"),NULL);
	if(ptr == NULL){
		nprintf(("Network","Error reading in PXO squad name\n"));
		strcpy_s(Multi_tracker_squad_name, "");
	} else {		
		strcpy_s(Multi_tracker_squad_name, ptr);
	}

	// If less than 48MB of RAM, use low memory model.
	if (
#ifdef _WIN32
		(FreeSpace_total_ram < 48*1024*1024) ||
#endif
		Use_low_mem )	{
		mprintf(( "Using normal memory settings...\n" ));
		bm_set_low_mem(1);		// Use every other frame of bitmaps
	} else {
		mprintf(( "Using high memory settings...\n" ));
		bm_set_low_mem(0);		// Use all frames of bitmaps
	}

	//WMC - Initialize my new GUI system
	//This may seem scary, but it should take up 0 processing time and very little memory
	//as long as it's not being used.
	//Otherwise, it just keeps the parsed interface.tbl in memory.
	GUI_system.ParseClassInfo("interface.tbl");

	// load non-darkening pixel defs
	palman_load_pixels();

	iff_init();						// Goober5000 - this must be done even before species_defs :p
	species_init();					// Load up the species defs - this needs to be done FIRST -- Kazan

	brief_parse_icon_tbl();

	hud_init_comm_orders();	// Goober5000

	control_config_common_init();				// sets up localization stuff in the control config

	parse_rank_tbl();
	parse_medal_tbl();

	cutscene_init();
	key_init();
	mouse_init();
	gamesnd_parse_soundstbl();

	gameseq_init();

	multi_init();	

	// standalone's don't use the joystick and it seems to sometimes cause them to not get shutdown properly
	if(!Is_standalone){
		joy_init();
	}

	player_controls_init();
	model_init();	

	event_music_init();

	obj_init();	
	mflash_game_init();	
	armor_init();
	ai_init();
	ai_profiles_init();		// Goober5000
	weapon_init();	
	ship_init();						// read in ships.tbl	

	player_init();	
	mission_campaign_init();		// load in the default campaign	
	anim_init();
	context_help_init();			
	techroom_intel_init();			// parse species.tbl, load intel info  
	hud_positions_init();		//Setup hud positions
	
	// initialize psnet
	psnet_init( Multi_options_g.protocol, Multi_options_g.port );						// initialize the networking code		

	init_animating_pointer();	
	asteroid_init();
	mission_brief_common_init();	// Mark all the briefing structures as empty.

	neb2_init();						// fullneb stuff
	nebl_init();
	stars_init();
	ssm_init();	
	player_tips_init();				// helpful tips
	beam_init();
	
	// load the list of pilot pic filenames (for barracks and pilot select popup quick reference)
	pilot_load_pic_list();	
	pilot_load_squad_pic_list();

	load_animating_pointer(NOX("cursor"), 0, 0);	

	// initialize alpha colors
	// CommanderDJ: try with colors.tbl first, then use the old way if that doesn't work
	if (!new_alpha_colors_init()) {
		old_alpha_colors_init();
	}

	if (Cmdline_env) {
		ENVMAP = Default_env_map = bm_load("cubemap");
	}

	Viewer_mode = 0;
	Game_paused = 0;

	Script_system.RunBytecode(Script_gameinithook);
	Script_system.RunCondition(CHA_GAMEINIT);

	game_title_screen_close();

#ifdef _WIN32
	timeBeginPeriod(1);	
#endif

	nprintf(("General", "Ships.tbl is : %s\n", Game_ships_tbl_valid ? "VALID" : "INVALID!!!!"));
	nprintf(("General", "Weapons.tbl is : %s\n", Game_weapons_tbl_valid ? "VALID" : "INVALID!!!!"));

	mprintf(("cfile_init() took %d\n", e1 - s1));	
	Script_system.RunBytecode(Script_gameinithook);
}

char transfer_text[128];

float	Start_time = 0.0f;

float Framerate = 0.0f;

#ifndef NDEBUG
float Timing_total = 0.0f;
float Timing_render2 = 0.0f;
float Timing_render3 = 0.0f;
float Timing_flip = 0.0f;
float Timing_clear = 0.0f;
#endif

MONITOR(NumPolysDrawn)
MONITOR(NumPolys)
MONITOR(NumVerts)
MONITOR(BmpUsed)
MONITOR(BmpNew)


uint Mem_starttime_phys;
uint Mem_starttime_pagefile;
uint Mem_starttime_virtual;

void game_get_framerate()
{	
	if (frame_int == -1) {
		for (int i = 0; i < FRAME_FILTER; i++)
			frametimes[i] = 0.0f;

		frametotal = 0.0f;
		frame_int = 0;
	}

	frametotal -= frametimes[frame_int];
	frametotal += flRealframetime;
	frametimes[frame_int] = flRealframetime;
	frame_int = (frame_int + 1 ) % FRAME_FILTER;

	if (frametotal != 0.0f) {
		if (Framecount >= FRAME_FILTER)
			Framerate = FRAME_FILTER / frametotal;
		else
			Framerate = Framecount / frametotal;
	}

	Framecount++;
}

/**
 * Show FPS within game
 */
void game_show_framerate()
{	
	float	cur_time;

	cur_time = f2fl(timer_get_approx_seconds());
	if (cur_time - Start_time > 30.0f) {
		mprintf(("%i frames executed in %7.3f seconds, %7.3f frames per second.\n", Framecount, cur_time - Start_time, Framecount/(cur_time - Start_time)));
		Start_time += 1000.0f;
	}

#ifdef WMC
	//WMC - this code spits out the target of all turrets
	if ( (Player_ai->target_objnum != -1) && (Objects[Player_ai->target_objnum].type == OBJ_SHIP) ) {
		//Debug crap
		int t = 0;
		ship_subsys	*pss;

		gr_set_color_fast(&HUD_color_debug);

		object *objp = &Objects[Player_ai->target_objnum];
		for ( pss = GET_FIRST(&shipp->subsys_list); pss !=END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
			if (pss->system_info->type == SUBSYSTEM_TURRET) {
				if(pss->turret_enemy_objnum == -1)
					gr_printf(10, t*10, "Turret %d: <None>", t);
				else if (Objects[pss->turret_enemy_objnum].type == OBJ_SHIP)
					gr_printf(10, t*10, "Turret %d: %s", t, Ships[Objects[pss->turret_enemy_objnum].instance].ship_name);
				else
					gr_printf(10, t*10, "Turret %d: <Object %d>", t, pss->turret_enemy_objnum);

				t++;
			}
		}
	}
#endif


	if (Show_framerate)	{
		gr_set_color_fast(&HUD_color_debug);

		if (frametotal != 0.0f)
			gr_printf( 20, 100, "FPS: %0.1f", Framerate );
		else
			gr_string( 20, 100, "FPS: ?" );
	}

#ifndef NDEBUG
	if ( Debug_dump_frames )
		return;
#endif	

	// possibly show control checking info
	control_check_indicate();

#ifdef _WIN32
	if (Cmdline_show_stats && HUD_draw) {
		char mem_buffer[50];

		MEMORYSTATUS mem_stats;
		GlobalMemoryStatus(&mem_stats);

		// on win2k+, it should be == -1 if >4gig (indicates wrap around)
		if ( ((int)Mem_starttime_phys == -1) || ((int)mem_stats.dwAvailPhys == -1) )
			sprintf(mem_buffer, "Using Physical: *** (>4G)");
		else
			sprintf(mem_buffer,"Using Physical: %d Meg",(Mem_starttime_phys - mem_stats.dwAvailPhys)/1024/1024);

		gr_string( 20, 120, mem_buffer);
		sprintf(mem_buffer,"Using Pagefile: %d Meg",(Mem_starttime_pagefile - mem_stats.dwAvailPageFile)/1024/1024);
		gr_string( 20, 130, mem_buffer);
		sprintf(mem_buffer,"Using Virtual:  %d Meg",(Mem_starttime_virtual - mem_stats.dwAvailVirtual)/1024/1024);
		gr_string( 20, 140, mem_buffer);

		if ( ((int)mem_stats.dwAvailPhys == -1) || ((int)mem_stats.dwTotalPhys == -1) )
			sprintf(mem_buffer, "Physical Free: *** / *** (>4G)");
		else
			sprintf(mem_buffer,"Physical Free: %d / %d Meg",mem_stats.dwAvailPhys/1024/1024, mem_stats.dwTotalPhys/1024/1024);

		gr_string( 20, 160, mem_buffer);
		sprintf(mem_buffer,"Pagefile Free: %d / %d Meg",mem_stats.dwAvailPageFile/1024/1024, mem_stats.dwTotalPageFile/1024/1024);
		gr_string( 20, 170, mem_buffer);
		sprintf(mem_buffer,"Virtual Free:  %d / %d Meg",mem_stats.dwAvailVirtual/1024/1024, mem_stats.dwTotalVirtual/1024/1024);
		gr_string( 20, 180, mem_buffer);
	}
#endif

#ifndef NDEBUG
	if ( Show_cpu == 1 ) {
		
		int sx,sy,dy;
		sx = 530;
		sy = 15;
		dy = gr_get_font_height() + 1;

		gr_set_color_fast(&HUD_color_debug);

		gr_printf( sx, sy, NOX("DMA: %s"), transfer_text );
		sy += dy;
		gr_printf( sx, sy, NOX("POLYP: %d"), modelstats_num_polys );
		sy += dy;
		gr_printf( sx, sy, NOX("POLYD: %d"), modelstats_num_polys_drawn );
		sy += dy;
		gr_printf( sx, sy, NOX("VERTS: %d"), modelstats_num_verts );
		sy += dy;

		{

			extern int Num_pairs;		// Number of object pairs that were checked.
			gr_printf( sx, sy, NOX("PAIRS: %d"), Num_pairs );
			sy += dy;

			extern int Num_pairs_checked;	// What percent of object pairs were checked.
			gr_printf( sx, sy, NOX("FVI: %d"), Num_pairs_checked );
			sy += dy;
			Num_pairs_checked = 0;

		}

		gr_printf( sx, sy, NOX("Snds: %d"), snd_num_playing() );
		sy += dy;

		if ( Timing_total > 0.01f )	{
			gr_printf(  sx, sy, NOX("CLEAR: %.0f%%"), Timing_clear*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("REND2D: %.0f%%"), Timing_render2*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("REND3D: %.0f%%"), Timing_render3*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("FLIP: %.0f%%"), Timing_flip*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("GAME: %.0f%%"), (Timing_total-(Timing_render2+Timing_render3+Timing_flip+Timing_clear))*100.0f/Timing_total );
			sy += dy;
		}
	}
	 	
	if ( Show_mem  ) {

		int sx,sy,dy;
		sx = (gr_screen.res == GR_1024) ? 870 : 530;
		sy = 15;
		dy = gr_get_font_height() + 1;

		gr_set_color_fast(&HUD_color_debug);

		{
			extern int TotalRam;
			gr_printf( sx, sy, NOX("DYN: %d KB\n"), TotalRam/1024 );
			sy += dy;
		}	

		{
			extern int Model_ram;
			gr_printf( sx, sy, NOX("POF: %d KB\n"), Model_ram/1024 );
			sy += dy;
		}	

		gr_printf( sx, sy, NOX("%s: %d KB\n"), (Cmdline_cache_bitmaps) ? NOX("C-BMP") : NOX("BMP"), bm_texture_ram/1024 );
		sy += dy;

		gr_printf( sx, sy, NOX("S-SRAM: %d KB\n"), Snd_sram/1024 );		// mem used to store game sound
		sy += dy;

		{
			extern int GL_textures_in;
			extern int GL_vertex_data_in;
			gr_printf( sx, sy, NOX("VRAM: %d KB\n"), (GL_textures_in + GL_vertex_data_in)/1024 );
			sy += dy;
		}
	}


	if ( Show_player_pos ) {
		int sx, sy;
		sx = 320;
		sy = 100;
		gr_printf(sx, sy, NOX("Player Pos: (%d,%d,%d)"), fl2i(Player_obj->pos.xyz.x), fl2i(Player_obj->pos.xyz.y), fl2i(Player_obj->pos.xyz.z));
	}

#ifdef _WIN32
	if (Cmdline_show_mem_usage) {
		void memblockinfo_sort();
		void memblockinfo_sort_get_entry(int index, char *filename, int *size);

		char mem_buffer[1000];
		char filename[MAX_PATH];
		int size;

	  	memblockinfo_sort();

		int mi = 0;
		for( ; mi < 30; mi++) {
			memblockinfo_sort_get_entry(mi, filename, &size);

			size /= 1024;

			if (size == 0)
				break;

			char *short_name = strrchr(filename, '\\');

			if (short_name == NULL)
				short_name = filename;
			else
				short_name++;

			sprintf(mem_buffer,"%s:\t%d K", short_name, size);
			gr_string( 20, 220 + (mi*10), mem_buffer);
		}

		sprintf(mem_buffer,"Total RAM:\t%d K", TotalRam / 1024);
		gr_string( 20, 230 + (mi*10), mem_buffer);
	}
#endif

	MONITOR_INC(NumPolys, modelstats_num_polys);
	MONITOR_INC(NumPolysDrawn, modelstats_num_polys_drawn );
	MONITOR_INC(NumVerts, modelstats_num_verts );

	modelstats_num_polys = 0;
	modelstats_num_polys_drawn = 0;
	modelstats_num_verts = 0;
	modelstats_num_sortnorms = 0;
#endif
}

void game_show_eye_pos(camid cid)
{
	if ( !Cmdline_show_pos )
		return;

	if(!cid.isValid())
		return;

	camera *cam = cid.getCamera();
	vec3d cam_pos = vmd_zero_vector;
	matrix cam_orient = vmd_identity_matrix;
	cam->get_info(&cam_pos, &cam_orient);

	//Do stuff
	int font_height = 2*gr_get_font_height();
	angles rot_angles;

	gr_set_color_fast(&HUD_color_debug);

	//Position
	gr_printf(20, 100 - font_height, "X:%f Y:%f Z:%f", cam_pos.xyz.x, cam_pos.xyz.y, cam_pos.xyz.z);
	font_height -= font_height/2;

	//Orientation
	vm_extract_angles_matrix(&rot_angles, &cam_orient);
	rot_angles.p *= (180/PI);
	rot_angles.b *= (180/PI);
	rot_angles.h *= (180/PI);
	gr_printf(20, 100 - font_height, "Xr:%f Yr:%f Zr:%f", rot_angles.p, rot_angles.b, rot_angles.h);
}

void game_show_standalone_framerate()
{
	float frame_rate=30.0f;
	if ( frame_int == -1 )	{
		int i;
		for (i=0; i<FRAME_FILTER; i++ )	{
			frametimes[i] = 0.0f;
		}
		frametotal = 0.0f;
		frame_int = 0;
	}
	frametotal -= frametimes[frame_int];
	frametotal += flRealframetime;
	frametimes[frame_int] = flRealframetime;
	frame_int = (frame_int + 1 ) % FRAME_FILTER;

	if ( frametotal != 0.0 )	{
		if ( Framecount >= FRAME_FILTER ){
			frame_rate = FRAME_FILTER / frametotal;
		} else {
			frame_rate = Framecount / frametotal;
		}
	}
	std_set_standalone_fps(frame_rate);
	Framecount++;
}

/**
 * Show the time remaining in a mission.  Used only when the end-mission sexpression is used
 *
 * mission_end_time is a global from missionparse.cpp that contains the mission time at which the
 * mission should end (in fixed seconds).  There is code in missionparse.cpp which actually handles
 * checking how much time is left.
 */
void game_show_time_left()
{
	int diff;

	if (Mission_end_time == -1)
		return;

	diff = f2i(Mission_end_time - Missiontime);
	// be sure to bash to 0.  diff could be negative on frame that we quit mission
	if (diff < 0)
		diff = 0;

	hud_set_default_color();
	gr_printf( 5, 40, XSTR( "Mission time remaining: %d seconds", 179), diff );
}

//========================================================================================
//=================== NEW DEBUG CONSOLE COMMANDS TO REPLACE OLD DEBUG PAUSE MENU =========
//========================================================================================

#ifndef NDEBUG

DCF(ai_pause,"Pauses ai")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	ai_paused = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) ai_paused = 0;	
		else if ( Dc_arg_type & ARG_NONE ) ai_paused = !ai_paused;	

		if (ai_paused)	{	
			obj_init_all_ships_physics();
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: ai_paused [bool]\nSets ai_paused to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "ai_paused is %s\n", (ai_paused?"TRUE":"FALSE") );	
}

DCF(single_step,"Single steps the game")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	game_single_step = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) game_single_step = 0;	
		else if ( Dc_arg_type & ARG_NONE ) game_single_step = !game_single_step;	

		last_single_step = 0;	// Make so single step waits a frame before stepping

	}	
	if ( Dc_help )	dc_printf( "Usage: single_step [bool]\nSets single_step to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "single_step is %s\n", (game_single_step?"TRUE":"FALSE") );	
}

DCF_BOOL(physics_pause, physics_paused)
DCF_BOOL(ai_rendering, Ai_render_debug_flag)
DCF_BOOL(ai_firing, Ai_firing_enabled )

// Create some simple aliases to these commands...
debug_command dc_s("s","shortcut for single_step",dcf_single_step);
debug_command dc_p("p","shortcut for physics_pause", dcf_physics_pause );
debug_command dc_r("r","shortcut for ai_rendering", dcf_ai_rendering );
debug_command dc_f("f","shortcut for ai_firing", dcf_ai_firing);
debug_command dc_a("a","shortcut for ai_pause", dcf_ai_pause);
#endif

//========================================================================================
//========================================================================================


void game_training_pause_do()
{
	int key;

	key = game_check_key();
	if (key > 0){
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	gr_flip();
}


void game_increase_skill_level()
{
	Game_skill_level++;
	if (Game_skill_level >= NUM_SKILL_LEVELS){
		Game_skill_level = 0;
	}
}

int	Player_died_time;

int View_percent = 100;


DCF(view, "Sets the percent of the 3d view to render.")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_INT);
		if ( (Dc_arg_int >= 5 ) || (Dc_arg_int <= 100) ) {
			View_percent = Dc_arg_int;
		} else {
			dc_printf( "Illegal value for view. (Must be from 5-100) \n\n");
			Dc_help = 1;
		}
	}

	if ( Dc_help ) {
		dc_printf("Usage: view [n]\nwhere n is percent of view to show (5-100).\n");
	}
	
	if ( Dc_status ) {
		dc_printf("View is set to %d%%\n", View_percent );
	}
}


/**
 * Set the clip region for the 3d rendering window
 */
void game_reset_view_clip()
{
	Cutscene_bar_flags = CUB_NONE;
	Cutscene_delta_time = 1.0f;
	Cutscene_bars_progress = 1.0f;
}

void game_set_view_clip(float frametime)
{
	if ((Game_mode & GM_DEAD) || (supernova_active() >= 2))
	{
		// Set the clip region for the letterbox "dead view"
		int yborder = gr_screen.max_h/4;

		if (g3_in_frame() == 0) {
			// Ensure that the bars are black
			gr_set_color(0,0,0);
			gr_rect(0, 0, gr_screen.max_w, yborder, false);
			gr_rect(0, gr_screen.max_h-yborder, gr_screen.max_w, yborder, false);
		} else {
			//	Numeric constants encouraged by J "pig farmer" S, who shall remain semi-anonymous.
			// J.S. I've changed my ways!! See the new "no constants" code!!!
			gr_set_clip(0, yborder, gr_screen.max_w, gr_screen.max_h - yborder*2, false );	
		}
	}
	else {
		// Set the clip region for normal view
		if ( View_percent >= 100 )	{
			gr_reset_clip();
		} else {
			int xborder, yborder;

			if ( View_percent < 5 )	{
				View_percent = 5;
			}

			float fp = i2fl(View_percent)/100.0f;
			int fi = fl2i(fl_sqrt(fp)*100.0f);
			if ( fi > 100 ) fi=100;
			
			xborder = ( gr_screen.max_w*(100-fi) )/200;
			yborder = ( gr_screen.max_h*(100-fi) )/200;

			gr_set_clip(xborder, yborder, gr_screen.max_w-xborder*2,gr_screen.max_h-yborder*2, false );
		}
	}
}


void show_debug_stuff()
{
	int	i;
	int	laser_count = 0, missile_count = 0;

	for (i=0; i<MAX_OBJECTS; i++) {
		if (Objects[i].type == OBJ_WEAPON){
			if (Weapon_info[Weapons[Objects[i].instance].weapon_info_index].subtype == WP_LASER){
				laser_count++;
			} else if (Weapon_info[Weapons[Objects[i].instance].weapon_info_index].subtype == WP_MISSILE){
				missile_count++;
			}
		}
	}

	nprintf(("Mike", "Frame: %i Lasers: %4i, Missiles: %4i\n", Framecount, laser_count, missile_count));
}

extern int Tool_enabled;
int tst = 0;
int tst_time = 0;
int tst_big = 0;
vec3d tst_pos;
int tst_bitmap = -1;
float tst_x, tst_y;
float tst_offset, tst_offset_total;
int tst_mode;
int tst_stamp;
void game_tst_frame_pre()
{
	// start tst
	if(tst == 3){
		tst = 0;

		// screen position
		vertex v;
		g3_rotate_vertex(&v, &tst_pos);
		g3_project_vertex(&v);	
	
		// offscreen
		if(!(
				(v.screen.xyw.x >= 0)
				&& (v.screen.xyw.x <= gr_screen.max_w)
				&& (v.screen.xyw.y >= 0)
				&& (v.screen.xyw.y <= gr_screen.max_h)
			))
		{
			return;
		}	

		// big ship? always tst
		if(tst_big){
			// within 3000 meters
			if( vm_vec_dist_quick(&tst_pos, &Eye_position) <= 3000.0f){
				tst = 2;				
			}
		} else {			
			// within 300 meters
			if( (vm_vec_dist_quick(&tst_pos, &Eye_position) <= 300.0f) && ((tst_time == 0) || ((time(NULL) - tst_time) >= 10)) ){
				tst = 2;				
			} 
		}			
	}

}
void game_tst_frame()
{
	int left = 0;

	if(!Tool_enabled){
		return;
	}
	
	// setup tst
	if(tst == 2){		
		tst_time = (int) time(NULL);

		// load the tst bitmap		
		switch((int)frand_range(0.0f, 3.0)){
		case 0:			
			tst_bitmap = bm_load("ig_jim");
			left = 1;
			mprintf(("TST 0\n"));
			break;

		case 1:
			tst_bitmap = bm_load("ig_kan");
			left = 0;
			mprintf(("TST 1\n"));
			break;

		case 2:
			tst_bitmap = bm_load("ig_jim");
			left = 1;
			mprintf(("TST 2\n"));
			break;
			
		default:			
			tst_bitmap = bm_load("ig_kan");
			left = 0;
			mprintf(("TST 3\n"));
			break;
		}

		if(tst_bitmap < 0){
			tst = 0;
			return;
		}		

		// get the tst bitmap dimensions
		int w, h;
		bm_get_info(tst_bitmap, &w, &h);

		// tst y
		tst_y = frand_range(0.0f, (float)gr_screen.max_h - h);

		snd_play(&Snds[SND_VASUDAN_BUP]);

		// tst x and direction
		tst_mode = 0;
		if(left){
			tst_x = (float)-w;
			tst_offset_total = (float)w;
			tst_offset = (float)w;
		} else {
			tst_x = (float)gr_screen.max_w;
			tst_offset_total = (float)-w;
			tst_offset = (float)w;
		}

		tst = 1;
	}

	// run tst
	if(tst == 1){
		float diff = (tst_offset_total / 0.5f) * flFrametime;

		// move the bitmap
		if(tst_mode == 0){
			tst_x += diff;
			
			tst_offset -= fl_abs(diff);
		} else if(tst_mode == 2){
			tst_x -= diff;
			
			tst_offset -= fl_abs(diff);
		}

		// draw the bitmap
		gr_set_bitmap(tst_bitmap);
		gr_bitmap((int)tst_x, (int)tst_y);

		if(tst_mode == 1){
			if(timestamp_elapsed_safe(tst_stamp, 1100)){
				tst_mode = 2;
			}
		} else {
			// if we passed the switch point
			if(tst_offset <= 0.0f){
				// switch modes
				switch(tst_mode){
				case 0:
					tst_mode = 1;
					tst_stamp = timestamp(1000);
					tst_offset = fl_abs(tst_offset_total);
					break;				

				case 2:				
					tst = 0;
					return;
				}
			}				
		}
	}
}
void game_tst_mark(object *objp, ship *shipp)
{
	ship_info *sip;	

	if(!Tool_enabled){
		return;
	}

	// bogus
	if((objp == NULL) || (shipp == NULL) || (shipp->ship_info_index < 0) || (shipp->ship_info_index >= Num_ship_classes)){
		return;
	}
	sip = &Ship_info[shipp->ship_info_index];

	// already tst
	if(tst){
		return;
	}

	tst_pos = objp->pos;
	if(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)){
		tst_big = 1;
	}
	tst = 3;
}

extern void render_shields();

void player_repair_frame(float frametime)
{
	if(MULTIPLAYER_MASTER){
		int idx;
		for(idx=0;idx<MAX_PLAYERS;idx++){
			net_player *np;

			np = &Net_players[idx];

			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != NULL) && (Net_player->player_id != Net_players[idx].player_id) && (Net_players[idx].m_player != NULL) && (Net_players[idx].m_player->objnum >= 0) && (Net_players[idx].m_player->objnum < MAX_OBJECTS)){

				// don't rearm/repair if the player is dead or dying/departing
				if ( !NETPLAYER_IS_DEAD(np) && !(Ships[Objects[np->m_player->objnum].instance].flags & (SF_DYING|SF_DEPARTING)) ) {
					ai_do_repair_frame(&Objects[Net_players[idx].m_player->objnum],&Ai_info[Ships[Objects[Net_players[idx].m_player->objnum].instance].ai_index],frametime);
				}
			}
		}
	}	

	if ( (Player_obj != NULL) && (Player_obj->type == OBJ_SHIP) && !(Game_mode & GM_STANDALONE_SERVER) && (Player_ship != NULL) && !(Player_ship->flags & SF_DYING) ) {
		ai_do_repair_frame(Player_obj, &Ai_info[Ships[Player_obj->instance].ai_index], frametime);
	}
}

#define NUM_FRAMES_TEST		300
#define NUM_MIXED_SOUNDS	16
void do_timing_test(float frame_time)
{
	static int framecount = 0;
	static int test_running = 0;
	static float test_time = 0.0f;

	static int snds[NUM_MIXED_SOUNDS];
	int i;

	if ( test_running ) {
		framecount++;
		test_time += frame_time;
		if ( framecount >= NUM_FRAMES_TEST ) {
			test_running = 0;
			nprintf(("General", "%d frames took %.3f seconds\n", NUM_FRAMES_TEST, test_time));
			for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
				snd_stop(snds[i]);
		}
	}

	if ( Test_begin == 1 ) {
		framecount = 0;
		test_running = 1;
		test_time = 0.0f;
		Test_begin = 0;

		for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
			snds[i] = -1;

		// start looping digital sounds
		for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
			snds[i] = snd_play_looping( &Snds[i], 0.0f, -1, -1);
	}
	

}

DCF(dcf_fov, "Change the field of view of the main camera")
{
	camera *cam = Main_camera.getCamera();
	if ( Dc_command )
	{
		if(cam == NULL)
			return;

		dc_get_arg(ARG_FLOAT|ARG_NONE);
		if ( Dc_arg_type & ARG_NONE )	{
			cam->set_fov(VIEWER_ZOOM_DEFAULT);
			dc_printf( "Zoom factor reset\n" );
		}
		if ( Dc_arg_type & ARG_FLOAT )	{
			if (Dc_arg_float < 0.25f) {
				cam->set_fov(0.25f);
				dc_printf("Zoom factor pinned at 0.25.\n");
			} else if (Dc_arg_float > 1.25f) {
				cam->set_fov(1.25f);
				dc_printf("Zoom factor pinned at 1.25.\n");
			} else {
				cam->set_fov(Dc_arg_float);
			}
		}
	}

	if ( Dc_help )	
		dc_printf( "Usage: fov [factor]\nFactor is the zoom factor btwn .25 and 1.25\nNo parameter resets it to default.\n" );

	if ( Dc_status )
	{
		if(cam == NULL)
			dc_printf("Camera unavailable.");
		else
			dc_printf("Zoom factor set to %6.3f (original = 0.5, John = 0.75)", cam->get_fov());
	}
}


DCF(framerate_cap, "Sets the framerate cap")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_INT);
		if ( (Dc_arg_int >= 1 ) || (Dc_arg_int <= 120) ) {
			Framerate_cap = Dc_arg_int;
		} else {
			dc_printf( "Illegal value for framerate cap. (Must be from 1-120) \n\n");
			Dc_help = 1;
		}
	}

	if ( Dc_help ) {
		dc_printf("Usage: framerate_cap [n]\nwhere n is the frames per second to cap framerate at.\n");
		dc_printf("If n is 0 or omitted, then the framerate cap is removed\n");
		dc_printf("[n] must be from 1 to 120.\n");
	}
	
	if ( Dc_status ) {
		if ( Framerate_cap )
			dc_printf("Framerate cap is set to %d fps\n", Framerate_cap );
		else
			dc_printf("There is no framerate cap currently active.\n");
	}
}

#define	MIN_DIST_TO_DEAD_CAMERA		50.0f
int Show_viewing_from_self = 0;

void say_view_target()
{
	object	*view_target;

	if ((Viewer_mode & VM_OTHER_SHIP) && (Player_ai->target_objnum != -1))
		view_target = &Objects[Player_ai->target_objnum];
	else
		view_target = Player_obj;

	if (Game_mode & GM_DEAD) {
		if (Player_ai->target_objnum != -1)
			view_target = &Objects[Player_ai->target_objnum];
	}

	if (!(Game_mode & GM_DEAD_DIED) && ((Game_mode & (GM_DEAD_BLEW_UP)) || ((Last_view_target != NULL) && (Last_view_target != view_target)))) {
		if (view_target != Player_obj){

			char view_target_name[128] = "";
			switch(Objects[Player_ai->target_objnum].type) {
			case OBJ_SHIP:
				if (Ships[Objects[Player_ai->target_objnum].instance].flags2 & SF2_HIDE_SHIP_NAME) {
					strcpy_s(view_target_name, "targeted ship");
				} else {
					strcpy_s(view_target_name, Ships[Objects[Player_ai->target_objnum].instance].ship_name);
				}
				break;
			case OBJ_WEAPON:
				strcpy_s(view_target_name, Weapon_info[Weapons[Objects[Player_ai->target_objnum].instance].weapon_info_index].name);
				Viewer_mode &= ~VM_OTHER_SHIP;
				break;
			case OBJ_JUMP_NODE: {
				strcpy_s(view_target_name, XSTR( "jump node", 184));
				Viewer_mode &= ~VM_OTHER_SHIP;
				break;
				}
			case OBJ_DEBRIS: {
				strcpy_s(view_target_name, "Debris");
				Viewer_mode &= ~VM_OTHER_SHIP;
				break;
				}

			default:
				Int3();
				break;
			}

			end_string_at_first_hash_symbol(view_target_name);
			if ( strlen(view_target_name) ) {
				HUD_fixed_printf(0.0f, XSTR( "Viewing %s%s\n", 185), (Viewer_mode & VM_OTHER_SHIP) ? XSTR( "from ", 186) : "", view_target_name);
				Show_viewing_from_self = 1;
			}
		} else {
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER) && (Player_obj->type == OBJ_OBSERVER)){
				HUD_fixed_printf(2.0f,XSTR( "Viewing from observer\n", 187));
				Show_viewing_from_self = 1;
			} else {
				if (Show_viewing_from_self)
					HUD_fixed_printf(2.0f, XSTR( "Viewing from self\n", 188));
			}
		}
	}

	Last_view_target = view_target;
}


float Game_hit_x = 0.0f;
float Game_hit_y = 0.0f;

// Reset at the beginning of each frame
void game_whack_reset()
{
	Game_hit_x = 0.0f;
	Game_hit_y = 0.0f;
}

// Apply a 2d whack to the player
void game_whack_apply( float x, float y )
{
	// Do some force feedback
	joy_ff_play_dir_effect(x * 80.0f, y * 80.0f);

	// Move the eye 
	Game_hit_x += x;
	Game_hit_y += y;

//	mprintf(( "WHACK = %.1f, %.1f\n", Game_hit_x, Game_hit_y ));
}

// call to apply a "shudder"
void game_shudder_apply(int time, float intensity)
{
	Game_shudder_time = timestamp(time);
	Game_shudder_total = time;
	Game_shudder_intensity = intensity;
}

float get_shake(float intensity, int decay_time, int max_decay_time)
{
	int r = myrand();

	float shake = intensity * (float) (r-RAND_MAX_2)/RAND_MAX;
	
	if (decay_time >= 0) {
		Assert(max_decay_time > 0);
		shake *= (0.5f - fl_abs(0.5f - (float) decay_time / (float) max_decay_time));
	}

	return shake;
}

#define FF_SCALE	10000
extern int Wash_on;
extern float sn_shudder;
void apply_view_shake(matrix *eye_orient)
{
	angles tangles;
	tangles.p = 0.0f;
	tangles.h = 0.0f;
	tangles.b = 0.0f;

	// do shakes that only affect the HUD
	if (Viewer_obj == Player_obj) {
		physics_info *pi = &Player_obj->phys_info;

		// Make eye shake due to afterburner
		if (!timestamp_elapsed(pi->afterburner_decay)) {
			tangles.p += get_shake(0.07f, timestamp_until(pi->afterburner_decay), ABURN_DECAY_TIME);
			tangles.h += get_shake(0.07f, timestamp_until(pi->afterburner_decay), ABURN_DECAY_TIME);
		}

		// Make eye shake due to engine wash
		if (Player_obj->type == OBJ_SHIP && (Ships[Player_obj->instance].wash_intensity > 0) && Wash_on ) {
			float wash_intensity = Ships[Player_obj->instance].wash_intensity;
	
			tangles.p += get_shake(0.07f * wash_intensity, -1, 0);
			tangles.h += get_shake(0.07f * wash_intensity, -1, 0);
	
			// play the force feedback effect
			vec3d rand_vec;
			vm_vec_rand_vec_quick(&rand_vec);
			joy_ff_play_dir_effect(FF_SCALE * wash_intensity * rand_vec.xyz.x, FF_SCALE * wash_intensity * rand_vec.xyz.y);
		}

		// Make eye shake due to shuddering
		if (Game_shudder_time != -1) {
			if (timestamp_elapsed(Game_shudder_time)) {
				Game_shudder_time = -1;
			} else {
				tangles.p += get_shake(Game_shudder_intensity * 0.005f, timestamp_until(Game_shudder_time), Game_shudder_total);
				tangles.h += get_shake(Game_shudder_intensity * 0.005f, timestamp_until(Game_shudder_time), Game_shudder_total);
			}
		}
	}
	// do shakes that affect external cameras
	else {
		// Make eye shake due to supernova
		if (supernova_camera_cut()) {
			float cut_pct = 1.0f - (supernova_time_left() / SUPERNOVA_CUT_TIME);
			tangles.p += get_shake(0.07f * cut_pct * sn_shudder, -1, 0);
			tangles.h += get_shake(0.07f * cut_pct * sn_shudder, -1, 0);
		}
	}

	// maybe bail
	if (tangles.p == 0.0f && tangles.h == 0.0f && tangles.b == 0.0f)
		return;

	matrix	tm, tm2;
	vm_angles_2_matrix(&tm, &tangles);
	Assert(vm_vec_mag(&tm.vec.fvec) > 0.0f);
	Assert(vm_vec_mag(&tm.vec.rvec) > 0.0f);
	Assert(vm_vec_mag(&tm.vec.uvec) > 0.0f);
	vm_matrix_x_matrix(&tm2, eye_orient, &tm);
	*eye_orient = tm2;
}

//	Player's velocity just before he blew up.  Used to keep camera target moving.
vec3d	Dead_player_last_vel = { { { 1.0f, 1.0f, 1.0f } } };

extern float View_zoom;
inline void render_environment(int i, vec3d *eye_pos, matrix *new_orient, float new_zoom)
{
	bm_set_render_target(gr_screen.envmap_render_target, i);

	gr_clear();

	g3_set_view_matrix( eye_pos, new_orient, new_zoom );

	gr_set_proj_matrix( PI_2 * new_zoom, 1.0f, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix( &Eye_position, &Eye_matrix );

	if ( Game_subspace_effect ) {
		stars_draw(0, 0, 0, 1, 1);
	} else {
		stars_draw(0, 1, 1, 0, 1);
	}

	gr_end_view_matrix();
	gr_end_proj_matrix();
}

void setup_environment_mapping(camid cid)
{
	matrix new_orient = IDENTITY_MATRIX;
	float old_zoom = View_zoom, new_zoom = 1.0f;//0.925f;
	int i = 0;


	if (Cmdline_nohtl)
		return;

	if(!cid.isValid())
		return;

	vec3d cam_pos;
	matrix cam_orient;
	cid.getCamera()->get_info(&cam_pos, &cam_orient);

	// prefer the mission specified envmap over the static-generated envmap, but
	// the dynamic envmap should always get preference if in a subspace mission
	if ( !Dynamic_environment && strlen(The_mission.envmap_name) ) {
		ENVMAP = bm_load(The_mission.envmap_name);

		if (ENVMAP >= 0)
			return;
	}

	if (gr_screen.envmap_render_target < 0) {
		if (ENVMAP >= 0)
			return;

		if (strlen(The_mission.envmap_name)) {
			ENVMAP = bm_load(The_mission.envmap_name);

			if (ENVMAP < 0)
				ENVMAP = Default_env_map;
		} else {
			ENVMAP = Default_env_map;
		}

		return;
	}

	ENVMAP = gr_screen.envmap_render_target;

/*
	Envmap matrix setup -- left-handed
	-------------------------------------------------
	Face --	Forward		Up		Right
	px		+X			+Y		-Z
	nx		-X			+Y		+Z
	py		+Y			-Z		+X
	ny		-Y			+Z		+X
	pz		+Z 			+Y		+X
	nz		-Z			+Y		-X
*/

	// NOTE: OpenGL needs up/down reversed

	// face 1 (px / right)
	memset( &new_orient, 0, sizeof(matrix) );
	new_orient.vec.fvec.xyz.x =  1.0f;
	new_orient.vec.uvec.xyz.y =  1.0f;
	new_orient.vec.rvec.xyz.z = -1.0f;
	render_environment(i, &cam_pos, &new_orient, new_zoom);
	i++; // bump!

	// face 2 (nx / left)
	memset( &new_orient, 0, sizeof(matrix) );
	new_orient.vec.fvec.xyz.x = -1.0f;
	new_orient.vec.uvec.xyz.y =  1.0f;
	new_orient.vec.rvec.xyz.z =  1.0f;
	render_environment(i, &cam_pos, &new_orient, new_zoom);
	i++; // bump!

	// face 3 (py / up)
	memset( &new_orient, 0, sizeof(matrix) );
	new_orient.vec.fvec.xyz.y =  (gr_screen.mode == GR_OPENGL) ? -1.0f :  1.0f;
	new_orient.vec.uvec.xyz.z =  (gr_screen.mode == GR_OPENGL) ?  1.0f : -1.0f;
	new_orient.vec.rvec.xyz.x =  1.0f;
	render_environment(i, &cam_pos, &new_orient, new_zoom);
	i++; // bump!

	// face 4 (ny / down)
	memset( &new_orient, 0, sizeof(matrix) );
	new_orient.vec.fvec.xyz.y =  (gr_screen.mode == GR_OPENGL) ?  1.0f : -1.0f;
	new_orient.vec.uvec.xyz.z =  (gr_screen.mode == GR_OPENGL) ? -1.0f :  1.0f;
	new_orient.vec.rvec.xyz.x =  1.0f;
	render_environment(i, &cam_pos, &new_orient, new_zoom);
	i++; // bump!

	// face 5 (pz / forward)
	memset( &new_orient, 0, sizeof(matrix) );
	new_orient.vec.fvec.xyz.z =  1.0f;
	new_orient.vec.uvec.xyz.y =  1.0f;
	new_orient.vec.rvec.xyz.x =  1.0f;
	render_environment(i, &cam_pos, &new_orient, new_zoom);
	i++; // bump!

	// face 6 (nz / back)
	memset( &new_orient, 0, sizeof(matrix) );
	new_orient.vec.fvec.xyz.z = -1.0f;
	new_orient.vec.uvec.xyz.y =  1.0f;
	new_orient.vec.rvec.xyz.x = -1.0f;
	render_environment(i, &cam_pos, &new_orient, new_zoom);


	// we're done, so now reset
	bm_set_render_target(-1);
	g3_set_view_matrix( &cam_pos, &cam_orient, old_zoom );
}

// setup the render target ready for this mission's environment map
void game_environment_map_gen()
{
	const int size = 512;
	int gen_flags = (BMP_FLAG_RENDER_TARGET_STATIC | BMP_FLAG_CUBEMAP);

	if ( !Cmdline_env ) {
		return;
	}

	if (gr_screen.envmap_render_target >= 0) {
		if ( !bm_release(gr_screen.envmap_render_target, 1) ) {
			Warning(LOCATION, "Unable to release environment map render target.");
		}

		gr_screen.envmap_render_target = -1;
	}

	if ( Dynamic_environment || (The_mission.flags & MISSION_FLAG_SUBSPACE) ) {
		Dynamic_environment = true;
		gen_flags &= ~BMP_FLAG_RENDER_TARGET_STATIC;
		gen_flags |= BMP_FLAG_RENDER_TARGET_DYNAMIC;
	}
	// bail if we are going to be static, and have an envmap specified already
	else if ( strlen(The_mission.envmap_name) ) {
		return;
	}

	gr_screen.envmap_render_target = bm_make_render_target(size, size, gen_flags);
}

int Scripting_didnt_draw_hud = 1;

camid chase_get_camera()
{
	static camid chase_camera;
	if(!chase_camera.isValid())
	{
		chase_camera = cam_create("Chase camera");
	}

	return chase_camera;
}

extern vec3d Dead_camera_pos;

//	Set eye_pos and eye_orient based on view mode.
camid game_render_frame_setup()
{
    bool fov_changed;

	if(!Main_camera.isValid())
	{
		Main_camera = cam_create("Main camera");
	}
	camera *main_cam = Main_camera.getCamera();
	if(main_cam == NULL)
	{
		Error(LOCATION, "Unable to generate main camera");
		return camid();
	}

	vec3d	eye_pos;
	matrix	eye_orient = vmd_identity_matrix;
	vec3d	tmp_dir;

	static int last_Viewer_mode = 0;
	static int last_Game_mode = 0;
	static int last_Viewer_objnum = -1;
	static float last_FOV = Sexp_fov;

    fov_changed = ((last_FOV != Sexp_fov) && (Sexp_fov > 0.0f));

	//First, make sure we take into account 2D Missions.
	//These replace the normal player in-cockpit view with a topdown view.
	if(The_mission.flags & MISSION_FLAG_2D_MISSION)
	{
		if(!Viewer_mode)
		{
			Viewer_mode = VM_TOPDOWN;
		}
	}

	// This code is supposed to detect camera "cuts"... like going between
	// different views.

	// determine if we need to regenerate the nebula
	if(	(!(last_Viewer_mode & VM_EXTERNAL) && (Viewer_mode & VM_EXTERNAL)) ||							// internal to external 
			((last_Viewer_mode & VM_EXTERNAL) && !(Viewer_mode & VM_EXTERNAL)) ||							// external to internal
			(!(last_Viewer_mode & VM_DEAD_VIEW) && (Viewer_mode & VM_DEAD_VIEW)) ||							// non dead-view to dead-view
			((last_Viewer_mode & VM_DEAD_VIEW) && !(Viewer_mode & VM_DEAD_VIEW)) ||							// dead-view to non dead-view
			(!(last_Viewer_mode & VM_WARP_CHASE) && (Viewer_mode & VM_WARP_CHASE)) ||						// non warp-chase to warp-chase
			((last_Viewer_mode & VM_WARP_CHASE) && !(Viewer_mode & VM_WARP_CHASE)) ||						// warp-chase to non warp-chase
			(!(last_Viewer_mode & VM_OTHER_SHIP) && (Viewer_mode & VM_OTHER_SHIP)) ||						// non other-ship to other-ship
			((last_Viewer_mode & VM_OTHER_SHIP) && !(Viewer_mode & VM_OTHER_SHIP)) ||						// other-ship to non-other ship
			(!(last_Viewer_mode & VM_FREECAMERA) && (Viewer_mode & VM_FREECAMERA)) ||
			((last_Viewer_mode & VM_FREECAMERA) && !(Viewer_mode & VM_FREECAMERA)) ||
			(!(last_Viewer_mode & VM_TOPDOWN) && (Viewer_mode & VM_TOPDOWN)) ||
			((last_Viewer_mode & VM_TOPDOWN) && !(Viewer_mode & VM_TOPDOWN)) ||
			(fov_changed) ||
			((Viewer_mode & VM_OTHER_SHIP) && (last_Viewer_objnum != Player_ai->target_objnum)) 		// other ship mode, but targets changes
			) {

		// regenerate the nebula
		neb2_eye_changed();
	}		

	if ( (last_Viewer_mode != Viewer_mode)
		|| (last_Game_mode != Game_mode)
		|| (fov_changed)
		|| (Viewer_mode & VM_FREECAMERA))	{
		//mprintf(( "************** Camera cut! ************\n" ));
		last_Viewer_mode = Viewer_mode;
		last_Game_mode = Game_mode;
		last_FOV = main_cam->get_fov();

		// Camera moved.  Tell stars & debris to not do blurring.
		stars_camera_cut();		
	}

	say_view_target();

	if ( Viewer_mode & VM_PADLOCK_ANY ) {
		player_display_packlock_view();
	}

	if (Game_mode & GM_DEAD) {
		vec3d	vec_to_deader, view_pos;
		float		dist;

		Viewer_mode |= VM_DEAD_VIEW;

		if (Player_ai->target_objnum != -1) {
			int view_from_player = 1;

			if (Viewer_mode & VM_OTHER_SHIP) {
				//	View from target.
				Viewer_obj = &Objects[Player_ai->target_objnum];

				last_Viewer_objnum = Player_ai->target_objnum;

				if ( Viewer_obj->type == OBJ_SHIP ) {
					ship_get_eye( &eye_pos, &eye_orient, Viewer_obj );
					view_from_player = 0;
				}
			} else {
				last_Viewer_objnum = -1;
			}

			if(Viewer_obj)
				Script_system.SetHookObject("Viewer", Viewer_obj);
			else
				Script_system.RemHookVar("Viewer");

			if ( view_from_player ) {
				//	View target from player ship.
				Viewer_obj = NULL;
				eye_pos = Player_obj->pos;
				vm_vec_normalized_dir(&tmp_dir, &Objects[Player_ai->target_objnum].pos, &eye_pos);
				vm_vector_2_matrix(&eye_orient, &tmp_dir, NULL, NULL);
				//rtn_cid = ship_get_followtarget_eye( Player_obj );
			}
		} else {
			dist = vm_vec_normalized_dir(&vec_to_deader, &Player_obj->pos, &Dead_camera_pos);
			
			if (dist < MIN_DIST_TO_DEAD_CAMERA)
				dist += flFrametime * 16.0f;

			vm_vec_scale(&vec_to_deader, -dist);
			vm_vec_add(&Dead_camera_pos, &Player_obj->pos, &vec_to_deader);
			
			view_pos = Player_obj->pos;

			if (!(Game_mode & GM_DEAD_BLEW_UP)) {
				Viewer_mode &= ~(VM_EXTERNAL | VM_CHASE);
				vm_vec_scale_add2(&Dead_camera_pos, &Original_vec_to_deader, 25.0f * flFrametime);
				Dead_player_last_vel = Player_obj->phys_info.vel;
				//nprintf(("AI", "Player death roll vel = %7.3f %7.3f %7.3f\n", Player_obj->phys_info.vel.x, Player_obj->phys_info.vel.y, Player_obj->phys_info.vel.z));
			} else if (Player_ai->target_objnum != -1) {
				view_pos = Objects[Player_ai->target_objnum].pos;
			} else {
				//	Make camera follow explosion, but gradually slow down.
				vm_vec_scale_add2(&Player_obj->pos, &Dead_player_last_vel, flFrametime);
				view_pos = Player_obj->pos;
				vm_vec_scale(&Dead_player_last_vel, 0.99f);
				vm_vec_scale_add2(&Dead_camera_pos, &Original_vec_to_deader, MIN(25.0f, vm_vec_mag_quick(&Dead_player_last_vel)) * flFrametime);
			}

			eye_pos = Dead_camera_pos;

			vm_vec_normalized_dir(&tmp_dir, &Player_obj->pos, &eye_pos);

			vm_vector_2_matrix(&eye_orient, &tmp_dir, NULL, NULL);
			Viewer_obj = NULL;
		}
	} 

	// if supernova shockwave
	if(supernova_camera_cut()){
		// no viewer obj
		Viewer_obj = NULL;

		// call it dead view
		Viewer_mode |= VM_DEAD_VIEW;

		// set eye pos and orient
		//rtn_cid = supernova_set_view();
		supernova_get_eye(&eye_pos, &eye_orient);
	} else {	
		//	If already blown up, these other modes can override.
		if (!(Game_mode & (GM_DEAD | GM_DEAD_BLEW_UP))) {
			Viewer_mode &= ~VM_DEAD_VIEW;

			if(!(Viewer_mode & VM_FREECAMERA))
				Viewer_obj = Player_obj;
 
			if (Viewer_mode & VM_OTHER_SHIP) {
				if (Player_ai->target_objnum != -1){
					Viewer_obj = &Objects[Player_ai->target_objnum];
					last_Viewer_objnum = Player_ai->target_objnum;
				} else {
					Viewer_mode &= ~VM_OTHER_SHIP;
					last_Viewer_objnum = -1;
				}
			} else {
				last_Viewer_objnum = -1;
			}

			if(Viewer_mode & VM_FREECAMERA) {
				Viewer_obj = NULL;
				return cam_get_current();
			} else if (Viewer_mode & VM_EXTERNAL) {
				matrix	tm, tm2;

				vm_angles_2_matrix(&tm2, &Viewer_external_info.angles);
				vm_matrix_x_matrix(&tm, &Viewer_obj->orient, &tm2);

				vm_vec_scale_add(&eye_pos, &Viewer_obj->pos, &tm.vec.fvec, 2.0f * Viewer_obj->radius + Viewer_external_info.distance);

				vm_vec_sub(&tmp_dir, &Viewer_obj->pos, &eye_pos);
				vm_vec_normalize(&tmp_dir);
				vm_vector_2_matrix(&eye_orient, &tmp_dir, &Viewer_obj->orient.vec.uvec, NULL);
				Viewer_obj = NULL;

				//	Modify the orientation based on head orientation.
				compute_slew_matrix(&eye_orient, &Viewer_slew_angles);

			} else if ( Viewer_mode & VM_CHASE ) {
				vec3d	move_dir;
				vec3d aim_pt;
								
				

				if ( Viewer_obj->phys_info.speed < 62.5f )
					move_dir = Viewer_obj->phys_info.vel;
				else {
					move_dir = Viewer_obj->phys_info.vel;
					vm_vec_scale(&move_dir, (62.5f/Viewer_obj->phys_info.speed));
				}

				//create a better 3rd person view if this is the player ship
				if (Viewer_obj==Player_obj)
				{
					//get a point 1000m forward of ship
					vm_vec_copy_scale(&aim_pt,&Viewer_obj->orient.vec.fvec,1000.0f);
					vm_vec_add2(&aim_pt,&Viewer_obj->pos);

					vm_vec_scale_add(&eye_pos, &Viewer_obj->pos, &move_dir, -0.02f * Viewer_obj->radius);
					vm_vec_scale_add2(&eye_pos, &Viewer_obj->orient.vec.fvec, -2.125f * Viewer_obj->radius - Viewer_chase_info.distance);
					vm_vec_scale_add2(&eye_pos, &Viewer_obj->orient.vec.uvec, 0.625f * Viewer_obj->radius + 0.35f * Viewer_chase_info.distance);
					vm_vec_sub(&tmp_dir, &aim_pt, &eye_pos);
					vm_vec_normalize(&tmp_dir);
				}
				else
				{
					vm_vec_scale_add(&eye_pos, &Viewer_obj->pos, &move_dir, -0.02f * Viewer_obj->radius);
					vm_vec_scale_add2(&eye_pos, &Viewer_obj->orient.vec.fvec, -2.5f * Viewer_obj->radius - Viewer_chase_info.distance);
					vm_vec_scale_add2(&eye_pos, &Viewer_obj->orient.vec.uvec, 0.75f * Viewer_obj->radius + 0.35f * Viewer_chase_info.distance);
					vm_vec_sub(&tmp_dir, &Viewer_obj->pos, &eye_pos);
					vm_vec_normalize(&tmp_dir);
				}
					
				// JAS: I added the following code because if you slew up using
				// Descent-style physics, tmp_dir and Viewer_obj->orient.vec.uvec are
				// equal, which causes a zero-length vector in the vm_vector_2_matrix
				// call because the up and the forward vector are the same.   I fixed
				// it by adding in a fraction of the right vector all the time to the
				// up vector.
				vec3d tmp_up = Viewer_obj->orient.vec.uvec;
				vm_vec_scale_add2( &tmp_up, &Viewer_obj->orient.vec.rvec, 0.00001f );

				vm_vector_2_matrix(&eye_orient, &tmp_dir, &tmp_up, NULL);
				Viewer_obj = NULL;

				//	Modify the orientation based on head orientation.
				compute_slew_matrix(&eye_orient, &Viewer_slew_angles);
			} else if ( Viewer_mode & VM_WARP_CHASE ) {
					Warp_camera.get_info(&eye_pos, NULL);

					ship * shipp = &Ships[Player_obj->instance];

					vec3d warp_pos = Player_obj->pos;
					shipp->warpout_effect->getWarpPosition(&warp_pos);
					vm_vec_sub(&tmp_dir, &warp_pos, &eye_pos);
					vm_vec_normalize(&tmp_dir);
					vm_vector_2_matrix(&eye_orient, &tmp_dir, &Player_obj->orient.vec.uvec, NULL);
					Viewer_obj = NULL;
			} else if (Viewer_mode & VM_TOPDOWN) {
					angles rot_angles = { PI_2, 0.0f, 0.0f };
					bool position_override = false;
					if(Viewer_obj->type == OBJ_SHIP) {
						ship_info *sip = &Ship_info[Ships[Viewer_obj->instance].ship_info_index];
						if(sip->topdown_offset_def) {
							eye_pos.xyz.x = Viewer_obj->pos.xyz.x + sip->topdown_offset.xyz.x;
							eye_pos.xyz.y = Viewer_obj->pos.xyz.y + sip->topdown_offset.xyz.y;
							eye_pos.xyz.z = Viewer_obj->pos.xyz.z + sip->topdown_offset.xyz.z;
							position_override = true;
						}
					}
					if(!position_override) {
						eye_pos.xyz.x = Viewer_obj->pos.xyz.x;
						eye_pos.xyz.y = Viewer_obj->pos.xyz.y + Viewer_obj->radius * 25.0f;
						eye_pos.xyz.z = Viewer_obj->pos.xyz.z;
					}
					vm_angles_2_matrix(&eye_orient, &rot_angles);
					Viewer_obj = NULL;
			} else {
				// get an eye position based upon the correct type of object
				switch(Viewer_obj->type){
				case OBJ_SHIP:
					// make a call to get the eye point for the player object
					ship_get_eye( &eye_pos, &eye_orient, Viewer_obj );
					break;
				case OBJ_OBSERVER:
					// make a call to get the eye point for the player object
					observer_get_eye( &eye_pos, &eye_orient, Viewer_obj );					
					break;
				default :
					mprintf(("Invalid Value for Viewer_obj->type. Expected values are OBJ_SHIP (1) and OBJ_OBSERVER (12), we encountered %d. Please tell a coder.\n", Viewer_obj->type));
					Int3();
				}

				#ifdef JOHNS_DEBUG_CODE
				john_debug_stuff(&eye_pos, &eye_orient);
				#endif
			}
		}
	}

	main_cam->set_position(&eye_pos);
	main_cam->set_rotation(&eye_orient);

	// setup neb2 rendering
	neb2_render_setup(Main_camera);

	return Main_camera;
}

#ifndef NDEBUG
extern void ai_debug_render_stuff();
#endif

int Game_subspace_effect = 0;
DCF_BOOL( subspace, Game_subspace_effect )

void clip_frame_view();

// Does everything needed to render a frame
extern SCP_vector<object*> effect_ships; 
void game_render_frame( camid cid )
{

	g3_start_frame(game_zbuffer);

	camera *cam = cid.getCamera();
	matrix eye_no_jitter = vmd_identity_matrix;
	if(cam != NULL)
	{
		vec3d eye_pos;
		matrix eye_orient;

		//Get current camera info
		cam->get_info(&eye_pos, &eye_orient);

		//Handle jitter if not cutscene camera
		eye_no_jitter = eye_orient;
		if( !(Viewer_mode & VM_FREECAMERA) ) {
			apply_view_shake(&eye_orient);
			cam->set_rotation(&eye_orient);
		}

		//Maybe override FOV from SEXP
		if(Sexp_fov <= 0.0f)
			g3_set_view_matrix(&eye_pos, &eye_orient, cam->get_fov());
		else
			g3_set_view_matrix(&eye_pos, &eye_orient, Sexp_fov);
	}
	else
	{
		g3_set_view_matrix(&vmd_zero_vector, &vmd_identity_matrix, VIEWER_ZOOM_DEFAULT);
	}

	// maybe offset the HUD (jitter stuff) and measure the 2D displacement between the player's view and ship vector
	int dont_offset = ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER));
	HUD_set_offsets(Viewer_obj, !dont_offset, &eye_no_jitter);

	// for multiplayer clients, call code in Shield.cpp to set up the Shield_hit array.  Have to
	// do this becaues of the disjointed nature of this system (in terms of setup and execution).
	// must be done before ships are rendered
	if ( MULTIPLAYER_CLIENT ) {
		shield_point_multi_setup();
	}

	// this needs to happen after g3_start_frame() and before the primary projection and view matrix is setup
	if ( Cmdline_env && !Env_cubemap_drawn ) {
		setup_environment_mapping(cid);

		if ( !Dynamic_environment ) {
			Env_cubemap_drawn = true;
		}
	}
	gr_zbuffer_clear(TRUE);

	gr_scene_texture_begin();

	neb2_render_setup(cid);

#ifndef DYN_CLIP_DIST
	if (!Cmdline_nohtl) {
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}
#endif

	if ( Game_subspace_effect )	{
		stars_draw(0,0,0,1,0);
	} else {
		stars_draw(1,1,1,0,0);
	}

	bool draw_viewer_last = false;
	obj_render_all(obj_render, &draw_viewer_last);
	
	//	Why do we not show the shield effect in these modes?  Seems ok.
	//if (!(Viewer_mode & (VM_EXTERNAL | VM_SLEWED | VM_CHASE | VM_DEAD_VIEW))) {
	render_shields();
	//}
	particle_render_all();					// render particles after everything else.
#ifdef DYN_CLIP_DIST
	if(!Cmdline_nohtl)
	{
		gr_end_proj_matrix();
		gr_end_view_matrix();
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);
	}
#endif

	beam_render_all();						// render all beam weapons
	trail_render_all();						// render missilie trails after everything else.	

	// render nebula lightning
	nebl_render_all();

	// render local player nebula
	neb2_render_player();

	// render all ships with shader effects on them
	SCP_vector<object*>::iterator obji = effect_ships.begin();
	for(;obji != effect_ships.end();++obji)
		ship_render(*obji);
	effect_ships.clear();
	//Draw the viewer 'cause we didn't before.
	//This is so we can change the minimum clipping distance without messing everything up.
	if(draw_viewer_last && Viewer_obj)
	{
		gr_post_process_save_zbuffer();
		gr_zbuffer_clear(TRUE);
		ship_render(Viewer_obj);
	}


#ifndef NDEBUG
	ai_debug_render_stuff();
	extern void snd_spew_debug_info();
	snd_spew_debug_info();
#endif

	if(!Cmdline_nohtl)
	{
		gr_end_proj_matrix();
		gr_end_view_matrix();
	}

	//Draw viewer cockpit
	if(Viewer_obj != NULL && Viewer_mode != VM_TOPDOWN)
	{
		gr_post_process_save_zbuffer();
		gr_zbuffer_clear(TRUE);
		ship_render_cockpit(Viewer_obj);
	}

	if (!Cmdline_nohtl) {
		gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
		gr_set_view_matrix(&Eye_position, &Eye_matrix);

		// Do the sunspot
		game_sunspot_process(flFrametime);

		gr_end_proj_matrix();
		gr_end_view_matrix();
	}

	//================ END OF 3D RENDERING STUFF ====================

	gr_scene_texture_end();

	extern int Multi_display_netinfo;
	if(Multi_display_netinfo){
		extern void multi_display_netinfo();
		multi_display_netinfo();
	}	

	game_tst_frame_pre();

#ifndef NDEBUG
	do_timing_test(flFrametime);

	extern int OO_update_index;	
	multi_rate_display(OO_update_index, 375, 0);

	// test
	extern void oo_display();
	oo_display();			
#endif
	
	g3_end_frame();
}

//#define JOHNS_DEBUG_CODE	1

#ifdef JOHNS_DEBUG_CODE
void john_debug_stuff(vec3d *eye_pos, matrix *eye_orient)
{
	//if ( keyd_pressed[KEY_LSHIFT] )		
	{
		ship_subsys *tsys = Players[Player_num].targeted_subobject;
		if ( tsys )	{
			model_subsystem *turret = tsys->system_info;

			if (turret->type == SUBSYSTEM_TURRET )	{
				vec3d fvec, uvec;
				object * tobj = &Objects[Players[Player_num].targeted_subobject_parent];

				ship_model_start(tobj);

				model_find_world_point(eye_pos, &turret->turret_firing_point[0], turret->model_num, turret->turret_gun_sobj, &tobj->orient, &tobj->pos );
				model_find_world_dir(&fvec, &turret->turret_matrix.vec.fvec, turret->model_num, turret->turret_gun_sobj, &tobj->orient, NULL );
				model_find_world_dir(&uvec, &turret->turret_matrix.vec.uvec, turret->model_num, turret->turret_gun_sobj, &tobj->orient, NULL );
				
				vm_vector_2_matrix( eye_orient, &fvec, &uvec, NULL );

				ship_model_stop(tobj);

				Viewer_obj = NULL;
			}
		}

	}
}
#endif

// following function for dumping frames for purposes of building trailers.
#ifndef NDEBUG

// function to toggle state of dumping every frame into PCX when playing the game
DCF(dump_frames, "Starts/stop frame dumping at 15 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 15;
			Debug_dump_trigger = 0;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 15 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames_trigger, "Starts/stop frame dumping at 15 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 15;
			Debug_dump_trigger = 1;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 15 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames30, "Starts/stop frame dumping at 30 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 30;
			Debug_dump_trigger = 0;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 30 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames30_trigger, "Starts/stop frame dumping at 30 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 30;
			Debug_dump_trigger = 1;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Triggered frame dumping at 30 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Triggered frame dumping is now OFF\n" );
		}
		
	}
}

void game_maybe_dump_frame()
{
	if ( !Debug_dump_frames ){
		return;
	}

	if( Debug_dump_trigger && !keyd_pressed[KEY_Q] ){
		return;
	}

	game_stop_time();

	gr_dump_frame();
	Debug_dump_frame_num++;

	game_start_time();
}
#endif

extern int Player_dead_state;

//	Flip the page and time how long it took.
void game_flip_page_and_time_it()
{	
	fix t1, t2,d;
	int t;
	t1 = timer_get_fixed_seconds();
	gr_flip();
	t2 = timer_get_fixed_seconds();
	d = t2 - t1;
	t = (gr_screen.max_w*gr_screen.max_h*gr_screen.bytes_per_pixel)/1024;
	sprintf( transfer_text, NOX("%d MB/s"), (int)fixmuldiv(t,65,d) );
}

void game_simulation_frame()
{
	//Do camera stuff
	//This is for the warpout cam
	if ( Player->control_mode != PCM_NORMAL )
		Warp_camera.do_frame(flFrametime);

	//Do ingame cutscenes stuff
	if(!Time_compression_locked)
	{
		cam_do_frame(flFrametime);
	}
	else
	{
		cam_do_frame(flRealframetime);
	}

	// blow ships up in multiplayer dogfight
	if( MULTIPLAYER_MASTER && (Net_player != NULL) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && (f2fl(Missiontime) >= 2.0f) && !dogfight_blown){
		// blow up all non-player ships
		ship_obj *moveup = GET_FIRST(&Ship_obj_list);
		ship *shipp;
		ship_info *sip;
		while((moveup != END_OF_LIST(&Ship_obj_list)) && (moveup != NULL)){
			// bogus
			if((moveup->objnum < 0) || (moveup->objnum >= MAX_OBJECTS) || (Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0) || (Objects[moveup->objnum].instance >= MAX_SHIPS) || (Ships[Objects[moveup->objnum].instance].ship_info_index < 0) || (Ships[Objects[moveup->objnum].instance].ship_info_index >= Num_ship_classes)){
				moveup = GET_NEXT(moveup);
				continue;
			}
			shipp = &Ships[Objects[moveup->objnum].instance];
			sip = &Ship_info[shipp->ship_info_index];

			// only blow up small ships			
			if((sip->flags & SIF_SMALL_SHIP) && (multi_find_player_by_object(&Objects[moveup->objnum]) < 0) && (shipp->team == Iff_traitor) ){							
				// function to simply explode a ship where it is currently at
				ship_self_destruct( &Objects[moveup->objnum] );					
			}

			moveup = GET_NEXT(moveup);
		}

		dogfight_blown = 1;
	}

	// process AWACS stuff - do this first thing
	awacs_process();

	//Do autopilot stuff
	NavSystem_Do();

	// single player, set Player hits_this_frame to 0
	if ( !(Game_mode & GM_MULTIPLAYER) && Player ) {
		Player->damage_this_burst -= (flFrametime * MAX_BURST_DAMAGE  / (0.001f * BURST_DURATION));
		Player->damage_this_burst = MAX(Player->damage_this_burst, 0.0f);
	}

	// supernova
	supernova_process();
	if(supernova_active() >= 5){
		return;
	}

	// fire targeting lasers now so that 
	// 1 - created this frame
	// 2 - collide this frame
	// 3 - render this frame
	// 4 - ignored and deleted next frame
	// the basic idea being that because it con be confusing to deal with them on a multi-frame basis, they are only valid for
	// frame
	ship_process_targeting_lasers();	

	// do this here so that it works for multiplayer
	if ( Viewer_obj ) {
		// get viewer direction
		int viewer_direction = PHYSICS_VIEWER_REAR;

		if(Viewer_mode == 0){
			viewer_direction = PHYSICS_VIEWER_FRONT;
		}
		if(Viewer_mode & VM_PADLOCK_UP){
			viewer_direction = PHYSICS_VIEWER_UP;
		}
		else if(Viewer_mode & VM_PADLOCK_REAR){
			viewer_direction = PHYSICS_VIEWER_REAR;
		} 
		else if(Viewer_mode & VM_PADLOCK_LEFT){
			viewer_direction = PHYSICS_VIEWER_LEFT;
		} 
		else if(Viewer_mode & VM_PADLOCK_RIGHT){
			viewer_direction = PHYSICS_VIEWER_RIGHT;
		}

		physics_set_viewer( &Viewer_obj->phys_info, viewer_direction );
	} else {
		physics_set_viewer( NULL, PHYSICS_VIEWER_FRONT );
	}

	// evaluate mission departures and arrivals before we process all objects.
	if ( !(Game_mode & GM_MULTIPLAYER) || (MULTIPLAYER_MASTER && !multi_endgame_ending()) ) {

		// we don't want to evaluate mission stuff when any ingame joiner in multiplayer is receiving
		// ships/wing packets.
		if ( !((Game_mode & GM_MULTIPLAYER) && (Netgame.flags & NG_FLAG_INGAME_JOINING_CRITICAL))) {
			mission_parse_eval_stuff();
		}

		// if we're an observer, move ourselves seperately from the standard physics
		if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){
			obj_observer_move(flFrametime);
		}
		
		// move all the objects now
		obj_move_all(flFrametime);

		mission_eval_goals();
	}

	// always check training objectives, even in multiplayer missions. we need to do this so that the directives gauge works properly on clients
	training_check_objectives();
	
	// do all interpolation now
	if ( MULTIPLAYER_CLIENT && !multi_endgame_ending() && !(Netgame.flags & NG_FLAG_SERVER_LOST)) {
		// client side processing of warping in effect stages
		multi_do_client_warp(flFrametime);     
	
		// client side movement of an observer
		if((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER)){
			obj_observer_move(flFrametime);   
		}

		// move all objects - does interpolation now as well
		obj_move_all(flFrametime);


	}

	// only process the message queue when the player is "in" the game
	if ( !Pre_player_entry ){
		message_queue_process();				// process any messages send to the player
	}

	message_maybe_distort();				// maybe distort incoming message if comms damaged
	player_repair_frame(flFrametime);	//	AI objects get repaired in ai_process, called from move code...deal with player.
	player_process_pending_praise();		// maybe send off a delayed praise message to the player
	player_maybe_play_all_alone_msg();	// maybe tell the player he is all alone	

	if(!(Game_mode & GM_STANDALONE_SERVER)){		
		// process some stuff every frame (before frame is rendered)
		emp_process_local();

		hud_update_frame(flFrametime);						// update hud systems

		if (!physics_paused)	{
			// Move particle system
			particle_move_all(flFrametime);	

			// Move missile trails
			trail_move_all(flFrametime);		

			// Flash the gun flashes
			shipfx_flash_do_frame(flFrametime);			

			shockwave_move_all(flFrametime);	// update all the shockwaves
		}

		// subspace missile strikes
		ssm_process();

		obj_snd_do_frame();						// update the object-linked persistant sounds

		game_maybe_update_sound_environment();
		snd_update_listener(&View_position, &Player_obj->phys_info.vel, &Player_obj->orient);

// AL: debug code used for testing ambient subspace sound (ie when enabling subspace through debug console)
#ifndef NDEBUG
		if ( Game_subspace_effect ) {
			game_start_subspace_ambient_sound();
		}
#endif
	}

	Script_system.RunBytecode(Script_simulationhook);
}

// Maybe render and process the dead-popup
void game_maybe_do_dead_popup(float frametime)
{
	if ( popupdead_is_active() ) {
		int leave_popup=1;
		int choice = popupdead_do_frame(frametime);

		if ( Game_mode & GM_NORMAL ) {
			switch(choice) {
			case 0:
				gameseq_post_event(GS_EVENT_ENTER_GAME);
				break;

			case 1:
				gameseq_post_event(GS_EVENT_END_GAME);
				break;

			case 2:
				gameseq_post_event(GS_EVENT_START_GAME);
				break;

			// this should only happen during a red alert mission
			case 3:				
				if (The_mission.flags & MISSION_FLAG_RED_ALERT)
				{
					// choose the previous mission
					mission_campaign_previous_mission();
				}
				else
				{
					// bogus?
					Int3();
				}

				gameseq_post_event(GS_EVENT_START_GAME);
				break;

			default:
				leave_popup=0;
				break;
			}
		} else {
			switch( choice ) {

			case POPUPDEAD_DO_MAIN_HALL:
				multi_quit_game(PROMPT_NONE,-1);
				break;

			case POPUPDEAD_DO_RESPAWN:				
				multi_respawn_normal();
				event_music_player_respawn();
				break;

			case POPUPDEAD_DO_OBSERVER:
				multi_respawn_observer();
				event_music_player_respawn_as_observer();
				break;

			default:
				leave_popup = 0;
				break;
			}
		}

		if ( leave_popup ) {
			popupdead_close();
		}
	}
}

// returns true if player is actually in a game_play stats
int game_actually_playing()
{
	int state;

	state = gameseq_get_state();
	if ( (state != GS_STATE_GAME_PLAY) && (state != GS_STATE_DEATH_DIED) && (state != GS_STATE_DEATH_BLEW_UP) )
		return 0;
	else
		return 1;
}

void game_render_hud(camid cid)
{
	gr_reset_clip();

	if(cid.isValid()) {
		g3_start_frame(0);		// 0 = turn zbuffering off
		g3_set_view( cid.getCamera() );

		hud_render_preprocess(flFrametime);

		g3_end_frame();
	}

	// main HUD rendering function
	hud_render_all();

	// Diminish the palette effect
	game_flash_diminish(flFrametime);
}

//100% blackness
void game_reset_shade_frame()
{
	Fade_type = FI_NONE;
	Fade_delta_time = 1.0f;
	gr_create_shader(&Viewer_shader, 0, 0, 0, 0);
}

void game_shade_frame(float frametime)
{
	int alpha = 0;

	// only do frame shade if we are actually in a game play state
	if ( !game_actually_playing() ) {
		return;
	}

	if (Fade_type != FI_NONE)
	{
		if ( (Viewer_shader.c == 0) && (Fade_type != FI_FADEOUT) ) {
			return;
		}

		alpha = Viewer_shader.c;

		// Fade in or out if necessary
		if (Fade_type == FI_FADEOUT) {
			alpha += fl2i(frametime * (255.0f / Fade_delta_time) + 0.5f);
		} else if (Fade_type == FI_FADEIN) {
			alpha -= fl2i(frametime * (255.0f / Fade_delta_time) + 0.5f);
		}

		// Limit and set fade type if done
		if (alpha < 0) {
			alpha = 0;

			if (Fade_type == FI_FADEIN) {
				Fade_type = FI_NONE;
			}
		}

		if (alpha > 255) {
			alpha = 255;

			if (Fade_type == FI_FADEOUT) {
				Fade_type = FI_NONE;
			}
		}

		Viewer_shader.c = (ubyte)alpha;
	}

	gr_flash_alpha(Viewer_shader.r, Viewer_shader.g, Viewer_shader.b, Viewer_shader.c);
}

const static int CUTSCENE_BAR_DIVISOR = 8;
void bars_do_frame(float frametime)
{
	if((Cutscene_bar_flags & CUB_GRADUAL) && Cutscene_bars_progress < 1.0f)
	{
		//Determine how far along we are
		Assert(Cutscene_delta_time > 0.0f);

		Cutscene_bars_progress += frametime / Cutscene_delta_time;
		if(Cutscene_bars_progress >= 1.0f)
		{
			//Reset this stuff
			Cutscene_delta_time = 1.0f;
			Cutscene_bars_progress = 1.0f;
		}

		//Figure out where the bars should be
		int yborder;
		if(Cutscene_bar_flags & CUB_CUTSCENE)
			yborder = fl2i(Cutscene_bars_progress*(gr_screen.max_h/CUTSCENE_BAR_DIVISOR));
		else
			yborder = gr_screen.max_h/CUTSCENE_BAR_DIVISOR - fl2i(Cutscene_bars_progress*(gr_screen.max_h/CUTSCENE_BAR_DIVISOR));

		if (g3_in_frame() == 0) {
			//Set rectangles
			gr_set_color(0,0,0);
			gr_rect(0, 0, gr_screen.max_w, yborder, false);
			gr_rect(0, gr_screen.max_h-yborder, gr_screen.max_w, yborder, false);
		} else {
			//Set clipping
			gr_reset_clip();
			gr_set_clip(0, yborder, gr_screen.max_w, gr_screen.max_h - yborder*2, false );
		}
	}
	else if(Cutscene_bar_flags & CUB_CUTSCENE)
	{
		int yborder = gr_screen.max_h/CUTSCENE_BAR_DIVISOR;

		if (g3_in_frame() == 0) {
			gr_set_color(0,0,0);
			gr_rect(0, 0, gr_screen.max_w, yborder, false);
			gr_rect(0, gr_screen.max_h-yborder, gr_screen.max_w, yborder, false);
		} else {
			gr_reset_clip();
			gr_set_clip(0, yborder, gr_screen.max_w, gr_screen.max_h - (yborder*2), false );
		}
	}
}

void clip_frame_view() {
	if(!Time_compression_locked) {
		// Player is dead
		game_set_view_clip(flFrametime);

		// Cutscene bars
		bars_do_frame(flRealframetime);
	} else {
		// Player is dead
		game_set_view_clip(flRealframetime);

		// Cutscene bars
		bars_do_frame(flRealframetime);
	}
}

//WMC - This does stuff like fading in and out and subtitles. Special FX?
//Basically stuff you need rendered after everything else (including HUD)
void game_render_post_frame()
{
	float frametime = flFrametime;
	if(Time_compression_locked)
	{
		frametime = flRealframetime;
	}

	subtitles_do_frame(frametime);
	game_shade_frame(frametime);
	subtitles_do_frame_post_shaded(frametime);
}

#ifndef NDEBUG
#define DEBUG_GET_TIME(x)	{ x = timer_get_fixed_seconds(); }
#else
#define DEBUG_GET_TIME(x)
#endif

void game_frame(int paused)
{
#ifndef NDEBUG
	fix total_time1, total_time2;
	fix render2_time1=0, render2_time2=0;
	fix render3_time1=0, render3_time2=0;
	fix flip_time1=0, flip_time2=0;
	fix clear_time1=0, clear_time2=0;
#endif
	int actually_playing;

	//vec3d eye_pos;
	//matrix eye_orient;

#ifndef NDEBUG
	if (Framerate_delay) {
		int	start_time = timer_get_milliseconds();
		while (timer_get_milliseconds() < start_time + Framerate_delay)
			;
	}
#endif

#ifdef DEMO_SYSTEM
	demo_do_frame_start();
	if(Demo_error){
		mprintf(("Error (%d) while processing demo!\n", Demo_error));
		demo_close();
	}
#endif
	
		// start timing frame
		timing_frame_start();
	
		DEBUG_GET_TIME( total_time1 )

	if(paused)
	{
		// Reset the lights here or they just keep on increasing
		light_reset();
	}
	else
	{
		// var to hold which state we are in
		actually_playing = game_actually_playing();
	
		
		if ((!(Game_mode & GM_MULTIPLAYER)) || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER))) {
			if (!(Game_mode & GM_STANDALONE_SERVER)){
				Assert( OBJ_INDEX(Player_obj) >= 0 );
			}
		}
	
		if (Missiontime > Entry_delay_time){
			Pre_player_entry = 0;
		} else {
			; //nprintf(("AI", "Framecount = %i, time = %7.3f\n", Framecount, f2fl(Missiontime)));
		}
	
		//	Note: These are done even before the player enters, else buffers can overflow.
		if (! (Game_mode & GM_STANDALONE_SERVER)){
			radar_frame_init();
		}
	
		shield_frame_init();
	
		if ( !Pre_player_entry && actually_playing ) {		   		
			if (! (Game_mode & GM_STANDALONE_SERVER) ) {
	
				if( (!popup_running_state()) && (!popupdead_is_active()) ){
					game_process_keys();
					read_player_controls( Player_obj, flFrametime);
				}
				
				// if we're not the master, we may have to send the server-critical ship status button_info bits
				if (MULTIPLAYER_CLIENT && !(Net_player->flags & NETINFO_FLAG_OBSERVER)){
					multi_maybe_send_ship_status();
				}
			}
		}
	
		// Reset the whack stuff
		game_whack_reset();
	
		// These two lines must be outside of Pre_player_entry code,
		// otherwise too many lights are added.
		light_reset();
	
		if ((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state == NETGAME_STATE_SERVER_TRANSFER)){
			return;
		}
		
		
		game_simulation_frame(); 
		
		// if not actually in a game play state, then return.  This condition could only be true in 
		// a multiplayer game.
		if (!actually_playing ) {
			Assert( Game_mode & GM_MULTIPLAYER );
			return;
		}

	}

	if (!Pre_player_entry) {
		if (! (Game_mode & GM_STANDALONE_SERVER)) {
			DEBUG_GET_TIME( clear_time1 )
			// clear the screen to black
			gr_reset_clip();
			if ( (Game_detail_flags & DETAIL_FLAG_CLEAR) ) {
				gr_clear();
			}

			if(Player_obj)
				Script_system.SetHookObject("Player", Player_obj);
			else
				Script_system.RemHookVar("Player");

			DEBUG_GET_TIME( clear_time2 )
			DEBUG_GET_TIME( render3_time1 )
			
			camid cid = game_render_frame_setup();
			game_render_frame( cid );

			// save the eye position and orientation
			if ( Game_mode & GM_MULTIPLAYER ) {
				cid.getCamera()->get_info(&Net_player->s_info.eye_pos, &Net_player->s_info.eye_orient);
			}

			Scripting_didnt_draw_hud = 1;
			Script_system.SetHookObject("Self", Viewer_obj);
			if(Script_system.IsOverride(Script_hudhook) || Script_system.IsConditionOverride(CHA_HUDDRAW, Viewer_obj))
				Scripting_didnt_draw_hud = 0;
			Script_system.RemHookVar("Self");

			if(Scripting_didnt_draw_hud) {
				game_render_hud(cid);
			}
			HUD_reset_clip();

			if( (Game_detail_flags & DETAIL_FLAG_HUD) && (!(Game_mode & GM_MULTIPLAYER) || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER))) ) {
				anim_render_all(0, flFrametime);
			}

			Script_system.SetHookObject("Self", Viewer_obj);
			if (!hud_disabled_except_messages() && !(Viewer_mode & (VM_EXTERNAL | VM_DEAD_VIEW | VM_WARP_CHASE | VM_PADLOCK_ANY))) 
			{
				Script_system.RunBytecode(Script_hudhook);
				Script_system.RunCondition(CHA_HUDDRAW, '\0', NULL, Viewer_obj);
			}
			Script_system.RemHookVar("Self");
			
			// check to see if we should display the death died popup
			if(Game_mode & GM_DEAD_BLEW_UP){				
				if(Game_mode & GM_MULTIPLAYER){
					// catch the situation where we're supposed to be warping out on this transition
					if(Net_player->flags & NETINFO_FLAG_WARPING_OUT){
						multi_handle_sudden_mission_end();
						send_debrief_event();
					} else if((Player_died_popup_wait != -1) && (timestamp_elapsed(Player_died_popup_wait))){
						Player_died_popup_wait = -1;
						popupdead_start();
					}
				} else {
					if((Player_died_popup_wait != -1) && (timestamp_elapsed(Player_died_popup_wait))){
						Player_died_popup_wait = -1;
						popupdead_start();
					}
				}
			}

			//Cutscene bars
			clip_frame_view();

			DEBUG_GET_TIME( render3_time2 )
			DEBUG_GET_TIME( render2_time1 )

			gr_reset_clip();
			game_get_framerate();
			game_show_framerate();
			game_show_eye_pos(cid);

			game_show_time_left();

			gr_reset_clip();
			game_render_post_frame();

			game_tst_frame();

			DEBUG_GET_TIME( render2_time2 )

			// maybe render and process the dead popup
			game_maybe_do_dead_popup(flFrametime);

			// start timing frame
			timing_frame_stop();
			// timing_display(30, 10);			

			// If a regular popup is active, don't flip (popup code flips)
			if( !popup_running_state() ){
				DEBUG_GET_TIME( flip_time1 )
				game_flip_page_and_time_it();
				DEBUG_GET_TIME( flip_time2 )
			}

#ifndef NDEBUG
			game_maybe_dump_frame();			// used to dump pcx files for building trailers
#endif		
		} else {
			game_show_standalone_framerate();
		}
	}

	game_do_training_checks();
	asteroid_frame();

	// process lightning (nebula only)
	nebl_process();

	DEBUG_GET_TIME( total_time2 )

#ifndef NDEBUG
	// Got some timing numbers
	Timing_total = f2fl( total_time2 - total_time1 ) * 1000.0f;
	Timing_clear = f2fl( clear_time2 - clear_time1 ) * 1000.0f;
	Timing_render2 = f2fl( render2_time2- render2_time1 ) * 1000.0f;
	Timing_render3 = f2fl( render3_time2- render3_time1 ) * 1000.0f;
	Timing_flip = f2fl( flip_time2 - flip_time1 ) * 1000.0f;
#endif

#ifdef DEMO_SYSTEM
	demo_do_frame_end();
	if(Demo_error){
		mprintf(("Error (%d) while processing demo!\n", Demo_error));
		demo_close();
	}
#endif
}

#define	MAX_FRAMETIME	(F1_0/4)		// Frametime gets saturated at this.  Changed by MK on 11/1/97.
												//	Some bug was causing Frametime to always get saturated at 2.0 seconds after the player
												//	died.  This resulted in screwed up death sequences.

fix Last_time = 0;						// The absolute time of game at end of last frame (beginning of this frame)
fix Last_delta_time = 0;				// While game is paused, this keeps track of how much elapsed in the frame before paused.
static int timer_paused=0;
#if defined(TIMER_TEST) && !defined(NDEBUG)
static int stop_count,start_count;
static int time_stopped,time_started;
#endif
int saved_timestamp_ticker = -1;

void game_reset_time()
{
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state == NETGAME_STATE_SERVER_TRANSFER)){
		return ;
	}

	//	Last_time = timer_get_fixed_seconds();
	game_start_time();
	timestamp_reset();
	game_stop_time();
}

void game_stop_time()
{
	if (timer_paused==0) {
		fix time;
		time = timer_get_fixed_seconds();
		// Save how much time progressed so far in the frame so we can
		// use it when we unpause.
		Last_delta_time = time - Last_time;		

		//mprintf(("Last_time in game_stop_time = %7.3f\n", f2fl(Last_delta_time)));
		if (Last_delta_time < 0) {
			#if defined(TIMER_TEST) && !defined(NDEBUG)
			Int3();		//get Matt!!!!
			#endif
			Last_delta_time = 0;
		}
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		time_stopped = time;
		#endif

		// Stop the timer_tick stuff...
		// Normally, you should never access 'timestamp_ticker', consider this a low-level routine
		saved_timestamp_ticker = timestamp_ticker;
	}
	timer_paused++;

	#if defined(TIMER_TEST) && !defined(NDEBUG)
	stop_count++;
	#endif
}

void game_start_time()
{
	timer_paused--;
	Assert(timer_paused >= 0);
	if (timer_paused==0) {
		fix time;
		time = timer_get_fixed_seconds();
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		if (Last_time < 0)
			Int3();		//get Matt!!!!
		}
		#endif
		// Take current time, and set it backwards to account for time	
		// that the frame already executed, so that timer_get_fixed_seconds() - Last_time
		// will be correct when it goes to calculate the frametime next
		// frame.
		Last_time = time - Last_delta_time;		
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		time_started = time;
		#endif

		// Restore the timer_tick stuff...
		// Normally, you should never access 'timestamp_ticker', consider this a low-level routine
		Assert( saved_timestamp_ticker > -1 );		// Called out of order, get JAS
		timestamp_ticker = saved_timestamp_ticker;
		saved_timestamp_ticker = -1;
	}

	#if defined(TIMER_TEST) && !defined(NDEBUG)
	start_count++;
	#endif
}

void lock_time_compression(bool is_locked)
{
	Time_compression_locked = is_locked;
}

void change_time_compression(float multiplier)
{
	fix modified = F1_0;

	modified = fl2f( f2fl(Game_time_compression) * multiplier );

	Desired_time_compression = Game_time_compression = modified;
	Time_compression_change_rate = 0;
}

void set_time_compression(float multiplier, float change_time)
{
	if(change_time <= 0.0f)
	{
		Game_time_compression = Desired_time_compression = fl2f(multiplier);
		Time_compression_change_rate = 0;
		return;
	}

	Desired_time_compression = fl2f(multiplier);
	Time_compression_change_rate = fl2f( f2fl(Desired_time_compression - Game_time_compression) / change_time );
}

void game_set_frametime(int state)
{
	fix thistime;
	float frame_cap_diff;

	thistime = timer_get_fixed_seconds();

	if ( Last_time == 0 )	
		Frametime = F1_0 / 30;
	else
		Frametime = thistime - Last_time;

//	Frametime = F1_0 / 30;

#ifndef NDEBUG
	fix	debug_frametime = Frametime;	//	Just used to display frametime.
#endif

	//	If player hasn't entered mission yet, make frame take 1/4 second.
	if ((Pre_player_entry) && (state == GS_STATE_GAME_PLAY))
		Frametime = F1_0/4;
#ifndef NDEBUG
	else if ((Debug_dump_frames) && (state == GS_STATE_GAME_PLAY)) {				// note link to above if!!!!!
	
		fix frame_speed = F1_0 / Debug_dump_frames;

		if (Frametime > frame_speed ){
			nprintf(("warning","slow frame: %x\n",Frametime));
		} else {			
			do {
				thistime = timer_get_fixed_seconds();
				Frametime = thistime - Last_time;
			} while (Frametime < frame_speed );			
		}
		Frametime = frame_speed;
	}
#endif

	Assertion( Framerate_cap > 0, "Framerate cap %d is too low. Needs to be a positive, non-zero number", Framerate_cap );

	// Cap the framerate so it doesn't get too high.
	if (!Cmdline_NoFPSCap)
	{
		fix cap;

		cap = F1_0/Framerate_cap;
		if (Frametime < cap) {
			thistime = cap - Frametime;
//  			mprintf(("Sleeping for %6.3f seconds.\n", f2fl(thistime)));
			Sleep( DWORD(f2fl(thistime) * 1000.0f) );
			Frametime = cap;
			thistime = timer_get_fixed_seconds();
		}
	}

	if((Game_mode & GM_STANDALONE_SERVER) && 
		(f2fl(Frametime) < ((float)1.0/(float)Multi_options_g.std_framecap))){

		frame_cap_diff = ((float)1.0/(float)Multi_options_g.std_framecap) - f2fl(Frametime);		
		Sleep((DWORD)(frame_cap_diff*1000)); 				
		
		thistime += fl2f((frame_cap_diff));		

		Frametime = thistime - Last_time;
   }

	// If framerate is too low, cap it.
	if (Frametime > MAX_FRAMETIME)	{
#ifndef NDEBUG
		mprintf(("Frame %2i too long!!: frametime = %.3f (%.3f)\n", Framecount, f2fl(Frametime), f2fl(debug_frametime)));
#endif
		Frametime = MAX_FRAMETIME;
	}

	flRealframetime = f2fl(Frametime);

	//Handle changes in time compression
	if(Game_time_compression != Desired_time_compression)
	{
		bool ascending = Desired_time_compression > Game_time_compression;
		if(Time_compression_change_rate)
			Game_time_compression += fixmul(Time_compression_change_rate, Frametime);
		if((ascending && Game_time_compression > Desired_time_compression)
			|| (!ascending && Game_time_compression < Desired_time_compression))
			Game_time_compression = Desired_time_compression;
	}

	Frametime = fixmul(Frametime, Game_time_compression);

    if (Frametime <= 0)
    {
        // If the Frametime is zero or below due to Game_time_compression, set
        // the Frametime to 1 (1/65536 of a second).
        Frametime = 1;
    }

	Last_time = thistime;
	//mprintf(("Frame %i, Last_time = %7.3f\n", Framecount, f2fl(Last_time)));

	flFrametime = f2fl(Frametime);
	//if(!(Game_mode & GM_PLAYING_DEMO)){
	timestamp_inc(flFrametime);

	// wrap overall frametime if needed
	if ( FrametimeOverall > (INT_MAX - F1_0) )
		FrametimeOverall = 0;

	FrametimeOverall += Frametime;

/*	if ((Framecount > 0) && (Framecount < 10)) {
		mprintf(("Frame %2i: frametime = %.3f (%.3f)\n", Framecount, f2fl(Frametime), f2fl(debug_frametime)));
	}
*/
}

fix game_get_overall_frametime()
{
	return FrametimeOverall;
}

// This is called from game_do_frame(), and from navmap_do_frame() 
void game_update_missiontime()
{
	// TODO JAS: Put in if and move this into game_set_frametime, 
	// fix navmap to call game_stop/start_time
	//if ( !timer_paused )	
		Missiontime += Frametime;
}

void game_do_frame()
{	
	game_set_frametime(GS_STATE_GAME_PLAY);
	game_update_missiontime();

//	if (Player_ship->flags & SF_DYING)
//		flFrametime /= 15.0;

	if (Game_mode & GM_STANDALONE_SERVER) {
		std_multi_set_standalone_missiontime(f2fl(Missiontime));
	}

	if ( game_single_step && (last_single_step == game_single_step) ) {
		os_set_title( NOX("SINGLE STEP MODE (Pause exits, any other key steps)") );
		while( key_checkch() == 0 )
			os_sleep(10);
		os_set_title( XSTR( "FreeSpace", 171) );
  		Last_time = timer_get_fixed_seconds();
	}

	last_single_step = game_single_step;

	if ((gameseq_get_state() == GS_STATE_GAME_PLAY) && Use_mouse_to_fly){
		Keep_mouse_centered = 1;  // force mouse to center of our window (so we don't hit movement limits)
	}
	game_frame();

	Keep_mouse_centered = 0;
	monitor_update();			// Update monitor variables
}

void multi_maybe_do_frame()
{
	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_IN_MISSION) && !Multi_pause_status){
		game_do_frame(); 
	}
}

int Joymouse_button_status = 0;

// Flush all input devices
void game_flush()
{
	key_flush();
	mouse_flush();
	joy_flush();
	snazzy_flush();

	Joymouse_button_status = 0;

	//mprintf(("Game flush!\n" ));
}

// function for multiplayer only which calls game_do_state_common() when running the
// debug console
void game_do_dc_networking()
{
	Assert( Game_mode & GM_MULTIPLAYER );

	game_do_state_common( gameseq_get_state() );
}

// Call this whenever in a loop, or when you need to check for a keystroke.
int game_check_key()
{
	int k;

	k = game_poll();

	// convert keypad enter to normal enter
	if ((k & KEY_MASK) == KEY_PADENTER)
		k = (k & ~KEY_MASK) | KEY_ENTER;

	return k;
}

// same as game_check_key(), except this is used while actually in the game.  Since there
// generally are differences between game control keys and general UI keys, makes sense to
// have seperate functions for each case.  If you are not checking a game control while in a
// mission, you should probably be using game_check_key() instead.
int game_poll()
{
	int k, state;

	if (!os_foreground()) {		
		game_stop_time();
//		os_sleep(100);
		os_sleep(1);
		game_start_time();

		// If we're in a single player game, pause it.
		if (!(Game_mode & GM_MULTIPLAYER)){
			if ( (gameseq_get_state() == GS_STATE_GAME_PLAY) && (!popup_active()) && (!popupdead_is_active()) )	{
				game_process_pause_key();
			}
		}
	}

	k = key_inkey();

	// Move the mouse cursor with the joystick.
	if (os_foreground() && (!Mouse_hidden) && (Use_joy_mouse) )	{
		// Move the mouse cursor with the joystick
		int mx, my, dx, dy;
		int jx, jy, jz, jr;

		joy_get_pos( &jx, &jy, &jz, &jr );

		dx = fl2i(f2fl(jx)*flFrametime*500.0f);
		dy = fl2i(f2fl(jy)*flFrametime*500.0f);

		if ( dx || dy )	{
			mouse_get_real_pos( &mx, &my );
			mouse_set_pos( mx+dx, my+dy );
		}

		int j, m;
		j = joy_down(0);
		m = mouse_down(MOUSE_LEFT_BUTTON);

		if ( j != Joymouse_button_status )	{
			//mprintf(( "Joy went from %d to %d, mouse is %d\n", Joymouse_button_status, j, m ));
			Joymouse_button_status = j;
			if ( j && (!m) )	{
				mouse_mark_button( MOUSE_LEFT_BUTTON, 1 );
			} else if ( (!j) && (m) )	{
				mouse_mark_button( MOUSE_LEFT_BUTTON, 0 );
			}
		}
	}

	// if we should be ignoring keys because of some multiplayer situations
	if((Game_mode & GM_MULTIPLAYER) && multi_ignore_controls(k)){
		return 0;
	}

	state = gameseq_get_state();

	// If a popup is running, don't process all the Fn keys
	if( popup_active()) {
		if (state != GS_STATE_DEATH_BLEW_UP) {
			return k;
		}
	}

//	if ( k ) nprintf(( "General", "Key = %x\n", k ));

	switch (k) {
		case KEY_DEBUGGED + KEY_BACKSP:
			if(!(Game_mode & GM_MULTIPLAYER))
			{
				gameseq_post_event(GS_EVENT_LAB);
				k = 0;
			}
			break;

		case KEY_F1:
			launch_context_help();
			k = 0;
			break;

		case KEY_F2:
//			if (state != GS_STATE_INITIAL_PLAYER_SELECT) {

			// don't allow f2 while warping out in multiplayer	
			if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_WARPING_OUT)){
				break;
			}

			switch (state) {
				case GS_STATE_INITIAL_PLAYER_SELECT:
				case GS_STATE_OPTIONS_MENU:
				case GS_STATE_HUD_CONFIG:
				case GS_STATE_CONTROL_CONFIG:
				case GS_STATE_DEATH_DIED:
//				case GS_STATE_DEATH_BLEW_UP:	// 	DEATH_BLEW_UP might be okay but do not comment out DEATH_DIED as otherwise no clean up is performed on the dead ship
				case GS_STATE_VIEW_MEDALS:
					break;

				default:
					gameseq_post_event(GS_EVENT_OPTIONS_MENU);
					k = 0;
					break;
			}

			break;

			// hotkey selection screen -- only valid from briefing and beyond.
		case KEY_F3:	
			if ( (state == GS_STATE_TEAM_SELECT) || (state == GS_STATE_BRIEFING) || (state == GS_STATE_SHIP_SELECT) || (state == GS_STATE_WEAPON_SELECT) || (state == GS_STATE_GAME_PLAY) || (state == GS_STATE_GAME_PAUSED) ) {
				gameseq_post_event( GS_EVENT_HOTKEY_SCREEN );
				k = 0;
			}
			break;

		case KEY_DEBUGGED + KEY_F3:
			gameseq_post_event( GS_EVENT_TOGGLE_FULLSCREEN );
			break;

		case KEY_F4:
			if(Game_mode & GM_MULTIPLAYER){
				if((state == GS_STATE_GAME_PLAY) || (state == GS_STATE_MULTI_PAUSED)){
					gameseq_post_event( GS_EVENT_MISSION_LOG_SCROLLBACK );
					k = 0;
				} 
			} else {
				if ((state == GS_STATE_GAME_PLAY) || (state == GS_STATE_DEATH_DIED) || (state == GS_STATE_DEATH_BLEW_UP) || (state == GS_STATE_GAME_PAUSED) ) {
					gameseq_post_event( GS_EVENT_MISSION_LOG_SCROLLBACK );
					k = 0;
				}
			}
			break;

		case KEY_ESC | KEY_SHIFTED:
			// make sure to quit properly out of multiplayer
			if(Game_mode & GM_MULTIPLAYER){
				multi_quit_game(PROMPT_NONE);
			}

			gameseq_post_event( GS_EVENT_QUIT_GAME );
			k = 0;

			break;

		case KEY_DEBUGGED + KEY_P:			
			break;			

		case KEY_PRINT_SCRN: 
			{
				static int counter = os_config_read_uint(NULL, "ScreenshotNum", 0);
				char tmp_name[MAX_FILENAME_LEN];

				game_stop_time();

				// we could probably go with .3 here for 1,000 shots but people really need to clean out
				// their directories better than that so it's 100 for now.
				sprintf( tmp_name, NOX("screen%.4i"), counter );
				counter++;

				// we've got two character precision so we can only have 100 shots at a time, reset if needed
				//Now we have four digit precision :) -WMC
				if (counter > 9999)
				{
					//This should pop up a dialogue or something ingame.
					Warning(LOCATION, "Screenshot count has reached max of 9999. Resetting to 0.");
					counter = 0;
				}

				mprintf(( "Dumping screen to '%s'\n", tmp_name ));
				gr_print_screen(tmp_name);

				game_start_time();
				os_config_write_uint(NULL, "ScreenshotNum", counter);
			}

			k = 0;
			break;

		case KEY_SHIFTED | KEY_ENTER: {

#if !defined(NDEBUG)

			if ( Game_mode & GM_NORMAL ){
				game_stop_time();
			}

			// if we're in multiplayer mode, do some special networking
			if(Game_mode & GM_MULTIPLAYER){
				debug_console(game_do_dc_networking);
			} else {				
				debug_console();
			}

			game_flush();

			if ( Game_mode & GM_NORMAL )
				game_start_time();

#endif

			break;
		}
	}

	return k;
}

void os_close()
{
	gameseq_post_event(GS_EVENT_QUIT_GAME);
}

void end_demo_campaign_do()
{
	// drop into main hall
	gameseq_post_event( GS_EVENT_MAIN_MENU );
}

// All code to process events.   This is the only place
// that you should change the state of the game.
void game_process_event( int current_state, int event )
{
	mprintf(("Got event %s (%d) in state %s (%d)\n", GS_event_text[event], event, GS_state_text[current_state], current_state));

	switch (event) {
		case GS_EVENT_SIMULATOR_ROOM:
			gameseq_set_state(GS_STATE_SIMULATOR_ROOM);
			break;

		case GS_EVENT_MAIN_MENU:
			gameseq_set_state(GS_STATE_MAIN_MENU);		
			break;

		case GS_EVENT_OPTIONS_MENU:
			gameseq_push_state( GS_STATE_OPTIONS_MENU );
			break;

		case GS_EVENT_BARRACKS_MENU:
			gameseq_set_state(GS_STATE_BARRACKS_MENU);		
			break;

		case GS_EVENT_TECH_MENU:
			gameseq_set_state(GS_STATE_TECH_MENU);		
			break;
		case GS_EVENT_LAB:
			gameseq_push_state(GS_STATE_LAB);
			break;
		case GS_EVENT_TRAINING_MENU:
			gameseq_set_state(GS_STATE_TRAINING_MENU);		
			break;

		case GS_EVENT_START_GAME:
			Select_default_ship = 0;			
			gameseq_set_state(GS_STATE_START_GAME);
			break;

		case GS_EVENT_START_GAME_QUICK:
			Select_default_ship = 1;
			gameseq_post_event(GS_EVENT_ENTER_GAME);
			break;

		case GS_EVENT_CMD_BRIEF:
			gameseq_set_state(GS_STATE_CMD_BRIEF);
			break;

		case GS_EVENT_RED_ALERT:
			gameseq_set_state(GS_STATE_RED_ALERT);
			break;

		case GS_EVENT_START_BRIEFING:
			gameseq_set_state(GS_STATE_BRIEFING);		
			break;

		case GS_EVENT_DEBRIEF:
			gameseq_set_state(GS_STATE_DEBRIEF);
			break;

		case GS_EVENT_SHIP_SELECTION:
			gameseq_set_state( GS_STATE_SHIP_SELECT );
			break;

		case GS_EVENT_WEAPON_SELECTION:
			gameseq_set_state( GS_STATE_WEAPON_SELECT );
			break;

		case GS_EVENT_ENTER_GAME:		
#ifdef DEMO_SYSTEM
			// maybe start recording a demo
			if(Demo_make){
				demo_start_record("test.fsd");
			}
#endif

			if (Game_mode & GM_MULTIPLAYER) {
				// if we're respawning, make sure we change the view mode so that the hud shows up
				if (current_state == GS_STATE_DEATH_BLEW_UP) {
					Viewer_mode = 0;
				}

				gameseq_set_state(GS_STATE_GAME_PLAY);
			} else {
				gameseq_set_state(GS_STATE_GAME_PLAY, 1);
			}

			// clear multiplayer button info			
			extern button_info Multi_ship_status_bi;
			memset(&Multi_ship_status_bi, 0, sizeof(button_info));

			Script_system.RunCondition(CHA_GAMEPLAYSTART);

			Start_time = f2fl(timer_get_approx_seconds());
			//Framecount = 0;
			mprintf(("Entering game at time = %7.3f\n", Start_time));
			break;

		case GS_EVENT_END_GAME:
			if ( (current_state == GS_STATE_GAME_PLAY) || (current_state == GS_STATE_DEATH_DIED) ||
				(current_state == GS_STATE_DEATH_BLEW_UP) ||	(current_state == GS_STATE_DEBRIEF) || (current_state == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)) {
					gameseq_set_state(GS_STATE_MAIN_MENU);

			} else
				Int3();

			break;

		case GS_EVENT_QUIT_GAME:
			main_hall_stop_music();
			main_hall_stop_ambient();
			gameseq_set_state(GS_STATE_QUIT_GAME);
			break;

		case GS_EVENT_GAMEPLAY_HELP:
			gameseq_push_state( GS_STATE_GAMEPLAY_HELP );
			break;

		case GS_EVENT_PAUSE_GAME:
			gameseq_push_state(GS_STATE_GAME_PAUSED);
			break;

		case GS_EVENT_DEBUG_PAUSE_GAME:
			gameseq_push_state(GS_STATE_DEBUG_PAUSED);
			break;

		case GS_EVENT_TRAINING_PAUSE:
			gameseq_push_state(GS_STATE_TRAINING_PAUSED);
			break;

		case GS_EVENT_PREVIOUS_STATE:
			gameseq_pop_state();
			break;

		case GS_EVENT_LOAD_MISSION_MENU:
			gameseq_set_state(GS_STATE_LOAD_MISSION_MENU);
			break;

		case GS_EVENT_MISSION_LOG_SCROLLBACK:
			gameseq_push_state( GS_STATE_MISSION_LOG_SCROLLBACK );
			break;

		case GS_EVENT_HUD_CONFIG:
			gameseq_push_state( GS_STATE_HUD_CONFIG );
			break;

		case GS_EVENT_CONTROL_CONFIG:
			gameseq_push_state( GS_STATE_CONTROL_CONFIG );
			break;	

		case GS_EVENT_DEATH_DIED:
			gameseq_set_state( GS_STATE_DEATH_DIED );
			break;

		case GS_EVENT_DEATH_BLEW_UP:
			if (  current_state == GS_STATE_DEATH_DIED )	{
				gameseq_set_state( GS_STATE_DEATH_BLEW_UP );
				event_music_player_death();	
			} else {
				mprintf(( "Ignoring GS_EVENT_DEATH_BLEW_UP because we're in state %d\n", current_state ));
			}
			break;

		case GS_EVENT_NEW_CAMPAIGN:
			if (!mission_load_up_campaign()){
				readyroom_continue_campaign();
			}
			break;

		case GS_EVENT_CAMPAIGN_CHEAT:
			if (!mission_load_up_campaign()){
				readyroom_continue_campaign();
			}
			break;

		case GS_EVENT_CAMPAIGN_ROOM:
			gameseq_set_state(GS_STATE_CAMPAIGN_ROOM);
			break;

		case GS_EVENT_CREDITS:
			gameseq_set_state( GS_STATE_CREDITS );
			break;

		case GS_EVENT_VIEW_MEDALS:
			gameseq_push_state( GS_STATE_VIEW_MEDALS );
			break;

		case GS_EVENT_SHOW_GOALS:
			gameseq_push_state( GS_STATE_SHOW_GOALS );	// use push_state() since we might get to this screen through a variety of states
			break;

		case GS_EVENT_HOTKEY_SCREEN:
			gameseq_push_state( GS_STATE_HOTKEY_SCREEN );	// use push_state() since we might get to this screen through a variety of states
			break;
		
	// multiplayer stuff follow these comments
		case GS_EVENT_PXO:
			gameseq_set_state( GS_STATE_PXO );
			break;

		case GS_EVENT_PXO_HELP:
			gameseq_set_state( GS_STATE_PXO_HELP );
			break;

		case GS_EVENT_MULTI_JOIN_GAME:
			gameseq_set_state( GS_STATE_MULTI_JOIN_GAME );
			break;

		case GS_EVENT_MULTI_HOST_SETUP:
			gameseq_set_state( GS_STATE_MULTI_HOST_SETUP );
			break;

		case GS_EVENT_MULTI_CLIENT_SETUP:
			gameseq_set_state( GS_STATE_MULTI_CLIENT_SETUP );
			break;

  		case GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN:
			gameseq_set_state(GS_STATE_VIEW_CUTSCENES);
			break;

		case GS_EVENT_MULTI_STD_WAIT:
			gameseq_set_state( GS_STATE_MULTI_STD_WAIT );
			break;

		case GS_EVENT_STANDALONE_MAIN:
			gameseq_set_state( GS_STATE_STANDALONE_MAIN );
			break;   

		case GS_EVENT_MULTI_PAUSE:
			gameseq_push_state( GS_STATE_MULTI_PAUSED );
			break;			

		case GS_EVENT_INGAME_PRE_JOIN:
			gameseq_set_state( GS_STATE_INGAME_PRE_JOIN );
			break;
		
		case GS_EVENT_EVENT_DEBUG:
			gameseq_push_state(GS_STATE_EVENT_DEBUG);
			break;

		// Start a warpout where player automatically goes 70 no matter what
		// and can't cancel out of it.
		case GS_EVENT_PLAYER_WARPOUT_START_FORCED:
			Warpout_forced = 1;							// If non-zero, bash the player to speed and go through effect

			// Same code as in GS_EVENT_PLAYER_WARPOUT_START only ignores current mode
			Player->saved_viewer_mode = Viewer_mode;
			Player->control_mode = PCM_WARPOUT_STAGE1;
			Warpout_sound = snd_play(&Snds[SND_PLAYER_WARP_OUT]);
			Warpout_time = 0.0f;			// Start timer!
			break;

		case GS_EVENT_PLAYER_WARPOUT_START:
			if ( Player->control_mode != PCM_NORMAL )	{
				mprintf(( "Player isn't in normal mode; cannot warp out.\n" ));
			} else {
				Player->saved_viewer_mode = Viewer_mode;
				Player->control_mode = PCM_WARPOUT_STAGE1;
				Warpout_sound = snd_play(&Snds[SND_PLAYER_WARP_OUT]);
				Warpout_time = 0.0f;			// Start timer!
				Warpout_forced = 0;				// If non-zero, bash the player to speed and go through effect
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_STOP:
			if ( Player->control_mode != PCM_NORMAL )	{
				if ( !Warpout_forced )	{		// cannot cancel forced warpout
					Player->control_mode = PCM_NORMAL;
					Viewer_mode = Player->saved_viewer_mode;
					hud_subspace_notify_abort();
					mprintf(( "Player put back to normal mode.\n" ));
					if ( Warpout_sound > -1 )	{
						snd_stop( Warpout_sound );
						Warpout_sound = -1;
					}
				}
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE_STAGE1:		// player ship got up to speed
			if ( Player->control_mode != PCM_WARPOUT_STAGE1 )	{
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
				mprintf(( "Player put back to normal mode, because of invalid sequence in stage1.\n" ));
			} else {
				mprintf(( "Hit target speed.  Starting warp effect and moving to stage 2!\n" ));
				shipfx_warpout_start( Player_obj );
				Player->control_mode = PCM_WARPOUT_STAGE2;

				if (!(The_mission.ai_profile->flags2 & AIPF2_NO_WARP_CAMERA)) {
					Player->saved_viewer_mode = Viewer_mode;
					Viewer_mode |= VM_WARP_CHASE;
					Warp_camera = warp_camera(Player_obj);
				}
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2:		// player ship got into the warp effect
			if ( Player->control_mode != PCM_WARPOUT_STAGE2 )	{
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
				mprintf(( "Player put back to normal mode, because of invalid sequence in stage2.\n" ));
			} else {
				mprintf(( "Hit warp effect.  Moving to stage 3!\n" ));
				Player->control_mode = PCM_WARPOUT_STAGE3;
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE:	// player ship got through the warp effect
			mprintf(( "Player warped out.  Going to debriefing!\n" ));
			Player->control_mode = PCM_NORMAL;
			Viewer_mode = Player->saved_viewer_mode;
			Warpout_sound = -1;

			send_debrief_event();
			break;

		case GS_EVENT_STANDALONE_POSTGAME:
			gameseq_set_state(GS_STATE_STANDALONE_POSTGAME);
			break;

		case GS_EVENT_INITIAL_PLAYER_SELECT:
			gameseq_set_state(GS_STATE_INITIAL_PLAYER_SELECT);
			break;

		case GS_EVENT_GAME_INIT:
			// see if the command line option has been set to use the last pilot, and act acoordingly
			if( player_select_get_last_pilot() ) {	
				// always enter the main menu -- do the automatic network startup stuff elsewhere
				// so that we still have valid checks for networking modes, etc.
				gameseq_set_state(GS_STATE_MAIN_MENU);
			} else {
				gameseq_set_state(GS_STATE_INITIAL_PLAYER_SELECT);
			}
			break;

		case GS_EVENT_MULTI_MISSION_SYNC:
			gameseq_set_state(GS_STATE_MULTI_MISSION_SYNC);
			break;		

		case GS_EVENT_MULTI_START_GAME:
			gameseq_set_state(GS_STATE_MULTI_START_GAME);
			break;

		case GS_EVENT_MULTI_HOST_OPTIONS:
			gameseq_set_state(GS_STATE_MULTI_HOST_OPTIONS);
			break;

		case GS_EVENT_MULTI_DOGFIGHT_DEBRIEF:
			gameseq_set_state(GS_STATE_MULTI_DOGFIGHT_DEBRIEF);
			break;

		case GS_EVENT_TEAM_SELECT:
			gameseq_set_state(GS_STATE_TEAM_SELECT);
			break;

		case GS_EVENT_END_CAMPAIGN:			
			gameseq_set_state(GS_STATE_END_OF_CAMPAIGN);
			break;		

		case GS_EVENT_END_DEMO:
			gameseq_set_state(GS_STATE_END_DEMO);
			break;

		case GS_EVENT_LOOP_BRIEF:
			gameseq_set_state(GS_STATE_LOOP_BRIEF);
			break;

		case GS_EVENT_FICTION_VIEWER:
			gameseq_set_state(GS_STATE_FICTION_VIEWER);
			break;

		case GS_EVENT_SCRIPTING:
			gameseq_set_state(GS_STATE_SCRIPTING);
			break;

		default:
			Int3();
			break;
	}
}

// Called when a state is being left.
// The current state is still at old_state, but as soon as
// this function leaves, then the current state will become
// new state.     You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.
void game_leave_state( int old_state, int new_state )
{
	int end_mission = 1;

	switch (new_state) {
		case GS_STATE_GAME_PAUSED:
		case GS_STATE_DEBUG_PAUSED:
		case GS_STATE_OPTIONS_MENU:
		case GS_STATE_CONTROL_CONFIG:		
		case GS_STATE_MISSION_LOG_SCROLLBACK:
		case GS_STATE_DEATH_DIED:
		case GS_STATE_SHOW_GOALS:
		case GS_STATE_HOTKEY_SCREEN:		
		case GS_STATE_MULTI_PAUSED:
		case GS_STATE_TRAINING_PAUSED:
		case GS_STATE_EVENT_DEBUG:				
		case GS_STATE_GAMEPLAY_HELP:
		case GS_STATE_LAB:
			end_mission = 0;  // these events shouldn't end a mission
			break;
	}

	//WMC - Scripting override
	/*
	if(GS_state_hooks[old_state].IsValid() && Script_system.IsOverride(GS_state_hooks[old_state])) {
		return;
	}
	*/

	if(Script_system.IsConditionOverride(CHA_ONSTATEEND)) {
		Script_system.RunCondition(CHA_ONSTATEEND);
		return;
	}

	//WMC - Clear scripting bitmaps
	Script_system.UnloadImages();

	switch (old_state) {
		case GS_STATE_BRIEFING:
			brief_stop_voices();
			if ( (new_state != GS_STATE_OPTIONS_MENU) && (new_state != GS_STATE_WEAPON_SELECT)
				  && (new_state != GS_STATE_SHIP_SELECT) && (new_state != GS_STATE_HOTKEY_SCREEN)
				  && (new_state != GS_STATE_TEAM_SELECT) && (new_state != GS_STATE_MULTI_MISSION_SYNC)){
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			
			// COMMAND LINE OPTION
			if (Cmdline_multi_stream_chat_to_file){
				cfwrite_string(NOX("-------------------------------------------\n"),Multi_chat_stream);
				cfclose(Multi_chat_stream);
			}
			break;

		case GS_STATE_DEBRIEF:
			if ( (new_state != GS_STATE_VIEW_MEDALS) && (new_state != GS_STATE_OPTIONS_MENU) ) {
				debrief_close();				
				fsspeech_stop();
			}
			break;

		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			multi_df_debrief_close();
			break;

		case GS_STATE_LOAD_MISSION_MENU:
			mission_load_menu_close();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			sim_room_close();
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			campaign_room_close();
			break;

		case GS_STATE_CMD_BRIEF:
			if (new_state == GS_STATE_OPTIONS_MENU) {
				cmd_brief_hold();

			} else {
				cmd_brief_close();
				if (new_state == GS_STATE_MAIN_MENU)
					freespace_stop_mission();	
					common_select_close();
			}
			break;

		case GS_STATE_RED_ALERT:
			red_alert_close();
			common_select_close();
			if (new_state == GS_STATE_MAIN_MENU) {
				freespace_stop_mission();
			}
			break;

		case GS_STATE_SHIP_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_WEAPON_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_TEAM_SELECT  && (new_state != GS_STATE_MULTI_MISSION_SYNC)) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			break;

		case GS_STATE_WEAPON_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_SHIP_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_TEAM_SELECT && (new_state != GS_STATE_MULTI_MISSION_SYNC)) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			break;

		case GS_STATE_TEAM_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_SHIP_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_WEAPON_SELECT && (new_state != GS_STATE_MULTI_MISSION_SYNC)) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}					
			break;

		case GS_STATE_MAIN_MENU:
			main_hall_close();
			break;

		case GS_STATE_OPTIONS_MENU:
			//game_start_time();
			if(new_state == GS_STATE_MULTI_JOIN_GAME){
				multi_join_clear_game_list();
			}
			options_menu_close();
			break;

		case GS_STATE_BARRACKS_MENU:
			if(new_state != GS_STATE_VIEW_MEDALS){
				barracks_close();
			}
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			hud_scrollback_close();
			break;

		case GS_STATE_TRAINING_MENU:
			training_menu_close();
			break;

		case GS_STATE_GAME_PLAY:
			if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
				player_save_target_and_weapon_link_prefs();
				game_stop_looped_sounds();
			}

			sound_env_disable();
			joy_ff_stop_effects();

			// stop game time under certain conditions
			if ( end_mission || (Game_mode & GM_NORMAL) || ((Game_mode & GM_MULTIPLAYER) && (new_state == GS_STATE_MULTI_PAUSED)) ){
				game_stop_time();
			}

			if (end_mission) {
			// shut down any recording or playing demos
#ifdef DEMO_SYSTEM
				demo_close();
#endif

				// when in multiplayer and going back to the main menu, send a leave game packet
				// right away (before calling stop mission).  stop_mission was taking to long to
				// close mission down and I want people to get notified ASAP.
				if ( (Game_mode & GM_MULTIPLAYER) && (new_state == GS_STATE_MAIN_MENU) ){
					multi_quit_game(PROMPT_NONE);
				}
				snd_aav_init();

				freespace_stop_mission();			
			}
			break;

		case GS_STATE_TECH_MENU:
			techroom_close();
			break;

		case GS_STATE_TRAINING_PAUSED:
			Training_num_lines = 0;
			// fall through to GS_STATE_GAME_PAUSED

		case GS_STATE_GAME_PAUSED:
			game_start_time();
			if ( end_mission ) {
				pause_close();
			}
			break;

		case GS_STATE_DEBUG_PAUSED:
			#ifndef NDEBUG
				game_start_time();
				pause_debug_close();
			#endif
			break;

		case GS_STATE_HUD_CONFIG:
			hud_config_close();
			break;

		case GS_STATE_PXO:
			if (new_state != GS_STATE_PXO_HELP) {
				multi_pxo_close();
			}
			break;

		case GS_STATE_PXO_HELP:
			multi_pxo_help_close();
			break;

		// join/start a game
		case GS_STATE_MULTI_JOIN_GAME:
			if(new_state != GS_STATE_OPTIONS_MENU){
				multi_join_game_close();
			}
			break;

		case GS_STATE_MULTI_HOST_SETUP:
		case GS_STATE_MULTI_CLIENT_SETUP:
			// if this is just the host going into the options screen, don't do anything
			if((new_state == GS_STATE_MULTI_HOST_OPTIONS) || (new_state == GS_STATE_OPTIONS_MENU)){
				break;
			}

			// close down the proper state
			if(old_state == GS_STATE_MULTI_HOST_SETUP){
				multi_create_game_close();
			} else {
				multi_game_client_setup_close();
			}

			// COMMAND LINE OPTION
			if (Cmdline_multi_stream_chat_to_file){
				if( (new_state != GS_STATE_TEAM_SELECT) && (Multi_chat_stream!=NULL) ) {
					cfwrite_string(NOX("-------------------------------------------\n"),Multi_chat_stream);
					cfclose(Multi_chat_stream);
				}
			}			
			break;

		case GS_STATE_CONTROL_CONFIG:
			control_config_close();
			break;

		case GS_STATE_DEATH_DIED:
			Game_mode &= ~GM_DEAD_DIED;

			if ( !(Game_mode & GM_MULTIPLAYER) ) {
				if ( end_mission && (new_state == GS_STATE_DEBRIEF) ) {
					freespace_stop_mission();
				}
			} else {
				// early end while respawning or blowing up in a multiplayer game
				if ( (new_state == GS_STATE_DEBRIEF) || (new_state == GS_STATE_MULTI_DOGFIGHT_DEBRIEF) ) {
					game_stop_time();
					freespace_stop_mission();
				}
			}
			break;

		case GS_STATE_DEATH_BLEW_UP:
			Game_mode &= ~GM_DEAD_BLEW_UP;

			// for single player, we might reload mission, etc.  For multiplayer, look at my new state
			// to determine if I should do anything.
			if ( !(Game_mode & GM_MULTIPLAYER) ) {
				if ( end_mission ){
					freespace_stop_mission();
				}
			} else {
				// if we are not respawing as an observer or as a player, our new state will not
				// be gameplay state.
				if ( (new_state != GS_STATE_GAME_PLAY) && (new_state != GS_STATE_MULTI_PAUSED) ) {
					game_stop_time();									// hasn't been called yet!!
					freespace_stop_mission();
				}
			}
			break;


		case GS_STATE_CREDITS:
			credits_close();
			break;

		case GS_STATE_VIEW_MEDALS:
			medal_main_close();
			break;

		case GS_STATE_SHOW_GOALS:
			mission_show_goals_close();
			break;

		case GS_STATE_HOTKEY_SCREEN:
			if ( new_state != GS_STATE_OPTIONS_MENU ) {
				mission_hotkey_close();
			}
			break;

		case GS_STATE_MULTI_MISSION_SYNC:
			common_select_close();

			// if we're moving into the options menu we don't need to do anything else
			if(new_state == GS_STATE_OPTIONS_MENU){	
				break;
			}

			Assert( Game_mode & GM_MULTIPLAYER );
			multi_sync_close();
			if ( new_state == GS_STATE_GAME_PLAY ){
				// palette_restore_palette();

				// change a couple of flags to indicate our state!!!
				Net_player->state = NETPLAYER_STATE_IN_MISSION;
				send_netplayer_update_packet();

				// set the game mode
				Game_mode |= GM_IN_MISSION;
			}

			main_hall_stop_music();
			main_hall_stop_ambient();		
			break;		
   
		case GS_STATE_VIEW_CUTSCENES:
			cutscenes_screen_close();
			break;

		case GS_STATE_MULTI_STD_WAIT:
			multi_standalone_wait_close();
	  		break;

		case GS_STATE_STANDALONE_MAIN:			
			standalone_main_close();
			if(new_state == GS_STATE_MULTI_STD_WAIT){		
				init_multiplayer_stats();										
			}			
			break;

		case GS_STATE_STANDALONE_POSTGAME:
			multi_standalone_postgame_close();
			break;

		case GS_STATE_MULTI_PAUSED:
			multi_pause_close(end_mission);
			break;			

		case GS_STATE_INGAME_PRE_JOIN:
			multi_ingame_select_close();
			break;

		case GS_STATE_INITIAL_PLAYER_SELECT:			
			player_select_close();			
			break;		

		case GS_STATE_MULTI_START_GAME:
			multi_start_game_close();
			break;

		case GS_STATE_MULTI_HOST_OPTIONS:
			multi_host_options_close();
			break;				

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_close();
			break;

		case GS_STATE_LOOP_BRIEF:
			loop_brief_close();
			break;

		case GS_STATE_FICTION_VIEWER:
			fiction_viewer_close();
			break;

		case GS_STATE_LAB:
			lab_close();
			break;

		case GS_STATE_SCRIPTING:
			scripting_state_close();
			break;
	}

	//WMC - Now run scripting stuff
	Script_system.RunCondition(CHA_ONSTATEEND);
}

// variable used for automatic netgame starting/joining
int Main_hall_netgame_started = 0;

// Called when a state is being entered.
// The current state is set to the state we're entering at
// this point, and old_state is set to the state we're coming
// from.    You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.

void game_enter_state( int old_state, int new_state )
{
	//WMC - Scripting override
	/*
	if(GS_state_hooks[new_state].IsValid() && Script_system.IsOverride(GS_state_hooks[new_state])) {
		return;
	}
	*/
	if(Script_system.IsConditionOverride(CHA_ONSTATESTART)) {
		Script_system.RunCondition(CHA_ONSTATESTART);
		return;
	}

	switch (new_state) {
		case GS_STATE_MAIN_MENU:				
			// in multiplayer mode, be sure that we are not doing networking anymore.
			if ( Game_mode & GM_MULTIPLAYER ) {
				Assert( Net_player != NULL );
				Net_player->flags &= ~NETINFO_FLAG_DO_NETWORKING;
			}

			// remove any multiplayer flags from the game mode
			Game_mode &= ~(GM_MULTIPLAYER);
	
			// determine which ship this guy is currently based on
			main_hall_init(Player->main_hall);
			// set the game_mode based on the type of player
			Assert( Player != NULL );

			if ( Player->flags & PLAYER_FLAGS_IS_MULTI ){
				Game_mode = GM_MULTIPLAYER;
			} else {
				Game_mode = GM_NORMAL;
			}

			//if ( (Cmdline_start_netgame || (Cmdline_connect_addr != NULL)) && !Main_hall_netgame_started ) {
			//	Main_hall_netgame_started = 1;
			//	main_hall_do_multi_ready();
			//} DTP commented out to keep original source
			if ( (Cmdline_start_netgame || (Cmdline_connect_addr != NULL)) && (!Main_hall_netgame_started) /*&& (Game_mode == GM_MULTIPLAYER)*/) { // DTP added "&& (game_mode == GM_multiplayer)" so that ppl don't get thrown into Multiplayer with a Singleplayer Pilot.
				Main_hall_netgame_started = 1;
				main_hall_do_multi_ready();
			}

			if(Cmdline_start_mission) {
				strcpy_s(Game_current_mission_filename, Cmdline_start_mission);
				mprintf(( "Straight to mission '%s'\n", Game_current_mission_filename ));
 				gameseq_post_event(GS_EVENT_START_GAME);
				// This stops the mission from loading again when you go back to the hall
				Cmdline_start_mission = NULL;
			}
			break;

		case GS_STATE_START_GAME:
			main_hall_stop_music();
			main_hall_stop_ambient();
			
			if (Game_mode & GM_NORMAL) {
				// this should put us into a new state on failure!
				if (!game_start_mission())
					break;
			}

			// maybe play a movie before the mission
			mission_campaign_maybe_play_movie(CAMPAIGN_MOVIE_PRE_MISSION);

			// determine where to go next
			if (mission_has_fiction()) {
				gameseq_post_event(GS_EVENT_FICTION_VIEWER);
			} else if (mission_has_cmd_brief()) {
				gameseq_post_event(GS_EVENT_CMD_BRIEF);
			} else if (red_alert_mission()) {
				gameseq_post_event(GS_EVENT_RED_ALERT);
			} else {
				gameseq_post_event(GS_EVENT_START_BRIEFING);
			}
			break;

		case GS_STATE_FICTION_VIEWER:
			common_maybe_play_cutscene(MOVIE_PRE_FICTION); 	
			fiction_viewer_init();
			break;

		case GS_STATE_CMD_BRIEF: {
			if (old_state == GS_STATE_OPTIONS_MENU) {
				cmd_brief_unhold();
			} else {
				common_maybe_play_cutscene(MOVIE_PRE_CMD_BRIEF); 	
				int team_num = 0;  // team number used as index for which cmd brief to use.
				cmd_brief_init(team_num);
			}
			break;
		}

		case GS_STATE_RED_ALERT:
			common_maybe_play_cutscene(MOVIE_PRE_BRIEF); 	
			red_alert_init();
			break;

		case GS_STATE_BRIEFING:
			common_maybe_play_cutscene(MOVIE_PRE_BRIEF); 
			brief_init();
			break;

		case GS_STATE_DEBRIEF:
			game_stop_looped_sounds();
			mission_goal_fail_incomplete();				// fail all incomplete goals before entering debriefing
			if ( (old_state != GS_STATE_VIEW_MEDALS) && (old_state != GS_STATE_OPTIONS_MENU) ){
				common_maybe_play_cutscene(MOVIE_PRE_DEBRIEF); 	
				debrief_init();
			}
			break;

		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			multi_df_debrief_init();
			break;

		case GS_STATE_LOAD_MISSION_MENU:
			mission_load_menu_init();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			sim_room_init();
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			campaign_room_init();
			break;

		case GS_STATE_SHIP_SELECT:
			ship_select_init();
			break;

		case GS_STATE_WEAPON_SELECT:
			weapon_select_init();
			break;

		case GS_STATE_TEAM_SELECT:		
			multi_ts_init();
			break;

		case GS_STATE_GAME_PAUSED:
			game_stop_time();
			pause_init();
			break;

		case GS_STATE_DEBUG_PAUSED:
	//		game_stop_time();
	//		os_set_title("FreeSpace - PAUSED");
	//		break;
	//
		case GS_STATE_TRAINING_PAUSED:
			#ifndef NDEBUG
				game_stop_time();
				pause_debug_init();
			#endif
			break;

		case GS_STATE_OPTIONS_MENU:
			//game_stop_time();
			options_menu_init();
			break;
 
		case GS_STATE_GAME_PLAY:

			// maybe play a cutscene
			if((old_state == GS_STATE_BRIEFING) || (old_state == GS_STATE_SHIP_SELECT) || 
			   (old_state == GS_STATE_WEAPON_SELECT) || (old_state == GS_STATE_RED_ALERT) ) {
				common_maybe_play_cutscene(MOVIE_PRE_GAME); 
			}
			// reset time compression to default level so it's right at the beginning of a mission - taylor
			if(old_state != GS_STATE_GAME_PAUSED)
			{
				//Game_time_compression = F1_0;
			}

			/* game could be comming from a restart (rather than first start)
			so make sure that we zero the hud gauge overrides (Sexp_hud_*)
			\sa sexp_hud_display_gauge*/
			if ( (old_state == GS_STATE_GAME_PLAY)
				|| (old_state == GS_STATE_BRIEFING)
				|| (old_state == GS_STATE_DEBRIEF)
				|| (old_state == GS_STATE_SHIP_SELECT)
				|| (old_state == GS_STATE_WEAPON_SELECT)
				|| (old_state == GS_STATE_RED_ALERT) )
			{
				Sexp_hud_display_warpout = 0;
			}

			// coming from the gameplay state or the main menu, we might need to load the mission
			if ( (Game_mode & GM_NORMAL) && ((old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_GAME_PLAY) || (old_state == GS_STATE_DEATH_BLEW_UP)) ) {
				if ( !game_start_mission() )		// this should put us into a new state.
					// Failed!!!
					break;
			}

			// if we are coming from the briefing, ship select, weapons loadout, or main menu (in the
			// case of quick start), then do bitmap loads, etc  Don't do any of the loading stuff
			// if we are in multiplayer -- this stuff is all handled in the multi-wait section
			if ( !(Game_mode & GM_MULTIPLAYER) && ((old_state == GS_STATE_BRIEFING) || (old_state == GS_STATE_SHIP_SELECT) ||
				(old_state == GS_STATE_WEAPON_SELECT) || (old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_MULTI_STD_WAIT) || (old_state == GS_STATE_SIMULATOR_ROOM)) ) {
					// JAS: Used to do all paging here.

					#ifndef NDEBUG
					//XSTR:OFF
						HUD_printf("Skill level is set to ** %s **", Skill_level_names(Game_skill_level));
					//XSTR:ON
					#endif

					main_hall_stop_music();
					main_hall_stop_ambient();
					event_music_first_pattern();	// start the first pattern
			}

			// special code that restores player ship selection and weapons loadout when doing a quick start
			if ( !(Game_mode & GM_MULTIPLAYER) && ((old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_DEATH_BLEW_UP)  || (old_state == GS_STATE_GAME_PLAY)) ) {
				if ( !stricmp(Player_loadout.filename, Game_current_mission_filename) ) {
					wss_direct_restore_loadout();
				}
			}

			// single-player, quick-start after just died... we need to set weapon linking and kick off the event music
			if (!(Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_DEATH_BLEW_UP) ) {
				event_music_first_pattern();	// start the first pattern
			}

			if ( !(Game_mode & GM_STANDALONE_SERVER) && ((old_state != GS_STATE_GAME_PAUSED) && (old_state != GS_STATE_MULTI_PAUSED)) ) {
				event_music_first_pattern();	// start the first pattern
			}			
			player_restore_target_and_weapon_link_prefs();

			if ( !Is_standalone )
				radar_mission_init();

			//Set the current hud
			set_current_hud();

			ship_init_cockpit_displays(Player_ship);

			Game_mode |= GM_IN_MISSION;

#ifndef NDEBUG
			// required to truely make mouse deltas zeroed in debug mouse code
void mouse_force_pos(int x, int y);
			mouse_force_pos(gr_screen.max_w / 2, gr_screen.max_h / 2);
#endif

			game_flush();

			// only start time if in single player, or coming from multi wait state
			if (
					(
						(Game_mode & GM_NORMAL) && 
						(old_state != GS_STATE_VIEW_CUTSCENES)
					) || (
						(Game_mode & GM_MULTIPLAYER) && (
							(old_state == GS_STATE_MULTI_PAUSED) ||
							(old_state == GS_STATE_MULTI_MISSION_SYNC)
						)
					)
				)
					game_start_time();

			// when coming from the multi paused state, reset the timestamps
			if ( (Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_MULTI_PAUSED) ){
				multi_reset_timestamps();
			}

			if ((Game_mode & GM_MULTIPLAYER) && (old_state != GS_STATE_DEATH_BLEW_UP) ) {
				// initialize all object update details
				multi_oo_gameplay_init();
			}
	
			// under certain circumstances, the server should reset the object update rate limiting stuff
			if( MULTIPLAYER_MASTER && ((old_state == GS_STATE_MULTI_PAUSED) || (old_state == GS_STATE_MULTI_MISSION_SYNC)) ){
				
				// reinitialize the rate limiting system for all clients
				multi_oo_rate_init_all();
			}

			// multiplayer clients should always re-initialize their control info rate limiting system			
			if(MULTIPLAYER_CLIENT){
				multi_oo_rate_init_all();
			}
			
			// reset ping times
			if(Game_mode & GM_MULTIPLAYER){
				multi_ping_reset_players();
			}

			Game_subspace_effect = 0;
			if (The_mission.flags & MISSION_FLAG_SUBSPACE) {
				Game_subspace_effect = 1;
				if( !(Game_mode & GM_STANDALONE_SERVER) ){	
					game_start_subspace_ambient_sound();
				}
			}

			sound_env_set(&Game_sound_env);
			joy_ff_mission_init(Ship_info[Player_ship->ship_info_index].rotation_time);

			// clear multiplayer button info			i
			extern button_info Multi_ship_status_bi;
			memset(&Multi_ship_status_bi, 0, sizeof(button_info));
			break;

		case GS_STATE_HUD_CONFIG:
			hud_config_init();
			break;

		case GS_STATE_PXO:
			if (old_state != GS_STATE_PXO_HELP) {
				multi_pxo_init( 0 );
			}
			break;

		case GS_STATE_PXO_HELP:
			multi_pxo_help_init();
			break;

		case GS_STATE_MULTI_JOIN_GAME:
			multi_join_clear_game_list();

			if (old_state != GS_STATE_OPTIONS_MENU) {
				multi_join_game_init();
			}

			break;

		case GS_STATE_MULTI_HOST_SETUP:		
			// don't reinitialize if we're coming back from the host options screen
			if ((old_state != GS_STATE_MULTI_HOST_OPTIONS) && (old_state != GS_STATE_OPTIONS_MENU)) {
				multi_create_game_init();
			}

			break;

		case GS_STATE_MULTI_CLIENT_SETUP:		
			if (old_state != GS_STATE_OPTIONS_MENU) {
				multi_game_client_setup_init();
			}

			break;

		case GS_STATE_CONTROL_CONFIG:
			control_config_init();
			break;

		case GS_STATE_TECH_MENU:
			techroom_init();
			break;

		case GS_STATE_BARRACKS_MENU:
			if(old_state != GS_STATE_VIEW_MEDALS){
				barracks_init();
			}
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			hud_scrollback_init();
			break;

		case GS_STATE_DEATH_DIED:
 			Player_died_time = timestamp(10);

			if(!(Game_mode & GM_MULTIPLAYER)){
				player_show_death_message();
			}
			Game_mode |= GM_DEAD_DIED;
			break;

		case GS_STATE_DEATH_BLEW_UP:
			if ( !popupdead_is_active() ) {
				Player_ai->target_objnum = -1;
			}

			// stop any local EMP effect
			emp_stop_local();

			Players[Player_num].flags &= ~PLAYER_FLAGS_AUTO_TARGETING;	//	Prevent immediate switch to a hostile ship.
			Game_mode |= GM_DEAD_BLEW_UP;		
			Show_viewing_from_self = 0;

			// timestamp how long we should wait before displaying the died popup
			if ( !popupdead_is_active() ) {
				Player_died_popup_wait = timestamp(PLAYER_DIED_POPUP_WAIT);
			}
			break;

		case GS_STATE_GAMEPLAY_HELP:
			gameplay_help_init();
			break;

		case GS_STATE_CREDITS:
			main_hall_stop_music();
			main_hall_stop_ambient();
			credits_init();
			break;

		case GS_STATE_VIEW_MEDALS:
			medal_main_init(Player);
			break;

		case GS_STATE_SHOW_GOALS:
			mission_show_goals_init();
			break;

		case GS_STATE_HOTKEY_SCREEN:
			mission_hotkey_init();
			break;

		case GS_STATE_MULTI_MISSION_SYNC:
			// if we're coming from the options screen, don't do any
			if(old_state == GS_STATE_OPTIONS_MENU){
				break;
			}

			switch(Multi_sync_mode){
			case MULTI_SYNC_PRE_BRIEFING:
				// if moving from game forming to the team select state						
				multi_sync_init();			
				break;
			case MULTI_SYNC_POST_BRIEFING:
				// if moving from briefing into the mission itself			
				multi_sync_init();
			
				// tell everyone that we're now loading data
				Net_player->state = NETPLAYER_STATE_DATA_LOAD;
				send_netplayer_update_packet();

				// JAS: Used to do all paging here!!!!
								
				Net_player->state = NETPLAYER_STATE_WAITING;			
				send_netplayer_update_packet();
				break;
			case MULTI_SYNC_INGAME:
				multi_sync_init();
				break;
			}
			break;		
   
		case GS_STATE_VIEW_CUTSCENES:
			cutscenes_screen_init();
			break;

		case GS_STATE_MULTI_STD_WAIT:
			multi_standalone_wait_init();
			break;

		case GS_STATE_STANDALONE_MAIN:
			// don't initialize if we're coming from one of these 2 states unless there are no 
			// players left (reset situation)
			if((old_state != GS_STATE_STANDALONE_POSTGAME) || multi_endgame_ending()){
				standalone_main_init();
			}
			break;	

		case GS_STATE_STANDALONE_POSTGAME:
			multi_standalone_postgame_init();
			break;

		case GS_STATE_MULTI_PAUSED:
			multi_pause_init();
			break;

		case GS_STATE_INGAME_PRE_JOIN:
			multi_ingame_select_init();
			break;

		case GS_STATE_INITIAL_PLAYER_SELECT:
			player_select_init();
			break;		

		case GS_STATE_MULTI_START_GAME:
			multi_start_game_init();
			break;

		case GS_STATE_MULTI_HOST_OPTIONS:
			multi_host_options_init();
			break;		

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_init();
			break;		

		case GS_STATE_LOOP_BRIEF:
			loop_brief_init();
			break;

		case GS_STATE_LAB:
			lab_init();
			break;

		case GS_STATE_SCRIPTING:
			scripting_state_init();
			break;
	} // end switch

	//WMC - now do user scripting stuff
	Script_system.RunCondition(CHA_ONSTATESTART);
}

// do stuff that may need to be done regardless of state
void game_do_state_common(int state,int no_networking)
{
	game_maybe_draw_mouse(flFrametime);		// determine if to draw the mouse this frame
	snd_do_frame();								// update sound system
	event_music_do_frame();						// music needs to play across many states

	multi_log_process();	

	if (no_networking) {
		return;
	}

	// maybe do a multiplayer frame based on game mode and state type	
	if (Game_mode & GM_MULTIPLAYER) {
		switch (state) {
			case GS_STATE_OPTIONS_MENU:
			case GS_STATE_GAMEPLAY_HELP:
			case GS_STATE_HOTKEY_SCREEN:
			case GS_STATE_HUD_CONFIG:
			case GS_STATE_CONTROL_CONFIG:
			case GS_STATE_MISSION_LOG_SCROLLBACK:
			case GS_STATE_SHOW_GOALS:
			case GS_STATE_VIEW_CUTSCENES:
			case GS_STATE_EVENT_DEBUG:
				multi_maybe_do_frame();
				break;
		}
		
		game_do_networking();
	}
}

// Called once a frame.
// You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.
int Game_do_state_should_skip = 0;
void game_do_state(int state)
{
	// always lets the do_state_common() function determine if the state should be skipped
	Game_do_state_should_skip = 0;
	
	// legal to set the should skip state anywhere in this function
	game_do_state_common(state);	// do stuff that may need to be done regardless of state

	if(Game_do_state_should_skip){
		return;
	}

	if(Script_system.IsConditionOverride(CHA_ONFRAME)) {
		game_set_frametime(state);
		gr_clear();
		gr_flip();	//Does state hook automagically
		return;
	}
	/*
	if(Script_system.IsOverride(GS_state_hooks[state]))
	{
		game_set_frametime(state);
		gr_clear();
		Script_system.RunBytecode(GS_state_hooks[state]);
		gr_flip();
		return;
	}
	*/
	
	switch (state) {
		case GS_STATE_MAIN_MENU:
			game_set_frametime(GS_STATE_MAIN_MENU);
			main_hall_do(flFrametime);
			break;

		case GS_STATE_OPTIONS_MENU:
			game_set_frametime(GS_STATE_OPTIONS_MENU);
			options_menu_do_frame(flFrametime);
			break;

		case GS_STATE_BARRACKS_MENU:
			game_set_frametime(GS_STATE_BARRACKS_MENU);
			barracks_do_frame(flFrametime);
			break;

		case GS_STATE_TRAINING_MENU:
			game_set_frametime(GS_STATE_TRAINING_MENU);
			training_menu_do_frame(flFrametime);
			break;

		case GS_STATE_TECH_MENU:
			game_set_frametime(GS_STATE_TECH_MENU);
			techroom_do_frame(flFrametime);
			break;

		case GS_STATE_GAMEPLAY_HELP:
			game_set_frametime(GS_STATE_GAMEPLAY_HELP);
			gameplay_help_do_frame(flFrametime);
			break;

		case GS_STATE_GAME_PLAY:	// do stuff that should be done during gameplay
			game_do_frame();
			break;

		case GS_STATE_GAME_PAUSED:

			if(pause_get_type() == PAUSE_TYPE_VIEWER)	{

				read_player_controls( Player_obj, flFrametime);
			//	game_process_keys();
				game_frame(true);
			}
				
			pause_do();
			break;

		case GS_STATE_DEBUG_PAUSED:
			#ifndef NDEBUG
				game_set_frametime(GS_STATE_DEBUG_PAUSED);
				pause_debug_do();
			#endif
			break;

		case GS_STATE_TRAINING_PAUSED:
			game_training_pause_do();
			break;

		case GS_STATE_LOAD_MISSION_MENU:
			game_set_frametime(GS_STATE_LOAD_MISSION_MENU);
			mission_load_menu_do();
			break;
		
		case GS_STATE_BRIEFING:
			game_set_frametime(GS_STATE_BRIEFING);
			brief_do_frame(flFrametime);
			break;

		case GS_STATE_DEBRIEF:
			game_set_frametime(GS_STATE_DEBRIEF);
			debrief_do_frame(flFrametime);
			break;

		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			game_set_frametime(GS_STATE_MULTI_DOGFIGHT_DEBRIEF);
			multi_df_debrief_do();
			break;

		case GS_STATE_SHIP_SELECT:
			game_set_frametime(GS_STATE_SHIP_SELECT);
			ship_select_do(flFrametime);
			break;

		case GS_STATE_WEAPON_SELECT:
			game_set_frametime(GS_STATE_WEAPON_SELECT);
			weapon_select_do(flFrametime);
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			game_set_frametime(GS_STATE_MISSION_LOG_SCROLLBACK);
			hud_scrollback_do_frame(flFrametime);
			break;

		case GS_STATE_HUD_CONFIG:
			game_set_frametime(GS_STATE_HUD_CONFIG);
			hud_config_do_frame(flFrametime);
			break;

		case GS_STATE_PXO:
			game_set_frametime(GS_STATE_PXO);
			multi_pxo_do();
			break;

		case GS_STATE_PXO_HELP:
			game_set_frametime(GS_STATE_PXO_HELP);
			multi_pxo_help_do();
			break;

		case GS_STATE_MULTI_JOIN_GAME:
			game_set_frametime(GS_STATE_MULTI_JOIN_GAME);
			multi_join_game_do_frame();
			break;

		case GS_STATE_MULTI_HOST_SETUP:
			game_set_frametime(GS_STATE_MULTI_HOST_SETUP);
			multi_create_game_do();
			break;

		case GS_STATE_MULTI_CLIENT_SETUP:
			game_set_frametime(GS_STATE_MULTI_CLIENT_SETUP);
			multi_game_client_setup_do_frame();
			break;

		case GS_STATE_CONTROL_CONFIG:
			game_set_frametime(GS_STATE_CONTROL_CONFIG);
			control_config_do_frame(flFrametime);
			break;	

		case GS_STATE_DEATH_DIED:
			game_do_frame();			
			break;

		case GS_STATE_DEATH_BLEW_UP:
			game_do_frame();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			game_set_frametime(GS_STATE_SIMULATOR_ROOM);
			sim_room_do_frame(flFrametime);
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			game_set_frametime(GS_STATE_CAMPAIGN_ROOM);
			campaign_room_do_frame(flFrametime);
			break;

		case GS_STATE_RED_ALERT:
			game_set_frametime(GS_STATE_RED_ALERT);
			red_alert_do_frame(flFrametime);
			break;

		case GS_STATE_CMD_BRIEF:
			game_set_frametime(GS_STATE_CMD_BRIEF);
			cmd_brief_do_frame(flFrametime);
			break;

		case GS_STATE_CREDITS:
			game_set_frametime(GS_STATE_CREDITS);
			credits_do_frame(flFrametime);
			break;

		case GS_STATE_VIEW_MEDALS:
			game_set_frametime(GS_STATE_VIEW_MEDALS);
			medal_main_do();
			break;

		case GS_STATE_SHOW_GOALS:
			game_set_frametime(GS_STATE_SHOW_GOALS);
			mission_show_goals_do_frame(flFrametime);
			break;

		case GS_STATE_HOTKEY_SCREEN:
			game_set_frametime(GS_STATE_HOTKEY_SCREEN);
			mission_hotkey_do_frame(flFrametime);
			break;	
   
		case GS_STATE_VIEW_CUTSCENES:
			game_set_frametime(GS_STATE_VIEW_CUTSCENES);
			cutscenes_screen_do_frame();
			break;

		case GS_STATE_MULTI_STD_WAIT:
			game_set_frametime(GS_STATE_STANDALONE_MAIN);
			multi_standalone_wait_do();
			break;

		case GS_STATE_STANDALONE_MAIN:
			game_set_frametime(GS_STATE_STANDALONE_MAIN);
			standalone_main_do();
			break;	

		case GS_STATE_MULTI_PAUSED:
			game_set_frametime(GS_STATE_MULTI_PAUSED);
			multi_pause_do();
			break;

		case GS_STATE_TEAM_SELECT:
			game_set_frametime(GS_STATE_TEAM_SELECT);
			multi_ts_do();
			break;

		case GS_STATE_INGAME_PRE_JOIN:
			game_set_frametime(GS_STATE_INGAME_PRE_JOIN);
			multi_ingame_select_do();
			break;

		case GS_STATE_EVENT_DEBUG:
	#ifndef NDEBUG
			game_set_frametime(GS_STATE_EVENT_DEBUG);
			game_show_event_debug(flFrametime);
	#endif
			break;

		case GS_STATE_STANDALONE_POSTGAME:
			game_set_frametime(GS_STATE_STANDALONE_POSTGAME);
			multi_standalone_postgame_do();
			break;

		case GS_STATE_INITIAL_PLAYER_SELECT:
			game_set_frametime(GS_STATE_INITIAL_PLAYER_SELECT);
			player_select_do();
			break;

		case GS_STATE_MULTI_MISSION_SYNC:
			game_set_frametime(GS_STATE_MULTI_MISSION_SYNC);
			multi_sync_do();
			break;		

		case GS_STATE_MULTI_START_GAME:
			game_set_frametime(GS_STATE_MULTI_START_GAME);
			multi_start_game_do();
			break;
		
		case GS_STATE_MULTI_HOST_OPTIONS:
			game_set_frametime(GS_STATE_MULTI_HOST_OPTIONS);
			multi_host_options_do();
			break;		

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_do();
			break;		

		case GS_STATE_END_DEMO:
			game_set_frametime(GS_STATE_END_DEMO);
			end_demo_campaign_do();
			break;

		case GS_STATE_LOOP_BRIEF:
			game_set_frametime(GS_STATE_LOOP_BRIEF);
			loop_brief_do();
			break;

		case GS_STATE_FICTION_VIEWER:
			game_set_frametime(GS_STATE_FICTION_VIEWER);
			fiction_viewer_do_frame(flFrametime);
			break;

		case GS_STATE_LAB:
			game_set_frametime(GS_STATE_LAB);
			lab_do_frame(flFrametime);
			break;

		case GS_STATE_SCRIPTING:
			game_set_frametime(GS_STATE_SCRIPTING);
			scripting_state_do_frame(flFrametime);
			break;

   } // end switch(gs_current_state)
}


#ifdef _WIN32
// return 0 if there is enough RAM to run FreeSpace, otherwise return -1
int game_do_ram_check(uint ram_in_bytes)
{
	if ( ram_in_bytes < 30*1024*1024 )	{
		int allowed_to_run = 1;
		if ( ram_in_bytes < 25*1024*1024 ) {
			allowed_to_run = 0;
		}

		char tmp[1024];
		uint FreeSpace_total_ram_MB;
		FreeSpace_total_ram_MB = (uint)(ram_in_bytes/(1024*1024));

		if ( allowed_to_run ) {

			sprintf( tmp, XSTR( "FreeSpace has detected that you only have %dMB of free memory.\n\nFreeSpace requires at least 32MB of memory to run.  If you think you have more than %dMB of physical memory, ensure that you aren't running SmartDrive (SMARTDRV.EXE).  Any memory allocated to SmartDrive is not usable by applications\n\nPress 'OK' to continue running with less than the minimum required memory\n", 193), FreeSpace_total_ram_MB, FreeSpace_total_ram_MB);

			int msgbox_rval;
			msgbox_rval = MessageBox( NULL, tmp, XSTR( "Not Enough RAM", 194), MB_OKCANCEL );
			if ( msgbox_rval == IDCANCEL ) {
				return -1;
			}

		} else {
			sprintf( tmp, XSTR( "FreeSpace has detected that you only have %dMB of free memory.\n\nFreeSpace requires at least 32MB of memory to run.  If you think you have more than %dMB of physical memory, ensure that you aren't running SmartDrive (SMARTDRV.EXE).  Any memory allocated to SmartDrive is not usable by applications\n", 195), FreeSpace_total_ram_MB, FreeSpace_total_ram_MB);
			MessageBox( NULL, tmp, XSTR( "Not Enough RAM", 194), MB_OK );
			return -1;
		}
	}

	return 0;
}


#if 0 // no updater for fs2
// Check if there is a freespace.exe in the /update directory (relative to where fs.exe is installed).
// If so, copy it over and remove the update directory.
void game_maybe_update_launcher(char *exe_dir)
{
	char src_filename[MAX_PATH];
	char dest_filename[MAX_PATH];

	strcpy_s(src_filename, exe_dir);
	strcat_s(src_filename, NOX("\\update\\freespace.exe"));

	strcpy_s(dest_filename, exe_dir);
	strcat_s(dest_filename, NOX("\\freespace.exe"));

	// see if src_filename exists
	FILE *fp;
	fp = fopen(src_filename, "rb");
	if ( !fp ) {
		return;
	}
	fclose(fp);

	SetFileAttributes(dest_filename, FILE_ATTRIBUTE_NORMAL);

	// copy updated freespace.exe to freespace exe dir
	if ( CopyFile(src_filename, dest_filename, 0) == 0 ) {
		MessageBox( NULL, XSTR("Unable to copy freespace.exe from update directory to installed directory.  You should copy freespace.exe from the update directory (located in your FreeSpace install directory) to your install directory", 988), NULL, MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
		return;
	}

	// delete the file in the update directory
	DeleteFile(src_filename);

	// safe to assume directory is empty, since freespace.exe should only be the file ever in the update dir
	char update_dir[MAX_PATH];
	strcpy_s(update_dir, exe_dir);
	strcat_s(update_dir, NOX("\\update"));
	RemoveDirectory(update_dir);
}
#endif // no launcher

#endif // ifdef WIN32

void game_spew_pof_info_sub(int model_num, polymodel *pm, int sm, CFILE *out, int *out_total, int *out_destroyed_total)
{
	int i;
	int sub_total = 0;
	int sub_total_destroyed = 0;
	int total = 0;
	char str[255] = "";		
	
	// get the total for all his children
	for (i=pm->submodel[sm].first_child; i >= 0; i = pm->submodel[i].next_sibling )	{
		game_spew_pof_info_sub(model_num, pm, i, out, &sub_total, &sub_total_destroyed);
	}	

	// find the # of faces for this _individual_ object	
	total = submodel_get_num_polys(model_num, sm);
	if(strstr(pm->submodel[sm].name, "-destroyed")){
		sub_total_destroyed = total;
	}
	
	// write out total
	sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[sm].name, total);
	cfputs(str, out);		

	*out_total += total + sub_total;
	*out_destroyed_total += sub_total_destroyed;
}

#define BAIL()			do { int _idx; for(_idx=0; _idx<num_files; _idx++){ if(pof_list[_idx] != NULL){vm_free(pof_list[_idx]); pof_list[_idx] = NULL;}} return;} while(0);
void game_spew_pof_info()
{
	char *pof_list[1000];
	int num_files;	
	CFILE *out;
	int idx, model_num, i, j;
	polymodel *pm;
	int total, root_total, model_total, destroyed_total, counted;
	char str[255] = "";

	// get file list
	num_files = cf_get_file_list(1000, pof_list, CF_TYPE_MODELS, "*.pof");

	// spew info on all the pofs
	if(!num_files){
		return;
	}

	// go
	out = cfopen("pofspew.txt", "wt", CFILE_NORMAL, CF_TYPE_DATA);
	if(out == NULL){
		BAIL();
	}	
	counted = 0;	
	for(idx=0; idx<num_files; idx++, counted++){
		sprintf(str, "%s.pof", pof_list[idx]);
		model_num = model_load(str, 0, NULL);
		if(model_num >= 0){
			pm = model_get(model_num);

			// if we have a real model
			if(pm != NULL){				
				cfputs(str, out);
				cfputs("\n", out);
				
				// go through and print all raw submodels
				cfputs("RAW\n", out);
				total = 0;
				model_total = 0;				
				for (i=0; i<pm->n_models; i++)	{					
					total = submodel_get_num_polys(model_num, i);					
					
					model_total += total;
					sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[i].name, total);
					cfputs(str, out);
				}				
				sprintf(str, "Model total %d\n", model_total);				
				cfputs(str, out);				

				// now go through and do it by LOD
				cfputs("BY LOD\n\n", out);				
				for(i=0; i<pm->n_detail_levels; i++){
					sprintf(str, "LOD %d\n", i);
					cfputs(str, out);

					// submodels
					root_total = submodel_get_num_polys(model_num, pm->detail[i] );
					total = 0;
					destroyed_total = 0;
					for (j=pm->submodel[pm->detail[i]].first_child; j >= 0; j = pm->submodel[j].next_sibling )	{
						game_spew_pof_info_sub(model_num, pm, j, out, &total, &destroyed_total);
					}

					sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[pm->detail[i]].name, root_total);
					cfputs(str, out);

					sprintf(str, "TOTAL: %d\n", total + root_total);					
					cfputs(str, out);
					sprintf(str, "TOTAL not counting destroyed faces %d\n", (total + root_total) - destroyed_total);
					cfputs(str, out);
					sprintf(str, "TOTAL destroyed faces %d\n\n", destroyed_total);
					cfputs(str, out);
				}				
				cfputs("------------------------------------------------------------------------\n\n", out);				
			}
		}

		if(counted >= MAX_POLYGON_MODELS - 5){
			model_free_all();
			counted = 0;
		}
	}

	cfclose(out);
	model_free_all();
	BAIL();
}

DCF(pofspew, "")
{
	game_spew_pof_info();
}

// returns:
//		0 on an error
//		1 on a clean exit
int game_main(char *cmdline)
{
	int state;		

	// check if networking should be disabled, this could probably be done later but the sooner the better
	// TODO: remove this when multi is fixed to handle more than MAX_SHIP_CLASSES_MULTI
	if ( Num_ship_classes > MAX_SHIP_CLASSES_MULTI ) {
		Networking_disabled = 1;
	}

#ifndef NDEBUG				
	extern void windebug_memwatch_init();
	windebug_memwatch_init();
#endif

#ifdef _WIN32
	// Find out how much RAM is on this machine
	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	FreeSpace_total_ram = ms.dwTotalPhys;

	Mem_starttime_phys      = ms.dwAvailPhys;
	Mem_starttime_pagefile  = ms.dwAvailPageFile;
	Mem_starttime_virtual   = ms.dwAvailVirtual;

	if ( game_do_ram_check(FreeSpace_total_ram) == -1 ) {
		return 1;
	}

	if ( ms.dwTotalVirtual < 1024 )	{
		MessageBox( NULL, XSTR( "FreeSpace requires virtual memory to run.\r\n", 196), XSTR( "No Virtual Memory", 197), MB_OK );
		return 1;
	}

	if (!vm_init(24*1024*1024)) {
		MessageBox( NULL, XSTR( "Not enough memory to run FreeSpace.\r\nTry closing down some other applications.\r\n", 198), XSTR( "Not Enough Memory", 199), MB_OK );
		return 1;
	}
		
	char *tmp_mem = (char *) vm_malloc(16 * 1024 * 1024);
	if (!tmp_mem) {
		MessageBox(NULL, XSTR( "Not enough memory to run FreeSpace.\r\nTry closing down some other applications.\r\n", 198), XSTR( "Not Enough Memory", 199), MB_OK);
		return 1;
	}

	vm_free(tmp_mem);
	tmp_mem = NULL;

#else

	vm_init(0); 

#endif // _WIN32


	if ( !parse_cmdline(cmdline) ) {
		return 1;
	}


#ifdef STANDALONE_ONLY_BUILD
	Is_standalone = 1;
	nprintf(("Network", "Standalone running"));
#else
	if (Is_standalone){
		nprintf(("Network", "Standalone running"));
	}
#endif

#ifdef _WIN32
	if ( !Is_standalone )
		disableWindowsKey( );
#endif


	init_cdrom();

	game_init();
	// calling the function that will init all the function pointers for TrackIR stuff (Swifty)
	int trackIrInitResult = gTirDll_TrackIR.Init( (HWND)os_get_window( ) );
	if ( trackIrInitResult != SCP_INITRESULT_SUCCESS )
	{
		mprintf( ("TrackIR Init Failed - %d\n", trackIrInitResult) );
	}
	game_stop_time();

	if (Cmdline_spew_mission_crcs) {
		multi_spew_pxo_checksums(1024, "mission_crcs.csv");

		if (Cmdline_spew_table_crcs) {
			fs2netd_spew_table_checksums("table_crcs.csv");
		}

		game_shutdown();
		return 0;
	}


	if (Cmdline_spew_table_crcs) {
		fs2netd_spew_table_checksums("table_crcs.csv");
		game_shutdown();
		return 0;
	}

	// maybe spew pof stuff
	if(Cmdline_spew_pof_info){
		game_spew_pof_info();
		game_shutdown();
		return 0;
	}


	// maybe spew VP CRCs, and exit
	if (Cmdline_verify_vps) {
		extern void cfile_spew_pack_file_crcs();
		cfile_spew_pack_file_crcs();
		game_shutdown();
		return 0;
	}

	if (!Is_standalone) {
		movie_play( NOX("intro.mve") );
	}

	if (Is_standalone){
		gameseq_post_event(GS_EVENT_STANDALONE_MAIN);
	} else {
		gameseq_post_event(GS_EVENT_GAME_INIT);		// start the game rolling -- check for default pilot, or go to the pilot select screen
	}

	while (1) {
		// only important for non THREADED mode
		os_poll();

		state = gameseq_process_events();
		if ( state == GS_STATE_QUIT_GAME ){
			break;
		}
	} 

	game_shutdown();

#ifdef _WIN32
	if ( !Is_standalone )
		enableWindowsKey( );
#endif

	return 0;
}


// ------------------------------------------------------------------------------
// Platform specific main() functions, nothing directly related to game function
// should go here.  Direct game related info should go in the game_main() function
// TODO: this should end up in a separate file in the not too distant future.
//

#ifdef _WIN32
// Windows Specific
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int nCmdShow)
{
	int result = -1;

	// Don't let more than one instance of FreeSpace run.
	HWND hwnd = FindWindow( NOX( "FreeSpaceClass" ), NULL );
	if ( hwnd )	{
		SetForegroundWindow(hwnd);
		return 0;
	}

	::CoInitialize(NULL);

#ifdef _DEBUG
	void memblockinfo_output_memleak();
	atexit(memblockinfo_output_memleak);
#endif

	//=====================================================
	// Make sure we're running in the right directory.
	char exe_dir[1024];
	
	if ( GetModuleFileName( hInst, exe_dir, 1023 ) > 0 )	{
		char *p = exe_dir + strlen(exe_dir);
		
		// chop off the filename
		while( (p>exe_dir) && (*p!='\\') && (*p!='/') && (*p!=':') )	{
			p--;
		}
		*p = 0;
		
		// Set directory
		if ( strlen(exe_dir) > 0 )	{ //-V805
			SetCurrentDirectory(exe_dir);
		}
	}

	SCP_mspdbcs_Initialise( );

#ifdef GAME_ERRORLOG_TXT
#ifdef _MSC_VER
	__try {
#endif
#endif
		result = !game_main(szCmdLine);
#ifdef GAME_ERRORLOG_TXT
#ifdef _MSC_VER
	} __except( RecordExceptionInfo(GetExceptionInformation(), "FreeSpace 2 Main Thread") ) {
		// Do nothing here - RecordExceptionInfo() has already done
		// everything that is needed. Actually this code won't even
		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
		// the __except clause.
	}
#endif // _MSC_VER
#endif

	SCP_mspdbcs_Cleanup( );

	::CoUninitialize();

#ifndef _MINGW
	_CrtDumpMemoryLeaks();
#endif

	return result;
}

#else

// *NIX specific
int main(int argc, char *argv[])
{
	int result = EXIT_FAILURE;
	char *argptr = NULL;
	int i, len = 0;
	char userdir[MAX_PATH];

#ifdef APPLE_APP
	// Finder sets the working directory to the root of the drive so we have to get a little creative
	// to find out where on the disk we should be running from for CFILE's sake.
	strncpy(full_path, *argv, 1024);
#endif

	// create user's directory	
	snprintf(userdir, MAX_PATH - 1, "%s/%s/", detect_home(), Osreg_user_dir);
	_mkdir(userdir);


	// clean up the cmdline to just send arguments through
	for (i = 1; i < argc; i++) {
		len += strlen(argv[i]) + 1;
	}

	argptr = (char*) calloc(len + 1, sizeof(char));
	
	if (argptr == NULL) {
		fprintf(stderr, "ERROR: Out of memory in main()!\n");
		exit(EXIT_FAILURE);
	}
	
	memset( argptr, 0, len+1 );
	
	for (i = 1; i < argc; i++) {
		strcat(argptr, argv[i]);
		strcat(argptr, " ");
	}

	// switch to game_main()
	try {
		result = game_main(argptr);

		if (argptr != NULL) {
			free(argptr);
			argptr = NULL;
		}
	} catch (std::exception &ex) {
		fprintf(stderr, "Caught std::exception in main(): '%s'!\n", ex.what());
		result = EXIT_FAILURE;
	} catch ( ... ) {
		fprintf(stderr, "Caught exception in main()!\n");
		result = EXIT_FAILURE;
	}

	return result;
}

#endif // _WIN32

//
// End of platform specific main() section
// ------------------------------------------------------------------------------


#if 0  // don't have an updater for fs2_open
// launch the fslauncher program on exit
void game_launch_launcher_on_exit()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char cmd_line[2048];
	char original_path[1024] = "";
	
	memset( &si, 0, sizeof(STARTUPINFO) );
	si.cb = sizeof(si);

	// directory
	_getcwd(original_path, 1023);

	// set up command line
	strcpy_s(cmd_line, original_path);
	strcat_s(cmd_line, DIR_SEPARATOR_STR);
	strcat_s(cmd_line, LAUNCHER_FNAME);
	strcat_s(cmd_line, " -straight_to_update");		

	BOOL ret = CreateProcess(	NULL,									// pointer to name of executable module 
										cmd_line,							// pointer to command line string
										NULL,									// pointer to process security attributes 
										NULL,									// pointer to thread security attributes 
										FALSE,								// handle inheritance flag 
										CREATE_DEFAULT_ERROR_MODE,		// creation flags 
										NULL,									// pointer to new environment block 
										NULL,									// pointer to current directory name 
										&si,									// pointer to STARTUPINFO 
										&pi									// pointer to PROCESS_INFORMATION  
										);			
	// to eliminate build warnings
	ret;
}
#endif


// game_shutdown()
//
// This function is called when FreeSpace terminates normally.  
//
void game_shutdown(void)
{
	gTirDll_TrackIR.Close( );


	fsspeech_deinit();
#ifdef FS2_VOICER
	if(Cmdline_voice_recognition)
	{
		VOICEREC_deinit();
	}
#endif

	// don't ever flip a page on the standalone!
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		gr_reset_clip();
		gr_clear();
		gr_flip();
	}

   // if the player has left the "player select" screen and quit the game without actually choosing
	// a player, Player will be NULL, in which case we shouldn't write the player file out!
	if (!(Game_mode & GM_STANDALONE_SERVER) && (Player!=NULL) && !Is_standalone){
		write_pilot_file();
		mission_campaign_savefile_save();
	}

	// load up common multiplayer icons
	multi_unload_common_icons();
	hud_close();	
	fireball_close();				// free fireball system
	particle_close();			// close out the particle system
	weapon_close();					// free any memory that was allocated for the weapons
	ship_close();					// free any memory that was allocated for the ships
	hud_free_scrollback_list();// free space allocated to store hud messages in hud scrollback
	unload_animating_pointer();// frees the frames used for the animating mouse pointer
	mission_campaign_close();	// close out the campaign stuff
	message_mission_close();	// clear loaded table data from message.tbl
	mission_parse_close();		// clear out any extra memory that may be in use by mission parsing
	multi_voice_close();			// close down multiplayer voice (including freeing buffers, etc)
	multi_log_close();
#ifdef MULTI_USE_LAG
	multi_lag_close();
#endif
	fs2netd_close();
	obj_pairs_close();		// free memory from object collision pairs
	stars_close();			// clean out anything used by stars code

	// the menu close functions will unload the bitmaps if they were displayed during the game
	main_hall_close();
	training_menu_close();
	gr_close();

	// free left over memory from table parsing
	player_tips_close();

	joy_close();

	audiostream_close();
	snd_close();
	event_music_close();
	gamesnd_close();		// close out gamesnd, needs to happen *after* other sounds are closed
	psnet_close();

	model_free_all();
	bm_unload_all();			// unload/free bitmaps, has to be called *after* model_free_all()!

	os_cleanup();

	// although the comment in cmdline.cpp said this isn't needed,
	// Valgrind disagrees (quite possibly incorrectly), but this is just cleaner
	if (Cmdline_mod != NULL) {
		delete[] Cmdline_mod;
		Cmdline_mod = NULL;
	}

#if 0  // don't have an updater for fs2_open
	// HACKITY HACK HACK
	// if this flag is set, we should be firing up the launcher when exiting freespace
	extern int Multi_update_fireup_launcher_on_exit;
	if(Multi_update_fireup_launcher_on_exit){
		game_launch_launcher_on_exit();
	}
#endif
}

// game_stop_looped_sounds()
//
// This function will call the appropriate stop looped sound functions for those
// modules which use looping sounds.  It is not enough just to stop a looping sound
// at the DirectSound level, the game is keeping track of looping sounds, and this
// function is used to inform the game that looping sounds are being halted.
//
void game_stop_looped_sounds()
{
	hud_stop_looped_locking_sounds();
	hud_stop_looped_engine_sounds();
	afterburner_stop_sounds();
	player_stop_looped_sounds();
	obj_snd_stop_all();		// stop all object-linked persistant sounds
	game_stop_subspace_ambient_sound();
	snd_stop(Radar_static_looping);
	Radar_static_looping = -1;
	snd_stop(Target_static_looping);
	shipfx_stop_engine_wash_sound();
	Target_static_looping = -1;
}

//////////////////////////////////////////////////////////////////////////
//
// Code for supporting an animating mouse pointer
//
//
//////////////////////////////////////////////////////////////////////////

typedef struct animating_obj
{
	int	first_frame;
	int	num_frames;
	int	current_frame;
	float time;
	float elapsed_time;
} animating_obj;

static animating_obj Animating_mouse;

// ----------------------------------------------------------------------------
// init_animating_pointer()
//
// Called by load_animating_pointer() to ensure the Animating_mouse struct
// gets properly initialized
//
void init_animating_pointer()
{
	Animating_mouse.first_frame	= -1;
	Animating_mouse.num_frames		= 0;
	Animating_mouse.current_frame	= -1;
	Animating_mouse.time				= 0.0f;
	Animating_mouse.elapsed_time	= 0.0f;
}

// ----------------------------------------------------------------------------
// load_animating_pointer()
//
// Called at game init to load in the frames for the animating mouse pointer
//
// input:	filename	=>	filename of animation file that holds the animation
// 
void load_animating_pointer(char *filename, int dx, int dy)
{
	int				fps;
	animating_obj *am;

	init_animating_pointer();

//TEMP
	mprintf(("loading animated cursor \"%s\"\n", filename));


	am = &Animating_mouse;
	am->first_frame = bm_load_animation(filename, &am->num_frames, &fps);
	if ( am->first_frame == -1 ) 
		Error(LOCATION, "Could not load animation %s for the mouse pointer\n", filename);
	am->current_frame = 0;
	am->time = am->num_frames / i2fl(fps);
}

// ----------------------------------------------------------------------------
// unload_animating_pointer()
//
// Called at game shutdown to free the memory used to store the animation frames
//
void unload_animating_pointer()
{
	int				i;
	animating_obj	*am;

	am = &Animating_mouse;
	for ( i = 0; i < am->num_frames; i++ ) {
		Assert( (am->first_frame+i) >= 0 );

		// if we are the current cursor then reset to avoid gr_close() issues - taylor
		gr_unset_cursor_bitmap(am->first_frame + i);
	}

	// this will release all of the frames at once
	if (am->first_frame >= 0)
		bm_release(am->first_frame);

	am->first_frame	= -1;
	am->num_frames		= 0;
	am->current_frame = -1;
}

// draw the correct frame of the game mouse... called from game_maybe_draw_mouse()
void game_render_mouse(float frametime)
{
	int				mx, my;
	animating_obj	*am;

	// if animating cursor exists, play the next frame
	am = &Animating_mouse;
	if ( am->first_frame != -1 ) {
		mouse_get_pos(&mx, &my);
		am->elapsed_time += frametime;
		am->current_frame = fl2i( ( am->elapsed_time / am->time ) * (am->num_frames-1) );
		if ( am->current_frame >= am->num_frames ) {
			am->current_frame = 0;
			am->elapsed_time = 0.0f;
		}
		gr_set_cursor_bitmap(am->first_frame + am->current_frame);
	}
}

// ----------------------------------------------------------------------------
// game_maybe_draw_mouse()
//
// determines whether to draw the mouse pointer at all, and what frame of
// animation to use if the mouse is animating
//
// Sets mouse.cpp globals Mouse_hidden and Mouse_moved based on the state of the game.
//
// input:	frametime => elapsed frame time in seconds since last call
//
void game_maybe_draw_mouse(float frametime)
{
	int game_state;

	game_state = gameseq_get_state();

	switch ( game_state ) {
		case GS_STATE_GAME_PAUSED:
		// case GS_STATE_MULTI_PAUSED:
		case GS_STATE_GAME_PLAY:
		case GS_STATE_DEATH_DIED:
		case GS_STATE_DEATH_BLEW_UP:
			if ( popup_active() || popupdead_is_active() ) {
				Mouse_hidden = 0;
			} else {
				Mouse_hidden = 1;	
			}
			break;

		default:
			Mouse_hidden = 0;
			break;
	}	// end switch

	if ( !Mouse_hidden ) 
		game_render_mouse(frametime);

}

void game_do_training_checks()
{
	int i, s;
	float d;
	waypoint_list *wplp;

	if (Training_context & TRAINING_CONTEXT_SPEED) {
		s = (int) Player_obj->phys_info.fspeed;
		if ((s >= Training_context_speed_min) && (s <= Training_context_speed_max)) {
			if (!Training_context_speed_set) {
				Training_context_speed_set = 1;
				Training_context_speed_timestamp = timestamp();
			}

		} else
			Training_context_speed_set = 0;
	}

	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		wplp = Training_context_path;
		if (wplp->get_waypoints().size() > (uint) Training_context_goal_waypoint) {
			i = Training_context_goal_waypoint;
			Warning(LOCATION, "The following is very inefficient with the new waypoint code!  Contact Goober5000 if you need to use it.");
			do {
				waypoint *wpt = find_waypoint_at_index(wplp, i);
				Assert(wpt != NULL);
				d = vm_vec_dist(wpt->get_pos(), &Player_obj->pos);
				if (d <= Training_context_distance) {
					Training_context_at_waypoint = i;
					if (Training_context_goal_waypoint == i) {
						Training_context_goal_waypoint++;
						snd_play(&Snds[SND_CARGO_REVEAL], 0.0f);
					}

					break;
				}

				i++;
				if ((uint) i == wplp->get_waypoints().size())
					i = 0;

			} while (i != Training_context_goal_waypoint);
		}
	}

	if ((Players_target == UNINITIALIZED) || (Player_ai->target_objnum != Players_target) || (Player_ai->targeted_subsys != Players_targeted_subsys)) {
		Players_target = Player_ai->target_objnum;
		Players_targeted_subsys = Player_ai->targeted_subsys;
		Players_target_timestamp = timestamp();
	}
	// following added by Sesquipedalian for is_missile_locked
	if ((Players_mlocked == UNINITIALIZED) || (Player_ai->current_target_is_locked != Players_mlocked)) {
		Players_mlocked = Player_ai->current_target_is_locked;
		Players_mlocked_timestamp = timestamp();
	}

}

/////////// Following is for event debug view screen

#ifndef NDEBUG

#define EVENT_DEBUG_MAX	5000
#define EVENT_DEBUG_EVENT 0x8000

int Event_debug_index[EVENT_DEBUG_MAX];
int ED_count;

void game_add_event_debug_index(int n, int indent)
{
	if (ED_count < EVENT_DEBUG_MAX)
		Event_debug_index[ED_count++] = n | (indent << 16);
}

void game_add_event_debug_sexp(int n, int indent)
{
	if (n < 0)
		return;

	if (Sexp_nodes[n].first >= 0) {
		game_add_event_debug_sexp(Sexp_nodes[n].first, indent);
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent);
		return;
	}

	game_add_event_debug_index(n, indent);
	if (Sexp_nodes[n].subtype == SEXP_ATOM_OPERATOR)
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent + 1);
	else
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent);
}

void game_event_debug_init()
{
	int e;

	ED_count = 0;
	for (e=0; e<Num_mission_events; e++) {
		game_add_event_debug_index(e | EVENT_DEBUG_EVENT, 0);
		game_add_event_debug_sexp(Mission_events[e].formula, 1);
	}
}

void game_show_event_debug(float frametime) 
{
	char buf[256];
	int i, k, z;
	int font_height, font_width;	
	int y_index, y_max;
	static int scroll_offset = 0;
	
	k = game_check_key();
	if (k)
		switch (k) {
			case KEY_UP:
			case KEY_PAD8:
				scroll_offset--;
				if (scroll_offset < 0)
					scroll_offset = 0;
				break;

			case KEY_DOWN:
			case KEY_PAD2:
				scroll_offset++;
				break;

			case KEY_PAGEUP:
				scroll_offset -= 20;
				if (scroll_offset < 0)
					scroll_offset = 0;
				break;

			case KEY_PAGEDOWN:
				scroll_offset += 20;	// not font-independent, hard-coded since I counted the lines!
				break;

			default:
				gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
				key_flush();
				break;
		} // end switch

	gr_clear();
	gr_set_color_fast(&Color_bright);
	gr_set_font(FONT1);
	gr_printf(0x8000, 5, NOX("EVENT DEBUG VIEW"));

	gr_set_color_fast(&Color_normal);
	gr_set_font(FONT1);
	gr_get_string_size(&font_width, &font_height, NOX("test"));
	y_max = gr_screen.max_h - font_height - 5;
	y_index = 45;

	k = scroll_offset;
	while (k < ED_count) {
		if (y_index > y_max)
			break;

		z = Event_debug_index[k];
		if (z & EVENT_DEBUG_EVENT) {
			z &= 0x7fff;
			sprintf(buf, NOX("%s%s (%s) %s%d %d"), (Mission_events[z].flags & MEF_CURRENT) ? NOX("* ") : "",
				Mission_events[z].name, Mission_events[z].result ? NOX("True") : NOX("False"),
				(Mission_events[z].chain_delay < 0) ? "" : NOX("x "),
				Mission_events[z].repeat_count, Mission_events[z].interval);

		} else {
			i = (z >> 16) * 3;
			buf[i] = 0;
			while (i--)
				buf[i] = ' ';

			strcat_s(buf, Sexp_nodes[z & 0x7fff].text);
			switch (Sexp_nodes[z & 0x7fff].value) {
				case SEXP_TRUE:
					strcat_s(buf, NOX(" (True)"));
					break;

				case SEXP_FALSE:
					strcat_s(buf, NOX(" (False)"));
					break;

				case SEXP_KNOWN_TRUE:
					strcat_s(buf, NOX(" (Always true)"));
					break;

				case SEXP_KNOWN_FALSE:
					strcat_s(buf, NOX(" (Always false)"));
					break;

				case SEXP_CANT_EVAL:
					strcat_s(buf, NOX(" (Can't eval)"));
					break;

				case SEXP_NAN:
				case SEXP_NAN_FOREVER:
					strcat_s(buf, NOX(" (Not a number)"));
					break;
			}
		}

		gr_printf(10, y_index, buf);
		y_index += font_height;
		k++;
	}

	gr_flip();
}

#endif // NDEBUG

#ifndef NDEBUG
FILE * Time_fp;
FILE * Texture_fp;

int Tmap_npixels=0;
int Tmap_num_too_big = 0;
int Num_models_needing_splitting = 0;

void Time_model( int modelnum )
{
//	mprintf(( "Timing ship '%s'\n", si->name ));

	vec3d eye_pos, model_pos;
	matrix eye_orient, model_orient;

	polymodel *pm = model_get( modelnum );

	int l = strlen(pm->filename);
	while( (l>0) )	{
		if ( (l == '/') || (l=='\\') || (l==':'))	{
			l++;
			break;
		}
		l--;
	}
	char *pof_file = &pm->filename[l];

	int model_needs_splitting = 0;

	//fprintf( Texture_fp, "Model: %s\n", pof_file );
	int i;
	for (i=0; i<pm->n_textures; i++ )	{
		char filename[1024];
		ubyte pal[768];
		texture_map *tmap = &pm->maps[i];

		for(int j = 0; j < TM_NUM_TYPES; j++)
		{
			int bmp_num = tmap->textures[j].GetOriginalTexture();
			if ( bmp_num > -1 )	{
				bm_get_palette(bmp_num, pal, filename );		
				int w,h;
				bm_get_info( bmp_num,&w, &h );


				if ( (w > 512) || (h > 512) )	{
					fprintf( Texture_fp, "%s\t%s\t%d\t%d\n", pof_file, filename, w, h );
					Tmap_num_too_big++;
					model_needs_splitting++;
				}
			} else {
				//fprintf( Texture_fp, "\tTexture %d is bogus\n", i );
			}
		}
	}

	if ( model_needs_splitting )	{
		Num_models_needing_splitting++;
	}
	eye_orient = model_orient = vmd_identity_matrix;
	eye_pos = model_pos = vmd_zero_vector;

	eye_pos.xyz.z = -pm->rad*2.0f;

	vec3d eye_to_model;

	vm_vec_sub( &eye_to_model, &model_pos, &eye_pos );
	vm_vector_2_matrix( &eye_orient, &eye_to_model, NULL, NULL );

	fix t1 = timer_get_fixed_seconds();

	angles ta;
	ta.p = ta.b = ta.h = 0.0f; 
	int framecount = 0;

	Tmap_npixels = 0;

	int bitmaps_used_this_frame, bitmaps_new_this_frame;
		
	bm_get_frame_usage(&bitmaps_used_this_frame,&bitmaps_new_this_frame);

	modelstats_num_polys = modelstats_num_verts = 0;

	while( ta.h < PI2 )	{

		matrix m1;
		vm_angles_2_matrix(&m1, &ta );
		vm_matrix_x_matrix( &model_orient, &vmd_identity_matrix, &m1 );

		gr_reset_clip();
//		gr_clear();

		g3_start_frame(1);

		//WMC - I think I can set this to VIEWER_ZOOM_DEFAULT.
		//If it's not appropriate, use cam_get_current()
		g3_set_view_matrix( &eye_pos, &eye_orient, VIEWER_ZOOM_DEFAULT );	

		model_clear_instance( modelnum );
		model_set_detail_level(0);		// use highest detail level
		model_render( modelnum, &model_orient, &model_pos, MR_LOCK_DETAIL);	//|MR_NO_POLYS );

		g3_end_frame();
//		gr_flip();

		framecount++;
		ta.h += 0.1f;

		int k = key_inkey();
		if ( k == KEY_ESC ) {
			exit(1);
		}
	}

	fix t2 = timer_get_fixed_seconds();

	bm_get_frame_usage(&bitmaps_used_this_frame,&bitmaps_new_this_frame);

	modelstats_num_polys /= framecount;
	modelstats_num_verts /= framecount;

	Tmap_npixels /=framecount;


	mprintf(( "'%s' is %.2f FPS\n", pof_file, i2fl(framecount)/f2fl(t2-t1) ));
	fprintf( Time_fp, "\"%s\"\t%.0f\t%d\t%d\t%d\t%d\n", pof_file, i2fl(framecount)/f2fl(t2-t1), bitmaps_used_this_frame, modelstats_num_polys, modelstats_num_verts, Tmap_npixels );
}

int Time_models = 0;
DCF_BOOL( time_models, Time_models )

void Do_model_timings_test()
{
	

	if ( !Time_models ) return;

	mprintf(( "Timing models!\n" ));

	int i;

	ubyte model_used[MAX_POLYGON_MODELS];
	int model_id[MAX_POLYGON_MODELS];
	for (i=0; i<MAX_POLYGON_MODELS; i++ )	{
		model_used[i] = 0;
	}
	
	// Load them all
	for (i=0; i<Num_ship_classes; i++ )	{
		ship_info *sip = &Ship_info[i];

		sip->model_num = model_load(sip->pof_file, 0, NULL);

		model_used[sip->model_num % MAX_POLYGON_MODELS]++;
		model_id[sip->model_num % MAX_POLYGON_MODELS] = sip->model_num;
	}

	Texture_fp = fopen( NOX("ShipTextures.txt"), "wt" );
	if ( !Texture_fp ) return;

	Time_fp = fopen( NOX("ShipTimings.txt"), "wt" );
	if ( !Time_fp ) return;

	fprintf( Time_fp, "Name\tFPS\tTRAM\tPolys\tVerts\tPixels\n" );
//	fprintf( Time_fp, "FPS\tTRAM\tPolys\tVerts\tPixels\n" );
	
	for (i=0; i<MAX_POLYGON_MODELS; i++ )	{
		if ( model_used[i] )	{
			Time_model( model_id[i] );
		}
	}
	
	fprintf( Texture_fp, "Number too big: %d\n", Tmap_num_too_big );
	fprintf( Texture_fp, "Number of models needing splitting: %d\n", Num_models_needing_splitting );
	
	fclose(Time_fp);
	fclose(Texture_fp);

	exit(1);
}
#endif

// Call this function when you want to inform the player that a feature is disabled in this build
void game_feature_disabled_popup()
{
	popup(PF_USE_AFFIRMATIVE_ICON|PF_BODY_BIG, 1, POPUP_OK, XSTR( "Sorry, the requested feature is currently disabled in this build", -1));
}

// format the specified time (fixed point) into a nice string
void game_format_time(fix m_time,char *time_str)
{
	float mtime;
	int hours,minutes,seconds;
	char tmp[10];

	mtime = f2fl(m_time);		

	// get the hours, minutes and seconds	
	hours = (int)(mtime / 3600.0f);
	if(hours > 0){
		mtime -= (3600.0f * (float)hours);
	}
	seconds = (int)mtime%60;
	minutes = (int)mtime/60;			

	// print the hour if necessary
	if(hours > 0){		
		sprintf(time_str,XSTR( "%d:", 201),hours);
		// if there are less than 10 minutes, print a leading 0
		if(minutes < 10){
			strcpy_s(tmp,NOX("0"));
			strcat(time_str,tmp);
		}		
	}	
	
	// print the minutes
	if(hours){
		sprintf(tmp,XSTR( "%d:", 201),minutes);
		strcat(time_str,tmp);
	} else {
		sprintf(time_str,XSTR( "%d:", 201),minutes);
	}

	// print the seconds
	if(seconds < 10){
		strcpy_s(tmp,NOX("0"));
		strcat(time_str,tmp);
	} 
	sprintf(tmp,"%d",seconds);
	strcat(time_str,tmp);
}

//	Stuff version string in *str.
void get_version_string(char *str, int max_size)
{
//XSTR:OFF
	Assert( max_size > 6 );

	#if FS_VERSION_REVIS == 0
		sprintf(str, "FreeSpace 2 Open v%i.%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
	#else
		sprintf(str, "FreeSpace 2 Open v%i.%i.%i.%i", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS);
	#endif

#ifdef INF_BUILD
	strcat_s( str, max_size, " Inferno" );
#endif

#ifndef NDEBUG
	strcat_s( str, max_size, " Debug" );
#endif

	// Lets get some more info in here
	switch(gr_screen.mode)
	{
		case GR_OPENGL:
			strcat_s( str, max_size, " OpenGL" );
			break;
	}

	if (Cmdline_nohtl)
		strcat_s( str, max_size, " non-HT&L" );
}

void get_version_string_short(char *str)
{
	sprintf(str,"v%d.%d", FS_VERSION_MAJOR, FS_VERSION_MINOR);
}

// ----------------------------------------------------------------
//
// Subspace Ambient Sound START
//
// ----------------------------------------------------------------

static int Subspace_ambient_left_channel = -1;
static int Subspace_ambient_right_channel = -1;

// 
void game_start_subspace_ambient_sound()
{
	if ( Subspace_ambient_left_channel < 0 ) {
		Subspace_ambient_left_channel = snd_play_looping(&Snds[SND_SUBSPACE_LEFT_CHANNEL], -1.0f);
	}

	if ( Subspace_ambient_right_channel < 0 ) {
		Subspace_ambient_right_channel = snd_play_looping(&Snds[SND_SUBSPACE_RIGHT_CHANNEL], 1.0f);
	}
}

void game_stop_subspace_ambient_sound()
{
	if ( Subspace_ambient_left_channel >= 0 ) {
		snd_stop(Subspace_ambient_left_channel);
		Subspace_ambient_left_channel = -1;
	}

	if ( Subspace_ambient_right_channel >= 0 ) {
		snd_stop(Subspace_ambient_right_channel);
		Subspace_ambient_right_channel = -1;
	}
}

// ----------------------------------------------------------------
//
// Subspace Ambient Sound END
//
// ----------------------------------------------------------------


// ----------------------------------------------------------------
//
// CDROM detection code START
//
// ----------------------------------------------------------------

#define CD_SIZE_72_MINUTE_MAX			(697000000)

uint game_get_cd_used_space(char *path)
{
#ifdef _WIN32
	uint total = 0;
	char use_path[512] = "";
	char sub_path[512] = "";
	WIN32_FIND_DATA	find;
	HANDLE find_handle;

	// recurse through all files and directories
	strcpy_s(use_path, path);
	strcat_s(use_path, "*.*");
	find_handle = FindFirstFile(use_path, &find);

	// bogus
	if(find_handle == INVALID_HANDLE_VALUE){
		return 0;
	}	

	// whee
	do {
		// subdirectory. make sure to ignore . and ..
		if((find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && stricmp(find.cFileName, ".") && stricmp(find.cFileName, "..")){
			// subsearch
			strcpy_s(sub_path, path);
			strcat_s(sub_path, find.cFileName);
			strcat_s(sub_path, DIR_SEPARATOR_STR);
			total += game_get_cd_used_space(sub_path);	
		} else {
			total += (uint)find.nFileSizeLow;
		}				
	} while(FindNextFile(find_handle, &find));	

	// close
	FindClose(find_handle);

	// total
	return total;
#else
	if (path == NULL) {
		// bail
		mprintf(("NULL path passed to game_get_cd_used_space.\n"));
		return 0;
	}

	STUB_FUNCTION;

	return 0;
#endif // _WIN32
}


// if volume_name is non-null, the CD name must match that
int find_freespace_cd(char *volume_name)
{
#ifdef _WIN32
	char oldpath[MAX_PATH];
	char volume[256];
	int i;
	int cdrom_drive=-1;
	int volume_match = 0;
	_finddata_t find;
	int find_handle;

	GetCurrentDirectory(MAX_PATH, oldpath);

	for (i = 0; i < 26; i++) 
	{
//XSTR:OFF
		char path[]="d:\\";
//XSTR:ON

		path[0] = (char)('A'+i);
		if (GetDriveType(path) == DRIVE_CDROM) {
			cdrom_drive = -3;
			if ( GetVolumeInformation(path, volume, 256, NULL, NULL, NULL, NULL, 0) == TRUE ) {
				nprintf(("CD", "CD volume: %s\n", volume));
			
				// check for any CD volume
				int volume1_present = 0;
				int volume2_present = 0;
				int volume3_present = 0;		

				char full_check[512] = "";

				// look for setup.exe
				strcpy_s(full_check, path);
				strcat_s(full_check, "setup.exe");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume1_present = 1;				
					_findclose(find_handle);				
				}

				// look for intro.mve
				strcpy_s(full_check, path);
				strcat_s(full_check, "intro.mve");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume2_present = 1;
					_findclose(find_handle);						
				}				

				// look for endpart1.mve
				strcpy_s(full_check, path);
				strcat_s(full_check, "endpart1.mve");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume3_present = 1;
					_findclose(find_handle);				
				}				
			
				// see if we have the specific CD we're looking for
				if ( volume_name ) {
					// volume 1
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_1) && volume1_present) {
						volume_match = 1;
					}
					// volume 2
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_2) && volume2_present) {
						volume_match = 1;
					}
					// volume 3
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_3) && volume3_present) {
						volume_match = 1;
					}
				} else {										
					if ( volume1_present || volume2_present || volume3_present ) {
						volume_match = 1;
					}
				}
				
				// here's where we make sure that CD's 2 and 3 are not just ripped - check to make sure its capacity is > 697,000,000 bytes				
				if ( volume_match ){				
					// we don't care about CD1 though. let it be whatever size it wants, since the game will demand CD's 2 and 3 at the proper time
					if(volume2_present || volume3_present) {
						// first step - check to make sure its a cdrom
						if(GetDriveType(path) != DRIVE_CDROM){							
							break;
						}
						// oem not on 80 min cds, so don't check tha size
						// check its size
						uint used_space = game_get_cd_used_space(path);											
						if(used_space < CD_SIZE_72_MINUTE_MAX){							
							break;
						}
					}					

					cdrom_drive = i;
					break;
				}
			}
		}
	}	

	SetCurrentDirectory(oldpath);
	return cdrom_drive;
#else
//	STUB_FUNCTION;

	if (volume_name != NULL) {
		// volume specific checks
		STUB_FUNCTION;
	}

	return -1;
#endif // _WIN32
}

int set_cdrom_path(int drive_num)
{
	int rval;

	if (drive_num < 0) {			//no CD
//		#ifndef NDEBUG
//		strcpy_s(CDROM_dir,"j:\\FreeSpaceCD\\");				//set directory
//		rval = 1;
//		#else
		memset(Game_CDROM_dir, 0, sizeof(Game_CDROM_dir));
		rval = 0;
//		#endif
	} else {
		sprintf(Game_CDROM_dir,NOX("%c:\\"), 'a' + drive_num );			//set directory
		rval = 1;
	}

	return rval;
}

int init_cdrom()
{
	int i, rval;

	//scan for CD, etc.

	rval = 1;

	i = find_freespace_cd();

	rval = set_cdrom_path(i);

	return rval;
}

int Last_cd_label_found = 0;
char Last_cd_label[256];

int game_cd_changed()
{
#ifdef _WIN32
	char label[256];
	int found;
	int changed = 0;
	
	if ( strlen(Game_CDROM_dir) == 0 ) { //-V805
		init_cdrom();
	}

	if ( strlen(Game_CDROM_dir) == 0 ) { //-V805
		return 0;
	}

	found = GetVolumeInformation(Game_CDROM_dir, label, 256, NULL, NULL, NULL, NULL, 0);

	if ( found != Last_cd_label_found )	{
		Last_cd_label_found = found;
		if ( found )	{
			mprintf(( "CD '%s' was inserted\n", label ));
			changed = 1;
		} else {
			mprintf(( "CD '%s' was removed\n", Last_cd_label ));
			changed = 1;
		}
	} else {
		if ( Last_cd_label_found )	{
			if ( !stricmp( Last_cd_label, label ))	{
				//mprintf(( "CD didn't change\n" ));
			} else {
				mprintf(( "CD was changed from '%s' to '%s'\n", Last_cd_label, label ));
				changed = 1;
			}
		} else {
			// none found before, none found now.
			//mprintf(( "still no CD...\n" ));
		}
	}
	
	Last_cd_label_found = found;
	if ( found )	{
		strcpy_s( Last_cd_label, label );
	} else {
		strcpy_s( Last_cd_label, "" );
	}

	return changed;
#else
	STUB_FUNCTION;

	return 0;
#endif // _WIN32
}

// check if _any_ FreeSpace2 CDs are in the drive
// return: 1	=> CD now in drive
//			  0	=>	Could not find CD, they refuse to put it in the drive
int game_do_cd_check(char *volume_name)
{	
#if !defined(GAME_CD_CHECK)
	return 1;
#else
	int cd_present = 0;
	int cd_drive_num;

	int num_attempts = 0;
	int refresh_files = 0;
	while(1) {
		int path_set_ok, popup_rval;

		cd_drive_num = find_freespace_cd(volume_name);
		path_set_ok = set_cdrom_path(cd_drive_num);
		if ( path_set_ok ) {
			cd_present = 1;
			if ( refresh_files ) {
				cfile_refresh();
				refresh_files = 0;
			}
			break;
		}

		// standalone mode
		if(Is_standalone){
			cd_present = 0;
			break;
		} else {
			// no CD found, so prompt user
			popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR( "FreeSpace 2 CD not found\n\nInsert a FreeSpace 2 CD to continue", 202));
			refresh_files = 1;
			if ( popup_rval != 1 ) {
				cd_present = 0;
				break;
			}

			if ( num_attempts++ > 5 ) {
				cd_present = 0;
				break;
			}
		}
	}

	return cd_present;
#endif
}

// check if _any_ FreeSpace2 CDs are in the drive
// return: 1	=> CD now in drive
//			  0	=>	Could not find CD, they refuse to put it in the drive
int game_do_cd_check_specific(char *volume_name, int cdnum)
{	
	int cd_present = 0;
	int cd_drive_num;

	int num_attempts = 0;
	int refresh_files = 0;
	while(1) {
		int path_set_ok, popup_rval;

		cd_drive_num = find_freespace_cd(volume_name);
		path_set_ok = set_cdrom_path(cd_drive_num);
		if ( path_set_ok ) {
			cd_present = 1;
			if ( refresh_files ) {
				cfile_refresh();
				refresh_files = 0;
			}
			break;
		}

		if(Is_standalone){
			cd_present = 0;
			break;
		} else {
			// no CD found, so prompt user
			popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert CD %d", 1468), cdnum);
			refresh_files = 1;
			if ( popup_rval != 1 ) {
				cd_present = 0;
				break;
			}

			if ( num_attempts++ > 5 ) {
				cd_present = 0;
				break;
			}
		}
	}

	return cd_present;
}

// ----------------------------------------------------------------
//
// CDROM detection code END
//
// ----------------------------------------------------------------


// ----------------------------------------------------------------
// Language autodetection stuff
//

// this layout *must* match Lcl_languages in localize.cpp in order for the
// correct language to be detected
int Lang_auto_detect_checksums[LCL_NUM_LANGUAGES] = {
	589986744,						// English
	-1132430286,					// German
	0,								// French
};

// default setting is "-1" to use registry setting with English as fall back
// DO NOT change that default setting here or something uncouth might happen
// in the localization code
int detect_lang()
{
	uint file_checksum;
	int idx;
	char first_font[MAX_FILENAME_LEN];

	// if the reg is set then let lcl_init() figure out what to do
	if (os_config_read_string( NULL, NOX("Language"), NULL ) != NULL)
		return -1;

	// try and open the file to verify
	gr_stuff_first_font(first_font, sizeof(first_font));
	CFILE *detect = cfopen(first_font, "rb");

	// will use default setting if something went wrong
	if (!detect)
		return -1;

	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);
	detect = NULL;

	// now compare the checksum/filesize against known #'s
	for (idx=0; idx<LCL_NUM_LANGUAGES; idx++) {
		if (Lang_auto_detect_checksums[idx] == (int)file_checksum) {
			mprintf(( "AutoLang: Language auto-detection successful...\n" ));
			return idx;
		}
	}

	// notify if a match was not found, include detected checksum
	mprintf(( "ERROR: Unknown Language Checksum: %i\n", (int)file_checksum ));
	mprintf(( "Using default language settings...\n" ));

	return -1;
}

//
// Eng Auto Lang stuff
// ----------------------------------------------------------------


// ----------------------------------------------------------------
// SHIPS TBL VERIFICATION STUFF
//

// checksums, just keep a list of all valid ones, if it matches any of them, keep it
#define NUM_SHIPS_TBL_CHECKSUMS		1

int Game_ships_tbl_checksums[NUM_SHIPS_TBL_CHECKSUMS] = {
//	-1022810006,					// 1.0 FULL
	-1254285366						// 1.2 FULL (German)
};

void verify_ships_tbl()
{	
	/*
#ifdef NDEBUG
	Game_ships_tbl_valid = 1;
#else
	*/
	uint file_checksum;		
	int idx;

	// detect if the packfile exists
	CFILE *detect = cfopen("ships.tbl", "rb");
	Game_ships_tbl_valid = 0;	 
	
	// not mission-disk
	if(!detect){
		Game_ships_tbl_valid = 0;
		return;
	}	

	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);
	detect = NULL;	

	// now compare the checksum/filesize against known #'s
	for(idx=0; idx<NUM_SHIPS_TBL_CHECKSUMS; idx++){
		if(Game_ships_tbl_checksums[idx] == (int)file_checksum){
			Game_ships_tbl_valid = 1;
			return;
		}
	}
// #endif
}

DCF(shipspew, "display the checksum for the current ships.tbl")
{
	uint file_checksum;
	CFILE *detect = cfopen("ships.tbl", "rb");
	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);

	dc_printf("%d", file_checksum);
}

// ----------------------------------------------------------------
// WEAPONS TBL VERIFICATION STUFF
//

// checksums, just keep a list of all valid ones, if it matches any of them, keep it
#define NUM_WEAPONS_TBL_CHECKSUMS		1

int Game_weapons_tbl_checksums[NUM_WEAPONS_TBL_CHECKSUMS] = {
//	399297860,				// 1.0 FULL	
	-553984927				// 1.2 FULL (german)
};

void verify_weapons_tbl()
{
	/*
#ifdef NDEBUG
	Game_weapons_tbl_valid = 1;
#else
	*/
	uint file_checksum;
	int idx;

	// detect if the packfile exists
	CFILE *detect = cfopen("weapons.tbl", "rb");
	Game_weapons_tbl_valid = 0;	 
	
	// not mission-disk
	if(!detect){
		Game_weapons_tbl_valid = 0;
		return;
	}	

	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);
	detect = NULL;	

	// now compare the checksum/filesize against known #'s
	for(idx=0; idx<NUM_WEAPONS_TBL_CHECKSUMS; idx++){
		if(Game_weapons_tbl_checksums[idx] == (int)file_checksum){
			Game_weapons_tbl_valid = 1;
			return;
		}
	}
// #endif
}

DCF(wepspew, "display the checksum for the current weapons.tbl")
{
	uint file_checksum;
	CFILE *detect = cfopen("weapons.tbl", "rb");
	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);

	dc_printf("%d", file_checksum);
}

// if the game is running using hacked data
static bool Hacked_data_check_ready = false;
static bool Hacked_data = false;

int game_hacked_data()
{
	int rc = 0;

	if (Hacked_data) {
		return 1;
	}


	if ( Om_tracker_flag && !(Hacked_data_check_ready) ) {
		// this may fail the first time or two
		if ( (rc = fs2netd_update_valid_tables()) != -1 ) {
			Hacked_data = (rc != 0);
			Hacked_data_check_ready = true;
		}
	}

	// LAN game, only check if weapon and ship are valid since we can't or don't
	// want to validate against PXO server
	if ( !Om_tracker_flag && !(Game_weapons_tbl_valid && Game_ships_tbl_valid) ) {
		Hacked_data = true;
	}


	return (int)Hacked_data;
}


//#define MAX_SPLASHSCREENS 64
//char Splash_screens[MAX_SPLASHSCREENS][MAX_FILENAME_LEN];


void game_title_screen_display()
{
/*	_finddata_t find;
	long		find_handle;
	char current_dir[256];

	//Get the find string
	_getcwd(current_dir, 256);
	strcat_s(current_dir, DIR_SEPARATOR_STR);
	strcat_s(current_dir, "*.pcx");

	//Let the search begin!
	find_handle = _findfirst(current_dir, &find);
	int i = 0;
	if(find_handle != -1)
	{
		char *p;

		do {
			if(!(find.attrib & _A_SUBDIR) && (strlen(find.name) < MAX_FILENAME_LEN)) {
				p = strchr( find.name, '.' );
				if(p) {
					*p = '\0';
				}

				if(stricmp(find.name, Game_logo_screen_fname[gr_screen.res])
					&& stricmp(find.name, Game_title_screen_fname[gr_screen.res]))
				{
					strcpy_s(Splash_screens[i], find.name);
					i++;
				}

				if(i == MAX_SPLASHSCREENS) {
					break;
				}
			}
		} while(!_findnext(find_handle, &find));
	}

	if(i) {
		srand(time(NULL));
		title_bitmap = bm_load(Splash_screens[rand() % i]);

	} else {
		title_bitmap = bm_load(Game_title_screen_fname[gr_screen.res]);
	}
	
	if (title_bitmap == -1 && title_logo == -1) {
//		return;
	}
	*/

	//Script_system.SetHookVar("SplashScreenImage", 's', Game_title_screen_fname[gr_screen.res]);
	//Script_system.SetHookVar("SplashScreenLogo", 's', Game_logo_screen_fname[gr_screen.res]);
	bool globalhook_override = Script_system.IsOverride(Script_splashhook);
	bool condhook_override = Script_system.IsConditionOverride(CHA_SPLASHSCREEN);
	mprintf(("SCRIPTING: Splash screen overrides checked\n"));
	if(!globalhook_override && !condhook_override)
	{
		Game_title_logo = bm_load(Game_logo_screen_fname[gr_screen.res]);
		Game_title_bitmap = bm_load(Game_title_screen_fname[gr_screen.res]);

		if (Game_title_bitmap != -1)
		{
			// set
			gr_set_bitmap(Game_title_bitmap);

			// get bitmap's width and height
			int width, height;
			bm_get_info(Game_title_bitmap, &width, &height);

			// draw it in the center of the screen
			gr_bitmap((gr_screen.max_w_unscaled - width)/2, (gr_screen.max_h_unscaled - height)/2);
		}

		if (Game_title_logo != -1)
		{
			gr_set_bitmap(Game_title_logo);

			gr_bitmap(0, 0);

		}
	}

	if(!condhook_override)
		Script_system.RunBytecode(Script_splashhook);
	
	mprintf(("SCRIPTING: Splash hook has been run\n"));

	if(!globalhook_override || condhook_override)
		Script_system.RunCondition(CHA_SPLASHSCREEN);
		
	mprintf(("SCRIPTING: Splash screen conditional hook has been run\n"));
		
	Script_system.RemHookVars(2, "SplashScreenImage", "SplashScreenLogo");

	// flip
	gr_flip();
}

void game_title_screen_close()
{
	if (Game_title_bitmap != -1) {
		bm_release(Game_title_bitmap);
		Game_title_bitmap = -1;
	}

	if (Game_title_logo != -1) {
		bm_release(Game_title_logo);
		Game_title_bitmap = -1;
	}
}

// return true if the game is running with "low memory", which is less than 48MB
bool game_using_low_mem()
{
	if (Use_low_mem == 0) {
		return false;
	} else {
		return true;
	}
}

//  place calls here that need to take effect immediately when the game is
//  minimized.  Called from osapi.cpp 
void game_pause()
{
	// Protection against flipping out -- Kazan
	if (!GameState_Stack_Valid())
		return;

	if (!(Game_mode & GM_MULTIPLAYER)){
		switch ( gameseq_get_state() )
		{
			case GS_STATE_MAIN_MENU:
				main_hall_pause(); // not an instant shutoff of misc anims and sounds
				break;

			case GS_STATE_BRIEFING:
				brief_pause();
				break;

			case GS_STATE_DEBRIEF:
				debrief_pause();
				break;

			case GS_STATE_CMD_BRIEF:
				cmd_brief_pause();
				break;

			case GS_STATE_RED_ALERT:
				red_alert_voice_pause();
				break;

			// anything that would leave the ambient mainhall sound going
			case GS_STATE_TECH_MENU:
			case GS_STATE_BARRACKS_MENU:
				main_hall_stop_ambient();
				main_hall_stop_music(); // not an instant shutoff
				break;

			// things that would get music except if they are called while in-mission
			case GS_STATE_OPTIONS_MENU:
			case GS_STATE_HUD_CONFIG:
				if ( !(Game_mode & GM_IN_MISSION) ) {
					main_hall_stop_ambient();
					main_hall_stop_music(); // not an instant shutoff
				}
				break;

			// only has the ambient sound, no music
			case GS_STATE_INITIAL_PLAYER_SELECT:
				main_hall_stop_ambient();
				break;

			// pause_init is a special case and we don't unpause it ourselves
			case GS_STATE_GAME_PLAY:
				if ( (!popup_active()) && (!popupdead_is_active()) )
					pause_init();
				break;

			default:
				audiostream_pause_all();
		}
	}
}

// calls to be executed when the game is restored from minimized or inactive state
void game_unpause()
{
	if (!GameState_Stack_Valid())
		return;

	// automatically recover from everything but an in-mission pause
	if (!(Game_mode & GM_MULTIPLAYER)) {
		switch ( gameseq_get_state() )
		{
			case GS_STATE_MAIN_MENU:
				main_hall_unpause();
				break;

			case GS_STATE_BRIEFING:
				brief_unpause();
				break;

			case GS_STATE_DEBRIEF:
				debrief_unpause();
				break;

			case GS_STATE_CMD_BRIEF:
				cmd_brief_unpause();
				break;

			case GS_STATE_RED_ALERT:
				red_alert_voice_unpause();
				break;

			// anything that would leave the ambient mainhall sound going
			case GS_STATE_TECH_MENU:
			case GS_STATE_BARRACKS_MENU:
				main_hall_start_ambient();
				main_hall_start_music(); // not an instant shutoff
				break;

			// things that would get music except if they are called while in-mission
			case GS_STATE_OPTIONS_MENU:
			case GS_STATE_HUD_CONFIG:
				if ( !(Game_mode & GM_IN_MISSION) ) {
					main_hall_start_ambient();
					main_hall_start_music(); // not an instant shutoff
				}
				break;

			// only has the ambient sound, no music
			case GS_STATE_INITIAL_PLAYER_SELECT:
				main_hall_start_ambient();
				break;

			// if in a game then do nothing, pause_init() should have been called
			// and will get cleaned up elsewhere
			case GS_STATE_GAME_PLAY:
				break;

			default:
				audiostream_unpause_all();
		}
	}
}
