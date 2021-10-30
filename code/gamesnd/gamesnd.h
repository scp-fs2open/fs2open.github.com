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
#include "utils/id.h"

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
enum class GameSounds {
	MISSILE_TRACKING           = 0,  //!< Missle tracking to acquire a lock (looped)
	MISSILE_LOCK               = 1,  //!< Missle lock (non-looping)
	PRIMARY_CYCLE              = 2,  //!< cycle primary weapon
	SECONDARY_CYCLE            = 3,  //!< cycle secondary weapon
	ENGINE                     = 4,  //!< engine sound (as heard in cockpit)
	CARGO_REVEAL               = 5,  //!< cargo revealed
	DEATH_ROLL                 = 6,  //!< ship death roll
	SHIP_EXPLODE_1             = 7,  //!< ship explosion 1
	TARGET_ACQUIRE             = 8,  //!< target acquried
	ENERGY_ADJUST              = 9,  //!< energy level change success
	ENERGY_ADJUST_FAIL         = 10, //!< energy level change fail
	ENERGY_TRANS               = 11, //!< energy transfer success
	ENERGY_TRANS_FAIL          = 12, //!< energy transfer fail
	FULL_THROTTLE              = 13, //!< set full throttle
	ZERO_THROTTLE              = 14, //!< set zero throttle
	THROTTLE_UP                = 15, //!< set 1/3 or 2/3 throttle (up)
	THROTTLE_DOWN              = 16, //!< set 1/3 or 2/3 throttle (down)
	DOCK_APPROACH              = 17, //!< dock approach retros
	DOCK_ATTACH                = 18, //!< dock attach
	DOCK_DETACH                = 19, //!< dock detach
	DOCK_DEPART                = 20, //!< dock depart retros
	ABURN_ENGAGE               = 21, //!< afterburner engage
	ABURN_LOOP                 = 22, //!< afterburner burn sound (looped)
	VAPORIZED                  = 23, //!< Destroyed by a beam (vaporized)
	ABURN_FAIL                 = 24, //!< afterburner fail (no fuel when aburn pressed)
	HEATLOCK_WARN              = 25, //!< heat-seeker launch warning
	OUT_OF_MISSLES             = 26, //!< tried to fire a missle when none are left
	OUT_OF_WEAPON_ENERGY       = 27, //!< tried to fire lasers when not enough energy left
	TARGET_FAIL                = 28, //!< target fail sound (i.e. press targeting key, but nothing happens)
	SQUADMSGING_ON             = 29, //!< squadmate message menu appears
	SQUADMSGING_OFF            = 30, //!< squadmate message menu disappears
	DEBRIS                     = 31, //!< debris sound (persistant, looping)
	SUBSYS_DIE_1               = 32, //!< subsystem gets destroyed on player ship
	MISSILE_START_LOAD         = 33, //!< missle start load (during rearm/repair)
	MISSILE_LOAD               = 34, //!< missle load (during rearm/repair)
	SHIP_REPAIR                = 35, //!< ship is being repaired (during rearm/repair)
	PLAYER_HIT_LASER           = 36, //!< player ship is hit by laser fire
	PLAYER_HIT_MISSILE         = 37, //!< player ship is hit by missile
	CMEASURE_CYCLE             = 38, //!< countermeasure cycle
	SHIELD_HIT                 = 39, //!< shield hit
	SHIELD_HIT_YOU             = 40, //!< player shield is hit
	GAME_MOUSE_CLICK           = 41, //!< mouse click
	ASPECTLOCK_WARN            = 42, //!< aspect launch warning
	SHIELD_XFER_OK             = 43, //!< shield quadrant transfer successful
	ENGINE_WASH                = 44, //!< Engine wash (looped)
	WARP_IN                    = 45, //!< warp hole opening up for arriving
	WARP_OUT                   = 46, //!< warp hole opening up for departing (Same as warp in for now)
	PLAYER_WARP_FAIL           = 47, //!< player warp has failed
	STATIC                     = 48, //!< hud gauge static
	SHIP_EXPLODE_2             = 49, //!< ship explosion 2
	PLAYER_WARP_OUT            = 50, //!< ship is warping out in 3rd person
	SHIP_SHIP_HEAVY            = 51, //!< heavy ship-ship collide sound
	SHIP_SHIP_LIGHT            = 52, //!< light ship-ship collide sound
	SHIP_SHIP_SHIELD           = 53, //!< shield ship-ship collide overlay sound
	THREAT_FLASH               = 54, //!< missile threat indicator flashes
	PROXIMITY_WARNING          = 55, //!< proximity warning (heat seeker)
	PROXIMITY_ASPECT_WARNING   = 56, //!< proximity warning (aspect)
	DIRECTIVE_COMPLETE         = 57, //!< directive complete
	SUBSYS_EXPLODE             = 58, //!< other ship subsystem destroyed
	CAPSHIP_EXPLODE            = 59, //!< captial ship explosion
	CAPSHIP_SUBSYS_EXPLODE     = 60, //!< captial ship subsystem destroyed
	LARGESHIP_WARPOUT          = 61, //!< large ship warps out
	ASTEROID_EXPLODE_LARGE     = 62, //!< large asteroid blows up
	ASTEROID_EXPLODE_SMALL     = 63, //!< small asteroid blows up
	CUE_VOICE                  = 64, //!< sound to indicate voice is about to start
	END_VOICE                  = 65, //!< sound to indicate voice has ended
	CARGO_SCAN                 = 66, //!< cargo scanning (looped)
	WEAPON_FLYBY               = 67, //!< weapon flyby sound
	ASTEROID                   = 68, //!< asteroid sound (persistant, looped)
	CAPITAL_WARP_IN            = 69, //!< capital warp hole opening
	CAPITAL_WARP_OUT           = 70, //!< capital warp hole closing
	ENGINE_LOOP_LARGE          = 71, //!< LARGE engine ambient
	SUBSPACE_LEFT_CHANNEL      = 72, //!< subspace ambient sound (left channel) (looped)
	SUBSPACE_RIGHT_CHANNEL     = 73, //!< subspace ambient sound (right channel) (looped)
	MISSILE_EVADED_POPUP       = 74, //!< "evaded" HUD popup
	ENGINE_LOOP_HUGE           = 75, //!< HUGE engine ambient
	//Weapons section
		LIGHT_LASER_FIRE           = 76, //!< SD-4 Sidearm laser fired
	LIGHT_LASER_IMPACT         = 77, //!< DR-2 Scalpel fired
	HVY_LASER_FIRE             = 78, //!< Flail II fired
	HVY_LASER_IMPACT           = 79, //!< Prometheus R laser fired
	MASSDRV_FIRED              = 80, //!< Prometheus S laser fired
	MASSDRV_IMPACT             = 81, //!< GTW-66 Newton Cannon fired
	FLAIL_FIRED                = 82, //!< UD-8 Kayser Laser fired
	FLAIL_IMPACT               = 83, //!< GTW-19 Circe laser fired
	NEUTRON_FLUX_FIRED         = 84, //!< GTW-83 Lich laser fired
	NEUTRON_FLUX_IMPACT        = 85, //!< Laser impact
	DEBUG_LASER_FIRED          = 86, //!< Subach-HLV Vasudan laser
	ROCKEYE_FIRED              = 87, //!< rockeye missile launch
	MISSILE_IMPACT1            = 88, //!< missile impact 1
	MAG_MISSILE_LAUNCH         = 89, //!< mag pulse missile launch
	FURY_MISSILE_LAUNCH        = 90, //!< fury missile launch
	SHRIKE_MISSILE_LAUNCH      = 91, //!< shrike missile launch
	ANGEL_MISSILE_LAUNCH       = 92, //!< angel fire missile launch
	CLUSTER_MISSILE_LAUNCH     = 93, //!< cluster bomb launch
	CLUSTERB_MISSILE_LAUNCH    = 94, //!< cluster baby bomb launch
	STILETTO_MISSILE_LAUNCH    = 95, //!< stiletto bomb launch
	TSUNAMI_MISSILE_LAUNCH     = 96, //!< tsunami bomb launch
	HARBINGER_MISSILE_LAUNCH   = 97, //!< harbinger bomb launch
	MEGAWOKKA_MISSILE_LAUNCH   = 98, //!< mega wokka launch
	CMEASURE1_LAUNCH           = 99, //!< countermeasure 1 launch
	SHIVAN_LIGHT_LASER_FIRE    = 100,//!< Shivan light laser
	SHOCKWAVE_EXPLODE          = 101,//!< shockwave ignition
	SWARM_MISSILE_LAUNCH       = 102,//!< swarm missile sound
	UNDEFINED_103              = 103,//!< Shivan heavy laser
	UNDEFINED_104              = 104,//!< Vasudan SuperCap engine
	UNDEFINED_105              = 105,//!< Shivan SuperCap engine
	UNDEFINED_106              = 106,//!< Terran SuperCap engine
	UNDEFINED_107              = 107,//!< Vasudan light laser fired
	UNDEFINED_108              = 108,//!< Shivan heavy laser
	SHOCKWAVE_IMPACT           = 109,//!< shockwave impact
	UNDEFINED_110              = 110,//!< TERRAN TURRET 1
	UNDEFINED_111              = 111,//!< TERRAN TURRET 2
	UNDEFINED_112              = 112,//!< VASUDAN TURRET 1
	UNDEFINED_113              = 113,//!< VASUDAN TURRET 2
	UNDEFINED_114              = 114,//!< SHIVAN TURRET 1
	TARG_LASER_LOOP            = 115,//!< targeting laser loop sound
	FLAK_FIRE                  = 116,//!< Flak Gun Launch
	SHIELD_BREAKER             = 117,//!< Flak Gun Impact
	EMP_MISSILE                = 118,//!< EMP Missle
	AUTOCANNON_LOOP            = 119,//!< Escape Pod Drone
	AUTOCANNON_SHOT            = 120,//!< Beam Hit 1
	BEAM_LOOP                  = 121,//!< beam loop
	BEAM_UP                    = 122,//!< beam power up
	BEAM_DOWN                  = 123,//!< beam power down
	BEAM_SHOT                  = 124,//!< Beam shot 1
	BEAM_VAPORIZE              = 125,//!< Beam shot 2
	//Ship Engine Sounds section
		TERRAN_FIGHTER_ENG         = 126,//!< Terran fighter engine
	TERRAN_BOMBER_ENG          = 127,//!< Terran bomber engine
	TERRAN_CAPITAL_ENG         = 128,//!< Terran cruiser engine
	SPECIESB_FIGHTER_ENG       = 129,//!< Vasudan fighter engine
	SPECIESB_BOMBER_ENG        = 130,//!< Vasudan bomber engine
	SPECIESB_CAPITAL_ENG       = 131,//!< Vasudan cruiser engine
	SHIVAN_FIGHTER_ENG         = 132,//!< Shivan fighter engine
	SHIVAN_BOMBER_ENG          = 133,//!< Shivan bomber engine
	SHIVAN_CAPITAL_ENG         = 134,//!< Shivan cruiser engine
	REPAIR_SHIP_ENG            = 135,//!< Repair ship beacon/engine sound
	UNDEFINED_136              = 136,//!< Terran capital engine
	UNDEFINED_137              = 137,//!< Vasudan capital engine
	UNDEFINED_138              = 138,//!< Shivan capital engine
	// Electrical arc sound fx on the debris pieces
		DEBRIS_ARC_01              = 139,//!< 0.10 second spark sound effect
	DEBRIS_ARC_02              = 140,//!< 0.25 second spark sound effect
	DEBRIS_ARC_03              = 141,//!< 0.50 second spark sound effect
	DEBRIS_ARC_04              = 142,//!< 0.75 second spark sound effect
	DEBRIS_ARC_05              = 143,//!< 1.00 second spark sound effect
	// Beam Sounds
		UNDEFINED_144              = 144,//!< LTerSlash beam loop
	UNDEFINED_145              = 145,//!< TerSlash	beam loop
	UNDEFINED_146              = 146,//!< SGreen 	beam loop
	UNDEFINED_147              = 147,//!< BGreen	beem loop
	UNDEFINED_148              = 148,//!< BFGreen	been loop
	UNDEFINED_149              = 149,//!< Antifighter 	beam loop
	UNDEFINED_150              = 150,//!< 1 sec		warm up
	UNDEFINED_151              = 151,//!< 1.5 sec 	warm up
	UNDEFINED_152              = 152,//!< 2.5 sec 	warm up
	UNDEFINED_153              = 153,//!< 3 sec 	warm up
	UNDEFINED_154              = 154,//!< 3.5 sec 	warm up
	UNDEFINED_155              = 155,//!< 5 sec 	warm up
	UNDEFINED_156              = 156,//!< LTerSlash	warm down
	UNDEFINED_157              = 157,//!< TerSlash	warm down
	UNDEFINED_158              = 158,//!< SGreen	warm down
	UNDEFINED_159              = 159,//!< BGreen	warm down
	UNDEFINED_160              = 160,//!< BFGreen	warm down
	UNDEFINED_161              = 161,//!< T_AntiFtr	warm down

	COPILOT                    = 162,//!< copilot (SCP)
	UNDEFINED_163              = 163,//!< (Empty in Retail)
	UNDEFINED_164              = 164,//!< (Empty in Retail)
	UNDEFINED_165              = 165,//!< (Empty in Retail)
	UNDEFINED_166              = 166,//!< (Empty in Retail)
	UNDEFINED_167              = 167,//!< (Empty in Retail)
	UNDEFINED_168              = 168,//!< (Empty in Retail)
	UNDEFINED_169              = 169,//!< (Empty in Retail)
	UNDEFINED_170              = 170,//!< (Empty in Retail)
	UNDEFINED_171              = 171,//!< (Empty in Retail)
	UNDEFINED_172              = 172,//!< (Empty in Retail)
	SUPERNOVA_1                = 173,//!< SuperNova (distant)
	SUPERNOVA_2                = 174,//!< SuperNova (shockwave)
	UNDEFINED_175              = 175,//!< Shivan large engine
	UNDEFINED_176              = 176,//!< Shivan large engine
	UNDEFINED_177              = 177,//!< SRed 		beam loop
	UNDEFINED_178              = 178,//!< LRed		beam loop
	UNDEFINED_179              = 179,//!< Antifighter	beam loop
	LIGHTNING_1                = 180,//!< Thunder 1 sound in neblua
	LIGHTNING_2                = 181,//!< Thunder 2 sound in neblua
	UNDEFINED_182              = 182,//!< 1 sec 	warm up
	UNDEFINED_183              = 183,//!< 1.5 sec 	warm up
	UNDEFINED_184              = 184,//!< 3 sec 	warm up
	UNDEFINED_185              = 185,//!< Shivan Commnode
	UNDEFINED_186              = 186,//!< Volition PirateShip
	UNDEFINED_187              = 187,//!< SRed 		warm down
	UNDEFINED_188              = 188,//!< LRed 		warm down
	UNDEFINED_189              = 189,//!< AntiFtr	warm down
	UNDEFINED_190              = 190,//!< Instellation 1
	UNDEFINED_191              = 191,//!< Instellation 2
	UNDEFINED_192              = 192,//!< (Undefined in Retail)
	UNDEFINED_193              = 193,//!< (Undefined in Retail)
	UNDEFINED_194              = 194,//!< (Undefined in Retail)
	UNDEFINED_195              = 195,//!< (Undefined in Retail)
	UNDEFINED_196              = 196,//!< (Undefined in Retail)
	UNDEFINED_197              = 197,//!< (Undefined in Retail)
	UNDEFINED_198              = 198,//!< (Undefined in Retail)
	UNDEFINED_199              = 199,//!< (Undefined in Retail)

	BALLISTIC_START_LOAD       = 200,//!< (SCP)
	BALLISTIC_LOAD             = 201,//!< (SCP)

	/**
	 * Keep this below all defined enum values
	 */
		MIN_GAME_SOUNDS = 202
};

/**
 * Interface sounds
 */
enum class InterfaceSounds {
	IFACE_MOUSE_CLICK       =0, //!< mouse click
	ICON_PICKUP             =1, //!< pick up a ship icon (Empty in Retail)
	ICON_DROP_ON_WING       =2, //!< drop a ship icon on a wing slot
	ICON_DROP               =3, //!< drop a ship icon back to the list
	SCREEN_MODE_PRESSED     =4, //!< press briefing, ship selection or weapons bar (top-left)
	SWITCH_SCREENS          =5, //!< Switching to a new screen, but not commit
	HELP_PRESSED            =6, //!< help pressed
	COMMIT_PRESSED          =7, //!< commit pressed
	PREV_NEXT_PRESSED       =8, //!< prev/next pressed
	SCROLL                  =9, //!< scroll pressed (and scroll)
	GENERAL_FAIL            =10,//!< general failure sound for any event
	SHIP_ICON_CHANGE        =11,//!< ship animation starts (ie text and ship first appear)
	MAIN_HALL_AMBIENT       =12,//!< ambient sound for the Terran main hall (looping)
	BTN_SLIDE               =13,//!< ambient sound for the Vasudan main hall (looping)
	BRIEF_STAGE_CHG         =14,//!< brief stage change
	BRIEF_STAGE_CHG_FAIL    =15,//!< brief stage change fail
	BRIEF_ICON_SELECT       =16,//!< selet brief icon
	USER_OVER               =17,//!< user_over (mouse over a control)
	USER_SELECT             =18,//!< user_click (mouse selects a control)
	RESET_PRESSED           =19,//!< reset (or similar button) pressed
	BRIEF_TEXT_WIPE         =20,//!< briefing text wipe
	VASUDAN_PA_1            =21,//!< main hall - elevator
	WEAPON_ANIM_START       =22,//!< weapon animation starts
	MAIN_HALL_DOOR_OPEN     =23,//!< door in main hall opens
	MAIN_HALL_DOOR_CLOSE    =24,//!< door in main hall closes
	GLOW_OPEN               =25,//!< glow in main hall opens
	VASUDAN_PA_2            =26,//!< main hall - crane 1
	AMBIENT_MENU            =27,//!< ambient sound for menus off the main hall (looping)
	POPUP_APPEAR            =28,//!< popup dialog box appeared
	POPUP_DISAPPEAR         =29,//!< popup dialog box goes away
	VOICE_SLIDER_CLIP       =30,//!< voice clip played when volume slider changes
	VASUDAN_PA_3            =31,//!< main hall - crane 2
	MAIN_HALL_GET_PEPSI     =32,//!< main hall options - mouse on
	MAIN_HALL_LIFT_UP       =33,//!< main hall options - mouse off
	MAIN_HALL_WELD1         =34,//!< main hall tech room - mouse on
	MAIN_HALL_WELD2         =35,//!< main hall tech room - mouse off
	MAIN_HALL_WELD3         =36,//!< main hall exit open
	MAIN_HALL_WELD4         =37,//!< main hall exit close
	MAIN_HALL_INT1          =38,//!< main hall random intercom 1
	MAIN_HALL_INT2          =39,//!< main hall random intercom 2
	MAIN_HALL_INT3          =40,//!< main hall random intercom 3
	ICON_HIGHLIGHT          =41,//!< spinning highlight in briefing
	BRIEFING_STATIC         =42,//!< static in a briefing stage cut
	MAIN_HALL2_CRANE1_1     =43,//!< main hall campaign - mouse on
	MAIN_HALL2_CRANE1_2     =44,//!< main hall campaign - mouse off
	MAIN_HALL2_CRANE2_1     =45,//!< vasudan hall - hatch open
	MAIN_HALL2_CRANE2_2     =46,//!< vasudan hall - hatch close
	MAIN_HALL2_CAR1         =47,//!< vasudan hall - roll open
	MAIN_HALL2_CAR2         =48,//!< vasudan hall - roll close
	MAIN_HALL2_INT1         =49,//!< vasudan hall - lift up
	MAIN_HALL2_INT2         =50,//!< vasudan hall - lift down
	MAIN_HALL2_INT3         =51,//!< vasudan hall - glow on
	INTERFACE_UNDEFINED_52  =52,//!< vasudan hall - glow off
	INTERFACE_UNDEFINED_53  =53,//!< vasudan hall - skiff loop
	INTERFACE_UNDEFINED_54  =54,//!< vasudan hall - screen on
	INTERFACE_UNDEFINED_55  =55,//!< vasudan hall - screen off
	INTERFACE_UNDEFINED_56  =56,//!< vasudan hall - vasudan greeting
	INTERFACE_UNDEFINED_57  =57,//!< vasudan hall - vasudan bye
	INTERFACE_UNDEFINED_58  =58,//!< vasudan hall - vasudan pa 1
	INTERFACE_UNDEFINED_59  =59,//!< vasudan hall - vasudan pa 2
	INTERFACE_UNDEFINED_60  =60,//!< vasudan hall - vasudan pa 3
	VASUDAN_BUP             =61,//!< bup bup bup-bup bup bup
	INTERFACE_UNDEFINED_62  =62,//!< thankyou
	INTERFACE_UNDEFINED_63  =62,//!< vasudan hall - exit open
	INTERFACE_UNDEFINED_64  =62,//!< vasudan hall - exit close

	/**
	* Keep this below all defined enum values
	*/
		MIN_INTERFACE_SOUNDS        =70 //!< MIN_INTERFACE_SOUNDS
};

// These two id types are defined as sublcasses so that type safe implicit conversions are possible from the predefined
// sound enum values

struct gamesnd_id_tag{};
/**
 * @brief A game sound handle type
 *
 * This allows implicit conversions from the GameSounds enum class.
 */
class gamesnd_id : public util::ID<gamesnd_id_tag, int, -1> {
 public:
	/*implicit*/ gamesnd_id(GameSounds snd) : util::ID<gamesnd_id_tag, int, -1>(static_cast<int>(snd)) {}
	explicit gamesnd_id(int val) : ID(val) {
	}
	gamesnd_id() {
	}
};

struct interface_snd_tag{};
/**
 * @brief A interface sound handle type
 *
 * This allows implicit conversions from the InterfaceSounds enum class.
 */
class interface_snd_id : public util::ID<interface_snd_tag, int, -1> {
 public:
	/*implicit*/ interface_snd_id(InterfaceSounds snd) : util::ID<interface_snd_tag, int, -1>(static_cast<int>(snd)) {}
	interface_snd_id() {
	}
	explicit interface_snd_id(int val) : ID(val) {
	}
};

void gamesnd_parse_soundstbl();	// Loads in general game sounds from sounds.tbl
void gamesnd_close();	// close out gamesnd... only call from game_shutdown()!
void gamesnd_load_gameplay_sounds();
void gamesnd_unload_gameplay_sounds();
void gamesnd_load_interface_sounds();
void gamesnd_unload_interface_sounds();
void gamesnd_preload_common_sounds();
void gamesnd_load_gameplay_sounds();
void gamesnd_unload_gameplay_sounds();
void gamesnd_play_iface(interface_snd_id n);
void gamesnd_play_error_beep();
gamesnd_id gamesnd_get_by_name(const char* name);
interface_snd_id gamesnd_get_by_iface_name(const char* name);
gamesnd_id gamesnd_get_by_tbl_index(int index);
interface_snd_id gamesnd_get_by_iface_tbl_index(int index);

// This should handle NO_SOUND just fine since it doesn't directly access lowlevel code
// Does all parsing for a sound.  Returns true if a sound was successfully parsed.
bool parse_game_sound(const char* tag, gamesnd_id* idx_dest);

gamesnd_id parse_game_sound_inline();

void parse_iface_sound(const char* tag, interface_snd_id* idx_dest);
void parse_iface_sound_list(const char* tag, SCP_vector<interface_snd_id>& destination, const char* object_name, bool scp_list = false);

// this is a callback, so it needs to be a real function
void common_play_highlight_sound();

/**
 * @brief Gets a pointer to the game sound with the specified handle
 * @param handle The sound handle
 * @return The game sound handle
 */
game_snd* gamesnd_get_game_sound(gamesnd_id handle);

/**
 * @brief Gets a pointer to the interface sound with the specified handle
 * @param handle The sound handle
 * @return The interface sound handle
 */
game_snd* gamesnd_get_interface_sound(interface_snd_id handle);

/**
 * @brief Checks if the given sound handle is a valid game sound handle
 * @param sound The handle to check
 * @return @c true if the handle is valid
 */
bool gamesnd_game_sound_valid(gamesnd_id sound);

/**
* @brief Checks if the given sound handle can be loaded
*/
bool gamesnd_game_sound_try_load(gamesnd_id sound);

/**
 * @brief Checks if the given sound handle is a valid interface sound handle
 * @param sound The handle to check
 * @return @c true if the handle is valid
 */
bool gamesnd_interface_sound_valid(interface_snd_id sound);

/**
 * @brief Determines the maximum time this game sound may take to be played.
 *
 * @details Use this instead of snd_get_duration since a game sound may have multiple sounds with different durations.
 * This function also accounts for pitch changes which also changes the length of a sound.
 *
 * @param gs The game sound to check
 * @return The length of the sound in milliseconds
 */
float gamesnd_get_max_duration(game_snd* gs);

/**
 * @brief Chooses a sound entry for the specified game sound according to the rules specified by the game data
 *
 * @warning The returned pointer is temporary and may not be valid forever. Do no store this pointer anywhere!
 *
 * @param gs The game sound to choose the entry from
 * @return The chosen entry
 */
game_snd_entry* gamesnd_choose_entry(game_snd* gs);

#endif
