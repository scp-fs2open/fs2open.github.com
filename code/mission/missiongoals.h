/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Mission/MissionGoals.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:59 $
 * $Author: penguin $
 *
 *  Header file for Mission support.  Included detection of primary
 *  and secondary goals.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 8     9/06/99 9:46p Jefff
 * skip mission support
 * 
 * 7     8/28/99 4:54p Dave
 * Fixed directives display for multiplayer clients for wings with
 * multiple waves. Fixed hud threat indicator rendering color.
 * 
 * 6     7/29/99 3:06p Andsager
 * Increase max number of events to 150
 * 
 * 5     7/29/99 2:58p Jefff
 * Ingame objective screen icon key now uses normal objective icons and
 * text is drawn in code.
 * 
 * 4     2/17/99 2:10p Dave
 * First full run of squad war. All freespace and tracker side stuff
 * works.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 51    5/21/98 2:47a Lawrance
 * Fix some problems with event music
 * 
 * 50    4/15/98 9:05a Allender
 * fix skpping of training mission with branchs
 * 
 * 49    4/03/98 2:47p Allender
 * made directives act different when multiple waves of a wing take a long
 * time to reappear
 * 
 * 48    3/31/98 12:23a Allender
 * changed macro names of campaign types to be more descriptive.  Added
 * "team" to objectives dialog for team v. team missions.  Added two
 * distinct multiplayer campaign types
 * 
 * 47    2/27/98 4:37p Hoffoss
 * Combined Objectives screen into Mission Log screen.
 * 
 * 46    2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 45    2/22/98 4:30p John
 * More string externalization classification
 * 
 * 44    2/20/98 8:33p Lawrance
 * Added mission_goals_incomplete()
 * 
 * 43    1/30/98 4:24p Hoffoss
 * Added a 3 second delay for directives before they get displayed.
 * 
 * 42    1/28/98 6:21p Dave
 * Made the standalone use ~8 megs less memory. Fixed multiplayer submenu
 * endgame problem.
 * 
 * 41    1/27/98 11:00a Lawrance
 * Fix bug with showing number of resolved goals in the objective status
 * popup.
 * 
 * 40    1/15/98 5:23p Lawrance
 * Add HUD gauge to indicate completed objectives.
 * 
 * 39    1/12/98 5:17p Allender
 * fixed primary fired problem and ship warp out problem.  Made new
 * mission goal info packet to deal with goals more accurately. 
 * 
 * 38    12/29/97 12:16p Johnson
 * Upped event max.
 * 
 * 37    12/22/97 6:07p Hoffoss
 * Made directives flash when completed, fixed but with is-destroyed
 * operator.
 * 
 * 36    12/19/97 12:43p Hoffoss
 * Changed code to allow counts in directives.
 * 
 * 35    12/01/97 12:26a Lawrance
 * Add flag  MGF_NO_MUSIC to mission_goal struct, to avoid playing music
 * for certain goals
 * 
 * 34    11/02/97 10:09p Lawrance
 * add source control header
 *
 * $NoKeywords: $
 */

#ifndef _MISSIONGOAL_H
#define _MISSIONGOAL_H

#include "object.h"
#include "ai.h"
#include "cfile.h"

// defines for types of primary and secondary missions

#define	MAX_GOALS			30		// maximum number of goals for any given mission

// defines for types of goals.  We will use part of the int field of the mission_goal struct
// as a bit field for goal flags

#define	PRIMARY_GOAL		0
#define	SECONDARY_GOAL		1
#define	BONUS_GOAL			2

// defines for bitfields of type, (and mask to get the type field quickly)
#define	INVALID_GOAL		(1 << 16)			// is this goal valid or not?
#define	GOAL_TYPE_MASK		(0xffff)				// mask to get us the type

// defines for goal status.  These status are also used in campaign file for marking goal status
// in campaign save file
#define	GOAL_FAILED			0		// status of goal
#define	GOAL_COMPLETE		1		
#define	GOAL_INCOMPLETE	2

#define	PRIMARY_GOALS_COMPLETE		1
#define	PRIMARY_GOALS_INCOMPLETE	0
#define	PRIMARY_GOALS_FAILED			-1

extern char *Goal_type_text(int n);

// structures for primary and secondary goals

#define	MAX_GOAL_TEXT	128

#define	MGF_NO_MUSIC	(1<<0)		// don't play any event music when goal is achieved

typedef struct mission_goal {
	char	name[NAME_LENGTH];			// used for storing status of goals in player file
	int  	type;								// primary/secondary/bonus
	int	satisfied;						// has this goal been satisfied
	char	message[MAX_GOAL_TEXT];		//	Brief description, such as "Destroy all vile aliens!"
	int	rating;							//	Some importance figure or something.
	int	formula;							//	Index in Sexp_nodes of this Sexp.
	int	score;							// score for this goal
	int	flags;							// MGF_
	int	team;								// which team is this objective for.
} mission_goal;

extern mission_goal Mission_goals[MAX_GOALS];	// structure for the goals of this mission
extern int	Num_goals;									// number of goals for this mission

// structures and defines for mission events

#define MAX_MISSION_EVENTS		150
#define MISSION_EVENTS_WARN	100

// defined for event states.  We will also use the satisfied/failed for saving event information
// in campaign save file
#define EVENT_UNBORN			0  // event can't be evaluated yet
#define EVENT_CURRENT		1  // event can currently be evaluated, but not satisfied or failed yet
#define EVENT_SATISFIED		2
#define EVENT_FAILED			3
#define EVENT_INCOMPLETE	4	// used in campaign save file.  used when event isn't satisfied yet

#define MEF_CURRENT					(1 << 0)		// is event current or past current yet?
#define MEF_DIRECTIVE_SPECIAL		(1 << 1)		// used to mark a directive as true even though not fully satisfied
#define MEF_DIRECTIVE_TEMP_TRUE	(1 << 2)		// this directive is temporarily true.

typedef struct mission_event {
	char	name[NAME_LENGTH];	// used for storing status of events in player file
	int	formula;					// index into sexpression array for this formula
	int	result;					// result of most recent evaluation of event
	int	repeat_count;			// number of times to repeat this goal
	int	interval;				// interval (in seconds) at which an evaulation is repeated once true.
	int	timestamp;				// set at 'interval' seconds when we start to eval.
	int	score;					// score for this event
	int	chain_delay;
	int	flags;
	char	*objective_text;
	char	*objective_key_text;
	int	count;					// object count for directive display
	int	satisfied_time;
	int	born_on_date;			// timestamp at which event was born
	int	team;						// for multiplayer games
} mission_event;

extern int Num_mission_events;
extern mission_event Mission_events[MAX_MISSION_EVENTS];
extern int Mission_goal_timestamp;
extern int Event_index;  // used by sexp code to tell what event it came from

// prototypes
void	mission_init_goals( void );
void	mission_show_goals_init();
void	mission_show_goals_close();
void	mission_show_goals_do_frame(float frametime);	// displays goals on screen
void	mission_eval_goals();									// evaluate player goals
int	mission_ai_goal_achievable( ai_goals *aigp );	// determines if an AI goal is achievable
void	mission_add_ai_goal( int sexp, ai_info *aip );	// adds a goal onto the given ai_info structure
int	mission_evaluate_primary_goals(void);	// determine if the primary goals for the mission are complete -- returns one of the above defines
int	mission_goals_met();

// function used by single and multiplayer code to change the status on goals	
void	mission_goal_status_change( int goal_num, int new_status);

// functions used to change goal validity status
void mission_goal_mark_invalid( char *name );
void mission_goal_mark_valid( char *name );

// function used to mark all goals as invalid, and incomplete goals as invalid.
extern void mission_goal_fail_all();
extern void mission_goal_fail_incomplete();

void mission_goal_fetch_num_resolved(int desired_type, int *num_resolved, int *total, int team = -1);
int mission_goals_incomplete(int desired_type, int team = -1);
void mission_goal_mark_objectives_complete();
void mission_goal_mark_events_complete();

int mission_get_event_status(int event);
void mission_event_shutdown();
void mission_goal_validation_change( int goal_num, int valid );

// mark an event as directive special
void mission_event_set_directive_special(int event);
void mission_event_unset_directive_special(int event);

void mission_goal_exit();

int ML_objectives_init(int x, int y, int w, int h);
void ML_objectives_close();
void ML_objectives_do_frame(int scroll_offset);
void ML_render_objectives_key();			// renders objectives key on ingame objectives screen

#endif
