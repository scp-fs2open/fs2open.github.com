/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef CONTROLS_CONFIG_H
#define CONTROLS_CONFIG_H

#include "globalincs/pstypes.h"

#define CONTROL_CONFIG_XSTR	507

/*!
 * These are used to index a corresponding joystick axis value from an array.
 * Currently only used by ::Axis_map_to[] and ::Axis_map_to_defaults[]
 */
enum Joy_axis_index {
	JOY_X_AXIS		=0,
	JOY_Y_AXIS,
	JOY_Z_AXIS,
	JOY_RX_AXIS,
	JOY_RY_AXIS,
	JOY_RZ_AXIS
};

/*!
 * Controller index enumeration
 * @details For use with hardcoded bindings and scripting API to allow human-readable translation
 * @note These enums are hardcoded so that an int value of 0 would be JOY0
 */
enum CID : short {
	CID_NONE     = -3,	// No device bound
	CID_KEYBOARD = -2,	// button belongs to keyboard
	CID_MOUSE    = -1,  // to mouse
	CID_JOY0     =  0,  // to Joy0
	CID_JOY1     =  1,  // to Joy1 (Throttle?)
	CID_JOY2     =  2,  // to Joy2 (Pedals?)
	CID_JOY3     =  3   // to Joy3 (Head tracker?)
	                    // Any additional joystick is an int >3
};

/*!
 * These are used to index a corresponding (analog) action, namely controlling the orientation angles and throttle.
 */
enum Joy_axis_action_index {
	JOY_HEADING_AXIS	=0,	//
	JOY_PITCH_AXIS,
	JOY_BANK_AXIS,
	JOY_ABS_THROTTLE_AXIS,
	JOY_REL_THROTTLE_AXIS,

	/*!
	 * This must always be below the last defined item
	 */
	NUM_JOY_AXIS_ACTIONS			//!< The total number of actions an axis may map to
};

/*!
 * Joystick action modes
 * note: should this be in joy.h?
 */
enum Joy_axis_action_mode {
	JAAM_ABS,       //!< Absolute mode.  Axis position = output value
	JAAM_REL,       //!< Relative mode.  Axis postion away from its center adds or subtracts an output value register.
	JAAM_BTN_NEG,   //!< Button mode, negative side.  Axis position in the negative side will trigger a button action
	JAAM_BTN_POS,   //!< Button mode, positive side.  Axis position in the positive side will trigger a button action
};

/*!
 * Control Configuration Types. Namely differ in how the control is activated
 */
enum CC_type {
	CC_TYPE_TRIGGER			=0,		//!< A normal, one-shot type control that is activated when a key is or button is pressed
	CC_TYPE_CONTINUOUS				//!< A continous control that is activated as long as the key or button is held down
};

/*!
 * All available actions
 * This is the value of the id field in config_item
 * The first group of items are ship targeting.
 *
 * Note: Do not adjust the order or numeric value
 */
enum IoActionId  {
	TARGET_NEXT										=0,		//!< target next
	TARGET_PREV										=1,		//!< target previous
	TARGET_NEXT_CLOSEST_HOSTILE						=2,		//!< target the next hostile target
	TARGET_PREV_CLOSEST_HOSTILE						=3,		//!< target the previous closest hostile
	TOGGLE_AUTO_TARGETING							=4,		//!< toggle auto-targeting
	TARGET_NEXT_CLOSEST_FRIENDLY					=5,		//!< target the next friendly ship
	TARGET_PREV_CLOSEST_FRIENDLY					=6,		//!< target the closest friendly ship
	TARGET_SHIP_IN_RETICLE							=7,		//!< target ship closest to center of reticle
	TARGET_CLOSEST_SHIP_ATTACKING_TARGET			=8,		//!< target the closest ship attacking current target
	TARGET_LAST_TRANMISSION_SENDER					=9,		//!< TARGET_LAST_TRANMISSION_SENDER
	STOP_TARGETING_SHIP								=10,	//!< stop targeting ship

	//!< @n
	//!< Targeting a Ship's Subsystem
	//!< ------------------------------
	TARGET_SUBOBJECT_IN_RETICLE						=11,	//!< target ships subsystem in reticle
	TARGET_NEXT_SUBOBJECT							=12,	//!< target next subsystem on current target
	TARGET_PREV_SUBOBJECT							=13,	//!< TARGET_PREV_SUBOBJECT
	STOP_TARGETING_SUBSYSTEM						=14,	//!< stop targeting subsystems on ship

	//!< @n
	//!< Speed Matching
	//!< ----------------
	MATCH_TARGET_SPEED								=15,	//!< match target speed
	TOGGLE_AUTO_MATCH_TARGET_SPEED					=16,	//!< toggle auto-match target speed

	//!< @n
	//!< Weapons
	//!< ---------
	FIRE_PRIMARY									=17,	//!< FIRE_PRIMARY
	FIRE_SECONDARY									=18,	//!< FIRE_SECONDARY
	CYCLE_NEXT_PRIMARY								=19,	//!< cycle to next primary weapon
	CYCLE_PREV_PRIMARY								=20,	//!< cycle to previous primary weapon
	CYCLE_SECONDARY									=21,	//!< cycle to next secondary weapon
	CYCLE_NUM_MISSLES								=22,	//!< cycle number of missiles fired from secondary bank
	LAUNCH_COUNTERMEASURE							=23,	//!< LAUNCH_COUNTERMEASURE

	//!< @n
	//!< Controls
	//!< ----------
	FORWARD_THRUST									=24,	//!< FORWARD_THRUST
	REVERSE_THRUST									=25,	//!< REVERSE_THRUST
	BANK_LEFT										=26,	//!< BANK_LEFT
	BANK_RIGHT										=27,	//!< BANK_RIGHT
	PITCH_FORWARD									=28,	//!< PITCH_FORWARD
	PITCH_BACK										=29,	//!< PITCH_BACK
	YAW_LEFT										=30,	//!< YAW_LEFT
	YAW_RIGHT										=31,	//!< YAW_RIGHT

	//!< @n
	//!< Throttle Control
	//!< ------------------
	ZERO_THROTTLE									=32,	//!< ZERO_THROTTLE
	MAX_THROTTLE									=33,	//!< MAX_THROTTLE
	ONE_THIRD_THROTTLE								=34,	//!< ONE_THIRD_THROTTLE
	TWO_THIRDS_THROTTLE								=35,	//!< TWO_THIRDS_THROTTLE
	PLUS_5_PERCENT_THROTTLE							=36,	//!< PLUS_5_PERCENT_THROTTLE
	MINUS_5_PERCENT_THROTTLE						=37,	//!< MINUS_5_PERCENT_THROTTLE

	//!< @n
	//!< Squadmate Messaging Keys
	//!< --------------------------
	ATTACK_MESSAGE									=38,	//!< wingman message: attack current target
	DISARM_MESSAGE									=39,	//!< wingman message: disarm current target
	DISABLE_MESSAGE									=40,	//!< wingman message: disable current target
	ATTACK_SUBSYSTEM_MESSAGE						=41,	//!< wingman message: disable current target
	CAPTURE_MESSAGE									=42,	//!< wingman message: capture current target
	ENGAGE_MESSAGE									=43,	//!< wingman message: engage enemy
	FORM_MESSAGE									=44,	//!< wingman message: form on my wing
	IGNORE_MESSAGE									=45,	//!< IGNORE_MESSAGE
	PROTECT_MESSAGE									=46,	//!< wingman message: protect current target
	COVER_MESSAGE									=47,	//!< wingman message: cover me
	WARP_MESSAGE									=48,	//!< wingman message: warp out
	REARM_MESSAGE									=49,	//!< REARM_MESSAGE
	TARGET_CLOSEST_SHIP_ATTACKING_SELF				=50,	//!< target closest ship that is attacking player

	//!< @n
	//!< Views
	//!< -------
	VIEW_CHASE										=51,	//!< VIEW_CHASE
	VIEW_EXTERNAL									=52,	//!< VIEW_EXTERNAL
	VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK				=53,	//!< VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK
	VIEW_SLEW										=54,	//!< VIEW_SLEW
	VIEW_OTHER_SHIP									=55,	//!< VIEW_OTHER_SHIP
	VIEW_DIST_INCREASE								=56,	//!< VIEW_DIST_INCREASE
	VIEW_DIST_DECREASE								=57,	//!< VIEW_DIST_DECREASE
	VIEW_CENTER										=58,	//!< VIEW_CENTER
	PADLOCK_UP										=59,	//!< PADLOCK_UP
	PADLOCK_DOWN									=60,	//!< PADLOCK_DOWN
	PADLOCK_LEFT									=61,	//!< PADLOCK_LEFT
	PADLOCK_RIGHT									=62,	//!< PADLOCK_RIGHT

	//!< @n
	//!< Misc Controls 1
	//!< -----------------
	RADAR_RANGE_CYCLE								=63,	//!< cycle to next radar range
	SQUADMSG_MENU									=64,	//!< toggle the squadmate messaging menu
	SHOW_GOALS										=65,	//!< show the mission goals screen
	END_MISSION										=66,	//!< end the mission
	TARGET_TARGETS_TARGET							=67,	//!< target your target's target
	AFTERBURNER										=68,	//!< AFTERBURNER

	//!< @n
	//!< ETS
	//!< -----
	INCREASE_WEAPON									=69,	//!< increase weapon recharge rate
	DECREASE_WEAPON									=70,	//!< decrease weapon recharge rate
	INCREASE_SHIELD									=71,	//!< increase shield recharge rate
	DECREASE_SHIELD									=72,	//!< decrease shield recharge rate
	INCREASE_ENGINE									=73,	//!< increase energy to engines
	DECREASE_ENGINE									=74,	//!< decrease energy to engines
	ETS_EQUALIZE									=75,	//!< equalize recharge rates
	SHIELD_EQUALIZE									=76,	//!< equalize shield energy to all quadrants
	SHIELD_XFER_TOP									=77,	//!< transfer shield energy to front
	SHIELD_XFER_BOTTOM								=78,	//!< transfer shield energy to rear
	SHIELD_XFER_LEFT								=79,	//!< transfer shield energy to left
	SHIELD_XFER_RIGHT								=80,	//!< transfer shield energy to right
	XFER_SHIELD										=81,	//!< transfer energy to shield from weapons
	XFER_LASER										=82,	//!< transfer energy to weapons from shield

	//!< @n
	//!< Misc Controls 2
	//!< -----------------
	GLIDE_WHEN_PRESSED								=83, 	//!< GLIDE_WHEN_PRESSED
													//!< @remark Backslash -- this was a convenient place for Glide When Pressed, since Show Damage Popup isn't used
	BANK_WHEN_PRESSED								=84,	//!< BANK_WHEN_PRESSED
	SHOW_NAVMAP										=85,	//!< SHOW_NAVMAP
	ADD_REMOVE_ESCORT								=86,	//!< ADD_REMOVE_ESCORT
	ESCORT_CLEAR									=87,	//!< ESCORT_CLEAR

	TARGET_NEXT_ESCORT_SHIP							=88,	//!< TARGET_NEXT_ESCORT_SHIP
	TARGET_CLOSEST_REPAIR_SHIP						=89,	//!< target the closest repair ship
	TARGET_NEXT_UNINSPECTED_CARGO					=90,	//!< TARGET_NEXT_UNINSPECTED_CARGO
	TARGET_PREV_UNINSPECTED_CARGO					=91,	//!< TARGET_PREV_UNINSPECTED_CARGO
	TARGET_NEWEST_SHIP								=92,	//!< TARGET_NEWEST_SHIP

	TARGET_NEXT_LIVE_TURRET							=93, 	//!< TARGET_NEXT_LIVE_TURRET
	TARGET_PREV_LIVE_TURRET							=94,	//!< TARGET_PREV_LIVE_TURRET

	TARGET_NEXT_BOMB								=95,	//!< TARGET_NEXT_BOMB
	TARGET_PREV_BOMB								=96,	//!< TARGET_PREV_BOMB

	//!< @n
	//!< Multiplayer messaging keys
	//!< ----------------------------
	MULTI_MESSAGE_ALL								=97,	//!< message all netplayers
	MULTI_MESSAGE_FRIENDLY							=98,	//!< message all friendlies
	MULTI_MESSAGE_HOSTILE							=99,	//!< message all hostiles
	MULTI_MESSAGE_TARGET							=100,	//!< message targeted ship (if player)

	//!< @n
	//!< Multiplayer misc keys
	//!< -----------------------
	MULTI_OBSERVER_ZOOM_TO							=101,	//!< if i'm an observer, zoom to my targeted object

	TIME_SPEED_UP									=102,	//!< TIME_SPEED_UP
	TIME_SLOW_DOWN									=103,	//!< TIME_SLOW_DOWN

	TOGGLE_HUD_CONTRAST								=104,	//!< toggle between high and low HUD contrast

	MULTI_TOGGLE_NETINFO							=105,	//!< toggle network info

	MULTI_SELF_DESTRUCT								=106,	//!< self destruct (multiplayer only)

	//!< @n
	//!< Misc Controls 3
	//!< -----------------
	TOGGLE_HUD										=107,	//!< TOGGLE_HUD
	RIGHT_SLIDE_THRUST								=108,	//!< RIGHT_SLIDE_THRUST
	LEFT_SLIDE_THRUST								=109,	//!< LEFT_SLIDE_THRUST
	UP_SLIDE_THRUST									=110,	//!< UP_SLIDE_THRUST
	DOWN_SLIDE_THRUST								=111,	//!< DOWN_SLIDE_THRUST
	HUD_TARGETBOX_TOGGLE_WIREFRAME					=112,	//!< HUD_TARGETBOX_TOGGLE_WIREFRAME
	VIEW_TOPDOWN									=113,	//!< VIEW_TOPDOWN
	VIEW_TRACK_TARGET								=114,	//!< VIEW_TRACK_TARGET (Switfty) Toggle for VM_Track

	//!< @n
	//!< AutoPilot - Kazan
	//!< -------------------
	AUTO_PILOT_TOGGLE								=115,	//!< Autopilot key control
	NAV_CYCLE										=116,	//!< NAV_CYCLE

	//!< @n
	//!< Gliding
	//!< ---------
	TOGGLE_GLIDING									=117,	//!< TOGGLE_GLIDING

	//!< @n
	//!< Additional weapon controls
	//!< ----------------------------
	CYCLE_PRIMARY_WEAPON_SEQUENCE					=118,	//!< cycle num primaries to fire at once

	//!< @n
	//!< Custom control slots
	//!< ----------------------------
	CUSTOM_CONTROL_1								= 119,
	CUSTOM_CONTROL_2								= 120,
	CUSTOM_CONTROL_3								= 121,
	CUSTOM_CONTROL_4								= 122,
	CUSTOM_CONTROL_5								= 123,

	/*!
	 * This must always be below the last defined item
	 */
	CCFG_MAX                                  //!<  The total number of defined control actions (or last define + 1)
};

class CCB;

/*!
 *  A singular button binding
 */
class CC_bind {
public:
	CID cid   = CID_NONE;   //!< Which controller this belongs to
	short btn = -1;         //!< Which button to index; If cid == CID_KEYBOARD, this is a key hash.

public:
	CC_bind() = default;
	CC_bind(const CC_bind &A) = default;

	CC_bind(CID _cid, short _btn) : cid(_cid), btn(_btn) {};

	CC_bind& operator=(const CC_bind &A);

	/*!
	 * Checks if this CC_bind is equal to the given CC_bind
	 */
	bool operator==(const CC_bind &B) const;

	/*!
	 * Checks if this CC_bind is equal to either in the pair
	 */
	bool operator==(const CCB &pair) const;

	/*!
	 * Checks if this CC_bind is not equal to the given CC_bind
	 */
	bool operator!=(const CC_bind &B) const;

	/*!
	 * Checks if this CC_bind is not equal to either in the pair
	 */
	bool operator!=(const CCB &pair) const;

	/*!
	 * Clears the binding
	 */
	void clear();

	/*!
	 * True if not bound
	 */
	bool empty() const;

	/*!
	 * Returns a human-readable string of this binding
	 */
	SCP_string textify() const;
};

/*!
 * A pair of bindings.
 * @note Please don't set the bindings directly, use ::take() instead.
 */
class CCB {
public:
	CC_bind first;  // The primary binding
	CC_bind second; // The secondary binding

public:
	CCB() = default;
	CCB(const CCB& A) = default;

	/*!
	 * Returns true if nothing is bound
	 */
	bool empty() const;

	/*!
	 * Takes a given binding
	 *
	 * @param[in] A     The bind to take
	 * @param[in] order 0 is primary, 1 is secondary, -1 is overwrite existing bind (if any)
	 *
	 * @note Should there be a binding to the same controller as the given binding, it is cleared
	 */
	void take(CC_bind A, int order);

	/*!
	 * Clears both bindings
	 */
	void clear();

	/*!
	 * Gets the value of btn if either CC_bind has the given CID
	 *
	 * @returns the btn of the bound controller, or
	 * @returns -1 if that controller is not bound
	 */
	short get_btn(CID) const;

	/*!
	 * Assigns the contents of the given CCB to this CCB
	 */
	CCB& operator=(const CCB&);

	/*!
	 * Returns True if this CCB's first isn't empty and the given CCB has a binding equal to it
	 */
	bool has_first(const CCB&) const;

	/*!
	 * Returns True if this CCB's second isn't empty and the given CCB has a binding equal to it
	 */
	bool has_second(const CCB&) const;
};

/*!
 * A preset, a collection of bindings for use in Control_config with an associated name
 */
class CC_preset {
public:
	SCP_vector<CCB> bindings;
	SCP_string name;
};

/*!
 * Control configuration item type.
 * @detail Contains binding info, documentation, behavior, etc. for a single control
 */
class CCI : public CCB {
public:
// Inherited from CCB
	// CC_bind first;
	// CC_bind second;

// Items Set in menu
	char tab;               //!< what tab (category) it belongs in
	int  indexXSTR;         //!< what string index we should use to translate this with an XSTR 0 = None, 1= Use item index + CONTROL_CONFIG_XSTR, 2 <= use CCI::indexXSTR directly
	SCP_string text;        //!< describes the action in the config screen

	CC_type type;           //!< manner control should be checked in

// Items used during gameplay
	int  used;                  //!< has control been used yet in mission?  If so, this is the timestamp
	bool disabled = true;       //!< whether this action should be available at all
	bool continuous_ongoing;    //!< whether this action is a continuous one and is currently ongoing

public:
	CCI() = default;
	CCI(const CCI& A) = default;
	
	/*!
	 * Assigns the contents of the given CCI to this CCI
	 */
	CCI& operator=(const CCI&);

	/*!
	 * @brief Takes the bindings of the given CCB, but leaves all other members alone
	 */
	CCI& operator=(const CCB&);
};

/*!
 * Builder predicate to populate a ControlConfig vector with hardcoded default bindings.
 */
class CCI_builder {
public:
	/*!
	 * Initilizes the given ControlConfig vector
	 */
	CCI_builder(SCP_vector<CCI>& _ControlConfig);

	/*!
	 * Start a chain of factory methods. If there are any pre-init work to be done, that's done here.
	 */
	CCI_builder& start();

	/*!
	 * End a chain of factory methods.  If there any post-init work to be done, that's done here.
	 */
	void end();

	/*!
	 * Assigns the hardcoded binding to the given action
	 * @note This differs from the original hardcode from :V: in the hopes of it being more intuitive to future IoAction additions
	 */
	CCI_builder& operator()(IoActionId action_id, short key_default, short joy_default, char tab, int indexXSTR, const char *text, CC_type type, bool disabled = false);

private:
	CCI_builder();	// Only one builder per Control Config, so a default constructor is useless
	SCP_vector<CCI>& ControlConfig;
};


extern int Failed_key_index;

extern int Axis_map_to[];           // Array to map an axis action to a joy axis. size() = NUM_JOY_AXIS_ACTIONS
extern int Axis_map_to_defaults[];
extern int Invert_axis[];           // Array to hold inversion bools for a joy axis. size() = JOY_NUM_AXES
extern int Invert_axis_defaults[];

extern int Joy_dead_zone_size;
extern int Joy_sensitivity;

extern int Control_config_overlay_id;

extern SCP_vector<CCI> Control_config;		//!< Stores the keyboard configuration
extern SCP_vector<CC_preset> Control_config_presets; // tabled control presets; pointers to config_item arrays
extern const char **Scan_code_text;
extern const char **Joy_button_text;

/*!
 * @brief Checks if either binding in the CCB has the given cid
 *
 * @returns  0  if its the first element, or
 * @returns  1  if its the second element, or
 * @returns -1  if neither
 */
int is_cid_either(CID, const CCB);

/*!
* @brief initialize common control config stuff - call at game startup after localization has been initialized
*/
void control_config_common_init();

/*!
 * @brief close common control config stuff - call at game shutdown
 */
void control_config_common_close();

/*!
 * @brief init config menu
 */
void control_config_init();

/*!
 * @brief do a frame of the config menu
 */
void control_config_do_frame(float frametime);

/*!
 * @brief close config menu
 */
void control_config_close();

/*!
 * @brief Cancel configuration of controls, revert any changes, return to previous menu/game state
 */
void control_config_cancel_exit();

/*!
 * @brief Resets all controls to values within the currently selected preset
 */
void control_config_reset_defaults();

/*!
 * Returns the IoActionId of a control bound to the given key
 *
 * @param[in] key           The key combo to look for
 * @param[in] find_override If true, return the IoActionId of a control that has this key as its default
 * @details If find_override is set to true, then this returns the index of the action
 */
int translate_key_to_index(const char *key, bool find_override=true);


/*!
 * @brief Given the system default key 'key', return the current key that is bound to that function.
 *
 * @param[in] key  The default key combo (as string) to a certain control
 *
 * @returns The key combo (as string) currently bound to the control
 *
 * @details Both the 'key' and the return value are descriptive strings that can be displayed
 * directly to the user.  If 'key' isn't a real key, is not normally bound to anything,
 * or there is no key currently bound to the function, NULL is returned.
 */
char *translate_key(char *key);

/**
 * @brief Converts the specified key code to a human readable string
 *
 * @note The returned value is localized to the current language
 *
 * @param code The key code to convert
 * @return The text representation of the code. The returned value is stored in a temporary location, copy it to your
 * own buffer if you want to continue using it.
 */
const char *textify_scancode(int code);

/**
 * @note Same as textify_scancode but always returns the same value regardless of current language
 * @param code The key code to convert
 * @return The name of the key
 * @see textify_scancode
 */
const char *textify_scancode_universal(int code);

/*!
 * @brief Checks how long a control has been active
 *
 * @param[in] id The IoActionId of the control to check
 *
 * @returns Time, in milliseconds, that the control has been active.
 *
 * @note This function is only to be used on CC_CONTINOUS type controls, or it will trigger an assertion
 */
float check_control_timef(int id);

/**
 * @brief Wrapper for check_control_used. Allows the game to ignore the key if told to do so by the ignore-key SEXP.
 *
 * @brief id    The IoActionId of the control to check
 * @brief key   The key combo to check against the control.
 *
 * @returns 0 If the control wasn't used, or
 * @returns 1 If the control was used
 */
int check_control(int id, int key = -1);

/**
 * @brief Gets the scaled reading for all control axes.
 *
 * @param[out] h  heading
 * @param[out] p pitch
 * @param[out] b bank
 * @param[out] ta Throttle - Absolute
 * @param[out] tr Throttle - Relative
 *
 * @note None of the input pointers may be nullptr
 */
void control_get_axes_readings(int *h, int *p, int *b, int *ta, int *tr);

/**
 * @brief Markes the given control (by IoActionId) as used
 *
 * @details Updates the ::used timestamp, triggers a script hook, and marks ::continous_ongoing as true
 */
void control_used(int id);

/**
 * @brief Clears the bindings of all controls
 */
void control_config_clear();

/**
 * @brief debug function used to indicate number of controls checked
 */
void control_check_indicate();

/**
 * @brief Clears the used timestamp of all controls
 */
void control_config_clear_used_status();

/**
 * @brief Applies sensitivity multiplier and deadzone histerisis to the raw axis value
 *
 * @param[in] raw  The raw axis value to transform
 *
 * @return The transformed value
 */
int joy_get_scaled_reading(int raw);

#endif
