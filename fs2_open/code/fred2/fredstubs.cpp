/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/FredStubs.cpp $
 * $Revision: 1.6 $
 * $Date: 2007-02-11 09:37:18 $
 * $Author: taylor $
 *
 * Bogus C file for functions and variable stubs that Fred needs because it
 * includes some libraries that makes functions calls to other libraries that FRED
 * doesn't include.  In a perfect world, programmers would work harder to keep
 * the code they write more self-contained and not tie all the code everywhere
 * to each other.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/07/14 03:50:31  taylor
 * add missing stub for game_get_overall_frametime()
 *
 * Revision 1.4  2006/04/20 06:32:01  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 1.3  2006/03/18 21:23:38  taylor
 * add framerate var since it's contained in freespace.cpp and that's not used in a FRED2 compile
 *
 * Revision 1.2  2006/01/20 05:35:59  wmcoolmon
 * Updated FREDStubs.cpp to most current version
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.12  2005/12/06 21:45:04  taylor
 * stub fix for get_version_string() change
 *
 * Revision 1.11  2005/10/16 18:49:26  taylor
 * add Networking_disabled
 *
 * Revision 1.10  2005/08/28 20:36:38  phreak
 * Added "Multi_ping_timestamp" variable to fred stubs
 *
 * Revision 1.9  2005/06/20 16:07:24  phreak
 * added game_pause() and game_unpause() to fred stubs.
 * fixed models rendering out of position.
 *
 * Revision 1.8  2005/05/18 05:19:15  phreak
 * stub added for game_feature_disabled_popup()
 *
 * Revision 1.7  2005/04/13 20:11:06  chhogg
 * Changed vector to vec3d for Fred.
 *
 * Revision 1.6  2005/03/19 23:17:01  phreak
 * Added the "cube_map_drawen" (lol) variable to the stub for fred to compile
 *
 * Revision 1.5  2005/03/06 22:43:14  wmcoolmon
 * Fixx0red some of the jump node errors
 *
 * Revision 1.4  2004/04/29 03:35:47  wmcoolmon
 * Added alt_tab_pause() stub
 *
 * Revision 1.3  2004/03/15 12:14:46  randomtiger
 * Fixed a whole heap of problems with Fred introduced by changes to global vars.
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:10:57  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 26    9/27/99 10:13a Jefff
 * another stub for not_in_demo popup
 * 
 * 25    9/13/99 5:16p Dave
 * New stubs
 * 
 * 24    9/06/99 8:43p Dave
 * Checked in some stubs.
 * 
 * 23    9/01/99 10:14a Dave
 * Pirate bob.
 * 
 * 22    8/26/99 10:15a Dave
 * Don't apply beam whacks to docked ships.
 * 
 * 21    8/04/99 2:24a Dave
 * Fixed escort gauge ordering for dogfight.
 * 
 * 20    7/08/99 10:53a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 19    6/10/99 11:11a Jamesa
 * More stubs.
 * 
 * 18    4/12/99 10:36a Johnson
 * Stub for game_hacked_data
 * 
 * 17    3/19/99 6:15p Dave
 * Stubs
 * 
 * 16    2/23/99 7:03p Dave
 * Rewrote a horribly mangled and evil team loadout dialog. Bugs gone.
 * 
 * 15    2/17/99 2:11p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 14    1/14/99 12:49a Dave
 * Made an attempt to put briefing icons back into FRED.
 * 
 * 13    1/12/99 5:45p Dave
 * Whole slew of new stubs.
 * 
 * 12    1/08/99 2:07p Dave
 * Temporary checkin. Super early support for AWACS and beam weapons.
 * 
 * 11    12/06/98 2:36p Dave
 * Stub.
 * 
 * 10    12/03/98 10:14a Dave
 * 
 * 9     11/30/98 5:32p Dave
 * Fixed up Fred support for software mode.
 * 
 * 8     11/19/98 8:05a Dave
 * Psnet stub
 * 
 * 7     11/12/98 12:12a Dave
 * Stub for new turret fired packet.
 * 
 * 6     10/21/98 9:56a Dave
 * Fixed stupid linker thing.
 * 
 * 5     10/13/98 9:57a Dave
 * 
 * 4     10/13/98 9:27a Dave
 * Started neatening up freespace.h
 * 
 * 3     10/12/98 1:01p Dave
 * Fixed object rotation bug (uninitialized data). Changed a few stubs to
 * correspond to new var names.
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 238   9/20/98 9:46p Dave
 * send_change_iff_packet() stub.
 * 
 * 237   9/15/98 11:44a Dave
 * Renamed builtin ships and wepaons appropriately in FRED. Put in scoring
 * scale factors. Fixed standalone filtering of MD missions to non-MD
 * hosts.
 * 
 * 236   9/12/98 2:17p Dave
 * Multiplayer reinforcement fix stub.
 * 
 * 235   8/28/98 3:28p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 234   8/26/98 2:14p Dave
 * 
 * 233   8/25/98 1:48p Dave
 * First rev of EMP effect. Player side stuff basically done. Next comes
 * AI code.
 * 
 * 232   8/11/98 9:21a Dave
 * new TvT stub.
 * 
 * 231   7/14/98 4:57p Allender
 * 
 * 230   7/06/98 5:08p Hoffoss
 * 
 * 229   6/22/98 11:02a Hoffoss
 * 
 * 228   6/09/98 12:12p Hoffoss
 * Added XSTR localization code.
 * 
 * 227   6/07/98 3:25p Lawrance
 * 
 * 226   5/25/98 2:20p Allender
 * scoring_level_close()
 * 
 * 225   5/23/98 6:53p Allender
 * AL: add Skill_level to stubs
 * 
 * 224   5/21/98 4:14a Allender
 * 
 * 223   5/18/98 1:56a Allender
 * respawn limit to 999 max
 * 
 * 222   5/18/98 12:41a Allender
 * fixed subsystem problems on clients (i.e. not reporting properly on
 * damage indicator).  Fixed ingame join problem with respawns.  minor
 * comm menu stuff
 * 
 * 221   5/15/98 11:04p Sandeep
 * fixed a bug caused by Alan. :)
 * 
 * 220   5/14/98 2:24p Sandeep
 * 
 * 219   5/13/98 11:34p Mike
 * Model caching system.
 * 
 * 218   5/13/98 11:16p Allender
 * 
 * 217   5/09/98 10:35p Allender
 * 
 * 216   5/09/98 9:49p Allender
 * 
 * 215   5/09/98 3:38p Sandeep
 * 
 * 214   5/08/98 11:20a Allender
 * fix ingame join trouble.  Small messaging fix.  Enable collisions for
 * friendlies again
 * 
 * 213   5/05/98 5:12p Sandeep
 * 
 * 212   5/05/98 11:10a Johnson
 * Fix help overlay link errors
 * 
 * 211   5/04/98 6:06p Lawrance
 * Make red alert mode work!
 * 
 * 210   4/30/98 9:15a Allender
 * 
 * 209   4/28/98 8:07a Jasen
 * JAS: Stubbed in cdrom_path
 * 
 * 208   4/27/98 10:17p Allender
 * 
 * 207   4/27/98 8:56p Jim
 * stub for modifcation of scoring_eval_hit
 * 
 * 206   4/26/98 12:42p Sandeep
 * stubbed send_turret_fired_packet.  Stop breaking fred! :)
 * 
 * 205   4/25/98 1:57p Mike
 * Oops, someone else had just stubbed out big_explosion_flash.
 * 
 * 204   4/25/98 1:56p Mike
 * Stub out big_explosion_flash().
 * 
 * 203   4/25/98 1:20p Jim
 * stub for big_explosion_flash
 * 
 * 202   4/20/98 8:54a Mike
 * Stub opposing_team_mask().
 * 
 * 201   4/13/98 9:53p Hoffoss
 * 
 * 200   4/13/98 2:48p Allender
 * countermeasure sucess packet
 * 
 * 199   4/12/98 12:15p Jim
 * Stub out Subspace_effect
 * 
 * 198   4/10/98 12:16p Allender
 * fix ship hit kill and debris packets
 * 
 * 197   4/10/98 9:15a Allender
 * 
 * 196   4/09/98 8:42p Sandeep
 * 
 * 195   4/09/98 12:46a Sandeep
 * 
 * 194   4/08/98 8:37a Mike
 * Stub out hud_find_target_distance().
 * 
 * 193   4/07/98 10:11p Lawrance
 * stub out hud_communications_state
 * 
 * 192   4/07/98 2:52p Andsager
 * stub out Energy_levels
 * 
 * 191   4/05/98 10:33a John
 * Stubbed in a variable
 * 
 * 190   3/31/98 5:11p John
 * Removed demo/save/restore.  Made NDEBUG defined compile.  Removed a
 * bunch of debug stuff out of player file.  Made model code be able to
 * unload models and malloc out only however many models are needed.
 * 
 * 189   3/30/98 10:26a Hoffoss
 * 
 * 188   3/26/98 6:41p Lawrance
 * stub out bmap paging function
 * 
 * 187   3/19/98 10:26a Dave
 * Put in several HUD_offset problems.
 * 
 * 186   3/17/98 10:27a Johnson
 * AL: stub out some asteroid network calls
 * 
 * 185   3/14/98 4:57p Lawrance
 * stub out some wingman status functions
 * 
 * 184   3/12/98 5:36p John
 * Took out any unused shaders.  Made shader code take rgbc instead of
 * matrix and vector since noone used it like a matrix and it would have
 * been impossible to do in hardware.   Made Glide implement a basic
 * shader for online help.  
 * 
 * 183   3/12/98 2:21p Johnson
 * Fixed some Fred bugs related to jump nodes.
 * 
 * 182   3/11/98 10:22p Dave
 * Laid groundwork for new observer HUD. Split up multi respawning into
 * its own module.
 * 
 * 181   3/11/98 9:36p Allender
 * 
 * 180   3/11/98 12:25a Lawrance
 * stub yet another HUD function.
 * 
 * 179   3/10/98 5:08p Allender
 * fixed up multiplayer death messages (I hope).  changes in object update
 * packets
 * 
 * 178   3/09/98 9:54p Hoffoss
 * 
 * 177   3/09/98 5:03p Lawrance
 * stub hud function
 * 
 * 176   3/09/98 4:30p Allender
 * multiplayer secondary weapon changes.  red-alert and cargo-known-delay
 * sexpressions.  Add time cargo revealed to ship structure
 * 
 * 175   3/09/98 10:50a Hoffoss
 * 
 * 174   3/09/98 12:12a Lawrance
 * Add support for Red Alert missions
 * 
 * 173   3/08/98 12:03p Allender
 * changed how ship network signatures are handed out.  Done at mission
 * load time.  Space reserved in wings for all waves/counts for their
 * signatures.  Fixed some secondary firing issues
 * 
 * 172   3/06/98 5:10p Allender
 * made time to: field in extended targetbox use support time to dock code
 * for all docking shpis.  Only display for waypoints and docking (not
 * undocking).  Small fixups to message menu -- not allowing depart when
 * disabled.  Depart is now by default ignored for all non-small ships
 * 
 * 171   3/05/98 11:15p Hoffoss
 * Changed non-game key checking to use game_check_key() instead of
 * game_poll().
 * 
 * 170   3/04/98 5:04p Hoffoss
 * stub out hud function
 * 
 * 169   3/03/98 1:21a Lawrance
 * stub out some hud escort functions
 * 
 * 168   2/27/98 10:34a Johnson
 * 
 * 167   2/24/98 3:08p Allender
 * 
 * 166   2/23/98 5:07p Allender
 * made net_signature in the object structure an unsigned short.  Created
 * permanent and non-permanent network object "pools".
 * 
 * 165   2/19/98 7:06p Sandeep
 * 
 * 164   2/18/98 10:34p Allender
 * repair/rearm system (for single and multi) about finished.
 * dock/undock and ai goals packets implemented for multiplayer
 * 
 * 163   2/17/98 5:03p Allender
 * major cdhanges to rearm repair code.  All flag and variable setting
 * done in one function.  A little more work to do.  Fix bug in squad
 * messaging when hotkey was used on invalid target
 * 
 * 162   2/17/98 8:58a Mike
 * Resolve link errors with stubs in FredStubs.
 * 
 * 161   2/12/98 5:12p Lawrance
 * stub out hud function
 * 
 * 160   2/11/98 9:44p Allender
 * rearm repair code fixes.  hud support view shows abort status.  New
 * support ship killed message.  More network stats
 * 
 * 159   2/11/98 5:47p Dave
 * multiplayer packet function stub
 * 
 * 158   2/10/98 9:55a Lawrance
 * stub out cmeasure function
 * 
 * 157   1/29/98 5:22p Dave
 * Made ingame join ignore bad packets more gracefully.
 * 
 * 156   1/29/98 9:00a Allender
 * yet more stubs
 * 
 * 155   1/28/98 7:31p Lawrance
 * stub out some more hud stuff
 * 
 * 154   1/24/98 4:00p Lawrance
 * stub out some new hud functions
 * 
 * 153   1/22/98 11:46p Hoffoss
 * Fixed linking problem with Fred.
 * 
 * 152   1/20/98 4:45p Allender
 * more,  uh..., umm...., more stubs -- yeah, that's it
 * 
 * 151   1/20/98 11:43a Sandeep
 * fixed unresolved external
 * 
 * 150   1/17/98 12:35a Sandeep
 * fixed fred stub build error
 * 
 * 149   1/16/98 10:40a Lawrance
 * stub out hud function
 * 
 * 148   1/14/98 5:22p Allender
 * save/restore hotkey selections when replaying the same mission
 * 
 * 147   1/13/98 5:37p Dave
 * Reworked a lot of standalone interface code. Put in single and
 * multiplayer popups for death sequence. Solidified multiplayer kick
 * code.
 * 
 * 146   1/12/98 9:29p Mike
 * Stub out send_mission_goal_info_packet().
 * 
 * 145   1/10/98 1:22a Lawrance
 * fix link error in FRED
 * 
 * 144   1/07/98 4:40p Allender
 * minor modification to special messages.  Fixed cargo_revealed problem
 * for multiplayer and problem with is-cargo-known sexpression
 * 
 * 143   1/05/98 10:06p Lawrance
 * stub out some HUD functions
 * 
 * 142   1/02/98 10:20p Lawrance
 * stub out hud_set_default_color()
 * 
 * 141   1/02/98 9:12p Lawrance
 * remove some obsolete stubs
 * 
 * 140   12/24/97 9:57p Lawrance
 * remove stub
 * 
 * 139   12/24/97 3:37p Hoffoss
 * Moved control config stuff to seperate library to Fred can access it as
 * well.
 * 
 * 138   12/19/97 2:01p Johnson
 * added stubs for game_flash
 * 
 * 137   12/19/97 11:56a John
 * Added texturing to missilie trails.  Took out Alan's old sphere
 * debugging code.  Added palette flash effect code.
 * 
 * 136   12/19/97 11:21a Hoffoss
 * 
 * 135   12/18/97 8:46p Lawrance
 * Move IFF_color definitions from HUD->ship, so FRED can use them.
 * 
 * 134   12/16/97 9:32p Lawrance
 * stub out demo_query_debug()
 * 
 * 133   12/16/97 6:20p Hoffoss
 * Added more debugging code for demos, and fixed a bug in demo
 * recording/playback.
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "object/object.h"
#include "io/key.h"
#include "ship/ship.h"
#include "graphics/2d.h"
#include "mission/missionparse.h"
#include "network/psnet.h"
#include "stats/scoring.h"
#include "cfile/cfile.h"

float flFrametime;
int	game_zbuffer = 1;
int	Current_mission = 0xdeadbeef;
char **Builtin_mission_names;
char *Game_current_mission_filename;
CFILE *Working_demo;
struct beam_info;
bool cube_map_drawen=false;
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
float Viewer_zoom;

int Warpout_forced = 0;
float Warpout_time;
vec3d Camera_pos;
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
void game_load_palette(){}
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
int game_do_cd_mission_check(char *){return 1;}
int Player_multi_died_check;
int Show_framerate = 0;

void game_feature_not_in_demo_popup() {}
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
void game_render_frame_setup(vec3d *eye_pos, matrix *eye_orient){}
void game_render_frame(vec3d *eye_pos, matrix *eye_orient){}
void game_simulation_frame(){}
void game_update_missiontime(){}
void game_render_post_frame(){}
