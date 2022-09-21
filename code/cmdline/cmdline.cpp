/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "cmdline/cmdline.h"
#include "camera/camera.h" //VIEWER_ZOOM_DEFAULT
#include "cfile/cfilesystem.h"
#include "fireball/fireballs.h"
#include "globalincs/linklist.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "globalincs/version.h"
#include "graphics/shadows.h"
#include "hud/hudconfig.h"
#include "io/joy.h"
#include "network/multi.h"
#include "network/multi_log.h"
#include "options/OptionsManager.h"
#include "osapi/osapi.h"
#include "osapi/dialogs.h"
#include "parse/sexp.h"
#include "scripting/scripting.h"
#include "sound/openal.h"
#include "sound/speech.h"
#include "starfield/starfield.h"

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#elif defined(APPLE_APP)
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef SCP_UNIX
#include "osapi/osapi.h"
#include <dirent.h>
#endif

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sstream>

#include <jansson.h>

// Stupid windows workaround...
#ifdef MessageBox
#undef MessageBox
#endif

enum cmdline_arg_type
{
	AT_NONE       =0,
	AT_INT,
	AT_FLOAT,
	AT_STRING
};
// values and order MUST match cmdline_arg_type
const char *cmdline_arg_types[] =
{
	"NONE",
	"INT",
	"FLOAT",
	"STRING",
};

enum class flag_output_type {
	Binary,
	Json_V1,
};

// variables
class cmdline_parm {
public:
	cmdline_parm *next, *prev;
	const char *name;						// name of parameter, must start with '-' char
	const char *help;						// help text for this parameter
	const bool stacks;					// whether this arg stacks with each use or is replaced by newest use (should only be used for strings!!)
	char *args;						// string value for parameter arguments (NULL if no arguments)
	int name_found;				// true if parameter on command line, otherwise false
	const int arg_type;					// from enum cmdline_arg_type; used for help

	cmdline_parm(const char *name, const char *help, const int arg_type, const bool stacks = false) noexcept;
	~cmdline_parm() noexcept;
	int found();
	int get_int();
	float get_float();
	char *str();
	bool check_if_args_is_valid();
	bool has_param();
};

static cmdline_parm Parm_list(NULL, NULL, AT_NONE);
static int Parm_list_inited = 0;

extern int Show_framerate;	// from freespace.cpp

enum
{
	// DO NOT CHANGE ANYTHING ABOUT THESE FIRST TWO OR WILL MESS UP THE LAUNCHER
	EASY_DEFAULT  =  1 << 1,		// Default FS2 (All features off)
	EASY_ALL_ON   =  1 << 2,		// All features on

	EASY_HI_MEM_ON   =  1 << 3,		// High memory usage features on
	EASY_HI_MEM_OFF  =  1 << 4,		// High memory usage features off

	// Add new flags here
};

enum BuildCaps
{
	BUILD_CAPS_OPENAL = (1<<0),
	BUILD_CAPS_NO_D3D = (1<<1),
	BUILD_CAPS_NEW_SND = (1<<2),
	BUILD_CAPS_SDL = (1<<3)
};

#define PARSE_COMMAND_LINE_STRING	"-parse_cmdline_only"
#define GET_FLAGS_STRING			"-get_flags"

typedef struct
{
	// DO NOT CHANGE THE SIZE OF THIS AT_STRING!
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
	int   on_flags;		// "Easy setting" which will turn this option on
	int   off_flags;	// "Easy setting" which will turn this option off
	char  type[16];		// Launcher uses this to put flags under different headings
	char  web_url[256];	// Link to documentation of feature (please use wiki or somewhere constant)

} Flag;

// clang-format off
// Please group them by type, ie graphics, gameplay etc, maximum 20 different types
Flag exe_params[] = 
{
	{ "-nospec",			"Disable specular",							true,	EASY_DEFAULT | EASY_HI_MEM_OFF,		EASY_ALL_ON  | EASY_HI_MEM_ON,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nospec", },
	{ "-noglow",			"Disable glow maps",						true,	EASY_DEFAULT | EASY_HI_MEM_OFF,		EASY_ALL_ON  | EASY_HI_MEM_ON,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noglow", },
	{ "-noenv",				"Disable environment maps",					true,	EASY_DEFAULT | EASY_HI_MEM_OFF,		EASY_ALL_ON  | EASY_HI_MEM_ON,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noenv", },
	{ "-nonormal",			"Disable normal maps",						true,	EASY_DEFAULT | EASY_HI_MEM_OFF,		EASY_ALL_ON  | EASY_HI_MEM_ON,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nonormal" },
	{ "-emissive_light",	"Enable emissive light from ships",			true,	0,									EASY_DEFAULT,					"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-emissive_light" },
	{ "-noheight",			"Disable height/parallax maps",				true,	EASY_DEFAULT | EASY_HI_MEM_OFF,		EASY_ALL_ON  | EASY_HI_MEM_ON,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noheight" },
	{ "-3dshockwave",		"Enable 3D shockwaves",						true,	EASY_ALL_ON  | EASY_HI_MEM_ON,		EASY_DEFAULT | EASY_HI_MEM_OFF,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-3dshockwave" },
	{ "-no_post_process",	"Disable post-processing",					true,	EASY_DEFAULT | EASY_HI_MEM_OFF,		EASY_ALL_ON | EASY_HI_MEM_ON,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_post_process" },
	{ "-soft_particles",	"Enable soft particles",					true,	EASY_ALL_ON,						EASY_DEFAULT,					"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-soft_particles" },
	{ "-aa",				"Enable Post-process anti-aliasing",		true,	EASY_ALL_ON  | EASY_HI_MEM_ON,		EASY_DEFAULT | EASY_HI_MEM_OFF,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-aa" },
	{ "-nolightshafts",		"Disable lightshafts",						true,	EASY_DEFAULT | EASY_HI_MEM_OFF,		EASY_ALL_ON | EASY_HI_MEM_ON,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nolightshafts"},
	{ "-fb_explosions",		"Enable Framebuffer Shockwaves",			true,	EASY_ALL_ON,						EASY_DEFAULT,					"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fb_explosions", },
    { "-fb_thrusters",      "Enable Framebuffer Thrusters",             true,   EASY_ALL_ON,						EASY_DEFAULT,					"Graphics",     "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fb_thrusters", },
	{ "-no_deferred",		"Disable Deferred Lighting",				true,	EASY_DEFAULT | EASY_HI_MEM_OFF,		EASY_ALL_ON | EASY_HI_MEM_ON,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_deferred"},
	{ "-enable_shadows",	"Enable Shadows",							true,	EASY_ALL_ON  | EASY_HI_MEM_ON,		EASY_DEFAULT | EASY_HI_MEM_OFF,	"Graphics",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-enable_shadows"},

	{ "-no_vsync",			"Disable vertical sync",					true,	0,									EASY_DEFAULT,					"Game Speed",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_vsync", },

	{ "-fps",				"Show frames per second on HUD",			false,	0,									EASY_DEFAULT,					"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fps", },
	{ "-dualscanlines",		"Add another pair of scanning lines",		true,	0,									EASY_DEFAULT,					"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dualscanlines", },
	{ "-targetinfo",		"Enable info next to target",				true,	0,									EASY_DEFAULT,					"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-targetinfo", },
	{ "-orbradar",			"Enable 3D radar",							true,	0,									EASY_DEFAULT,					"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-orbradar", },
	{ "-rearm_timer",		"Enable rearm/repair completion timer",		true,	0,									EASY_DEFAULT,					"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-rearm_timer", },
	{ "-ballistic_gauge",	"Enable analog ballistic ammo gauge",		true,	0,									EASY_DEFAULT,					"HUD",			"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ballistic_gauge", },

	{ "-window",			"Run in window",							true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-window", },
	{ "-fullscreen_window",	"Run in fullscreen window",					false,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fullscreen_window", },
	{ "-stretch_menu",		"Stretch interface to fill screen",			true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-stretch_menu", },
	{ "-noscalevid",		"Disable scale-to-window for movies",		true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noscalevid", },
	{ "-nomotiondebris",	"Disable motion debris",					true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomotiondebris",},
	{ "-ship_choice_3d",	"Use 3D models for ship selection",			true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ship_choice_3d", },
	{ "-weapon_choice_3d",	"Use 3D models for weapon selection",		true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-weapon_choice_3d", },
	{ "-3dwarp",			"Enable 3D warp",							true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-3dwarp", },
	{ "-warp_flash",		"Enable flash upon warp",					true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-warp_flash", },
	{ "-no_ap_interrupt",	"Disable interrupting autopilot",			true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_ap_interrupt", },
	{ "-no_screenshake",	"Disable screen shaking",					true,	0,									EASY_DEFAULT,					"Gameplay",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_screenshake", },

	{ "-nosound",			"Disable all sound",						false,	0,									EASY_DEFAULT,					"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nosound", },
	{ "-nomusic",			"Disable music",							false,	0,									EASY_DEFAULT,					"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomusic", },
	{ "-no_enhanced_sound",	"Disable enhanced sound",					false,	0,									EASY_DEFAULT,					"Audio",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_enhanced_sound", },

	{ "-portable_mode",		"Store config in portable location",		false,	0,									EASY_DEFAULT,					"Launcher",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-portable_mode", },
	{ "-joy_info",			"Outputs SDL joystick info",				true,	0,									EASY_DEFAULT,					"Launcher",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-joy_info",},

	{ "-standalone",		"Run as standalone server",					false,	0,									EASY_DEFAULT,					"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-standalone", },
	{ "-startgame",			"Skip mainhall and start hosting",			false,	0,									EASY_DEFAULT,					"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-startgame", },
	{ "-closed",			"Start hosted server as closed",			false,	0,									EASY_DEFAULT,					"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-closed", },
	{ "-restricted",		"Host confirms join requests",				false,	0,									EASY_DEFAULT,					"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-restricted", },
	{ "-multilog",			"",											false,	0,									EASY_DEFAULT,					"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-multilog", },
	{ "-clientdamage",		"",											false,	0,									EASY_DEFAULT,					"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-clientdamage", },
	{ "-mpnoreturn",		"Disable flight deck option",				true,	0,									EASY_DEFAULT,					"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-mpnoreturn", },
	{ "-gateway_ip",		"Set gateway IP address",					false,	0,									EASY_DEFAULT,					"Multiplayer",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-gateway_ip", },

	{ "-no_set_gamma",		"Disable setting of gamma",					true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_set_gamma", },
	{ "-nomovies",			"Disable video playback",					true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nomovies", },
	{ "-noparseerrors",		"Disable parsing errors",					true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noparseerrors", },
	{ "-loadallweps",		"Load all weapons, even those not used",	true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-loadallweps", },
	{ "-disable_fbo",		"Disable OpenGL RenderTargets",				true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-disable_fbo", },
	{ "-disable_pbo",		"Disable OpenGL Pixel Buffer Objects",		true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-disable_pbo", },
	{ "-ati_swap",			"Fix colour issues on some ATI cards",		true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ati_swap", },
	{ "-no_3d_sound",		"Use only 2D/stereo for sound effects",		true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_3d_sound", },
	{ "-mipmap",			"Enable mipmapping",						true,	0,									EASY_DEFAULT | EASY_HI_MEM_OFF,	"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-mipmap", },
	{ "-use_gldrawelements","Don't use glDrawRangeElements",			true,	0,									EASY_DEFAULT,					"Troubleshoot",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-use_gldrawelements", },
	{ "-gl_finish",			"Fix input lag on some ATI+Linux systems",	true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-gl_finish", },
	{ "-no_geo_effects",	"Disable geometry shader for effects",		true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_geo_effects", },
	{ "-set_cpu_affinity",	"Sets processor affinity to config value",	true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-set_cpu_affinity", },
	{ "-nograb",			"Disables mouse grabbing",					true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-nograb", },
	{ "-noshadercache",		"Disables the shader cache",				true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noshadercache", },
	{ "-prefer_ipv4",		"Prefer IPv4 DNS lookups",					true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-prefer_ipv4", },
	{ "-prefer_ipv6",		"Prefer IPv6 DNS lookups",					true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-prefer_ipv6", },
	{ "-log_multi_packet",	"Log multi packet types ",					true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-log_multi_packet",},
#ifdef WIN32
	{ "-fix_registry",	"Use a different registry path",				true,	0,									EASY_DEFAULT,					"Troubleshoot", "http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-fix_registry", },
#endif

	{ "-ingame_join",		"Allow in-game joining",					true,	0,									EASY_DEFAULT,					"Experimental",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-ingame_join", },
	{ "-voicer",			"Enable voice recognition",					true,	0,									EASY_DEFAULT,					"Experimental",	"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-voicer", },

	{ "-bmpmanusage",		"Show how many BMPMAN slots are in use",	false,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-bmpmanusage", },
	{ "-pos",				"Show position of camera",					false,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-pos", },
	{ "-stats",				"Show statistics",							true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-stats", },
	{ "-coords",			"Show coordinates",							false,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-coords", },
	{ "-pofspew",			"Dump model information to pofspew.txt",	false,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-pofspew", },
	{ "-weaponspew",		"Dump weapon stats and spreadsheets",		true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-weaponspew", },
	{ "-tablecrcs",			"Dump table CRCs for multi validation",		true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-tablecrcs", },
	{ "-missioncrcs",		"Dump mission CRCs for multi validation",	true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-missioncrcs", },
	{ "-dis_collisions",	"Disable collisions",						true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dis_collisions", },
	{ "-dis_weapons",		"Disable weapon rendering",					true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-dis_weapons", },
	{ "-output_sexps",		"Output SEXPs to sexps.html",				true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-output_sexps", },
	{ "-output_scripting",	"Output scripting to scripting.html",		true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-output_scripting", },
	{ "-output_script_json",	"Output scripting doc to scripting.json",	true,	0,								EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-output_script_json", },
	{ "-controlconfig_tbl",	"Save control presets to table",			true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-controlconfig_tbl", },
	{ "-save_render_target",	"Save render targets to file",			true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-save_render_target", },
	{ "-verify_vps",		"Spew VP CRCs to vp_crcs.txt",				true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-verify_vps", },
	{ "-reparse_mainhall",	"Reparse mainhall.tbl when loading halls",	false,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-reparse_mainhall", },
	{ "-noninteractive",	"Disables interactive dialogs",				true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-noninteractive", },
	{ "-no_unfocused_pause","Don't pause if the window isn't focused",	true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-no_unfocused_pause", },
	{ "-benchmark_mode",	"Puts the game into benchmark mode",		true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-benchmark_mode", },
	{ "-profile_frame_time","Profile frame time",						true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-profile_frame_time", },
	{ "-profile_write_file", "Write profiling information to file",		true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-profile_write_file", },
	{ "-json_profiling",	"Generate JSON profiling output",			true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-json_profiling", },
	{ "-debug_window",		"Enable the debug window",					true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-debug_window", },
	{ "-gr_debug",		"Output graphics debug information",			true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-gr_debug", },
	{ "-stdout_log",		"Output log file to stdout",				true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-stdout_log", },
	{ "-slow_frames_ok",	"Don't adjust timestamps for slow frames",	true,	0,									EASY_DEFAULT,					"Dev Tool",		"http://www.hard-light.net/wiki/index.php/Command-Line_Reference#-slow_frames_ok", },
};
// clang-format on

// forward declaration
const char * get_param_desc(const char *flag_name);

// here are the command line parameters that we will be using for FreeSpace

// RETAIL options ----------------------------------------------
cmdline_parm connect_arg("-connect", "Automatically connect to multiplayer IP:PORT", AT_STRING);			// Cmdline_connect_addr
cmdline_parm gamename_arg("-gamename", "Set multiplayer game name", AT_STRING);		// Cmdline_game_name
cmdline_parm gamepassword_arg("-password", "Set multiplayer game password", AT_STRING);	// Cmdline_game_password
cmdline_parm allowabove_arg("-allowabove", "Ranks above this can join multi", AT_STRING);	// Cmdline_rank_above
cmdline_parm allowbelow_arg("-allowbelow", "Ranks below this can join multi", AT_STRING);	// Cmdline_rank_below
cmdline_parm standalone_arg("-standalone", NULL, AT_NONE);
cmdline_parm nosound_arg("-nosound", NULL, AT_NONE);			// Cmdline_freespace_no_sound
cmdline_parm nomusic_arg("-nomusic", NULL, AT_NONE);			// Cmdline_freespace_no_music
cmdline_parm noenhancedsound_arg("-no_enhanced_sound", NULL, AT_NONE);	// Cmdline_no_enhanced_sound
cmdline_parm startgame_arg("-startgame", NULL, AT_NONE);		// Cmdline_start_netgame
cmdline_parm gameclosed_arg("-closed", NULL, AT_NONE);		// Cmdline_closed_game
cmdline_parm gamerestricted_arg("-restricted", NULL, AT_NONE);	// Cmdline_restricted_game
cmdline_parm port_arg("-port", "Multiplayer network port", AT_INT);
cmdline_parm multilog_arg("-multilog", NULL, AT_NONE);		// Cmdline_multi_log
cmdline_parm pof_spew("-pofspew", NULL, AT_NONE);			// Cmdline_spew_pof_info
cmdline_parm weapon_spew("-weaponspew", nullptr, AT_STRING);			// Cmdline_spew_weapon_stats
cmdline_parm mouse_coords("-coords", NULL, AT_NONE);			// Cmdline_mouse_coords
cmdline_parm timeout("-timeout", "Multiplayer network timeout (secs)", AT_INT);				// Cmdline_timeout
cmdline_parm bit32_arg("-32bit", "Deprecated", AT_NONE);				// (only here for retail compatibility reasons, doesn't actually do anything)

char *Cmdline_connect_addr = NULL;
char *Cmdline_game_name = NULL;
char *Cmdline_game_password = NULL;
char *Cmdline_rank_above = NULL;
char *Cmdline_rank_below = NULL;
int Cmdline_cd_check = 1;
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
WeaponSpewType Cmdline_spew_weapon_stats = WeaponSpewType::NONE;
int Cmdline_start_netgame = 0;
int Cmdline_timeout = -1;
int Cmdline_use_last_pilot = 0;


// FSO options -------------------------------------------------

// Graphics related
cmdline_parm fov_arg("-fov", "Vertical field-of-view factor", AT_FLOAT);					// Cmdline_fov  -- comand line FOV -Bobboau
cmdline_parm fov_cockpit_arg("-fov_cockpit", "Vertical field-of-view factor for Cockpits", AT_FLOAT);
cmdline_parm clip_dist_arg("-clipdist", "Changes the distance from the viewpoint for the near-clipping plane", AT_FLOAT);		// Cmdline_clip_dist
cmdline_parm ambient_power_arg("-ambient", "Multiplies the brightness of all ambient light", AT_FLOAT);
cmdline_parm light_power_arg("-light", "Multiplies the brightness of all light", AT_FLOAT);
cmdline_parm emissive_power_arg("-emissive", "Multiplies the brightness of all ambient light", AT_FLOAT);
cmdline_parm emissive_arg("-emissive_light", "Enable emissive light from ships", AT_NONE);		// semi-deprecated but still functional
cmdline_parm env("-noenv", NULL, AT_NONE);								// Cmdline_env
cmdline_parm glow_arg("-noglow", NULL, AT_NONE); 						// Cmdline_glow  -- use Bobs glow code
cmdline_parm nomotiondebris_arg("-nomotiondebris", NULL, AT_NONE);		// Cmdline_nomotiondebris  -- Removes those ugly floating rocks -C
cmdline_parm noscalevid_arg("-noscalevid", NULL, AT_NONE);				// Cmdline_noscalevid  -- disable video scaling that fits to window
cmdline_parm spec_arg("-nospec", NULL, AT_NONE);			// Cmdline_spec  --
cmdline_parm normal_arg("-nonormal", NULL, AT_NONE);						// Cmdline_normal  -- disable normal mapping
cmdline_parm height_arg("-noheight", NULL, AT_NONE);						// Cmdline_height  -- enable support for parallax mapping
cmdline_parm enable_3d_shockwave_arg("-3dshockwave", NULL, AT_NONE);
cmdline_parm softparticles_arg("-soft_particles", NULL, AT_NONE);
cmdline_parm no_postprocess_arg("-no_post_process", "Disables post-processing", AT_NONE);
cmdline_parm bloom_intensity_arg("-bloom_intensity", "Set bloom intensity, requires post-processing", AT_INT);
cmdline_parm post_process_aa_arg("-aa", "Enables post-process antialiasing", AT_NONE);
cmdline_parm post_process_aa_preset_arg("-aa_preset", "Sets the AA effect to use. See the wiki for details", AT_INT);
cmdline_parm deprecated_fxaa_arg("-fxaa", nullptr, AT_NONE);
cmdline_parm deprecated_fxaa_preset_arg("-fxaa_preset", "FXAA quality (0-2), requires post-processing and -fxaa", AT_INT);
cmdline_parm deprecated_smaa_arg("-smaa", nullptr, AT_NONE);
cmdline_parm deprecated_smaa_preset_arg("-smaa_preset", "SMAA quality (0-3), requires post-processing and -smaa", AT_INT);
cmdline_parm fb_explosions_arg("-fb_explosions", NULL, AT_NONE);
cmdline_parm fb_thrusters_arg("-fb_thrusters", NULL, AT_NONE);
cmdline_parm flightshaftsoff_arg("-nolightshafts", NULL, AT_NONE);
cmdline_parm shadow_quality_arg("-shadow_quality", NULL, AT_INT);
cmdline_parm enable_shadows_arg("-enable_shadows", NULL, AT_NONE);
cmdline_parm no_deferred_lighting_arg("-no_deferred", NULL, AT_NONE);	// Cmdline_no_deferred
cmdline_parm anisotropy_level_arg("-anisotropic_filter", NULL, AT_INT);

float Cmdline_clip_dist = Default_min_draw_distance;
float Cmdline_ambient_power = 1.0f;
float Cmdline_emissive_power = 0.0f;
float Cmdline_light_power = 1.0f;
int Cmdline_env = 1;
int Cmdline_mipmap = 0;
int Cmdline_glow = 1;
int Cmdline_noscalevid = 0;
int Cmdline_spec = 1;
int Cmdline_emissive = 0;
int Cmdline_normal = 1;
int Cmdline_height = 1;
int Cmdline_enable_3d_shockwave = 0;
int Cmdline_softparticles = 0;
int Cmdline_bloom_intensity = 25;
bool Cmdline_force_lightshaft_off = false;
int Cmdline_no_deferred_lighting = 0;
int Cmdline_aniso_level = 0;

// Game Speed related
cmdline_parm no_fpscap("-no_fps_capping", "Don't limit frames-per-second", AT_NONE);	// Cmdline_NoFPSCap
cmdline_parm no_vsync_arg("-no_vsync", NULL, AT_NONE);		// Cmdline_no_vsync

int Cmdline_NoFPSCap = 0; // Disable FPS capping - kazan
int Cmdline_no_vsync = 0;

// HUD related
cmdline_parm ballistic_gauge("-ballistic_gauge", NULL, AT_NONE);	// Cmdline_ballistic_gauge
cmdline_parm dualscanlines_arg("-dualscanlines", NULL, AT_NONE); // Cmdline_dualscanlines  -- Change to phreaks options including new targeting code
cmdline_parm orb_radar("-orbradar", NULL, AT_NONE);			// Cmdline_orb_radar
cmdline_parm rearm_timer_arg("-rearm_timer", NULL, AT_NONE);	// Cmdline_rearm_timer
cmdline_parm targetinfo_arg("-targetinfo", NULL, AT_NONE);	// Cmdline_targetinfo  -- Adds ship name/class to right of target box -C

int Cmdline_ballistic_gauge = 0;	// WMCoolmon's gauge thingy
int Cmdline_dualscanlines = 0;
int Cmdline_orb_radar = 0;
int Cmdline_rearm_timer = 0;
int Cmdline_targetinfo = 0;

// Gameplay related
cmdline_parm use_3dwarp("-3dwarp", nullptr, AT_NONE);			// Is now Fireball_use_3d_warp
cmdline_parm ship_choice_3d_arg("-ship_choice_3d", nullptr, AT_NONE);	// Cmdline_ship_choice_3d
cmdline_parm weapon_choice_3d_arg("-weapon_choice_3d", nullptr, AT_NONE);	// Cmdline_weapon_choice_3d
cmdline_parm use_warp_flash("-warp_flash", nullptr, AT_NONE);	// Cmdline_warp_flash
cmdline_parm allow_autpilot_interrupt("-no_ap_interrupt", nullptr, AT_NONE);
cmdline_parm stretch_menu("-stretch_menu", nullptr, AT_NONE);	// Cmdline_stretch_menu
cmdline_parm no_screenshake("-no_screenshake", nullptr, AT_NONE); // Cmdline_no_screenshake
cmdline_parm deadzone("-deadzone", 
"Sets the joystick deadzone. Integer value from 0 to 100 as a percentage of the joystick's range (100% would make the stick do nothing). Disables deadzone slider in the in-game Options menu.", AT_INT); //Cmdline_deadzone

int Cmdline_ship_choice_3d = 0;
int Cmdline_weapon_choice_3d = 0;
int Cmdline_warp_flash = 0;
int Cmdline_autopilot_interruptable = 1;
int Cmdline_stretch_menu = 0;
int Cmdline_no_screenshake = 0;
int Cmdline_deadzone = -1;

// Audio related
cmdline_parm voice_recognition_arg("-voicer", NULL, AT_NONE);	// Cmdline_voice_recognition

int Cmdline_voice_recognition = 0;
int Cmdline_no_enhanced_sound = 0;

// MOD related
cmdline_parm mod_arg("-mod", "List of folders to overwrite/add-to the default data", AT_STRING, true);	// Cmdline_mod  -- DTP modsupport

char *Cmdline_mod = NULL; //DTP for mod argument

// Multiplayer/Network related
cmdline_parm almission_arg("-almission", "Autoload multiplayer mission", AT_STRING);		// Cmdline_almission  -- DTP for autoload Multi mission
cmdline_parm ingamejoin_arg("-ingame_join", NULL, AT_NONE);	// Cmdline_ingamejoin
cmdline_parm mpnoreturn_arg("-mpnoreturn", NULL, AT_NONE);	// Cmdline_mpnoreturn  -- Removes 'Return to Flight Deck' in respawn dialog -C
cmdline_parm objupd_arg("-cap_object_update", "Multiplayer object update cap (0-3)", AT_INT);
cmdline_parm gateway_ip_arg("-gateway_ip", "Set gateway IP address", AT_STRING);

char *Cmdline_almission = NULL;	//DTP for autoload multi mission.
int Cmdline_ingamejoin = 0;
int Cmdline_mpnoreturn = 0;
int Cmdline_objupd = 3;		// client object updates on LAN by default
char *Cmdline_gateway_ip = nullptr;

// Launcher related options
cmdline_parm portable_mode("-portable_mode", NULL, AT_NONE);
cmdline_parm joy_info("-joy_info", "Outputs SDL joystick info", AT_NONE);

bool Cmdline_portable_mode = false;

// Troubleshooting
cmdline_parm loadallweapons_arg("-loadallweps", NULL, AT_NONE);	// Cmdline_load_all_weapons
cmdline_parm nomovies_arg("-nomovies", NULL, AT_NONE);		// Cmdline_nomovies  -- Allows video streaming
cmdline_parm no_set_gamma_arg("-no_set_gamma", NULL, AT_NONE);	// Cmdline_no_set_gamma
cmdline_parm no_fbo_arg("-disable_fbo", NULL, AT_NONE);		// Cmdline_no_fbo
cmdline_parm no_pbo_arg("-disable_pbo", NULL, AT_NONE);		// Cmdline_no_pbo
cmdline_parm mipmap_arg("-mipmap", NULL, AT_NONE);			// Cmdline_mipmap
cmdline_parm atiswap_arg("-ati_swap", NULL, AT_NONE);        // Cmdline_atiswap - Fix ATI color swap issue for screenshots.
cmdline_parm no3dsound_arg("-no_3d_sound", NULL, AT_NONE);		// Cmdline_no_3d_sound - Disable use of full 3D sounds
cmdline_parm no_drawrangeelements("-use_gldrawelements", NULL, AT_NONE); // Cmdline_drawelements -- Uses glDrawElements instead of glDrawRangeElements
cmdline_parm keyboard_layout("-keyboard_layout", "Specify keyboard layout (qwertz or azerty)", AT_STRING);
cmdline_parm gl_finish ("-gl_finish", NULL, AT_NONE);
cmdline_parm no_geo_sdr_effects("-no_geo_effects", NULL, AT_NONE);
cmdline_parm set_cpu_affinity("-set_cpu_affinity", NULL, AT_NONE);
cmdline_parm nograb_arg("-nograb", NULL, AT_NONE);
cmdline_parm noshadercache_arg("-noshadercache", NULL, AT_NONE);
cmdline_parm prefer_ipv4_arg("-prefer_ipv4", nullptr, AT_NONE);
cmdline_parm prefer_ipv6_arg("-prefer_ipv6", nullptr, AT_NONE);
cmdline_parm log_multi_packet_arg("-log_multi_packet", nullptr, AT_NONE);
#ifdef WIN32
cmdline_parm fix_registry("-fix_registry", NULL, AT_NONE);
#endif

int Cmdline_load_all_weapons = 0;
int Cmdline_nomovies = 0;
int Cmdline_no_set_gamma = 0;
int Cmdline_no_fbo = 0;
int Cmdline_no_pbo = 0;
int Cmdline_ati_color_swap = 0;
int Cmdline_no_3d_sound = 0;
int Cmdline_drawelements = 0;
char* Cmdline_keyboard_layout = NULL;
bool Cmdline_gl_finish = false;
bool Cmdline_no_geo_sdr_effects = false;
bool Cmdline_set_cpu_affinity = false;
bool Cmdline_nograb = false;
bool Cmdline_noshadercache = false;
bool Cmdline_prefer_ipv4 = false;
bool Cmdline_prefer_ipv6 = false;
bool Cmdline_dump_packet_type = false;
#ifdef WIN32
bool Cmdline_alternate_registry_path = false;
#endif
uint Cmdline_rng_seed = 0;
bool Cmdline_reuse_rng_seed = false;

// Developer/Testing related
cmdline_parm start_mission_arg("-start_mission", "Skip mainhall and run this mission", AT_STRING);	// Cmdline_start_mission
cmdline_parm dis_collisions("-dis_collisions", NULL, AT_NONE);	// Cmdline_dis_collisions
cmdline_parm dis_weapons("-dis_weapons", NULL, AT_NONE);		// Cmdline_dis_weapons
cmdline_parm noparseerrors_arg("-noparseerrors", NULL, AT_NONE);	// Cmdline_noparseerrors  -- turns off parsing errors -C
cmdline_parm extra_warn_arg("-extra_warn", "Enable 'extra' warnings", AT_NONE);	// Cmdline_extra_warn
cmdline_parm fps_arg("-fps", NULL, AT_NONE);					// Cmdline_show_fps
cmdline_parm bmpmanusage_arg("-bmpmanusage", NULL, AT_NONE);	// Cmdline_bmpman_usage
cmdline_parm pos_arg("-pos", NULL, AT_NONE);					// Cmdline_show_pos
cmdline_parm stats_arg("-stats", NULL, AT_NONE);				// Cmdline_show_stats
cmdline_parm save_render_targets_arg("-save_render_target", NULL, AT_NONE);	// Cmdline_save_render_targets
cmdline_parm window_arg("-window", NULL, AT_NONE);				// Cmdline_window
cmdline_parm fullscreen_window_arg("-fullscreen_window", "Fullscreen/borderless window (Windows only)", AT_NONE);
cmdline_parm res_arg("-res", "Resolution, formatted like 1600x900", AT_STRING);
cmdline_parm center_res_arg("-center_res", "Resolution of center monitor, formatted like 1600x900", AT_STRING);
cmdline_parm verify_vps_arg("-verify_vps", NULL, AT_NONE);	// Cmdline_verify_vps  -- spew VP crcs to vp_crcs.txt
cmdline_parm parse_cmdline_only(PARSE_COMMAND_LINE_STRING, "Ignore any cmdline_fso.cfg files", AT_NONE);
cmdline_parm reparse_mainhall_arg("-reparse_mainhall", NULL, AT_NONE); //Cmdline_reparse_mainhall
cmdline_parm frame_profile_write_file("-profile_write_file", NULL, AT_NONE); // Cmdline_profile_write_file
cmdline_parm no_unfocused_pause_arg("-no_unfocused_pause", NULL, AT_NONE); //Cmdline_no_unfocus_pause
cmdline_parm benchmark_mode_arg("-benchmark_mode", NULL, AT_NONE); //Cmdline_benchmark_mode
cmdline_parm pilot_arg("-pilot", nullptr, AT_STRING); //Cmdline_pilot
cmdline_parm noninteractive_arg("-noninteractive", NULL, AT_NONE); //Cmdline_noninteractive
cmdline_parm json_profiling("-json_profiling", NULL, AT_NONE); //Cmdline_json_profiling
cmdline_parm show_video_info("-show_video_info", NULL, AT_NONE); //Cmdline_show_video_info
cmdline_parm frame_profile_arg("-profile_frame_time", NULL, AT_NONE); //Cmdline_frame_profile
cmdline_parm debug_window_arg("-debug_window", NULL, AT_NONE);	// Cmdline_debug_window
cmdline_parm graphics_debug_output_arg("-gr_debug", nullptr, AT_NONE); // Cmdline_graphics_debug_output
cmdline_parm log_to_stdout_arg("-stdout_log", nullptr, AT_NONE); // Cmdline_log_to_stdout
cmdline_parm slow_frames_ok_arg("-slow_frames_ok", nullptr, AT_NONE);	// Cmdline_slow_frames_ok
cmdline_parm fixed_seed_rand("-seed", nullptr, AT_INT);	// Cmdline_rng_seed,Cmdline_reuse_rng_seed;
cmdline_parm luadev_arg("-luadev", "nullptr", AT_NONE);	// Cmdline_lua_devmode


char *Cmdline_start_mission = NULL;
int Cmdline_dis_collisions = 0;
int Cmdline_dis_weapons = 0;
bool Cmdline_output_sexp_info = false;
int Cmdline_noparseerrors = 0;
#ifdef Allow_NoWarn
int Cmdline_nowarn = 0; // turn warnings off in FRED
#endif
int Cmdline_extra_warn = 0;
int Cmdline_bmpman_usage = 0;
int Cmdline_show_pos = 0;
int Cmdline_show_stats = 0;
int Cmdline_save_render_targets = 0;
int Cmdline_window = 0;
int Cmdline_fullscreen_window = 0;
char *Cmdline_res = 0;
char *Cmdline_center_res = 0;
int Cmdline_verify_vps = 0;
int Cmdline_reparse_mainhall = 0;
bool Cmdline_profile_write_file = false;
bool Cmdline_no_unfocus_pause = false;
bool Cmdline_benchmark_mode = false;
const char *Cmdline_pilot = nullptr;
bool Cmdline_noninteractive = false;
bool Cmdline_json_profiling = false;
bool Cmdline_frame_profile = false;
bool Cmdline_show_video_info = false;
bool Cmdline_debug_window = false;
bool Cmdline_graphics_debug_output = false;
bool Cmdline_log_to_stdout = false;
bool Cmdline_slow_frames_ok = false;
bool Cmdline_lua_devmode = false;

// Other
cmdline_parm get_flags_arg(GET_FLAGS_STRING, "Output the launcher flags file", AT_STRING);
cmdline_parm output_sexp_arg("-output_sexps", NULL, AT_NONE); //WMC - outputs all SEXPs to sexps.html
cmdline_parm output_scripting_arg("-output_scripting", NULL, AT_NONE);	//WMC
cmdline_parm output_script_json_arg("-output_script_json", nullptr, AT_NONE);	// m!m
cmdline_parm generate_controlconfig_arg("-controlconfig_tbl", nullptr, AT_NONE);	

// Deprecated flags - CommanderDJ
cmdline_parm deprecated_spec_arg("-spec", "Deprecated", AT_NONE);
cmdline_parm deprecated_glow_arg("-glow", "Deprecated", AT_NONE);
cmdline_parm deprecated_normal_arg("-normal", "Deprecated", AT_NONE);
cmdline_parm deprecated_env_arg("-env", "Deprecated", AT_NONE);
cmdline_parm deprecated_tbp_arg("-tbp", "Deprecated", AT_NONE);
cmdline_parm deprecated_jpgtga_arg("-jpgtga", "Deprecated", AT_NONE);
cmdline_parm deprecated_htl_arg("-nohtl", "Deprecated", AT_NONE);
cmdline_parm deprecated_brieflighting_arg("-brief_lighting", "Deprecated", AT_NONE);
cmdline_parm deprecated_sndpreload_arg("-snd_preload", "Deprecated", AT_NONE);
cmdline_parm deprecated_missile_lighting_arg("-missile_lighting", "Deprecated", AT_NONE);
cmdline_parm deprecated_cache_bitmaps_arg("-cache_bitmaps", "Deprecated", AT_NONE);
cmdline_parm deprecated_no_emissive_arg("-no_emissive_light", "Deprecated", AT_NONE);
cmdline_parm deprecated_postprocess_arg("-post_process", "Deprecated", AT_NONE);
cmdline_parm deprecated_spec_static_arg("-spec_static", "Deprecated", AT_NONE);
cmdline_parm deprecated_spec_point_arg("-spec_point", "Deprecated", AT_NONE);
cmdline_parm deprecated_spec_tube_arg("-spec_tube", "Deprecated", AT_NONE);
cmdline_parm deprecated_ambient_factor_arg("-ambient_factor", "Deprecated", AT_NONE);	//

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

	//Print log messages about any deprecated flags we found - CommanderDJ
	//Do it programmatically, rather than enumerating them by hand - MageKing17
	for (parmp = GET_FIRST(&Parm_list); parmp != END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp)) {
		if (parmp->name_found && !stricmp("deprecated", parmp->help)) {
			mprintf(("Deprecated flag '%s' found. Please remove from your cmdline.\n", parmp->name));
		}
	}
}
#endif

// prints simple cmdline to multi.log
void cmdline_print_cmdline_multi()
{
	cmdline_parm *parmp;
	int found = 0;
	std::ostringstream cmdline;

	for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
		if ( parmp->name_found ) {
			cmdline << " " << parmp->name;

			if (parmp->args) {
				cmdline << " " << parmp->args;
			}

			found++;
		}
	}

	if ( !found ) {
		cmdline << " <none>";
	}

	ml_printf("Command line:%s", cmdline.str().c_str());
}

//	Return true if this character is an extra char (white space and quotes)
int is_extra_space(char ch)
{
	return ((ch == ' ') || (ch == '\t') || (ch == 0x0a) || (ch == '\'') || (ch == '\"'));
}


// eliminates all leading and trailing extra chars from a string.  Returns pointer passed in.
char *drop_extra_chars(char *str)
{
	size_t s;
	size_t e;

	s = 0;
	while (str[s] && is_extra_space(str[s]))
		s++;

	e = strlen(str);

	if (e > 0) {
		// we already account for NULL later on, so the -1 is here to make
		// sure we do our math without taking it into consideration
		e -= 1;
	}

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


/*
 * @brief Processes one argument for the given parameter
 *
 * @param param The parameter to check
 * @param argc The argument count
 * @param argc The argument values
 * @param argc The current index
 * @return @c true when an extra parameter was found, @c false otherwise
 */
bool parm_stuff_args(cmdline_parm *parm, int argc, char *argv[], int index)
{
	Assert(index < argc);

	if (index + 1 < argc)
	{
		char* value = argv[index + 1];
		if (value[0] == '-')
		{
			// Found another argument, just return
			return false;
		}
		else
		{
			char* saved_args = NULL;

			if (parm->args != NULL) {
				if (parm->stacks) {
					saved_args = parm->args;
				}
				else {
					delete[] parm->args;
				}

				parm->args = NULL;
			}

			size_t argsize = strlen(argv[index + 1]);
			size_t buffersize = argsize;

			if (saved_args != NULL)
			{
				// Add one for the , separating args
				buffersize += strlen(saved_args) + 1;
			}

			buffersize += 1; // Null-terminator

			parm->args = new char[buffersize];
			memset(parm->args, 0, buffersize);

			if (saved_args != NULL)
			{
				// saved args go first, then new arg
				strcpy_s(parm->args, buffersize, saved_args);
				// add a separator too, so that we can tell the args apart
				strcat_s(parm->args, buffersize, ",");
				// now the new arg
				strcat_s(parm->args, buffersize, argv[index + 1]);

				delete[] saved_args;
			}
			else
			{
				strcpy_s(parm->args, buffersize, argv[index + 1]);
			}

			return true;
		}
	}
	else
	{
		// Last argument, can't have any values
		return false;
	}
}


// internal function - parse the command line, extracting parameter arguments if they exist
// cmdline - command line string passed to the application
void os_parse_parms(int argc, char *argv[])
{
	// locate command line parameters
	cmdline_parm *parmp;

	for (int i = 0; i < argc; i++)
	{
		// On OS X this gets passed if the application was launched by double-clicking in the Finder
		if (i == 1 && strncmp(argv[i], "-psn", 4) == 0)
		{
			continue;
		}

		for (parmp = GET_FIRST(&Parm_list); parmp != END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp)) {
			if (!stricmp(parmp->name, argv[i]))
			{
				parmp->name_found = 1;
				if (parm_stuff_args(parmp, argc, argv, i))
				{
					i++;
				}
			}
		}
	}
}


// validate the command line parameters.  Display an error if an unrecognized parameter is located.
void os_validate_parms(int argc, char *argv[])
{
	cmdline_parm *parmp;
	char *token;
	int parm_found;

	for (int i = 0; i < argc; i++)
	{
		token = argv[i];

		// On OS X this gets passed if the application was launched by double-clicking in the Finder
		if (i == 1 && strncmp(token, "-psn", 4) == 0) {
			continue;
		}

		if (token[0] == '-') {
			parm_found = 0;

			for (parmp = GET_FIRST(&Parm_list); parmp != END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp)) {
				if (!stricmp(parmp->name, token)) {
					parm_found = 1;
					break;
				}
			}

			if (parm_found == 0) {
				// if we got a -help, --help, -h, or -? then show the help text, otherwise show unknown option
				if (!stricmp(token, "-help") || !stricmp(token, "--help") || !stricmp(token, "-h") || !stricmp(token, "-?")) {
					printf("FreeSpace 2 Open, version %s\n", FS_VERSION_FULL);
					printf("Website: http://scp.indiegames.us\n");
					printf("Github (bug reporting): https://github.com/scp-fs2open/fs2open.github.com/issues\n\n");
					printf("Usage: fs2_open [options]\n");

					// not the prettiest thing but the job gets done
					static const int STR_SIZE = 25;  // max len of exe_params.name + 5 spaces
					const int AT_SIZE = 8;  // max len of cmdline_arg_types[] + 2 spaces
					size_t atp = 0;
					size_t sp = 0;
					for (parmp = GET_FIRST(&Parm_list); parmp !=END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp) ) {
						// don't output deprecated flags
						if (stricmp("deprecated", parmp->help) != 0) {
							sp = strlen(parmp->name);
							if (parmp->arg_type != AT_NONE) {
								atp = strlen(cmdline_arg_types[parmp->arg_type]);
								printf("    [ %s ]%*s[ %s ]%*s- %s\n", parmp->name, (int)(STR_SIZE - sp -1), NOX(" "), cmdline_arg_types[parmp->arg_type], (int)(AT_SIZE-atp), NOX(" "), parmp->help);
							} else {
								printf("    [ %s ]%*s- %s\n", parmp->name, (int)(STR_SIZE - sp -1 +AT_SIZE+4), NOX(" "), parmp->help);
							}
						}
					}

					printf("\n");
					exit(0);
				}
				else {
					char buffer[128];
					sprintf(buffer, "Unrecognized command line parameter %s.", token);

					os::dialogs::Message(os::dialogs::MESSAGEBOX_INFORMATION, buffer);
				}
			}
		}
	}
}

int parse_cmdline_string(char* cmdline, char** argv)
{
	size_t length = strlen(cmdline);

	bool start_found = false;
	bool quoted = false;

	size_t argc = 0;
	char* current_argv = NULL;
	for (size_t i = 0; i < length; i++)
	{
		if (!start_found && !isspace(cmdline[i]))
		{
			start_found = true;
			current_argv = (cmdline + i);
		}
		else if (start_found)
		{
			if (cmdline[i] == '"')
			{
				quoted = !quoted;

				if (!quoted && current_argv != NULL)
				{
					if (argv != NULL)
					{
						// Terminate string at quote
						cmdline[i] = '\0';
						argv[argc] = current_argv;
						current_argv = NULL;
					}

					argc++;
				}
			}
			else if (isspace(cmdline[i]) && !quoted)
			{
				// Parameter terminated by space
				if (current_argv != NULL) // == NULL means that we currently don't have a parameter
				{
					if (argv != NULL)
					{
						// Terminate string at quote
						cmdline[i] = '\0';
						argv[argc] = current_argv;
						current_argv = NULL;
					}

					argc++;
				}
			}
			else if (current_argv == NULL)
			{
				current_argv = cmdline + i;
			}
		}
	}

	if (current_argv != NULL)
	{
		if (argv != NULL)
		{
			// Terminate string at quote
			argv[argc] = current_argv;
			current_argv = NULL;
		}

		argc++;
	}

	return (int)argc;
}

void os_process_cmdline(char* cmdline)
{
	int argc = parse_cmdline_string(cmdline, NULL);

	char** argv = new char*[argc];

	argc = parse_cmdline_string(cmdline, argv);

	os_parse_parms(argc, argv);
	os_validate_parms(argc, argv);

	delete[] argv;
}

bool has_cmdline_only_or_get_flags(int argc, char *argv[])
{
	for (int i = 0; i < argc; ++i)
	{
		if (!strcmp(argv[i], PARSE_COMMAND_LINE_STRING) || !strcmp(argv[i], GET_FLAGS_STRING))
		{
			return true;
		}
	}

	return false;
}

// remove old parms - needed for tests
static void reset_cmdline_parms()
{
	for (cmdline_parm *parmp = GET_FIRST(&Parm_list); parmp != END_OF_LIST(&Parm_list); parmp = GET_NEXT(parmp)) {
		if (parmp->args != NULL) {
			delete[] parmp->args;
			parmp->args = NULL;
		}
		parmp->name_found = 0;
	}
}

// Call once to initialize the command line system
//
// cmdline - command line string passed to the application
void os_init_cmdline(int argc, char *argv[])
{
	// Tests call this multiple times, so reset the params here.
	// Otherwise e.g. the modlist just grows and grows...
	reset_cmdline_parms();

	FILE *fp;

	// Don't parse any command-line config files if we specified PARSE_COMMAND_LINE_STRING (to explicitly prevent it),
	// or GET_FLAGS_STRING (because the engine is going to do a quick exit and for GitHub issue #1221)
	if (!has_cmdline_only_or_get_flags(argc, argv)) {
		// Only parse the config file in the current directory if we are in legacy config mode
		if (os_is_legacy_mode()) {
			// read the cmdline_fso.cfg file from the data folder, and pass the command line arguments to
			// the the parse_parms and validate_parms line.  Read these first so anything actually on
			// the command line will take precedence
#ifdef APPLE_APP
			char resolved_path[MAX_PATH], data_path[MAX_PATH_LEN];

			getcwd(data_path, sizeof(data_path));
			snprintf(resolved_path, MAX_PATH, "%s/data/cmdline_fso.cfg", data_path);

			fp = fopen(resolved_path, "rt");
#else
			fp = fopen("data" DIR_SEPARATOR_STR "cmdline_fso.cfg", "rt");
#endif
			// if the file exists, get a single line, and deal with it
			if (fp) {
				char *buf, *p;

				auto len = static_cast<int>(filelength(fileno(fp))) + 2;
				buf = new char[len];

				if (fgets(buf, len - 1, fp) != nullptr)
				{
					// replace the newline character with a NULL
					if ((p = strrchr(buf, '\n')) != NULL) {
						*p = '\0';
					}
#ifdef SCP_UNIX
					// append a space for the os_parse_parms() check
					strcat_s(buf, len, " ");
#endif
					os_process_cmdline(buf);
				}
				delete[] buf;
				fclose(fp);
			}
		} else {
			// parse user specific cmdline_fso config file (will supersede options in global file)
			fp = fopen(os_get_config_path("data/cmdline_fso.cfg").c_str(), "rt");

			// if the file exists, get a single line, and deal with it
			if ( fp ) {
				char *buf, *p;

				auto len = static_cast<int>(filelength( fileno(fp) )) + 2;
				buf = new char [len];

				if (fgets(buf, len-1, fp) != nullptr)
				{
					// replace the newline character with a NULL
					if ( (p = strrchr(buf, '\n')) != NULL ) {
						*p = '\0';
					}

					// append a space for the os_parse_parms() check
					strcat_s(buf, len, " ");

					os_process_cmdline(buf);
				}
				delete [] buf;
				fclose(fp);
			}
		}
	} // If cmdline included PARSE_COMMAND_LINE_STRING or GET_FLAGS_STRING

	// By parsing cmdline last, anything actually on the command line will take precedence.
	os_parse_parms(argc, argv);
	os_validate_parms(argc, argv);
}


/*
 * arg constructor
 *
 * @param name_    name of the parameter, must start with '-' character
 * @param help_    help text for this parameter
 * @param arg_type_    parameters arguement type (if any)
 * @param stacks_    can the parameter be stacked
 */
cmdline_parm::cmdline_parm(const char *name_, const char *help_, const int arg_type_, const bool stacks_) noexcept:
	name(name_), help(help_), stacks(stacks_), arg_type(arg_type_)
{
	args = NULL;
	name_found = 0;

	if (Parm_list_inited == 0) {
		Assertion(&Parm_list == this, "Coding error! 1st initialised cmdline_parm must be static Parm_list\n");
		list_init(this);
		Parm_list_inited = 1;
	} else {
		Assertion(name, "Coding error! cmdline_parm's must have a non-NULL name\n");
		Assertion(name[0] == '-', "Coding error! cmdline_parm's must start with a '-'\n");
		// not in the static Parm_list init, so lookup the NULL help args
		if (help == NULL) {
			help = get_param_desc(name);
		}
		list_append(&Parm_list, this);
	}
}


// destructor - frees any allocated memory
cmdline_parm::~cmdline_parm() noexcept
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

// returns - the interger representation for the parameter argument
int cmdline_parm::get_int()
{
	Assertion(arg_type == AT_INT, "Coding error! Cmdline arg (%s) called cmdline_parm::get_int() with invalid arg_type (%s)", name, cmdline_arg_types[arg_type]);
	check_if_args_is_valid();

	size_t offset = 0;

	if (stacks) {
		// first off, DON'T STACK NON-STRINGS!!
		Int3();

		// secondly, we still need to get it right for the user's sake...
		char *moron = strstr(args, ",");

		if ( moron && ((strlen(moron) + 1) < strlen(args)) ) {
			// we get the last arg, since it's the newest one
			offset = strlen(args) - strlen(moron) + 1;
		}
	}

	return atoi(args+offset);
}


// returns - the float representation for the parameter argument
float cmdline_parm::get_float()
{
	Assertion(arg_type == AT_FLOAT, "Coding error! Cmdline arg (%s) called cmdline_parm::get_float() with invalid arg_type (%s)", name, cmdline_arg_types[arg_type]);
	check_if_args_is_valid();

	size_t offset = 0;

	if (stacks) {
		// first off, DON'T STACK NON-STRINGS!!
		Int3();

		// secondly, we still need to get it right for the user's sake
		char *moron = strstr(args, ",");

		if ( moron && ((strlen(moron) + 1) < strlen(args)) ) {
			// we get the last arg, since it's the newest one
			offset = strlen(args) - strlen(moron) + 1;
		}
	}

	return (float)atof(args+offset);
}


// returns - the string value for the parameter argument
char *cmdline_parm::str()
{
	Assertion(arg_type == AT_STRING, "Coding error! Cmdline arg (%s) called cmdline_parm::str() with invalid arg_type (%s)", name, cmdline_arg_types[arg_type]);
	check_if_args_is_valid();

	return args;
}
bool cmdline_parm::has_param() {
	return args != nullptr;
}

#ifdef SCP_UNIX
// Return a vector with all filesystem names of "parent/dir" relative to parent.
// dir must not contain a slash.
static SCP_vector<SCP_string> unix_get_single_dir_names(const SCP_string& parent, const SCP_string& dir)
{
	SCP_vector<SCP_string> ret;

	DIR *dp;
	if ((dp = opendir(parent.c_str())) == NULL) {
		Warning(LOCATION, "Can't open directory '%s' when searching mod paths. Ignoring. errno=%d", parent.c_str(), errno);
		return ret;
	}

	dirent *dirp;
	while ((dirp = readdir(dp)) != NULL) {
		if (!stricmp(dirp->d_name, dir.c_str())) {
			ret.push_back(dirp->d_name);
		}
	}
	(void)closedir(dp);

	return ret;
}

// Return a vector with all filesystem names of "parent/dir" relative to parent.
// Recurses to deal with slashes in dir.
static SCP_vector<SCP_string> unix_get_dir_names(const SCP_string& parent, const SCP_string& dir)
{
	size_t slash = dir.find_first_of("/\\");

	// no subdirectories, no need to recurse
	if (slash == std::string::npos) {
		return unix_get_single_dir_names(parent, dir);
	}

	// get the names of the first component of dir
	SCP_vector<SCP_string> this_dir_names = unix_get_single_dir_names(parent, dir.substr(0, slash));

	SCP_string rest = dir.substr(slash + 1);

	SCP_vector<SCP_string> ret;

	// search for the rest of dir in each of these
	SCP_vector<SCP_string>::iterator ii, end = this_dir_names.end();
	for (ii = this_dir_names.begin(); ii != end; ++ii) {
		SCP_string this_dir_path = parent + "/" + *ii;
		SCP_vector<SCP_string> mod_path = unix_get_dir_names(this_dir_path, rest);

		// add all found paths relative to parent
		SCP_vector<SCP_string>::iterator ii2, end2 = mod_path.end();
		for (ii2 = mod_path.begin(); ii2 != end2; ++ii2) {
			ret.push_back(*ii + "/" + *ii2);
		}
	}

	return ret;
}

// For case sensitive filesystems (e.g. Linux/BSD) perform case-insensitive dir matches.
static void handle_unix_modlist(char **modlist, size_t *len)
{
	// search filesystem for given paths
	SCP_vector<SCP_string> mod_paths;
	for (char *cur_mod = strtok(*modlist, ","); cur_mod != NULL; cur_mod = strtok(NULL, ","))
	{
		SCP_vector<SCP_string> this_mod_paths = unix_get_dir_names(".", cur_mod);
		// Ignore non-existing mods for unit tests
		if (!running_unittests && this_mod_paths.empty()) {
			ReleaseWarning(LOCATION, "Can't find mod '%s'. Ignoring.", cur_mod);
		}
		mod_paths.insert(mod_paths.end(), this_mod_paths.begin(), this_mod_paths.end());
	}

	// create new char[] to replace modlist
	size_t total_len = 0;
	SCP_vector<SCP_string>::iterator ii, end = mod_paths.end();
	for (ii = mod_paths.begin(); ii != end; ++ii) {
		total_len += ii->length() + 1;
	}

	char *new_modlist = new char[total_len + 1];
	memset(new_modlist, 0, total_len + 1);
	end = mod_paths.end();
	for (ii = mod_paths.begin(); ii != end; ++ii) {
		strcat_s(new_modlist, total_len + 1, ii->c_str());
		strcat_s(new_modlist, total_len + 1, ","); // replace later with NUL
	}

	// make the rest of the modlist manipulation unaware that anything happened here
	delete [] *modlist;
	*modlist = new_modlist;
	*len = total_len;
}
#endif /* SCP_UNIX */

// external entry point into this modules

static json_t* json_get_v1() {
	auto root = json_object();

	{
		auto version_obj = json_object();

		json_object_set_new(version_obj, "full", json_string(FS_VERSION_FULL));
		json_object_set_new(version_obj, "major", json_integer(FS_VERSION_MAJOR));
		json_object_set_new(version_obj, "minor", json_integer(FS_VERSION_MINOR));
		json_object_set_new(version_obj, "build", json_integer(FS_VERSION_BUILD));

		json_object_set_new(version_obj, "has_revision", json_boolean(FS_VERSION_HAS_REVIS));
		json_object_set_new(version_obj, "revision", json_integer(FS_VERSION_REVIS));
		json_object_set_new(version_obj, "revision_str", json_string(FS_VERSION_REVIS_STR));

		json_object_set_new(root, "version", version_obj);
	}
	{
		auto easy_array = json_array();

		for (auto& easy_flag : easy_flags) {
			json_array_append_new(easy_array, json_string(easy_flag.name));
		}

		json_object_set_new(root, "easy_flags", easy_array);
	}
	{
		auto flags_array = json_array();

		for (auto& flag : exe_params) {
			auto flag_obj = json_object();

			json_object_set_new(flag_obj, "name", json_string(flag.name));
			json_object_set_new(flag_obj, "description", json_string(flag.desc));
			json_object_set_new(flag_obj, "fso_only", json_boolean(flag.fso_only));
			json_object_set_new(flag_obj, "on_flags", json_integer(flag.on_flags));
			json_object_set_new(flag_obj, "off_flags", json_integer(flag.off_flags));
			json_object_set_new(flag_obj, "type", json_string(flag.type));
			json_object_set_new(flag_obj, "web_url", json_string(flag.web_url));

			json_array_append_new(flags_array, flag_obj);
		}

		json_object_set_new(root, "flags", flags_array);
	}
	{
		auto caps_array = json_array();

		json_array_append_new(caps_array, json_string("OpenAL"));
		json_array_append_new(caps_array, json_string("No D3D"));
		json_array_append_new(caps_array, json_string("New Sound"));
		json_array_append_new(caps_array, json_string("SDL"));
		json_array_append_new(caps_array, json_string("Multijoy"));

		json_object_set_new(root, "caps", caps_array);
	}
	{
		auto voices_array = json_array();

		auto voices = speech_enumerate_voices();

		for (auto& voice : voices) {
			json_array_append_new(voices_array, json_string(voice.c_str()));
		}

		json_object_set_new(root, "voices", voices_array);
	}
	{
		auto resolution_array = json_array();

		auto displays = gr_enumerate_displays();

		for (auto& display : displays) {
			auto display_obj = json_object();

			json_object_set_new(display_obj, "index", json_integer(display.index));
			json_object_set_new(display_obj, "name", json_string(display.name.c_str()));

			json_object_set_new(display_obj, "x", json_integer(display.x));
			json_object_set_new(display_obj, "y", json_integer(display.y));
			json_object_set_new(display_obj, "width", json_integer(display.width));
			json_object_set_new(display_obj, "height", json_integer(display.height));

			auto modes_array = json_array();

			for (auto& mode : display.video_modes) {
				auto mode_obj = json_object();

				json_object_set_new(mode_obj, "x", json_integer(mode.width));
				json_object_set_new(mode_obj, "y", json_integer(mode.height));
				json_object_set_new(mode_obj, "bits", json_integer(mode.bit_depth));

				json_array_append_new(modes_array, mode_obj);
			}

			json_object_set_new(display_obj, "modes", modes_array);

			json_array_append_new(resolution_array, display_obj);
		}

		json_object_set_new(root, "displays", resolution_array);
	}
	{
		auto openal_obj = json_object();

		auto openal_info = openal_get_platform_information();

		json_object_set_new(openal_obj, "version_major", json_integer(openal_info.version_major));
		json_object_set_new(openal_obj, "version_minor", json_integer(openal_info.version_minor));

		json_object_set(openal_obj, "default_playback", json_string(openal_info.default_playback_device.c_str()));
		json_object_set(openal_obj, "default_capture", json_string(openal_info.default_capture_device.c_str()));

		{
			auto playback_array = json_array();

			for (auto& device : openal_info.playback_devices) {
				json_array_append_new(playback_array, json_string(device.c_str()));
			}

			json_object_set_new(openal_obj, "playback_devices", playback_array);
		}
		{
			auto capture_array = json_array();

			for (auto& device : openal_info.capture_devices) {
				json_array_append_new(capture_array, json_string(device.c_str()));
			}

			json_object_set_new(openal_obj, "capture_devices", capture_array);
		}
		{
			auto efx_support_obj = json_object();

			for (auto& pair : openal_info.efx_support) {
				json_object_set_new(efx_support_obj, pair.first.c_str(), json_boolean(pair.second));
			}

			json_object_set_new(openal_obj, "efx_support", efx_support_obj);
		}

		json_object_set_new(root, "openal", openal_obj);
	}
	{
		auto joystick_array = io::joystick::getJsonArray();

		json_object_set_new(root, "joysticks", joystick_array);
	}
	{
		json_object_set_new(root, "pref_path", json_string(os_get_config_path().c_str()));
	}

	return root;
}

static void write_flags_file() {
	FILE *fp = fopen("flags.lch","w");

	if (fp == NULL) {
		os::dialogs::Message(os::dialogs::MESSAGEBOX_ERROR, "Error creating flag list for launcher");
		return;
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
		build_caps |= BUILD_CAPS_OPENAL;
		build_caps |= BUILD_CAPS_NO_D3D;
		build_caps |= BUILD_CAPS_NEW_SND;
		build_caps |= BUILD_CAPS_SDL;

		fwrite(&build_caps, 1, 1, fp);
	}

	fflush(fp);
	fclose(fp);
}

static flag_output_type get_flags_output_type() {
	Assertion(get_flags_arg.found(), "This function is only valid if " GET_FLAGS_STRING " is present!");

	if (!get_flags_arg.has_param()) {
		// Default to binary mode
		return flag_output_type::Binary;
	}

	SCP_string type(get_flags_arg.str());

	if (type == "binary") {
		return flag_output_type::Binary;
	} else if (type == "json_v1") {
		return flag_output_type::Json_V1;
	} else {
		// This is supposed to make it easy for the launcher to recognize an unsupported type
		printf("OUTPUT TYPE NOT SUPPORTED!\n");
		// Default to json in this case so that no flags.lch file is created
		return flag_output_type::Json_V1;
	}
}

static void write_flags() {
	auto type = get_flags_output_type();
	switch(type) {
	case flag_output_type::Binary:
		write_flags_file();
		break;
	case flag_output_type::Json_V1:
		json_t* root = json_get_v1();

		json_dumpf(root, stdout, JSON_INDENT(4));
		json_decref(root);
		break;
	}
}

bool SetCmdlineParams()
// Sets externed variables used for communication cmdline information
{
	//getcwd(FreeSpace_Directory, 256); // set the directory to our fs2 root

	// DO THIS FIRST to avoid unrecognized flag warnings when just getting flag file
	if ( get_flags_arg.found() ) {
		write_flags();
		
		return false; 
	}

	if (joy_info.found()) {
		io::joystick::printJoyJSON();
		return false;
	}

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

#ifdef Allow_NoWarn
	if (nowarn_arg.found())
	{
		Cmdline_nowarn = 1;
	}
#endif

	if (extra_warn_arg.found())
	{
		Cmdline_extra_warn = 1;
	}

	if (!Fred_running) { //There is no standalone FRED
		// is this a standalone server??
		if (standalone_arg.found()) {
			Is_standalone = 1;

			// also enable non-interactive by default here
			Cmdline_noninteractive = true;
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

	// Disable enhanced sound
	if (noenhancedsound_arg.found()) {
		Cmdline_no_enhanced_sound = 1;
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
		if ( strlen(Cmdline_game_name) >= MAX_GAMENAME_LEN ) {
			Cmdline_game_name[MAX_GAMENAME_LEN-1] = '\0';
		}
	}

	// get the password for a pssword game
	if ( gamepassword_arg.found() ) {
		Cmdline_game_password = gamepassword_arg.str();

		// be sure that this string fits in our limits
		if ( strlen(Cmdline_game_password) >= MAX_PASSWD_LEN ) {
			ReleaseWarning(LOCATION, "Multi game password is longer than max of %d charaters and will be trimmed to fit!", MAX_PASSWD_LEN-1);
			Cmdline_game_password[MAX_PASSWD_LEN-1] = '\0';
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

	// get IP address of gateway, for auto port forwarding
	if ( gateway_ip_arg.found() ) {
		Cmdline_gateway_ip = gateway_ip_arg.str();
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

	// spew pof info
	if(pof_spew.found()){
		Cmdline_spew_pof_info = 1;
	}

	// spew weapon stats
	if (weapon_spew.found()) {
		Cmdline_spew_weapon_stats = WeaponSpewType::STANDARD;

		// currently just one argument
		if (weapon_spew.has_param()) {
			if (!stricmp(weapon_spew.str(), "all")) {
				Cmdline_spew_weapon_stats = WeaponSpewType::ALL;
			}
		}
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
	if(window_arg.found()) {
		// We need to set both values since we don't know if we are going to use the new config system
		options::OptionsManager::instance()->setOverride("Graphics.WindowMode", "0");
		Cmdline_window = 1;
	}

	if ( fullscreen_window_arg.found( ) )
	{
		options::OptionsManager::instance()->setOverride("Graphics.WindowMode", "1");
		Cmdline_fullscreen_window = 1;
		Cmdline_window = 0; /* Make sure no-one sets both */
	}

	if(res_arg.found()){
		Cmdline_res = res_arg.str();

		int width = 0;
		int height = 0;

		if ( sscanf(Cmdline_res, "%dx%d", &width, &height) == 2 ) {
			SCP_string override;
			sprintf(override, "{\"width\":%d,\"height\":%d}", width, height);
			options::OptionsManager::instance()->setOverride("Graphics.Resolution", override);
		} else {
			mprintf(("Failed to parse -res parameter \"%s\". Must be in format \"<width>x<height>\".\n", Cmdline_res));
		}
	}
	if(center_res_arg.found()){
		Cmdline_center_res = center_res_arg.str();
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

		// strip off blank space it it's there
		if ( Cmdline_mod[strlen(Cmdline_mod)-1] == ' ' ) {
			Cmdline_mod[strlen(Cmdline_mod)-1] = '\0';
		}

		// Ok - mod stacking support
		size_t len = strlen(Cmdline_mod);
		char *modlist = new char[len+2];
		memset( modlist, 0, len + 2 );
		strcpy_s(modlist, len+2, Cmdline_mod);

#ifdef SCP_UNIX
		// handle case-insensitive searching
		handle_unix_modlist(&modlist, &len);
#endif

		// null terminate each individual
		for (size_t i = 0; i < len; i++)
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

	if (bmpmanusage_arg.found())
	{
		Cmdline_bmpman_usage = 1;
	}

	if(pos_arg.found())
	{
		Cmdline_show_pos = 1;
	}

	if ( nomotiondebris_arg.found() ) {
		Motion_debris_enabled = false;
	}

	if( mipmap_arg.found() ) {
		Cmdline_mipmap = 1;
	}

	if( stats_arg.found() ) {
		Cmdline_show_stats = 1;
	}

	if ( fov_arg.found() ) {
		auto val = fov_arg.get_float();
		if (val > 0.1) {
			VIEWER_ZOOM_DEFAULT = val;
		} else {
			VIEWER_ZOOM_DEFAULT = 0.75f;
		}
	}

	if ( fov_cockpit_arg.found() ) {
		auto val = fov_cockpit_arg.get_float();
		if (val > 0.1) {
			COCKPIT_ZOOM_DEFAULT = val;
		}
		else {
			COCKPIT_ZOOM_DEFAULT = VIEWER_ZOOM_DEFAULT;
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
		Fireball_use_3d_warp = true;
	}

	if ( use_warp_flash.found() ) {
		Cmdline_warp_flash = 1;
	}

	if ( allow_autpilot_interrupt.found() )	{
		Cmdline_autopilot_interruptable = 0;
	}

	if ( no_screenshake.found() ) {
		Cmdline_no_screenshake = 1;
	}
	
	if ( deadzone.found() ) {
		Cmdline_deadzone = deadzone.get_int();
	}

	if ( stretch_menu.found() )	{
		Cmdline_stretch_menu = 1;
	}
	// new lighting lines
	if ( ambient_power_arg.found() )
		Cmdline_ambient_power = ambient_power_arg.get_float();

	if (emissive_power_arg.found() && emissive_power_arg.has_param()) {
		Cmdline_emissive_power = emissive_power_arg.get_float();
	} else if (emissive_arg.found() || emissive_power_arg.found()) {
		// for legacy support no argument param defaults to the old emissive value
		Cmdline_emissive_power = 0.30f;
	}

	if (light_power_arg.found())
		Cmdline_light_power = light_power_arg.get_float();

	if (spec_arg.found()) {
		Cmdline_spec = 0;
	}

	if( no_set_gamma_arg.found() )
	{
		Cmdline_no_set_gamma = 1;
	}

	if(no_vsync_arg.found() )
	{
		Gr_enable_vsync = false;
	}

	if ( normal_arg.found() ) {
		Cmdline_normal = 0;
	}

	if ( height_arg.found() ) {
		Cmdline_height = 0;
	}

	if (post_process_aa_arg.found() || post_process_aa_preset_arg.found()) {
		Gr_aa_mode = AntiAliasMode::SMAA_Low;

		if (post_process_aa_preset_arg.found()) {
			switch (post_process_aa_preset_arg.get_int()) {
			case 0: 
				Gr_aa_mode = AntiAliasMode::FXAA_Low;
				break;
			case 1: 
				Gr_aa_mode = AntiAliasMode::FXAA_Medium;
				break;
			case 2: 
				Gr_aa_mode = AntiAliasMode::FXAA_High;
				break;
			case 3: 
				Gr_aa_mode = AntiAliasMode::SMAA_Low;
				break;
			case 4: 
				Gr_aa_mode = AntiAliasMode::SMAA_Medium;
				break;
			case 5: 
				Gr_aa_mode = AntiAliasMode::SMAA_High;
				break;
			case 6: 
				Gr_aa_mode = AntiAliasMode::SMAA_Ultra;
				break;
			}
		}
	}

	if ( glow_arg.found() )
		Cmdline_glow = 0;

	if ( ship_choice_3d_arg.found() )
		Cmdline_ship_choice_3d = 1;

	if ( weapon_choice_3d_arg.found() )
		Cmdline_weapon_choice_3d = 1;

	if (ingamejoin_arg.found() )
		Cmdline_ingamejoin = 1;

	if ( start_mission_arg.found() ) {
		Cmdline_start_mission = start_mission_arg.str();
	}

	if ( output_scripting_arg.found() )
		Output_scripting_meta = true;

	if (output_script_json_arg.found() )
		Output_scripting_json = true;

	if (output_sexp_arg.found() ) {
		Cmdline_output_sexp_info = true;
	}

	if (generate_controlconfig_arg.found())
		Generate_controlconfig_table = true;

	if ( no_pbo_arg.found() )
	{
		Cmdline_no_pbo = 1;
	}

	if ( no_drawrangeelements.found() )
	{
		Cmdline_drawelements = 1;
	}

	if( keyboard_layout.found())
	{
		Cmdline_keyboard_layout = keyboard_layout.str();
	}

	if (gl_finish.found())
	{
		Cmdline_gl_finish = true;
	}

	if ( no_geo_sdr_effects.found() )
	{
		Cmdline_no_geo_sdr_effects = true;
	}

	if (set_cpu_affinity.found())
	{
		Cmdline_set_cpu_affinity = true;
	}

	if (nograb_arg.found())
	{
		Cmdline_nograb = true;
	}

	if (noshadercache_arg.found())
	{
		Cmdline_noshadercache = true;
	}

	if (portable_mode.found())
	{
		Cmdline_portable_mode = true;
	}
	
#ifdef WIN32
	if (fix_registry.found()) {
		Cmdline_alternate_registry_path = true;
	}
#endif

	if ( env.found() ) {
		Cmdline_env = 0;
	}

	if ( ballistic_gauge.found() ) {
		Cmdline_ballistic_gauge = 1;
	}

	if(dis_collisions.found())
		Cmdline_dis_collisions = 1;

	if(dis_weapons.found())
		Cmdline_dis_weapons = 1;

	if ( no_fbo_arg.found() ) {
		Cmdline_no_fbo = 1;
	}

	if ( rearm_timer_arg.found() )
		Cmdline_rearm_timer = 1;

	if ( save_render_targets_arg.found() )
		Cmdline_save_render_targets = 1;
	
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
		Gr_framebuffer_effects.set(FramebufferEffects::Shockwaves, true);
	}

    if (fb_thrusters_arg.found()) 
    {
	    Gr_framebuffer_effects.set(FramebufferEffects::Thrusters, true);
    }

	if ( no_postprocess_arg.found() )
	{
		Gr_post_processing_enabled = false;
	}

	if ( bloom_intensity_arg.found() )
	{
		Cmdline_bloom_intensity = bloom_intensity_arg.get_int();
	}

	if ( flightshaftsoff_arg.found() )
	{
		Cmdline_force_lightshaft_off = true;
	}

	if( reparse_mainhall_arg.found() )
	{
		Cmdline_reparse_mainhall = 1;
	}

	if( enable_shadows_arg.found() )
	{
		// if only using `enable_shadows` then default quality level can be overriden in mod_settings.tbl --wookieejedi
		Shadow_quality_uses_mod_option = true;

		Shadow_quality = ShadowQuality::Medium;
	}

	if( shadow_quality_arg.found() )
	{
		// set that we are not using default shadow quality level --wookieejedi
		Shadow_quality_uses_mod_option = false; 

		switch (shadow_quality_arg.get_int()) {
		case 0:
			Shadow_quality = ShadowQuality::Disabled;
			break;
		case 1:
			Shadow_quality = ShadowQuality::Low;
			break;
		case 2:
			Shadow_quality = ShadowQuality::Medium;
			break;
		case 3:
			Shadow_quality = ShadowQuality::High;
			break;
		case 4:
			Shadow_quality = ShadowQuality::Ultra;
			break;
		default:
			mprintf(("Invalid shadow quality %d. Disabling shadows...\n", shadow_quality_arg.get_int()));
			Shadow_quality = ShadowQuality::Disabled;
			break;
		}
	}

	if( no_deferred_lighting_arg.found() )
	{
		Cmdline_no_deferred_lighting = 1;
	}

	if (anisotropy_level_arg.found()) 
	{
		Cmdline_aniso_level = anisotropy_level_arg.get_int();
	}

	if (frame_profile_write_file.found())
	{
		Cmdline_profile_write_file = true;
	}

	if (no_unfocused_pause_arg.found())
	{
		Cmdline_no_unfocus_pause = true;
	}
	
	if (benchmark_mode_arg.found())
	{
		Cmdline_benchmark_mode = true;
	}

	if (pilot_arg.found())
	{
		Cmdline_pilot = pilot_arg.str();
	}

	if (noninteractive_arg.found())
	{
		Cmdline_noninteractive = true;
	}

	if (json_profiling.found())
	{
		Cmdline_json_profiling = true;
	}

	if (frame_profile_arg.found() )
	{
		Cmdline_frame_profile = true;
	}

	if (debug_window_arg.found()) {
		Cmdline_debug_window = true;
	}

	if (graphics_debug_output_arg.found()) {
		Cmdline_graphics_debug_output = true;
	}

	if (log_to_stdout_arg.found()) {
		Cmdline_log_to_stdout = true;
	}

	if (slow_frames_ok_arg.found()) {
		Cmdline_slow_frames_ok = true;
	}
	if ( luadev_arg.found()) {
		Cmdline_lua_devmode = true;
	}

	if (show_video_info.found())
	{
		Cmdline_show_video_info = true;
	}

	//Deprecated flags - CommanderDJ
	if (deprecated_no_emissive_arg.found()) {
		Cmdline_emissive = 0;
	}

	if (deprecated_fxaa_arg.found() ) {
		Gr_aa_mode = AntiAliasMode::FXAA_Medium;

		if (deprecated_fxaa_preset_arg.found()) {
			auto val = deprecated_fxaa_preset_arg.get_int();
			if (val > 6) {
				Gr_aa_mode = AntiAliasMode::FXAA_High;
			} else if (val > 3) {
				Gr_aa_mode = AntiAliasMode::FXAA_Medium;
			} else {
				Gr_aa_mode = AntiAliasMode::FXAA_Low;
			}
		}
	}

	if (deprecated_smaa_arg.found()) {
		Gr_aa_mode = AntiAliasMode::SMAA_Medium;

		if (deprecated_smaa_preset_arg.found()) {
			switch (deprecated_smaa_preset_arg.get_int()) {
			case 0:
				Gr_aa_mode = AntiAliasMode::SMAA_Low;
				break;
			case 1:
				Gr_aa_mode = AntiAliasMode::SMAA_Medium;
				break;
			case 2:
				Gr_aa_mode = AntiAliasMode::SMAA_High;
				break;
			case 3:
				Gr_aa_mode = AntiAliasMode::SMAA_Ultra;
				break;
			default:
				Gr_aa_mode = AntiAliasMode::SMAA_Ultra;
				break;
			}
		}
	}

	if (prefer_ipv4_arg.found()) {
		Cmdline_prefer_ipv4 = true;
	}

	if (prefer_ipv6_arg.found()) {
		Cmdline_prefer_ipv6 = true;

		// Rule 2: Not both
		if (Cmdline_prefer_ipv4) {
			ReleaseWarning(LOCATION, "Cannot set preference for both IPv4 and IPv6! Reverting to default behavior...\n");

			Cmdline_prefer_ipv4 = false;
			Cmdline_prefer_ipv6 = false;
		}
	}
 
	if (log_multi_packet_arg.found()) {
		Cmdline_dump_packet_type = true;
	}

	if (fixed_seed_rand.found()) {
		Cmdline_rng_seed = abs(fixed_seed_rand.get_int());
		if (Cmdline_rng_seed>0) {
			Cmdline_reuse_rng_seed = true;
		}
		else {
			Warning(LOCATION,"-seed must be an integer greater than 0. Invalid input seed will be disregarded.");
		}
	}

	return true; 
}

int parse_cmdline(int argc, char *argv[])
{
//	mprintf(("I got to parse_cmdline()!!\n"));

	os_init_cmdline(argc, argv);

	// --------------- Kazan -------------
	// If you're looking for the list of if (someparam.found()) { cmdline_someparam = something; } look above at this function
	// I did this because of fred2_parse_cmdline()
	return SetCmdlineParams();
}

const char * get_param_desc(const char *flag_name)
{
	int i;
	int flag_size = sizeof(Flag);
	int num_flags = sizeof(exe_params) / flag_size;
	for (i = 0; i < num_flags; ++i) {
		if (!strcmp(flag_name, exe_params[i].name)) {
			return exe_params[i].desc;
		}
	}
	return "UNKNOWN - FIXME!";
}
