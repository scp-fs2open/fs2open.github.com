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
} config_item;

// --------------------------------------------------
// Keyboard #defines for the actions  
// This is the value of the id field in config_item
// --------------------------------------------------

// targeting a ship

#define TARGET_NEXT										0
#define TARGET_PREV										1
#define TARGET_NEXT_CLOSEST_HOSTILE						2
#define TARGET_PREV_CLOSEST_HOSTILE						3
#define TOGGLE_AUTO_TARGETING							4
#define TARGET_NEXT_CLOSEST_FRIENDLY					5
#define TARGET_PREV_CLOSEST_FRIENDLY					6
#define TARGET_SHIP_IN_RETICLE							7
#define TARGET_CLOSEST_SHIP_ATTACKING_TARGET			8
#define TARGET_LAST_TRANMISSION_SENDER					9
#define STOP_TARGETING_SHIP								10

// targeting a ship's subsystem
#define TARGET_SUBOBJECT_IN_RETICLE						11
#define TARGET_NEXT_SUBOBJECT							12
#define TARGET_PREV_SUBOBJECT							13	
#define STOP_TARGETING_SUBSYSTEM						14

// speed matching 
#define MATCH_TARGET_SPEED								15
#define TOGGLE_AUTO_MATCH_TARGET_SPEED					16

// weapons
#define FIRE_PRIMARY									17
#define FIRE_SECONDARY									18
#define CYCLE_NEXT_PRIMARY								19
#define CYCLE_PREV_PRIMARY								20
#define CYCLE_SECONDARY									21
#define CYCLE_NUM_MISSLES								22
#define LAUNCH_COUNTERMEASURE							23

// controls
#define FORWARD_THRUST									24
#define REVERSE_THRUST									25
#define BANK_LEFT										26
#define BANK_RIGHT										27
#define PITCH_FORWARD									28
#define PITCH_BACK										29
#define YAW_LEFT										30
#define YAW_RIGHT										31

// throttle control
#define ZERO_THROTTLE									32
#define MAX_THROTTLE									33
#define ONE_THIRD_THROTTLE								34
#define TWO_THIRDS_THROTTLE								35
#define PLUS_5_PERCENT_THROTTLE							36
#define MINUS_5_PERCENT_THROTTLE						37

// squadmate messaging keys
#define ATTACK_MESSAGE									38
#define DISARM_MESSAGE									39
#define DISABLE_MESSAGE									40
#define ATTACK_SUBSYSTEM_MESSAGE						41
#define CAPTURE_MESSAGE									42
#define ENGAGE_MESSAGE									43
#define FORM_MESSAGE									44
#define IGNORE_MESSAGE									45
#define PROTECT_MESSAGE									46
#define COVER_MESSAGE									47
#define WARP_MESSAGE									48
#define REARM_MESSAGE									49

#define TARGET_CLOSEST_SHIP_ATTACKING_SELF				50

// Views
#define VIEW_CHASE										51
#define VIEW_EXTERNAL									52
#define VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK				53
#define VIEW_SLEW										54
#define VIEW_OTHER_SHIP									55
#define VIEW_DIST_INCREASE								56
#define VIEW_DIST_DECREASE								57
#define VIEW_CENTER										58
#define PADLOCK_UP										59
#define PADLOCK_DOWN									60
#define PADLOCK_LEFT									61
#define PADLOCK_RIGHT									62


#define RADAR_RANGE_CYCLE								63
#define SQUADMSG_MENU									64
#define SHOW_GOALS										65
#define END_MISSION										66
#define TARGET_TARGETS_TARGET							67
#define AFTERBURNER										68

#define INCREASE_WEAPON									69
#define DECREASE_WEAPON									70
#define INCREASE_SHIELD									71
#define DECREASE_SHIELD									72
#define INCREASE_ENGINE									73
#define DECREASE_ENGINE									74
#define ETS_EQUALIZE									75
#define SHIELD_EQUALIZE									76
#define SHIELD_XFER_TOP									77
#define SHIELD_XFER_BOTTOM								78
#define SHIELD_XFER_LEFT								79
#define SHIELD_XFER_RIGHT								80

#define XFER_SHIELD										81
#define XFER_LASER										82
//#define SHOW_DAMAGE_POPUP								83 // AL: this binding should be removing next time the controls are reorganized

#define GLIDE_WHEN_PRESSED								83
//Backslash -- this was a convenient place for Glide When Pressed, since Show Damage Popup isn't used
#define BANK_WHEN_PRESSED								84
#define SHOW_NAVMAP										85
#define ADD_REMOVE_ESCORT								86
#define ESCORT_CLEAR									87
#define TARGET_NEXT_ESCORT_SHIP							88

#define TARGET_CLOSEST_REPAIR_SHIP						89
#define TARGET_NEXT_UNINSPECTED_CARGO					90
#define TARGET_PREV_UNINSPECTED_CARGO					91
#define TARGET_NEWEST_SHIP								92
#define TARGET_NEXT_LIVE_TURRET							93
#define TARGET_PREV_LIVE_TURRET							94

#define TARGET_NEXT_BOMB								95
#define TARGET_PREV_BOMB								96

// multiplayer messaging keys
#define MULTI_MESSAGE_ALL								97
#define MULTI_MESSAGE_FRIENDLY							98
#define MULTI_MESSAGE_HOSTILE							99
#define MULTI_MESSAGE_TARGET							100

// multiplayer misc keys
#define MULTI_OBSERVER_ZOOM_TO							101

#define TIME_SPEED_UP									102
#define TIME_SLOW_DOWN									103

#define TOGGLE_HUD_CONTRAST								104

#define MULTI_TOGGLE_NETINFO							105

#define MULTI_SELF_DESTRUCT								106

#define TOGGLE_HUD										107
#define RIGHT_SLIDE_THRUST								108
#define LEFT_SLIDE_THRUST								109
#define UP_SLIDE_THRUST									110
#define DOWN_SLIDE_THRUST								111
#define HUD_TARGETBOX_TOGGLE_WIREFRAME					112
#define VIEW_TOPDOWN									113
#define VIEW_TRACK_TARGET								114

//AutoPilot - Kazan
#define AUTO_PILOT_TOGGLE								115
#define NAV_CYCLE										116

//Gliding
#define TOGGLE_GLIDING									117

// this should be the total number of control action defines above (or last define + 1)
#define CCFG_MAX 118

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
