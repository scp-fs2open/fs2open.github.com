/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MODELANIM_H
#define _MODELANIM_H


#define MAX_TRIGGERED_ANIMATIONS 15

// WMC: Steps to adding a triggered animation
// 1 - add TRIGGER_TYPE_* define
// 2 - increment MAX_TRIGGER_ANIMATION_TYPES
// 3 - add name to animation_type_names array
// 4 - add start trigger (model_anim_start_type)
// 5 - add stop trigger (model_anim_start_type)

#define TRIGGER_TYPE_NONE					-1		// No animation
#define TRIGGER_TYPE_INITIAL				0		// This is just the position the subobject should be placed in
#define TRIGGER_TYPE_DOCKING_STAGE_1		1		// Before you dock
#define TRIGGER_TYPE_DOCKING_STAGE_2		2		// Before you dock
#define TRIGGER_TYPE_DOCKING_STAGE_3		3		// Before you dock
#define TRIGGER_TYPE_DOCKED					4		// As you dock
#define TRIGGER_TYPE_PRIMARY_BANK			5		// Primary banks
#define TRIGGER_TYPE_SECONDARY_BANK			6		// Secondary banks
#define TRIGGER_TYPE_DOCK_BAY_DOOR			7		// Fighter bays
#define TRIGGER_TYPE_AFTERBURNER			8		// Afterburner -C
#define TRIGGER_TYPE_TURRET_FIRING			9		// Turret shooting -C
#define TRIGGER_TYPE_SCRIPTED				10		// Triggered exclusively by scripting...maybe SEXPs? -C
#define TRIGGER_TYPE_TURRET_FIRED			11		// Triggered after a turret has fired -The E

#define MAX_TRIGGER_ANIMATION_TYPES			12

extern char *Animation_type_names[MAX_TRIGGER_ANIMATION_TYPES];


// Model Animation Position settings
enum EModelAnimationPosition {
    MA_POS_NOT_SET          = 0,	// not yet setup
    MA_POS_SET              = 1,	// set, but is moving
    MA_POS_READY            = 2     // set, done with move
};

#define ANIMATION_SUBTYPE_ALL -1

/**
 * This is an object responsable for storing the animation information assosiated with 
 * a specific triggered animation, one subobject can have many triggered animations
 */
struct queued_animation {
	queued_animation();
	void correct();

	vec3d angle;
	vec3d vel;
	vec3d accel;
	int start;
	int start_time;
	int end;
	int end_time;
	int reverse_start;
	bool absolute;
	int type;
	int subtype;
	int instance;
	int real_end_time;

	int start_sound;
	int loop_sound;
	int end_sound;
	float snd_rad;

	char sub_name[NAME_LENGTH];
};

/*
struct trigger_instance{
	int type;
	int sub_type;
	queued_animation properties;
	void corect();
};
*/

/**
 * This is the triggered animation object, it is responsable for controlling how the current triggered animation works
 * rot_accel is the acceleration for starting to move and stopping, so figure it in twice.
 */
class triggered_rotation
{
	private:
		vec3d snd_pnt;
		int start_sound;
		int loop_sound;
		int end_sound;
		int current_snd;
		int current_snd_index;
		float snd_rad;
		int obj_num;

		int n_queue;
		queued_animation queue[MAX_TRIGGERED_ANIMATIONS];
		queued_animation queue_tmp[MAX_TRIGGERED_ANIMATIONS];

	public:
		triggered_rotation();
		~triggered_rotation();

		void start(queued_animation *q);
		void set_to_initial(queued_animation *q);
		void set_to_final(queued_animation *q);
		void apply_trigger_angles(angles *submodel_angles);

		void add_queue(queued_animation *new_queue, int dir);
		void process_queue();

		vec3d current_ang;
		vec3d current_vel;
		vec3d rot_accel;	// rotational acceleration, 0 means instant
		vec3d rot_vel;		// radians per second, hold this speed when rot_accel has pushed it to this
		vec3d slow_angle;	// angle that we should start to slow down
		vec3d end_angle;	// lock it in
		vec3d direction;

		int instance;		// which animation this is (for reversals)
		bool has_started;	// animation has started playing
		int end_time;		// time that we should stop
		int start_time;		// the time the current animation started
};


// functions...

struct model_subsystem;
struct ship_subsys;
struct ship;
struct ship_info;

void model_anim_submodel_trigger_rotate(model_subsystem *psub, ship_subsys *ss);
void model_anim_set_initial_states(ship *shipp);
void model_anim_fix_reverse_times(ship_info *sip);

// gets animation type index from string name
int model_anim_match_type(char *p);

// starts an animation of a certan type that may be assosiated with a submodel of a ship (returns true if an animation was started)
bool model_anim_start_type(ship_subsys *pss, int animation_type, int subtype, int direction, bool instant = false);	// for a specific subsystem
bool model_anim_start_type(ship *shipp, int animation_type, int subtype, int direction, bool instant = false);		// for all valid subsystems

// how long until the animation is done
int model_anim_get_time_type(ship_subsys *pss, int animation_type, int subtype);	// for a specific subsystem
int model_anim_get_time_type(ship *shipp, int animation_type, int subtype);			// for all valid subsystems

// this is for handling multiplayer-safe, client-side, animations
void model_anim_handle_multiplayer(ship *shipp);


// for pushing and popping animations
typedef struct stack_item
{
	ship *shipp;
	int animation_type;
	int subtype;
	int direction;
	bool instant;
} stack_item;

typedef SCP_vector<stack_item> animation_stack;

extern SCP_map<int, animation_stack> Animation_map;

bool model_anim_push_and_start_type(int stack_unique_id, ship *shipp, int animation_type, int subtype, int direction, bool instant = false);
bool model_anim_pop_and_start_type(int stack_unique_id);

#endif // _MODELANIM_H
