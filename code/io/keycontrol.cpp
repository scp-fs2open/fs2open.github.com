/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "globalincs/linklist.h"
#include "io/key.h"
#include "io/joy.h"
#include "io/timer.h"
#include "ship/ship.h"
#include "playerman/player.h"
#include "weapon/weapon.h"
#include "hud/hud.h"
#include "gamesequence/gamesequence.h"
#include "mission/missiongoals.h"
#include "hud/hudets.h"
#include "gamesnd/gamesnd.h"
#include "hud/hudsquadmsg.h"
#include "gamesnd/eventmusic.h"
#include "freespace2/freespace.h"
#include "mission/missionhotkey.h"
#include "hud/hudescort.h"
#include "hud/hudshield.h"
#include "io/keycontrol.h"
#include "ship/shiphit.h"
#include "ship/shipfx.h"
#include "mission/missionlog.h"
#include "hud/hudtargetbox.h"
#include "popup/popup.h"
#include "object/objcollide.h"
#include "object/object.h"
#include "hud/hudconfig.h"
#include "hud/hudmessage.h"
#include "network/multi_pmsg.h"
#include "starfield/supernova.h"
#include "mission/missionmessage.h"
#include "menuui/mainhallmenu.h"
#include "missionui/missionpause.h"
#include "hud/hudgauges.h"
#include "freespace2/freespace.h"	//For time compression stuff
#include "species_defs/species_defs.h"
#include "asteroid/asteroid.h"
#include "iff_defs/iff_defs.h"
#include "network/multi.h"
#include "network/multiutil.h"
#include "network/multimsgs.h"
#include "network/multi_pause.h"
#include "network/multi_observer.h"
#include "network/multi_endgame.h"
#include "autopilot/autopilot.h"
#include "cmdline/cmdline.h"

// --------------------------------------------------------------
// Global to file 
// --------------------------------------------------------------

// time compression/dilation values - Goober5000
// (Volition sez "can't compress below 0.25"... not sure if
// this is arbitrary or dictated by code)
#define MAX_TIME_MULTIPLIER		64
#define MAX_TIME_DIVIDER		4

#define CHEAT_BUFFER_LEN	17
char CheatBuffer[CHEAT_BUFFER_LEN+1];

enum cheatCode {
	CHEAT_CODE_NONE = 0,
	CHEAT_CODE_FREESPACE,
	CHEAT_CODE_FISH,
	CHEAT_CODE_HEADZ,
	CHEAT_CODE_TOOLED,
	CHEAT_CODE_PIRATE,
	CHEAT_CODE_SKIP
};

struct Cheat {
	cheatCode code;
	char* data;
};

static struct Cheat cheatsTable[] = {
  { CHEAT_CODE_FREESPACE, "www.freespace2.com" },
  { CHEAT_CODE_FISH,      "vasudanswuvfishes" },
  { CHEAT_CODE_HEADZ,     "humanheadsinside." },
  { CHEAT_CODE_TOOLED,    "tooledworkedowned" },
  { CHEAT_CODE_PIRATE,    "arrrrwalktheplank" },
  { CHEAT_CODE_SKIP,      "skipmemymissionyo" }
};

#define CHEATS_TABLE_LEN	6

int Tool_enabled = 0;
bool Perspective_locked=false;
bool quit_mission_popup_shown = false;

extern int AI_watch_object;
extern int Countermeasures_enabled;

extern float do_subobj_hit_stuff(object *ship_obj, object *other_obj, vec3d *hitpos, float damage, bool *hull_should_apply_armor);

extern void mission_goal_mark_all_true( int type );

int Normal_key_set[] = {
	TARGET_NEXT,
	TARGET_PREV,
	TARGET_NEXT_CLOSEST_HOSTILE,
	TARGET_PREV_CLOSEST_HOSTILE,
	TARGET_NEXT_CLOSEST_FRIENDLY,
	TARGET_PREV_CLOSEST_FRIENDLY,
	TARGET_TARGETS_TARGET,
	TARGET_SHIP_IN_RETICLE,
	TARGET_LAST_TRANMISSION_SENDER,
	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,
	TARGET_CLOSEST_SHIP_ATTACKING_SELF,
	STOP_TARGETING_SHIP,
	TOGGLE_AUTO_TARGETING,
	TARGET_SUBOBJECT_IN_RETICLE,
	TARGET_PREV_SUBOBJECT,
	TARGET_NEXT_SUBOBJECT,
	STOP_TARGETING_SUBSYSTEM,

	TARGET_NEXT_UNINSPECTED_CARGO,
	TARGET_PREV_UNINSPECTED_CARGO,
	TARGET_NEWEST_SHIP,
	TARGET_NEXT_LIVE_TURRET,
	TARGET_PREV_LIVE_TURRET,
	TARGET_NEXT_BOMB,
	TARGET_PREV_BOMB,

	ATTACK_MESSAGE,
	DISARM_MESSAGE,
	DISABLE_MESSAGE,
	ATTACK_SUBSYSTEM_MESSAGE,
	CAPTURE_MESSAGE,
	ENGAGE_MESSAGE,
	FORM_MESSAGE,
	PROTECT_MESSAGE,
	COVER_MESSAGE,
	WARP_MESSAGE,
	REARM_MESSAGE,
	IGNORE_MESSAGE,
	SQUADMSG_MENU,

	VIEW_CHASE,
	VIEW_OTHER_SHIP,
	VIEW_TOPDOWN,
	VIEW_TRACK_TARGET,

	SHOW_GOALS,
	END_MISSION,

	ADD_REMOVE_ESCORT,
	ESCORT_CLEAR,
	TARGET_NEXT_ESCORT_SHIP,

	XFER_SHIELD,
	XFER_LASER,
	INCREASE_SHIELD,
	INCREASE_WEAPON,
	INCREASE_ENGINE,
	DECREASE_SHIELD,
	DECREASE_WEAPON,
	DECREASE_ENGINE,
	ETS_EQUALIZE,
	SHIELD_EQUALIZE,
	SHIELD_XFER_TOP,
	SHIELD_XFER_BOTTOM,
	SHIELD_XFER_RIGHT,
	SHIELD_XFER_LEFT,

	CYCLE_NEXT_PRIMARY,
	CYCLE_PREV_PRIMARY,
	CYCLE_SECONDARY,
	CYCLE_NUM_MISSLES,
	RADAR_RANGE_CYCLE,

	MATCH_TARGET_SPEED,
	TOGGLE_AUTO_MATCH_TARGET_SPEED,

	VIEW_EXTERNAL,
	VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK,
	LAUNCH_COUNTERMEASURE,
	ONE_THIRD_THROTTLE,
	TWO_THIRDS_THROTTLE,
	PLUS_5_PERCENT_THROTTLE,
	MINUS_5_PERCENT_THROTTLE,
	ZERO_THROTTLE,
	MAX_THROTTLE,

	TARGET_CLOSEST_REPAIR_SHIP,

	MULTI_MESSAGE_ALL,
	MULTI_MESSAGE_FRIENDLY,
	MULTI_MESSAGE_HOSTILE,
	MULTI_MESSAGE_TARGET,
	MULTI_OBSERVER_ZOOM_TO,

	TIME_SPEED_UP,
	TIME_SLOW_DOWN,

	TOGGLE_HUD_CONTRAST,

	MULTI_TOGGLE_NETINFO,
	MULTI_SELF_DESTRUCT,

	TOGGLE_HUD,

	HUD_TARGETBOX_TOGGLE_WIREFRAME,
	AUTO_PILOT_TOGGLE,
	NAV_CYCLE,

	TOGGLE_GLIDING
};

int Dead_key_set[] = {
	TARGET_NEXT,
	TARGET_PREV,
	TARGET_NEXT_CLOSEST_HOSTILE,
	TARGET_PREV_CLOSEST_HOSTILE,
	TARGET_NEXT_CLOSEST_FRIENDLY,
	TARGET_PREV_CLOSEST_FRIENDLY,
	TARGET_TARGETS_TARGET,
	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,
	STOP_TARGETING_SHIP,
	TOGGLE_AUTO_TARGETING,
	TARGET_SUBOBJECT_IN_RETICLE,
	TARGET_PREV_SUBOBJECT,
	TARGET_NEXT_SUBOBJECT,
	STOP_TARGETING_SUBSYSTEM,
	TARGET_NEWEST_SHIP,
	TARGET_NEXT_LIVE_TURRET,
	TARGET_PREV_LIVE_TURRET,
	TARGET_NEXT_BOMB,
	TARGET_PREV_BOMB,

	VIEW_CHASE,
	VIEW_OTHER_SHIP,
	VIEW_TOPDOWN,

	SHOW_GOALS,

	ADD_REMOVE_ESCORT,
	ESCORT_CLEAR,
	TARGET_NEXT_ESCORT_SHIP,
	TARGET_CLOSEST_REPAIR_SHIP,	

	MULTI_MESSAGE_ALL,
	MULTI_MESSAGE_FRIENDLY,
	MULTI_MESSAGE_HOSTILE,
	MULTI_MESSAGE_TARGET,
	MULTI_OBSERVER_ZOOM_TO,

	TIME_SPEED_UP,
	TIME_SLOW_DOWN
};

int Critical_key_set[] = {	
	CYCLE_NEXT_PRIMARY,		
	CYCLE_PREV_PRIMARY,				
	CYCLE_SECONDARY,			
	CYCLE_NUM_MISSLES,			
	INCREASE_WEAPON,			
	DECREASE_WEAPON,	
	INCREASE_SHIELD,			
	DECREASE_SHIELD,			
	INCREASE_ENGINE,			
	DECREASE_ENGINE,			
	ETS_EQUALIZE,
	SHIELD_EQUALIZE,			
	SHIELD_XFER_TOP,			
	SHIELD_XFER_BOTTOM,			
	SHIELD_XFER_LEFT,			
	SHIELD_XFER_RIGHT,			
	XFER_SHIELD,			
	XFER_LASER,			
};

int Non_critical_key_set[] = {
	MATCH_TARGET_SPEED,			
	TOGGLE_AUTO_MATCH_TARGET_SPEED,			
	TARGET_NEXT,	
	TARGET_PREV,			
	TARGET_NEXT_CLOSEST_HOSTILE,			
	TARGET_PREV_CLOSEST_HOSTILE,			
	TOGGLE_AUTO_TARGETING,			
	TARGET_NEXT_CLOSEST_FRIENDLY,			
	TARGET_PREV_CLOSEST_FRIENDLY,			
	TARGET_SHIP_IN_RETICLE,			
	TARGET_LAST_TRANMISSION_SENDER,
	TARGET_CLOSEST_REPAIR_SHIP,			
	TARGET_CLOSEST_SHIP_ATTACKING_TARGET,			
	STOP_TARGETING_SHIP,			
	TARGET_CLOSEST_SHIP_ATTACKING_SELF,			
	TARGET_TARGETS_TARGET,			
	TARGET_SUBOBJECT_IN_RETICLE,			
	TARGET_PREV_SUBOBJECT,			
	TARGET_NEXT_SUBOBJECT,			
	STOP_TARGETING_SUBSYSTEM,
	TARGET_NEXT_BOMB,
	TARGET_PREV_BOMB,
	TARGET_NEXT_UNINSPECTED_CARGO,
	TARGET_PREV_UNINSPECTED_CARGO,
	TARGET_NEWEST_SHIP,
	TARGET_NEXT_LIVE_TURRET,
	TARGET_PREV_LIVE_TURRET,
	ATTACK_MESSAGE,
	DISARM_MESSAGE,
	DISABLE_MESSAGE,
	ATTACK_SUBSYSTEM_MESSAGE,
	CAPTURE_MESSAGE,
	ENGAGE_MESSAGE,
	FORM_MESSAGE,
	PROTECT_MESSAGE,
	COVER_MESSAGE,
	WARP_MESSAGE,
	IGNORE_MESSAGE,
	REARM_MESSAGE,
	VIEW_CHASE,
	VIEW_EXTERNAL,
	VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK,
	VIEW_OTHER_SHIP,
	VIEW_TOPDOWN,
	VIEW_TRACK_TARGET,
	RADAR_RANGE_CYCLE,
	SQUADMSG_MENU,
	SHOW_GOALS,
	END_MISSION,
	ADD_REMOVE_ESCORT,
	ESCORT_CLEAR,
	TARGET_NEXT_ESCORT_SHIP,
	MULTI_MESSAGE_ALL,
	MULTI_MESSAGE_FRIENDLY,
	MULTI_MESSAGE_HOSTILE,
	MULTI_MESSAGE_TARGET,
	MULTI_OBSERVER_ZOOM_TO,
	TOGGLE_HUD_CONTRAST,

	MULTI_TOGGLE_NETINFO,
	MULTI_SELF_DESTRUCT,

	TOGGLE_HUD,

	HUD_TARGETBOX_TOGGLE_WIREFRAME,
	AUTO_PILOT_TOGGLE,
	NAV_CYCLE,
	TOGGLE_GLIDING
};

int Ignored_keys[CCFG_MAX];

// set sizes of the key sets automatically
int Normal_key_set_size = sizeof(Normal_key_set) / sizeof(int);
int Dead_key_set_size = sizeof(Dead_key_set) / sizeof(int);
int Critical_key_set_size = sizeof(Critical_key_set) / sizeof(int);
int Non_critical_key_set_size = sizeof(Non_critical_key_set) / sizeof(int);

// --------------------------------------------------------------
// routine to process keys used only for debugging
// --------------------------------------------------------------

void debug_cycle_player_ship(int delta)
{
	if ( Player_obj == NULL )
		return;

	int si_index = Ships[Player_obj->instance].ship_info_index;
	int sanity = 0;
	ship_info	*sip;
	while ( TRUE ) {
		si_index += delta;
		if ( si_index > Num_ship_classes ){
			si_index = 0;
		}
		if ( si_index < 0 ){
			si_index = Num_ship_classes - 1;
		}
		sip = &Ship_info[si_index];
		if ( sip->flags & SIF_PLAYER_SHIP ){
			break;
		}

		// just in case
		sanity++;
		if ( sanity > Num_ship_classes ){
			break;
		}
	}

	change_ship_type(Player_obj->instance, si_index);
	HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Player ship changed to %s", 0), Ship_info[si_index].name);			
}

/**
 * Cycle targeted ship to next ship in that species
 * @param delta Increment
 */
void debug_cycle_targeted_ship(int delta)
{
	object		*objp;
	ship_info	*sip;
	int			si_index, species;
	char			name[NAME_LENGTH];

	if ( Player_ai->target_objnum == -1 )
		return;

	objp = &Objects[Player_ai->target_objnum];
	if ( objp->type != OBJ_SHIP )
		return;

	si_index = Ships[objp->instance].ship_info_index;
	Assert(si_index != -1 );
	species = Ship_info[si_index].species;

	int sanity = 0;

	while ( TRUE ) {
		si_index += delta;
		if ( si_index > Num_ship_classes )
			si_index = 0;
		if ( si_index < 0 )
			si_index = Num_ship_classes-1;

	
		sip = &Ship_info[si_index];
	
		// if it has test in the name, jump over it
		strcpy_s(name, sip->name);
		_strlwr(name);
		if ( strstr(name,NOX("test")) != NULL )
			continue;

		if ( sip->species == species && (sip->flags & (SIF_FIGHTER | SIF_BOMBER | SIF_TRANSPORT) ) )
			break;

		// just in case
		sanity++;
		if ( sanity > Num_ship_classes )
			break;
	}

	change_ship_type(objp->instance, si_index);
	HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Changed player target to %s", 1), Ship_info[si_index].name);			
}

void debug_max_secondary_weapons(object *objp)
{
	int index;
	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];
	ship_weapon *swp = &shipp->weapons;

	for ( index = 0; index < MAX_SHIP_SECONDARY_BANKS; index++ )
	{
		swp->secondary_bank_ammo[index] = sip->secondary_bank_ammo_capacity[index];
	}
}

void debug_max_primary_weapons(object *objp)	// Goober5000
{
	Assert(objp);	// Goober5000

	int index;
	ship *shipp = &Ships[objp->instance];
	ship_info *sip = &Ship_info[shipp->ship_info_index];
	ship_weapon *swp = &shipp->weapons;
	weapon_info *wip;

	if (sip->flags & SIF_BALLISTIC_PRIMARIES)
	{
		for ( index = 0; index < MAX_SHIP_PRIMARY_BANKS; index++ )
		{
			wip = &Weapon_info[swp->primary_bank_weapons[index]];
			if (wip->wi_flags2 & WIF2_BALLISTIC)
			{
				float capacity, size;
				capacity = (float) sip->primary_bank_ammo_capacity[index];
				size = (float) wip->cargo_size;
				swp->primary_bank_ammo[index] = fl2i((capacity / size)+0.5f);
			}
		}
	}
}

void debug_change_song(int delta)
{
	char buf[256];
	if ( event_music_next_soundtrack(delta) != -1 ) {
		event_music_get_soundtrack_name(buf);
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Soundtrack changed to: %s", 2), buf);

	} else {
		HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Event music is not playing", 3));
	}
}

extern void hud_target_asteroid();
extern int Framerate_delay;

extern void snd_stop_any_sound();

extern vec3d Eye_position;
extern matrix Eye_matrix;
extern void g3_set_view_matrix(vec3d *view_pos,matrix *view_matrix,float zoom);

extern int Show_cpu;

int get_prev_weapon_looped(int current_weapon, int subtype)
{
	int i, new_index;

	for (i = 1; i < Num_weapon_types; i++)
	{
		new_index = (Num_weapon_types + current_weapon - i) % Num_weapon_types;

		if(Weapon_info[new_index].subtype == subtype)
		{
			return new_index;
		}
	}

	return current_weapon;
}

int get_next_weapon_looped(int current_weapon, int subtype)
{
	int i, new_index;

	for (i = 1; i < Num_weapon_types; i++)
	{
		new_index = (current_weapon + i) % Num_weapon_types;

		if(Weapon_info[new_index].subtype == subtype)
		{
			return new_index;
		}
	}

	return current_weapon;
}

void process_debug_keys(int k)
{
	// Kazan -- NO CHEATS IN MULTI
	if (Game_mode & GM_MULTIPLAYER)
	{
		Cheats_enabled = 0;
		return;
	}

	switch (k) {
		case KEY_DEBUGGED + KEY_H:
			hud_target_toggle_hidden_from_sensors();
			break;

		case KEY_DEBUGGED + KEY_F: 
			extern int wacky_scheme;
			if(wacky_scheme == 3){
				wacky_scheme = 0;
			} else {
				wacky_scheme++;
			}
			break;
		
		case KEY_DEBUGGED + KEY_ALTED + KEY_F:
			Framerate_delay += 10;
			HUD_printf(XSTR( "Framerate delay increased to %i milliseconds per frame.", 4), Framerate_delay);
			break;

		case KEY_DEBUGGED + KEY_ALTED + KEY_SHIFTED + KEY_F:
			Framerate_delay -= 10;
			if (Framerate_delay < 0)
				Framerate_delay = 0;

			HUD_printf(XSTR( "Framerate delay decreased to %i milliseconds per frame.", 5), Framerate_delay);
			break;

		case KEY_DEBUGGED + KEY_X:
		case KEY_DEBUGGED1 + KEY_X:
			HUD_printf("Cloaking has been disabled, thank you for playing fs2_open, %s", Player->callsign);
			break;

		case KEY_DEBUGGED + KEY_C:
		case KEY_DEBUGGED1 + KEY_C:
			if(Player_obj->flags & OF_COLLIDES){
				obj_set_flags(Player_obj, Player_obj->flags & ~(OF_COLLIDES));
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, "Player no longer collides");
			} else {
				obj_set_flags(Player_obj, Player_obj->flags | OF_COLLIDES);
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, "Player collides");
			}
			break;

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_C:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_C:
			Countermeasures_enabled = !Countermeasures_enabled;
			HUD_printf(XSTR( "Countermeasure firing: %s", 6), Countermeasures_enabled ? XSTR( "ENABLED", 7) : XSTR( "DISABLED", 8));
			break;

// Goober5000: this should only be compiled in debug builds, since it crashes release
#ifndef NDEBUG
		case KEY_DEBUGGED + KEY_E:
			gameseq_post_event(GS_EVENT_EVENT_DEBUG);
			break;
#endif

		// Goober5000: handle time dilation in cheat section
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_COMMA:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_COMMA:
			if ( Game_mode & GM_NORMAL ) {
				if ( Game_time_compression > (F1_0/MAX_TIME_DIVIDER) ) {
					change_time_compression(0.5);
				} else {
					gamesnd_play_error_beep();
				}
			} else {
				gamesnd_play_error_beep();
			}
			break;

		// Goober5000: handle as normal here
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_PERIOD:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_PERIOD:
			if ( Game_mode & GM_NORMAL ) {
				if ( Game_time_compression < (F1_0*MAX_TIME_MULTIPLIER) ) {
					change_time_compression(2);
				} else {
					gamesnd_play_error_beep();
				}
			} else {
				gamesnd_play_error_beep();
			}
			break;

		//	Kill! the currently targeted ship.
		case KEY_DEBUGGED + KEY_K:
		case KEY_DEBUGGED1 + KEY_K:
			if (Player_ai->target_objnum != -1) {
				object	*objp = &Objects[Player_ai->target_objnum];

				switch (objp->type) {
				case OBJ_SHIP:
					
					// remove guardian flag -- kazan
					Ships[objp->instance].ship_guardian_threshold = 0;
					
					ship_apply_local_damage( objp, Player_obj, &objp->pos, 100000.0f, MISS_SHIELDS, CREATE_SPARKS);
					ship_apply_local_damage( objp, Player_obj, &objp->pos, 1.0f, MISS_SHIELDS, CREATE_SPARKS);
					break;
				case OBJ_WEAPON:
					Weapons[objp->instance].lifeleft = 0.01f;
					break;
				}
			}

			break;
		
		// play the next mission message
		case KEY_DEBUGGED + KEY_V:		
			extern int Message_debug_index;
			extern int Num_messages_playing;
			// stop any other messages
			if(Num_messages_playing){
				message_kill_all(1);
			}

			// next message
			if(Message_debug_index >= Num_messages - 1){
				Message_debug_index = Num_builtin_messages;
			} else {
				Message_debug_index++;
			}
			
			// play the message
			message_send_unique_to_player( Messages[Message_debug_index].name, Message_waves[Messages[Message_debug_index].wave_info.index].name, MESSAGE_SOURCE_SPECIAL, MESSAGE_PRIORITY_HIGH, 0, 0 );			
			if (Messages[Message_debug_index].avi_info.index == -1) {
				HUD_printf("No anim set for message \"%s\"; None will play!", Messages[Message_debug_index].name);
			}
			break;

		// play the previous mission message
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_V:
			extern int Message_debug_index;
			extern int Num_messages_playing;
			// stop any other messages
			if(Num_messages_playing){
				message_kill_all(1);
			}

			// go maybe go down one
			if(Message_debug_index == Num_builtin_messages - 1){
				Message_debug_index = Num_builtin_messages;
			} else if(Message_debug_index > Num_builtin_messages){
				Message_debug_index--;
			}
			
			// play the message
			message_send_unique_to_player( Messages[Message_debug_index].name, Message_waves[Messages[Message_debug_index].wave_info.index].name, MESSAGE_SOURCE_SPECIAL, MESSAGE_PRIORITY_HIGH, 0, 0 );
			if (Messages[Message_debug_index].avi_info.index == -1) {
				HUD_printf("No avi associated with this message; None will play!");
			}
			break;

		// reset to the beginning of mission messages
		case KEY_DEBUGGED + KEY_ALTED + KEY_V:
			extern int Message_debug_index;
			Message_debug_index = Num_builtin_messages - 1;
			HUD_printf("Resetting to first mission message");
			break;

		//	Kill! the currently targeted ship.
		case KEY_DEBUGGED + KEY_ALTED + KEY_SHIFTED + KEY_K:
		case KEY_DEBUGGED1 + KEY_ALTED + KEY_SHIFTED + KEY_K:
			if (Player_ai->target_objnum != -1) {
				object	*objp = &Objects[Player_ai->target_objnum];

				if (objp->type == OBJ_SHIP) {
					ship_apply_local_damage( objp, Player_obj, &objp->pos, Ships[objp->instance].ship_max_hull_strength * 0.1f + 10.0f, MISS_SHIELDS, CREATE_SPARKS);
				}
			}
			break;

			//	Kill the currently targeted subsystem.
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_K:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_K:
			if ((Player_ai->target_objnum != -1) && (Player_ai->targeted_subsys != NULL)) {
				object	*objp = &Objects[Player_ai->target_objnum];
				if ( objp->type == OBJ_SHIP ) {
					ship		*sp = &Ships[objp->instance];
					vec3d	g_subobj_pos;

					get_subsystem_world_pos(objp, Player_ai->targeted_subsys, &g_subobj_pos);

					do_subobj_hit_stuff(objp, Player_obj, &g_subobj_pos, (float) -Player_ai->targeted_subsys->system_info->type, NULL); //100.0f);

					if ( sp->subsys_info[SUBSYSTEM_ENGINE].aggregate_current_hits <= 0.0f ) {
						mission_log_add_entry(LOG_SHIP_DISABLED, sp->ship_name, NULL );
						sp->flags |= SF_DISABLED;				// add the disabled flag
					}

					if ( sp->subsys_info[SUBSYSTEM_TURRET].aggregate_current_hits <= 0.0f ) {
						mission_log_add_entry(LOG_SHIP_DISARMED, sp->ship_name, NULL );
					}
				}
			}
			break;

		case KEY_DEBUGGED + KEY_ALTED + KEY_K:
		case KEY_DEBUGGED1 + KEY_ALTED + KEY_K:
			{
				float	shield, integrity;
				vec3d	pos, randvec;

				vm_vec_rand_vec_quick(&randvec);
				vm_vec_scale_add(&pos, &Player_obj->pos, &randvec, Player_obj->radius);
			ship_apply_local_damage(Player_obj, Player_obj, &pos, 25.0f, MISS_SHIELDS, CREATE_SPARKS);
			hud_get_target_strength(Player_obj, &shield, &integrity);
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "You knocked yourself down to %7.3f percent hull.\n", 9), 100.0f * integrity);
			break;
			}
			
		//	Whack down the player's shield and hull by a little more than 50%
		//	Select next object to be viewed by AI.
		case KEY_DEBUGGED + KEY_I:
		case KEY_DEBUGGED1 + KEY_I:
			Player_obj->flags ^= OF_INVULNERABLE;
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "You are %s", 10), Player_obj->flags & OF_INVULNERABLE ? XSTR( "now INVULNERABLE!", 11) : XSTR( "no longer invulnerable...", 12));
			break;

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_I:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_I:
			if (Player_ai->target_objnum != -1) {
				object	*objp = &Objects[Player_ai->target_objnum];

				objp->flags ^= OF_INVULNERABLE;
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Player's target [%s] is %s", 13), Ships[objp->instance].ship_name, objp->flags & OF_INVULNERABLE ? XSTR( "now INVULNERABLE!", 11) : XSTR( "no longer invulnerable...", 12));
			}
			break;

		case KEY_DEBUGGED + KEY_N:
			AI_watch_object++;
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Spewing debug info about object #%d", 14), AI_watch_object);
			break;

		case KEY_DEBUGGED + KEY_O:
		case KEY_DEBUGGED1 + KEY_O:
			toggle_player_object();
			break;				

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_O:
			extern int Debug_octant;
			if(Debug_octant == 7){
				Debug_octant = -1;
			} else {
				Debug_octant++;
			}
			nprintf(("General", "Debug_octant == %d\n", Debug_octant));
			break;

		case KEY_DEBUGGED + KEY_P:
			supernova_start(20);
			break;

		case KEY_DEBUGGED + KEY_W:
		case KEY_DEBUGGED1 + KEY_W:
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_W:
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_W:
			// temp code for testing purposes, toggles weapon energy cheat
			Weapon_energy_cheat = !Weapon_energy_cheat;
			if (Weapon_energy_cheat) {
				if (k & KEY_SHIFTED)
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Weapon energy and missile count will always be at full ALL SHIPS!", 15));
				else
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Weapon energy and missile count will always be at full for player", 16));

				debug_max_secondary_weapons(Player_obj);
				debug_max_primary_weapons(Player_obj);
				if (k & KEY_SHIFTED) {
					object	*objp;

					for ( objp = GET_FIRST(&obj_used_list); objp !=END_OF_LIST(&obj_used_list); objp = GET_NEXT(objp) )
						if (objp->type == OBJ_SHIP)
							debug_max_secondary_weapons(objp);
							debug_max_primary_weapons(objp);
				}

			} else
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Normal weapon energy system / missile count restored", 17));

			break;

		case KEY_DEBUGGED + KEY_G:
		case KEY_DEBUGGED1 + KEY_G:
			mission_goal_mark_all_true( PRIMARY_GOAL );
			break;

		case KEY_DEBUGGED + KEY_G + KEY_SHIFTED:
		case KEY_DEBUGGED1 + KEY_G + KEY_SHIFTED:
			mission_goal_mark_all_true( SECONDARY_GOAL );
			break;

		case KEY_DEBUGGED + KEY_G + KEY_ALTED:
		case KEY_DEBUGGED1 + KEY_G + KEY_ALTED:
			mission_goal_mark_all_true( BONUS_GOAL );
			break;

		case KEY_DEBUGGED + KEY_9: {
		case KEY_DEBUGGED1 + KEY_9:
			ship* shipp;
			
			shipp = &Ships[Player_obj->instance];
			int *weap = &shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank];
			*weap = get_next_weapon_looped(*weap, WP_MISSILE);

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Secondary Weapon forced to %s", 18), Weapon_info[*weap].name);
			break;
		}

			
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_9: {
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_9:
			ship* shipp;

			shipp = &Ships[Player_obj->instance];
			int *weap = &shipp->weapons.secondary_bank_weapons[shipp->weapons.current_secondary_bank];
			*weap = get_prev_weapon_looped(*weap, WP_MISSILE);

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Secondary Weapon forced to %s", 18), Weapon_info[*weap].name);
			break;
		}
		
		case KEY_DEBUGGED + KEY_U: {
		case KEY_DEBUGGED1 + KEY_U:
			// launch asteroid
			object *asteroid_create(asteroid_field *asfieldp, int asteroid_type, int subtype);
			object *objp = asteroid_create(&Asteroid_field, 0, 0);
			vec3d vel;
			vm_vec_copy_scale(&vel, &Player_obj->orient.vec.fvec, 50.0f);
			objp->phys_info.vel = vel;
			objp->phys_info.desired_vel = vel;
			objp->pos = Player_obj->pos;
			break;
		}

		case KEY_DEBUGGED + KEY_0: {
		case KEY_DEBUGGED1 + KEY_0:
			ship* shipp;

			shipp = &Ships[Player_obj->instance];
			int *weap = &shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank];
			*weap = get_next_weapon_looped(*weap, WP_LASER);
			
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Primary Weapon forced to %s", 19), Weapon_info[*weap].name);
			break;
		}

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_0: {
		case KEY_DEBUGGED1 + KEY_SHIFTED + KEY_0:
			ship* shipp;

			shipp = &Ships[Player_obj->instance];
			int *weap = &shipp->weapons.primary_bank_weapons[shipp->weapons.current_primary_bank];
			*weap = get_prev_weapon_looped(*weap, WP_LASER);
		
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Primary Weapon forced to %s", 19), Weapon_info[*weap].name);
			break;
		}

		case KEY_DEBUGGED + KEY_J: {
			int new_pattern = event_music_return_current_pattern();

			new_pattern++;
			if ( new_pattern >= MAX_PATTERNS )
				new_pattern = 0;

			event_music_change_pattern(new_pattern);
			break;
		}

		case KEY_DEBUGGED + KEY_M: {
			if ( Event_music_enabled ) {
				event_music_disable();
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Event music disabled", 20));

			} else {
				event_music_enable();
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Event music enabled", 21));
			}

			break;
		}

		case KEY_DEBUGGED + KEY_R: {
		case KEY_DEBUGGED1 + KEY_R:
			if (Player_ai->target_objnum != -1)
				ai_issue_rearm_request(&Objects[Player_ai->target_objnum]);
			else
				ai_issue_rearm_request(Player_obj);

			break;
		}

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_UP:
			Game_detail_level++;
			HUD_printf( XSTR( "Detail level set to %+d\n", 22), Game_detail_level );
			break;

		case KEY_DEBUGGED + KEY_SHIFTED + KEY_DOWN:
			Game_detail_level--;
			HUD_printf( XSTR( "Detail level set to %+d\n", 22), Game_detail_level );
			break;

#ifndef NDEBUG
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_T:	{
			extern int Test_begin;

			if ( Test_begin == 1 )
				break;

			Test_begin = 1;
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Frame Rate test started", 23));

			break;
		}
#endif
		case KEY_DEBUGGED + KEY_A:	{

			HUD_printf("frame rate currently is %0.2 FPS", 1/flFrametime);

			break;
		}


		case KEY_DEBUGGED + KEY_D:
			extern int OO_update_index;			

			if(MULTIPLAYER_MASTER){
				do {
					OO_update_index++;
				} while((OO_update_index < (MAX_PLAYERS-1)) && !MULTI_CONNECTED(Net_players[OO_update_index]));
				if(OO_update_index >= MAX_PLAYERS-1){
					OO_update_index = -1;
				}			
			} else {
				if(OO_update_index < 0){
					OO_update_index = MY_NET_PLAYER_NUM;
				} else {
					OO_update_index = -1;
				}
			}
			break;

		// change player ship to next flyable type
		case KEY_DEBUGGED + KEY_RIGHT:
			debug_cycle_player_ship(1);
			break;

		// change player ship to previous flyable ship
		case KEY_DEBUGGED + KEY_LEFT:
			debug_cycle_player_ship(-1);
			break;
		
		// cycle target to ship
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_RIGHT:
			debug_cycle_targeted_ship(1);
			break;

		// cycle target to previous ship
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_LEFT:
			debug_cycle_targeted_ship(-1);
			break;

		// change species of the targeted ship
		case KEY_DEBUGGED + KEY_S: {
			if ( Player_ai->target_objnum < 0 )
				break;

			object		*objp;
			ship_info	*sip;

			objp = &Objects[Player_ai->target_objnum];
			if ( objp->type != OBJ_SHIP )
				return;

			sip = &Ship_info[Ships[objp->instance].ship_info_index];
			sip->species++;

			if (sip->species >= (int)Species_info.size())
				sip->species = 0;

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Species of target changed to: %s", 24), Species_info[sip->species].species_name);
			break;
		}
			
		case KEY_DEBUGGED + KEY_SHIFTED + KEY_S:
			game_increase_skill_level();
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Skill level set to %s.", 25), Skill_level_names(Game_skill_level));
			break;
					
		case KEY_DEBUGGED + KEY_T: {
			char buf[256];
			event_music_get_info(buf);
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, buf);
			break;
		}

		case KEY_DEBUGGED + KEY_UP:
		case KEY_DEBUGGED1 + KEY_UP:
			debug_change_song(1);
			break;

		case KEY_DEBUGGED + KEY_DOWN:
		case KEY_DEBUGGED1 + KEY_DOWN:
			debug_change_song(-1);
			break;

		case KEY_PADMINUS: {
			int init_flag = 0;

			if ( keyd_pressed[KEY_1] )	{
				init_flag = 1;
				HUD_color_red -= 4;
			} 

			if ( keyd_pressed[KEY_2] )	{
				init_flag = 1;
				HUD_color_green -= 4;
			} 

			if ( keyd_pressed[KEY_3] )	{
				init_flag = 1;
				HUD_color_blue -= 4;
			} 

			if (init_flag)
				HUD_init_colors();

			break;
		}
		
		case KEY_DEBUGGED + KEY_Y:
			extern int tst;
			tst = 2;
			break;

		case KEY_PADPLUS: {
			int init_flag = 0;

			if ( keyd_pressed[KEY_1] )	{
				init_flag = 1;
				HUD_color_red += 4;
			} 

			if ( keyd_pressed[KEY_2] )	{
				init_flag = 1;
				HUD_color_green += 4;
			} 

			if ( keyd_pressed[KEY_3] )	{
				init_flag = 1;
				HUD_color_blue += 4;
			} 

			if (init_flag)
				HUD_init_colors();

			break;
		}
		case KEY_DEBUGGED + KEY_ALTED + KEY_EQUAL:
		case KEY_DEBUGGED1 + KEY_ALTED + KEY_EQUAL:
			{
			camera *cam = Main_camera.getCamera();
			if(cam == NULL)
			{
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, "Couldn't get camera FOV");
				break;
			}
			cam->set_fov(cam->get_fov() + 0.1f);
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, "Camera fov raised to %0.2f" , cam->get_fov());
			}
			break;

		case KEY_DEBUGGED + KEY_ALTED + KEY_MINUS:
		case KEY_DEBUGGED1 + KEY_ALTED + KEY_MINUS:
			{
			camera *cam = Main_camera.getCamera();
			if(cam == NULL)
			{
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, "Couldn't get camera FOV");
				break;
			}
			cam->set_fov(cam->get_fov() - 0.1f);
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, "Camera fov lowered to %0.2f" , cam->get_fov());
			}
			break;
		case KEY_DEBUGGED + KEY_Z:
		case KEY_DEBUGGED1 + KEY_Z:
			{
				Show_cpu = !Show_cpu;
			}
			break;

	}	// end switch
}

void ppsk_hotkeys(int k)
{
	// use k to check for keys that can have Shift,Ctrl,Alt,Del status
	int hotkey_set;

#ifndef NDEBUG
	k &= ~KEY_DEBUGGED;			// since hitting F11 will set this bit
#endif

	switch (k) {
		case KEY_F5:
		case KEY_F6:
		case KEY_F7:
		case KEY_F8:
		case KEY_F9:
		case KEY_F10:
		case KEY_F11:
		case KEY_F12:
			hotkey_set = mission_hotkey_get_set_num(k);
			if ( !(Players[Player_num].flags & PLAYER_FLAGS_MSG_MODE) )
				hud_target_hotkey_select( hotkey_set );
			else
				hud_squadmsg_hotkey_select( hotkey_set );

			break;

		case KEY_F5 + KEY_SHIFTED:
		case KEY_F6 + KEY_SHIFTED:
		case KEY_F7 + KEY_SHIFTED:
		case KEY_F8 + KEY_SHIFTED:
		case KEY_F9 + KEY_SHIFTED:
		case KEY_F10 + KEY_SHIFTED:
		case KEY_F11 + KEY_SHIFTED:
		case KEY_F12 + KEY_SHIFTED:
			hotkey_set = mission_hotkey_get_set_num(k&(~KEY_SHIFTED));
			mprintf(("Adding to set %d\n", hotkey_set+1));
			if ( Player_ai->target_objnum == -1)
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "No target to add/remove from set %d.", 26), hotkey_set+1);
			else  {
				hud_target_hotkey_add_remove( hotkey_set, &Objects[Player_ai->target_objnum], HOTKEY_USER_ADDED);
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "%s added to set %d. (F%d)", 27), Ships[Objects[Player_ai->target_objnum].instance].ship_name, hotkey_set, 4+hotkey_set+1);
			}

			break;

		case KEY_F5 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F6 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F7 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F8 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F9 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F10 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F11 + KEY_SHIFTED + KEY_ALTED:
		case KEY_F12 + KEY_SHIFTED + KEY_ALTED:
			hotkey_set = mission_hotkey_get_set_num(k & ~KEY_SHIFTED+KEY_ALTED);
			hud_target_hotkey_clear( hotkey_set );
			break;

		case KEY_SHIFTED + KEY_MINUS:
			if ( HUD_color_alpha > HUD_COLOR_ALPHA_USER_MIN )	{
				HUD_color_alpha--;
				HUD_init_colors();
			}
			break;

		case KEY_SHIFTED + KEY_EQUAL:
			if ( HUD_color_alpha < HUD_COLOR_ALPHA_USER_MAX ) {
				HUD_color_alpha++;
				HUD_init_colors();
			}
			break;
	}	// end switch
}

/**
 * Check keypress 'key' against a set of valid controls and mark the match in the
 * player's button info bitfield.  Also checks joystick controls in the set.
 *
 * @param key Scancode (plus modifiers).
 * @param count Total size of the list
 * @param list List of ::Control_config struct action indices to check for
 */
void process_set_of_keys(int key, int count, int *list)
{
	int i;

	for (i=0; i<count; i++)
		if (check_control(list[i], key))
			button_info_set(&Player->bi, list[i]);
}

/**
 * Routine to process keys used for player ship stuff (*not* ship movement).
 */
void process_player_ship_keys(int k)
{
	int masked_k;

	masked_k = k & ~KEY_CTRLED;	// take out CTRL modifier only	

	// moved this line to beginning of function since hotkeys now encompass
	// F5 - F12.  We can return after using F11 as a hotkey.
	ppsk_hotkeys(masked_k);
	if (keyd_pressed[KEY_DEBUG_KEY]){
		return;
	}

	// if we're in supernova mode. do nothing
	if(Player->control_mode == PCM_SUPERNOVA){
		return;
	}

	// pass the key to the squadmate messaging code.  If the messaging code took the key, then return
	// from here immediately since we don't want to do further key processing.
	if ( hud_squadmsg_read_key(k) )
		return;

	if ( Player->control_mode == PCM_NORMAL )	{
		//	The following things are not legal to do while dead.
		if ( !(Game_mode & GM_DEAD) ) {
			process_set_of_keys(masked_k, Normal_key_set_size, Normal_key_set);
		} else	{
			process_set_of_keys(masked_k, Dead_key_set_size, Dead_key_set);
		}
		if (lua_game_control & LGC_B_POLL_ALL) {
			// first clear all
			button_info_clear(&Player->lua_bi_full);

			// then check the keys.
			int i;
			for(i = 0; i < CCFG_MAX; i++) {
				if (check_control(i, masked_k))
					button_info_set(&Player->lua_bi_full, i);
			}
		}
	} else {

	}
}

/**
 * Handler for when player hits 'ESC' during the game
 */
void game_do_end_mission_popup()
{
	int	pf_flags, choice;

	// do the multiplayer version of this
	if(Game_mode & GM_MULTIPLAYER){
		multi_quit_game(PROMPT_ALL);
	} else {
		// single player version....
		// do housekeeping things.
		game_stop_time();
		game_stop_looped_sounds();
		snd_stop_all();

		quit_mission_popup_shown = true;

		pf_flags = PF_BODY_BIG | PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON;
		choice = popup(pf_flags, 3, POPUP_NO, XSTR( "&Yes, Quit", 28), XSTR( "Yes, &Restart", 29), XSTR( "Do you really want to end the mission?", 30));

		switch (choice) {
		case 1:
			gameseq_post_event(GS_EVENT_END_GAME);
			break;

		case 2:
			gameseq_post_event(GS_EVENT_ENTER_GAME);
			break;

		default:
			break;  // do nothing
		}
		quit_mission_popup_shown = false;

		game_start_time();
		game_flush();
	}
}

/**
 * Handle pause keypress
 */
void game_process_pause_key()
{
	// special processing for multiplayer
	if (Game_mode & GM_MULTIPLAYER) {							
		if(Multi_pause_status){
			multi_pause_request(0);
		} else {
			multi_pause_request(1);
		}		
	} else {
		gameseq_post_event( GS_EVENT_PAUSE_GAME );
	}
}

/**
 * Process cheat codes
 */
void game_process_cheats(int k)
{
	size_t i;

	if ( k == 0 ){
		return;
	}

	// no cheats in multiplayer, ever
	if(Game_mode & GM_MULTIPLAYER){
		Cheats_enabled = 0;
		return;
	}

	for (i = 0; i < CHEAT_BUFFER_LEN; i++){
		CheatBuffer[i]=CheatBuffer[i+1];
	}

	CheatBuffer[CHEAT_BUFFER_LEN - 1] = (char)key_to_ascii(k);
	
	cheatCode detectedCheatCode = CHEAT_CODE_NONE;

	for(i=0; i < CHEATS_TABLE_LEN; i++) {
		Cheat cheat = cheatsTable[i];

		if(!strncmp(cheat.data, CheatBuffer, CHEAT_BUFFER_LEN)){
			detectedCheatCode = cheat.code;
			break;
		}
	}

	if(detectedCheatCode == CHEAT_CODE_FREESPACE){
		Cheats_enabled = 1;

		// cheating allows the changing of weapons so we have to grab anything
		// that we don't already have loaded, just in case
		extern void weapons_page_in_cheats();
		weapons_page_in_cheats();

		HUD_printf("Cheats enabled");
	}
	if(detectedCheatCode == CHEAT_CODE_FISH){
		// only enable in the Vasudan main hall
		if ((gameseq_get_state() == GS_STATE_MAIN_MENU) && main_hall_is_vasudan()) {
			extern void fishtank_start();
			fishtank_start();
		}
	}
	if(detectedCheatCode == CHEAT_CODE_HEADZ){
		// only enable in the Vasudan main hall
		if ((gameseq_get_state() == GS_STATE_MAIN_MENU) && main_hall_is_vasudan()) {
			main_hall_vasudan_funny();
		}
	}
	if(detectedCheatCode ==  CHEAT_CODE_SKIP && (gameseq_get_state() == GS_STATE_MAIN_MENU)){
		extern void main_hall_campaign_cheat();
		main_hall_campaign_cheat();
	}
	if(detectedCheatCode == CHEAT_CODE_TOOLED && (Game_mode & GM_IN_MISSION)){
		Tool_enabled = 1;
		HUD_printf("Prepare to be taken to school");
	}
	if(detectedCheatCode == CHEAT_CODE_PIRATE && (Game_mode & GM_IN_MISSION) && (Player_obj != NULL)){
		extern void prevent_spawning_collision(object *new_obj);
		ship_subsys *ptr;
		char name[NAME_LENGTH];
		int ship_idx, ship_class; 

		// if not found, then don't create it :(
		ship_class = ship_info_lookup("Volition Bravos");
		if (ship_class < 0)
			return;

		HUD_printf(NOX("Walk the plank"));

		vec3d pos = Player_obj->pos;
		matrix orient = Player_obj->orient;
		pos.xyz.x += frand_range(-700.0f, 700.0f);
		pos.xyz.y += frand_range(-700.0f, 700.0f);
		pos.xyz.z += frand_range(-700.0f, 700.0f);

		int objnum = ship_create(&orient, &pos, ship_class);
		if (objnum < 0)
			return;

		ship *shipp = &Ships[Objects[objnum].instance];
		shipp->ship_name[0] = '\0';
		shipp->orders_accepted = (1<<NUM_COMM_ORDER_ITEMS)-1;

		// Goober5000 - stolen from support ship creation
		// create a name for the ship.  use "Volition Bravos #".  look for collisions until one isn't found anymore
		ship_idx = 1;
		do {
			sprintf(name, NOX("Volition Bravos %d"), ship_idx);
			if ( (ship_name_lookup(name) == -1) && (ship_find_exited_ship_by_name(name) == -1) )
			{
				strcpy_s(shipp->ship_name, name);
				break;
			}

			ship_idx++;
		} while(1);

		shipp->flags |= SF_ESCORT;
		shipp->escort_priority = 1000 - ship_idx;

		// now make sure we're not colliding with anyone
		prevent_spawning_collision(&Objects[objnum]);
			
		// Goober5000 - beam free
		for (ptr = GET_FIRST(&shipp->subsys_list); ptr != END_OF_LIST(&shipp->subsys_list); ptr = GET_NEXT(ptr))
		{
			// mark all turrets as beam free
			if (ptr->system_info->type == SUBSYSTEM_TURRET)
			{
				ptr->weapons.flags |= SW_FLAG_BEAM_FREE;
				ptr->turret_next_fire_stamp = timestamp((int) frand_range(50.0f, 4000.0f));
			}
		}
				
		// warpin
		shipfx_warpin_start(&Objects[objnum]);
	}
}

void game_process_keys()
{
	int k;

	button_info_clear(&Player->bi);	// clear out the button info struct for the player
    do
	{		
		k = game_poll();

		if ( Game_mode & GM_DEAD_BLEW_UP ) {
			continue;
		}

		game_process_cheats( k );
		process_player_ship_keys(k);
		process_debug_keys(k);
		
		switch (k) {
			case 0:
				// No key
				break;
			
			case KEY_ESC:
				if ( Player->control_mode != PCM_NORMAL )	{
					if ( Player->control_mode == PCM_WARPOUT_STAGE1 )	{
						gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
					} else {
						// too late to abort warp out!
					}
				} else {
					// let the ESC key break out of messaging mode
					if ( Players[Player_num].flags & PLAYER_FLAGS_MSG_MODE ) {
						hud_squadmsg_toggle();
						break;
					}

					//If topdown view in non-2D mission, go back to cockpit view.
					if ( (Viewer_mode & VM_TOPDOWN) && !(The_mission.flags & MISSION_FLAG_2D_MISSION) && !(Perspective_locked) ) {
						Viewer_mode &= ~VM_TOPDOWN;
						break;
					}

					// if in external view or chase view, go back to cockpit view
					if ( (Viewer_mode & (VM_EXTERNAL|VM_CHASE|VM_OTHER_SHIP)) && !(Perspective_locked) ) {
						Viewer_mode &= ~(VM_EXTERNAL|VM_CHASE|VM_OTHER_SHIP);
						break;
					}

					if (!(Game_mode & GM_DEAD_DIED))
						game_do_end_mission_popup();

				}
				break;

			case KEY_Y:								
				break;

			case KEY_N:
				break;			

			case KEY_ALTED + KEY_SHIFTED+KEY_J:
				// treat the current joystick position as the center position
				joy_set_cen();
				break;

			case KEY_DEBUGGED | KEY_PAUSE:
				gameseq_post_event( GS_EVENT_DEBUG_PAUSE_GAME );
				break;

			case KEY_ALTED + KEY_PAUSE:
				if( Game_mode & GM_DEAD_BLEW_UP || 
					Game_mode & GM_DEAD_DIED) {
					break;
				}

				pause_set_type(PAUSE_TYPE_VIEWER);
				game_process_pause_key();
				break;
			case KEY_PAUSE:
				if( Game_mode & GM_DEAD_BLEW_UP || 
					Game_mode & GM_DEAD_DIED) {
					break;
				}

				pause_set_type(PAUSE_TYPE_NORMAL);
				game_process_pause_key();
				break;

		} // end switch
	}
	while (k);

	// lua button command override goes here!!
	if (lua_game_control & LGC_B_OVERRIDE) {
		button_info temp = Player->bi;
		Player->bi = Player->lua_bi;
		Player->lua_bi = temp;
	} else if (lua_game_control & LGC_B_ADDITIVE) {
		// add the lua commands to current commands 
		int i;
		for (i=0; i<NUM_BUTTON_FIELDS; i++)
			Player->bi.status[i] |= Player->lua_bi.status[i];
		Player->lua_bi = Player->bi;		
	} else {
		// just copy over the values
		Player->lua_bi = Player->bi;
	}
	// there.. wasnt that bad hack was it?

	button_info_do(&Player->bi);	// call functions based on status of button_info bit vectors
}

int button_function_critical(int n, net_player *p = NULL)
{
	object *objp;
	player *pl;
	net_player *npl;
	int at_self;    // flag indicating the object is local (for hud messages, etc)

	Assert(n >= 0);
   
	// multiplayer clients should leave critical button bits alone and pass them to the server instead
	if (MULTIPLAYER_CLIENT) {
		// if this flag is set, we should apply the button itself (came from the server)
		if (!Multi_button_info_ok){
			return 0;
		}
	}

	// in single player mode make sure we're using the player object and the player himself, otherwise use the object and
	// player pertaining to the passed net_player
	npl = NULL;
	if (p == NULL) {
		objp = Player_obj;
		pl = Player;

		if(Game_mode & GM_MULTIPLAYER){
			npl = Net_player;

			// if we're the server in multiplayer and we're an observer, don't process our own critical button functions
			if((Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){
				return 0;
			}
		}

		at_self = 1;
	} else {
		objp = &Objects[p->m_player->objnum];
		pl = p->m_player;
		npl = p;
		at_self = 0;

		if ( NETPLAYER_IS_DEAD(npl) || (Ships[Objects[pl->objnum].instance].flags & SF_DYING) )
			return 0;
	}
	
	switch (n) {
		// cycle to next primary weapon
		case CYCLE_NEXT_PRIMARY:			
			if (at_self) {
				control_used(CYCLE_NEXT_PRIMARY);
			}

			hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
			if (ship_select_next_primary(objp, CYCLE_PRIMARY_NEXT)) {
				ship* shipp = &Ships[objp->instance];
				if ( timestamp_elapsed(shipp->weapons.next_primary_fire_stamp[shipp->weapons.current_primary_bank]) ) {
					shipp->weapons.next_primary_fire_stamp[shipp->weapons.current_primary_bank] = timestamp(250);	//	1/4 second delay until can fire
				}

				// multiplayer server should maintain bank/link status here
				if ( MULTIPLAYER_MASTER ) {
					Assert(npl != NULL);
					multi_server_update_player_weapons(npl,shipp);										
				}
			}			
			break;

		// cycle to previous primary weapon
		case CYCLE_PREV_PRIMARY:
			if (at_self) {
				control_used(CYCLE_PREV_PRIMARY);
			}

			hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
			if (ship_select_next_primary(objp, CYCLE_PRIMARY_PREV)) {
				ship* shipp = &Ships[objp->instance];
				if ( timestamp_elapsed(shipp->weapons.next_primary_fire_stamp[shipp->weapons.current_primary_bank]) ) {
					shipp->weapons.next_primary_fire_stamp[shipp->weapons.current_primary_bank] = timestamp(250);	//	1/4 second delay until can fire
				}

				// multiplayer server should maintain bank/link status here
				if ( MULTIPLAYER_MASTER ) {
					Assert(npl != NULL);
					multi_server_update_player_weapons(npl,shipp);										
				}
			}			
			break;

		// cycle to next secondary weapon
		case CYCLE_SECONDARY:
			if(at_self)
				control_used(CYCLE_SECONDARY);
			
			hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
			if (ship_select_next_secondary(objp)) {
				ship* shipp = &Ships[objp->instance];
				if ( timestamp_elapsed(shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank]) ) {
					shipp->weapons.next_secondary_fire_stamp[shipp->weapons.current_secondary_bank] = timestamp(250);	//	1/4 second delay until can fire
				}

				// multiplayer server should maintain bank/link status here
				if( MULTIPLAYER_MASTER ){
					Assert(npl != NULL);
					multi_server_update_player_weapons(npl,shipp);										
				}					
			}			
			break;

		// cycle number of missiles
		case CYCLE_NUM_MISSLES:
			if(at_self)
				control_used(CYCLE_NUM_MISSLES);

			if ( objp == Player_obj ) {
				if ( Player_ship->weapons.num_secondary_banks <= 0 ) {
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "This ship has no secondary weapons", 33));
					gamesnd_play_iface(SND_GENERAL_FAIL);
					break;
				}
			}
					
			if ( Ships[objp->instance].flags & SF_SECONDARY_DUAL_FIRE ) {		
				Ships[objp->instance].flags &= ~SF_SECONDARY_DUAL_FIRE;
				if(at_self) {
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Secondary weapon set to normal fire mode", 34));
					snd_play( &Snds[ship_get_sound(Player_obj, SND_SECONDARY_CYCLE)] );
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
				}
			} else {
				Ships[objp->instance].flags |= SF_SECONDARY_DUAL_FIRE;
				if(at_self) {
					HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Secondary weapon set to dual fire mode", 35));
					snd_play( &Snds[ship_get_sound(Player_obj, SND_SECONDARY_CYCLE)] );
					hud_gauge_popup_start(HUD_WEAPONS_GAUGE);
				}
			}

			// multiplayer server should maintain bank/link status here
			if( MULTIPLAYER_MASTER ){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}
			break;

		// increase weapon recharge rate
		case INCREASE_WEAPON:
			if(at_self)
				control_used(INCREASE_WEAPON);
			increase_recharge_rate(objp, WEAPONS);

			// multiplayer server should maintain bank/link status here
			if( MULTIPLAYER_MASTER ){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}
			break;

		// decrease weapon recharge rate
		case DECREASE_WEAPON:
			if(at_self)
				control_used(DECREASE_WEAPON);
			decrease_recharge_rate(objp, WEAPONS);

			// multiplayer server should maintain bank/link status here
			if( MULTIPLAYER_MASTER ){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}
			break;

		// increase shield recharge rate
		case INCREASE_SHIELD:
			if(at_self)
				control_used(INCREASE_SHIELD);
			increase_recharge_rate(objp, SHIELDS);

			// multiplayer server should maintain bank/link status here
			if( MULTIPLAYER_MASTER ){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}
			break;

		// decrease shield recharge rate
		case DECREASE_SHIELD:
			if(at_self)
				control_used(DECREASE_SHIELD);
			decrease_recharge_rate(objp, SHIELDS);

			// multiplayer server should maintain bank/link status here
			if( MULTIPLAYER_MASTER ){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}
			break;

		// increase energy to engines
		case INCREASE_ENGINE:
			if(at_self)
				control_used(INCREASE_ENGINE);
			increase_recharge_rate(objp, ENGINES);

			// multiplayer server should maintain bank/link status here
			if( MULTIPLAYER_MASTER ){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}
			break;

		// decrease energy to engines
		case DECREASE_ENGINE:
			if(at_self)
   			control_used(DECREASE_ENGINE);
			decrease_recharge_rate(objp, ENGINES);

			// multiplayer server should maintain bank/link status here
			if( MULTIPLAYER_MASTER ){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}
			break;

		// equalize recharge rates
		case ETS_EQUALIZE:
			if (at_self) {
   			control_used(ETS_EQUALIZE);
			}

			set_default_recharge_rates(objp);
			snd_play( &Snds[SND_ENERGY_TRANS] );

			// multiplayer server should maintain bank/link status here
			if( MULTIPLAYER_MASTER ){
				Assert(npl != NULL);
				multi_server_update_player_weapons(npl,&Ships[objp->instance]);										
			}
			break;

		// equalize shield energy to all quadrants
		case SHIELD_EQUALIZE:
			if(at_self){
				control_used(SHIELD_EQUALIZE);
			}
			hud_shield_equalize(objp, pl);
			break;

		// transfer shield energy to front
		case SHIELD_XFER_TOP:
			if(at_self){
   			control_used(SHIELD_XFER_TOP);
			}
			hud_augment_shield_quadrant(objp, 1);
			break;

		// transfer shield energy to rear
		case SHIELD_XFER_BOTTOM:
			if(at_self)
				control_used(SHIELD_XFER_BOTTOM);
			hud_augment_shield_quadrant(objp, 2);
			break;

		// transfer shield energy to left
		case SHIELD_XFER_LEFT:
			if(at_self)
				control_used(SHIELD_XFER_LEFT);
			hud_augment_shield_quadrant(objp, 3);
			break;
			
		// transfer shield energy to right
		case SHIELD_XFER_RIGHT:
			if(at_self)
				control_used(SHIELD_XFER_RIGHT);
			hud_augment_shield_quadrant(objp, 0);
			break;

		// transfer energy to shield from weapons
		case XFER_SHIELD:
			if(at_self)
				control_used(XFER_SHIELD);
			transfer_energy_to_shields(objp);
			break;

		// transfer energy to weapons from shield
		case XFER_LASER:
			if(at_self)
				control_used(XFER_LASER);
			transfer_energy_to_weapons(objp);
			break;

		// following are not handled here, but we need to bypass the Int3()
		case LAUNCH_COUNTERMEASURE:
		case VIEW_SLEW:
		case VIEW_EXTERNAL:
		case VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK:
		case VIEW_TRACK_TARGET:
		case ONE_THIRD_THROTTLE:
		case TWO_THIRDS_THROTTLE:
		case MINUS_5_PERCENT_THROTTLE:
		case PLUS_5_PERCENT_THROTTLE:
		case ZERO_THROTTLE:
		case MAX_THROTTLE:
		case TOGGLE_GLIDING:
			return 0;

		default :
			Int3(); // bad bad bad
			break;
	}

	return 1;
}

/**
 * Execute function corresponding to action n
 * Basically, these are actions which don't affect demo playback at all
 * @param n Action number
 */
int button_function_demo_valid(int n)
{
	// by default, we'll return "not processed". ret will get set to 1, if this is one of the keys which is always allowed, even in demo
	// playback.
	int ret = 0;

	//	No keys, not even targeting keys, when player in death roll.  He can press keys after he blows up.
	if (Game_mode & GM_DEAD_DIED){
		return 0;
	}

	// any of these buttons are valid
	switch(n){
	case VIEW_CHASE:
		control_used(VIEW_CHASE);
		if(!Perspective_locked)
		{
			Viewer_mode ^= VM_CHASE;
		}
		else
		{
			snd_play( &Snds[SND_TARGET_FAIL] );
		}
		ret = 1;
		break;

	case VIEW_TRACK_TARGET: // Target padlock mode toggle (Swifty)
		control_used(VIEW_TRACK_TARGET);
		if (!Perspective_locked) {
			if(Viewer_mode & VM_TRACK) {
				chase_slew_angles.h = 0;
				chase_slew_angles.p = 0;
			}
			Viewer_mode ^= VM_TRACK;
		} else {
			snd_play( &Snds[SND_TARGET_FAIL] );
		}
		ret = 1;
		break;

	case VIEW_EXTERNAL:
		control_used(VIEW_EXTERNAL);
		if(!Perspective_locked)
		{
			Viewer_mode ^= VM_EXTERNAL;
			Viewer_mode &= ~VM_EXTERNAL_CAMERA_LOCKED;	// reset camera lock when leaving/entering external view
		}
		else
		{
			snd_play( &Snds[SND_TARGET_FAIL] );
		}
		ret = 1;
		break;

	case VIEW_TOPDOWN:
		control_used(VIEW_TOPDOWN);
		if(!Perspective_locked)
		{
			Viewer_mode ^= VM_TOPDOWN;
		}
		else
		{
			snd_play( &Snds[SND_TARGET_FAIL] );
		}
		ret = 1;
		break;

	case VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK:
		control_used(VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK);
		if ( Viewer_mode & VM_EXTERNAL ) {
		Viewer_mode ^= VM_EXTERNAL_CAMERA_LOCKED;
		if ( Viewer_mode & VM_EXTERNAL_CAMERA_LOCKED ) {
			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "External camera is locked, controls will move ship", 36));
			} else {
				HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "External camera is free, controls will move the camera, not the ship", 37));
			}
		}
		ret = 1;
		break;

	case VIEW_OTHER_SHIP:
		control_used(VIEW_OTHER_SHIP);
		if ( Player_ai->target_objnum < 0 || Perspective_locked) {
			snd_play( &Snds[SND_TARGET_FAIL] );
		} else {
			if ( Objects[Player_ai->target_objnum].type != OBJ_SHIP )  {
				snd_play( &Snds[SND_TARGET_FAIL] );
			} else {
				Viewer_mode ^= VM_OTHER_SHIP;
			}
		}
		ret = 1;
		break;

	case TIME_SLOW_DOWN:
		if ( Game_mode & GM_NORMAL ) {
			// Goober5000 - time dilation only available in cheat mode (see above);
			// now you can do it with or without pressing the tilde, per Kazan's request
			if ( ((Game_time_compression > F1_0) || (Cheats_enabled && (Game_time_compression > (F1_0/MAX_TIME_DIVIDER)))) && !Time_compression_locked) {
				change_time_compression(0.5f);
			} else {
				gamesnd_play_error_beep();
			}
		} else {
			gamesnd_play_error_beep();
		}
		ret = 1;
		break;

	case TIME_SPEED_UP:
		if ( Game_mode & GM_NORMAL ) {
			if ( (Game_time_compression < (F1_0*MAX_TIME_MULTIPLIER)) && !Time_compression_locked ) {
				change_time_compression(2.0f);
			} else {
				gamesnd_play_error_beep();
			}
		} else {
			gamesnd_play_error_beep();
		}
		ret = 1;
		break;
	}

	// done
	return ret;
}

/**
 * Execute function corresponding to action n (BUTTON_ from KeyControl.h)
 * @return 1 when action was taken
 */
int button_function(int n)
{
	Assert(n >= 0);

	// check if the button has been set to be ignored by a SEXP
	if (Ignored_keys[n]) {
		if (Ignored_keys[n] > 0) {
			Ignored_keys[n]--;
		}
		return 0;
	}

	//	No keys, not even targeting keys, when player in death roll.  He can press keys after he blows up.
	if (Game_mode & GM_DEAD_DIED){
		return 0;
	}

	// Goober5000 - if the ship doesn't have subspace drive, jump key doesn't work: so test and exit early
	if (Player_ship->flags2 & SF2_NO_SUBSPACE_DRIVE)
	{
		switch(n)
		{
			case END_MISSION:
				control_used(n);	// set the timestamp for when we used the control, in case we need it
				return 1;			// pretend we took the action: if we return 0, strange stuff may happen
		}
	}

	// Goober5000 - if we have primitive sensors, some keys don't work: so test and exit early
	if (Player_ship->flags2 & SF2_PRIMITIVE_SENSORS)
	{
		switch (n)
		{
			case MATCH_TARGET_SPEED:
			case TOGGLE_AUTO_MATCH_TARGET_SPEED:
			case TOGGLE_AUTO_TARGETING:
			case TARGET_NEXT:
			case TARGET_PREV:
			case TARGET_NEXT_CLOSEST_HOSTILE:
			case TARGET_PREV_CLOSEST_HOSTILE:
			case TARGET_NEXT_CLOSEST_FRIENDLY:
			case TARGET_PREV_CLOSEST_FRIENDLY:
			case TARGET_SHIP_IN_RETICLE:
			case TARGET_LAST_TRANMISSION_SENDER:
			case TARGET_CLOSEST_REPAIR_SHIP:
			case TARGET_CLOSEST_SHIP_ATTACKING_TARGET:
			case STOP_TARGETING_SHIP:
			case TARGET_CLOSEST_SHIP_ATTACKING_SELF:
			case TARGET_TARGETS_TARGET:
			case TARGET_SUBOBJECT_IN_RETICLE:
			case TARGET_NEXT_SUBOBJECT:
			case TARGET_PREV_SUBOBJECT:
			case STOP_TARGETING_SUBSYSTEM:
			case TARGET_NEXT_BOMB:
			case TARGET_PREV_BOMB:
			case TARGET_NEXT_UNINSPECTED_CARGO:
			case TARGET_PREV_UNINSPECTED_CARGO:
			case TARGET_NEWEST_SHIP:
			case TARGET_NEXT_LIVE_TURRET:
			case TARGET_PREV_LIVE_TURRET:
			case TARGET_NEXT_ESCORT_SHIP:
				control_used(n);	// set the timestamp for when we used the control, in case we need it
				return 1;			// pretend we took the action: if we return 0, strange stuff may happen
		}
	}

	// now handle keys the regular way
	switch (n) {
		// cycle to next primary weapon
		case CYCLE_NEXT_PRIMARY:			
			return button_function_critical(CYCLE_NEXT_PRIMARY);
			break;

		// cycle to previous primary weapon
		case CYCLE_PREV_PRIMARY:			
			return button_function_critical(CYCLE_PREV_PRIMARY);
			break;

		// cycle to next secondary weapon
		case CYCLE_SECONDARY:
			return button_function_critical(CYCLE_SECONDARY);
			break;

		// cycle number of missiles fired from secondary bank
		case CYCLE_NUM_MISSLES:
         return button_function_critical(CYCLE_NUM_MISSLES);			
			break;

		// undefined in multiplayer for clients right now
		// match target speed
		case MATCH_TARGET_SPEED:
			control_used(MATCH_TARGET_SPEED);
			// If player is auto-matching, break auto-match speed
			if ( Player->flags & PLAYER_FLAGS_AUTO_MATCH_SPEED ) {
				Player->flags &= ~PLAYER_FLAGS_AUTO_MATCH_SPEED;
			}
			player_match_target_speed();						
			break;

		// undefined in multiplayer for clients right now
		// toggle auto-match target speed
		case TOGGLE_AUTO_MATCH_TARGET_SPEED:
			// multiplayer observers can't match target speed
			if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && ((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER)) ){
				break;
			}

			Player->flags ^= PLAYER_FLAGS_AUTO_MATCH_SPEED;					
			control_used(TOGGLE_AUTO_MATCH_TARGET_SPEED);
			hud_gauge_popup_start(HUD_AUTO_SPEED);
			if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_MATCH_SPEED ) {
				snd_play(&Snds[SND_SHIELD_XFER_OK], 1.0f);
				if ( !(Player->flags & PLAYER_FLAGS_MATCH_TARGET) ) {
					player_match_target_speed();
				}
			}
			else
			{
				snd_play(&Snds[SND_SHIELD_XFER_OK], 1.0f);
				player_match_target_speed();
			}			
			break;

		// target next
		case TARGET_NEXT:
			control_used(TARGET_NEXT);			
			if ( hud_sensors_ok(Player_ship) ) {
				hud_target_next();
			}
			break;

		// target previous
		case TARGET_PREV:
			control_used(TARGET_PREV);			
			if ( hud_sensors_ok(Player_ship) ) {
				hud_target_prev();			
			}
			break;

		// target the next hostile target
		case TARGET_NEXT_CLOSEST_HOSTILE:
			control_used(TARGET_NEXT_CLOSEST_HOSTILE);			
			if (hud_sensors_ok(Player_ship)){
				hud_target_next_list();
			}			
			break;

		// target the previous closest hostile 
		case TARGET_PREV_CLOSEST_HOSTILE:
			control_used(TARGET_PREV_CLOSEST_HOSTILE);			
			if (hud_sensors_ok(Player_ship)){
				hud_target_next_list(1,0);
			}
			break;

		// toggle auto-targeting
		case TOGGLE_AUTO_TARGETING:
			control_used(TOGGLE_AUTO_TARGETING);
			hud_gauge_popup_start(HUD_AUTO_TARGET);
			Players[Player_num].flags ^= PLAYER_FLAGS_AUTO_TARGETING;
			if ( Players[Player_num].flags & PLAYER_FLAGS_AUTO_TARGETING ) {
				if (hud_sensors_ok(Player_ship)) {
					hud_target_closest(iff_get_attackee_mask(Player_ship->team), -1, FALSE, TRUE );
					snd_play(&Snds[SND_SHIELD_XFER_OK], 1.0f);
				} else {
					Players[Player_num].flags ^= PLAYER_FLAGS_AUTO_TARGETING;
				}
			} else {
				snd_play(&Snds[SND_SHIELD_XFER_OK], 1.0f);
			}
			break;

		// target the next friendly ship
		case TARGET_NEXT_CLOSEST_FRIENDLY:
			control_used(TARGET_NEXT_CLOSEST_FRIENDLY);			
			if (hud_sensors_ok(Player_ship)){
				hud_target_next_list(0);
			}
			break;

		// target the closest friendly ship
		case TARGET_PREV_CLOSEST_FRIENDLY:
			control_used(TARGET_PREV_CLOSEST_FRIENDLY);			
			if (hud_sensors_ok(Player_ship)) {
				hud_target_next_list(0,0);
			}
			break;

		// target ship closest to center of reticle
		case TARGET_SHIP_IN_RETICLE:
			control_used(TARGET_SHIP_IN_RETICLE);			
			if (hud_sensors_ok(Player_ship)){
				hud_target_in_reticle_new();
			}
			break;

		case TARGET_LAST_TRANMISSION_SENDER:
			control_used(TARGET_LAST_TRANMISSION_SENDER);
			if ( hud_sensors_ok(Player_ship)) {
				hud_target_last_transmit();
			}
			break;

		// target the closest repair ship
		case TARGET_CLOSEST_REPAIR_SHIP:
			control_used(TARGET_CLOSEST_REPAIR_SHIP);
			// AL: Try to find the closest repair ship coming to repair the player... if no support
			//		 ships are coming to rearm the player, just try for the closest repair ship
			if ( hud_target_closest_repair_ship(OBJ_INDEX(Player_obj)) == 0 ) {
				if ( hud_target_closest_repair_ship() == 0 ) {
					snd_play(&Snds[SND_TARGET_FAIL]);
				}
			}
			break;

		// target the closest ship attacking current target
		case TARGET_CLOSEST_SHIP_ATTACKING_TARGET:
			control_used(TARGET_CLOSEST_SHIP_ATTACKING_TARGET);
			if (hud_sensors_ok(Player_ship))
			{
				if (Player_ai->target_objnum < 0)
				{
					snd_play(&Snds[SND_TARGET_FAIL]);
					break;
				}

				hud_target_closest(iff_get_attacker_mask(obj_team(&Objects[Player_ai->target_objnum])), Player_ai->target_objnum);
			}
			break;

		// stop targeting ship
		case STOP_TARGETING_SHIP:
			control_used(STOP_TARGETING_SHIP);
			hud_cease_targeting();
			break;

		// target closest ship that is attacking player
		case TARGET_CLOSEST_SHIP_ATTACKING_SELF:
			control_used(TARGET_CLOSEST_SHIP_ATTACKING_SELF);
			if (hud_sensors_ok(Player_ship)){
				hud_target_closest(iff_get_attacker_mask(Player_ship->team), OBJ_INDEX(Player_obj), TRUE, 0, 1);
			}
			break;

		// target your target's target
		case TARGET_TARGETS_TARGET:
			control_used(TARGET_TARGETS_TARGET);
			if (hud_sensors_ok(Player_ship)){
				hud_target_targets_target();
			}
			break;

		// target ships subsystem in reticle
		case TARGET_SUBOBJECT_IN_RETICLE:
			control_used(TARGET_SUBOBJECT_IN_RETICLE);
			if (hud_sensors_ok(Player_ship)){
				hud_target_subsystem_in_reticle();
			}
			break;

		case TARGET_PREV_SUBOBJECT:
			control_used(TARGET_PREV_SUBOBJECT);
			if (hud_sensors_ok(Player_ship)){
				hud_target_prev_subobject();
			}
			break;

		// target next subsystem on current target
		case TARGET_NEXT_SUBOBJECT:
			control_used(TARGET_NEXT_SUBOBJECT);
			if (hud_sensors_ok(Player_ship)){
				hud_target_next_subobject();
			}
			break;

		// stop targeting subsystems on ship
		case STOP_TARGETING_SUBSYSTEM:
			control_used(STOP_TARGETING_SUBSYSTEM);
			hud_cease_subsystem_targeting();
			break;
			
		case TARGET_NEXT_BOMB:
			control_used(TARGET_NEXT_BOMB);
			hud_target_missile(Player_obj, 1);
			break;

		case TARGET_PREV_BOMB:
			control_used(TARGET_PREV_BOMB);
			hud_target_missile(Player_obj, 0);
			break;

		case TARGET_NEXT_UNINSPECTED_CARGO:
			hud_target_uninspected_object(1);
			break;

		case TARGET_PREV_UNINSPECTED_CARGO:
			hud_target_uninspected_object(0);
			break;

		case TARGET_NEWEST_SHIP:
			hud_target_newest_ship();
			break;

		case TARGET_NEXT_LIVE_TURRET:
			hud_target_live_turret(1);
			break;

		case TARGET_PREV_LIVE_TURRET:
			hud_target_live_turret(0);
			break;

		// wingman message: attack current target
		case ATTACK_MESSAGE:
			control_used(ATTACK_MESSAGE);
			hud_squadmsg_shortcut( ATTACK_TARGET_ITEM );
			break;

		// wingman message: disarm current target
		case DISARM_MESSAGE:
			control_used(DISARM_MESSAGE);
			hud_squadmsg_shortcut( DISARM_TARGET_ITEM );
			break;

		// wingman message: disable current target
		case DISABLE_MESSAGE:
			control_used(DISABLE_MESSAGE);
			hud_squadmsg_shortcut( DISABLE_TARGET_ITEM );
			break;

		// wingman message: disable current target
		case ATTACK_SUBSYSTEM_MESSAGE:
			control_used(ATTACK_SUBSYSTEM_MESSAGE);
			hud_squadmsg_shortcut( DISABLE_SUBSYSTEM_ITEM );
			break;

		// wingman message: capture current target
		case CAPTURE_MESSAGE:
			control_used(CAPTURE_MESSAGE);
			hud_squadmsg_shortcut( CAPTURE_TARGET_ITEM );
			break;

		// wingman message: engage enemy
		case ENGAGE_MESSAGE:
    		control_used(ENGAGE_MESSAGE);
			hud_squadmsg_shortcut( ENGAGE_ENEMY_ITEM );
			break;

		// wingman message: form on my wing
		case FORM_MESSAGE:
			control_used(FORM_MESSAGE);
			hud_squadmsg_shortcut( FORMATION_ITEM );
			break;

		// wingman message: protect current target
		case PROTECT_MESSAGE:
			control_used(PROTECT_MESSAGE);
			hud_squadmsg_shortcut( PROTECT_TARGET_ITEM );
			break;

		// wingman message: cover me
		case COVER_MESSAGE:
			control_used(COVER_MESSAGE);
			hud_squadmsg_shortcut( COVER_ME_ITEM );
			break;
		
		// wingman message: warp out
		case WARP_MESSAGE:
			control_used(WARP_MESSAGE);
			hud_squadmsg_shortcut( DEPART_ITEM );
			break;

		case IGNORE_MESSAGE:
			control_used(IGNORE_MESSAGE);
			hud_squadmsg_shortcut( IGNORE_TARGET_ITEM );
			break;

		// rearm message
		case REARM_MESSAGE:
			control_used(REARM_MESSAGE);
			hud_squadmsg_rearm_shortcut();
			break;

		// cycle to next radar range
		case RADAR_RANGE_CYCLE:
			control_used(RADAR_RANGE_CYCLE);
			HUD_config.rp_dist++;
			if ( HUD_config.rp_dist >= RR_MAX_RANGES )
				HUD_config.rp_dist = 0;

			HUD_sourced_printf(HUD_SOURCE_HIDDEN, XSTR( "Radar range set to %s", 38), Radar_range_text(HUD_config.rp_dist));
			break;

		// toggle the squadmate messaging menu
		case SQUADMSG_MENU:
			control_used(SQUADMSG_MENU);
			hud_squadmsg_toggle();				// leave the details to the messaging code!!!
			break;

		// show the mission goals screen
		case SHOW_GOALS:
			control_used(SHOW_GOALS);
			gameseq_post_event( GS_EVENT_SHOW_GOALS );
			break;

		// end the mission
		case END_MISSION:
			// in multiplayer, all end mission requests should go through the server
			if(Game_mode & GM_MULTIPLAYER){				
				multi_handle_end_mission_request();
				break;
			}

			control_used(END_MISSION);
			
			if (collide_predict_large_ship(Player_obj, 200.0f) 
			|| (Ship_info[Ships[Player_obj->instance].ship_info_index].warpout_type == WT_HYPERSPACE 
			&& collide_predict_large_ship(Player_obj, 100000.0f)))
			{
				gamesnd_play_iface(SND_GENERAL_FAIL);
				HUD_printf(XSTR( "** WARNING ** Collision danger.  Subspace drive not activated.", 39));
			} else if (!ship_engine_ok_to_warp(Player_ship)) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				HUD_printf(XSTR("Engine failure.  Cannot engage subspace drive.", 40));
			} else if (!ship_navigation_ok_to_warp(Player_ship)) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				HUD_printf(XSTR("Navigation failure.  Cannot engage subspace drive.", 1572));
			} else if (Player_obj != NULL && object_get_gliding(Player_obj)) {
				gamesnd_play_iface(SND_GENERAL_FAIL);
				HUD_printf(XSTR("Cannot engage subspace drive while gliding.", 1573));			
			} else {
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_START );
			}			
			break;

		case ADD_REMOVE_ESCORT:
			if ( Player_ai->target_objnum >= 0 ) {
				control_used(ADD_REMOVE_ESCORT);
				hud_add_remove_ship_escort(Player_ai->target_objnum);
			}
			break;

		case ESCORT_CLEAR:
			control_used(ESCORT_CLEAR);
			hud_escort_clear_all();
			break;

		case TARGET_NEXT_ESCORT_SHIP:
			control_used(TARGET_NEXT_ESCORT_SHIP);
			hud_escort_target_next();
			break;

		// increase weapon recharge rate
		case INCREASE_WEAPON:
			if ((Player_ship->flags2 & SF2_NO_ETS) == 0) {
				hud_gauge_popup_start(HUD_ETS_GAUGE);
				return button_function_critical(INCREASE_WEAPON);
			}
			break;

		// decrease weapon recharge rate
		case DECREASE_WEAPON:
			if ((Player_ship->flags2 & SF2_NO_ETS) == 0) {
				hud_gauge_popup_start(HUD_ETS_GAUGE);
				return button_function_critical(DECREASE_WEAPON);
			}
			break;

		// increase shield recharge rate
		case INCREASE_SHIELD:
			if ((Player_ship->flags2 & SF2_NO_ETS) == 0) {
				hud_gauge_popup_start(HUD_ETS_GAUGE);
				return button_function_critical(INCREASE_SHIELD);
			}
			break;

		// decrease shield recharge rate
		case DECREASE_SHIELD:
			if ((Player_ship->flags2 & SF2_NO_ETS) == 0) {
				hud_gauge_popup_start(HUD_ETS_GAUGE);
				return button_function_critical(DECREASE_SHIELD);
			}
			break;

		// increase energy to engines
		case INCREASE_ENGINE:
			if ((Player_ship->flags2 & SF2_NO_ETS) == 0) {
				hud_gauge_popup_start(HUD_ETS_GAUGE);
				return button_function_critical(INCREASE_ENGINE);
			}
			break;

		// decrease energy to engines
		case DECREASE_ENGINE:
			if ((Player_ship->flags2 & SF2_NO_ETS) == 0) {
				hud_gauge_popup_start(HUD_ETS_GAUGE);
				return button_function_critical(DECREASE_ENGINE);
			}
			break;

		case ETS_EQUALIZE:
			if ((Player_ship->flags2 & SF2_NO_ETS) == 0) {
				hud_gauge_popup_start(HUD_ETS_GAUGE);
				return button_function_critical(ETS_EQUALIZE);
			}
			break;

		// equalize shield energy to all quadrants
		case SHIELD_EQUALIZE:
			return button_function_critical(SHIELD_EQUALIZE);
			break;

		// transfer shield energy to front
		case SHIELD_XFER_TOP:
         return button_function_critical(SHIELD_XFER_TOP);
			break;

		// transfer shield energy to rear
		case SHIELD_XFER_BOTTOM:
			return button_function_critical(SHIELD_XFER_BOTTOM);
			break;

		// transfer shield energy to left
		case SHIELD_XFER_LEFT:
			return button_function_critical(SHIELD_XFER_LEFT);
			break;
		
		// transfer shield energy to right
		case SHIELD_XFER_RIGHT:
			return button_function_critical(SHIELD_XFER_RIGHT);
			break;

		// transfer energy to shield from weapons
		case XFER_SHIELD:
			return button_function_critical(XFER_SHIELD);
			break;

		// transfer energy to weapons from shield
		case XFER_LASER:
			return button_function_critical(XFER_LASER);
			break;

		// message all netplayers button
		case MULTI_MESSAGE_ALL:
			multi_msg_key_down(MULTI_MSG_ALL);
			break;

		// message all friendlies button
		case MULTI_MESSAGE_FRIENDLY:
			multi_msg_key_down(MULTI_MSG_FRIENDLY);
			break;

		// message all hostiles button
		case MULTI_MESSAGE_HOSTILE:
			multi_msg_key_down(MULTI_MSG_HOSTILE);
			break;

		// message targeted ship (if player)
		case MULTI_MESSAGE_TARGET:
			multi_msg_key_down(MULTI_MSG_TARGET);
			break;

		// if i'm an observer, zoom to my targeted object
		case MULTI_OBSERVER_ZOOM_TO:
			multi_obs_zoom_to_target();
			break;

		// toggle between high and low HUD contrast
		case TOGGLE_HUD_CONTRAST:
			gamesnd_play_iface(SND_USER_SELECT);
			hud_toggle_contrast();
			break;

		// toggle network info
		case MULTI_TOGGLE_NETINFO:
			extern int Multi_display_netinfo;
			Multi_display_netinfo = !Multi_display_netinfo;
			break;

		// self destruct (multiplayer only)
		case MULTI_SELF_DESTRUCT:
			if(!(Game_mode & GM_MULTIPLAYER)){
				break;
			}

			// bogus netplayer
			if((Net_player == NULL) || (Net_player->m_player == NULL)){
				break;
			}

			// blow myself up, if I'm the server
			if(Net_player->flags & NETINFO_FLAG_AM_MASTER){
				if((Net_player->m_player->objnum >= 0) && (Net_player->m_player->objnum < MAX_OBJECTS) && 
					(Objects[Net_player->m_player->objnum].type == OBJ_SHIP) && (Objects[Net_player->m_player->objnum].instance >= 0) && (Objects[Net_player->m_player->objnum].instance < MAX_SHIPS)){

					ship_self_destruct(&Objects[Net_player->m_player->objnum]);
				}
			}
			// otherwise send a packet to the server
			else {
				send_self_destruct_packet();
			}
			break;

		case TOGGLE_HUD:
			gamesnd_play_iface(SND_USER_SELECT);
			hud_toggle_draw();
			break;

		case HUD_TARGETBOX_TOGGLE_WIREFRAME:
			if (!Lock_targetbox_mode) {
				gamesnd_play_iface(SND_USER_SELECT);
				hud_targetbox_switch_wireframe_mode();
			}
			else {
				gamesnd_play_iface(SND_GENERAL_FAIL); 
			}
			break;	

		// Autopilot key control
		case AUTO_PILOT_TOGGLE:
			if (!(The_mission.flags & MISSION_FLAG_DEACTIVATE_AP)) {
				if (AutoPilotEngaged)
				{
					if (Cmdline_autopilot_interruptable == 1) //allow WCS to disable autopilot interrupt via commandline
						EndAutoPilot();
				}
				else
				{
					if (!StartAutopilot())
						gamesnd_play_iface(SND_GENERAL_FAIL);
				}
			}
			break;

		case NAV_CYCLE:
			if (!Sel_NextNav())
				gamesnd_play_iface(SND_GENERAL_FAIL);
			break;
		
		// following are not handled here, but we need to bypass the Int3()
		case LAUNCH_COUNTERMEASURE:
		case VIEW_SLEW:
		case VIEW_TRACK_TARGET:
		case ONE_THIRD_THROTTLE:
		case TWO_THIRDS_THROTTLE:
		case MINUS_5_PERCENT_THROTTLE:
		case PLUS_5_PERCENT_THROTTLE:
		case ZERO_THROTTLE:
		case MAX_THROTTLE:
		case TOGGLE_GLIDING:
		case GLIDE_WHEN_PRESSED:
			return 0;

		default:
			mprintf(("Unknown key %d at %s:%u\n", n, __FILE__, __LINE__));
			break;
	} // end switch

	return 1;
}

/**
 * Calls multiple event handlers for each active button
 * @param bi currently active buttons
 */
void button_info_do(button_info *bi)
{
	for (int i = 0; i < CCFG_MAX; i++) {
		if( button_info_query(bi, i) ) {
			int keyHasBeenUsed = FALSE;
			
			if( !keyHasBeenUsed ) {
				keyHasBeenUsed = button_function_demo_valid(i);
			}
			
			if( !keyHasBeenUsed ) {
				keyHasBeenUsed = button_function(i);
			}
			
			if( keyHasBeenUsed ) {
				button_info_unset(bi, i);
			}
		}
	}
}


/**
 * Set the bit for the corresponding action n (BUTTON_ from KeyControl.h)
 */
void button_info_set(button_info *bi, int n)
{
	int field_num, bit_num;
	
	field_num = n / 32;
	bit_num = n % 32;

	bi->status[field_num] |= (1 << bit_num);	
}

/**
 * Unset the bit for the corresponding action n (BUTTON_ from KeyControl.h)
 */
void button_info_unset(button_info *bi, int n)
{
	int field_num, bit_num;
	
	field_num = n / 32;
	bit_num = n % 32;

	bi->status[field_num] &= ~(1 << bit_num);	
}

int button_info_query(button_info *bi, int n)
{
	return bi->status[n / 32] & (1 << (n % 32));
}

/**
 * Clear out the ::button_info struct
 */
void button_info_clear(button_info *bi)
{
	int i;

	for (i=0; i<NUM_BUTTON_FIELDS; i++) {
		bi->status[i] = 0;
	}
}

/**
 * Strip out all noncritical keys from the ::button_info struct
 */
void button_strip_noncritical_keys(button_info *bi)
{
	int idx;

	// clear out all noncritical keys
	for(idx=0;idx<Non_critical_key_set_size;idx++){
		button_info_unset(bi,Non_critical_key_set[idx]);
	}
}