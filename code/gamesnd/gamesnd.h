/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Gamesnd/GameSnd.h $
 * $Revision: 2.16 $
 * $Date: 2006-08-18 04:34:54 $
 * $Author: Goober5000 $
 *
 * Routines to keep track of which sound files go where
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.15  2005/09/25 08:25:15  Goober5000
 * Okay, everything should now work again. :p Still have to do a little more with the asteroids.
 * --Goober5000
 *
 * Revision 2.14  2005/09/25 05:13:06  Goober5000
 * hopefully complete species upgrade
 * --Goober5000
 *
 * Revision 2.13  2005/07/13 02:50:51  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.12  2005/07/13 00:44:22  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.11  2005/06/29 18:50:13  taylor
 * little cleanup and error checking
 *
 * Revision 2.10  2005/04/25 00:22:34  wmcoolmon
 * Added parse_sound; replaced Assert() with an if() (The latter may not be a good idea, but it keeps missions from being un-debuggable)
 *
 * Revision 2.9  2005/04/21 15:58:08  taylor
 * initial changes to mission loading and status in debug builds
 *  - move bmpman page in init to an earlier stage to avoid unloading sexp loaded images
 *  - small changes to progress reports in debug builds so that it's easier to tell what's slow
 *  - initialize the loading screen before mission_parse() so that we'll be about to get a more accurate load time
 * fix memory leak in gamesnd (yes, I made a mistake ;))
 * make sure we unload models on game shutdown too
 *
 * Revision 2.8  2005/03/30 02:32:40  wmcoolmon
 * Made it so *Snd fields in ships.tbl and weapons.tbl take the sound name
 * as well as its index (ie "L_sidearm.wav" instead of "76")
 *
 * Revision 2.7  2005/03/24 23:31:46  taylor
 * make sounds.tbl dynamic
 * "filename" will never be larger than 33 chars so having it 260 is a waste (freespace.cpp)
 *
 * Revision 2.6  2004/08/11 05:06:23  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.5  2004/03/12 16:23:49  phreak
 * bumped MAX_GAME_SOUNDS to 400 if INF_BUILD is defined
 *
 * Revision 2.4  2003/03/20 22:58:43  Goober5000
 * moved ballistic primary reload sounds to indexes 171 and 172
 *
 * Revision 2.3  2003/03/18 01:44:31  Goober5000
 * fixed some misspellings
 * --Goober5000
 *
 * Revision 2.2  2002/12/10 05:43:35  Goober5000
 * Full-fledged ballistic primary support added!  Try it and see! :)
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/10 06:08:08  mharris
 * Porting... added ifndef NO_SOUND
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 15    9/12/99 8:09p Dave
 * Fixed problem where skip-training button would cause mission messages
 * not to get paged out for the current mission.
 * 
 * 14    9/09/99 11:40p Dave
 * Handle an Assert() in beam code. Added supernova sounds. Play the right
 * 2 end movies properly, based upon what the player did in the mission.
 * 
 * 13    8/27/99 11:59a Jefff
 * changed some sound names to better reflect new uses
 * 
 * 12    8/26/99 9:45a Dave
 * First pass at easter eggs and cheats.
 * 
 * 11    7/19/99 11:47a Jefff
 * Added sound hook for countermeasure success
 * 
 * 10    7/02/99 4:31p Dave
 * Much more sophisticated lightning support.
 * 
 * 9     7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 8     6/25/99 3:08p Dave
 * Multiple flyby sounds.
 * 
 * 7     6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 6     4/19/99 11:01p Dave
 * More sophisticated targeting laser support. Temporary checkin.
 * 
 * 5     2/04/99 6:29p Dave
 * First full working rev of FS2 PXO support.  Fixed Glide lighting
 * problems.
 * 
 * 4     1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 3     10/26/98 9:42a Dave
 * Early flak gun support.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 76    5/18/98 12:59a Lawrance
 * Replace shockwave impact sound with a new "whoosh" sound that
 * originates from the shockwave center
 * 
 * 75    5/06/98 10:29a Dave
 * Put in support for panning sounds. Put in new sound hooks for main hall
 * 2 intercom sounds.
 * 
 * 74    4/27/98 3:36p Dave
 * 
 * 73    4/25/98 1:25p Lawrance
 * Make function for playing generic error beep
 * 
 * 72    4/19/98 9:33p Lawrance
 * Added sound hooks for Shivan flyby sound, subspace ambient effect
 * 
 * 71    3/23/98 4:13p Lawrance
 * Add hooks for capital ship specific warp sounds
 * 
 * 70    3/21/98 3:34p Lawrance
 * Added highlight icon sound, added static sound for briefing cut
 * 
 * 69    3/17/98 5:55p Lawrance
 * Support object-linked sounds for asteroids.
 * 
 * 68    3/17/98 3:53p Lawrance
 * First pass at weapon flyby sounds
 * 
 * 67    3/16/98 5:54p Lawrance
 * Play cargo scanning sound
 * 
 * 66    3/05/98 10:18p Lawrance
 * Play voice cue sound when there is no voice file present
 * 
 * 65    2/19/98 4:33p Lawrance
 * add asteroid sound hooks
 * 
 * 64    2/12/98 4:59p Lawrance
 * Add new sound hooks for subsystem explosions, and subsystems getting
 * destroyed
 * 
 * 63    2/11/98 5:32p Dave
 * Put in hooks for 3 random intercom sounds in the main hall.
 * 
 * 62    2/06/98 11:33a Dave
 * Made sounds customizable through sounds.tbl
 * 
 * 61    2/04/98 6:08p Lawrance
 * Add a light collision sound, overlay a shield collide sound if
 * applicable.
 * 
 * 60    1/30/98 11:48a John
 * Made debris arcs cast light.  Added sound effects for them.
 * 
 * 59    1/11/98 11:14p Lawrance
 * Preload sounds that we expect will get played.
 * 
 * 58    1/07/98 11:10a Lawrance
 * Add new several new sound hooks.
 * 
 * 57    12/24/97 8:54p Lawrance
 * Integrating new popup code
 * 
 * 56    12/09/97 11:31a Lawrance
 * add missile launch warnings, re-work proximity beep
 * 
 * 55    12/05/97 2:39p Lawrance
 * added some different sounds to main hall, add support for looping
 * ambient sounds
 * 
 * 54    12/03/97 4:58p Hoffoss
 * 
 * 53    12/03/97 4:16p Hoffoss
 * Changed sound stuff used in interface screens for interface purposes.
 * 
 * 52    12/01/97 5:25p Hoffoss
 * Routed interface sound playing through a special function that will
 * only allow one instance of the sound to play at a time, avoiding
 * over-mixing problems.
 * 
 * 51    11/20/97 5:36p Dave
 * Hooked in a bunch of main hall changes (including sound). Made it
 * possible to reposition (rewind/ffwd) 
 * sound buffer pointers. Fixed animation direction change framerate
 * problem.
 * 
 * 50    11/11/97 10:25p Lawrance
 * add sound hook for when missile threat flashes
 * 
 * 49    11/03/97 11:08p Lawrance
 * Add sound for collisions with shields.
 * 
 * 48    11/03/97 2:07p Lawrance
 * add ship-to-ship collision sound
 * 
 * 47    10/28/97 4:49p Lawrance
 * add sound hook that gets played when player warps out
 * 
 * 46    10/27/97 10:48p Lawrance
 * add second explosion sound
 * 
 * 45    10/11/97 6:39p Lawrance
 * added sound hooks for static sound
 * 
 * 44    10/10/97 7:45p Lawrance
 * add warp fail sound
 * 
 * 43    9/19/97 2:45p Lawrance
 * add hook for weapon animation sound
 * 
 * 42    9/12/97 4:02p John
 * put in ship warp out effect.
 * put in dynamic lighting for warp in/out
 * 
 * 41    9/07/97 10:01p Lawrance
 * add some interface hooks
 * 
 * 40    9/05/97 4:59p Lawrance
 * added warp sound hook
 * 
 * 39    9/03/97 5:05p Lawrance
 * add support for ship flyby sound
 * 
 * 38    8/25/97 12:25a Lawrance
 * added sound for when shield energy is transferred between quadrants 
 * 
 * 37    8/18/97 5:28p Lawrance
 * added some new interface sounds
 * 
 * 36    8/12/97 5:52p Lawrance
 * added hook for lock warning from aspect missile
 * 
 * 35    8/11/97 9:48p Lawrance
 * add sound hook for swarm missile
 * 
 * 34    8/06/97 10:27a Lawrance
 * added hook for shockwave create sound
 * 
 * 33    7/17/97 10:50a Lawrance
 * create a Shivan light laser so sound can be assigned
 * 
 * 32    6/24/97 11:47p Lawrance
 * add briefing sound fx
 * 
 * 31    6/24/97 3:14p Lawrance
 * add briefing sounds
 * 
 * 30    6/13/97 4:44p Lawrance
 * added another sound hook in ship selection
 * 
 * 29    6/12/97 5:15p Lawrance
 * added hook for ambient sound in briefing/ship select
 * 
 * 28    6/05/97 11:25a Lawrance
 * use sound signatures to ensure correct sound is loaded
 * 
 * 27    6/05/97 1:07a Lawrance
 * changes to support sound interface
 * 
 * 26    6/04/97 1:18p Lawrance
 * added hooks for shield impacts
 * 
 * 25    5/23/97 11:20a Lawrance
 * counter measure sounds specified in weapons.tbl
 * 
 * 24    5/22/97 12:04p Lawrance
 * added soundhook for cmeasure cycle
 * 
 * 23    5/16/97 11:33a Lawrance
 * add countermeasures and rearm sounds
 * 
 * 22    5/14/97 11:24a Lawrance
 * break up docking sound effect into an approach/depart and attach/detach
 * 
 * 21    5/14/97 11:09a Lawrance
 * add hooks for sounds played when player is hit by lasers or missiles
 * 
 * 20    5/14/97 9:54a Lawrance
 * supporting mission-specific briefing music
 * 
 * 19    5/13/97 3:09p Lawrance
 * added repair/rearm sound
 * 
 * 18    5/13/97 10:44a Lawrance
 * add several new sound effects
 * 
 * 17    5/12/97 11:58a Lawrance
 * removed obsolete sound
 * 
 * 16    5/06/97 2:12p Lawrance
 * rearrange order of sounds
 * 
 * 15    5/06/97 9:36a Lawrance
 * added support for min and max distances for 3d sounds
 * 
 * 14    4/23/97 5:19p Lawrance
 * split up misc sounds into: gamewide, ingame, and interface
 * 
 * 13    4/20/97 11:48a Lawrance
 * added array of filenames for misc sounds.  Will be useful if we want to
 * unload then re-load sounds
 * 
 * 12    4/18/97 2:54p Lawrance
 * sounds now have a default volume, when playing, pass a scaling factor
 * not the actual volume
 * 
 * 11    4/15/97 2:49p Lawrance
 * add new sounds
 * 
 * 10    3/26/97 11:27a Lawrance
 * added hooks for sounds to be played when trying to fire missles when
 * none left / trying to fire lasers with no energy
 * 
 * 9     3/19/97 5:53p Lawrance
 * integrating new Misc_sounds[] array (replaces old Game_sounds
 * structure)
 * 
 * 8     3/17/97 3:47p Mike
 * Homing missile lock sound.
 * More on AI ships firing missiles.
 * 
 * 7     3/10/97 8:54a Lawrance
 * added gamesnd_init_looping_sounds()
 * 
 * 6     2/28/97 8:41a Lawrance
 * added afterburner engage and burn sounds
 * 
 * 5     2/14/97 12:37a Lawrance
 * added hooks to play docking/undocking sounds
 * 
 * 4     2/13/97 12:03p Lawrance
 * hooked in throttle sounds
 * 
 * 3     2/05/97 10:35a Lawrance
 * supporting spooled music at menus, briefings, credits etc.
 * 
 * 2     1/20/97 7:58p John
 * Fixed some link errors with testcode.
 * 
 * 1     1/20/97 7:08p John
 *
 * $NoKeywords: $
 */

#ifndef __GAMESND_H__
#define __GAMESND_H__

#include "sound/sound.h"
#include "mission/missionparse.h"

#ifndef NO_SOUND

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

#else

#define gamesnd_parse_soundstbl()
#define gamesnd_init_sounds()
#define gamesnd_load_gameplay_sounds()
#define gamesnd_unload_gameplay_sounds()
#define gamesnd_load_interface_sounds()
#define gamesnd_unload_interface_sounds()
#define gamesnd_preload_common_sounds()
#define gamesnd_play_iface(n)							((void)(n))
#define gamesnd_play_error_beep()
#define gamesnd_get_by_name(n)							(-1)

#endif  // ifndef NO_SOUND

//This should handle NO_SOUND just fine since it doesn't directly access lowlevel code
//Does all parsing for a sound
void parse_sound(char* tag, int *idx_dest, char* object_name);

// this is a callback, so it needs to be a real function
void common_play_highlight_sound();


// match original values by default, grow from this
#define MIN_GAME_SOUNDS					202
#define MIN_INTERFACE_SOUNDS			70

extern int Num_game_sounds;
extern int Num_iface_sounds;

extern game_snd *Snds;
extern game_snd *Snds_iface;


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
