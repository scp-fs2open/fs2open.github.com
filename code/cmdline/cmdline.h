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

#include <tl/optional.hpp>

int parse_cmdline(int argc, char *argv[]);

// COMMAND LINE SETTINGS
// This section is for reference by all the *_init() functions. For example, the multiplayer init function
// could check to see if (int Cmdline_multi_stream_chat_to_file) has been set by the command line parser.
//
// Add any extern definitions here and put the actual variables inside of cmdline.cpp for ease of use
// Also, check to make sure anything you add doesn't break Fred or TestCode


// RETAIL OPTIONS ----------------------------------------------
extern char *Cmdline_connect_addr;
extern char *Cmdline_game_name;
extern char *Cmdline_game_password;
extern char *Cmdline_rank_above;
extern char *Cmdline_rank_below;
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
extern char *Cmdline_res;
extern tl::optional<std::pair<uint16_t, uint16_t>>Cmdline_window_res;
extern char *Cmdline_center_res;


// FSO OPTIONS -------------------------------------------------

// Graphics related
extern float Cmdline_ambient_power;
extern float Cmdline_emissive_power;
extern float Cmdline_light_power;
extern int Cmdline_env;
extern int Cmdline_glow;
extern int Cmdline_noscalevid;	// disables fit-to-window for movies - taylor
extern int Cmdline_spec;
extern int Cmdline_normal;
extern int Cmdline_height;
extern int Cmdline_softparticles;
extern int Cmdline_no_deferred_lighting;
extern bool Cmdline_deferred_lighting_cockpit;
extern int Cmdline_emissive;
extern int Cmdline_aniso_level;
extern int Cmdline_msaa_enabled;

// Game Speed related
extern int Cmdline_NoFPSCap;
extern int Cmdline_no_vsync;

// HUD related
extern int Cmdline_ballistic_gauge;
extern int Cmdline_dualscanlines;
extern int Cmdline_orb_radar;
extern int Cmdline_rearm_timer;

// Gameplay related
extern int Cmdline_autopilot_interruptable;
extern int Cmdline_stretch_menu;
extern bool Cmdline_capture_mouse;
extern int Cmdline_deadzone;
extern bool Cmdline_enable_vr;

// Audio related
extern int Cmdline_voice_recognition;
extern int Cmdline_no_enhanced_sound;

// MOD related
extern char *Cmdline_mod;	 // DTP for mod support
// Multiplayer/Network related
extern char *Cmdline_almission;	// DTP for autoload mission (for multi only)
extern int Cmdline_ingamejoin;
extern int Cmdline_mpnoreturn;
extern int Cmdline_objupd;
extern char *Cmdline_gateway_ip;

// Launcher related options
extern bool Cmdline_portable_mode;
extern SCP_string Cmdline_lang;

// Troubleshooting
extern int Cmdline_load_all_weapons;
extern int Cmdline_nomovies;	// WMC Toggles movie playing support
extern int Cmdline_no_set_gamma;
extern bool Cmdline_no_ingame_options;
extern int Cmdline_no_fbo;
extern int Cmdline_no_pbo;
extern int Cmdline_mipmap;
extern int Cmdline_ati_color_swap;
extern int Cmdline_no_3d_sound;
extern int Cmdline_drawelements;
extern char* Cmdline_keyboard_layout;
extern bool Cmdline_gl_finish;
extern bool Cmdline_no_geo_sdr_effects;
extern bool Cmdline_set_cpu_affinity;
extern bool Cmdline_nograb;
extern bool Cmdline_noshadercache;
extern bool Cmdline_prefer_ipv4;
extern bool Cmdline_prefer_ipv6;
extern bool Cmdline_dump_packet_type;
extern bool Cmdline_no_large_shaders;
#ifdef WIN32
extern bool Cmdline_alternate_registry_path;
#endif

// Developer/Testing related
extern char *Cmdline_start_mission;
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
extern bool Cmdline_retail_time_compression_range;
extern bool Cmdline_benchmark_mode;
extern const char *Cmdline_pilot;
extern bool Cmdline_noninteractive;
extern bool Cmdline_json_profiling;
extern bool Cmdline_frame_profile;
extern bool Cmdline_show_video_info;
extern bool Cmdline_debug_window;
extern bool Cmdline_graphics_debug_output;
extern bool Cmdline_log_to_stdout;
extern bool Cmdline_slow_frames_ok;
extern bool Cmdline_lua_devmode;
extern bool Cmdline_override_data;
extern bool Cmdline_show_imgui_debug;
extern bool Cmdline_vulkan;

enum class WeaponSpewType { NONE = 0, STANDARD, ALL };
extern WeaponSpewType Cmdline_spew_weapon_stats;


#endif
