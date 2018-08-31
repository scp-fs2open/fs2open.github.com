/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *

*/ 


#ifndef FS_CMDLINE_HEADER_FILE
#define FS_CMDLINE_HEADER_FILE

#include "globalincs/pstypes.h"

int parse_cmdline(const SCP_string& baseDir, int argc, char *argv[]);

// COMMAND LINE SETTINGS
// This section is for reference by all the *_init() functions. For example, the multiplayer init function
// could check to see if (int Cmdline_multi_stream_chat_to_file) has been set by the command line parser.
//
// Add any extern definitions here and put the actual variables inside of cmdline.cpp for ease of use
// Also, check to make sure anything you add doesn't break Fred or TestCode


// RETAIL OPTIONS ----------------------------------------------
extern SCP_string Cmdline_connect_addr;
extern SCP_string Cmdline_game_name;
extern SCP_string Cmdline_game_password;
extern SCP_string Cmdline_rank_above;
extern SCP_string Cmdline_rank_below;
extern int Cmdline_cd_check;
extern int Cmdline_closed_game;
extern int Cmdline_freespace_no_music;
extern int Cmdline_freespace_no_sound;
extern int Cmdline_gimme_all_medals;
extern int Cmdline_mouse_coords;
extern int Cmdline_multi_log;
extern int Cmdline_multi_stream_chat_to_file;
extern int Cmdline_network_port;
extern int Cmdline_restricted_game;
extern int Cmdline_spew_pof_info;
extern int Cmdline_start_netgame;
extern int Cmdline_timeout;
extern int Cmdline_use_last_pilot;
extern int Cmdline_window;
extern int Cmdline_fullscreen_window;
extern SCP_string Cmdline_res;
extern SCP_string Cmdline_center_res;


// FSO OPTIONS -------------------------------------------------

// Graphics related
extern float Cmdline_clip_dist;
extern float static_light_factor;
extern float static_point_factor;
extern float static_tube_factor;
extern int Cmdline_ambient_factor;
extern int Cmdline_env;
extern int Cmdline_glow;
extern int Cmdline_noscalevid;	// disables fit-to-window for movies - taylor
extern int Cmdline_spec;
extern int Cmdline_normal;
extern int Cmdline_height;
extern int Cmdline_enable_3d_shockwave;
extern int Cmdline_softparticles;
extern int Cmdline_bloom_intensity;
extern bool Cmdline_force_lightshaft_off;
extern int Cmdline_no_deferred_lighting;
extern int Cmdline_emissive;
extern int Cmdline_aniso_level;

// Game Speed related
extern int Cmdline_NoFPSCap;
extern int Cmdline_no_vsync;

// HUD related
extern int Cmdline_ballistic_gauge;
extern int Cmdline_dualscanlines;
extern int Cmdline_orb_radar;
extern int Cmdline_rearm_timer;
extern int Cmdline_targetinfo;

// Gameplay related
extern int Cmdline_3dwarp;
extern int Cmdline_ship_choice_3d;
extern int Cmdline_weapon_choice_3d;
extern int Cmdline_warp_flash;
extern int Cmdline_autopilot_interruptable;
extern int Cmdline_stretch_menu;
extern int Cmdline_no_screenshake;

// Audio related
extern int Cmdline_voice_recognition;
extern int Cmdline_no_enhanced_sound;

namespace cfile {
struct ModRoot;
} // namespace cfile

// MOD related
extern SCP_vector<cfile::ModRoot> Cmdline_mod;	 // DTP for mod support
// Multiplayer/Network related
extern SCP_string Cmdline_almission;	// DTP for autoload mission (for multi only)
extern int Cmdline_ingamejoin;
extern int Cmdline_mpnoreturn;
extern int Cmdline_objupd;
extern SCP_string Cmdline_gateway_ip;

// Launcher related options
extern bool Cmdline_portable_mode;

// Troubleshooting
extern int Cmdline_load_all_weapons;
extern int Cmdline_nomovies;	// WMC Toggles movie playing support
extern int Cmdline_no_set_gamma;
extern int Cmdline_no_fbo;
extern int Cmdline_no_pbo;
extern int Cmdline_mipmap;
extern int Cmdline_ati_color_swap;
extern int Cmdline_no_3d_sound;
extern int Cmdline_drawelements;
extern SCP_string Cmdline_keyboard_layout;
extern bool Cmdline_gl_finish;
extern bool Cmdline_no_geo_sdr_effects;
extern bool Cmdline_set_cpu_affinity;
extern bool Cmdline_nograb;
extern bool Cmdline_noshadercache;
#ifdef WIN32
extern bool Cmdline_alternate_registry_path;
#endif

// Developer/Testing related
extern SCP_string Cmdline_start_mission;
extern int Cmdline_dis_collisions;
extern int Cmdline_dis_weapons;
extern bool Cmdline_output_sexp_info;
extern int Cmdline_noparseerrors;
extern int Cmdline_extra_warn;
extern int Cmdline_bmpman_usage;
extern int Cmdline_show_pos;
extern int Cmdline_show_stats;
extern int Cmdline_save_render_targets;
extern int Cmdline_verify_vps;
extern int Cmdline_reparse_mainhall;
extern bool Cmdline_profile_write_file;
extern bool Cmdline_no_unfocus_pause;
extern bool Cmdline_benchmark_mode;
extern bool Cmdline_noninteractive;
extern bool Cmdline_json_profiling;
extern bool Cmdline_frame_profile;
extern bool Cmdline_show_video_info;
extern bool Cmdline_debug_window;
extern bool Cmdline_graphics_debug_output;
extern bool Cmdline_log_to_stdout;

enum class WeaponSpewType { NONE = 0, STANDARD, ALL };
extern WeaponSpewType Cmdline_spew_weapon_stats;


#endif
