/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/pstypes.h"
#include "object/object.h"
#include "io/key.h"
#include "ship/ship.h"
#include "graphics/2d.h"
#include "mission/missionparse.h"
#include "network/psnet2.h"
#include "stats/scoring.h"
#include "cfile/cfile.h"

float flFrametime;
int	game_zbuffer = 1;
int	Current_mission = 0xdeadbeef;
char **Builtin_mission_names;
char Game_current_mission_filename[MAX_FILENAME_LEN];
CFILE *Working_demo;
struct beam_info;
bool Env_cubemap_drawn = false;
int Multi_ping_timestamp = -1;
int Sun_drew = 0;
int Networking_disabled = 0;

void init_ets(struct object*){}

control_info PlayerControls;

char *  Game_CDROM_dir = NULL;

void game_flash(float r, float g, float b )
{
}

void freespace_menu_background()
{
	gr_reset_clip();
	gr_clear();
}

int My_observer_num;

void std_update_goals()
{
}

void os_close()
{
}

int gameseq_get_state(void)
{
	return 0;
}

int game_check_key()
{
	return key_inkey();
}

int game_poll()
{
	return key_inkey();
}

void multi_delete_ship(object *obj)
{
}

void send_homing_fired_packet()
{
}

void game_flush()
{
}

typedef struct config_struct
{
	int	boob;
} config_struct;

config_struct default_config;

typedef struct netgame_info
{
	int bubba;
} netgame_info;

void send_netgame_state_packet()
{
}

void send_update_state_packet()
{
}

void send_goal_status_packet()
{
}

int Show_area_effect;

void state_set_mem(unsigned char *c, int i) {}
int state_check_mem(unsigned char *c, int i) { return 0; }

void demo_do_flag_dead(int i) {}
void demo_checkpoint() {}
void demo_set_playback_filter() {}

void multi_end_sequence()
{
}
void multi_server_respawn() {}

void multi_build_respawn_points() {}

void store_p_object( p_object *pbojp, CFILE *fp ) {}
void restore_p_object( p_object *pobjp, CFILE *dp) {}

int Multi_squad_msg_targ;
int Multi_squad_msg_local;
void send_support_warpin_packet(int,int,int){}
void send_support_warpin_packet( int net_sig, int how ) {}

int demo_query_debug(int id) { return 0; };

void send_support_warpin_packet(int){}
void game_whack_apply(float x, float y) {}

void save_restore_vector(vec3d *vec, CFILE *fp, int version, vec3d *deflt) {}
void save_restore_matrix(matrix *mat, CFILE *fp, int version, matrix *deflt) {}
void save_restore_float(float *fl, CFILE *fp, int version, float deflt) {}
void save_restore_angles(angles *ang, CFILE *fp, int version, angles *deflt) {}
void save_restore_int(int *n, CFILE *fp, int version, int deflt) {}
void save_restore_uint(uint *n, CFILE *fp, int version, uint deflt) {}
void save_restore_short(short *n, CFILE *fp, int version, short deflt) {}
void save_restore_ushort(ushort *n, CFILE *fp, int version, ushort deflt) {}
void save_restore_ubyte(ubyte *n, CFILE *fp, int version, ubyte deflt) {}
void save_restore_fix(fix *n, CFILE *fp, int version, fix deflt) {}
void save_restore_string(char *str, CFILE *fp, int len, int version, char *deflt) {}
char *restore_string_alloc(CFILE *fp, int version, char *deflt) { return NULL; }

void save_restore_p_object(p_object *pobj, CFILE *fp) {}

void demo_write_char(char x) {}
char demo_read_char() { return 0; }

int	red_alert_default_status() {return 0;}

void send_ship_kill_packet(struct object *,struct object *,float,unsigned char) {}
void send_debris_create_packet( object *objp, ushort net_sig, int model_num, vec3d pos) {}

int Game_subspace_effect;
void big_explosion_flash(float x) {}

int game_do_cd_check(char *) {return 0;}

void game_stop_looped_sounds() {}

int Game_skill_level;
int game_cd_changed(void) {return 0;}

int Interface_framerate;
void game_set_view_clip(){}

int Warpout_forced = 0;
float Warpout_time;
vec3d Dead_player_last_vel;
int game_start_mission(){return 0;}
int Game_weapons_tbl_valid;
int Game_ships_tbl_valid;
void game_level_close(){}
void game_enter_state(int, int){}
void game_leave_state(int, int){}
int Test_begin;
int Debug_octant;
int Framerate_delay;
void game_start_time(){}
void game_stop_time(){}
int game_get_default_skill_level(){return 0;}
float FreeSpace_gamma;
int set_cdrom_path(int){return 0;}
int find_freespace_cd(char*){return 0;}
void get_version_string(){}
void game_do_state_common(int, int){}
void game_set_frametime(int){}
void game_increase_skill_level(){}
void get_version_string(char*, int){}
int Show_target_weapons;
int Show_target_debug_info;
int Game_do_state_should_skip;
long Game_time_compression;
struct fs_builtin_mission *game_find_builtin_mission(char*){return NULL;}
void game_format_time(long, char*){}
void game_do_state(int){}
void game_process_event(int, int){}
void game_shudder_apply(int, float){}
int game_hacked_data(){return 0;}
int game_single_step;
int last_single_step;
void get_version_string_short(char *){}
void game_tst_mark(struct object *, struct ship *){}
int tst;
//int Player_multi_died_check;
int Show_framerate = 0;

void alt_tab_pause(){}

void game_feature_disabled_popup() {}

void game_pause() {}
void game_unpause() {}

//Time stuff
bool Time_compression_locked;
float flRealframetime;
void lock_time_compression(bool is_locked){}
void change_time_compression(float multiplier){}
void set_time_compression(float multiplier, float change_time){}
fix game_get_overall_frametime() { return 0; }

//WMC
void game_level_init(int seed){}
void game_post_level_init(){}
camid game_render_frame_setup(){return camid();}
void game_render_frame(camid cid){}
void game_simulation_frame(){}
void game_update_missiontime(){}
void game_render_post_frame(){}
