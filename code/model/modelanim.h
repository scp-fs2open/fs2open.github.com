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
// 1 - add TRIGGER_TYPE define
// 2 - increment MAX_TRIGGER_ANIMATION_TYPES
// 3 - add name to animation_type_names array
// 4 - add start trigger (model_anim_start_type)
// 5 - add stop trigger (model_anim_start_type)

#define TRIGGER_TYPE_NONE					-1		//no animation
#define TRIGGER_TYPE_INITIAL				0		//this is just the position the subobject should be placed in
#define TRIGGER_TYPE_DOCKING				1		//before you dock
#define TRIGGER_TYPE_DOCKED					2		//after you have docked
#define TRIGGER_TYPE_PRIMARY_BANK			3		//primary banks
#define TRIGGER_TYPE_SECONDARY_BANK			4		//secondary banks
#define TRIGGER_TYPE_DOCK_BAY_DOOR			5		//fighter bays
#define TRIGGER_TYPE_AFTERBURNER			6		//Afterburner -C
#define TRIGGER_TYPE_TURRET_FIRING			7		//Turret shooting -C
#define TRIGGER_TYPE_SCRIPTED				8		//Triggered exclusively by scripting...maybe SEXPs? -C

#define MAX_TRIGGER_ANIMATION_TYPES			9

// Model Animation Position settings
#define MA_POS_NOT_SET		0	// not yet setup
#define MA_POS_SET			1	// set, but is moving
#define MA_POS_READY		2	// set, done with move

#define ANIMATION_SUBTYPE_ALL -1

// this is an object responsable for storeing the animation information assosiated with
// a specific triggered animation, one subobject can have many triggered animations
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
};

/*
struct trigger_instance{
	int type;
	int sub_type;
	queued_animation properties;
	void corect();
};
*/

// this is the triggered animation object, it is responcable for controleing how the current triggered animation works
// rot_accel is the accelleration for starting to move and stopping, so figure it in twice
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
		void set_to_end(queued_animation *q);

		void add_queue(queued_animation *new_queue, int dir);
		void process_queue();

		vec3d current_ang;
		vec3d current_vel;
		vec3d rot_accel;	// rotational accelleration, 0 means instant
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
bool model_anim_start_type(ship_subsys *pss, int animation_type, int subtype, int direction);	// for a specific subsystem
bool model_anim_start_type(ship *shipp, int animation_type, int subtype, int direction);		// for all valid subsystems

// how long until the animation is done
int model_anim_get_time_type(ship_subsys *pss, int animation_type, int subtype);	// for a specific subsystem
int model_anim_get_time_type(ship *shipp, int animation_type, int subtype);			// for all valid subsystems

// this is for handling multiplayer-safe, client-side, animations
void model_anim_handle_multiplayer(ship *shipp);

#endif // _MODELANIM_H
