/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _GLOBALS_H
#define _GLOBALS_H


// from parselo.h
#define	PATHNAME_LENGTH			192
#define	NAME_LENGTH				32
#define	SEXP_LENGTH				128
#define	DATE_LENGTH				32
#define	TIME_LENGTH				16
#define	DATE_TIME_LENGTH		48
#define	NOTES_LENGTH			1024
#define	MULTITEXT_LENGTH		4096
#define	FILESPEC_LENGTH			64
#define	MESSAGE_LENGTH			512
#define TRAINING_MESSAGE_LENGTH	512

// from missionparse.h
#define MISSION_DESC_LENGTH		512

// from player.h
#define CALLSIGN_LEN					28		//	shortened from 32 to allow .plr to be attached without exceeding MAX_FILENAME_LEN
#define SHORT_CALLSIGN_PIXEL_W	80		// max width of short_callsign[] in pixels

#define MAX_IFFS		10

// from ship.h
#define MAX_SHIPS					400			// max number of ship instances there can be.DTP; bumped from 200 to 400
#define SHIPS_LIMIT					400			// what MAX_SHIPS will be at release time (for error checking in debug mode); dtp Bumped from 200 to 400

// ****************************************************************
// DO NOT CHANGE THIS - IT WILL LIKELY BREAK FREESPACE2 PXO SUPPORT
// TALK TO DAVE B FIRST
// ****************************************************************
#define MAX_SHIP_CLASSES_MULTI	130

#ifdef INF_BUILD
#define MAX_SHIP_CLASSES		250
#else
#define MAX_SHIP_CLASSES		130
#endif

#define MAX_WINGS				75

#define MAX_SHIPS_PER_WING			6

#define MAX_STARTING_WINGS			3	// number of wings player can start a mission with
#define MAX_SQUADRON_WINGS			5	// number of wings in squadron (displayed on HUD)

#define MAX_TVT_TEAMS				2	// number of teams in a TVT game
#define	MAX_TVT_WINGS_PER_TEAM		1 	// number of wings per team in a TVT game
#define MAX_TVT_WINGS		MAX_TVT_TEAMS * MAX_TVT_WINGS_PER_TEAM	// number of wings in a TVT game

// from model.h
#define MAX_SHIP_PRIMARY_BANKS		3
#define MAX_SHIP_SECONDARY_BANKS	4
#define MAX_SHIP_WEAPONS			(MAX_SHIP_PRIMARY_BANKS+MAX_SHIP_SECONDARY_BANKS)

// Goober5000 - moved from hudescort.cpp
// size of complete escort list, including all those wanting to get onto list but without space
#define MAX_COMPLETE_ESCORT_LIST	20
             
// from weapon.h
#define MAX_WEAPONS	2000

#ifdef INF_BUILD
#define MAX_WEAPON_TYPES				300
#else
#define MAX_WEAPON_TYPES				200
#endif

// from model.h

#define MAX_MODEL_TEXTURES	64

#ifdef INF_BUILD
	#define MAX_POLYGON_MODELS  300
#else
	#define MAX_POLYGON_MODELS  128 //DTP reset from 198 to original value of 128
#endif

// from scoring.h
// ARGH. IMPORTANT : do not change NUM_MEDALS without talking to DaveB first. It will affect the size of the scoring struct and hence, will break
// a lot of PXO related stuff. SEE ALSO : MAX_SHIP_CLASSES
#define MAX_MEDALS			18
#define NUM_MEDALS_FS1		16
extern int Num_medals;

// object.h
#define MAX_OBJECTS			3500		

// from lighting.cpp
#define MAX_LIGHTS 256

// from weapon.h (and beam.h)
#define MAX_BEAM_SECTIONS				5

#endif	// _GLOBALS_H
