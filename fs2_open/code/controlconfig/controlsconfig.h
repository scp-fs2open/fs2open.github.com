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

#define CONTROL_CONFIG_XSTR	507

#define JOY_X_AXIS	0
#define JOY_Y_AXIS	1
#define JOY_Z_AXIS	2
#define JOY_RX_AXIS	3
#define JOY_RY_AXIS	4
#define JOY_RZ_AXIS	5

#define NUM_JOY_AXIS_ACTIONS	5

#define JOY_HEADING_AXIS		0
#define JOY_PITCH_AXIS			1
#define JOY_BANK_AXIS			2
#define JOY_ABS_THROTTLE_AXIS	3
#define JOY_REL_THROTTLE_AXIS	4

// --------------------------------------------------
// different types of controls that can be assigned 
// --------------------------------------------------

#define CC_TYPE_TRIGGER		0
#define CC_TYPE_CONTINUOUS	1

typedef struct config_item {
	short key_default;  // default key bound to action
	short joy_default;  // default joystick button bound to action
	char tab;				// what tab (category) it belongs in
	bool hasXSTR;			// whether we should translate this with an XSTR
	char *text;				// describes the action in the config screen
	char type;				// manner control should be checked in
	short key_id;  // actual key bound to action
	short joy_id;  // joystick button bound to action
	int used;				// has control been used yet in mission?  If so, this is the timestamp
	bool disabled;			// whether this action should be available at all
} config_item;

/**
 * All available actions
 * This is the value of the id field in config_item
 */
enum IoActionId  {
	// targeting a ship
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

	// targeting a ship's subsystem
	TARGET_SUBOBJECT_IN_RETICLE						=11,	//!< target ships subsystem in reticle
	TARGET_NEXT_SUBOBJECT							=12,	//!< target next subsystem on current target
	TARGET_PREV_SUBOBJECT							=13,	//!< TARGET_PREV_SUBOBJECT
	STOP_TARGETING_SUBSYSTEM						=14,	//!< stop targeting subsystems on ship

	// speed matching
	MATCH_TARGET_SPEED								=15,	//!< match target speed
	TOGGLE_AUTO_MATCH_TARGET_SPEED					=16,	//!< toggle auto-match target speed

	// weapons
	FIRE_PRIMARY									=17,	//!< FIRE_PRIMARY
	FIRE_SECONDARY									=18,	//!< FIRE_SECONDARY
	CYCLE_NEXT_PRIMARY								=19,	//!< cycle to next primary weapon
	CYCLE_PREV_PRIMARY								=20,	//!< cycle to previous primary weapon
	CYCLE_SECONDARY									=21,	//!< cycle to next secondary weapon
	CYCLE_NUM_MISSLES								=22,	//!< cycle number of missiles fired from secondary bank
	LAUNCH_COUNTERMEASURE							=23,	//!< LAUNCH_COUNTERMEASURE

	// controls
	FORWARD_THRUST									=24,	//!< FORWARD_THRUST
	REVERSE_THRUST									=25,	//!< REVERSE_THRUST
	BANK_LEFT										=26,	//!< BANK_LEFT
	BANK_RIGHT										=27,	//!< BANK_RIGHT
	PITCH_FORWARD									=28,	//!< PITCH_FORWARD
	PITCH_BACK										=29,	//!< PITCH_BACK
	YAW_LEFT										=30,	//!< YAW_LEFT
	YAW_RIGHT										=31,	//!< YAW_RIGHT

	// throttle control
	ZERO_THROTTLE									=32,	//!< ZERO_THROTTLE
	MAX_THROTTLE									=33,	//!< MAX_THROTTLE
	ONE_THIRD_THROTTLE								=34,	//!< ONE_THIRD_THROTTLE
	TWO_THIRDS_THROTTLE								=35,	//!< TWO_THIRDS_THROTTLE
	PLUS_5_PERCENT_THROTTLE							=36,	//!< PLUS_5_PERCENT_THROTTLE
	MINUS_5_PERCENT_THROTTLE						=37,	//!< MINUS_5_PERCENT_THROTTLE

	// squadmate messaging keys
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

	// Views
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

	RADAR_RANGE_CYCLE								=63,	//!< cycle to next radar range
	SQUADMSG_MENU									=64,	//!< toggle the squadmate messaging menu
	SHOW_GOALS										=65,	//!< show the mission goals screen
	END_MISSION										=66,	//!< end the mission
	TARGET_TARGETS_TARGET							=67,	//!< target your target's target
	AFTERBURNER										=68,	//!< AFTERBURNER

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

	//Backslash -- this was a convenient place for Glide When Pressed, since Show Damage Popup isn't used
	GLIDE_WHEN_PRESSED								=83, 	//!< GLIDE_WHEN_PRESSED
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

	// multiplayer messaging keys
	MULTI_MESSAGE_ALL								=97,	//!< message all netplayers
	MULTI_MESSAGE_FRIENDLY							=98,	//!< message all friendlies
	MULTI_MESSAGE_HOSTILE							=99,	//!< message all hostiles
	MULTI_MESSAGE_TARGET							=100,	//!< message targeted ship (if player)

	// multiplayer misc keys
	MULTI_OBSERVER_ZOOM_TO							=101,	//!< if i'm an observer, zoom to my targeted object

	TIME_SPEED_UP									=102,	//!< TIME_SPEED_UP
	TIME_SLOW_DOWN									=103,	//!< TIME_SLOW_DOWN

	TOGGLE_HUD_CONTRAST								=104,	//!< toggle between high and low HUD contrast

	MULTI_TOGGLE_NETINFO							=105,	//!< toggle network info

	MULTI_SELF_DESTRUCT								=106,	//!< self destruct (multiplayer only)

	TOGGLE_HUD										=107,	//!< TOGGLE_HUD
	RIGHT_SLIDE_THRUST								=108,	//!< RIGHT_SLIDE_THRUST
	LEFT_SLIDE_THRUST								=109,	//!< LEFT_SLIDE_THRUST
	UP_SLIDE_THRUST									=110,	//!< UP_SLIDE_THRUST
	DOWN_SLIDE_THRUST								=111,	//!< DOWN_SLIDE_THRUST
	HUD_TARGETBOX_TOGGLE_WIREFRAME					=112,	//!< HUD_TARGETBOX_TOGGLE_WIREFRAME
	VIEW_TOPDOWN									=113,	//!< VIEW_TOPDOWN
	VIEW_TRACK_TARGET								=114,	//!< VIEW_TRACK_TARGET

	//AutoPilot - Kazan
	AUTO_PILOT_TOGGLE								=115,	//!< Autopilot key control
	NAV_CYCLE										=116,	//!< NAV_CYCLE

	//Gliding
	TOGGLE_GLIDING									=117,	//!< TOGGLE_GLIDING

	/**
	 * This must always be below the last defined action
	 */
	CCFG_MAX                                  //!<  The total number of defined control actions (or last define + 1)
};

extern int Failed_key_index;
extern int Invert_heading;
extern int Invert_pitch;
extern int Invert_roll;
extern int Invert_thrust;
extern int Disable_axis2;
extern int Disable_axis3;

extern int Axis_map_to[];
extern int Invert_axis[];
extern int Invert_axis_defaults[];

extern config_item Control_config[];	// stores the keyboard configuration
extern char **Scan_code_text;
extern char **Joy_button_text;

// initialize common control config stuff - call at game startup after localization has been initialized
void control_config_common_init();

void control_config_init();
void control_config_do_frame(float frametime);
void control_config_close();

void control_config_cancel_exit();

void control_config_reset_defaults();
int translate_key_to_index(char *key);
char *translate_key(char *key);
char *textify_scancode(int code);
float check_control_timef(int id);
int check_control(int id, int key = -1);
void control_get_axes_readings(int *h, int *p, int *b, int *ta, int *tr);
void control_used(int id);
void control_config_clear();
void clear_key_binding(short key);
void control_check_indicate();
void control_config_clear_used_status();

#endif
