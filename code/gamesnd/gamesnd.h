/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __GAMESND_H__
#define __GAMESND_H__

#include "sound/sound.h"
#include "mission/missionparse.h"


void gamesnd_parse_soundstbl();	// Loads in general game sounds from sounds.tbl
void gamesnd_init_sounds();		// initializes the Snds[] and Snds_iface[] array
void gamesnd_close();	// close out gamesnd... only call from game_shutdown()!
void gamesnd_load_gameplay_sounds();
void gamesnd_unload_gameplay_sounds();
void gamesnd_load_interface_sounds();
void gamesnd_unload_interface_sounds();
void gamesnd_preload_common_sounds();
void gamesnd_play_iface(int n);
void gamesnd_play_error_beep();
int gamesnd_get_by_name(char* name);
int gamesnd_get_by_tbl_index(int index);
int gamesnd_get_by_iface_tbl_index(int index);

//This should handle NO_SOUND just fine since it doesn't directly access lowlevel code
//Does all parsing for a sound
void parse_sound(char* tag, int *idx_dest, char* object_name);

// this is a callback, so it needs to be a real function
void common_play_highlight_sound();


// match original values by default, grow from this
#define MIN_GAME_SOUNDS					202
#define MIN_INTERFACE_SOUNDS			70

extern SCP_vector<game_snd> Snds;
extern SCP_vector<game_snd> Snds_iface;


// symbolic names for misc. game sounds.  The order here must match the order in
// sounds.tbl
//
// INSTRUCTIONS FOR ADDING A NEW SOUND:
//
// Add interface (ie non-gameplay) sounds to the end of the interface list of sounds.
// Add gameplay sounds to the correct portion of the gameplay sounds section (ie Misc, 
// Weapons, or Ship).
//
// Then add a symbolic name to the appropriate position in the #define list below
// and add an entry to sounds.tbl.  If there is no .wav file for the sound yet, 
// specify sound_hook.wav in sounds.tbl.
//
//

//---------------------------------------------------
// Misc Sounds
//---------------------------------------------------
#define	SND_MISSILE_TRACKING			0
#define	SND_MISSILE_LOCK				1
#define	SND_PRIMARY_CYCLE				2
#define	SND_SECONDARY_CYCLE			3
#define	SND_ENGINE						4
#define	SND_CARGO_REVEAL				5
#define	SND_DEATH_ROLL					6
#define	SND_SHIP_EXPLODE_1			7
#define	SND_TARGET_ACQUIRE			8
#define	SND_ENERGY_ADJUST				9
#define	SND_ENERGY_ADJUST_FAIL		10
#define	SND_ENERGY_TRANS				11
#define	SND_ENERGY_TRANS_FAIL		12
#define	SND_FULL_THROTTLE				13
#define	SND_ZERO_THROTTLE				14
#define	SND_THROTTLE_UP				15
#define	SND_THROTTLE_DOWN				16
#define	SND_DOCK_APPROACH				17
#define	SND_DOCK_ATTACH				18
#define	SND_DOCK_DETACH				19
#define	SND_DOCK_DEPART				20
#define	SND_ABURN_ENGAGE				21
#define	SND_ABURN_LOOP					22
#define	SND_VAPORIZED					23
#define	SND_ABURN_FAIL					24
#define	SND_HEATLOCK_WARN				25
#define	SND_OUT_OF_MISSLES			26
#define	SND_OUT_OF_WEAPON_ENERGY	27
#define	SND_TARGET_FAIL				28
#define	SND_SQUADMSGING_ON			29
#define	SND_SQUADMSGING_OFF			30
#define	SND_DEBRIS						31
#define	SND_SUBSYS_DIE_1				32
#define	SND_MISSILE_START_LOAD		33
#define	SND_MISSILE_LOAD				34
#define  SND_SHIP_REPAIR				35
#define  SND_PLAYER_HIT_LASER			36
#define  SND_PLAYER_HIT_MISSILE		37	
#define  SND_CMEASURE_CYCLE			38
#define  SND_SHIELD_HIT					39
#define  SND_SHIELD_HIT_YOU			40
#define	SND_GAME_MOUSE_CLICK			41
#define	SND_ASPECTLOCK_WARN			42
#define	SND_SHIELD_XFER_OK			43
#define  SND_ENGINE_WASH				44
#define	SND_WARP_IN						45
#define	SND_WARP_OUT					46		// Same as warp in for now
#define	SND_PLAYER_WARP_FAIL			47
#define	SND_STATIC						48
#define	SND_SHIP_EXPLODE_2			49
#define	SND_PLAYER_WARP_OUT			50
#define	SND_SHIP_SHIP_HEAVY				51
#define	SND_SHIP_SHIP_LIGHT				52
#define	SND_SHIP_SHIP_SHIELD				53
#define	SND_THREAT_FLASH					54
#define	SND_PROXIMITY_WARNING			55
#define	SND_PROXIMITY_ASPECT_WARNING	56
#define	SND_DIRECTIVE_COMPLETE			57
#define	SND_SUBSYS_EXPLODE				58
#define	SND_CAPSHIP_EXPLODE				59
#define	SND_CAPSHIP_SUBSYS_EXPLODE		60
#define	SND_LARGESHIP_WARPOUT			61
#define	SND_ASTEROID_EXPLODE_LARGE		62
#define	SND_ASTEROID_EXPLODE_SMALL		63
#define	SND_CUE_VOICE						64
#define	SND_END_VOICE						65
#define	SND_CARGO_SCAN						66
#define	SND_WEAPON_FLYBY					67
#define	SND_ASTEROID						68
#define	SND_CAPITAL_WARP_IN				69
#define	SND_CAPITAL_WARP_OUT				70
#define	SND_ENGINE_LOOP_LARGE			71
#define	SND_SUBSPACE_LEFT_CHANNEL		72
#define	SND_SUBSPACE_RIGHT_CHANNEL		73
#define	SND_MISSILE_EVADED_POPUP		74
#define  SND_ENGINE_LOOP_HUGE				75

// Weapon sounds
#define	SND_LIGHT_LASER_FIRE				76
#define	SND_LIGHT_LASER_IMPACT			77
#define	SND_HVY_LASER_FIRE				78
#define	SND_HVY_LASER_IMPACT				79
#define	SND_MASSDRV_FIRED					80
#define	SND_MASSDRV_IMPACT				81
#define	SND_FLAIL_FIRED	 				82
#define	SND_FLAIL_IMPACT					83
#define	SND_NEUTRON_FLUX_FIRED			84
#define	SND_NEUTRON_FLUX_IMPACT			85
#define	SND_DEBUG_LASER_FIRED			86
#define	SND_ROCKEYE_FIRED					87
#define	SND_MISSILE_IMPACT1				88
#define	SND_MAG_MISSILE_LAUNCH			89
#define	SND_FURY_MISSILE_LAUNCH			90
#define	SND_SHRIKE_MISSILE_LAUNCH		91
#define	SND_ANGEL_MISSILE_LAUNCH		92
#define	SND_CLUSTER_MISSILE_LAUNCH		93
#define	SND_CLUSTERB_MISSILE_LAUNCH	94
#define	SND_STILETTO_MISSILE_LAUNCH	95
#define	SND_TSUNAMI_MISSILE_LAUNCH		96
#define	SND_HARBINGER_MISSILE_LAUNCH	97
#define	SND_MEGAWOKKA_MISSILE_LAUNCH	98
#define	SND_CMEASURE1_LAUNCH				99
#define	SND_SHIVAN_LIGHT_LASER_FIRE	100
#define	SND_SHOCKWAVE_EXPLODE			101
#define	SND_SWARM_MISSILE_LAUNCH		102
#define	SND_SHOCKWAVE_IMPACT				109

#define	SND_TARG_LASER_LOOP				115
#define  SND_FLAK_FIRE						116
#define	SND_SHIELD_BREAKER				117
#define	SND_EMP_MISSILE					118
#define	SND_AUTOCANNON_LOOP				119
#define	SND_AUTOCANNON_SHOT				120
#define	SND_BEAM_LOOP						121
#define	SND_BEAM_UP							122
#define	SND_BEAM_DOWN						123
#define	SND_BEAM_SHOT						124
#define	SND_BEAM_VAPORIZE					125

// Ship engine sounds
#define	SND_TERRAN_FIGHTER_ENG			126
#define	SND_TERRAN_BOMBER_ENG			127
#define	SND_TERRAN_CAPITAL_ENG			128
#define	SND_SPECIESB_FIGHTER_ENG		129
#define	SND_SPECIESB_BOMBER_ENG			130
#define	SND_SPECIESB_CAPITAL_ENG		131
#define	SND_SHIVAN_FIGHTER_ENG			132
#define	SND_SHIVAN_BOMBER_ENG			133
#define	SND_SHIVAN_CAPITAL_ENG			134
#define	SND_REPAIR_SHIP_ENG				135

// Debris electric arcing sounds
#define	SND_DEBRIS_ARC_01					139		// 0.10 second spark sound effect (3d sound)
#define	SND_DEBRIS_ARC_02					140		// 0.25 second spark sound effect (3d sound)
#define	SND_DEBRIS_ARC_03					141		// 0.50 second spark sound effect (3d sound)
#define	SND_DEBRIS_ARC_04					142		// 0.75 second spark sound effect (3d sound)
#define	SND_DEBRIS_ARC_05					143		// 1.00 second spark sound effect (3d sound)

// copilot
#define	SND_COPILOT							162

// supernova 1 and supernova 2
#define	SND_SUPERNOVA_1					173
#define	SND_SUPERNOVA_2					174

// lightning sounds
#define	SND_LIGHTNING_1					180
#define	SND_LIGHTNING_2					181

// added for ballistic primaries - Goober5000
#define	SND_BALLISTIC_START_LOAD		200
#define	SND_BALLISTIC_LOAD				201

//---------------------------------------------------
// Interface sounds
//---------------------------------------------------
#define  SND_IFACE_MOUSE_CLICK		0
#define  SND_ICON_PICKUP				1
#define  SND_ICON_DROP_ON_WING		2
#define  SND_ICON_DROP					3
#define  SND_SCREEN_MODE_PRESSED		4
#define  SND_SWITCH_SCREENS			5
#define  SND_HELP_PRESSED				6
#define  SND_COMMIT_PRESSED			7
#define  SND_PREV_NEXT_PRESSED		8
#define  SND_SCROLL						9
#define  SND_GENERAL_FAIL				10
#define  SND_SHIP_ICON_CHANGE			11
#define  SND_MAIN_HALL_AMBIENT		12
#define  SND_BTN_SLIDE					13
#define	SND_BRIEF_STAGE_CHG			14
#define	SND_BRIEF_STAGE_CHG_FAIL	15
#define	SND_BRIEF_ICON_SELECT		16
#define	SND_USER_OVER					17
#define	SND_USER_SELECT				18
#define	SND_RESET_PRESSED				19
#define	SND_BRIEF_TEXT_WIPE			20
#define	SND_VASUDAN_PA_1				21				// vasudan pa 1
#define	SND_WEAPON_ANIM_START		22
#define  SND_MAIN_HALL_DOOR_OPEN    23
#define  SND_MAIN_HALL_DOOR_CLOSE   24
#define	SND_GLOW_OPEN					25
#define	SND_VASUDAN_PA_2				26				// vasudan pa 2
#define	SND_AMBIENT_MENU				27
#define	SND_POPUP_APPEAR				28
#define	SND_POPUP_DISAPPEAR			29
#define	SND_VOICE_SLIDER_CLIP		30
#define  SND_VASUDAN_PA_3				31				// vasudan pa 3
#define  SND_MAIN_HALL_GET_PEPSI		32
#define  SND_MAIN_HALL_LIFT_UP		33
#define  SND_MAIN_HALL_WELD1			34
#define  SND_MAIN_HALL_WELD2			35
#define  SND_MAIN_HALL_WELD3			36
#define  SND_MAIN_HALL_WELD4			37
#define  SND_MAIN_HALL_INT1			38			// random intercom message 1
#define  SND_MAIN_HALL_INT2			39			// random intercom message 2
#define  SND_MAIN_HALL_INT3			40			// random intercom message 3		
#define	SND_ICON_HIGHLIGHT			41
#define	SND_BRIEFING_STATIC			42
#define	SND_MAIN_HALL2_CRANE1_1		43
#define	SND_MAIN_HALL2_CRANE1_2		44
#define	SND_MAIN_HALL2_CRANE2_1		45
#define	SND_MAIN_HALL2_CRANE2_2		46
#define	SND_MAIN_HALL2_CAR1			47
#define	SND_MAIN_HALL2_CAR2			48
#define	SND_MAIN_HALL2_INT1			49
#define	SND_MAIN_HALL2_INT2			50
#define	SND_MAIN_HALL2_INT3			51

#define	SND_VASUDAN_BUP				61

#endif	/* __GAMESND_H__ */
