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


int parse_cmdline(char *cmdline);

int fred2_parse_cmdline(int argc, char *argv[]);
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
extern int Cmdline_client_dodamage;
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


// FSO OPTIONS -------------------------------------------------

// Graphics related
extern double specular_exponent_value;
extern float Cmdline_clip_dist;
extern float Cmdline_fov;
extern float Cmdline_ogl_spec;
extern float static_light_factor;
extern float static_point_factor;
extern float static_tube_factor;
extern int Cmdline_ambient_factor;
extern int Cmdline_env;
extern int Cmdline_mipmap;
extern int Cmdline_missile_lighting;
extern int Cmdline_glow;
extern int Cmdline_nomotiondebris;
extern int Cmdline_noscalevid;	// disables fit-to-window for movies - taylor
extern int Cmdline_spec;
extern int Cmdline_normal;
extern int Cmdline_height;
extern int Cmdline_enable_3d_shockwave;
extern int Cmdline_softparticles;
extern int Cmdline_postprocess;
extern int Cmdline_bloom_intensity;
extern bool Cmdline_fxaa;
extern int Cmdline_fxaa_preset;

// Game Speed related
extern int Cmdline_cache_bitmaps;
extern int Cmdline_img2dds;
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

// Audio related
extern int Cmdline_query_speech;
extern int Cmdline_snd_preload;
extern int Cmdline_voice_recognition;

// MOD related
extern char *Cmdline_mod;	 // DTP for mod support
// Multiplayer/Network related
extern char *Cmdline_almission;	// DTP for autoload mission (for multi only)
extern int Cmdline_ingamejoin;
extern int Cmdline_mpnoreturn;
extern char *Cmdline_spew_mission_crcs;
extern char *Cmdline_spew_table_crcs;
extern int Cmdline_objupd;

// Troubleshooting
extern int Cmdline_load_all_weapons;
extern int Cmdline_nohtl;
extern int Cmdline_noibx;
extern int Cmdline_nomovies;	// WMC Toggles movie playing support
extern int Cmdline_no_set_gamma;
extern int Cmdline_novbo;
extern int Cmdline_no_fbo;
extern int Cmdline_noglsl;
extern int Cmdline_ati_color_swap;
extern int Cmdline_no_3d_sound;
extern int Cmdline_no_glsl_model_rendering;
extern int Cmdline_no_di_mouse;
extern int Cmdline_drawelements;

// Developer/Testing related
extern char *Cmdline_start_mission;
extern int Cmdline_dis_collisions;
extern int Cmdline_dis_weapons;
extern int Cmdline_noparseerrors;
extern int Cmdline_nowarn;
extern int Cmdline_extra_warn;
extern int Cmdline_show_mem_usage;
extern int Cmdline_show_pos;
extern int Cmdline_show_stats;
extern int Cmdline_save_render_targets;
extern int Cmdline_debug_window;
extern int Cmdline_verify_vps;
#ifdef SCP_UNIX
extern int Cmdline_no_grab;
#endif



//extern char FreeSpace_Directory[]; // allievating a cfilesystem problem caused by fred -- Kazan
#endif
