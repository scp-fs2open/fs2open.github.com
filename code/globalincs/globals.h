/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/globals.h $
 * $Revision: 1.3 $
 * $Date: 2004-05-10 10:51:54 $
 * $Author: Goober5000 $
 *
 * Header for common global #defines, to cut down on #includes
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/03/05 09:01:52  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 1.1  2004/02/28 20:26:10  Goober5000
 * preliminary checking of globals.h
 * --Goober5000
 *
 * Revision 1.0  2004/02/28 20:26:00  Goober5000
 * Addition to CVS repository
 *
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
// Kazan - this used to be 1024, now 4096
#define	MULTITEXT_LENGTH		4096
#define	FILESPEC_LENGTH			64
#define	MESSAGE_LENGTH			512

// from missionparse.h
#define MISSION_DESC_LENGTH		512


// from player.h
#define CALLSIGN_LEN					28		//	shortened from 32 to allow .plr to be attached without exceeding MAX_FILENAME_LEN
#define SHORT_CALLSIGN_PIXEL_W	80		// max width of short_callsign[] in pixels


// from ship.h
#ifdef NDEBUG
	#ifdef FRED
		#define	MAX_SHIPS					400			// max number of ship instances there can be. DTP; bumped from 100 to 400
		#define	SHIPS_LIMIT					400			// what MAX_SHIPS will be at release time (for error checking in debug mode), DTP bumped from 100 to 400
	#else
		#define	MAX_SHIPS					400			// max number of ship instances there can be. /DTP bumped from 100 to 400
		#define	SHIPS_LIMIT					400			// what MAX_SHIPS will be at release time (for error checking in debug mode); DTP bumped from 100 to 400
	#endif
#else
#define MAX_SHIPS					400			// max number of ship instances there can be.DTP; bumped from 200 to 400
#define	SHIPS_LIMIT					400			// what MAX_SHIPS will be at release time (for error checking in debug mode); dtp Bumped from 200 to 400
#endif

// ****************************************************************
// DO NOT CHANGE THIS - IT WILL LIKELY BREAK FREESPACE2 PXO SUPPORT
// TALK TO DAVE B FIRST
// ****************************************************************
#ifdef INF_BUILD
#define MAX_SHIP_TYPES		250
#else
#define MAX_SHIP_TYPES		130
#endif

#ifdef FS2_DEMO
	#define MAX_WINGS				15
#else
	#define MAX_WINGS				25
#endif


// from ship.h
#define MAX_PLAYER_PRIMARY_BANKS	2
#define MAX_PLAYER_SECONDARY_BANKS	3
#define MAX_PLAYER_WEAPONS			5


// from model.h
#define MAX_SHIP_PRIMARY_BANKS		3
#define MAX_SHIP_SECONDARY_BANKS	4	//	Lowered from 5 to 4 by MK on 3/25/98.  This needs to be <= MAX_WL_SECONDARY or you'll get stack overwrites.
#define MAX_SHIP_WEAPONS			7


// limit checks - Goober5000

#if (MAX_PLAYER_PRIMARY_BANKS+MAX_PLAYER_SECONDARY_BANKS != MAX_PLAYER_WEAPONS)
	#error MAX_PLAYER_PRIMARY_BANKS + MAX_PLAYER_SECONDARY_BANKS must equal MAX_PLAYER_WEAPONS
#endif

#if (MAX_SHIP_PRIMARY_BANKS+MAX_SHIP_SECONDARY_BANKS != MAX_SHIP_WEAPONS)
	#error MAX_SHIP_PRIMARY_BANKS + MAX_SHIP_SECONDARY_BANKS must equal MAX_SHIP_WEAPONS
#endif

#if (MAX_PLAYER_PRIMARY_BANKS > MAX_SHIP_PRIMARY_BANKS)
	#error MAX_PLAYER_PRIMARY_BANKS must be less than or equal to MAX_SHIP_PRIMARY_BANKS)
#endif

#if (MAX_PLAYER_SECONDARY_BANKS > MAX_SHIP_SECONDARY_BANKS)
	#error MAX_PLAYER_SECONDARY_BANKS must be less than or equal to MAX_SHIP_SECONDARY_BANKS)
#endif


// Goober5000 - moved from hudescort.cpp
// size of complete escort list, including all those wanting to get onto list but without space
#define MAX_COMPLETE_ESCORT_LIST	20
             

// from weapon.h
#ifdef FS2_DEMO
	#define MAX_WEAPONS	100
#else
	// upped 5/6/98 from 200 - DB
	#define MAX_WEAPONS	350
#endif

#ifdef INF_BUILD
#define MAX_WEAPON_TYPES				300
#else
#define MAX_WEAPON_TYPES				200
#endif


// from model.h

#define MAX_MODEL_TEXTURES	64


// from object.h; probably will be redone eventually
//	Team bitmasks.
#define TEAM_HOSTILE		(1 << 0)
#define TEAM_FRIENDLY	(1 << 1)
#define TEAM_NEUTRAL		(1 << 2)
#define TEAM_UNKNOWN		(1 << 3)
#define TEAM_TRAITOR		(1	<< 4)
#define TEAM_ANY			(TEAM_HOSTILE|TEAM_FRIENDLY|TEAM_NEUTRAL|TEAM_UNKNOWN|TEAM_TRAITOR)
#define MAX_TEAM_NAMES_INDEX	TEAM_TRAITOR


// from pstypes.h - don't ask me why it was there
#define MAX_TEAMS		3


// from scoring.h
// ARGH. IMPORTANT : do not change NUM_MEDALS without talking to DaveB first. It will affect the size of the scoring struct and hence, will break
// a lot of PXO related stuff. SEE ALSO : MAX_SHIP_TYPES
#ifdef FS2_DEMO
	#define NUM_MEDALS			16
	#define NUM_MEDALS_FS1		16
#else 
	#define NUM_MEDALS			18
	#define NUM_MEDALS_FS1		16
#endif


// object.h
#ifdef FS2_DEMO
	#define MAX_OBJECTS			300		
#else
	#define MAX_OBJECTS			1000		
#endif



#endif	// _GLOBALS_H