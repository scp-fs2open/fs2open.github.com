/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "camera/camera.h" //VIEWER_ZOOM_DEFAULT
#include "cmdline/cmdline.h"
#include "globalincs/linklist.h"
#include "globalincs/systemvars.h"
#include "network/multi.h"
#include "hud/hudconfig.h"
#include "parse/scripting.h"
#include "parse/sexp.h"
#include "globalincs/version.h"
#include "globalincs/pstypes.h"

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#elif defined(APPLE_APP)
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef SCP_UNIX
#include "osapi/osapi.h"
#endif

#include <string.h>
#include <stdlib.h>

// variables
class cmdline_parm {
public:
	cmdline_parm *next, *prev;
	char *name;						// name of parameter, must start with '-' char
	char *help;						// help text for this parameter
	bool stacks;					// whether this arg stacks with each use or is replaced by newest use (should only be used for strings!!)
	char *args;						// string value for parameter arguements (NULL if no arguements)
	int name_found;				// true if parameter on command line, otherwise false

	cmdline_parm(char *name, char *help, bool stacks = false);
	~cmdline_parm();
	int found();
	int get_int();
	float get_float();
	char *str();
	bool check_if_args_is_valid();
};

static cmdline_parm Parm_list(NULL, NULL);
static int Parm_list_inited = 0;

extern int Show_framerate;	// from freespace.cpp


enum
{
	// DO NOT CHANGE ANYTHING ABOUT THESE FIRST TWO OR WILL MESS UP THE LAUNCHER
	EASY_DEFAULT  =  1 << 1,
	EASY_ALL_ON   =  1 << 2,

	EASY_MEM_ON   =  1 << 3,
	EASY_MEM_OFF  =  1 << 4,

	// Add new flags here

	// Combos
	EASY_MEM_ALL_ON  = EASY_ALL_ON  | EASY_MEM_ON,
	EASY_DEFAULT_MEM = EASY_DEFAULT | EASY_MEM_OFF
};

#define BUILD_CAP_OPENAL	(1<<0)
#define BUILD_CAP_NO_D3D	(1<<1)
#define BUILD_CAP_NEW_SND	(1<<2)

#define PARSE_COMMAND_LINE_STRING	"-parse_cmdline_only"

typedef struct
{
	// DO NOT CHANGE THE SIZE OF THIS STRING!
	char name[32];

} EasyFlag;

EasyFlag easy_flags[] =
{
	{ "Custom" },
	{ "Default FS2 (All features off)" },
	{ "All features on" },
	{ "High memory usage features on" },
	{ "High memory usage features off" }
};

// DO NOT CHANGE **ANYTHING** ABOUT THIS STRUCTURE AND ITS CONTENT
typedef struct
{
	char  name[20];		// The actual flag
	char  desc[40];		// The text that will appear in the launcher (unless its blank, other name is shown)
	bool  fso_only;		// true if this is a fs2_open only feature
	int   on_flags;		// Easy flag which will turn this feature on
	int   off_flags;	// Easy flag which will turn this feature off
	char  type[16];		// Launcher uses this to put flags under different headings
	char  web_url[256];	// Link to documentation of feature (please use wiki or somewhere constant)

} Flag;

// Please group them by type, ie graphics, gameplay etc, maximum 20 different types
Flag exe_params[] = 
{
	{ "-spec",				"Enable specular",							true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-spec", },
	{ "-glow",				"Enable glow maps",							true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-glow", },
	{ "-env",				"Enable environment maps",					true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-env", },
	{ "-mipmap",			"Enable mipmapping",						true,	0,					EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-mipmap", },
	{ "-nomotiondebris",	"Disable motion debris",					true,	EASY_ALL_ON,		EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomotiondebris",},
	{ "-noscalevid",		"Disable scale-to-window for movies",		true,	0,					EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noscalevid", },
	{ "-missile_lighting",	"Apply lighting to missiles"	,			true,	EASY_ALL_ON,		EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-missile_lighting", },
	{ "-normal",			"Enable normal maps",						true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-normal" },
	{ "-3dshockwave",		"Enable 3D shockwaves",						true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-3dshockwave" },
	{ "-post_process",		"Enable post processing",					true,	0,					EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-post_process" },
	{ "-soft_particles",	"Enable soft particles",					true,	0,					EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-soft_particles" },
	{ "-fxaa",				"Enable FXAA anti-aliasing",				true,	0,					EASY_DEFAULT,		"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fxaa" },

	{ "-img2dds",			"Compress non-compressed images",			true,	0,					EASY_DEFAULT,		"Game Speed",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-img2dds", },
	{ "-no_vsync",			"Disable vertical sync",					true,	0,					EASY_DEFAULT,		"Game Speed",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_vsync", },
	{ "-cache_bitmaps",		"Cache bitmaps between missions",			true,	0,					EASY_DEFAULT_MEM,	"Game Speed",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-cache_bitmaps", },

	{ "-dualscanlines",		"Add another pair of scanning lines",		true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dualscanlines", },
	{ "-targetinfo",		"Enable info next to target",				true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-targetinfo", },
	{ "-orbradar",			"Enable 3D radar",							true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-orbradar", },
	{ "-rearm_timer",		"Enable rearm/repair completion timer",		true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-rearm_timer", },
	{ "-ballistic_gauge",	"Enable analog ballistic ammo gauge",		true,	0,					EASY_DEFAULT,		"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ballistic_gauge", },

	{ "-ship_choice_3d",	"Use 3D models for ship selection",			true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ship_choice_3d", },
	{ "-weapon_choice_3d",	"Use 3D models for weapon selection",		true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-weapon_choice_3d", },
	{ "-3dwarp",			"Enable 3D warp",							true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-3dwarp", },
	{ "-warp_flash",		"Enable flash upon warp",					true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-warp_flash", },
	{ "-no_ap_interrupt",	"Disable interrupting autopilot",			true,	0,					EASY_DEFAULT,		"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_ap_interrupt", },

	{ "-snd_preload",		"Preload mission game sounds",				true,	EASY_MEM_ALL_ON,	EASY_DEFAULT_MEM,	"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-snd_preload", },
	{ "-nosound",			"Disable all sound",						false,	0,					EASY_DEFAULT,		"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nosound", },
	{ "-nomusic",			"Disable music",							false,	0,					EASY_DEFAULT,		"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomusic", },

	{ "-standalone",		"Run as standalone server",					false,	0,					EASY_DEFAULT,		"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-standalone", },
	{ "-startgame",			"Skip mainhall and start hosting",			false,	0,					EASY_DEFAULT,		"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-startgame", },
	{ "-closed",			"Start hosted server as closed",			false,	0,					EASY_DEFAULT,		"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-closed", },
	{ "-restricted",		"Host confirms join requests",				false,	0,					EASY_DEFAULT,		"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-restricted", },
	{ "-multilog",			"",											false,	0,					EASY_DEFAULT,		"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-multilog", },
	{ "-clientdamage",		"",											false,	0,					EASY_DEFAULT,		"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-clientdamage", },
	{ "-mpnoreturn",		"Disable flight deck option",				true,	0,					EASY_DEFAULT,		"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-mpnoreturn", },

	{ "-nohtl",				"Software mode (very slow)",				true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nohtl", },
	{ "-no_set_gamma",		"Disable setting of gamma",					true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_set_gamma", },
	{ "-nomovies",			"Disable video playback",					true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomovies", },
	{ "-noparseerrors",		"Disable parsing errors",					true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noparseerrors", },
	{ "-query_speech",		"Check if this build has speech",			true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-query_speech", },
	{ "-novbo",				"Disable OpenGL VBO",						true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-novbo", },
	{ "-noibx",				"Don't use cached index buffers (IBX)",		true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noibx", },
	{ "-loadallweps",		"Load all weapons, even those not used",	true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-loadallweps", },
	{ "-disable_fbo",		"Disable OpenGL RenderTargets",				true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-disable_fbo", },
	{ "-no_glsl",			"Disable GLSL (shader) support",			true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_glsl", },
	{ "-ati_swap",			"Fix colour issues on some ATI cards",		true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://scp.indiegames.us/mantis/view.php?id=1669", },
	{ "-no_3d_sound",		"Use only 2D/stereo for sound effects",		true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_3d_sound", },
	{ "-disable_glsl_model","Don't use shaders for model rendering",	true,	0,					EASY_DEFAULT,		"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-disable_glsl_model", },
 #ifndef SCP_UNIX
	{ "-disable_di_mouse",	"Don't use DirectInput for mouse control",	true,	0,					EASY_DEFAULT,		"Troubleshoot",	"", },
 #endif
	{ "-use_gldrawelements","Don't use glDrawRangeElements",			true,	0,					EASY_DEFAULT,		"Troubleshoot",	"", },

	{ "-ingame_join",		"Allow in-game joining",					true,	0,					EASY_DEFAULT,		"Experimental",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ingame_join", },
	{ "-voicer",			"Enable voice recognition",					true,	0,					EASY_DEFAULT,		"Experimental",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-voicer", },
	{ "-fb_explosions",		"Enable Framebuffer Shockwaves",			true,	0,					EASY_DEFAULT,		"Experimental",	"", },

	{ "-fps",				"Show frames per second on HUD",			false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fps", },
	{ "-pos",				"Show position of camera",					false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-pos", },
	{ "-window",			"Run in window",							true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-window", },
 #ifndef SCP_UNIX
	{ "-fullscreen_window",	"Run in fullscreen window",					false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fullscreen_window", },
 #endif
	{ "-stats",				"Show statistics",							true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-stats", },
	{ "-coords",			"Show coordinates",							false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-coords", },
	{ "-show_mem_usage",	"Show memory usage",						true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-show_mem_usage", },
	{ "-pofspew",			"",											false,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-pofspew", },
	{ "-tablecrcs",			"",											true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-tablecrcs", },
	{ "-missioncrcs",		"",											true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-missioncrcs", },
	{ "-dis_collisions",	"Disable collisions",						true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dis_collisions", },
	{ "-dis_weapons",		"Disable weapon rendering",					true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dis_weapons", },
	{ "-output_sexps",		"Output SEXPs to sexps.html",				true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-output_sexps", },
	{ "-output_scripting",	"Output scripting to scripting.html",		true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-output_scripting", },
	{ "-save_render_target",	"Save render targets to file",			true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-save_render_target", },
	{ "-debug_window",		"Display debug window",						true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-debug_window", },
	{ "-verify_vps",		"Spew VP CRCs to vp_crcs.txt",				true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-verify_vps", },
 #ifdef SCP_UNIX
	{ "-nograb",			"Don't grab mouse/keyboard in a window",	true,	0,					EASY_DEFAULT,		"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nograb", },
 #endif
};

// here are the command line parameters that we will be using for FreeSpace

// RETAIL options ----------------------------------------------
cmdline_parm connect_arg("-connect", NULL);			// Cmdline_connect_addr
cmdline_parm gamename_arg("-gamename", NULL);		// Cmdline_game_name
cmdline_parm gamepassword_arg("-password", NULL);	// Cmdline_game_password
cmdline_parm allowabove_arg("-allowabove", NULL);	// Cmdline_rank_above
cmdline_parm allowbelow_arg("-allowbelow", NULL);	// Cmdline_rank_below
cmdline_parm standalone_arg("-standalone", NULL);
cmdline_parm nosound_arg("-nosound", NULL);			// Cmdline_freespace_no_sound
cmdline_parm nomusic_arg("-nomusic", NULL);			// Cmdline_freespace_no_music
cmdline_parm startgame_arg("-startgame", NULL);		// Cmdline_start_netgame
cmdline_parm gameclosed_arg("-closed", NULL);		// Cmdline_closed_game
cmdline_parm gamerestricted_arg("-restricted", NULL);	// Cmdline_restricted_game
cmdline_parm port_arg("-port", NULL);
cmdline_parm multilog_arg("-multilog", NULL);		// Cmdline_multi_log
cmdline_parm client_dodamage("-clientdamage", NULL);	// Cmdline_client_dodamage
cmdline_parm pof_spew("-pofspew", NULL);			// Cmdline_spew_pof_info
cmdline_parm mouse_coords("-coords", NULL);			// Cmdline_mouse_coords
cmdline_parm timeout("-timeout", NULL);				// Cmdline_timeout
cmdline_parm bit32_arg("-32bit", NULL);				// (only here for retail compatibility reasons, doesn't actually do anything)

char *Cmdline_connect_addr = NULL;
char *Cmdline_game_name = NULL;
char *Cmdline_game_password = NULL;
char *Cmdline_rank_above = NULL;
char *Cmdline_rank_below = NULL;
int Cmdline_cd_check = 1;
int Cmdline_client_dodamage = 0;
int Cmdline_closed_game = 0;
int Cmdline_freespace_no_music = 0;
int Cmdline_freespace_no_sound = 0;
int Cmdline_gimme_all_medals = 0;
int Cmdline_mouse_coords = 0;
int Cmdline_multi_log = 0;
int Cmdline_multi_stream_chat_to_file = 0;
int Cmdline_network_port = -1;
int Cmdline_restricted_game = 0;
int Cmdline_spew_pof_info = 0;
int Cmdline_start_netgame = 0;
int Cmdline_timeout = -1;
int Cmdline_use_last_pilot = 0;


// FSO options -------------------------------------------------

// Graphics related
cmdline_parm spec_exp_arg("-spec_exp", NULL);		// comand line FOV -Bobboau
cmdline_parm clip_dist_arg("-clipdist", NULL);		// Cmdline_clip_dist
cmdline_parm fov_arg("-fov", NULL);					// Cmdline_fov  -- comand line FOV -Bobboau
cmdline_parm ogl_spec_arg("-ogl_spec", NULL);		// Cmdline_ogl_spec
cmdline_parm spec_static_arg("-spec_static", NULL);
cmdline_parm spec_point_arg("-spec_point", NULL);
cmdline_parm spec_tube_arg("-spec_tube", NULL);
cmdline_parm ambient_factor_arg("-ambient_factor", NULL);	// Cmdline_ambient_factor
cmdline_parm env("-env", NULL);						// Cmdline_env
cmdline_parm mipmap_arg("-mipmap", NULL);			// Cmdline_mipmap
cmdline_parm missile_lighting_arg("-missile_lighting", NULL);	// Cmdline_missile_lighting
cmdline_parm glow_arg("-glow", NULL); 				// Cmdline_glow  -- use Bobs glow code
cmdline_parm nomotiondebris_arg("-nomotiondebris", NULL); // Cmdline_nomotiondebris  -- Removes those ugly floating rocks -C
cmdline_parm noscalevid_arg("-noscalevid", NULL);	// Cmdline_noscalevid  -- disable video scaling that fits to window
cmdline_parm spec_arg("-spec", NULL);				// Cmdline_spec  -- use specular highlighting -Sticks
cmdline_parm noemissive_arg("-no_emissive_light", NULL);		// Cmdline_no_emissive  -- don't use emissive light in OGL
cmdline_parm normal_arg("-normal", NULL);				// Cmdline_normal  -- enable normal mapping
cmdline_parm height_arg("-height", NULL);			// Cmdline_height  -- enable support for parallax mapping
cmdline_parm enable_3d_shockwave_arg("-3dshockwave", NULL);
cmdline_parm softparticles_arg("-soft_particles", NULL);
cmdline_parm postprocess_arg("-post_process", NULL);
cmdline_parm bloom_intensity_arg("-bloom_intensity", NULL);
cmdline_parm fxaa_arg("-fxaa", NULL);
cmdline_parm fxaa_preset_arg("-fxaa_preset", NULL);
cmdline_parm fb_explosions_arg("-fb_explosions", NULL);

float Cmdline_clip_dist = Default_min_draw_distance;
float Cmdline_fov = 0.75f;
float Cmdline_ogl_spec = 80.0f;
int Cmdline_ambient_factor = 128;
int Cmdline_env = 0;
int Cmdline_mipmap = 0;
int Cmdline_missile_lighting = 0;
int Cmdline_glow = 0;
int Cmdline_nomotiondebris = 0;
int Cmdline_noscalevid = 0;
int Cmdline_spec = 0;
int Cmdline_no_emissive = 0;
int Cmdline_normal = 0;
int Cmdline_height = 0;
int Cmdline_enable_3d_shockwave = 0;
int Cmdline_softparticles = 0;
int Cmdline_postprocess = 0;
int Cmdline_bloom_intensity = 75;
bool Cmdline_fxaa = false;
int Cmdline_fxaa_preset = 6;
extern int Fxaa_preset_last_frame;
bool Cmdline_fb_explosions = 0;

// Game Speed related
cmdline_parm cache_bitmaps_arg("-cache_bitmaps", NULL);	// Cmdline_cache_bitmaps
cmdline_parm img2dds_arg("-img2dds", NULL);			// Cmdline_img2dds
cmdline_parm no_fpscap("-no_fps_capping", NULL);	// Cmdline_NoFPSCap
cmdline_parm no_vsync_arg("-no_vsync", NULL);		// Cmdline_no_vsync

int Cmdline_cache_bitmaps = 0;	// caching of bitmaps between missions (faster loads, can hit swap on reload with <512 Meg RAM though) - taylor
int Cmdline_img2dds = 0;
int Cmdline_NoFPSCap = 0; // Disable FPS capping - kazan
int Cmdline_no_vsync = 0;

// HUD related
cmdline_parm ballistic_gauge("-ballistic_gauge", NULL);	// Cmdline_ballistic_gauge
cmdline_parm dualscanlines_arg("-dualscanlines", NULL); // Cmdline_dualscanlines  -- Change to phreaks options including new targetting code
cmdline_parm orb_radar("-orbradar", NULL);			// Cmdline_orb_radar
cmdline_parm rearm_timer_arg("-rearm_timer", NULL);	// Cmdline_rearm_timer
cmdline_parm targetinfo_arg("-targetinfo", NULL);	// Cmdline_targetinfo  -- Adds ship name/class to right of target box -C

int Cmdline_ballistic_gauge = 0;	// WMCoolmon's gauge thingy
int Cmdline_dualscanlines = 0;
int Cmdline_orb_radar = 0;
int Cmdline_rearm_timer = 0;
int Cmdline_targetinfo = 0;

// Gameplay related
cmdline_parm use_3dwarp("-3dwarp", NULL);			// Cmdline_3dwarp
cmdline_parm ship_choice_3d_arg("-ship_choice_3d", NULL);	// Cmdline_ship_choice_3d
cmdline_parm weapon_choice_3d_arg("-weapon_choice_3d", NULL);	// Cmdline_weapon_choice_3d
cmdline_parm use_warp_flash("-warp_flash", NULL);	// Cmdline_warp_flash
cmdline_parm allow_autpilot_interrupt("-no_ap_interrupt", NULL);	// Cmdline_warp_flash

int Cmdline_3dwarp = 0;
int Cmdline_ship_choice_3d = 0;
int Cmdline_weapon_choice_3d = 0;
int Cmdline_warp_flash = 0;
int Cmdline_autopilot_interruptable = 1;

// Audio related
cmdline_parm query_speech_arg("-query_speech", NULL);	// Cmdline_query_speech
cmdline_parm snd_preload_arg("-snd_preload", NULL);		// Cmdline_snd_preload
cmdline_parm voice_recognition_arg("-voicer", NULL);	// Cmdline_voice_recognition

int Cmdline_query_speech = 0;
int Cmdline_snd_preload = 0; // preload game sounds during mission load
int Cmdline_voice_recognition = 0;

// MOD related
cmdline_parm mod_arg("-mod", NULL, true);	// Cmdline_mod  -- DTP modsupport

char *Cmdline_mod = NULL; //DTP for mod arguement

// Multiplayer/Network related
cmdline_parm almission_arg("-almission", NULL);		// Cmdline_almission  -- DTP for autoload Multi mission
cmdline_parm ingamejoin_arg("-ingame_join", NULL);	// Cmdline_ingamejoin
cmdline_parm mpnoreturn_arg("-mpnoreturn", NULL);	// Cmdline_mpnoreturn  -- Removes 'Return to Flight Deck' in respawn dialog -C
cmdline_parm missioncrcspew_arg("-missioncrcs", NULL);		// Cmdline_spew_mission_crcs
cmdline_parm tablecrcspew_arg("-tablecrcs", NULL);			// Cmdline_spew_table_crcs
cmdline_parm objupd_arg("-cap_object_update", NULL);

char *Cmdline_almission = NULL;	//DTP for autoload multi mission.
int Cmdline_ingamejoin = 0;
int Cmdline_mpnoreturn = 0;
char *Cmdline_spew_mission_crcs = NULL;
char *Cmdline_spew_table_crcs = NULL;
int Cmdline_objupd = 3;		// client object updates on LAN by default

// Troubleshooting
cmdline_parm loadallweapons_arg("-loadallweps", NULL);	// Cmdline_load_all_weapons
cmdline_parm htl_arg("-nohtl", NULL);				// Cmdline_nohtl  -- don't use HT&L
cmdline_parm noibx_arg("-noibx", NULL);				// Cmdline_noibx
cmdline_parm nomovies_arg("-nomovies", NULL);		// Cmdline_nomovies  -- Allows video streaming
cmdline_parm no_set_gamma_arg("-no_set_gamma", NULL);	// Cmdline_no_set_gamma
cmdline_parm no_vbo_arg("-novbo", NULL);			// Cmdline_novbo
cmdline_parm no_fbo_arg("-disable_fbo", NULL);		// Cmdline_no_fbo
cmdline_parm noglsl_arg("-no_glsl", NULL);			// Cmdline_noglsl  -- disable GLSL support in OpenGL
cmdline_parm atiswap_arg("-ati_swap", NULL);        // Cmdline_atiswap - Fix ATI color swap issue for screenshots.
cmdline_parm no3dsound_arg("-no_3d_sound", NULL);		// Cmdline_no_3d_sound - Disable use of full 3D sounds
cmdline_parm no_glsl_models_arg("-disable_glsl_model", NULL); // Cmdline_no_glsl_model_rendering -- switches model rendering to fixed pipeline
cmdline_parm no_di_mouse_arg("-disable_di_mouse", NULL); // Cmdline_no_di_mouse -- Disables directinput use for mouse control
cmdline_parm no_drawrangeelements("-use_gldrawelements", NULL); // Cmdline_drawelements -- Uses glDrawElements instead of glDrawRangeElements
cmdline_parm keyboard_layout("-keyboard_layout", NULL);

int Cmdline_load_all_weapons = 0;
int Cmdline_nohtl = 0;
int Cmdline_noibx = 0;
int Cmdline_nomovies = 0;
int Cmdline_no_set_gamma = 0;
int Cmdline_novbo = 0; // turn off OGL VBO support, troubleshooting
int Cmdline_no_fbo = 0;
int Cmdline_noglsl = 0;
int Cmdline_ati_color_swap = 0;
int Cmdline_no_3d_sound = 0;
int Cmdline_no_glsl_model_rendering = 0;
int Cmdline_no_di_mouse = 0;
int Cmdline_drawelements = 0;
char* Cmdline_keyboard_layout = NULL;

// Developer/Testing related
cmdline_parm start_mission_arg("-start_mission", NULL);	// Cmdline_start_mission
cmdline_parm dis_collisions("-dis_collisions", NULL);	// Cmdline_dis_collisions
cmdline_parm dis_weapons("-dis_weapons", NULL);		// Cmdline_dis_weapons
cmdline_parm noparseerrors_arg("-noparseerrors", NULL);	// Cmdline_noparseerrors  -- turns off parsing errors -C
cmdline_parm nowarn_arg("-no_warn", NULL);			// Cmdline_nowarn
cmdline_parm extra_warn_arg("-extra_warn", NULL);	// Cmdline_extra_warn
cmdline_parm fps_arg("-fps", NULL);					// Cmdline_show_fps
cmdline_parm show_mem_usage_arg("-show_mem_usage", NULL);	// Cmdline_show_mem_usage
cmdline_parm pos_arg("-pos", NULL);					// Cmdline_show_pos
cmdline_parm stats_arg("-stats", NULL);				// Cmdline_show_stats
cmdline_parm save_render_targets_arg("-save_render_target", NULL);	// Cmdline_save_render_targets
cmdline_parm debug_window_arg("-debug_window", NULL);	// Cmdline_debug_window
cmdline_parm window_arg("-window", NULL);				// Cmdline_window
cmdline_parm fullscreen_window_arg("-fullscreen_window",NULL);
cmdline_parm res_arg("-res", NULL);					// Cmdline_lores
cmdline_parm verify_vps_arg("-verify_vps", NULL);	// Cmdline_verify_vps  -- spew VP crcs to vp_crcs.txt
cmdline_parm parse_cmdline_only(PARSE_COMMAND_LINE_STRING, NULL); 
#ifdef SCP_UNIX
cmdline_parm no_grab("-nograb", NULL);				// Cmdline_no_grab
#endif

char *Cmdline_start_mission = NULL;
int Cmdline_dis_collisions = 0;
int Cmdline_dis_weapons = 0;
int Cmdline_noparseerrors = 0;
int Cmdline_nowarn = 0; // turn warnings off in FRED
int Cmdline_extra_warn = 0;
int Cmdline_show_mem_usage = 0;
int Cmdline_show_pos = 0;
int Cmdline_show_stats = 0;
int Cmdline_save_render_targets = 0;
int Cmdline_debug_window = 0;
int Cmdline_window = 0;
int Cmdline_fullscreen_window = 0;
char *Cmdline_res = 0;
int Cmdline_verify_vps = 0;
#ifdef SCP_UNIX
int Cmdline_no_grab = 0;
#endif

// Other
cmdline_parm get_flags_arg("-get_flags", NULL);
cmdline_parm output_sexp_arg("-output_sexps", NULL); //WMC - outputs all SEXPs to sexps.html
cmdline_parm output_scripting_arg("-output_scripting", NULL);	//WMC



#ifndef NDEBUG
// NOTE: this assumes that os_init() has already been called but isn't a fatal error if it hasn't
void cmdline_debug_print_cmdline()
{
	cmdline_parm *parmp;
	int found = 0;

	mprintf(("Passed cmdline options:"));

	for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
		if ( parmp->name_found ) {
			if ( parmp->args != NULL ) {
				mprintf(("\n  %s %s", parmp->name, parmp->args));
			} else {
				mprintf(("\n  %s", parmp->name));
			}
			found++;
		}
	}

	if ( !found )
		mprintf(("\n  <none>"));

	mprintf(("\n"));
}
#endif

//	Return true if this character is an extra char (white space and quotes)
int is_extra_space(char ch)
{
	return ((ch == ' ') || (ch == '\t') || (ch == 0x0a) || (ch == '\'') || (ch == '\"'));
}


// eliminates all leading and trailing extra chars from a string.  Returns pointer passed in.
char *drop_extra_chars(char *str)
{
	int s, e;

	s = 0;
	while (str[s] && is_extra_space(str[s]))
		s++;

	e = strlen(str) - 1;	// we already account for NULL later on, so the -1 is here to make
							// sure we do our math without taking it into consideration

	if (e < 0)
		e = 0;

	while (e > s) {
		if (!is_extra_space(str[e])){
			break;
		}

		e--;
	}

	if (e >= s && e !=0 ){
		memmove(str, str + s, e - s + 1);
	}

	str[e - s + 1] = 0;
	return str;
}


// internal function - copy the value for a parameter agruement into the cmdline_parm arg field
void parm_stuff_args(cmdline_parm *parm, char *cmdline)
{
	char buffer[1024];
	memset( buffer, 0, sizeof( buffer ) );
	char *dest = buffer;
	char *saved_args = NULL;

	cmdline += strlen(parm->name);

	while ((*cmdline != '\0') && strncmp(cmdline, " -", 2) && ((size_t)(dest-buffer) < sizeof(buffer))) {
		*dest++ = *cmdline++;
	}

	drop_extra_chars(buffer);

	// mwa 9/14/98 -- made it so that newer command line arguments found will overwrite the old arguments
	// taylor 7/25/06 -- made it so that you can stack newer arguments if that option should support stacking

	if ( parm->args != NULL ) {
		if (parm->stacks) {
			saved_args = parm->args;
		} else {
			delete[] parm->args;
		}

		parm->args = NULL;
	}

	int size = strlen(buffer);

	if (size > 0) {
		size++;	// nul char

		if (saved_args != NULL) {
			size += (strlen(saved_args) + 1);	// an ',' is used as a separator when combining, so be sure to account for it
		}

		parm->args = new char[size];
		memset(parm->args, 0, size);

		if (saved_args != NULL) {
			// saved args go first, then new arg
			strcpy_s(parm->args, size, saved_args);
			// add a separator too, so that we can tell the args apart
			strcat_s(parm->args, size, ",");
			// now the new arg
			strcat_s(parm->args, size, buffer);

			delete [] saved_args;
		} else {
			strcpy_s(parm->args, size, buffer);
		}
	} else {
		parm->args = saved_args;
	}
}


// internal function - parse the command line, extracting parameter arguements if they exist
// cmdline - command line string passed to the application
void os_parse_parms(char *cmdline)
{
	// locate command line parameters
	cmdline_parm *parmp;
	char *cmdline_offset = NULL;
	size_t get_new_offset = 0;

	for (parmp = GET_FIRST(&Parm_list); parmp != END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp)) {
		// continue processing every option on the line to make sure that we account for every listing of each option
		do {
			// while going through the cmdline make sure to grab only the option that we are
			// looking for, but if one similar then keep searching for the exact match
			while (true) {
				cmdline_offset = strstr(cmdline + get_new_offset, parmp->name);

				if ( !cmdline_offset )
					break;

				int parmp_len = strlen(parmp->name);

				// the new offset should be our currently location + the length of the current option
				get_new_offset = (strlen(cmdline) - strlen(cmdline_offset) + parmp_len);

				if ( (*(cmdline_offset + parmp_len)) && !is_extra_space(*(cmdline_offset + parmp_len)) ) {
					// we found a similar, but not exact, match for this option, continue checking for the correct one
				} else {
					// we found what we were looking for so break out and process it
					break;
				}
			}

			if (cmdline_offset) {
				parmp->name_found = 1;
				parm_stuff_args(parmp, cmdline_offset);
			}
		} while (cmdline_offset);

		// reset the offset for the next param that we will look for
		get_new_offset = 0;
		cmdline_offset = NULL;
	}
}


// validate the command line parameters.  Display an error if an unrecognized parameter is located.
void os_validate_parms(char *cmdline)
{
	cmdline_parm *parmp;
	char seps[] = " ,\t\n";
	char *token;
	int parm_found;

	token = strtok(cmdline, seps);
	while(token != NULL) {
	
		if (token[0] == '-') {
			parm_found = 0;
			for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
				if (!stricmp(parmp->name, token)) {
					parm_found = 1;
					break;
				}
			}

			if (parm_found == 0) {
#ifdef _WIN32
				// Changed this to MessageBox, this is a user error not a developer
				char buffer[128];
				sprintf(buffer,"Unrecogzined command line parameter %s, continue?",token);
				if( MessageBox(NULL, buffer, "Warning", MB_OKCANCEL | MB_ICONQUESTION) == IDCANCEL)
					exit(0);
#elif defined(APPLE_APP)
				CFStringRef message;
				char buffer[128];
				CFOptionFlags result;

				snprintf(buffer, 128, "Unrecognized command line parameter, \"%s\", continue?", token);
				message = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingASCII);

				if ( CFUserNotificationDisplayAlert(0, kCFUserNotificationPlainAlertLevel, NULL, NULL, NULL, CFSTR("Unknown Command"), message, NULL, CFSTR("Quit"), NULL, &result) )
					exit(0);

				if (result != kCFUserNotificationDefaultResponse)
					exit(0);
#else
				// if we got a -help, --help, -h, or -? then show the help text, otherwise show unknown option
				if ( !stricmp(token, "-help") || !stricmp(token, "--help") || !stricmp(token, "-h") || !stricmp(token, "-?") ) {
					if (FS_VERSION_REVIS == 0) {
						printf("FreeSpace 2 Open, version %i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD);
					} else {
						printf("FreeSpace 2 Open, version %i.%i.%i.%i\n", FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD, FS_VERSION_REVIS);
					}
					printf("Website: http://scp.indiegames.us\n");
					printf("Mantis (bug reporting): http://scp.indiegames.us/mantis/\n\n");
					printf("Usage: fs2_open [options]\n");

					// not the prettiest thing but the job gets done
					static const int STR_SIZE = 25;  // max len of exe_params.name + 5 spaces
					int p=0, sp=0;
					while (exe_params[p].name[0] == '-') {
						sp = strlen(exe_params[p].name);
						printf("    [ %s ]%*s- %s\n", exe_params[p].name, (STR_SIZE - sp - 1), NOX(" "), exe_params[p].desc);
						p++;
					}

					printf("\n");
					exit(0);
				} else {
					printf("Unrecognized command line parameter \"%s\".  Ignoring...\n", token);
				}
#endif
			}
		}

		token = strtok(NULL, seps);
	}
}


// Call once to initialize the command line system
//
// cmdline - command line string passed to the application
void os_init_cmdline(char *cmdline)
{
	FILE *fp;

	bool parse_config = true;
	
	if (strstr(cmdline, PARSE_COMMAND_LINE_STRING) != NULL) {
		parse_config =  false;
	}

	// read the cmdline.cfg file from the data folder, and pass the command line arguments to
	// the the parse_parms and validate_parms line.  Read these first so anything actually on
	// the command line will take precedence
#ifdef _WIN32
	fp = fopen("data\\cmdline_fso.cfg", "rt");
#elif defined(APPLE_APP)
	extern char full_path[1024];
	char *c = NULL, data_path[1024];

	c = strstr(full_path, ".app");
	if ( c != NULL ) {
		while (c && (*c != '/'))
			c--;
		
		*c = '\0';
	}
	snprintf(data_path, 1024, "%s/data/cmdline_fso.cfg", full_path);

	fp = fopen(data_path, "rt");
#else
	fp = fopen("data/cmdline_fso.cfg", "rt");
#endif

	// if the file exists, get a single line, and deal with it
	if ( fp && parse_config ) {
		char *buf, *p;

		size_t len = filelength( fileno(fp) ) + 2;
		buf = new char [len];

		fgets(buf, len-1, fp);

		// replace the newline character with a NULL
		if ( (p = strrchr(buf, '\n')) != NULL ) {
			*p = '\0';
		}

#ifdef SCP_UNIX
		// append a space for the os_parse_parms() check
		strcat_s(buf, len, " ");
#endif

		os_parse_parms(buf);
		os_validate_parms(buf);
		delete [] buf;
		fclose(fp);
	}

#ifdef SCP_UNIX
	// parse user specific cmdline config file (will supersede options in global file)
	char cmdname[MAX_PATH];

	snprintf(cmdname, MAX_PATH, "%s/%s/data/cmdline_fso.cfg", detect_home(), Osreg_user_dir);
	fp = fopen(cmdname, "rt");

	if ( !fp ) {
		// try for non "_fso", for older code versions
		snprintf(cmdname, MAX_PATH, "%s/%s/data/cmdline.cfg", detect_home(), Osreg_user_dir);
		fp = fopen(cmdname, "rt");
	}

	// if the file exists, get a single line, and deal with it
	if ( fp ) {
		char *buf, *p;

		size_t len = filelength( fileno(fp) ) + 2;
		buf = new char [len];

		fgets(buf, len-1, fp);

		// replace the newline character with a NULL
		if ( (p = strrchr(buf, '\n')) != NULL ) {
			*p = '\0';
		}

		// append a space for the os_parse_parms() check
		strcat_s(buf, len, " ");

		os_parse_parms(buf);
		os_validate_parms(buf);
		delete [] buf;
		fclose(fp);
	}
#endif

	os_parse_parms(cmdline);
	os_validate_parms(cmdline);
}


// arg constructor
// name_ - name of the parameter, must start with '-' character
// help_ - help text for this parameter
cmdline_parm::cmdline_parm(char *name_, char *help_, bool stacks_)
{
	name = name_;
	help = help_;
	stacks = stacks_;
	args = NULL;
	name_found = 0;

	if (Parm_list_inited == 0) {
		list_init(&Parm_list);
		Parm_list_inited = 1;
	}

	if (name != NULL) {
		list_append(&Parm_list, this);
	}
}


// destructor - frees any allocated memory
cmdline_parm::~cmdline_parm()
{
#ifndef FRED
	if (args) {
		delete [] args;
		args = NULL;
	}
#endif
}

// checks if the objects args variable is valid
// returns true if it is, shows an error box and returns false if not valid.
bool cmdline_parm::check_if_args_is_valid() {
	if ( args == NULL ) {
		Error(__FILE__, __LINE__, 
			"Command line flag passed that requires an argument, but the argument is missing!\r\n"
			"The flag is '%s', make sure that you have an argument that follows it.\r\n"
			"You may need to close your launcher and remove the flag manually from %s/data/cmdline_fso.cfg\r\n",
			name, "<Freespace directory>");
		return false;
	} else {
		return true;
	}
}


// returns - true if the parameter exists on the command line, otherwise false
int cmdline_parm::found()
{
	return name_found;
}

// returns - the interger representation for the parameter arguement
int cmdline_parm::get_int()
{
	check_if_args_is_valid();

	int offset = 0;

	if (stacks) {
		// first off, DON'T STACK NON-STRINGS!!
		Int3();

		// secondly, we still need to get it right for the users sake...
		char *moron = strstr(args, ",");

		if ( moron && ((strlen(moron) + 1) < strlen(args)) ) {
			// we get the last arg, since it's the newest one
			offset = strlen(args) - strlen(moron) + 1;
		}
	}

	return atoi(args+offset);
}


// returns - the float representation for the parameter arguement
float cmdline_parm::get_float()
{
	check_if_args_is_valid();

	int offset = 0;

	if (stacks) {
		// first off, DON'T STACK NON-STRINGS!!
		Int3();

		// secondly, we still need to get it right for the users sake
		char *moron = strstr(args, ",");

		if ( moron && ((strlen(moron) + 1) < strlen(args)) ) {
			// we get the last arg, since it's the newest one
			offset = strlen(args) - strlen(moron) + 1;
		}
	}

	return (float)atof(args+offset);
}


// returns - the string value for the parameter arguement
char *cmdline_parm::str()
{
	check_if_args_is_valid();

	return args;
}

// external entry point into this modules

bool SetCmdlineParams()
// Sets externed variables used for communication cmdline information
{
	//getcwd(FreeSpace_Directory, 256); // set the directory to our fs2 root
	if (no_fpscap.found())
	{
		Cmdline_NoFPSCap = 1;
	}

	if(loadallweapons_arg.found())
	{
		Cmdline_load_all_weapons = 1;
	}

	if(voice_recognition_arg.found())
	{
		Cmdline_voice_recognition = 1;
	}

	if (nowarn_arg.found())
	{
		Cmdline_nowarn = 1;
	}

	if (extra_warn_arg.found())
	{
		Cmdline_extra_warn = 1;
	}

	if ( missioncrcspew_arg.found() ) {
		Cmdline_spew_mission_crcs = missioncrcspew_arg.str();

		// strip off blank space at end if it's there
		if ( Cmdline_spew_mission_crcs[strlen(Cmdline_spew_mission_crcs)-1] == ' ' ) {
			Cmdline_spew_mission_crcs[strlen(Cmdline_spew_mission_crcs)-1] = '\0';
		}
 	}
 
	if ( tablecrcspew_arg.found() ) {
		Cmdline_spew_table_crcs = tablecrcspew_arg.str();

		// strip off blank space at end if it's there
		if ( Cmdline_spew_table_crcs[strlen(Cmdline_spew_table_crcs)-1] == ' ' ) {
			Cmdline_spew_table_crcs[strlen(Cmdline_spew_table_crcs)-1] = '\0';
		}
 	}

	if (!Fred_running) { //There is no standalone FRED
		// is this a standalone server??
		if (standalone_arg.found()) {
			Is_standalone = 1;
		}
	}

	// object update control
	if(objupd_arg.found()){
		Cmdline_objupd = objupd_arg.get_int();
		if (Cmdline_objupd < 0)
		{
			Cmdline_objupd = 0;
		}
		if (Cmdline_objupd > 3)
		{
			Cmdline_objupd = 3;
		}
	}

	if(mpnoreturn_arg.found()) {
		Cmdline_mpnoreturn = 1;
	}

	// run with no sound
	if ( nosound_arg.found() ) {
		Cmdline_freespace_no_sound = 1;
		// and since music is automatically unusable...
		Cmdline_freespace_no_music = 1; 
	}

	// run with no music
	if ( nomusic_arg.found() ) {
		Cmdline_freespace_no_music = 1;
	}

	// should we start a network game
	if ( startgame_arg.found() ) {
		Cmdline_use_last_pilot = 1;
		Cmdline_start_netgame = 1;
	}

	// closed network game
	if ( gameclosed_arg.found() ) {
		Cmdline_closed_game = 1;
	}

	// restircted network game
	if ( gamerestricted_arg.found() ) {
		Cmdline_restricted_game = 1;
	}

	// get the name of the network game
	if ( gamename_arg.found() ) {
		Cmdline_game_name = gamename_arg.str();

		// be sure that this string fits in our limits
		if ( strlen(Cmdline_game_name) > MAX_GAMENAME_LEN ) {
			Cmdline_game_name[MAX_GAMENAME_LEN-1] = '\0';
		}
	}

	// get the password for a pssword game
	if ( gamepassword_arg.found() ) {
		Cmdline_game_password = gamepassword_arg.str();

		// be sure that this string fits in our limits
		if ( strlen(Cmdline_game_name) > MAX_PASSWD_LEN ) {
			Cmdline_game_name[MAX_PASSWD_LEN-1] = '\0';
		}
	}

	// set the rank above/below arguments
	if ( allowabove_arg.found() ) {
		Cmdline_rank_above = allowabove_arg.str();
	}
	if ( allowbelow_arg.found() ) {
		Cmdline_rank_below = allowbelow_arg.str();
	}

	// get the port number for games
	if ( port_arg.found() ) {
		Cmdline_network_port = port_arg.get_int();
	}

	// the connect argument specifies to join a game at this particular address
	if ( connect_arg.found() ) {
		Cmdline_use_last_pilot = 1;
		Cmdline_connect_addr = connect_arg.str();
	}

	// see if the multilog flag was set
	if ( multilog_arg.found() ){
		Cmdline_multi_log = 1;
	}	


	// maybe use old-school client damage
	if(client_dodamage.found()){
		Cmdline_client_dodamage = 1;
	}	

	// spew pof info
	if(pof_spew.found()){
		Cmdline_spew_pof_info = 1;
	}

	// mouse coords
	if(mouse_coords.found()){
		Cmdline_mouse_coords = 1;
	}

	// net timeout
	if(timeout.found()){
		Cmdline_timeout = timeout.get_int();
	}

	// d3d windowed
	if(window_arg.found()){
		Cmdline_window = 1;
	}

	if ( fullscreen_window_arg.found( ) )
	{
#ifdef WIN32
		Cmdline_fullscreen_window = 1;
		Cmdline_window = 0; /* Make sure no-one sets both */
#endif
	}

	if(res_arg.found()){
		Cmdline_res = res_arg.str();
	}
	if(almission_arg.found()){//DTP for autoload mission // developer oritentated
		Cmdline_almission = almission_arg.str();
		Cmdline_use_last_pilot = 1;
		Cmdline_start_netgame = 1;
	}

	if(dualscanlines_arg.found() ) {
		Cmdline_dualscanlines = 1;
	}

	if(targetinfo_arg.found())
	{
		Cmdline_targetinfo = 1;
	}

	if(nomovies_arg.found() ) {
		Cmdline_nomovies = 1;
	}

	if ( noscalevid_arg.found() ) {
		Cmdline_noscalevid = 1;
	}

	if(noparseerrors_arg.found()) {
		Cmdline_noparseerrors = 1;
	}


	if(mod_arg.found() ) {
		Cmdline_mod = mod_arg.str();

		// be sure that this string fits in our limits
		/* This has to be disabled because the max size is going to be mods*MAX_FILENAME_LEN
		if ( strlen(Cmdline_mod) > MAX_FILENAME_LEN ) {
			Cmdline_mod[MAX_FILENAME_LEN-1] = '\0';
		}*/

		// strip off blank space it it's there
		if ( Cmdline_mod[strlen(Cmdline_mod)-1] == ' ' ) {
			Cmdline_mod[strlen(Cmdline_mod)-1] = '\0';
		}

		// Ok - mod stacking support
		int len = strlen(Cmdline_mod);
		char *modlist = new char[len+2];
		memset( modlist, 0, len + 2 );
		strcpy_s(modlist, len+2, Cmdline_mod);

		//modlist[len]= '\0'; // double null termination at the end

		// null terminate each individual
		for (int i = 0; i < len; i++)
		{
			if (modlist[i] == ',')
				modlist[i] = '\0';
		}
		
		//copy over - we don't have to delete[] Cmdline_mod because it's a pointer to an automatic global char*
		Cmdline_mod = modlist;
	}


	if (fps_arg.found())
	{
		Show_framerate = 1;
	}

	if(pos_arg.found())
	{
		Cmdline_show_pos = 1;
	}

	if ( nomotiondebris_arg.found() ) {
		Cmdline_nomotiondebris = 1;
	}

	if( mipmap_arg.found() ) {
		Cmdline_mipmap = 1;
	}

	if( stats_arg.found() ) {
		Cmdline_show_stats = 1;
	}

	if ( fov_arg.found() ) {
		Cmdline_fov = fov_arg.get_float();
		if (Cmdline_fov > 0.1) {
			VIEWER_ZOOM_DEFAULT = Cmdline_fov;
		} else {
			VIEWER_ZOOM_DEFAULT = Cmdline_fov = 0.75f;
		}
	}

	if( clip_dist_arg.found() ) {
		Min_draw_distance = Cmdline_clip_dist = clip_dist_arg.get_float();
	}

	if (orb_radar.found())
	{
		Cmdline_orb_radar = 1;
	}

	if ( use_3dwarp.found() ) {
		Cmdline_3dwarp = 1;
	}

	if ( use_warp_flash.found() ) {
		Cmdline_warp_flash = 1;
	}

	if ( allow_autpilot_interrupt.found() )	{
		Cmdline_autopilot_interruptable = 0;
	}
	// specular comand lines
	if ( spec_exp_arg.found() ) {
		specular_exponent_value = spec_exp_arg.get_float();
	}

	if ( spec_point_arg.found() ) {
		static_point_factor = spec_point_arg.get_float();
	}

	if ( spec_static_arg.found() ) {
		static_light_factor = spec_static_arg.get_float();
	}

	if ( spec_tube_arg.found() ) {
		static_tube_factor = spec_tube_arg.get_float();
	}

	if ( spec_arg.found() )
	{
		Cmdline_spec = 1;
	}

	if ( htl_arg.found() ) 
	{
		Cmdline_nohtl = 1;
	}

	if( no_set_gamma_arg.found() )
	{
		Cmdline_no_set_gamma = 1;
	}

	if(no_vsync_arg.found() )
	{
		Cmdline_no_vsync = 1;
	}

#ifdef SCP_UNIX
	// no key/mouse grab
	if(no_grab.found()){
		Cmdline_no_grab = 1;
	}
#endif

	if ( normal_arg.found() ) {
		Cmdline_normal = 1;

		// height maps are only used if normal maps are
		if ( height_arg.found() ) {
			Cmdline_height = 1;
		}
	}

	if ( noglsl_arg.found() ) {
		Cmdline_noglsl = 1;
	}

	if (no_glsl_models_arg.found() ) {
		Cmdline_no_glsl_model_rendering = 1;
	}

	if (fxaa_arg.found() ) {
		Cmdline_fxaa = true;

		if (fxaa_preset_arg.found()) {
			Cmdline_fxaa_preset = fxaa_preset_arg.get_int();
		}

		Fxaa_preset_last_frame = Cmdline_fxaa_preset;
	}

	if (no_di_mouse_arg.found() ) {
		Cmdline_no_di_mouse = 1;
	}

	if ( img2dds_arg.found() )
		Cmdline_img2dds = 1;

	if ( glow_arg.found() )
		Cmdline_glow = 1;

	if ( query_speech_arg.found() )
		Cmdline_query_speech = 1;

	if ( ship_choice_3d_arg.found() )
		Cmdline_ship_choice_3d = 1;

    if ( weapon_choice_3d_arg.found() )
        Cmdline_weapon_choice_3d = 1;

	if ( show_mem_usage_arg.found() )
		Cmdline_show_mem_usage = 1;

	if (ingamejoin_arg.found() )
		Cmdline_ingamejoin = 1;

	if ( start_mission_arg.found() ) {
		Cmdline_start_mission = start_mission_arg.str();
	}

	if ( ambient_factor_arg.found() )
		Cmdline_ambient_factor = ambient_factor_arg.get_int();

	if ( get_flags_arg.found() ) {
		FILE *fp = fopen("flags.lch","w");

		if (fp == NULL) {
			MessageBox(NULL,"Error creating flag list for launcher", "Error", MB_OK);
			return false; 
		}

		int easy_flag_size	= sizeof(EasyFlag);
		int flag_size		= sizeof(Flag);

		int num_easy_flags	= sizeof(easy_flags) / easy_flag_size;
		int num_flags		= sizeof(exe_params) / flag_size;

		// Launcher will check its using structures of the same size
		fwrite(&easy_flag_size, sizeof(int), 1, fp);
		fwrite(&flag_size, sizeof(int), 1, fp);

		fwrite(&num_easy_flags, sizeof(int), 1, fp);
		fwrite(&easy_flags, sizeof(easy_flags), 1, fp);

		fwrite(&num_flags, sizeof(int), 1, fp);
		fwrite(&exe_params, sizeof(exe_params), 1, fp);

		{
			// cheap and bastardly cap check for builds
			// (needs to be compatible with older Launchers, which means having
			//  this implies an OpenAL build for old Launchers)
			ubyte build_caps = 0;

			/* portej05 defined this always */
			build_caps |= BUILD_CAP_OPENAL;
			build_caps |= BUILD_CAP_NO_D3D;
			build_caps |= BUILD_CAP_NEW_SND;


			fwrite(&build_caps, 1, 1, fp);
		}

		fflush(fp);
		fclose(fp);

		return false; 
	}

	if ( output_scripting_arg.found() )
		Output_scripting_meta = true;

	if (output_sexp_arg.found() ) {
		output_sexps("sexps.html");
	}

	if ( no_vbo_arg.found() )
	{
		Cmdline_novbo = 1;
	}

	if ( no_drawrangeelements.found() )
	{
		Cmdline_drawelements = 1;
	}

	if( keyboard_layout.found())
	{
		Cmdline_keyboard_layout = keyboard_layout.str();
	}

	if ( snd_preload_arg.found() )
	{
		Cmdline_snd_preload = 1;
	}

	if ( env.found() ) {
		Cmdline_env = 1;
	}

	if ( ballistic_gauge.found() ) {
		Cmdline_ballistic_gauge = 1;
	}

	if ( cache_bitmaps_arg.found() ) {
		Cmdline_cache_bitmaps = 1;
	}

	if(dis_collisions.found())
		Cmdline_dis_collisions = 1;

	if(dis_weapons.found())
		Cmdline_dis_weapons = 1;

	if ( noibx_arg.found() ) {
		Cmdline_noibx = 1;
	}

	if ( no_fbo_arg.found() ) {
		Cmdline_no_fbo = 1;
	}

	if ( noemissive_arg.found() ) {
		Cmdline_no_emissive = 1;
	}

	if ( ogl_spec_arg.found() ) {
		Cmdline_ogl_spec = ogl_spec_arg.get_float();

		if ( Cmdline_ogl_spec < 0.0f )
			Cmdline_ogl_spec = 0.0f;

		if ( Cmdline_ogl_spec > 128.0f )
			Cmdline_ogl_spec = 128.0f;
	}

	if ( rearm_timer_arg.found() )
		Cmdline_rearm_timer = 1;

	if ( missile_lighting_arg.found() )
		Cmdline_missile_lighting = 1;

	if ( save_render_targets_arg.found() )
		Cmdline_save_render_targets = 1;

	if ( debug_window_arg.found() )
		Cmdline_debug_window = 1;

	if ( verify_vps_arg.found() )
		Cmdline_verify_vps = 1;

	if ( no3dsound_arg.found() )
		Cmdline_no_3d_sound = 1;

    if ( atiswap_arg.found() )
    {
        Cmdline_ati_color_swap = 1;
    }

	if ( enable_3d_shockwave_arg.found() )
	{
		Cmdline_enable_3d_shockwave = 1;
	}

	if ( softparticles_arg.found() )
	{
		Cmdline_softparticles = 1;
	}

	if ( fb_explosions_arg.found() )
	{
		Cmdline_fb_explosions = 1;
	}

	if ( postprocess_arg.found() )
	{
		Cmdline_postprocess = 1;
	}

	if ( bloom_intensity_arg.found() )
	{
		Cmdline_bloom_intensity = bloom_intensity_arg.get_int();
	}

	return true; 
}


int fred2_parse_cmdline(int argc, char *argv[])
{
	if (argc > 1) {
		// kind of silly -- combine arg list into single string for parsing,
		// but it fits with the win32-centric existing code.
		char *cmdline = NULL;
		unsigned int arglen = 0;
		int i;
		for (i = 1;  i < argc;  i++)
			arglen += strlen(argv[i]);
		if (argc > 2)
			arglen += argc + 2; // leave room for the separators
		cmdline = new char [arglen+1];
		i = 1;

		strcpy_s(cmdline, arglen+1, argv[i]);
		for (i=2; i < argc;  i++) {
			strcat_s(cmdline, arglen+1, " ");
			strcat_s(cmdline, arglen+1, argv[i]);
		}
		os_init_cmdline(cmdline);
		delete [] cmdline;
	} else {
		// no cmdline args
		os_init_cmdline("");
	}

	return SetCmdlineParams();
}


int parse_cmdline(char *cmdline)
{
//	mprintf(("I got to parse_cmdline()!!\n"));

	os_init_cmdline(cmdline);

	// --------------- Kazan -------------
	// If you're looking for the list of if (someparam.found()) { cmdline_someparam = something; } look above at this function
	// I did this because of fred2_parse_cmdline()
	return SetCmdlineParams();
}
