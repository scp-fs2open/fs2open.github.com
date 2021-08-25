/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/

#include <cstdio>
#include <cstdarg>
#include <string>

#include "cfile/cfile.h"
#include "controlconfig/controlsconfig.h"
#include "debugconsole/console.h"
#include "def_files/def_files.h"
#include "globalincs/systemvars.h"
#include "io/joy.h"
#include "io/key.h"
#include "localization/localize.h"
#include "options/Option.h"
#include "parse/parselo.h"

// z64: These enumerations MUST equal to those in controlsconfig.cpp...
// z64: Really need a better way than this.
enum CC_tab {
	NO_TAB          =-1,    // Not on any tab accisible by the player
	TARGET_TAB      =0,     // Targeting controls
	SHIP_TAB        =1,     // Flight controls
	WEAPON_TAB      =2,     // Weapon selection and firing controls (and countermeasures)
	COMPUTER_TAB    =3      // Energy Management and Misc. controls
};

int Failed_key_index;

// Joystick configuration
int Joy_dead_zone_size = 10;

auto DeadZoneOption =
    options::OptionBuilder<int>("Input.JoystickDeadZone", "Deadzone", "The deadzone of the selected joystick.")
        .category("Input")
        .range(0, 45)
        .level(options::ExpertLevel::Beginner)
        .default_val(10)
        .bind_to(&Joy_dead_zone_size)
        .importance(1)
        .finish();

int Joy_sensitivity = 9;

auto SensitivityOption =
    options::OptionBuilder<int>("Input.JoystickSensitivity", "Sensitivity", "The sentitivity of the selected joystick.")
        .category("Input")
        .range(0, 9)
        .level(options::ExpertLevel::Beginner)
        .default_val(9)
        .bind_to(&Joy_sensitivity)
        .importance(2)
        .finish();

// Containers for axis binding and defaults
int Axis_map_to[NUM_JOY_AXIS_ACTIONS] = { JOY_X_AXIS, JOY_Y_AXIS, JOY_RX_AXIS, -1, -1 };
int Axis_map_to_defaults[NUM_JOY_AXIS_ACTIONS] = { JOY_X_AXIS, JOY_Y_AXIS, JOY_RX_AXIS, -1, -1 };
int Invert_axis[JOY_NUM_AXES] = { 0, 0, 0, 0, 0, 0 };
int Invert_axis_defaults[JOY_NUM_AXES] = { 0, 0, 0, 0, 0, 0 };

//! arrays which hold the key mappings.  The array index represents a key-independent action.
//! please use SPACES for aligning the fields of this array
//! When adding new controls, order that they show up is dependant on their location in IoActionId, not on their hardcoded locations here
//! It is still recommended however that the order between declaration and definition is maintained for easy lookup
//XSTR:OFF
SCP_vector<CCI> Control_config;

//! Vector of presets. Each preset is a collection of bindings that can be copied into Control_config's bindings. [0] is the default preset.
SCP_vector<CC_preset> Control_config_presets;

/**
 * Initializes the Control_config vector and the hardcoded defaults preset
 */
void control_config_common_init_bindings() {
	Control_config.clear();	// Clear exisitng vectory, just in case init is run more than once for whatever reason

	CCI_builder Builder(Control_config);
	Builder.start()
	// Note: when adding new controls, group them according to the tab they would show up on.
	// action_id, key_default, joy_default, tab, XStR index, Text, CC_Type

	// Note: when adding new controls, if a control does nothing in retail data, it should be disabled by default.  The controls config menu
	// is cluttered enough as is, don't need to mess it up any more. --z64

	// Ship targeting
	(TARGET_NEXT,                                         KEY_T, -1, TARGET_TAB, 1, "Target Next Ship",                      CC_TYPE_TRIGGER)
	(TARGET_PREV,                           KEY_SHIFTED | KEY_T, -1, TARGET_TAB, 1, "Target Previous Ship",                  CC_TYPE_TRIGGER)
	(TARGET_NEXT_CLOSEST_HOSTILE,                         KEY_H,  2, TARGET_TAB, 1, "Target Next Closest Hostile Ship",      CC_TYPE_TRIGGER)
	(TARGET_PREV_CLOSEST_HOSTILE,           KEY_SHIFTED | KEY_H, -1, TARGET_TAB, 1, "Target Previous Closest Hostile Ship",  CC_TYPE_TRIGGER)
	(TOGGLE_AUTO_TARGETING,                 KEY_ALTED   | KEY_H, -1, TARGET_TAB, 1, "Toggle Auto Targeting",                 CC_TYPE_TRIGGER)
	(TARGET_NEXT_CLOSEST_FRIENDLY,                        KEY_F, -1, TARGET_TAB, 1, "Target Next Closest Friendly Ship",     CC_TYPE_TRIGGER)
	(TARGET_PREV_CLOSEST_FRIENDLY,          KEY_SHIFTED | KEY_F, -1, TARGET_TAB, 1, "Target Previous Closest Friendly Ship", CC_TYPE_TRIGGER)
	(TARGET_SHIP_IN_RETICLE,                              KEY_Y,  4, TARGET_TAB, 1, "Target Ship in Reticle",                CC_TYPE_TRIGGER)
	(TARGET_CLOSEST_SHIP_ATTACKING_TARGET,                KEY_G, -1, TARGET_TAB, 1, "Target Target's Nearest Attacker",      CC_TYPE_TRIGGER)
	(TARGET_LAST_TRANMISSION_SENDER,        KEY_ALTED   | KEY_Y, -1, TARGET_TAB, 1, "Target Last Ship to Send Transmission", CC_TYPE_TRIGGER)
	(STOP_TARGETING_SHIP,                   KEY_ALTED   | KEY_T, -1, TARGET_TAB, 1, "Turn Off Targeting",                    CC_TYPE_TRIGGER)

	(TARGET_SUBOBJECT_IN_RETICLE,                         KEY_V, -1, TARGET_TAB, 1, "Target Subsystem in Reticle",           CC_TYPE_TRIGGER)
	(TARGET_NEXT_SUBOBJECT,                               KEY_S, -1, TARGET_TAB, 1, "Target Next Subsystem",                 CC_TYPE_TRIGGER)
	(TARGET_PREV_SUBOBJECT,                 KEY_SHIFTED | KEY_S, -1, TARGET_TAB, 1, "Target Previous Subsystem",             CC_TYPE_TRIGGER)
	(STOP_TARGETING_SUBSYSTEM,              KEY_ALTED   | KEY_S, -1, TARGET_TAB, 1, "Turn Off Targeting of Subsystems",      CC_TYPE_TRIGGER)

	(TARGET_CLOSEST_SHIP_ATTACKING_SELF,                  KEY_R,  6, TARGET_TAB, 1, "Target Closest Attacking Ship",         CC_TYPE_TRIGGER)
	(TARGET_TARGETS_TARGET,                               KEY_J, -1, TARGET_TAB, 1, "Target Target's Target",                CC_TYPE_TRIGGER)

	(TARGET_NEXT_ESCORT_SHIP,                             KEY_E, -1, TARGET_TAB, 1, "Target Next Escort Ship",           CC_TYPE_TRIGGER)
	(TARGET_CLOSEST_REPAIR_SHIP,              KEY_ALTED | KEY_R, -1, TARGET_TAB, 1, "Target Closest Repair Ship",        CC_TYPE_TRIGGER)
	(TARGET_NEXT_UNINSPECTED_CARGO,                       KEY_U, -1, TARGET_TAB, 1, "Target Next Uninspected Cargo",     CC_TYPE_TRIGGER)
	(TARGET_PREV_UNINSPECTED_CARGO,         KEY_SHIFTED | KEY_U, -1, TARGET_TAB, 1, "Target Previous Uninspected Cargo", CC_TYPE_TRIGGER)
	(TARGET_NEWEST_SHIP,                                  KEY_N, -1, TARGET_TAB, 1, "Target Newest Ship in Area",        CC_TYPE_TRIGGER)

	(TARGET_NEXT_LIVE_TURRET,                             KEY_K, -1, TARGET_TAB, 1, "Target Next Live Turret",           CC_TYPE_TRIGGER)
	(TARGET_PREV_LIVE_TURRET,               KEY_SHIFTED | KEY_K, -1, TARGET_TAB, 1, "Target Previous Live Turret",       CC_TYPE_TRIGGER)

	(TARGET_NEXT_BOMB,                                    KEY_B, -1, TARGET_TAB, 1, "Target Next Hostile Bomb or Bomber",     CC_TYPE_TRIGGER)
	(TARGET_PREV_BOMB,                      KEY_SHIFTED | KEY_B, -1, TARGET_TAB, 1, "Target Previous Hostile Bomb or Bomber", CC_TYPE_TRIGGER)

	// flight controls (Rotation)
	(BANK_LEFT,                                        KEY_PAD7, -1, SHIP_TAB, 1, "Bank Left",      CC_TYPE_CONTINUOUS)
	(BANK_RIGHT,                                       KEY_PAD9, -1, SHIP_TAB, 1, "Bank Right",     CC_TYPE_CONTINUOUS)
	(PITCH_FORWARD,                                    KEY_PAD8, -1, SHIP_TAB, 1, "Pitch Forward",  CC_TYPE_CONTINUOUS)
	(PITCH_BACK,                                       KEY_PAD2, -1, SHIP_TAB, 1, "Pitch Backward", CC_TYPE_CONTINUOUS)
	(YAW_LEFT,                                         KEY_PAD4, -1, SHIP_TAB, 1, "Yaw Left",       CC_TYPE_CONTINUOUS)
	(YAW_RIGHT,                                        KEY_PAD6, -1, SHIP_TAB, 1, "Yaw Right",      CC_TYPE_CONTINUOUS)

	// flight controls (Throttle)
	(ZERO_THROTTLE,                                  KEY_BACKSP, -1, SHIP_TAB, 1, "Set Throttle to Zero",           CC_TYPE_TRIGGER)
	(MAX_THROTTLE,                                    KEY_SLASH, -1, SHIP_TAB, 1, "Set Throttle to Max",            CC_TYPE_TRIGGER)
	(ONE_THIRD_THROTTLE,                           KEY_LBRACKET, -1, SHIP_TAB, 1, "Set Throttle to One-Third",      CC_TYPE_TRIGGER)
	(TWO_THIRDS_THROTTLE,                          KEY_RBRACKET, -1, SHIP_TAB, 1, "Set Throttle to Two-Thirds",     CC_TYPE_TRIGGER)
	(PLUS_5_PERCENT_THROTTLE,                         KEY_EQUAL, -1, SHIP_TAB, 1, "Increase Throttle 5 Percent",    CC_TYPE_TRIGGER)
	(MINUS_5_PERCENT_THROTTLE,                        KEY_MINUS, -1, SHIP_TAB, 1, "Decrease Throttle 5 Percent",    CC_TYPE_TRIGGER)

	// flight controls (Thrust)
	(FORWARD_THRUST,                                      KEY_A, -1, SHIP_TAB, 1, "Forward Thrust", CC_TYPE_CONTINUOUS)
	(REVERSE_THRUST,                                      KEY_Z, -1, SHIP_TAB, 1, "Reverse Thrust", CC_TYPE_CONTINUOUS)
	(RIGHT_SLIDE_THRUST,                    KEY_SHIFTED | KEY_3, -1, SHIP_TAB, 1, "Right Thrust",   CC_TYPE_CONTINUOUS)
	(LEFT_SLIDE_THRUST,                     KEY_SHIFTED | KEY_1, -1, SHIP_TAB, 1, "Left Thrust",    CC_TYPE_CONTINUOUS)
	(UP_SLIDE_THRUST,                 KEY_SHIFTED | KEY_PADPLUS, -1, SHIP_TAB, 1, "Up Thrust",      CC_TYPE_CONTINUOUS)
	(DOWN_SLIDE_THRUST,              KEY_SHIFTED | KEY_PADENTER, -1, SHIP_TAB, 1, "Down Thrust",    CC_TYPE_CONTINUOUS)

	// flight controls (flight modes)
	(BANK_WHEN_PRESSED,                                      -1, -1, SHIP_TAB, 1, "Bank When Pressed",  CC_TYPE_CONTINUOUS)
	(AFTERBURNER,                                       KEY_TAB,  5, SHIP_TAB, 1, "Afterburner",        CC_TYPE_CONTINUOUS)
	(GLIDE_WHEN_PRESSED,                                     -1, -1, SHIP_TAB, 0, "Glide When Pressed", CC_TYPE_CONTINUOUS)
	(TOGGLE_GLIDING,                          KEY_ALTED | KEY_G, -1, SHIP_TAB, 0, "Toggle Gliding",     CC_TYPE_TRIGGER)

	// weapons
	(FIRE_PRIMARY,                                    KEY_LCTRL,  0, WEAPON_TAB, 1, "Fire Primary Weapon",                    CC_TYPE_CONTINUOUS)
	(FIRE_SECONDARY,                               KEY_SPACEBAR,  1, WEAPON_TAB, 1, "Fire Secondary Weapon",                  CC_TYPE_CONTINUOUS)
	(CYCLE_NEXT_PRIMARY,                             KEY_PERIOD, -1, WEAPON_TAB, 1, "Cycle Primary Weapon Forward",           CC_TYPE_TRIGGER)
	(CYCLE_PREV_PRIMARY,                              KEY_COMMA, -1, WEAPON_TAB, 1, "Cycle Primary Weapon Backward",          CC_TYPE_TRIGGER)
	(CYCLE_PRIMARY_WEAPON_SEQUENCE,                       KEY_O, -1, WEAPON_TAB, 0, "Cycle Primary Weapon Firing Rate",       CC_TYPE_TRIGGER)
	(CYCLE_SECONDARY,                                KEY_DIVIDE, -1, WEAPON_TAB, 1, "Cycle Secondary Weapon Forward",         CC_TYPE_TRIGGER)
	(CYCLE_NUM_MISSLES,                KEY_SHIFTED | KEY_DIVIDE, -1, WEAPON_TAB, 1, "Cycle Secondary Weapon Firing Rate",     CC_TYPE_TRIGGER)
	(LAUNCH_COUNTERMEASURE,                               KEY_X,  3, WEAPON_TAB, 1, "Launch Countermeasure",                  CC_TYPE_TRIGGER)

	// matching speed
	(MATCH_TARGET_SPEED,                                  KEY_M, -1, COMPUTER_TAB, 1, "Match Target Speed",                    CC_TYPE_TRIGGER)
	(TOGGLE_AUTO_MATCH_TARGET_SPEED,        KEY_ALTED   | KEY_M, -1, COMPUTER_TAB, 1, "Toggle Auto Speed Matching",            CC_TYPE_TRIGGER)

	// squadmate messaging
	(ATTACK_MESSAGE,                        KEY_SHIFTED | KEY_A, -1, COMPUTER_TAB, 1, "(Squadmate) Attack My Target",                   CC_TYPE_TRIGGER)
	(DISARM_MESSAGE,                        KEY_SHIFTED | KEY_Z, -1, COMPUTER_TAB, 1, "(Squadmate) Disarm My Target",                   CC_TYPE_TRIGGER)
	(DISABLE_MESSAGE,                       KEY_SHIFTED | KEY_D, -1, COMPUTER_TAB, 1, "(Squadmate) Disable My Target",                  CC_TYPE_TRIGGER)
	(ATTACK_SUBSYSTEM_MESSAGE,              KEY_SHIFTED | KEY_V, -1, COMPUTER_TAB, 1, "(Squadmate) Attack My Subsystem",                CC_TYPE_TRIGGER)
	(CAPTURE_MESSAGE,                       KEY_SHIFTED | KEY_X, -1, COMPUTER_TAB, 1, "(Squadmate) Capture My Target",                  CC_TYPE_TRIGGER)
	(ENGAGE_MESSAGE,                        KEY_SHIFTED | KEY_E, -1, COMPUTER_TAB, 1, "(Squadmate) Engage Enemy",                       CC_TYPE_TRIGGER)
	(FORM_MESSAGE,                          KEY_SHIFTED | KEY_W, -1, COMPUTER_TAB, 1, "(Squadmate) Form on My Wing",                    CC_TYPE_TRIGGER)
	(IGNORE_MESSAGE,                        KEY_SHIFTED | KEY_I, -1, COMPUTER_TAB, 1, "(Squadmate) Ignore My Target",                   CC_TYPE_TRIGGER)
	(PROTECT_MESSAGE,                       KEY_SHIFTED | KEY_P, -1, COMPUTER_TAB, 1, "(Squadmate) Protect My Target",                  CC_TYPE_TRIGGER)
	(COVER_MESSAGE,                         KEY_SHIFTED | KEY_C, -1, COMPUTER_TAB, 1, "(Squadmate) Cover Me",                           CC_TYPE_TRIGGER)
	(WARP_MESSAGE,                          KEY_SHIFTED | KEY_J, -1, COMPUTER_TAB, 1, "(Squadmate) Return to Base",                     CC_TYPE_TRIGGER)
	(REARM_MESSAGE,                         KEY_SHIFTED | KEY_R, -1, COMPUTER_TAB, 1, "(Squadmate) Rearm Me",                           CC_TYPE_TRIGGER)

	// Views
	(VIEW_CHASE,                                KEY_PADMULTIPLY, -1, COMPUTER_TAB, 1, "Chase View",                         CC_TYPE_TRIGGER)
	(VIEW_EXTERNAL,                               KEY_PADPERIOD, -1, COMPUTER_TAB, 1, "External View",                      CC_TYPE_TRIGGER)
	(VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK,             KEY_PADENTER, -1, COMPUTER_TAB, 1, "Toggle External Camera Lock",        CC_TYPE_TRIGGER)
	(VIEW_SLEW,                                        KEY_PAD0, -1, COMPUTER_TAB, 1, "Free-Look View",                     CC_TYPE_CONTINUOUS)
	(VIEW_OTHER_SHIP,                             KEY_PADDIVIDE, -1, COMPUTER_TAB, 1, "Current Target View",                CC_TYPE_TRIGGER)
	(VIEW_DIST_INCREASE,                            KEY_PADPLUS, -1, COMPUTER_TAB, 1, "Increase View Distance",             CC_TYPE_CONTINUOUS)
	(VIEW_DIST_DECREASE,                           KEY_PADMINUS, -1, COMPUTER_TAB, 1, "Decrease View Distance",             CC_TYPE_CONTINUOUS)
	(VIEW_CENTER,                                      KEY_PAD5, -1, COMPUTER_TAB, 1, "Center View",                        CC_TYPE_CONTINUOUS)
	(PADLOCK_UP,                                             -1, 33, COMPUTER_TAB, 1, "View Up",                            CC_TYPE_CONTINUOUS)
	(PADLOCK_DOWN,                                           -1, 32, COMPUTER_TAB, 1, "View Rear",                          CC_TYPE_CONTINUOUS)
	(PADLOCK_LEFT,                                           -1, 34, COMPUTER_TAB, 1, "View Left",                          CC_TYPE_CONTINUOUS)
	(PADLOCK_RIGHT,                                          -1, 35, COMPUTER_TAB, 1, "View Right",                         CC_TYPE_CONTINUOUS)
	(VIEW_TOPDOWN,                                           -1, -1, COMPUTER_TAB, 0, "Top-Down View",                      CC_TYPE_TRIGGER)
	(VIEW_TRACK_TARGET,                                      -1, -1, COMPUTER_TAB, 0, "Target Padlock View",                CC_TYPE_TRIGGER)

	(RADAR_RANGE_CYCLE,                            KEY_RAPOSTRO, -1, COMPUTER_TAB, 1, "Cycle Radar Range",                 CC_TYPE_TRIGGER)
	(SQUADMSG_MENU,                                       KEY_C, -1, COMPUTER_TAB, 1, "Communications Menu",               CC_TYPE_TRIGGER)
	(SHOW_GOALS,                                             -1, -1, NO_TAB,       1, "Show Objectives",                   CC_TYPE_TRIGGER)
	(END_MISSION,                             KEY_ALTED | KEY_J, -1, COMPUTER_TAB, 1, "Enter Subspace (End Mission)",      CC_TYPE_TRIGGER)

	(INCREASE_WEAPON,                                KEY_INSERT, -1, COMPUTER_TAB, 1, "Weapon Energy Increase",            CC_TYPE_TRIGGER)
	(DECREASE_WEAPON,                                KEY_DELETE, -1, COMPUTER_TAB, 1, "Weapon Energy Decrease",            CC_TYPE_TRIGGER)
	(INCREASE_SHIELD,                                  KEY_HOME, -1, COMPUTER_TAB, 1, "Shield Energy Increase",            CC_TYPE_TRIGGER)
	(DECREASE_SHIELD,                                   KEY_END, -1, COMPUTER_TAB, 1, "Shield Energy Decrease",            CC_TYPE_TRIGGER)
	(INCREASE_ENGINE,                                KEY_PAGEUP, -1, COMPUTER_TAB, 1, "Engine Energy Increase",            CC_TYPE_TRIGGER)
	(DECREASE_ENGINE,                              KEY_PAGEDOWN, -1, COMPUTER_TAB, 1, "Engine Energy Decrease",            CC_TYPE_TRIGGER)
	(ETS_EQUALIZE,                            KEY_ALTED | KEY_D, -1, COMPUTER_TAB, 1, "Equalize Energy Settings",          CC_TYPE_TRIGGER)

	(SHIELD_EQUALIZE,                                     KEY_Q,  7, COMPUTER_TAB, 1, "Equalize Shields",                  CC_TYPE_TRIGGER)
	(SHIELD_XFER_TOP,                                    KEY_UP, -1, COMPUTER_TAB, 1, "Augment Shield Forward",            CC_TYPE_TRIGGER)
	(SHIELD_XFER_BOTTOM,                               KEY_DOWN, -1, COMPUTER_TAB, 1, "Augment Shield Rear",               CC_TYPE_TRIGGER)
	(SHIELD_XFER_LEFT,                                 KEY_LEFT, -1, COMPUTER_TAB, 1, "Augment Shield Left",               CC_TYPE_TRIGGER)
	(SHIELD_XFER_RIGHT,                               KEY_RIGHT, -1, COMPUTER_TAB, 1, "Augment Shield Right",              CC_TYPE_TRIGGER)
	(XFER_SHIELD,                                 KEY_SCROLLOCK, -1, COMPUTER_TAB, 1, "Transfer Energy Laser->Shield",     CC_TYPE_TRIGGER)
	(XFER_LASER,                    KEY_SHIFTED | KEY_SCROLLOCK, -1, COMPUTER_TAB, 1, "Transfer Energy Shield->Laser",     CC_TYPE_TRIGGER)

	// Navigation and Autopilot
	(SHOW_NAVMAP,                                            -1, -1, NO_TAB,       1, "Show Nav Map",       CC_TYPE_TRIGGER)
	(AUTO_PILOT_TOGGLE,                       KEY_ALTED | KEY_A, -1, COMPUTER_TAB, 0, "Toggle Auto Pilot",  CC_TYPE_TRIGGER)
	(NAV_CYCLE,                               KEY_ALTED | KEY_N, -1, COMPUTER_TAB, 0, "Cycle Nav Points",   CC_TYPE_TRIGGER)

	// Escort
	(ADD_REMOVE_ESCORT,                       KEY_ALTED | KEY_E, -1, COMPUTER_TAB, 1, "Add or Remove Escort",   CC_TYPE_TRIGGER)
	(ESCORT_CLEAR,              KEY_ALTED | KEY_SHIFTED | KEY_E, -1, COMPUTER_TAB, 1, "Clear Escort List",      CC_TYPE_TRIGGER)

	// Multiplayer
	(MULTI_MESSAGE_ALL,                                   KEY_1, -1, COMPUTER_TAB, 1, "(Multiplayer) Message All",              CC_TYPE_TRIGGER)
	(MULTI_MESSAGE_FRIENDLY,                              KEY_2, -1, COMPUTER_TAB, 1, "(Multiplayer) Message Friendly",         CC_TYPE_TRIGGER)
	(MULTI_MESSAGE_HOSTILE,                               KEY_3, -1, COMPUTER_TAB, 1, "(Multiplayer) Message Hostile",          CC_TYPE_TRIGGER)
	(MULTI_MESSAGE_TARGET,                                KEY_4, -1, COMPUTER_TAB, 1, "(Multiplayer) Message Target",           CC_TYPE_TRIGGER)
	(MULTI_OBSERVER_ZOOM_TO,                  KEY_ALTED | KEY_X, -1, COMPUTER_TAB, 1, "(Multiplayer) Observer Zoom to Target",  CC_TYPE_TRIGGER)
	(MULTI_TOGGLE_NETINFO,                  KEY_SHIFTED | KEY_N, -1, COMPUTER_TAB, 1, "(Multiplayer) Toggle Network Info",      CC_TYPE_TRIGGER)
	(MULTI_SELF_DESTRUCT,                 KEY_SHIFTED | KEY_END, -1, COMPUTER_TAB, 1, "(Multiplayer) Self Destruct",            CC_TYPE_TRIGGER)

	// Time compression
	(TIME_SPEED_UP,                    KEY_SHIFTED | KEY_PERIOD, -1, COMPUTER_TAB, 1, "Time Compression Increase",  CC_TYPE_TRIGGER)
	(TIME_SLOW_DOWN,                    KEY_SHIFTED | KEY_COMMA, -1, COMPUTER_TAB, 1, "Time Compression Decrease",  CC_TYPE_TRIGGER)

	// HUD
	(TOGGLE_HUD,                                    KEY_SHIFTED | KEY_O, -1, COMPUTER_TAB, 1, "Toggle HUD",                       CC_TYPE_TRIGGER)
	(TOGGLE_HUD_CONTRAST,                                         KEY_L, -1, COMPUTER_TAB, 1, "Toggle High HUD Contrast",         CC_TYPE_TRIGGER)
	(HUD_TARGETBOX_TOGGLE_WIREFRAME,    KEY_ALTED | KEY_SHIFTED | KEY_Q, -1, COMPUTER_TAB, 1, "Toggle HUD Wireframe Target View", CC_TYPE_TRIGGER)

	// Custom Controls
	(CUSTOM_CONTROL_1,                  KEY_ALTED | KEY_SHIFTED | KEY_1, -1, COMPUTER_TAB, 0, "Custom Control 1", CC_TYPE_TRIGGER, true)
	(CUSTOM_CONTROL_2,                  KEY_ALTED | KEY_SHIFTED | KEY_2, -1, COMPUTER_TAB, 0, "Custom Control 2", CC_TYPE_TRIGGER, true)
	(CUSTOM_CONTROL_3,                  KEY_ALTED | KEY_SHIFTED | KEY_3, -1, COMPUTER_TAB, 0, "Custom Control 3", CC_TYPE_TRIGGER, true)
	(CUSTOM_CONTROL_4,                  KEY_ALTED | KEY_SHIFTED | KEY_4, -1, COMPUTER_TAB, 0, "Custom Control 4", CC_TYPE_TRIGGER, true)
	(CUSTOM_CONTROL_5,                  KEY_ALTED | KEY_SHIFTED | KEY_5, -1, COMPUTER_TAB, 0, "Custom Control 5", CC_TYPE_TRIGGER, true)
	.end();	// Builder

	// init default preset
	Control_config_presets.clear();

	CC_preset preset;
	preset.bindings.reserve(Control_config.size());
	preset.name = "default";

	for (auto &item : Control_config) {
		preset.bindings.push_back(CCB(item));
	}

	Control_config_presets.push_back(preset);
};

// Map used to convert strings in the Controlconfigdefaults.tbl into their respective IoActionId
// This might also be used to save old pilotfiles, but the order will be important.
// The new .tbl shall use the IoActionId's themselves.
SCP_unordered_map<SCP_string, IoActionId> old_text = {
	{"Target Next Ship",                        TARGET_NEXT},
	{"Target Previous Ship",                    TARGET_PREV},
	{"Target Next Closest Hostile Ship",        TARGET_NEXT_CLOSEST_HOSTILE},
	{"Target Previous Closest Hostile Ship",    TARGET_PREV_CLOSEST_HOSTILE},
	{"Toggle Auto Targeting",                   TOGGLE_AUTO_TARGETING},
	{"Target Next Closest Friendly Ship",       TARGET_NEXT_CLOSEST_FRIENDLY},
	{"Target Previous Closest Friendly Ship",   TARGET_PREV_CLOSEST_FRIENDLY},
	{"Target Ship in Reticle",                  TARGET_SHIP_IN_RETICLE},
	{"Target Target's Nearest Attacker",        TARGET_CLOSEST_SHIP_ATTACKING_TARGET},
	{"Target Last Ship to Send Transmission",   TARGET_LAST_TRANMISSION_SENDER},
	{"Turn Off Targeting",                      STOP_TARGETING_SHIP},

	{"Target Subsystem in Reticle",             TARGET_SUBOBJECT_IN_RETICLE},
	{"Target Next Subsystem",                   TARGET_NEXT_SUBOBJECT},
	{"Target Previous Subsystem",               TARGET_PREV_SUBOBJECT},
	{"Turn Off Targeting of Subsystems",        STOP_TARGETING_SUBSYSTEM},

	{"Match Target Speed",                      MATCH_TARGET_SPEED},
	{"Toggle Auto Speed Matching",              TOGGLE_AUTO_MATCH_TARGET_SPEED},

	{"Fire Primary Weapon",                     FIRE_PRIMARY},
	{"Fire Secondary Weapon",                   FIRE_SECONDARY},
	{"Cycle Forward Primary Weapon",            CYCLE_NEXT_PRIMARY},
	{"Cycle Backward Primary Weapon",           CYCLE_PREV_PRIMARY},
	{"Cycle Secondary Weapon Bank",             CYCLE_SECONDARY},
	{"Cycle Secondary Weapon Firing Rate",      CYCLE_NUM_MISSLES},
	{"Launch Countermeasure",                   LAUNCH_COUNTERMEASURE},

	{"Forward Thrust",                          FORWARD_THRUST},
	{"Reverse Thrust",                          REVERSE_THRUST},
	{"Bank Left",                               BANK_LEFT},
	{"Bank Right",                              BANK_RIGHT},
	{"Pitch Forward",                           PITCH_FORWARD},
	{"Pitch Backward",                          PITCH_BACK},
	{"Turn Left",                               YAW_LEFT},
	{"Turn Right",                              YAW_RIGHT},

	{"Set Throttle to Zero",                    ZERO_THROTTLE},
	{"Set Throttle to Max",                     MAX_THROTTLE},
	{"Set Throttle to One-Third",               ONE_THIRD_THROTTLE},
	{"Set Throttle to Two-Thirds",              TWO_THIRDS_THROTTLE},
	{"Increase Throttle 5 Percent",             PLUS_5_PERCENT_THROTTLE},
	{"Decrease Throttle 5 Percent",             MINUS_5_PERCENT_THROTTLE},

	{"Attack My Target",                        ATTACK_MESSAGE},
	{"Disarm My Target",                        DISARM_MESSAGE},
	{"Disable My Target",                       DISABLE_MESSAGE},
	{"Attack My Subsystem",                     ATTACK_SUBSYSTEM_MESSAGE},
	{"Capture My Target",                       CAPTURE_MESSAGE},
	{"Engage Enemy",                            ENGAGE_MESSAGE},
	{"Form on My Wing",                         FORM_MESSAGE},
	{"Ignore My Target",                        IGNORE_MESSAGE},
	{"Protect My Target",                       PROTECT_MESSAGE},
	{"Cover Me",                                COVER_MESSAGE},
	{"Return to Base",                          WARP_MESSAGE},
	{"Rearm Me",                                REARM_MESSAGE},

	{"Target Closest Attacking Ship",           TARGET_CLOSEST_SHIP_ATTACKING_SELF},

	{"Chase View",                              VIEW_CHASE},
	{"External View",                           VIEW_EXTERNAL},
	{"Toggle External Camera Lock",             VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK},
	{"Free Look View",                          VIEW_SLEW},
	{"Current Target View",                     VIEW_OTHER_SHIP},
	{"Increase View Distance",                  VIEW_DIST_INCREASE},
	{"Decrease View Distance",                  VIEW_DIST_DECREASE},
	{"Center View",                             VIEW_CENTER},
	{"View Up",                                 PADLOCK_UP},
	{"View Rear",                               PADLOCK_DOWN},
	{"View Left",                               PADLOCK_LEFT},
	{"View Right",                              PADLOCK_RIGHT},

	{"Cycle Radar Range",                       RADAR_RANGE_CYCLE},
	{"Communications Menu",                     SQUADMSG_MENU},
	{"Show Objectives",                         SHOW_GOALS},
	{"Enter Subspace (End Mission)",            END_MISSION},
	{"Target Target's Target",                  TARGET_TARGETS_TARGET},
	{"Afterburner",                             AFTERBURNER},

	{"Increase Weapon Energy",                  INCREASE_WEAPON},
	{"Decrease Weapon Energy",                  DECREASE_WEAPON},
	{"Increase Shield Energy",                  INCREASE_SHIELD},
	{"Decrease Shield Energy",                  DECREASE_SHIELD},
	{"Increase Engine Energy",                  INCREASE_ENGINE},
	{"Decrease Engine Energy",                  DECREASE_ENGINE},
	{"Equalize Energy Settings",                ETS_EQUALIZE},

	{"Equalize Shields",                        SHIELD_EQUALIZE},
	{"Augment Forward Shield",                  SHIELD_XFER_TOP},
	{"Augment Rear Shield",                     SHIELD_XFER_BOTTOM},
	{"Augment Left Shield",                     SHIELD_XFER_LEFT},
	{"Augment Right Shield",                    SHIELD_XFER_RIGHT},
	{"Transfer Energy Laser->Shield",           XFER_SHIELD},
	{"Transfer Energy Shield->Laser",           XFER_LASER},
	{"Show Damage Popup Window",                GLIDE_WHEN_PRESSED},

	{"Glide When Pressed",                      GLIDE_WHEN_PRESSED},

	{"Bank When Pressed",                       BANK_WHEN_PRESSED},
	{"Show Nav Map",                            SHOW_NAVMAP},
	{"Add or Remove Escort",                    ADD_REMOVE_ESCORT},
	{"Clear Escort List",                       ESCORT_CLEAR},
	{"Target Next Escort Ship",                 TARGET_NEXT_ESCORT_SHIP},
	{"Target Closest Repair Ship",              TARGET_CLOSEST_REPAIR_SHIP},

	{"Target Next Uninspected Cargo",           TARGET_NEXT_UNINSPECTED_CARGO},
	{"Target Previous Uninspected Cargo",       TARGET_PREV_UNINSPECTED_CARGO},
	{"Target Newest Ship in Area",              TARGET_NEWEST_SHIP},
	{"Target Next Live Turret",                 TARGET_NEXT_LIVE_TURRET},
	{"Target Previous Live Turret",             TARGET_PREV_LIVE_TURRET},

	{"Target Next Hostile Bomb or Bomber",      TARGET_NEXT_BOMB},
	{"Target Previous Hostile Bomb or Bomber",  TARGET_PREV_BOMB},

	{"(Multiplayer) Message All",               MULTI_MESSAGE_ALL},
	{"(Multiplayer) Message Friendly",          MULTI_MESSAGE_FRIENDLY},
	{"(Multiplayer) Message Hostile",           MULTI_MESSAGE_HOSTILE},
	{"(Multiplayer) Message Target",            MULTI_MESSAGE_TARGET},
	{"(Multiplayer) Observer Zoom to Target",   MULTI_OBSERVER_ZOOM_TO},
	{"Increase Time Compression",               TIME_SPEED_UP},
	{"Decrease Time Compression",               TIME_SLOW_DOWN},
	{"Toggle High HUD Contrast",                TOGGLE_HUD_CONTRAST},
	{"(Multiplayer) Toggle Network Info",       MULTI_TOGGLE_NETINFO},
	{"(Multiplayer) Self Destruct",             MULTI_SELF_DESTRUCT},

	{"Toggle HUD",                              TOGGLE_HUD},
	{"Right Thrust",                            RIGHT_SLIDE_THRUST},
	{"Left Thrust",                             LEFT_SLIDE_THRUST},
	{"Up Thrust",                               UP_SLIDE_THRUST},
	{"Down Thrust",                             DOWN_SLIDE_THRUST},
	{"Toggle HUD Wireframe Target View",        HUD_TARGETBOX_TOGGLE_WIREFRAME},
	{"Top-Down View",                           VIEW_TOPDOWN},
	{"Target Padlock View",                     VIEW_TRACK_TARGET},

	{"Toggle Auto Pilot",                       AUTO_PILOT_TOGGLE},
	{"Cycle Nav Points",                        NAV_CYCLE},
	{"Toggle Gliding",                          TOGGLE_GLIDING},
	{"Cycle Primary Weapon Firing Rate",        CYCLE_PRIMARY_WEAPON_SEQUENCE},
	{"Custom Control 1",                        CUSTOM_CONTROL_1},
	{"Custom Control 2",                        CUSTOM_CONTROL_2},
	{"Custom Control 3",                        CUSTOM_CONTROL_3},
	{"Custom Control 4",                        CUSTOM_CONTROL_4},
	{"Custom Control 5",                        CUSTOM_CONTROL_5},
};

const char* Scan_code_text_german_u[] = {
	"",				"Esc",				"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"Akzent '",				"Eszett",				"R\xc3\xbc""cktaste",		"Tab", // NOLINT
	"Q",				"W",				"E",				"R",				"T",				"Z",				"U",				"I",
	"O",				"P",				"\xc3\x9c",				"+",				"Eingabe",			"Strg Links",			"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				"\xc3\x96",
	"\xc3\x84",				"`",				"Shift",			"#",				"Y",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"-",				"Shift",			"Num *",
	"Alt",				"Leertaste",			"Hochstell",			"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",				"Pause",			"Rollen",			"Num 7",
	"Num 8",			"Num 9",			"Num -",			"Num 4",			"Num 5",			"Num 6",			"Num +",			"Num 1",
	"Num 2",			"Num 3",			"Num 0",			"Num ,",			"",				"",				"",				"F11",
	"F12",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Num Eingabe",			"Strg Rechts",			"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Num /",			"",				"Druck",
	"Alt",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Num Lock",			"",				"Pos 1",
	"Pfeil Hoch",			"Bild Hoch",			"",				"Pfeil Links",			"",				"Pfeil Rechts",			"",				"Ende",
	"Pfeil Runter", 			"Bild Runter",			"Einfg",			"Entf",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

const char* Joy_button_text_german_u[] = {
	"Knopf 1",		"Knopf 2",		"Knopf 3",		"Knopf 4",		"Knopf 5",		"Knopf 6",
	"Knopf 7",		"Knopf 8",		"Knopf 9",		"Knopf 10",		"Knopf 11",		"Knopf 12",
	"Knopf 13",		"Knopf 14",		"Knopf 15",		"Knopf 16",		"Knopf 17",		"Knopf 18",
	"Knopf 19",		"Knopf 20",		"Knopf 21",		"Knopf 22",		"Knopf 23",		"Knopf 24",
	"Knopf 25",		"Knopf 26",		"Knopf 27",		"Knopf 28",		"Knopf 29",		"Knopf 30",
	"Knopf 31",		"Knopf 32",		"Hut Hinten",	"Hut Vorne",	"Hut Links",	"Hut Rechts"
};

const char* Scan_code_text_french_u[] = {
	"",				"\xc3\x89""chap",			"1",				"2",				"3",				"4",				"5",				"6", // NOLINT
	"7",				"8",				"9",				"0",				"-",				"=",				"Fl\xc3\xa9""che Ret.",			"Tab",  // NOLINT
	"Q",				"W",				"E",				"R",				"T",				"Y",				"U",				"I",
	"O",				"P",				"[",				"]",				"Entr\xc3\xa9""e",			"Ctrl Gauche",			"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				";",
	"'",				"`",				"Maj.",			"\\",				"Z",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"/",				"Maj.",			"Pav\xc3\xa9"" *",
	"Alt",				"Espace",			"Verr. Maj.",			"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",				"Pause",			"Arret defil",		"Pav\xc3\xa9"" 7",
	"Pav\xc3\xa9"" 8",			"Pav\xc3\xa9"" 9",			"Pav\xc3\xa9"" -",			"Pav\xc3\xa9"" 4",			"Pav\xc3\xa9"" 5",			"Pav\xc3\xa9"" 6",			"Pav\xc3\xa9"" +",			"Pav\xc3\xa9"" 1",
	"Pav\xc3\xa9"" 2",			"Pav\xc3\xa9"" 3",			"Pav\xc3\xa9"" 0",			"Pav\xc3\xa9"" .",			"",				"",				"",				"F11",
	"F12",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Pav\xc3\xa9"" Entr",			"Ctrl Droite",		"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Pav\xc3\xa9"" /",			"",				"Impr \xc3\xa9""cran",
	"Alt",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Verr num",			"",				"Orig.",
	"Fl\xc3\xa9""che Haut",			"Page Haut",			"",				"Fl\xc3\xa9""che Gauche",			"",				"Fl\xc3\xa9""che Droite",			"",			"Fin",
	"Fl\xc3\xa9""che Bas", 			"Page Bas",			"Inser",			"Suppr",			"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

const char* Joy_button_text_french_u[] = {
	"Bouton 1",		"Bouton 2",		"Bouton 3",		"Bouton 4",		"Bouton 5",		"Bouton 6",
	"Bouton 7",		"Bouton 8",		"Bouton 9",		"Bouton 10",		"Bouton 11",		"Bouton 12",
	"Bouton 13",		"Bouton 14",		"Bouton 15",		"Bouton 16",		"Bouton 17",		"Bouton 18",
	"Bouton 19",		"Bouton 20",		"Bouton 21",		"Bouton 22",		"Bouton 23",		"Bouton 24",
	"Bouton 25",		"Bouton 26",		"Bouton 27",		"Bouton 28",		"Bouton 29",		"Bouton 30",
	"Bouton 31",		"Bouton 32",		"Chapeau Arri\xc3\xa8""re",		"Chapeau Avant",		"Chapeau Gauche",		"Chapeau Droite"
};

const char* Scan_code_text_polish_u[] = {
	"",				"Esc",			"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"-",				"=",				"Backspace",	"Tab",
	"Q",				"W",				"E",				"R",				"T",				"Y",				"U",				"I",
	"O",				"P",				"[",				"]",				"Enter",			"Lewy Ctrl",	"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				";",
	"'",				"`",				"LShift",			"\\",				"Z",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"/",				"PShift",			"Num *",
	"Alt",			"Spacja",		"CapsLock",	"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",			"Pause",			"Scrlock",	"Num 7",
	"Num 8",			"Num 9",			"Num -",			"Num 4",			"Num 5",			"Num 6",			"Num +",			"Num 1",
	"Num 2",			"Num 3",			"Num 0",			"Num .",			"",				"",				"",				"F11",
	"F12",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Num Enter",	"Prawy Ctrl",	"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Num /",			"",				"PrntScr",
	"Alt",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Num Lock",		"",				"Home",
	"Kursor G\xc3\xb3""ra",		"Page Up",		"",				"Kursor Lewo",	"",				"Kursor Prawo",	"",				"End",
	"Kursor D\xc3\xb3\xc5\x82",  "Page Down",	"Insert",		"Delete",		"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

const char* Joy_button_text_polish_u[] = {
	"Przyc.1",		"Przyc.2",		"Przyc.3",		"Przyc.4",		"Przyc.5",		"Przyc.6",
	"Przyc.7",		"Przyc.8",		"Przyc.9",		"Przyc.10",	"Przyc.11",	"Przyc.12",
	"Przyc.13",	"Przyc.14",	"Przyc.15",	"Przyc.16",	"Przyc.17",	"Przyc.18",
	"Przyc.19",	"Przyc.20",	"Przyc.21",	"Przyc.22",	"Przyc.23",	"Przyc.24",
	"Przyc.25",	"Przyc.26",	"Przyc.27",	"Przyc.28",	"Przyc.29",	"Przyc.30",
	"Przyc.31",	"Przyc.32",	"Hat Ty\xc5\x82",		"Hat Prz\xc3\xb3""d",	"Hat Lewo",		"Hat Prawo"
};

//!	This is the text that is displayed on the screen for the keys a player selects
const char* Scan_code_text_english_u[] = {
	"",				"Esc",			"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"-",				"=",				"Backspace",	"Tab",
	"Q",				"W",				"E",				"R",				"T",				"Y",				"U",				"I",
	"O",				"P",				"[",				"]",				"Enter",			"Left Ctrl",	"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				";",
	"'",				"`",				"Shift",			"\\",				"Z",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"/",				"Shift",			"Pad *",
	"Alt",			"Spacebar",		"Caps Lock",	"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",			"Pause",			"Scroll Lock",	"Pad 7",
	"Pad 8",			"Pad 9",			"Pad -",			"Pad 4",			"Pad 5",			"Pad 6",			"Pad +",			"Pad 1",
	"Pad 2",			"Pad 3",			"Pad 0",			"Pad .",			"",				"",				"",				"F11",
	"F12",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Pad Enter",	"Right Ctrl",	"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Pad /",			"",				"Print Scrn",
	"Alt",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Num Lock",		"",				"Home",
	"Up Arrow",		"Page Up",		"",				"Left Arrow",	"",				"Right Arrow",	"",				"End",
	"Down Arrow",  "Page Down",	"Insert",		"Delete",		"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

const char* Joy_button_text_english_u[] = {
	"Button 1",		"Button 2",		"Button 3",		"Button 4",		"Button 5",		"Button 6",
	"Button 7",		"Button 8",		"Button 9",		"Button 10",	"Button 11",	"Button 12",
	"Button 13",	"Button 14",	"Button 15",	"Button 16",	"Button 17",	"Button 18",
	"Button 19",	"Button 20",	"Button 21",	"Button 22",	"Button 23",	"Button 24",
	"Button 25",	"Button 26",	"Button 27",	"Button 28",	"Button 29",	"Button 30",
	"Button 31",	"Button 32",	"Hat Back",		"Hat Forward",	"Hat Left",		"Hat Right"
};

const char* Scan_code_text_german[] = {
	"",				"Esc",				"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"Akzent '",				"Eszett",				"R\x81""cktaste",		"Tab", // NOLINT
	"Q",				"W",				"E",				"R",				"T",				"Z",				"U",				"I",
	"O",				"P",				"\x9A",				"+",				"Eingabe",			"Strg Links",			"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				"\x99",
	"\x8E",				"`",				"Shift",			"#",				"Y",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"-",				"Shift",			"Num *",
	"Alt",				"Leertaste",			"Hochstell",			"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",				"Pause",			"Rollen",			"Num 7",
	"Num 8",			"Num 9",			"Num -",			"Num 4",			"Num 5",			"Num 6",			"Num +",			"Num 1",
	"Num 2",			"Num 3",			"Num 0",			"Num ,",			"",				"",				"",				"F11",
	"F12",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Num Eingabe",			"Strg Rechts",			"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Num /",			"",				"Druck",
	"Alt",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Num Lock",			"",				"Pos 1",
	"Pfeil Hoch",			"Bild Hoch",			"",				"Pfeil Links",			"",				"Pfeil Rechts",			"",				"Ende",
	"Pfeil Runter", 			"Bild Runter",			"Einfg",			"Entf",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

const char* Joy_button_text_german[] = {
	"Knopf 1",		"Knopf 2",		"Knopf 3",		"Knopf 4",		"Knopf 5",		"Knopf 6",
	"Knopf 7",		"Knopf 8",		"Knopf 9",		"Knopf 10",		"Knopf 11",		"Knopf 12",
	"Knopf 13",		"Knopf 14",		"Knopf 15",		"Knopf 16",		"Knopf 17",		"Knopf 18",
	"Knopf 19",		"Knopf 20",		"Knopf 21",		"Knopf 22",		"Knopf 23",		"Knopf 24",
	"Knopf 25",		"Knopf 26",		"Knopf 27",		"Knopf 28",		"Knopf 29",		"Knopf 30",
	"Knopf 31",		"Knopf 32",		"Hut Hinten",	"Hut Vorne",	"Hut Links",	"Hut Rechts"
};

const char* Scan_code_text_french[] = {
	"",				"\x90""chap",			"1",				"2",				"3",				"4",				"5",				"6", // NOLINT
	"7",				"8",				"9",				"0",				"-",				"=",				"Fl\x82""che Ret.",			"Tab",  // NOLINT
	"Q",				"W",				"E",				"R",				"T",				"Y",				"U",				"I",
	"O",				"P",				"[",				"]",				"Entr\x82""e",			"Ctrl Gauche",			"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				";",
	"'",				"`",				"Maj.",			"\\",				"Z",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"/",				"Maj.",			"Pav\x82 *",
	"Alt",				"Espace",			"Verr. Maj.",			"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",				"Pause",			"Arret defil",		"Pav\x82 7",
	"Pav\x82 8",			"Pav\x82 9",			"Pav\x82 -",			"Pav\x82 4",			"Pav\x82 5",			"Pav\x82 6",			"Pav\x82 +",			"Pav\x82 1",
	"Pav\x82 2",			"Pav\x82 3",			"Pav\x82 0",			"Pav\x82 .",			"",				"",				"",				"F11",
	"F12",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Pav\x82 Entr",			"Ctrl Droite",		"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Pav\x82 /",			"",				"Impr \x82""cran",
	"Alt",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Verr num",			"",				"Orig.",
	"Fl\x82""che Haut",			"Page Haut",			"",				"Fl\x82""che Gauche",			"",				"Fl\x82""che Droite",			"",			"Fin",
	"Fl\x82""che Bas", 			"Page Bas",			"Inser",			"Suppr",			"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

const char* Joy_button_text_french[] = {
	"Bouton 1",		"Bouton 2",		"Bouton 3",		"Bouton 4",		"Bouton 5",		"Bouton 6",
	"Bouton 7",		"Bouton 8",		"Bouton 9",		"Bouton 10",		"Bouton 11",		"Bouton 12",
	"Bouton 13",		"Bouton 14",		"Bouton 15",		"Bouton 16",		"Bouton 17",		"Bouton 18",
	"Bouton 19",		"Bouton 20",		"Bouton 21",		"Bouton 22",		"Bouton 23",		"Bouton 24",
	"Bouton 25",		"Bouton 26",		"Bouton 27",		"Bouton 28",		"Bouton 29",		"Bouton 30",
	"Bouton 31",		"Bouton 32",		"Chapeau Arri\x8Are",		"Chapeau Avant",		"Chapeau Gauche",		"Chapeau Droite"
};

const char* Scan_code_text_polish[] = {
	"",				"Esc",			"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"-",				"=",				"Backspace",	"Tab",
	"Q",				"W",				"E",				"R",				"T",				"Y",				"U",				"I",
	"O",				"P",				"[",				"]",				"Enter",			"Lewy Ctrl",	"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				";",
	"'",				"`",				"LShift",			"\\",				"Z",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"/",				"PShift",			"Num *",
	"Alt",			"Spacja",		"CapsLock",	"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",			"Pause",			"Scrlock",	"Num 7",
	"Num 8",			"Num 9",			"Num -",			"Num 4",			"Num 5",			"Num 6",			"Num +",			"Num 1",
	"Num 2",			"Num 3",			"Num 0",			"Num .",			"",				"",				"",				"F11",
	"F12",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Num Enter",	"Prawy Ctrl",	"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Num /",			"",				"PrntScr",
	"Alt",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Num Lock",		"",				"Home",
	"Kursor G\xF3ra",		"Page Up",		"",				"Kursor Lewo",	"",				"Kursor Prawo",	"",				"End",
	"Kursor D\xF3\xB3",  "Page Down",	"Insert",		"Delete",		"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

const char* Joy_button_text_polish[] = {
	"Przyc.1",		"Przyc.2",		"Przyc.3",		"Przyc.4",		"Przyc.5",		"Przyc.6",
	"Przyc.7",		"Przyc.8",		"Przyc.9",		"Przyc.10",	"Przyc.11",	"Przyc.12",
	"Przyc.13",	"Przyc.14",	"Przyc.15",	"Przyc.16",	"Przyc.17",	"Przyc.18",
	"Przyc.19",	"Przyc.20",	"Przyc.21",	"Przyc.22",	"Przyc.23",	"Przyc.24",
	"Przyc.25",	"Przyc.26",	"Przyc.27",	"Przyc.28",	"Przyc.29",	"Przyc.30",
	"Przyc.31",	"Przyc.32",	"Hat Ty\xB3",		"Hat Prz\xF3\x64",	"Hat Lewo",		"Hat Prawo"
};

//!	This is the text that is displayed on the screen for the keys a player selects
const char* Scan_code_text_english[] = {
	"",				"Esc",			"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"-",				"=",				"Backspace",	"Tab",
	"Q",				"W",				"E",				"R",				"T",				"Y",				"U",				"I",
	"O",				"P",				"[",				"]",				"Enter",			"Left Ctrl",	"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				";",
	"'",				"`",				"Shift",			"\\",				"Z",				"X",				"C",				"V",
	"B",				"N",				"M",				",",				".",				"/",				"Shift",			"Pad *",
	"Alt",			"Spacebar",		"Caps Lock",	"F1",				"F2",				"F3",				"F4",				"F5",

	"F6",				"F7",				"F8",				"F9",				"F10",			"Pause",			"Scroll Lock",	"Pad 7",
	"Pad 8",			"Pad 9",			"Pad -",			"Pad 4",			"Pad 5",			"Pad 6",			"Pad +",			"Pad 1",
	"Pad 2",			"Pad 3",			"Pad 0",			"Pad .",			"",				"",				"",				"F11",
	"F12",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"Pad Enter",	"Right Ctrl",	"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"Pad /",			"",				"Print Scrn",
	"Alt",			"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"Num Lock",		"",				"Home",
	"Up Arrow",		"Page Up",		"",				"Left Arrow",	"",				"Right Arrow",	"",				"End",
	"Down Arrow",  "Page Down",	"Insert",		"Delete",		"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",

	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
	"",				"",				"",				"",				"",				"",				"",				"",
};

const char* Joy_button_text_english[] = {
	"Button 1",		"Button 2",		"Button 3",		"Button 4",		"Button 5",		"Button 6",
	"Button 7",		"Button 8",		"Button 9",		"Button 10",	"Button 11",	"Button 12",
	"Button 13",	"Button 14",	"Button 15",	"Button 16",	"Button 17",	"Button 18",
	"Button 19",	"Button 20",	"Button 21",	"Button 22",	"Button 23",	"Button 24",
	"Button 25",	"Button 26",	"Button 27",	"Button 28",	"Button 29",	"Button 30",
	"Button 31",	"Button 32",	"Hat Back",		"Hat Forward",	"Hat Left",		"Hat Right"
};

const char **Scan_code_text = Scan_code_text_english;
const char **Joy_button_text = Joy_button_text_english;

int translate_key_to_index(const char *key, bool find_override)
{
	unsigned int max_scan_codes;
	unsigned int i;
	int index = -1;
	bool alt = false;
	bool shift = false;

	max_scan_codes = sizeof(Scan_code_text_english) / sizeof(char *);

	// look for modifiers
	Assert(key);
	if (!strnicmp(key, "Alt", 3)) {
		alt = true;
		key += 3;
		if (*key)
			key++;
	}

	if (!strnicmp(key, "Shift", 5)) {
		shift = true;
		key += 5;
		if (*key)
			key++;
	}

	// look up index for default key
	if (*key) {
		for (i=0; i<max_scan_codes; i++)
			if (!stricmp(key, Scan_code_text_english[i])) {
				index = i;
				break;
			}

		if (i == max_scan_codes)
			return -1;

		if (shift)
			index |= KEY_SHIFTED;
		if (alt)
			index |= KEY_ALTED;

		// convert scancode to Control_config index
		if (find_override) {
			for (i = 0; i < Control_config.size(); ++i) {
				if (!Control_config[i].disabled && (Control_config[i].get_btn(CID_KEYBOARD) == index)) {
					index = static_cast<int>(i);
					break;
				}
			}
		} else {
			const auto& default_bindings = Control_config_presets[0].bindings;
			for (i = 0; i < Control_config.size(); ++i) {
				if (!Control_config[i].disabled && (default_bindings[i].get_btn(CID_KEYBOARD) == index)) {
					index = static_cast<int>(i);
					break;
				}
			}
		}

		if (i == Control_config.size())
			return -1;

		return index;
	}

	return -1;
}

char *translate_key(char *key)
{
	int index = -1, key_code = -1, joy_code = -1;
	const char *key_text = NULL;
	const char *joy_text = NULL;

	static char text[40] = {"None"};

	index = translate_key_to_index(key, false);
	if (index < 0) {
		return NULL;
	}

	key_code = Control_config[index].get_btn(CID_KEYBOARD);
	joy_code = Control_config[index].get_btn(CID_JOY0);

	Failed_key_index = index;

	if (key_code >= 0) {
		key_text = textify_scancode(key_code);
	}

	if (joy_code >= 0) {
		joy_text = Joy_button_text[joy_code];
	}

	// both key and joystick button are mapped to this control
	if ((key_code >= 0 ) && (joy_code >= 0) ) {
		strcpy_s(text, key_text);
		strcat_s(text, " ");
		strcat_s(text, XSTR("or", 1638));
		strcat_s(text, " ");
		strcat_s(text, joy_text);
	}
	// if we only have one
	else if (key_code >= 0 ) {
		strcpy_s(text, key_text);
	}
	else if (joy_code >= 0) {
		strcpy_s(text, joy_text);
	}
	else {
		strcpy_s(text, "None");
	}

	return text;
}

const char *textify_scancode(int code)
{
	static char text[40];

	if (code < 0)
		return "None";

	int keycode = code & KEY_MASK;

	*text = 0;
	if (code & KEY_ALTED && !(keycode == KEY_LALT || keycode == KEY_RALT)) {
		if(Lcl_gr){
			strcat_s(text, "Alt-");
		} else if(Lcl_fr){
			strcat_s(text, "Alt-");
		} else {
			strcat_s(text, "Alt-");
		}
	}

	if (code & KEY_SHIFTED && !(keycode == KEY_LSHIFT || keycode == KEY_RSHIFT)) {
		if(Lcl_gr){
			strcat_s(text, "Shift-");
		} else if(Lcl_fr){
			strcat_s(text, "Maj.-");
		} else {
			strcat_s(text, "Shift-");
		}
	}

	strcat_s(text, Scan_code_text[keycode]);
	return text;
}

const char *textify_scancode_universal(int code)
{
	if (code < 0)
		return "None";

	int keycode = code & KEY_MASK;

	static char text[40];
	*text = 0;
	if (code & KEY_ALTED && !(keycode == KEY_LALT || keycode == KEY_RALT)) {
		strcat_s(text, "Alt-");
	}

	if (code & KEY_SHIFTED && !(keycode == KEY_LSHIFT || keycode == KEY_RSHIFT)) {
		strcat_s(text, "Shift-");
	}

	// Always use the english version here
	strcat_s(text, Scan_code_text_english[keycode]);
	return text;
}
//XSTR:ON

void control_config_common_load_overrides();

// initialize common control config stuff - call at game startup after localization has been initialized
void control_config_common_init()
{
	control_config_common_init_bindings();

	for (int i=0; i<CCFG_MAX; i++) {
		Control_config[i].continuous_ongoing = false;
	}

	control_config_common_load_overrides();

	if (Unicode_text_mode) {
		if (Lcl_gr) {
			Scan_code_text = Scan_code_text_german_u;
			Joy_button_text = Joy_button_text_german_u;
		}
		else if (Lcl_fr) {
			Scan_code_text = Scan_code_text_french_u;
			Joy_button_text = Joy_button_text_french_u;
		}
		else if (Lcl_pl) {
			Scan_code_text = Scan_code_text_polish_u;
			Joy_button_text = Joy_button_text_polish_u;
		}
		else {
			Scan_code_text = Scan_code_text_english_u;
			Joy_button_text = Joy_button_text_english_u;
		}
	}
	else {
		if (Lcl_gr) {
			Scan_code_text = Scan_code_text_german;
			Joy_button_text = Joy_button_text_german;
		}
		else if (Lcl_fr) {
			Scan_code_text = Scan_code_text_french;
			Joy_button_text = Joy_button_text_french;
		}
		else if (Lcl_pl) {
			Scan_code_text = Scan_code_text_polish;
			Joy_button_text = Joy_button_text_polish;
		}
		else {
			Scan_code_text = Scan_code_text_english;
			Joy_button_text = Joy_button_text_english;
		}
	}

}

/*
 * @brief close any common control config stuff, called at game_shutdown()
 */
void control_config_common_close()
{
	/*
	// only need to worry control presets for now
	for (auto ii = Control_config_presets.cbegin(); ii != Control_config_presets.cend(); ++ii) {
		delete[] * ii;
	}
	*/
}


#include <map>
#include <string>
SCP_map<SCP_string, short> mKeyNameToVal;
SCP_map<SCP_string, CC_type> mCCTypeNameToVal;
SCP_map<SCP_string, char> mCCTabNameToVal;
SCP_map<SCP_string, IoActionId> mActionToVal;

/*! Helper function to LoadEnumsIntoMaps(), Loads the Keyboard definitions/enumerations into mKeyNameToVal
*/
void LoadEnumsIntoKeyMap() {
	// Dirty macro hack :D
#define ADD_ENUM_TO_KEY_MAP(Enum) mKeyNameToVal[#Enum] = (Enum);

	ADD_ENUM_TO_KEY_MAP(KEY_SHIFTED)
		/*
		ADD_ENUM_TO_KEY_MAP(KEY_ALTED)
		ADD_ENUM_TO_KEY_MAP(KEY_CTRLED)
		ADD_ENUM_TO_KEY_MAP(KEY_DEBUGGED)
		ADD_ENUM_TO_KEY_MAP(KEY_DEBUGGED1)
		ADD_ENUM_TO_KEY_MAP(KEY_MASK)

		ADD_ENUM_TO_KEY_MAP(KEY_DEBUG_KEY)
		*/
		ADD_ENUM_TO_KEY_MAP(KEY_0)
		ADD_ENUM_TO_KEY_MAP(KEY_1)
		ADD_ENUM_TO_KEY_MAP(KEY_2)
		ADD_ENUM_TO_KEY_MAP(KEY_3)
		ADD_ENUM_TO_KEY_MAP(KEY_4)
		ADD_ENUM_TO_KEY_MAP(KEY_5)
		ADD_ENUM_TO_KEY_MAP(KEY_6)
		ADD_ENUM_TO_KEY_MAP(KEY_7)
		ADD_ENUM_TO_KEY_MAP(KEY_8)
		ADD_ENUM_TO_KEY_MAP(KEY_9)

		ADD_ENUM_TO_KEY_MAP(KEY_A)
		ADD_ENUM_TO_KEY_MAP(KEY_B)
		ADD_ENUM_TO_KEY_MAP(KEY_C)
		ADD_ENUM_TO_KEY_MAP(KEY_D)
		ADD_ENUM_TO_KEY_MAP(KEY_E)
		ADD_ENUM_TO_KEY_MAP(KEY_F)
		ADD_ENUM_TO_KEY_MAP(KEY_G)
		ADD_ENUM_TO_KEY_MAP(KEY_H)
		ADD_ENUM_TO_KEY_MAP(KEY_I)
		ADD_ENUM_TO_KEY_MAP(KEY_J)
		ADD_ENUM_TO_KEY_MAP(KEY_K)
		ADD_ENUM_TO_KEY_MAP(KEY_L)
		ADD_ENUM_TO_KEY_MAP(KEY_M)
		ADD_ENUM_TO_KEY_MAP(KEY_N)
		ADD_ENUM_TO_KEY_MAP(KEY_O)
		ADD_ENUM_TO_KEY_MAP(KEY_P)
		ADD_ENUM_TO_KEY_MAP(KEY_Q)
		ADD_ENUM_TO_KEY_MAP(KEY_R)
		ADD_ENUM_TO_KEY_MAP(KEY_S)
		ADD_ENUM_TO_KEY_MAP(KEY_T)
		ADD_ENUM_TO_KEY_MAP(KEY_U)
		ADD_ENUM_TO_KEY_MAP(KEY_V)
		ADD_ENUM_TO_KEY_MAP(KEY_W)
		ADD_ENUM_TO_KEY_MAP(KEY_X)
		ADD_ENUM_TO_KEY_MAP(KEY_Y)
		ADD_ENUM_TO_KEY_MAP(KEY_Z)

		ADD_ENUM_TO_KEY_MAP(KEY_MINUS)
		ADD_ENUM_TO_KEY_MAP(KEY_EQUAL)
		ADD_ENUM_TO_KEY_MAP(KEY_DIVIDE)
		ADD_ENUM_TO_KEY_MAP(KEY_SLASH)
		ADD_ENUM_TO_KEY_MAP(KEY_SLASH_UK)
		ADD_ENUM_TO_KEY_MAP(KEY_COMMA)
		ADD_ENUM_TO_KEY_MAP(KEY_PERIOD)
		ADD_ENUM_TO_KEY_MAP(KEY_SEMICOL)

		ADD_ENUM_TO_KEY_MAP(KEY_LBRACKET)
		ADD_ENUM_TO_KEY_MAP(KEY_RBRACKET)

		ADD_ENUM_TO_KEY_MAP(KEY_RAPOSTRO)
		ADD_ENUM_TO_KEY_MAP(KEY_LAPOSTRO)

		ADD_ENUM_TO_KEY_MAP(KEY_ESC)
		ADD_ENUM_TO_KEY_MAP(KEY_ENTER)
		ADD_ENUM_TO_KEY_MAP(KEY_BACKSP)
		ADD_ENUM_TO_KEY_MAP(KEY_TAB)
		ADD_ENUM_TO_KEY_MAP(KEY_SPACEBAR)

		ADD_ENUM_TO_KEY_MAP(KEY_NUMLOCK)
		ADD_ENUM_TO_KEY_MAP(KEY_SCROLLOCK)
		ADD_ENUM_TO_KEY_MAP(KEY_CAPSLOCK)

		ADD_ENUM_TO_KEY_MAP(KEY_LSHIFT)
		ADD_ENUM_TO_KEY_MAP(KEY_RSHIFT)

		ADD_ENUM_TO_KEY_MAP(KEY_LALT)
		ADD_ENUM_TO_KEY_MAP(KEY_RALT)

		ADD_ENUM_TO_KEY_MAP(KEY_LCTRL)
		ADD_ENUM_TO_KEY_MAP(KEY_RCTRL)

		ADD_ENUM_TO_KEY_MAP(KEY_F1)
		ADD_ENUM_TO_KEY_MAP(KEY_F2)
		ADD_ENUM_TO_KEY_MAP(KEY_F3)
		ADD_ENUM_TO_KEY_MAP(KEY_F4)
		ADD_ENUM_TO_KEY_MAP(KEY_F5)
		ADD_ENUM_TO_KEY_MAP(KEY_F6)
		ADD_ENUM_TO_KEY_MAP(KEY_F7)
		ADD_ENUM_TO_KEY_MAP(KEY_F8)
		ADD_ENUM_TO_KEY_MAP(KEY_F9)
		ADD_ENUM_TO_KEY_MAP(KEY_F10)
		ADD_ENUM_TO_KEY_MAP(KEY_F11)
		ADD_ENUM_TO_KEY_MAP(KEY_F12)

		ADD_ENUM_TO_KEY_MAP(KEY_PAD0)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD1)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD2)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD3)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD4)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD5)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD6)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD7)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD8)
		ADD_ENUM_TO_KEY_MAP(KEY_PAD9)
		ADD_ENUM_TO_KEY_MAP(KEY_PADMINUS)
		ADD_ENUM_TO_KEY_MAP(KEY_PADPLUS)
		ADD_ENUM_TO_KEY_MAP(KEY_PADPERIOD)
		ADD_ENUM_TO_KEY_MAP(KEY_PADDIVIDE)
		ADD_ENUM_TO_KEY_MAP(KEY_PADMULTIPLY)
		ADD_ENUM_TO_KEY_MAP(KEY_PADENTER)

		ADD_ENUM_TO_KEY_MAP(KEY_INSERT)
		ADD_ENUM_TO_KEY_MAP(KEY_HOME)
		ADD_ENUM_TO_KEY_MAP(KEY_PAGEUP)
		ADD_ENUM_TO_KEY_MAP(KEY_DELETE)
		ADD_ENUM_TO_KEY_MAP(KEY_END)
		ADD_ENUM_TO_KEY_MAP(KEY_PAGEDOWN)
		ADD_ENUM_TO_KEY_MAP(KEY_UP)
		ADD_ENUM_TO_KEY_MAP(KEY_DOWN)
		ADD_ENUM_TO_KEY_MAP(KEY_LEFT)
		ADD_ENUM_TO_KEY_MAP(KEY_RIGHT)

		ADD_ENUM_TO_KEY_MAP(KEY_PRINT_SCRN)
		ADD_ENUM_TO_KEY_MAP(KEY_PAUSE)
		ADD_ENUM_TO_KEY_MAP(KEY_BREAK)

#undef ADD_ENUM_TO_KEY_MAP
}

/*! Helper function to LoadEnumsIntoMaps(), Loads the Control Types enumerations into mCCTypeNameToVal
 */
void LoadEnumsIntoCCTypeMap() {
	// Dirty macro hack :D
#define ADD_ENUM_TO_CCTYPE_MAP(Enum) mCCTypeNameToVal[#Enum] = (Enum);

	ADD_ENUM_TO_CCTYPE_MAP(CC_TYPE_TRIGGER)
		ADD_ENUM_TO_CCTYPE_MAP(CC_TYPE_CONTINUOUS)

#undef ADD_ENUM_TO_CCTYPE_MAP
}

/*! Helper function to LoadEnumsIntoMaps(), Loads the Control Tabs enumerations into mCCTabNameToVal
 */
void LoadEnumsIntoCCTabMap() {
	// Dirty macro hack :D
#define ADD_ENUM_TO_CCTAB_MAP(Enum) mCCTabNameToVal[#Enum] = (Enum);

	ADD_ENUM_TO_CCTAB_MAP(NO_TAB)
	ADD_ENUM_TO_CCTAB_MAP(TARGET_TAB)
	ADD_ENUM_TO_CCTAB_MAP(SHIP_TAB)
	ADD_ENUM_TO_CCTAB_MAP(WEAPON_TAB)
	ADD_ENUM_TO_CCTAB_MAP(COMPUTER_TAB)

#undef ADD_ENUM_TO_CCTAB_MAP
}

/*! Helper function to LoadEnumsIntoMaps(), Loads the IoActionId enums into mActionToVal
 */
void LoadEnumsIntoActionMap() {
#define ADD_ENUM_TO_ACTION_MAP(Enum) mActionToVal[#Enum] = (Enum);
	ADD_ENUM_TO_ACTION_MAP(TARGET_NEXT)
	ADD_ENUM_TO_ACTION_MAP(TARGET_PREV)
	ADD_ENUM_TO_ACTION_MAP(TARGET_NEXT_CLOSEST_HOSTILE)
	ADD_ENUM_TO_ACTION_MAP(TARGET_PREV_CLOSEST_HOSTILE)
	ADD_ENUM_TO_ACTION_MAP(TOGGLE_AUTO_TARGETING)
	ADD_ENUM_TO_ACTION_MAP(TARGET_NEXT_CLOSEST_FRIENDLY)
	ADD_ENUM_TO_ACTION_MAP(TARGET_PREV_CLOSEST_FRIENDLY)
	ADD_ENUM_TO_ACTION_MAP(TARGET_SHIP_IN_RETICLE)
	ADD_ENUM_TO_ACTION_MAP(TARGET_CLOSEST_SHIP_ATTACKING_TARGET)
	ADD_ENUM_TO_ACTION_MAP(TARGET_LAST_TRANMISSION_SENDER)
	ADD_ENUM_TO_ACTION_MAP(STOP_TARGETING_SHIP)

	ADD_ENUM_TO_ACTION_MAP(TARGET_SUBOBJECT_IN_RETICLE)
	ADD_ENUM_TO_ACTION_MAP(TARGET_NEXT_SUBOBJECT)
	ADD_ENUM_TO_ACTION_MAP(TARGET_PREV_SUBOBJECT)
	ADD_ENUM_TO_ACTION_MAP(STOP_TARGETING_SUBSYSTEM)

	ADD_ENUM_TO_ACTION_MAP(MATCH_TARGET_SPEED)
	ADD_ENUM_TO_ACTION_MAP(TOGGLE_AUTO_MATCH_TARGET_SPEED)

	ADD_ENUM_TO_ACTION_MAP(FIRE_PRIMARY)
	ADD_ENUM_TO_ACTION_MAP(FIRE_SECONDARY)
	ADD_ENUM_TO_ACTION_MAP(CYCLE_NEXT_PRIMARY)
	ADD_ENUM_TO_ACTION_MAP(CYCLE_PREV_PRIMARY)
	ADD_ENUM_TO_ACTION_MAP(CYCLE_SECONDARY)
	ADD_ENUM_TO_ACTION_MAP(CYCLE_NUM_MISSLES)
	ADD_ENUM_TO_ACTION_MAP(LAUNCH_COUNTERMEASURE)

	ADD_ENUM_TO_ACTION_MAP(FORWARD_THRUST)
	ADD_ENUM_TO_ACTION_MAP(REVERSE_THRUST)
	ADD_ENUM_TO_ACTION_MAP(BANK_LEFT)
	ADD_ENUM_TO_ACTION_MAP(BANK_RIGHT)
	ADD_ENUM_TO_ACTION_MAP(PITCH_FORWARD)
	ADD_ENUM_TO_ACTION_MAP(PITCH_BACK)
	ADD_ENUM_TO_ACTION_MAP(YAW_LEFT)
	ADD_ENUM_TO_ACTION_MAP(YAW_RIGHT)

	ADD_ENUM_TO_ACTION_MAP(ZERO_THROTTLE)
	ADD_ENUM_TO_ACTION_MAP(MAX_THROTTLE)
	ADD_ENUM_TO_ACTION_MAP(ONE_THIRD_THROTTLE)
	ADD_ENUM_TO_ACTION_MAP(TWO_THIRDS_THROTTLE)
	ADD_ENUM_TO_ACTION_MAP(PLUS_5_PERCENT_THROTTLE)
	ADD_ENUM_TO_ACTION_MAP(MINUS_5_PERCENT_THROTTLE)

	ADD_ENUM_TO_ACTION_MAP(ATTACK_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(DISARM_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(DISABLE_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(ATTACK_SUBSYSTEM_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(CAPTURE_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(ENGAGE_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(FORM_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(IGNORE_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(PROTECT_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(COVER_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(WARP_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(REARM_MESSAGE)
	ADD_ENUM_TO_ACTION_MAP(TARGET_CLOSEST_SHIP_ATTACKING_SELF)

	ADD_ENUM_TO_ACTION_MAP(VIEW_CHASE)
	ADD_ENUM_TO_ACTION_MAP(VIEW_EXTERNAL)
	ADD_ENUM_TO_ACTION_MAP(VIEW_EXTERNAL_TOGGLE_CAMERA_LOCK)
	ADD_ENUM_TO_ACTION_MAP(VIEW_SLEW)
	ADD_ENUM_TO_ACTION_MAP(VIEW_OTHER_SHIP)
	ADD_ENUM_TO_ACTION_MAP(VIEW_DIST_INCREASE)
	ADD_ENUM_TO_ACTION_MAP(VIEW_DIST_DECREASE)
	ADD_ENUM_TO_ACTION_MAP(VIEW_CENTER)
	ADD_ENUM_TO_ACTION_MAP(PADLOCK_UP)
	ADD_ENUM_TO_ACTION_MAP(PADLOCK_DOWN)
	ADD_ENUM_TO_ACTION_MAP(PADLOCK_LEFT)
	ADD_ENUM_TO_ACTION_MAP(PADLOCK_RIGHT)

	ADD_ENUM_TO_ACTION_MAP(RADAR_RANGE_CYCLE)
	ADD_ENUM_TO_ACTION_MAP(SQUADMSG_MENU)
	ADD_ENUM_TO_ACTION_MAP(SHOW_GOALS)
	ADD_ENUM_TO_ACTION_MAP(END_MISSION)
	ADD_ENUM_TO_ACTION_MAP(TARGET_TARGETS_TARGET)
	ADD_ENUM_TO_ACTION_MAP(AFTERBURNER)

	ADD_ENUM_TO_ACTION_MAP(INCREASE_WEAPON)
	ADD_ENUM_TO_ACTION_MAP(DECREASE_WEAPON)
	ADD_ENUM_TO_ACTION_MAP(INCREASE_SHIELD)
	ADD_ENUM_TO_ACTION_MAP(DECREASE_SHIELD)
	ADD_ENUM_TO_ACTION_MAP(INCREASE_ENGINE)
	ADD_ENUM_TO_ACTION_MAP(DECREASE_ENGINE)
	ADD_ENUM_TO_ACTION_MAP(ETS_EQUALIZE)
	ADD_ENUM_TO_ACTION_MAP(SHIELD_EQUALIZE)
	ADD_ENUM_TO_ACTION_MAP(SHIELD_XFER_TOP)
	ADD_ENUM_TO_ACTION_MAP(SHIELD_XFER_BOTTOM)
	ADD_ENUM_TO_ACTION_MAP(SHIELD_XFER_LEFT)
	ADD_ENUM_TO_ACTION_MAP(SHIELD_XFER_RIGHT)
	ADD_ENUM_TO_ACTION_MAP(XFER_SHIELD)
	ADD_ENUM_TO_ACTION_MAP(XFER_LASER)

	ADD_ENUM_TO_ACTION_MAP(GLIDE_WHEN_PRESSED)

	ADD_ENUM_TO_ACTION_MAP(BANK_WHEN_PRESSED)
	ADD_ENUM_TO_ACTION_MAP(SHOW_NAVMAP)
	ADD_ENUM_TO_ACTION_MAP(ADD_REMOVE_ESCORT)
	ADD_ENUM_TO_ACTION_MAP(ESCORT_CLEAR)

	ADD_ENUM_TO_ACTION_MAP(TARGET_NEXT_ESCORT_SHIP)
	ADD_ENUM_TO_ACTION_MAP(TARGET_CLOSEST_REPAIR_SHIP)
	ADD_ENUM_TO_ACTION_MAP(TARGET_NEXT_UNINSPECTED_CARGO)
	ADD_ENUM_TO_ACTION_MAP(TARGET_PREV_UNINSPECTED_CARGO)
	ADD_ENUM_TO_ACTION_MAP(TARGET_NEWEST_SHIP)

	ADD_ENUM_TO_ACTION_MAP(TARGET_NEXT_LIVE_TURRET)
	ADD_ENUM_TO_ACTION_MAP(TARGET_PREV_LIVE_TURRET)

	ADD_ENUM_TO_ACTION_MAP(TARGET_NEXT_BOMB)
	ADD_ENUM_TO_ACTION_MAP(TARGET_PREV_BOMB)

	ADD_ENUM_TO_ACTION_MAP(MULTI_MESSAGE_ALL)
	ADD_ENUM_TO_ACTION_MAP(MULTI_MESSAGE_FRIENDLY)
	ADD_ENUM_TO_ACTION_MAP(MULTI_MESSAGE_HOSTILE)
	ADD_ENUM_TO_ACTION_MAP(MULTI_MESSAGE_TARGET)

	ADD_ENUM_TO_ACTION_MAP(MULTI_OBSERVER_ZOOM_TO)

	ADD_ENUM_TO_ACTION_MAP(TIME_SPEED_UP)
	ADD_ENUM_TO_ACTION_MAP(TIME_SLOW_DOWN)

	ADD_ENUM_TO_ACTION_MAP(TOGGLE_HUD_CONTRAST)

	ADD_ENUM_TO_ACTION_MAP(MULTI_TOGGLE_NETINFO)

	ADD_ENUM_TO_ACTION_MAP(MULTI_SELF_DESTRUCT)

	ADD_ENUM_TO_ACTION_MAP(TOGGLE_HUD)
	ADD_ENUM_TO_ACTION_MAP(RIGHT_SLIDE_THRUST)
	ADD_ENUM_TO_ACTION_MAP(LEFT_SLIDE_THRUST)
	ADD_ENUM_TO_ACTION_MAP(UP_SLIDE_THRUST)
	ADD_ENUM_TO_ACTION_MAP(DOWN_SLIDE_THRUST)
	ADD_ENUM_TO_ACTION_MAP(HUD_TARGETBOX_TOGGLE_WIREFRAME)
	ADD_ENUM_TO_ACTION_MAP(VIEW_TOPDOWN)
	ADD_ENUM_TO_ACTION_MAP(VIEW_TRACK_TARGET)

	ADD_ENUM_TO_ACTION_MAP(AUTO_PILOT_TOGGLE)
	ADD_ENUM_TO_ACTION_MAP(NAV_CYCLE)

	ADD_ENUM_TO_ACTION_MAP(TOGGLE_GLIDING)

	ADD_ENUM_TO_ACTION_MAP(CYCLE_PRIMARY_WEAPON_SEQUENCE)

	ADD_ENUM_TO_ACTION_MAP(CUSTOM_CONTROL_1)
	ADD_ENUM_TO_ACTION_MAP(CUSTOM_CONTROL_2)
	ADD_ENUM_TO_ACTION_MAP(CUSTOM_CONTROL_3)
	ADD_ENUM_TO_ACTION_MAP(CUSTOM_CONTROL_4)
	ADD_ENUM_TO_ACTION_MAP(CUSTOM_CONTROL_5)
#undef ADD_ENUM_TO_ACTION_MAP

	Assertion(mActionToVal.size() == CCFG_MAX, "Missing or unknown IoActionId's detected.");
}

/*! Loads the various control configuration maps to allow the parsing functions to appropriately map string tokns to
* their associated enumerations. The string tokens in the controlconfigdefaults.tbl match directly to their names in
* the C++ code, such as "KEY_5" in the .tbl mapping to the #define KEY_5 value
*/
void LoadEnumsIntoMaps() {
	LoadEnumsIntoKeyMap();
	LoadEnumsIntoCCTypeMap();
	LoadEnumsIntoCCTabMap();
	LoadEnumsIntoActionMap();
}


/**
 * @brief Searches Control_config for a control that has the given ::text
 *
 * @returns The IoActionId of the control if successful, or
 * @returns Control_config.size() if unsuccessful
 *
 * @details This also checks the old hardcoded names for backward compat.  However, since the new ::text is overridable
 * by controlconfigdefaults.tbl, any references to the new hardcoded names may fail after they've been changed in the
 * default preset
 */
size_t find_control_by_text(SCP_string& text) {
	size_t item_id;

	// Search the current ::text
	for (item_id = 0; item_id < Control_config.size(); ++item_id) {
		if (text == Control_config[item_id].text) {
			return item_id;
		}
	}

	// Not found in new text, search old ::text
	try {
		item_id = old_text.at(text);

	} catch (const std::out_of_range &) {
		// Couldn't find in old ::text
		return Control_config.size();

	} // else, Found in old ::text

	return item_id;
}

/**
 * @brief Reads a section in controlconfigdefaults.tbl.
 *
 * @param[in] s Value of a call to optional_string_either(); 0 = "ControlConfigOverride" 1 = "ControlConfigPreset"
 *
 * @details ControlConfigPresets are read in the exact same manner as ControlConfigOverrides, however only the bindings are available for modification
 */
void control_config_common_read_section(int s, bool first_override) {
	CC_preset new_preset;

	// Init the new preset with the default (which is the 0th element Control_config_preset vector
	auto& default_preset = Control_config_presets[0];
	auto& default_bindings = default_preset.bindings;

	new_preset.bindings.clear();

	if (s == 0) {
		// ControlConfigOverride
		// Copy in defaults to have them overridden
		std::copy(default_bindings.begin(), default_bindings.end(), std::back_inserter(new_preset.bindings));

	} else {
		// ControlConfigPreset
		// Start with clean slate
		new_preset.bindings.resize(default_bindings.size());
	}
	

	// Assign name to the preset
	if (optional_string("$Name:")) {
		SCP_string name;
		stuff_string_line(name);
		drop_leading_white_space(name);
		new_preset.name = name;

		auto it = std::find_if(Control_config_presets.begin(), Control_config_presets.end(), [&name](CC_preset& S) {return S.name == name;});
		if ((s != 0) && (it != Control_config_presets.end())) {
			// Error: This isn't an override, and we found a preset with the same name
			throw parse::ParseException("Preset group found with same name as existing group: " + name);
		}

	} else {
		if (first_override) {
			new_preset.name = "default";
		} else {
			new_preset.name = "<unnamed preset>";
		}
	}

	// Read the section
	while (required_string_either("#End", "$Bind Name:")) {
		SCP_string szTempBuffer;

		required_string("$Bind Name:");
		stuff_string(szTempBuffer, F_NAME);

		// Find the control
		size_t item_id = find_control_by_text(szTempBuffer);

		if (item_id == Control_config.size()) {
			// Control wasn't found
			// Warning: Not Found
			error_display(0, "Unknown Bind Name: %s\n", szTempBuffer.c_str());

			// Try to resume
			if (!skip_to_start_of_string_either("#End", "$Bind Name:")) {
				// Couldn't find next binding or end. Fail
				throw parse::ParseException("Could not find #End or $Bind Name");

			}; // Found next binding or end, continue loop
			continue;
		}

		// Assign the various attributes to this control
		int iTemp;
		auto  item = &Control_config[item_id];
		auto& new_binding = new_preset.bindings[item_id];
		short key = new_binding.get_btn(CID_KEYBOARD);

		// Key assignment and modifiers
		if (optional_string("$Key Default:")) {
			if (optional_string("NONE")) {
				new_binding.take(CC_bind(CID_KEYBOARD, -1), -1);
			} else {
				stuff_string(szTempBuffer, F_NAME);
				key = mKeyNameToVal[szTempBuffer];
			}
		}

		if (optional_string("$Key Mod Shift:")) {
			stuff_int(&iTemp);
			key |= (iTemp == 1) ? KEY_SHIFTED : 0;
		}

		if (optional_string("$Key Mod Alt:")) {
			stuff_int(&iTemp);
			key |= (iTemp == 1) ? KEY_ALTED : 0;
		}

		if (optional_string("$Key Mod Ctrl:")) {
			stuff_int(&iTemp);
			key |= (iTemp == 1) ? KEY_CTRLED : 0;
		}

		new_binding.take(CC_bind(CID_KEYBOARD, key), 0);

		// Joy btn assignment
		if (optional_string("$Joy Default:")) {
			stuff_int(&iTemp);
			new_binding.take(CC_bind(CID_JOY0, static_cast<short>(iTemp)), 1);
		}

		// Section is #ControlConfigOverride
		// If the section is #ControlConfigPreset, then any of these options will throw an error
		if (s == 0) {
			// Config menu options
			if (optional_string("$Category:")) {
				stuff_string(szTempBuffer, F_NAME);
				item->tab = mCCTabNameToVal[szTempBuffer];
			}

			if (optional_string("$Text:")) {
				stuff_string(item->text, F_NAME);
				item->indexXSTR = 0;
			}

			if (optional_string("$Has XStr:")) {
				stuff_int(&iTemp);
				item->indexXSTR = iTemp;
			}

			if (optional_string("$Type:")) {
				stuff_string(szTempBuffer, F_NAME);
				item->type = mCCTypeNameToVal[szTempBuffer];
			}

			// Gameplay options
			if (optional_string("+Disable")) {
				item->disabled = true;
			}

			if (optional_string("$Disable:")) {
				stuff_boolean(&item->disabled);
			}
		}
	}

	required_string("#End");

	if ((s == 0) && (new_preset.name == default_preset.name)) {
		// If this is an override section, and the preset name is the same as the hardcoded defaults, override the defaults
		auto& new_bindings = new_preset.bindings;
		std::copy(new_bindings.begin(), new_bindings.end(), default_bindings.begin());

	} else {
		// Add new preset
		Control_config_presets.push_back(new_preset);
	}
};

/**
 * @brief Reads controlconfigdefaults.tbl.  Adds a preset to Control_config_presets or overrides the hardcoded default preset
 */
void control_config_common_read_tbl() {
	if (cf_exists_full("controlconfigdefaults.tbl", CF_TYPE_TABLES)) {
		read_file_text("controlconfigdefaults.tbl", CF_TYPE_TABLES);
	} else {
		read_file_text_from_default(defaults_get_file("controlconfigdefaults.tbl"));
	}

	reset_parse();

	// start parsing
	bool found_override = false;
	int s = optional_string_either("#ControlConfigOverride", "#ControlConfigPreset");
	while (s != -1) {
		// Found section header, parse it
		control_config_common_read_section(s, s == 0 && !found_override);
		if (s == 0 && !found_override) {
			found_override = true;
		}

		s = optional_string_either("#ControlConfigOverride", "#ControlConfigPreset");
	}
}

/**
 * @brief Writes the default preset into controlconfigdefaults.tbl
 *
 * @param[in] overwrite If true, overwrite any existing .tbl
 *
 * @returns 0 if successful
 * @returns 1 if not successful - nothing was saved
 */
int control_config_common_write_tbl(bool overwrite = false) {
	if (cf_exists_full("controlconfigdefaults.tbl", CF_TYPE_TABLES) && !overwrite) {
		// File exists, and we're told to not overwrite it. Bail
		return 1;
	}

	CFILE* cfile = cfopen("controlconfigdefaults.tbl", "w", CFILE_NORMAL, CF_TYPE_TABLES);
	if (cfile == nullptr) {
		// Could not open. Bail.
		return 1;
	}

	cfputs("#ControlConfigOverride\n", cfile);
	cfputs(("$Name: " + Control_config_presets[0].name + "\n").c_str(), cfile);

	// Write bindings for all controls
	for (size_t i = 0; i < Control_config.size(); ++i) {
		auto& item = Control_config[i];
		auto& bindings = Control_config_presets[0].bindings[i];

		short key = -1;
		int key_shift = 0;
		int key_alt = 0;
		int key_ctrl = 0;

		short btn = bindings.second.btn;

		SCP_string buf_str = "";
		
		if (bindings.first.btn != -1) {
			// Translate the key into string form
			key = bindings.first.btn & KEY_MASK;
			key_shift = (bindings.first.btn & KEY_SHIFTED) ? 1 : 0;
			key_alt = (bindings.first.btn & KEY_ALTED) ? 1 : 0;
			key_ctrl = (bindings.first.btn & KEY_CTRLED) ? 1 : 0;

			for (const auto& pair : mKeyNameToVal) {
				if (pair.second == key) {
					buf_str = pair.first;
					break;
				}
			}
			Assertion(buf_str != "", "Unknown key found during ccfg.tbl write: 0x%X", key);
		} else {
			// Not bound
			buf_str = "NONE";
		}

		cfputs(("$Bind Name: " + item.text + "\n").c_str(), cfile);
		
		cfputs(("  $Key Default: " + buf_str + "\n").c_str(), cfile);
		cfputs(("  $Key Mod Shift: " + std::to_string(key_shift) + "\n").c_str(), cfile);
		cfputs(("  $Key Mod Alt: " + std::to_string(key_alt) + "\n").c_str(), cfile);
		cfputs(("  $Key Mod Ctrl: " + std::to_string(key_ctrl) + "\n").c_str(), cfile);

		cfputs(("  $Joy Default: " + std::to_string(btn) + "\n").c_str(), cfile);

		// Config menu options
		buf_str = "";
		for (const auto& pair : mCCTabNameToVal) {
			if (pair.second == item.tab) {
				buf_str = pair.first;
				break;
			}
		}
		Assertion(buf_str != "", "Unknown tab found during ccfg.tbl write: %i", static_cast<int>(item.tab));
		cfputs(("  $Category: " + buf_str + "\n").c_str(), cfile);

		cfputs(("  $Text: " + item.text + "\n").c_str(), cfile);
		
		cfputs(("  $Has XStr: " + std::to_string(item.indexXSTR) + "\n").c_str(), cfile);
		
		buf_str = "";
		for (const auto& pair : mCCTypeNameToVal) {
			if (pair.second == item.type) {
				buf_str = pair.first;
				break;
			}
		}
		Assertion(buf_str != "", "Unknown CC_type found during ccfg.tbl write: %i", static_cast<int>(item.type));
		cfputs(("  $Type: " + buf_str + "\n").c_str(), cfile);
	}

	cfputs("#End\n", cfile);
	cfclose(cfile);

	return 0;
}

DCF(save_ccd, "Save the current Control Configuration Defaults to .tbl") {
	if (!control_config_common_write_tbl(true)) {
		dc_printf("Default bindings saved to controlconfigdefaults.tbl");
	} else {
		dc_printf("Error: Unable to save Control Configuration Defaults.");
	}
}

DCF(load_ccd, "Reloads Control Configuration Defaults and Presets from .tbl") {
	control_config_common_read_tbl();
	dc_printf("Default bindings and presets loaded.");
}

/**
 * @brief Parses controlconfigdefault.tbl, and overrides the default control configuration for each valid entry in the .tbl
 */
void control_config_common_load_overrides()
{
	LoadEnumsIntoMaps();

	try {
		control_config_common_read_tbl();
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse 'controlconfigdefaults.tbl'!  Error message = %s.\n", e.what()));
		return;
	}
}

CC_bind& CC_bind::operator=(const CC_bind &A)
{
	cid = A.cid;
	btn = A.btn;

	return *this;
}

bool CC_bind::operator==(const CC_bind &B) const
{
	return (btn == B.btn) && (cid == B.cid);
}

bool CC_bind::operator==(const CCB &pair) const
{
	return (*this == pair.first) || (*this == pair.second);
}

bool CC_bind::operator!=(const CC_bind &B) const
{
	return !(*this == B);
}

bool CC_bind::operator!=(const CCB &pair) const
{
	return !(*this == pair);
}

void CC_bind::clear()
{
	cid = CID_NONE;
	btn = -1;
}

bool CC_bind::empty() const
{
	return cid == CID_NONE;
}

SCP_string CC_bind::textify() const {
	SCP_string retval;

	switch (cid) {
	case CID_KEYBOARD:
		retval = textify_scancode(btn);
		break;

	case CID_MOUSE:
		// Keep this up to date with mouse.h.  Better yet, move it into mouse.h and mouse.cpp
		// TODO: XSTR this
		switch (btn) {
		case 0:
			retval = "Mouse Left";
			break;
		case 1:
			retval = "Mouse Right";
			break;
		case 2:
			retval = "Mouse Middle";
			break;
		case 3:
			retval = "Mouse X1";
			break;
		case 4:
			retval = "Mouse X2";
			break;
		case 5:
			retval = "Mouse Wheel Up";
			break;
		case 6:
			retval = "Mouse Wheel Down";
			break;
		case 7:
			retval = "Mouse Wheel Left";
			break;
		case 8:
			retval = "Mouse Wheel Right";
			break;
		default:
			retval = "Unknown Mouse Input";
			break;
		}
		break;

	// TODO XSTR the "Joy #" prefix
	case CID_JOY0:
		retval = "Joy 0 " + SCP_string(Joy_button_text[btn]);
		break;

	case CID_JOY1:
		retval = "Joy 1 " + SCP_string(Joy_button_text[btn]);
		break;

	case CID_JOY2:
		retval = "Joy 2 " + SCP_string(Joy_button_text[btn]);
		break;

	case CID_JOY3:
		retval = "Joy 3 " + SCP_string(Joy_button_text[btn]);
		break;

	case CID_NONE:
	default:
		retval = "None";
		break;
	}

	return retval;
}

bool CCB::empty() const {
	return ((first.cid == CID_NONE) && (second.cid == CID_NONE));
}

void CCB::take(CC_bind A, int order) {
	// If the button isn't a valid index, nuke the cid.
	if (A.btn < 0) {
		A.cid = CID_NONE;
	}
	
	switch (order) {
	case 0:
		first = A;

		if (second.cid == A.cid) {
			second.clear();
		}
		break;

	case 1:
		second = A;
	
		if (first.cid == A.cid) {
			first.clear();
		}
		break;

	case -1:
		// Overwrite existing
		if (first.cid == A.cid) {
			first = A;

		} else if (second.cid == A.cid) {
			second = A;
		}
	break;

	default:
		return;
	}
}

void CCB::clear() {
	first.clear();
	second.clear();
}

short CCB::get_btn(CID cid) const {
	if (first.cid == cid) {
		return first.btn;

	} else if (second.cid == cid) {
		return second.btn;

	} else {
		return -1;
	}
}

CCB& CCB::operator=(const CCB& A) {
	first = A.first;
	second = A.second;

	return *this;
}

bool CCB::has_first(const CCB& A) const {
	return !first.empty() && ((first == A.first) || (first == A.second));
}

bool CCB::has_second(const CCB& A) const {
	return !second.empty() && ((second == A.first) || (second == A.second));
}

CCI& CCI::operator=(const CCI& A) {
	first = A.first;
	second = A.second;
	tab = A.tab;
	indexXSTR = A.indexXSTR;
	text = A.text;
	type = A.type;
	used = A.used;
	disabled = A.disabled;
	continuous_ongoing = A.continuous_ongoing;

	return *this;;
};

CCI& CCI::operator=(const CCB& A) {
	first = A.first;
	second = A.second;

	return *this;
};

CCI_builder::CCI_builder(SCP_vector<CCI>& _ControlConfig) : ControlConfig(_ControlConfig) {
	ControlConfig.resize(CCFG_MAX);
};

CCI_builder& CCI_builder::start() {
	return *this;
};

void CCI_builder::end() {};

CCI_builder& CCI_builder::operator()(IoActionId action_id, short key_default, short joy_default, char tab, int indexXSTR, const char *text, CC_type type, bool disabled) {
	Assert(action_id < CCFG_MAX);
	CCI& item = ControlConfig[action_id];

	// Initialize the current bindings to defaults. Defaults will be saved to a preset after Control_config is built
	// Current bindings will be overwritten once the player's bindings is read in.
	item.take(CC_bind(CID_KEYBOARD, key_default), 0);
	item.take(CC_bind(CID_JOY0, joy_default), 1);

	// Assign the UI members
	item.text.assign(text);
	item.indexXSTR = indexXSTR;
	item.tab = tab;

	// Assign the CC_type
	item.type = type;

	if (tab == NO_TAB) {
		mprintf(("Control item defined without a valid tab. Disabling: %s\n", item.text.c_str()));
	}

	// Assign disabled state
	if ((tab == NO_TAB) || (disabled == true)) {
		item.disabled = true;

	} else {
		// Must have a valid tab, and not disabled by hardcode
		item.disabled = false;
	}

	return *this;
}
