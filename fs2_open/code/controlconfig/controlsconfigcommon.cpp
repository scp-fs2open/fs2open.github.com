/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include <stdio.h>
#include <stdarg.h>

#include "cfile/cfile.h"
#include "controlconfig/controlsconfig.h"
#include "io/key.h"
#include "io/joy.h"
#include "localization/localize.h"
#include "parse/parselo.h"
#include "globalincs/systemvars.h"
#include "globalincs/def_files.h"

#define TARGET_TAB			0
#define SHIP_TAB				1
#define WEAPON_TAB			2
#define COMPUTER_TAB			3

int Failed_key_index;

// assume control keys are used as modifiers until we find out 
int Shift_is_modifier;
int Ctrl_is_modifier;
int Alt_is_modifier;

int Axis_enabled[JOY_NUM_AXES] = { 1, 1, 1, 0, 0, 0 };
int Axis_enabled_defaults[JOY_NUM_AXES] = { 1, 1, 1, 0, 0, 0 };
int Invert_axis[JOY_NUM_AXES] = { 0, 0, 0, 0, 0, 0 };
int Invert_axis_defaults[JOY_NUM_AXES] = { 0, 0, 0, 0, 0, 0 };

// arrays which hold the key mappings.  The array index represents a key-independent action.
//
//XSTR:OFF
config_item Control_config[CCFG_MAX + 1] = {
	// targeting a ship
	{                           KEY_T,				-1, TARGET_TAB,	true, "Target Next Ship" },
	{             KEY_SHIFTED | KEY_T,				-1, TARGET_TAB,	true, "Target Previous Ship" },
	{                           KEY_H,				2,  TARGET_TAB,	true, "Target Next Closest Hostile Ship" },
	{	           KEY_SHIFTED | KEY_H,				-1, TARGET_TAB,	true, "Target Previous Closest Hostile Ship" },
	{ KEY_ALTED |               KEY_H,				-1, TARGET_TAB,	true, "Toggle Auto Targeting" },
	{                           KEY_F,				-1, TARGET_TAB,	true, "Target Next Closest Friendly Ship" },
	{             KEY_SHIFTED | KEY_F,				-1, TARGET_TAB,	true, "Target Previous Closest Friendly Ship" },
	{                           KEY_Y,				4,  TARGET_TAB,	true, "Target Ship in Reticle" },
	{                           KEY_G,				-1, TARGET_TAB,	true, "Target Target's Nearest Attacker" },
	{ KEY_ALTED	|					 KEY_Y,			-1, TARGET_TAB,	true, "Target Last Ship to Send Transmission" },
	{ KEY_ALTED |               KEY_T,				-1, TARGET_TAB,	true, "Turn Off Targeting" },

	// targeting a ship's subsystem
	{                           KEY_V,				-1, TARGET_TAB,	true, "Target Subsystem in Reticle" },
	{                           KEY_S,				-1, TARGET_TAB,	true, "Target Next Subsystem" },
	{             KEY_SHIFTED | KEY_S,				-1, TARGET_TAB,	true, "Target Previous Subsystem" },
	{ KEY_ALTED |               KEY_S,				-1, TARGET_TAB,	true, "Turn Off Targeting of Subsystems" },

	// matching speed
	{                           KEY_M,				-1, COMPUTER_TAB,	true, "Match Target Speed" },
	{ KEY_ALTED |               KEY_M,				-1, COMPUTER_TAB,	true, "Toggle Auto Speed Matching" },

	// weapons
	{                           KEY_LCTRL,			0,	 WEAPON_TAB,	true, "Fire Primary Weapon", CC_TYPE_CONTINUOUS },
	{                           KEY_SPACEBAR,		1,  WEAPON_TAB,	true, "Fire Secondary Weapon", CC_TYPE_CONTINUOUS },
	{                           KEY_PERIOD,			-1, WEAPON_TAB,	true, "Cycle Forward Primary Weapon" },
	{                           KEY_COMMA,			-1, WEAPON_TAB,	true, "Cycle Backward Primary Weapon" },
	{                           KEY_DIVIDE,			-1, WEAPON_TAB,	true, "Cycle Secondary Weapon Bank" },
	{             KEY_SHIFTED | KEY_DIVIDE,			-1, WEAPON_TAB,	true, "Cycle Secondary Weapon Firing Rate" },
	{                           KEY_X,				3,	 WEAPON_TAB,	true, "Launch Countermeasure" },

	// controls
	{                           KEY_A,				-1, SHIP_TAB,		true, "Forward Thrust", CC_TYPE_CONTINUOUS },
	{                           KEY_Z,				-1, SHIP_TAB,		true, "Reverse Thrust", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD7,			-1, SHIP_TAB,		true, "Bank Left", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD9,			-1, SHIP_TAB,		true, "Bank Right", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD8,			-1, SHIP_TAB,		true, "Pitch Forward", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD2,			-1, SHIP_TAB,		true, "Pitch Backward", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD4,			-1, SHIP_TAB,		true, "Turn Left", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD6,			-1, SHIP_TAB,		true, "Turn Right", CC_TYPE_CONTINUOUS },

	// throttle controls
	{                           KEY_BACKSP,			-1, SHIP_TAB,		true, "Set Throttle to Zero" },
	{                           KEY_SLASH,			-1, SHIP_TAB,		true, "Set Throttle to Max" },
	{                           KEY_LBRACKET,		-1, SHIP_TAB,		true, "Set Throttle to One-Third" },
	{                           KEY_RBRACKET,		-1, SHIP_TAB,		true, "Set Throttle to Two-Thirds" },
	{                           KEY_EQUAL,			-1, SHIP_TAB,		true, "Increase Throttle 5 Percent" },
	{                           KEY_MINUS,			-1, SHIP_TAB,		true, "Decrease Throttle 5 Percent" },

	// squadmate messaging
	{             KEY_SHIFTED | KEY_A,				-1, COMPUTER_TAB,	true, "Attack My Target" },
	{             KEY_SHIFTED | KEY_Z,				-1, COMPUTER_TAB,	true, "Disarm My Target" },
	{             KEY_SHIFTED | KEY_D,				-1, COMPUTER_TAB,	true, "Disable My Target" },
	{             KEY_SHIFTED | KEY_V,				-1, COMPUTER_TAB,	true, "Attack My Subsystem" },
	{             KEY_SHIFTED | KEY_X,				-1, COMPUTER_TAB,	true, "Capture My Target" },
	{             KEY_SHIFTED | KEY_E,				-1, COMPUTER_TAB,	true, "Engage Enemy" },
	{             KEY_SHIFTED | KEY_W,				-1, COMPUTER_TAB,	true, "Form on My Wing" },
	{             KEY_SHIFTED | KEY_I,				-1, COMPUTER_TAB,	true, "Ignore My Target" },
	{             KEY_SHIFTED | KEY_P,				-1, COMPUTER_TAB,	true, "Protect My Target" },
	{             KEY_SHIFTED | KEY_C,				-1, COMPUTER_TAB,	true, "Cover Me" },
	{             KEY_SHIFTED | KEY_J,				-1, COMPUTER_TAB,	true, "Return to Base" },
	{				  KEY_SHIFTED | KEY_R,			-1, COMPUTER_TAB, true, "Rearm Me" },

	{									 KEY_R,		6,  TARGET_TAB,	true, "Target Closest Attacking Ship" },

	// Views
	{                           KEY_PADMULTIPLY,	-1, COMPUTER_TAB,	true, "Chase View" },
	{                           KEY_PADPERIOD,		-1, COMPUTER_TAB,	true, "External View"},
	{                           KEY_PADENTER,		-1, COMPUTER_TAB,	true, "Toggle External Camera Lock"},
	{                           KEY_PAD0,			-1, COMPUTER_TAB,	true, "Free Look View", CC_TYPE_CONTINUOUS }, // Not in use anymore (Swifty)
	{                           KEY_PADDIVIDE,		-1, COMPUTER_TAB,	true, "Current Target View" },
	{                           KEY_PADPLUS,		-1, COMPUTER_TAB,	true, "Increase View Distance", CC_TYPE_CONTINUOUS },
	{                           KEY_PADMINUS,		-1, COMPUTER_TAB,	true, "Decrease View Distance", CC_TYPE_CONTINUOUS },
	{                           KEY_PAD5,			-1, COMPUTER_TAB,	true, "Center View", CC_TYPE_CONTINUOUS },
	{							-1,					33, COMPUTER_TAB, true, "View Up", CC_TYPE_CONTINUOUS },
	{							-1,					32, COMPUTER_TAB, true, "View Rear", CC_TYPE_CONTINUOUS },
	{							-1,					34, COMPUTER_TAB, true, "View Left", CC_TYPE_CONTINUOUS },
	{							-1,					35, COMPUTER_TAB, true, "View Right", CC_TYPE_CONTINUOUS },

	{                           KEY_RAPOSTRO,		-1, COMPUTER_TAB,	true, "Cycle Radar Range" },
	{                           KEY_C,				-1, COMPUTER_TAB, true, "Communications Menu" },
	{                           -1,					-1, -1,				true, "Show Objectives" },
	{ KEY_ALTED |               KEY_J,				-1, COMPUTER_TAB,	true, "Enter Subspace (End Mission)" },
	{                           KEY_J,				-1, TARGET_TAB,	true, "Target Target's Target" },
	{                           KEY_TAB,			5,  SHIP_TAB,		true, "Afterburner", CC_TYPE_CONTINUOUS },
	
	{                           KEY_INSERT,		-1, COMPUTER_TAB,	true, "Increase Weapon Energy" },
	{                           KEY_DELETE,		-1, COMPUTER_TAB,	true, "Decrease Weapon Energy" },
	{                           KEY_HOME,			-1, COMPUTER_TAB,	true, "Increase Shield Energy" },
	{                           KEY_END,			-1, COMPUTER_TAB,	true, "Decrease Shield Energy" },
	{                           KEY_PAGEUP,		-1, COMPUTER_TAB,	true, "Increase Engine Energy" },
	{                           KEY_PAGEDOWN,		-1, COMPUTER_TAB,	true, "Decrease Engine Energy" },
	{ KEY_ALTED |               KEY_D,				-1, COMPUTER_TAB, true, "Equalize Energy Settings" },

	{                           KEY_Q,				7,  COMPUTER_TAB,	true, "Equalize Shields" },
	{                           KEY_UP,				-1, COMPUTER_TAB,	true, "Augment Forward Shield" },
	{                           KEY_DOWN,			-1, COMPUTER_TAB,	true, "Augment Rear Shield" },
	{                           KEY_LEFT,			-1, COMPUTER_TAB,	true, "Augment Left Shield" },
	{                           KEY_RIGHT,			-1, COMPUTER_TAB,	true, "Augment Right Shield" },
	{                           KEY_SCROLLOCK,	-1, COMPUTER_TAB,	true, "Transfer Energy Laser->Shield" },
	{             KEY_SHIFTED | KEY_SCROLLOCK,	-1, COMPUTER_TAB,	true, "Transfer Energy Shield->Laser" },
//	{                           -1,					-1, -1,				true, "Show Damage Popup Window" },	

	{							-1,					-1, SHIP_TAB,		false, "Glide When Pressed", CC_TYPE_CONTINUOUS },
//Backslash -- this was a convenient place for Glide When Pressed, as Show Damage Popup isn't used
	{                           -1,					-1, SHIP_TAB,		true, "Bank When Pressed", CC_TYPE_CONTINUOUS },
	{									 -1,					-1, -1,				true, "Show Nav Map" },
	{ KEY_ALTED |	             KEY_E,				-1, COMPUTER_TAB,	true, "Add or Remove Escort" },
	{ KEY_ALTED | KEY_SHIFTED | KEY_E,				-1, COMPUTER_TAB,	true, "Clear Escort List" },
	{					             KEY_E,				-1, TARGET_TAB,	true, "Target Next Escort Ship" },
	{ KEY_ALTED	|					 KEY_R,				-1, TARGET_TAB,	true, "Target Closest Repair Ship" },

	{                           KEY_U,				-1, TARGET_TAB,	true, "Target Next Uninspected Cargo" },
	{             KEY_SHIFTED | KEY_U,				-1, TARGET_TAB,	true, "Target Previous Uninspected Cargo" },
	{									 KEY_N,		-1, TARGET_TAB,	true, "Target Newest Ship in Area" },
	{                           KEY_K,				-1, TARGET_TAB,	true, "Target Next Live Turret" },
	{             KEY_SHIFTED | KEY_K,				-1, TARGET_TAB,	true, "Target Previous Live Turret" },

	{									 KEY_B,		-1, TARGET_TAB,	true, "Target Next Hostile Bomb or Bomber" },
	{             KEY_SHIFTED | KEY_B,				-1, TARGET_TAB,	true, "Target Previous Hostile Bomb or Bomber" },

	// multiplayer messaging keys
	{									 KEY_1,				-1, COMPUTER_TAB, true, "(Multiplayer) Message All", CC_TYPE_CONTINUOUS },
	{									 KEY_2,				-1, COMPUTER_TAB, true, "(Multiplayer) Message Friendly", CC_TYPE_CONTINUOUS },
	{									 KEY_3,				-1, COMPUTER_TAB, true, "(Multiplayer) Message Hostile", CC_TYPE_CONTINUOUS },
	{									 KEY_4,				-1, COMPUTER_TAB, true, "(Multiplayer) Message Target", CC_TYPE_CONTINUOUS },
	{ KEY_ALTED	|					 KEY_X,				-1, COMPUTER_TAB, true, "(Multiplayer) Observer Zoom to Target"},	
	{             KEY_SHIFTED | KEY_PERIOD,		-1, COMPUTER_TAB,	true, "Increase Time Compression" },
	{             KEY_SHIFTED | KEY_COMMA,			-1, COMPUTER_TAB,	true, "Decrease Time Compression" },
	{									 KEY_L,				-1, COMPUTER_TAB, true, "Toggle High HUD Contrast" },	
	{				  KEY_SHIFTED | KEY_N,				-1, COMPUTER_TAB, true, "(Multiplayer) Toggle Network Info"},
	{				  KEY_SHIFTED | KEY_END,			-1, COMPUTER_TAB, true, "(Multiplayer) Self Destruct"},

	// Misc
	{				  KEY_SHIFTED | KEY_O,			-1, COMPUTER_TAB, true, "Toggle HUD"},
	{				  KEY_SHIFTED | KEY_3,			-1, SHIP_TAB, true, "Right Thrust", CC_TYPE_CONTINUOUS},
	{				  KEY_SHIFTED | KEY_1,			-1, SHIP_TAB, true, "Left Thrust", CC_TYPE_CONTINUOUS},
	{				  KEY_SHIFTED | KEY_PADPLUS,	-1, SHIP_TAB, true, "Up Thrust", CC_TYPE_CONTINUOUS},
	{				  KEY_SHIFTED | KEY_PADENTER,	-1, SHIP_TAB, true, "Down Thrust", CC_TYPE_CONTINUOUS},
	{ KEY_ALTED |     KEY_SHIFTED | KEY_Q,			-1, COMPUTER_TAB, true, "Toggle HUD Wireframe Target View"},
	{							-1,					-1,	COMPUTER_TAB, false, "Top-Down View"},
	{							-1,					-1, COMPUTER_TAB, false, "Target Padlock View"}, // (Swifty) Toggle for VM_TRACK
	// Auto Navigation Systen
	{ KEY_ALTED |					KEY_A,			-1, COMPUTER_TAB, false, "Toggle Auto Pilot"},
	{ KEY_ALTED |					KEY_N,			-1, COMPUTER_TAB, false, "Cycle Nav Points"},
	{ KEY_ALTED |					KEY_G,			-1, SHIP_TAB, false, "Toggle Gliding"},
	{                           -1,					-1, -1,			 false,	"" }
};

char *Scan_code_text_german[] = {
	"",				"Esc",				"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"Akzent '",				"\xE1",				"R\x81""cktaste",		"Tab",
	"Q",				"W",				"E",				"R",				"T",				"Z",				"U",				"I",
	"O",				"P",				"\x9A",				"+",				"Eingabe",			"Strg Links",			"A",				"S",

	"D",				"F",				"G",				"H",				"J",				"K",				"L",				"\x99",
	"\xAE",				"`",				"Shift",			"#",				"Y",				"X",				"C",				"V",
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

char *Joy_button_text_german[] = {
	"Knopf 1",		"Knopf 2",		"Knopf 3",		"Knopf 4",		"Knopf 5",		"Knopf 6",
	"Knopf 7",		"Knopf 8",		"Knopf 9",		"Knopf 10",		"Knopf 11",		"Knopf 12",
	"Knopf 13",		"Knopf 14",		"Knopf 15",		"Knopf 16",		"Knopf 17",		"Knopf 18",
	"Knopf 19",		"Knopf 20",		"Knopf 21",		"Knopf 22",		"Knopf 23",		"Knopf 24",
	"Knopf 25",		"Knopf 26",		"Knopf 27",		"Knopf 28",		"Knopf 29",		"Knopf 30",
	"Knopf 31",		"Knopf 32",		"Hut Hinten",	"Hut Vorne",	"Hut Links",	"Hut Rechts"
};

char *Scan_code_text_french[] = {
	"",				"\x90""chap",			"1",				"2",				"3",				"4",				"5",				"6",
	"7",				"8",				"9",				"0",				"-",				"=",				"Fl\x82""che Ret.",			"Tab",
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

char *Joy_button_text_french[] = {
	"Bouton 1",		"Bouton 2",		"Bouton 3",		"Bouton 4",		"Bouton 5",		"Bouton 6",
	"Bouton 7",		"Bouton 8",		"Bouton 9",		"Bouton 10",		"Bouton 11",		"Bouton 12",
	"Bouton 13",		"Bouton 14",		"Bouton 15",		"Bouton 16",		"Bouton 17",		"Bouton 18",
	"Bouton 19",		"Bouton 20",		"Bouton 21",		"Bouton 22",		"Bouton 23",		"Bouton 24",
	"Bouton 25",		"Bouton 26",		"Bouton 27",		"Bouton 28",		"Bouton 29",		"Bouton 30",
	"Bouton 31",		"Bouton 32",		"Chapeau Arriere",		"Chapeau Avant",		"Chapeau Gauche",		"Chapeau Droite"
};

//	This is the text that is displayed on the screen for the keys a player selects
char *Scan_code_text_english[] = {
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

char *Joy_button_text_english[] = {
	"Button 1",		"Button 2",		"Button 3",		"Button 4",		"Button 5",		"Button 6",
	"Button 7",		"Button 8",		"Button 9",		"Button 10",	"Button 11",	"Button 12",
	"Button 13",	"Button 14",	"Button 15",	"Button 16",	"Button 17",	"Button 18",
	"Button 19",	"Button 20",	"Button 21",	"Button 22",	"Button 23",	"Button 24",
	"Button 25",	"Button 26",	"Button 27",	"Button 28",	"Button 29",	"Button 30",
	"Button 31",	"Button 32",	"Hat Back",		"Hat Forward",	"Hat Left",		"Hat Right"
};

char **Scan_code_text = Scan_code_text_english;
char **Joy_button_text = Joy_button_text_english;

void set_modifier_status()
{
	int i;

	Alt_is_modifier = 0;
	Shift_is_modifier = 0;
	Ctrl_is_modifier = 0;

	for (i=0; i<CCFG_MAX; i++) {
		if (Control_config[i].key_id < 0)
			continue;

		if (Control_config[i].key_id & KEY_ALTED)
			Alt_is_modifier = 1;

		if (Control_config[i].key_id & KEY_SHIFTED)
			Shift_is_modifier = 1;

		if (Control_config[i].key_id & KEY_CTRLED) {
			Assert(0);  // get Alan
			Ctrl_is_modifier = 1;
		}
	}
}

int translate_key_to_index(char *key)
{
	int i, index = -1, alt = 0, shift = 0, max_scan_codes;

	if (Lcl_gr) {
		max_scan_codes = sizeof(Scan_code_text_german) / sizeof(char *);
	} else if (Lcl_fr) {
		max_scan_codes = sizeof(Scan_code_text_french) / sizeof(char *);
	} else {
		max_scan_codes = sizeof(Scan_code_text_english) / sizeof(char *);
	}

	// look for modifiers
	Assert(key);
	if (!strnicmp(key, "Alt", 3)) {
		alt = 1;
		key += 3;
		if (*key)
			key++;
	}

	char *translated_shift;
	
	if(Lcl_gr){
		translated_shift = "Shift";
	} else if(Lcl_fr){	
		translated_shift = "Maj.";
	} else {	
		translated_shift = "Shift";
	}

	if (!strnicmp(key, translated_shift, 5)) {
		shift = 1;
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
		for (i=0; i<CCFG_MAX; i++) {
			if (Control_config[i].key_default == index) {
				index = i;
				break;
			}
		}

		if (i == CCFG_MAX)
			return -1;

		return index;
	}

	return -1;
}

// Given the system default key 'key', return the current key that is bound to the function
// Both are 'key' and the return value are descriptive strings that can be displayed
// directly to the user.  If 'key' isn't a real key or not normally bound to anything,
// or there is no key current bound to the function, NULL is returned.
char *translate_key(char *key)
{
	int index = -1, key_code = -1, joy_code = -1;
	char *key_text = NULL;
	char *joy_text = NULL;

	static char text[40] = {"None"};

	index = translate_key_to_index(key);
	if (index < 0) {
		return NULL;
	}

	key_code = Control_config[index].key_id;
	joy_code = Control_config[index].joy_id;

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
		strcat_s(text, " or ");
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

char *textify_scancode(int code)
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
//XSTR:ON

void control_config_common_load_overrides();

// initialize common control config stuff - call at game startup after localization has been initialized
void control_config_common_init()
{
    control_config_common_load_overrides();
	if(Lcl_gr){
		Scan_code_text = Scan_code_text_german;
		Joy_button_text = Joy_button_text_german;
		
		// swap init bindings for y and z keys
		Control_config[TARGET_SHIP_IN_RETICLE].key_default = KEY_Z;
		Control_config[TARGET_LAST_TRANMISSION_SENDER].key_default = KEY_ALTED | KEY_Z;
		Control_config[REVERSE_THRUST].key_default = KEY_Y;
		Control_config[DISARM_MESSAGE].key_default = KEY_SHIFTED | KEY_Y;		
	} else if(Lcl_fr){
		Scan_code_text = Scan_code_text_french;
		Joy_button_text = Joy_button_text_french;
	} else {
		Scan_code_text = Scan_code_text_english;
		Joy_button_text = Joy_button_text_english;
	}
}



#include <map>
#include <string>
SCP_map<SCP_string, int> mEnumNameToVal;

void LoadEnumsIntoMap();
void control_config_common_load_overrides()
{
    LoadEnumsIntoMap();
    
    if (cf_exists_full("controlconfigdefaults.tbl", CF_TYPE_TABLES))
        read_file_text("controlconfigdefaults.tbl", CF_TYPE_TABLES);
    else
        read_file_text_from_array(defaults_get_file("controlconfigdefaults.tbl"));
	
    reset_parse();
    
	// start parsing
	required_string("#ControlConfigOverride");

	// read fonts
	while (required_string_either("#End","$Bind Name:"))
    {
        const int iBufferLength = 64;
        char szTempBuffer[iBufferLength];
        
        required_string("$Bind Name:");
        stuff_string(szTempBuffer, F_NAME, iBufferLength);
        
        const size_t cCntrlAryLength = sizeof(Control_config) / sizeof(Control_config[0]);
        for (size_t i = 0; i < cCntrlAryLength; ++i)
        {
            config_item& r_ccConfig = Control_config[i];
            
            if (!strcmp(szTempBuffer, r_ccConfig.text))
            {
                /**
                 * short key_default;
	             * short joy_default;
	             * char tab;
	             * bool hasXSTR;
	             * char type;
                 */
                
                int iTemp;
                
                if (optional_string("$Key Default:"))
                {stuff_string(szTempBuffer, F_NAME, iBufferLength);
                 r_ccConfig.key_default = (short)mEnumNameToVal[szTempBuffer];}
                
                if (optional_string("$Joy Default:"))
                {stuff_int(&iTemp); r_ccConfig.joy_default = (short)iTemp;}
                
                if (optional_string("$Key Mod Shift:"))
                {stuff_int(&iTemp); r_ccConfig.key_default |= (iTemp == 1) ? KEY_SHIFTED : 0;}
                
                if (optional_string("$Key Mod Alt:"))
                {stuff_int(&iTemp); r_ccConfig.key_default |= (iTemp == 1) ? KEY_ALTED : 0;}
                
                if (optional_string("$Key Mod Ctrl:"))
                {stuff_int(&iTemp); r_ccConfig.key_default |= (iTemp == 1) ? KEY_CTRLED : 0;}
                
                if (optional_string("$Category:"))
                {stuff_string(szTempBuffer, F_NAME, iBufferLength);
                 r_ccConfig.tab = (char)mEnumNameToVal[szTempBuffer];}
                
                if (optional_string("$Has XStr:"))
                {stuff_int(&iTemp); r_ccConfig.hasXSTR = (iTemp == 1);}
                
                if (optional_string("$Type:"))
                {stuff_string(szTempBuffer, F_NAME, iBufferLength);
                 r_ccConfig.type = (char)mEnumNameToVal[szTempBuffer];}
                
                // Nerf the buffer now.
                szTempBuffer[0] = '\0';
            }
            else if ((i + 1) == cCntrlAryLength)
            {
                error_display(1, "Bind Name not found: %s\n", szTempBuffer);
		        advance_to_eoln(NULL);
		        ignore_white_space();
                return;
            }
        }
    }
    
    required_string("#End");
}

#define ADD_ENUM_TO_ENUM_MAP(Enum) mEnumNameToVal[#Enum] = (Enum);

void LoadEnumsIntoMap()
{
    mEnumNameToVal["KEY_SHIFTED"] = KEY_SHIFTED;
    /*
    ADD_ENUM_TO_ENUM_MAP(KEY_SHIFTED)
    ADD_ENUM_TO_ENUM_MAP(KEY_ALTED)
    ADD_ENUM_TO_ENUM_MAP(KEY_CTRLED)
    ADD_ENUM_TO_ENUM_MAP(KEY_DEBUGGED)
    ADD_ENUM_TO_ENUM_MAP(KEY_DEBUGGED1)
    ADD_ENUM_TO_ENUM_MAP(KEY_MASK)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_DEBUG_KEY)
    */
    ADD_ENUM_TO_ENUM_MAP(KEY_0)
    ADD_ENUM_TO_ENUM_MAP(KEY_1)
    ADD_ENUM_TO_ENUM_MAP(KEY_2)
    ADD_ENUM_TO_ENUM_MAP(KEY_3)
    ADD_ENUM_TO_ENUM_MAP(KEY_4)
    ADD_ENUM_TO_ENUM_MAP(KEY_5)
    ADD_ENUM_TO_ENUM_MAP(KEY_6)
    ADD_ENUM_TO_ENUM_MAP(KEY_7)
    ADD_ENUM_TO_ENUM_MAP(KEY_8)
    ADD_ENUM_TO_ENUM_MAP(KEY_9)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_A)
    ADD_ENUM_TO_ENUM_MAP(KEY_B)
    ADD_ENUM_TO_ENUM_MAP(KEY_C)
    ADD_ENUM_TO_ENUM_MAP(KEY_D)
    ADD_ENUM_TO_ENUM_MAP(KEY_E)
    ADD_ENUM_TO_ENUM_MAP(KEY_F)
    ADD_ENUM_TO_ENUM_MAP(KEY_G)
    ADD_ENUM_TO_ENUM_MAP(KEY_H)
    ADD_ENUM_TO_ENUM_MAP(KEY_I)
    ADD_ENUM_TO_ENUM_MAP(KEY_J)
    ADD_ENUM_TO_ENUM_MAP(KEY_K)
    ADD_ENUM_TO_ENUM_MAP(KEY_L)
    ADD_ENUM_TO_ENUM_MAP(KEY_M)
    ADD_ENUM_TO_ENUM_MAP(KEY_N)
    ADD_ENUM_TO_ENUM_MAP(KEY_O)
    ADD_ENUM_TO_ENUM_MAP(KEY_P)
    ADD_ENUM_TO_ENUM_MAP(KEY_Q)
    ADD_ENUM_TO_ENUM_MAP(KEY_R)
    ADD_ENUM_TO_ENUM_MAP(KEY_S)
    ADD_ENUM_TO_ENUM_MAP(KEY_T)
    ADD_ENUM_TO_ENUM_MAP(KEY_U)
    ADD_ENUM_TO_ENUM_MAP(KEY_V)
    ADD_ENUM_TO_ENUM_MAP(KEY_W)
    ADD_ENUM_TO_ENUM_MAP(KEY_X)
    ADD_ENUM_TO_ENUM_MAP(KEY_Y)
    ADD_ENUM_TO_ENUM_MAP(KEY_Z)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_MINUS)
    ADD_ENUM_TO_ENUM_MAP(KEY_EQUAL)
    ADD_ENUM_TO_ENUM_MAP(KEY_DIVIDE)
    ADD_ENUM_TO_ENUM_MAP(KEY_SLASH)
    ADD_ENUM_TO_ENUM_MAP(KEY_SLASH_UK)
    ADD_ENUM_TO_ENUM_MAP(KEY_COMMA)
    ADD_ENUM_TO_ENUM_MAP(KEY_PERIOD)
    ADD_ENUM_TO_ENUM_MAP(KEY_SEMICOL)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_LBRACKET)
    ADD_ENUM_TO_ENUM_MAP(KEY_RBRACKET)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_RAPOSTRO)
    ADD_ENUM_TO_ENUM_MAP(KEY_LAPOSTRO)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_ESC)
    ADD_ENUM_TO_ENUM_MAP(KEY_ENTER)
    ADD_ENUM_TO_ENUM_MAP(KEY_BACKSP)
    ADD_ENUM_TO_ENUM_MAP(KEY_TAB)
    ADD_ENUM_TO_ENUM_MAP(KEY_SPACEBAR)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_NUMLOCK)
    ADD_ENUM_TO_ENUM_MAP(KEY_SCROLLOCK)
    ADD_ENUM_TO_ENUM_MAP(KEY_CAPSLOCK)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_LSHIFT)
    ADD_ENUM_TO_ENUM_MAP(KEY_RSHIFT)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_LALT)
    ADD_ENUM_TO_ENUM_MAP(KEY_RALT)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_LCTRL)
    ADD_ENUM_TO_ENUM_MAP(KEY_RCTRL)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_F1)
    ADD_ENUM_TO_ENUM_MAP(KEY_F2)
    ADD_ENUM_TO_ENUM_MAP(KEY_F3)
    ADD_ENUM_TO_ENUM_MAP(KEY_F4)
    ADD_ENUM_TO_ENUM_MAP(KEY_F5)
    ADD_ENUM_TO_ENUM_MAP(KEY_F6)
    ADD_ENUM_TO_ENUM_MAP(KEY_F7)
    ADD_ENUM_TO_ENUM_MAP(KEY_F8)
    ADD_ENUM_TO_ENUM_MAP(KEY_F9)
    ADD_ENUM_TO_ENUM_MAP(KEY_F10)
    ADD_ENUM_TO_ENUM_MAP(KEY_F11)
    ADD_ENUM_TO_ENUM_MAP(KEY_F12)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD0)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD1)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD2)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD3)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD4)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD5)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD6)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD7)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD8)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAD9)
    ADD_ENUM_TO_ENUM_MAP(KEY_PADMINUS)
    ADD_ENUM_TO_ENUM_MAP(KEY_PADPLUS)
    ADD_ENUM_TO_ENUM_MAP(KEY_PADPERIOD)
    ADD_ENUM_TO_ENUM_MAP(KEY_PADDIVIDE)
    ADD_ENUM_TO_ENUM_MAP(KEY_PADMULTIPLY)
    ADD_ENUM_TO_ENUM_MAP(KEY_PADENTER)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_INSERT)
    ADD_ENUM_TO_ENUM_MAP(KEY_HOME)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAGEUP)
    ADD_ENUM_TO_ENUM_MAP(KEY_DELETE)
    ADD_ENUM_TO_ENUM_MAP(KEY_END)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAGEDOWN)
    ADD_ENUM_TO_ENUM_MAP(KEY_UP)
    ADD_ENUM_TO_ENUM_MAP(KEY_DOWN)
    ADD_ENUM_TO_ENUM_MAP(KEY_LEFT)
    ADD_ENUM_TO_ENUM_MAP(KEY_RIGHT)
    
    ADD_ENUM_TO_ENUM_MAP(KEY_PRINT_SCRN)
    ADD_ENUM_TO_ENUM_MAP(KEY_PAUSE)
    ADD_ENUM_TO_ENUM_MAP(KEY_BREAK)
    
    ADD_ENUM_TO_ENUM_MAP(TARGET_TAB)
    ADD_ENUM_TO_ENUM_MAP(SHIP_TAB)
    ADD_ENUM_TO_ENUM_MAP(WEAPON_TAB)
    ADD_ENUM_TO_ENUM_MAP(COMPUTER_TAB)
}
