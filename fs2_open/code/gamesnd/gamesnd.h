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
void gamesnd_close();	// close out gamesnd... only call from game_shutdown()!
void gamesnd_load_gameplay_sounds();
void gamesnd_unload_gameplay_sounds();
void gamesnd_load_interface_sounds();
void gamesnd_unload_interface_sounds();
void gamesnd_preload_common_sounds();
void gamesnd_load_gameplay_sounds();
void gamesnd_unload_gameplay_sounds();
void gamesnd_play_iface(int n);
void gamesnd_play_error_beep();
int gamesnd_get_by_name(char* name);
int gamesnd_get_by_iface_name(char* name);
int gamesnd_get_by_tbl_index(int index);
int gamesnd_get_by_iface_tbl_index(int index);

//flags for parse_sound and parse_sound_list
enum parse_sound_flags
{
	PARSE_SOUND_GENERAL_SOUND = 0,				//!< search for sound in the general table in sound.tbl
	PARSE_SOUND_INTERFACE_SOUND = (1 << 0),		//!< Search for sound in the interface part of sounds.tbl
	PARSE_SOUND_SCP_SOUND_LIST = (1 << 1),		//!< Parse the list of sounds SCP style (just indexes and/or files names, no count first)
	PARSE_SOUND_MAX
};

//This should handle NO_SOUND just fine since it doesn't directly access lowlevel code
//Does all parsing for a sound
void parse_sound(char* tag, int* idx_dest, char* object_name, parse_sound_flags = PARSE_SOUND_GENERAL_SOUND);
void parse_sound_list(char* tag, SCP_vector<int>& destination, char* object_name, parse_sound_flags = PARSE_SOUND_GENERAL_SOUND);

// this is a callback, so it needs to be a real function
void common_play_highlight_sound();

extern SCP_vector<game_snd> Snds;
extern SCP_vector<game_snd> Snds_iface;

/**
 * symbolic names for misc. game sounds.
 *
 * The order here must match the order in sounds.tbl
 *
 * INSTRUCTIONS FOR ADDING A NEW SOUND:
 * Add interface (ie non-gameplay) sounds to the end of the interface list of sounds.
 * Add gameplay sounds to the correct portion of the gameplay sounds section (ie Misc, Weapons, or Ship).
 *
 * Then add a symbolic name to the appropriate position in the enum below
 * and add an entry to sounds.tbl.  If there is no .wav file for the sound yet, specify sound_hook.wav in sounds.tbl.
 */
enum GameSoundsIndex {
	SND_MISSILE_TRACKING           = 0,  //!< Missle tracking to acquire a lock (looped)
	SND_MISSILE_LOCK               = 1,  //!< Missle lock (non-looping)
	SND_PRIMARY_CYCLE              = 2,  //!< cycle primary weapon
	SND_SECONDARY_CYCLE            = 3,  //!< cycle secondary weapon
	SND_ENGINE                     = 4,  //!< engine sound (as heard in cockpit)
	SND_CARGO_REVEAL               = 5,  //!< cargo revealed
	SND_DEATH_ROLL                 = 6,  //!< ship death roll
	SND_SHIP_EXPLODE_1             = 7,  //!< ship explosion 1
	SND_TARGET_ACQUIRE             = 8,  //!< target acquried
	SND_ENERGY_ADJUST              = 9,  //!< energy level change success
	SND_ENERGY_ADJUST_FAIL         = 10, //!< energy level change fail
	SND_ENERGY_TRANS               = 11, //!< energy transfer success
	SND_ENERGY_TRANS_FAIL          = 12, //!< energy transfer fail
	SND_FULL_THROTTLE              = 13, //!< set full throttle
	SND_ZERO_THROTTLE              = 14, //!< set zero throttle
	SND_THROTTLE_UP                = 15, //!< set 1/3 or 2/3 throttle (up)
	SND_THROTTLE_DOWN              = 16, //!< set 1/3 or 2/3 throttle (down)
	SND_DOCK_APPROACH              = 17, //!< dock approach retros
	SND_DOCK_ATTACH                = 18, //!< dock attach
	SND_DOCK_DETACH                = 19, //!< dock detach
	SND_DOCK_DEPART                = 20, //!< dock depart retros
	SND_ABURN_ENGAGE               = 21, //!< afterburner engage
	SND_ABURN_LOOP                 = 22, //!< afterburner burn sound (looped)
	SND_VAPORIZED                  = 23, //!< Destroyed by a beam (vaporized)
	SND_ABURN_FAIL                 = 24, //!< afterburner fail (no fuel when aburn pressed)
	SND_HEATLOCK_WARN              = 25, //!< heat-seeker launch warning
	SND_OUT_OF_MISSLES             = 26, //!< tried to fire a missle when none are left
	SND_OUT_OF_WEAPON_ENERGY       = 27, //!< tried to fire lasers when not enough energy left
	SND_TARGET_FAIL                = 28, //!< target fail sound (i.e. press targeting key, but nothing happens)
	SND_SQUADMSGING_ON             = 29, //!< squadmate message menu appears
	SND_SQUADMSGING_OFF            = 30, //!< squadmate message menu disappears
	SND_DEBRIS                     = 31, //!< debris sound (persistant, looping)
	SND_SUBSYS_DIE_1               = 32, //!< subsystem gets destroyed on player ship
	SND_MISSILE_START_LOAD         = 33, //!< missle start load (during rearm/repair)
	SND_MISSILE_LOAD               = 34, //!< missle load (during rearm/repair)
	SND_SHIP_REPAIR                = 35, //!< ship is being repaired (during rearm/repair)
	SND_PLAYER_HIT_LASER           = 36, //!< player ship is hit by laser fire
	SND_PLAYER_HIT_MISSILE         = 37, //!< player ship is hit by missile
	SND_CMEASURE_CYCLE             = 38, //!< countermeasure cycle
	SND_SHIELD_HIT                 = 39, //!< shield hit
	SND_SHIELD_HIT_YOU             = 40, //!< player shield is hit
	SND_GAME_MOUSE_CLICK           = 41, //!< mouse click
	SND_ASPECTLOCK_WARN            = 42, //!< aspect launch warning
	SND_SHIELD_XFER_OK             = 43, //!< shield quadrant transfer successful
	SND_ENGINE_WASH                = 44, //!< Engine wash (looped)
	SND_WARP_IN                    = 45, //!< warp hole opening up for arriving
	SND_WARP_OUT                   = 46, //!< warp hole opening up for departing (Same as warp in for now)
	SND_PLAYER_WARP_FAIL           = 47, //!< player warp has failed
	SND_STATIC                     = 48, //!< hud gauge static
	SND_SHIP_EXPLODE_2             = 49, //!< ship explosion 2
	SND_PLAYER_WARP_OUT            = 50, //!< ship is warping out in 3rd person
	SND_SHIP_SHIP_HEAVY            = 51, //!< heavy ship-ship collide sound
	SND_SHIP_SHIP_LIGHT            = 52, //!< light ship-ship collide sound
	SND_SHIP_SHIP_SHIELD           = 53, //!< shield ship-ship collide overlay sound
	SND_THREAT_FLASH               = 54, //!< missile threat indicator flashes
	SND_PROXIMITY_WARNING          = 55, //!< proximity warning (heat seeker)
	SND_PROXIMITY_ASPECT_WARNING   = 56, //!< proximity warning (aspect)
	SND_DIRECTIVE_COMPLETE         = 57, //!< directive complete
	SND_SUBSYS_EXPLODE             = 58, //!< other ship subsystem destroyed
	SND_CAPSHIP_EXPLODE            = 59, //!< captial ship explosion
	SND_CAPSHIP_SUBSYS_EXPLODE     = 60, //!< captial ship subsystem destroyed
	SND_LARGESHIP_WARPOUT          = 61, //!< large ship warps out
	SND_ASTEROID_EXPLODE_LARGE     = 62, //!< large asteroid blows up
	SND_ASTEROID_EXPLODE_SMALL     = 63, //!< small asteroid blows up
	SND_CUE_VOICE                  = 64, //!< sound to indicate voice is about to start
	SND_END_VOICE                  = 65, //!< sound to indicate voice has ended
	SND_CARGO_SCAN                 = 66, //!< cargo scanning (looped)
	SND_WEAPON_FLYBY               = 67, //!< weapon flyby sound
	SND_ASTEROID                   = 68, //!< asteroid sound (persistant, looped)
	SND_CAPITAL_WARP_IN            = 69, //!< capital warp hole opening
	SND_CAPITAL_WARP_OUT           = 70, //!< capital warp hole closing
	SND_ENGINE_LOOP_LARGE          = 71, //!< LARGE engine ambient
	SND_SUBSPACE_LEFT_CHANNEL      = 72, //!< subspace ambient sound (left channel) (looped)
	SND_SUBSPACE_RIGHT_CHANNEL     = 73, //!< subspace ambient sound (right channel) (looped)
	SND_MISSILE_EVADED_POPUP       = 74, //!< "evaded" HUD popup
	SND_ENGINE_LOOP_HUGE           = 75, //!< HUGE engine ambient
	//Weapons section
	SND_LIGHT_LASER_FIRE           = 76, //!< SD-4 Sidearm laser fired
	SND_LIGHT_LASER_IMPACT         = 77, //!< DR-2 Scalpel fired
	SND_HVY_LASER_FIRE             = 78, //!< Flail II fired
	SND_HVY_LASER_IMPACT           = 79, //!< Prometheus R laser fired
	SND_MASSDRV_FIRED              = 80, //!< Prometheus S laser fired
	SND_MASSDRV_IMPACT             = 81, //!< GTW-66 Newton Cannon fired
	SND_FLAIL_FIRED                = 82, //!< UD-8 Kayser Laser fired
	SND_FLAIL_IMPACT               = 83, //!< GTW-19 Circe laser fired
	SND_NEUTRON_FLUX_FIRED         = 84, //!< GTW-83 Lich laser fired
	SND_NEUTRON_FLUX_IMPACT        = 85, //!< Laser impact
	SND_DEBUG_LASER_FIRED          = 86, //!< Subach-HLV Vasudan laser
	SND_ROCKEYE_FIRED              = 87, //!< rockeye missile launch
	SND_MISSILE_IMPACT1            = 88, //!< missile impact 1
	SND_MAG_MISSILE_LAUNCH         = 89, //!< mag pulse missile launch
	SND_FURY_MISSILE_LAUNCH        = 90, //!< fury missile launch
	SND_SHRIKE_MISSILE_LAUNCH      = 91, //!< shrike missile launch
	SND_ANGEL_MISSILE_LAUNCH       = 92, //!< angel fire missile launch
	SND_CLUSTER_MISSILE_LAUNCH     = 93, //!< cluster bomb launch
	SND_CLUSTERB_MISSILE_LAUNCH    = 94, //!< cluster baby bomb launch
	SND_STILETTO_MISSILE_LAUNCH    = 95, //!< stiletto bomb launch
	SND_TSUNAMI_MISSILE_LAUNCH     = 96, //!< tsunami bomb launch
	SND_HARBINGER_MISSILE_LAUNCH   = 97, //!< harbinger bomb launch
	SND_MEGAWOKKA_MISSILE_LAUNCH   = 98, //!< mega wokka launch
	SND_CMEASURE1_LAUNCH           = 99, //!< countermeasure 1 launch
	SND_SHIVAN_LIGHT_LASER_FIRE    = 100,//!< Shivan light laser
	SND_SHOCKWAVE_EXPLODE          = 101,//!< shockwave ignition
	SND_SWARM_MISSILE_LAUNCH       = 102,//!< swarm missile sound
	SND_UNDEFINED_103              = 103,//!< Shivan heavy laser
	SND_UNDEFINED_104              = 104,//!< Vasudan SuperCap engine
	SND_UNDEFINED_105              = 105,//!< Shivan SuperCap engine
	SND_UNDEFINED_106              = 106,//!< Terran SuperCap engine
	SND_UNDEFINED_107              = 107,//!< Vasudan light laser fired
	SND_UNDEFINED_108              = 108,//!< Shivan heavy laser
	SND_SHOCKWAVE_IMPACT           = 109,//!< shockwave impact
	SND_UNDEFINED_110              = 110,//!< TERRAN TURRET 1
	SND_UNDEFINED_111              = 111,//!< TERRAN TURRET 2
	SND_UNDEFINED_112              = 112,//!< VASUDAN TURRET 1
	SND_UNDEFINED_113              = 113,//!< VASUDAN TURRET 2
	SND_UNDEFINED_114              = 114,//!< SHIVAN TURRET 1
	SND_TARG_LASER_LOOP            = 115,//!< targeting laser loop sound
	SND_FLAK_FIRE                  = 116,//!< Flak Gun Launch
	SND_SHIELD_BREAKER             = 117,//!< Flak Gun Impact
	SND_EMP_MISSILE                = 118,//!< EMP Missle
	SND_AUTOCANNON_LOOP            = 119,//!< Escape Pod Drone
	SND_AUTOCANNON_SHOT            = 120,//!< Beam Hit 1
	SND_BEAM_LOOP                  = 121,//!< beam loop
	SND_BEAM_UP                    = 122,//!< beam power up
	SND_BEAM_DOWN                  = 123,//!< beam power down
	SND_BEAM_SHOT                  = 124,//!< Beam shot 1
	SND_BEAM_VAPORIZE              = 125,//!< Beam shot 2
	//Ship Engine Sounds section
	SND_TERRAN_FIGHTER_ENG         = 126,//!< Terran fighter engine
	SND_TERRAN_BOMBER_ENG          = 127,//!< Terran bomber engine
	SND_TERRAN_CAPITAL_ENG         = 128,//!< Terran cruiser engine
	SND_SPECIESB_FIGHTER_ENG       = 129,//!< Vasudan fighter engine
	SND_SPECIESB_BOMBER_ENG        = 130,//!< Vasudan bomber engine
	SND_SPECIESB_CAPITAL_ENG       = 131,//!< Vasudan cruiser engine
	SND_SHIVAN_FIGHTER_ENG         = 132,//!< Shivan fighter engine
	SND_SHIVAN_BOMBER_ENG          = 133,//!< Shivan bomber engine
	SND_SHIVAN_CAPITAL_ENG         = 134,//!< Shivan cruiser engine
	SND_REPAIR_SHIP_ENG            = 135,//!< Repair ship beacon/engine sound
	SND_UNDEFINED_136              = 136,//!< Terran capital engine
	SND_UNDEFINED_137              = 137,//!< Vasudan capital engine
	SND_UNDEFINED_138              = 138,//!< Shivan capital engine
	// Electrical arc sound fx on the debris pieces
	SND_DEBRIS_ARC_01              = 139,//!< 0.10 second spark sound effect
	SND_DEBRIS_ARC_02              = 140,//!< 0.25 second spark sound effect
	SND_DEBRIS_ARC_03              = 141,//!< 0.50 second spark sound effect
	SND_DEBRIS_ARC_04              = 142,//!< 0.75 second spark sound effect
	SND_DEBRIS_ARC_05              = 143,//!< 1.00 second spark sound effect
	// Beam Sounds
	SND_UNDEFINED_144              = 144,//!< LTerSlash beam loop
	SND_UNDEFINED_145              = 145,//!< TerSlash	beam loop
	SND_UNDEFINED_146              = 146,//!< SGreen 	beam loop
	SND_UNDEFINED_147              = 147,//!< BGreen	beem loop
	SND_UNDEFINED_148              = 148,//!< BFGreen	been loop
	SND_UNDEFINED_149              = 149,//!< Antifighter 	beam loop
	SND_UNDEFINED_150              = 150,//!< 1 sec		warm up
	SND_UNDEFINED_151              = 151,//!< 1.5 sec 	warm up
	SND_UNDEFINED_152              = 152,//!< 2.5 sec 	warm up
	SND_UNDEFINED_153              = 153,//!< 3 sec 	warm up
	SND_UNDEFINED_154              = 154,//!< 3.5 sec 	warm up
	SND_UNDEFINED_155              = 155,//!< 5 sec 	warm up
	SND_UNDEFINED_156              = 156,//!< LTerSlash	warm down
	SND_UNDEFINED_157              = 157,//!< TerSlash	warm down
	SND_UNDEFINED_158              = 158,//!< SGreen	warm down
	SND_UNDEFINED_159              = 159,//!< BGreen	warm down
	SND_UNDEFINED_160              = 160,//!< BFGreen	warm down
	SND_UNDEFINED_161              = 161,//!< T_AntiFtr	warm down

	SND_COPILOT                    = 162,//!< copilot (SCP)
	SND_UNDEFINED_163              = 163,//!< (Empty in Retail)
	SND_UNDEFINED_164              = 164,//!< (Empty in Retail)
	SND_UNDEFINED_165              = 165,//!< (Empty in Retail)
	SND_UNDEFINED_166              = 166,//!< (Empty in Retail)
	SND_UNDEFINED_167              = 167,//!< (Empty in Retail)
	SND_UNDEFINED_168              = 168,//!< (Empty in Retail)
	SND_UNDEFINED_169              = 169,//!< (Empty in Retail)
	SND_UNDEFINED_170              = 170,//!< (Empty in Retail)
	SND_UNDEFINED_171              = 171,//!< (Empty in Retail)
	SND_UNDEFINED_172              = 172,//!< (Empty in Retail)
	SND_SUPERNOVA_1                = 173,//!< SuperNova (distant)
	SND_SUPERNOVA_2                = 174,//!< SuperNova (shockwave)
	SND_UNDEFINED_175              = 175,//!< Shivan large engine
	SND_UNDEFINED_176              = 176,//!< Shivan large engine
	SND_UNDEFINED_177              = 177,//!< SRed 		beam loop
	SND_UNDEFINED_178              = 178,//!< LRed		beam loop
	SND_UNDEFINED_179              = 179,//!< Antifighter	beam loop
	SND_LIGHTNING_1                = 180,//!< Thunder 1 sound in neblua
	SND_LIGHTNING_2                = 181,//!< Thunder 2 sound in neblua
	SND_UNDEFINED_182              = 182,//!< 1 sec 	warm up
	SND_UNDEFINED_183              = 183,//!< 1.5 sec 	warm up
	SND_UNDEFINED_184              = 184,//!< 3 sec 	warm up
	SND_UNDEFINED_185              = 185,//!< Shivan Commnode
	SND_UNDEFINED_186              = 186,//!< Volition PirateShip
	SND_UNDEFINED_187              = 187,//!< SRed 		warm down
	SND_UNDEFINED_188              = 188,//!< LRed 		warm down
	SND_UNDEFINED_189              = 189,//!< AntiFtr	warm down
	SND_UNDEFINED_190              = 190,//!< Instellation 1
	SND_UNDEFINED_191              = 191,//!< Instellation 2
	SND_UNDEFINED_192              = 192,//!< (Undefined in Retail)
	SND_UNDEFINED_193              = 193,//!< (Undefined in Retail)
	SND_UNDEFINED_194              = 194,//!< (Undefined in Retail)
	SND_UNDEFINED_195              = 195,//!< (Undefined in Retail)
	SND_UNDEFINED_196              = 196,//!< (Undefined in Retail)
	SND_UNDEFINED_197              = 197,//!< (Undefined in Retail)
	SND_UNDEFINED_198              = 198,//!< (Undefined in Retail)
	SND_UNDEFINED_199              = 199,//!< (Undefined in Retail)

	SND_BALLISTIC_START_LOAD       = 200,//!< (SCP)
	SND_BALLISTIC_LOAD             = 201,//!< (SCP)

	/**
	 * Keep this below all defined enum values
	 */
	MIN_GAME_SOUNDS = 202
};

/**
 * Interface sounds
 */
enum InterfaceSoundsIndex {
	SND_IFACE_MOUSE_CLICK       =0, //!< mouse click
	SND_ICON_PICKUP             =1, //!< pick up a ship icon (Empty in Retail)
	SND_ICON_DROP_ON_WING       =2, //!< drop a ship icon on a wing slot
	SND_ICON_DROP               =3, //!< drop a ship icon back to the list
	SND_SCREEN_MODE_PRESSED     =4, //!< press briefing, ship selection or weapons bar (top-left)
	SND_SWITCH_SCREENS          =5, //!< Switching to a new screen, but not commit
	SND_HELP_PRESSED            =6, //!< help pressed
	SND_COMMIT_PRESSED          =7, //!< commit pressed
	SND_PREV_NEXT_PRESSED       =8, //!< prev/next pressed
	SND_SCROLL                  =9, //!< scroll pressed (and scroll)
	SND_GENERAL_FAIL            =10,//!< general failure sound for any event
	SND_SHIP_ICON_CHANGE        =11,//!< ship animation starts (ie text and ship first appear)
	SND_MAIN_HALL_AMBIENT       =12,//!< ambient sound for the Terran main hall (looping)
	SND_BTN_SLIDE               =13,//!< ambient sound for the Vasudan main hall (looping)
	SND_BRIEF_STAGE_CHG         =14,//!< brief stage change
	SND_BRIEF_STAGE_CHG_FAIL    =15,//!< brief stage change fail
	SND_BRIEF_ICON_SELECT       =16,//!< selet brief icon
	SND_USER_OVER               =17,//!< user_over (mouse over a control)
	SND_USER_SELECT             =18,//!< user_click (mouse selects a control)
	SND_RESET_PRESSED           =19,//!< reset (or similar button) pressed
	SND_BRIEF_TEXT_WIPE         =20,//!< briefing text wipe
	SND_VASUDAN_PA_1            =21,//!< main hall - elevator
	SND_WEAPON_ANIM_START       =22,//!< weapon animation starts
	SND_MAIN_HALL_DOOR_OPEN     =23,//!< door in main hall opens
	SND_MAIN_HALL_DOOR_CLOSE    =24,//!< door in main hall closes
	SND_GLOW_OPEN               =25,//!< glow in main hall opens
	SND_VASUDAN_PA_2            =26,//!< main hall - crane 1
	SND_AMBIENT_MENU            =27,//!< ambient sound for menus off the main hall (looping)
	SND_POPUP_APPEAR            =28,//!< popup dialog box appeared
	SND_POPUP_DISAPPEAR         =29,//!< popup dialog box goes away
	SND_VOICE_SLIDER_CLIP       =30,//!< voice clip played when volume slider changes
	SND_VASUDAN_PA_3            =31,//!< main hall - crane 2
	SND_MAIN_HALL_GET_PEPSI     =32,//!< main hall options - mouse on
	SND_MAIN_HALL_LIFT_UP       =33,//!< main hall options - mouse off
	SND_MAIN_HALL_WELD1         =34,//!< main hall tech room - mouse on
	SND_MAIN_HALL_WELD2         =35,//!< main hall tech room - mouse off
	SND_MAIN_HALL_WELD3         =36,//!< main hall exit open
	SND_MAIN_HALL_WELD4         =37,//!< main hall exit close
	SND_MAIN_HALL_INT1          =38,//!< main hall random intercom 1
	SND_MAIN_HALL_INT2          =39,//!< main hall random intercom 2
	SND_MAIN_HALL_INT3          =40,//!< main hall random intercom 3
	SND_ICON_HIGHLIGHT          =41,//!< spinning highlight in briefing
	SND_BRIEFING_STATIC         =42,//!< static in a briefing stage cut
	SND_MAIN_HALL2_CRANE1_1     =43,//!< main hall campaign - mouse on
	SND_MAIN_HALL2_CRANE1_2     =44,//!< main hall campaign - mouse off
	SND_MAIN_HALL2_CRANE2_1     =45,//!< vasudan hall - hatch open
	SND_MAIN_HALL2_CRANE2_2     =46,//!< vasudan hall - hatch close
	SND_MAIN_HALL2_CAR1         =47,//!< vasudan hall - roll open
	SND_MAIN_HALL2_CAR2         =48,//!< vasudan hall - roll close
	SND_MAIN_HALL2_INT1         =49,//!< vasudan hall - lift up
	SND_MAIN_HALL2_INT2         =50,//!< vasudan hall - lift down
	SND_MAIN_HALL2_INT3         =51,//!< vasudan hall - glow on
	SND_INTERFACE_UNDEFINED_52  =52,//!< vasudan hall - glow off
	SND_INTERFACE_UNDEFINED_53  =53,//!< vasudan hall - skiff loop
	SND_INTERFACE_UNDEFINED_54  =54,//!< vasudan hall - screen on
	SND_INTERFACE_UNDEFINED_55  =55,//!< vasudan hall - screen off
	SND_INTERFACE_UNDEFINED_56  =56,//!< vasudan hall - vasudan greeting
	SND_INTERFACE_UNDEFINED_57  =57,//!< vasudan hall - vasudan bye
	SND_INTERFACE_UNDEFINED_58  =58,//!< vasudan hall - vasudan pa 1
	SND_INTERFACE_UNDEFINED_59  =59,//!< vasudan hall - vasudan pa 2
	SND_INTERFACE_UNDEFINED_60  =60,//!< vasudan hall - vasudan pa 3
	SND_VASUDAN_BUP             =61,//!< bup bup bup-bup bup bup
	SND_INTERFACE_UNDEFINED_62  =62,//!< thankyou
	SND_INTERFACE_UNDEFINED_63  =62,//!< vasudan hall - exit open
	SND_INTERFACE_UNDEFINED_64  =62,//!< vasudan hall - exit close

	/**
	* Keep this below all defined enum values
	*/
	MIN_INTERFACE_SOUNDS        =70 //!< MIN_INTERFACE_SOUNDS
};

#endif
