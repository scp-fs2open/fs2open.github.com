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

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "io/timer.h"

struct ai_goal;
struct ai_info;

// defines for types of primary and secondary missions

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
	SCP_string name;                        // used for storing status of goals in player file
	int  type = PRIMARY_GOAL;               // primary/secondary/bonus
	int  satisfied = GOAL_INCOMPLETE;       // has this goal been satisfied
	SCP_string message;                     //	Brief description, such as "Destroy all vile aliens!"
	int	 formula = -1;                      //	Index in Sexp_nodes of this Sexp.
	int  score = 0;                         // score for this goal
	int  flags = 0;                         // MGF_
	int  team = 0;                          // which team is this objective for (defaults to the first team)
} mission_goal;
extern SCP_vector<mission_goal> Mission_goals;	// structure for the goals of this mission

// structures and defines for mission events

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
#define MEF_USE_MSECS				(1 << 4)		// Goober5000 - interval and chain delay are in milliseconds, not seconds
#define MEF_TIMESTAMP_HAS_INTERVAL	(1 << 5)		// Goober5000 - flag to simulate Volition's gloriously buggy hack

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
	SCP_string name;                                    // used for storing status of events in player file
	int  formula = -1;                                  // index into sexpression array for this formula
	int  result = 0;                                    // result of most recent evaluation of event
	int  repeat_count = 1;                              // number of times to test this goal
	int  trigger_count = 1;                             // number of times to allow this goal to trigger
	int  interval = 1;                                  // interval (in seconds) at which an evaulation is repeated once true.
	TIMESTAMP timestamp = TIMESTAMP::invalid();         // set at 'interval' seconds when we start to eval.
	int  score = 0;                                     // score for this event
	int  chain_delay = -1;
	int  flags = 0;                                     // MEF_*
	SCP_string objective_text;
	SCP_string objective_key_text;
	int	count = 0;                                      // object count for directive display
	TIMESTAMP satisfied_time = TIMESTAMP::invalid();    // this is used to temporarily mark the directive as satisfied when the event isn't (e.g. for a destroyed wave when there are more waves later)
	TIMESTAMP born_on_date = TIMESTAMP::invalid();      // timestamp at which event was born
	int team = -1;                                      // for multiplayer games

	// event log stuff
	int mission_log_flags = 0;                          // flags that are used to determing which events are written to the log
	SCP_vector<SCP_string> event_log_buffer;
	SCP_vector<SCP_string> event_log_variable_buffer;
	SCP_vector<SCP_string> event_log_container_buffer;
	SCP_vector<SCP_string> event_log_argument_buffer;
	SCP_vector<SCP_string> backup_log_buffer;
	int	previous_result = 0;                            // result of previous evaluation of event

} mission_event;
extern SCP_vector<mission_event> Mission_events;

extern TIMESTAMP Mission_goal_timestamp;
extern int Event_index;  // used by sexp code to tell what event it came from
extern bool Log_event;
extern bool Snapshot_all_events;


// only used in FRED
struct event_annotation
{
	void *handle = nullptr;			// the handle of the tree node in the event editor.  This is an HTREEITEM in FRED and TBD in qtFRED.
	int item_image = -1;			// the previous image of the tree node (replaced by a comment icon when there is a comment)
	SCP_list<int> path;				// a way to find the node that the annotation represents:
									// the first number is the event, the second number is the node on the first layer, etc.
	SCP_string comment;
	ubyte r = 255;
	ubyte g = 255;
	ubyte b = 255;
};
extern SCP_vector<event_annotation> Event_annotations;


// prototypes
void mission_goals_and_events_init( void );
void	mission_show_goals_init();
void	mission_show_goals_close();
void	mission_show_goals_do_frame(float frametime);	// displays goals on screen
void	mission_eval_goals();									// evaluate player goals
int	mission_evaluate_primary_goals(void);	// determine if the primary goals for the mission are complete -- returns one of the above defines
int	mission_goals_met();

// function used by single and multiplayer code to change the status on goals	
void	mission_goal_status_change( int goal_num, int new_status);

// function used to change goal validity status
void mission_goal_mark_valid( const char *name, bool valid );

// function used to mark all goals as invalid, and incomplete goals as invalid.
extern void mission_goal_fail_all();
extern void mission_goal_fail_incomplete();

void mission_goal_fetch_num_resolved(int desired_type, int *num_resolved, int *total, int team = -1);
int mission_goals_incomplete(int desired_type, int team = -1);
void mission_goal_mark_objectives_complete();
void mission_goal_mark_events_complete();

int mission_get_event_status(int event);
void mission_goal_validation_change( int goal_num, bool valid );

// mark an event as directive special
void mission_event_set_directive_special(int event);
void mission_event_unset_directive_special(int event);

// Cyborg - set the directive completion sound timestamp
void mission_event_set_completion_sound_timestamp();

// Maybe play a directive success sound... need to poll since the sound is delayed from when
// the directive is actually satisfied.
void mission_maybe_play_directive_success_sound();

void mission_goal_exit();

int mission_goal_find_sexp_tree(int root_node);
int mission_event_find_sexp_tree(int root_node);

int mission_goal_lookup(const char *name);
int mission_event_lookup(const char *name);

int ML_objectives_init(int x, int y, int w, int h);
void ML_objectives_close();
void ML_objectives_do_frame(int scroll_offset);
void ML_render_objectives_key();			// renders objectives key on ingame objectives screen

#endif
