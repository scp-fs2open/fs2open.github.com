/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MISSIONGOAL_H
#define _MISSIONGOAL_H

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"

struct ai_goal;
struct ai_info;

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

extern const char *Goal_type_text(int n);

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

#define MAX_MISSION_EVENTS		512
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
#define MEF_DIRECTIVE_TEMP_TRUE		(1 << 2)		// this directive is temporarily true.
#define MEF_USING_TRIGGER_COUNT		(1 << 3)		// Karajorma - use trigger count as well as repeat count to determine how many repeats this event has

#define MAX_MISSION_EVENT_LOG_FLAGS		9			// this must be changed if a mission log flag is added below

#define MLF_SEXP_TRUE				(1 << 0)
#define MLF_SEXP_FALSE				(1 << 1)
//#define MLF_SEXP_KNOWN_TRUE			(1 << 2)
#define MLF_SEXP_KNOWN_FALSE		(1 << 3)		
#define MLF_FIRST_REPEAT_ONLY		(1 << 4)
#define MLF_LAST_REPEAT_ONLY		(1 << 5)
#define MLF_FIRST_TRIGGER_ONLY		(1 << 6)
#define MLF_LAST_TRIGGER_ONLY		(1 << 7)
#define MLF_STATE_CHANGE			(1 << 8)	

#define MLF_ALL_REPETITION_FLAGS (MLF_FIRST_REPEAT_ONLY | MLF_LAST_REPEAT_ONLY | MLF_FIRST_TRIGGER_ONLY | MLF_LAST_TRIGGER_ONLY) 

typedef struct mission_event {
	char	name[NAME_LENGTH];	// used for storing status of events in player file
	int	formula;					// index into sexpression array for this formula
	int	result;					// result of most recent evaluation of event
	int	repeat_count;			// number of times to test this goal
	int trigger_count;			// number of times to allow this goal to trigger
	int	interval;				// interval (in seconds) at which an evaulation is repeated once true.
	int	timestamp;				// set at 'interval' seconds when we start to eval.
	int	score;					// score for this event
	int	chain_delay;
	int	flags;
	char	*objective_text;
	char	*objective_key_text;
	int	count;					// object count for directive display
	int	satisfied_time;			// this is used to temporarily mark the directive as satisfied when the event isn't (e.g. for a destroyed wave when there are more waves later)
	int	born_on_date;			// timestamp at which event was born
	int	team;						// for multiplayer games

	// event log stuff
	int mission_log_flags;		// flags that are used to determing which events are written to the log
	SCP_vector<SCP_string> event_log_buffer;
	SCP_vector<SCP_string> event_log_variable_buffer;
	SCP_vector<SCP_string> event_log_argument_buffer;
	SCP_vector<SCP_string> backup_log_buffer;
	int	previous_result;		// result of previous evaluation of event

} mission_event;

extern int Num_mission_events;
extern mission_event Mission_events[MAX_MISSION_EVENTS];
extern int Mission_goal_timestamp;
extern int Event_index;  // used by sexp code to tell what event it came from
extern bool Log_event;
extern bool Snapshot_all_events;

// prototypes
void	mission_init_goals( void );
void	mission_show_goals_init();
void	mission_show_goals_close();
void	mission_show_goals_do_frame(float frametime);	// displays goals on screen
void	mission_eval_goals();									// evaluate player goals
int	mission_ai_goal_achievable( ai_goal *aigp );	// determines if an AI goal is achievable
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
