/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/globals.h $
 * $Revision: 1.18 $
 * $Date: 2005-09-27 05:25:18 $
 * $Author: Goober5000 $
 *
 * Header for common global #defines, to cut down on #includes
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.17  2005/09/27 02:36:57  Goober5000
 * clarification
 * --Goober5000
 *
 * Revision 1.16  2005/09/24 01:50:10  Goober5000
 * a bunch of support ship bulletproofing
 * --Goober5000
 *
 * Revision 1.15  2005/07/13 02:50:48  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 1.14  2005/07/13 02:08:26  Goober5000
 * move inferno vs. network check here for the time being
 * --Goober5000
 *
 * Revision 1.13  2005/07/13 02:01:29  Goober5000
 * fixed a bunch of "issues" caused by me with the species stuff
 * --Goober5000
 *
 * Revision 1.12  2005/05/11 08:10:20  Goober5000
 * variables should now work properly in messages that are sent multiple times
 * --Goober5000
 *
 * Revision 1.11  2005/05/08 20:28:29  wmcoolmon
 * Dynamically allocated medals
 *
 * Revision 1.10  2005/04/28 05:29:29  wmcoolmon
 * Removed FS2_DEMO defines that looked like they wouldn't cause the universe to collapse
 *
 * Revision 1.9  2005/04/03 08:48:31  Goober5000
 * brought weapon loadout banks into agreement with ship info banks
 * improved error reporting on apply-to-all
 * --Goober5000
 *
 * Revision 1.8  2005/02/15 00:06:26  taylor
 * clean up some model related globals
 * code to disable individual thruster glows
 * fix issue where 1 extra OGL light pass didn't render
 *
 * Revision 1.7  2005/01/31 23:27:52  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 1.6  2004/12/30 21:41:24  Goober5000
 * stupid parenthesis fix
 * --Goober5000
 *
 * Revision 1.5  2004/12/14 14:46:12  Goober5000
 * allow different wing names than ABGDEZ
 * --Goober5000
 *
 * Revision 1.4  2004/08/11 05:06:23  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 1.3  2004/05/10 10:51:54  Goober5000
 * made primary and secondary banks quite a bit more friendly... added error-checking
 * and reorganized a bunch of code
 * --Goober5000
 *
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
#define TRAINING_MESSAGE_LENGTH	512

// from missionparse.h
#define MISSION_DESC_LENGTH		512


// from player.h
#define CALLSIGN_LEN					28		//	shortened from 32 to allow .plr to be attached without exceeding MAX_FILENAME_LEN
#define SHORT_CALLSIGN_PIXEL_W	80		// max width of short_callsign[] in pixels

// from missionparse.h
// This must be less than or equal to the number of bits in an int!  If you really must have that many species,
// then please update the support_ship_info struct.
#define	MAX_SPECIES		8

#define MAX_IFFS		10


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

// INF_BUILD error check...
#if defined(INF_BUILD) && !defined(NO_NETWORK)
#error "Networking *must* be disabled with Inferno builds.  Please #define NO_NETWORK."
#endif


#define MAX_WINGS				25

#define MAX_SHIPS_PER_WING			6

#define MAX_STARTING_WINGS			3	// number of wings player can start a mission with
#define MAX_SQUADRON_WINGS			5	// number of wings in squadron (displayed on HUD)
#define MAX_TVT_WINGS				2	// number of wings in a TVT game

// from ship.h
#define MAX_PLAYER_PRIMARY_BANKS	2
#define MAX_PLAYER_SECONDARY_BANKS	3
#define MAX_PLAYER_WEAPONS			(MAX_PLAYER_PRIMARY_BANKS+MAX_PLAYER_SECONDARY_BANKS)


// from model.h
#define MAX_SHIP_PRIMARY_BANKS		3
#define MAX_SHIP_SECONDARY_BANKS	4
#define MAX_SHIP_WEAPONS			(MAX_SHIP_PRIMARY_BANKS+MAX_SHIP_SECONDARY_BANKS)


// limit checks - Goober5000

#if (MAX_PLAYER_PRIMARY_BANKS > MAX_SHIP_PRIMARY_BANKS)
	#error MAX_PLAYER_PRIMARY_BANKS must be less than or equal to MAX_SHIP_PRIMARY_BANKS
#endif

#if (MAX_PLAYER_SECONDARY_BANKS > MAX_SHIP_SECONDARY_BANKS)
	#error MAX_PLAYER_SECONDARY_BANKS must be less than or equal to MAX_SHIP_SECONDARY_BANKS
#endif


// Goober5000 - moved from hudescort.cpp
// size of complete escort list, including all those wanting to get onto list but without space
#define MAX_COMPLETE_ESCORT_LIST	20
             

// from weapon.h
// upped 5/6/98 from 200 - DB
#define MAX_WEAPONS	350

#ifdef INF_BUILD
#define MAX_WEAPON_TYPES				300
#else
#define MAX_WEAPON_TYPES				200
#endif


// from model.h

#define MAX_MODEL_TEXTURES	64

#ifdef INF_BUILD
	#define MAX_POLYGON_MODELS  300
	#define MAX_BUFFERS_PER_SUBMODEL 24
#else
	#define MAX_POLYGON_MODELS  128 //DTP reset from 198 to original value of 128
	#define MAX_BUFFERS_PER_SUBMODEL 16
#endif


// from object.h; probably will be redone eventually
//	Team bitmasks.
#define TEAM_HOSTILE		(1 << 0)
#define TEAM_FRIENDLY		(1 << 1)
#define TEAM_NEUTRAL		(1 << 2)
#define TEAM_UNKNOWN		(1 << 3)
#define TEAM_TRAITOR		(1	<< 4)
#define TEAM_ANY			(TEAM_HOSTILE|TEAM_FRIENDLY|TEAM_NEUTRAL|TEAM_UNKNOWN|TEAM_TRAITOR)
#define MAX_TEAM_NAMES_INDEX	TEAM_TRAITOR


// from pstypes.h - don't ask me why it was there
#define MAX_TVT_TEAMS		3


// from scoring.h
// ARGH. IMPORTANT : do not change NUM_MEDALS without talking to DaveB first. It will affect the size of the scoring struct and hence, will break
// a lot of PXO related stuff. SEE ALSO : MAX_SHIP_TYPES
#ifdef FS2_DEMO
	//#define NUM_MEDALS			16
#else 
	//#define NUM_MEDALS			18
#endif

#define MAX_MEDALS			18
#define NUM_MEDALS_FS1		16
extern int Num_medals;

// object.h
#define MAX_OBJECTS			1000		



#endif	// _GLOBALS_H
