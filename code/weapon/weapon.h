/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _WEAPON_H
#define _WEAPON_H

#include "globalincs/systemvars.h"
#include "graphics/2d.h"
#include "globalincs/globals.h"
#include "weapon/trails.h"
#include "weapon/shockwave.h"
#include "graphics/generic.h"
#include "model/model.h"

class object;
class ship_subsys;

#define	WP_UNUSED			-1
#define	WP_LASER			0		// PLEASE NOTE that this flag specifies ballistic primaries as well - Goober5000
#define	WP_MISSILE			1
#define	WP_BEAM				2
extern char *Weapon_subtype_names[];
extern int Num_weapon_subtypes;

#define WRT_NONE	-1
#define	WRT_LASER	1
#define	WRT_POF		2

//	Bitflags controlling weapon behavior
#define	MAX_WEAPON_FLAGS	18						//	Maximum number of different bit flags legal to specify in a single weapons.tbl Flags line

#define WIF_DEFAULT_VALUE	0
#define WIF2_DEFAULT_VALUE	0
#define WIF3_DEFAULT_VALUE	0

#define	WIF_HOMING_HEAT	(1 << 0)				//	if set, this weapon homes via seeking heat
#define	WIF_HOMING_ASPECT	(1 << 1)				//	if set, this weapon homes via chasing aspect
#define	WIF_ELECTRONICS	(1 << 2)				//	Takes out electronics systems.
#define	WIF_SPAWN			(1 << 3)				//	Spawns projectiles on detonation.
#define	WIF_REMOTE			(1 << 4)				//	Can be remotely detonated by parent.
#define	WIF_PUNCTURE		(1 << 5)				//	Punctures armor, damaging subsystems.
#define	WIF_SUPERCAP		(1 << 6)				//	This is a weapon which does supercap class damage (meaning, it applies real damage to supercap ships)
#define WIF_CMEASURE		(1 << 7)				// Weapon acts as a countermeasure
//#define	WIF_AREA_EFFECT	(1 << 7)				//	Explosion has an area effect
//#define	WIF_SHOCKWAVE		(1 << 8)				//	Explosion has a shockwave
//WMC - These are no longer needed so these spots are free
#define	WIF_HOMING_JAVELIN	(1 << 8)				// WC Saga Javelin HS style heatseeker, locks only on target's engines
#define	WIF_TURNS			(1 << 9)				// Set this if the weapon ever changes heading.  If you
															// don't set this and the weapon turns, collision detection
															// won't work, I promise!
#define	WIF_SWARM			(1 << 10)			// Missile "swarms".. ie changes heading and twists on way to target
#define	WIF_TRAIL			(1 << 11)			//	Has a trail
#define	WIF_BIG_ONLY		(1 << 12)			//	Only big ships (cruiser, capital, etc.) can arm this weapon
#define	WIF_CHILD			(1 << 13)			//	No ship can have this weapon.  It gets created by weapon detonations.
#define	WIF_BOMB				(1 << 14)			// Bomb-type missile, can be targeted
#define	WIF_HUGE				(1 << 15)			//	Huge damage (generally 500+), probably only fired at huge ships.
#define	WIF_NO_DUMBFIRE	(1	<<	16)			// Missile cannot be fired dumbfire (ie requires aspect lock)
#define	WIF_THRUSTER		(1 << 17)			// Has thruster cone and/or glow
#define	WIF_IN_TECH_DATABASE		(1 << 18)
#define	WIF_PLAYER_ALLOWED		(1 << 19)   // allowed to be on starting wing ships/in weaponry pool
#define	WIF_BOMBER_PLUS	(1 << 20)			//	Fire this missile only at a bomber or big ship.  But not a fighter.

#define	WIF_CORKSCREW		(1 << 21)			// corkscrew style missile
#define	WIF_PARTICLE_SPEW	(1 << 22)			// spews particles as it travels
#define	WIF_EMP				(1 << 23)			// weapon explodes with a serious EMP effect
#define	WIF_ENERGY_SUCK	(1 << 24)			// energy suck primary (impact effect)
#define	WIF_FLAK				(1 << 25)			// use for big-ship turrets - flak gun
#define	WIF_BEAM				(1 << 26)			// if this is a beam weapon : NOTE - VERY SPECIAL CASE
#define	WIF_TAG				(1 << 27)			// this weapon has a tag effect when it hits
#define	WIF_SHUDDER			(1 << 28)			// causes the weapon to shudder. shudder is proportional to the mass and damage of the weapon
#define	WIF_LOCKARM			(1 << 29)			// if the missile was fired without a lock, it does significanlty less damage on impact
#define	WIF_STREAM			(1 << 30)			// handled by "trigger down/trigger up" instead of "fire - wait - fire - wait"
// NOTE: the remaining WIF is reserved for a flag that truly needs to be in the WIF_ flag field,
// as opposed to any typical new flag that can be added to WIF3

#define WIF2_BALLISTIC					(1 << 0)	// ballistic primaries - Goober5000
#define WIF2_PIERCE_SHIELDS				(1 << 1)	// shield pierceing -Bobboau
#define WIF2_DEFAULT_IN_TECH_DATABASE	(1 << 2)	// default in tech database - Goober5000
#define WIF2_LOCAL_SSM					(1 << 3)	// localized ssm. ship that fires ssm is in mission.  ssms also warp back in during mission
#define WIF2_TAGGED_ONLY				(1 << 4)	// can only fire if target is tagged
#define WIF2_CYCLE						(1 << 5)	// will only fire from (shots (defalts to 1)) points at a time
#define WIF2_SMALL_ONLY					(1 << 6)	// can only be used against small ships like fighters or bombers
#define WIF2_SAME_TURRET_COOLDOWN		(1 << 7)	// the weapon has the same cooldown time on turrets
#define WIF2_MR_NO_LIGHTING				(1 << 8)	// don't render with lighting, regardless of user options
#define WIF2_TRANSPARENT				(1 << 9)	// render as transparent
#define WIF2_TRAINING					(1 << 10)	// Weapon does shield/hull damage, but doesn't hurt subsystems, whack you, or put marks on your ship.
#define WIF2_SMART_SPAWN				(1 << 11)   // Spawn weapon that is fired via turrets like normal weapons
#define WIF2_INHERIT_PARENT_TARGET		(1 << 12)   // child weapons home in on the target their parent is homing on.
#define WIF2_NO_EMP_KILL				(1 << 13)	// though weapon has hitpoints it can not be disabled by EMP
#define WIF2_VARIABLE_LEAD_HOMING		(1 << 14)	// allows user defined scaler to be added to lead (to enable, lead, pure or lag pursuit for missiles)
#define WIF2_UNTARGETED_HEAT_SEEKER		(1 << 15)	// forces heat seeker to lose target immeadiately (and acquire a random new one)
#define WIF2_HARD_TARGET_BOMB			(1 << 16)	// removes the radius doubling effect bombs have for collisions
#define WIF2_NON_SUBSYS_HOMING			(1 << 17)	// spreads fired missiles around the target ships hull
#define WIF2_NO_LIFE_LOST_IF_MISSED		(1 << 18)	// prevents game from shortening the lifeleft of the missed but still homing missiles
#define WIF2_CUSTOM_SEEKER_STR			(1 << 19)	// sets the game to use custom seeker strengths instead of default values
#define WIF2_CAN_BE_TARGETED			(1 << 20)	// allows non-bomb weapons to be targeted
#define WIF2_SHOWN_ON_RADAR				(1 << 21)	// allows non-bombs be visible on radar
#define WIF2_SHOW_FRIENDLY				(1 << 22)	// allows friendly weapon radar dots be drawn
#define WIF2_CAPITAL_PLUS				(1 << 23)   // AI will not use this weapon on fighters or bombers
#define WIF2_EXTERNAL_WEAPON_FP			(1 << 24)	// will try to use external models FPs if possible
#define WIF2_EXTERNAL_WEAPON_LNCH		(1 << 25)	// render external secondary as a launcher
#define WIF2_TAKES_BLAST_DAMAGE			(1 << 26)	// This weapon can take blast damage
#define WIF2_TAKES_SHOCKWAVE_DAMAGE		(1 << 27)	// This weapon can take shockwave damage
#define WIF2_DONT_SHOW_ON_RADAR			(1 << 28)   // Force a weapon to not show on radar
#define WIF2_RENDER_FLAK				(1 << 29)	// Even though this is a flak weapon, render the shell
#define WIF2_CIWS						(1 << 30)	// This weapons' burst and shockwave damage can damage bombs (Basically, a reverse for TAKES_BLAST/SHOCKWAVE_DAMAGE
#define WIF2_ANTISUBSYSBEAM				(1 << 31)	// This beam can target subsystems as per normal

#define WIF3_NOLINK						(1 << 0)	// This weapon can not be linked with others
#define WIF3_USE_EMP_TIME_FOR_CAPSHIP_TURRETS	(1 << 1)	// override MAX_TURRET_DISRUPT_TIME in emp.cpp - Goober5000
#define WIF3_NO_LINKED_PENALTY			(1 << 2)	// This weapon does not count into linked firing penalty
#define WIF3_NO_HOMING_SPEED_RAMP 		(1 << 3)	// Disables the 1s long speed ramping when firing locked-on secondaries
#define WIF3_CMEASURE_ASPECT_HOME_ON	(1 << 4)	// This countermeasure flag makes aspect seekers home on the countermeasure instead of going into dumbfire mode
#define WIF3_TURRET_INTERCEPTABLE		(1 << 5)	// These two flags mark a weapon as being interceptable by the AI
#define WIF3_FIGHTER_INTERCEPTABLE		(1 << 6)	// (like WIF_BOMB), without forcing it to be tagetable -MageKing17
#define WIF3_AOE_ELECTRONICS			(1 << 7)	// Apply electronics effect across the weapon's entire area of effect instead of just on the impacted ship -MageKing17
#define WIF3_APPLY_RECOIL				(1 << 8)	// Apply recoil using weapon and ship info

#define	WIF_HOMING					(WIF_HOMING_HEAT | WIF_HOMING_ASPECT | WIF_HOMING_JAVELIN)
#define WIF_LOCKED_HOMING           (WIF_HOMING_ASPECT | WIF_HOMING_JAVELIN)
#define WIF_HURTS_BIG_SHIPS			(WIF_BOMB | WIF_BEAM | WIF_HUGE | WIF_BIG_ONLY)

#define	WEAPON_EXHAUST_DELTA_TIME	75		//	Delay in milliseconds between exhaust blobs

#define WF_LOCK_WARNING_PLAYED		(1<<0)		// set when a lock warning sound is played for the player
																//	(needed since we don't want to play multiple lock sounds)
#define WF_ALREADY_APPLIED_STATS		(1<<1)		// for use in ship_apply_local and ship_apply_global damage functions
																// so that we don't record multiple hits (stats) for one impact
#define WF_PLAYED_FLYBY_SOUND			(1<<2)		// flyby sound has been played for this weapon
#define WF_CONSIDER_FOR_FLYBY_SOUND	(1<<3)		// consider for flyby
#define WF_DEAD_IN_WATER				(1<<4)		// a missiles engines have died
#define WF_LOCKED_WHEN_FIRED			(1<<5)		// fired with a lock
#define WF_DESTROYED_BY_WEAPON		(1<<6)		// destroyed by damage from other weapon
#define WF_SPAWNED					(1<<7)		//Spawned from a spawning type weapon
#define WF_HOMING_UPDATE_NEEDED		(1<<8)		// this is a newly spawned homing weapon which needs to update client machines

// flags for setting burst fire 
#define WBF_FAST_FIRING				(1<<0)		// burst is to use only the firewait to determine firing delays
#define WBF_RANDOM_LENGTH			(1<<1)		// burst is to fire random length bursts

//particle names go here -nuke
#define PSPEW_NONE		-1			//used to disable a spew, useful for xmts
#define PSPEW_DEFAULT	0			//std fs2 pspew
#define PSPEW_HELIX		1			//q2 style railgun trail
#define PSPEW_SPARKLER	2			//random particles in every direction, can be sperical or ovoid
#define PSPEW_RING		3			//outward expanding ring
#define PSPEW_PLUME		4			//spewers arrayed within a radius for thruster style effects, may converge or scatter

#define MAX_PARTICLE_SPEWERS	4	//i figure 4 spewers should be enough for now -nuke
#define MAX_WEP_DAMAGE_SLOTS	32		//Maximum number of ships which can be counted as killer or assits on destroying this weapon

typedef struct weapon {
	int		weapon_info_index;			// index into weapon_info array
	int		objnum;							// object number for this weapon
	int		team;								// The team of the ship that fired this
	int		species;							// The species of the ship that fired thisz
	float		lifeleft;						// life left on this weapon	
	vec3d	start_pos;

	int		target_num;						//	Object index of target
	int		target_sig;						//	So we know if the target is the same one we've been tracking
	float		nearest_dist;					//	nearest distance yet attained to target
	fix		creation_time;					//	time at which created, stuffed Missiontime
	int		weapon_flags;					//	bit flags defining behavior, see WF_xxxx
	object*	homing_object;					//	object this weapon is homing on.
	ship_subsys*	homing_subsys;			// subsystem this weapon is homing on
	vec3d	homing_pos;						// world position missile is homing on
	short		swarm_index;					// index into swarm missile info, -1 if not WIF_SWARM
	int		missile_list_index;			// index for missiles into Missile_obj_list, -1 weapon not missile
	trail		*trail_ptr;						// NULL if no trail, otherwise a pointer to its trail
	ship_subsys *turret_subsys;			// points to turret that fired weapon, otherwise NULL
	int		group_id;						// Which group this is in.
	float	det_range;					//How far from start_pos it blows up

	// Stuff for thruster glows
	int		thruster_bitmap;					// What frame the current thruster bitmap is at for this weapon
	float		thruster_frame;					// Used to keep track of which frame the animation should be on.
	int		thruster_glow_bitmap;			// What frame the current thruster engine glow bitmap is at for this weapon
	float		thruster_glow_frame;				// Used to keep track of which frame the engine glow animation should be on.
	float		thruster_glow_noise;				// Noise for current frame

	// laser stuff
	float	laser_bitmap_frame;				// used to keep track of which frame the animation should be on
	float	laser_glow_bitmap_frame;		// used to keep track of which frame the glow animation should be on

	int		pick_big_attack_point_timestamp;	//	Timestamp at which to pick a new point to attack.
	vec3d	big_attack_point;				//	Target-relative location of attack point.

	int		cmeasure_ignore_objnum;		//	Ignoring this countermeasure.  It's failed to attract this weapon.
	int		cmeasure_chase_objnum;		//	Chasing this countermeasure.  Don't maybe ignore in future.

	// corkscrew info (taken out for now)
	short	cscrew_index;						// corkscrew info index

	// particle spew info
	int		particle_spew_time[MAX_PARTICLE_SPEWERS];			// time to spew next bunch of particles	
	float	particle_spew_rand;				// per weapon randomness value used by some particle spew types -nuke

	// flak info
	short flak_index;							// flak info index

	//local ssm stuff		
	fix lssm_warpout_time;		//time at which the missile warps out
	fix lssm_warpin_time;		//time at which the missile warps back in
	int lssm_stage;				//what stage its in 1=just launched, 2=warping out. 3=warped out, 4=warping back in, 5=terminal dive						
	int lssm_warp_idx;			//warphole index
	float lssm_warp_time;		//length of time warphole stays open		
	float lssm_warp_pct;		//how much of the warphole's life should be dedicated to stage 2
	vec3d lssm_target_pos;

	// weapon transparency info
	ubyte alpha_backward;		// 1 = move in reverse (ascending in value)
	float alpha_current;		// the current alpha value

	float weapon_max_vel;		// might just as well store the data here
	float launch_speed;			// the initial forward speed (can vary due to additive velocity or acceleration)
								// currently only gets set when weapon_info->acceleration_time is used

	bool collisionOccured;
	mc_info collisionInfo; // The last collision of this weapon or NULL if it had none
	//Scoring stuff
	float total_damage_received;        // total damage received (for scoring purposes)
	float damage_ship[MAX_WEP_DAMAGE_SLOTS];    // damage applied from each player
	int   damage_ship_id[MAX_WEP_DAMAGE_SLOTS]; // signature of the damager (corresponds to each entry in damage_ship)

	int hud_in_flight_snd_sig;					// Signature of the sound played while the weapon is in flight
} weapon;


// info specific to beam weapons
typedef struct beam_weapon_section_info {
	float width;							// width of the section
	float flicker;							// how much it flickers (0.0 to 1.0)
	float z_add;							// is this necessary?
	float tile_factor;						// texture tile factor -Bobboau
	int tile_type;							// is this beam tiled by it's length, or not
	float translation;						// makes the beam texture move -Bobboau
	generic_anim texture;					// texture anim/bitmap
} beam_weapon_section_info;

typedef struct beam_weapon_info {
	int beam_type;						// beam type
	float beam_life;					// how long it lasts
	int beam_warmup;					// how long it takes to warmup (in ms)
	int beam_warmdown;					// how long it takes to warmdown (in ms)
	float beam_muzzle_radius;			// muzzle glow radius
	int beam_particle_count;			// beam spew particle count
	float beam_particle_radius;			// radius of beam particles
	float beam_particle_angle;			// angle of beam particle spew cone
	generic_anim beam_particle_ani;		// particle_ani
	float beam_iff_miss_factor[MAX_IFFS][NUM_SKILL_LEVELS];	// magic # which makes beams miss more. by parent iff and player skill level
	int beam_loop_sound;				// looping beam sound
	int beam_warmup_sound;				// warmup sound
	int beam_warmdown_sound;			// warmdown sound
	int beam_num_sections;				// the # of visible "sections" on the beam
	generic_anim beam_glow;				// muzzle glow bitmap
	int beam_shots;						// # of shots the beam takes
	float beam_shrink_factor;			// what percentage of total beam lifetime when the beam starts shrinking
	float beam_shrink_pct;				// what percent/second the beam shrinks at
	beam_weapon_section_info sections[MAX_BEAM_SECTIONS];	// info on the visible sections of the beam 	
	float range;						// how far it will shoot-Bobboau
	float damage_threshold;				// point at wich damage will start being atenuated from 0.0 to 1.0
	float beam_width;					// width of the beam (for certain collision checks)
} beam_weapon_info;

typedef struct particle_spew_info {	//this will be used for multi spews
	// particle spew stuff
	int particle_spew_type;			//added pspew type field -nuke
	int particle_spew_count;
	int particle_spew_time;
	float particle_spew_vel;
	float particle_spew_radius;
	float particle_spew_lifetime;
	float particle_spew_scale;
	float particle_spew_z_scale;	//length value for some effects -nuke
	float particle_spew_rotation_rate;	//rotation rate for some particle effects -nuke
	vec3d particle_spew_offset;			//offsets and normals, yay!
	vec3d particle_spew_velocity;
	generic_anim particle_spew_anim;
} particle_spew_info;

typedef struct spawn_weapon_info 
{
	short	spawn_type;							//	Type of weapon to spawn when detonated.
	short	spawn_count;						//	Number of weapons of spawn_type to spawn.
	float	spawn_angle;						//  Angle to spawn the child weapons in.  default is 180
} spawn_weapon_info;

#define MAX_SPAWN_TYPES_PER_WEAPON 5

// use this to extend a beam to "infinity"
#define BEAM_FAR_LENGTH				30000.0f


extern weapon Weapons[MAX_WEAPONS];

#define WEAPON_TITLE_LEN			48

enum InFlightSoundType
{
	TARGETED,
	UNTARGETED,
	ALWAYS
};

#define MAX_SUBSTITUTION_PATTERNS	10

typedef struct weapon_info {
	char	name[NAME_LENGTH];				// name of this weapon
	char	alt_name[NAME_LENGTH];			// alt name of this weapon
	char	title[WEAPON_TITLE_LEN];		// official title of weapon (used by tooltips)
	char	*desc;								// weapon's description (used by tooltips)
	int		subtype;								// one of the WP_* macros above
	int		render_type;						//	rendering method, laser, pof, avi
	char	pofbitmap_name[MAX_FILENAME_LEN];	// Name of the pof representing this if POF, or bitmap filename if bitmap
	int		model_num;							// modelnum of weapon -- -1 if no model
	char	external_model_name[MAX_FILENAME_LEN];					//the model rendered on the weapon points of a ship
	int		external_model_num;					//the model rendered on the weapon points of a ship
	int		hud_target_lod;						// LOD to use when rendering weapon model to the hud targetbox
	int		num_detail_levels;					// number of LODs defined in table (optional)
	int		detail_distance[MAX_MODEL_DETAIL_LEVELS]; // LOD distances define in table (optional)
	char	*tech_desc;								// weapon's description (in tech database)
	char	tech_anim_filename[MAX_FILENAME_LEN];	// weapon's tech room animation
	char	tech_title[NAME_LENGTH];			// weapon's name (in tech database)
	char	tech_model[MAX_FILENAME_LEN];		//Image to display in the techroom (TODO) or the weapon selection screen if the ANI isn't specified/missing

	vec3d	closeup_pos;						// position for camera to set an offset for viewing the weapon model
	float	closeup_zoom;						// zoom when using weapon model in closeup view in loadout selection

	char hud_filename[MAX_FILENAME_LEN];			//Name of image to display on HUD in place of text
	int hud_image_index;					//teh index of the image

	generic_anim laser_bitmap;				// bitmap for a laser
	generic_anim laser_glow_bitmap;			// optional laser glow bitmap

	float laser_length;
	color	laser_color_1;						// for cycling between glow colors
	color	laser_color_2;						// for cycling between glow colors
	float	laser_head_radius, laser_tail_radius;

	float	max_speed;							// max speed of the weapon
	float	acceleration_time;					// how many seconds to reach max speed (secondaries only)
	float	vel_inherit_amount;					// how much of the parent ship's velocity is inherited (0.0..1.0)
	float	free_flight_time;
	float mass;									// mass of the weapon
	float fire_wait;							// fire rate -- amount of time before you can refire the weapon

	float	damage;								//	damage of weapon (for missile, damage within inner radius)

	shockwave_create_info shockwave;
	shockwave_create_info dinky_shockwave;
	fix arm_time;
	float arm_dist;
	float arm_radius;
	float det_range;
	float det_radius;					//How far from target or target subsystem it blows up
	float flak_detonation_accuracy;		//How far away from a target a flak shell will blow up. Standard is 65.0f
	float flak_targeting_accuracy;		//Determines the amount of jitter applied to flak targeting. USE WITH CAUTION!
	float untargeted_flak_range_penalty; //Untargeted flak shells detonate after travelling max range - this parameter. Default 20.0f

	float	armor_factor, shield_factor, subsystem_factor;	//	in 0.0..2.0, scale of damage done to type of thing
	float life_min;
	float life_max;
	float max_lifetime ;						// How long this weapon will actually live for
	float	lifetime;						// How long the AI thinks this thing lives (used for distance calculations etc)
	float energy_consumed;					// Energy used up when weapon is fired
	int	wi_flags;							//	bit flags defining behavior, see WIF_xxxx
	int wi_flags2;							// stupid int wi_flags, only 32 bits... argh - Goober5000
	int wi_flags3;							// stupid int wi_flags2, only 32 bits... argh - The E
	float turn_time;
	float	cargo_size;							// cargo space taken up by individual weapon (missiles only)
	float rearm_rate;							// rate per second at which secondary weapons are loaded during rearming
	float	weapon_range;						// max range weapon can be effectively fired.  (May be less than life * speed)

    // spawn weapons
    int num_spawn_weapons_defined;
    int total_children_spawned;
    spawn_weapon_info spawn_info[MAX_SPAWN_TYPES_PER_WEAPON];

	// swarm count
	short swarm_count;						// how many swarm missiles are fired for this weapon

	//	Specific to ASPECT homing missiles.
	float	min_lock_time;						// minimum time (in seconds) to achieve lock
	int	lock_pixels_per_sec;				// pixels/sec moved while locking
	int	catchup_pixels_per_sec;			// pixels/sec moved while catching-up for a lock				
	int	catchup_pixel_penalty;			// number of extra pixels to move while locking as a penalty for catching up for a lock			

	//	Specific to HEAT homing missiles.
	float	fov;

	// Seeker strength - for countermeasures overhaul.
	float seeker_strength;

	int pre_launch_snd;
	int	pre_launch_snd_min_interval;	//Minimum interval in ms between the last time the pre-launch sound was played and the next time it can play, as a limiter in case the player is pumping the trigger
	int	launch_snd;
	int	impact_snd;
	int disarmed_impact_snd;
	int	flyby_snd;							//	whizz-by sound, transmitted through weapon's portable atmosphere.
	
	// Specific to weapons with TRAILS:
	trail_info tr_info;			

	char	icon_filename[MAX_FILENAME_LEN];	// filename for icon that is displayed in weapon selection
	char	anim_filename[MAX_FILENAME_LEN];	// filename for animation that plays in weapon selection
	int 	selection_effect;

	int	impact_weapon_expl_index;		// Index into Weapon_expl_info of which ANI should play when this thing impacts something
	float	impact_explosion_radius;		// How big the explosion should be

	int dinky_impact_weapon_expl_index;
	float dinky_impact_explosion_radius;

	int flash_impact_weapon_expl_index;
	float flash_impact_explosion_radius;

	int piercing_impact_weapon_expl_index;
	float piercing_impact_explosion_radius;
	int piercing_impact_particle_count;
	float piercing_impact_particle_life;
	float piercing_impact_particle_velocity;
	float piercing_impact_particle_back_velocity;
	float piercing_impact_particle_variance;

	// EMP effect
	float emp_intensity;					// intensity of the EMP effect
	float emp_time;						// time of the EMP effect

	// Recoil effect
	float recoil_modifier;

	// Energy suck effect
	float weapon_reduce;					// how much energy removed from weapons systems
	float afterburner_reduce;			// how much energy removed from weapons systems

	// Beam weapon effect	
	beam_weapon_info	b_info;			// this must be valid if the weapon is a beam weapon WIF_BEAM or WIF_BEAM_SMALL

	// tag stuff
	float	tag_time;						// how long the tag lasts		
	int tag_level;							// tag level (1 - 3)

	// muzzle flash
	int muzzle_flash;						// muzzle flash stuff
	
	// SSM
	int SSM_index;							// wich entry in the SSM,tbl it uses -Bobboau

	// now using new particle spew struct -nuke
	particle_spew_info particle_spewers[MAX_PARTICLE_SPEWERS];

	// Corkscrew info - phreak 11/9/02
	int cs_num_fired;
	float cs_radius;
	float cs_twist;
	int cs_crotate;
	int cs_delay;

	//electronics info - phreak 5/3/03
	int elec_time;				//how long it lasts, in milliseconds
	float elec_eng_mult;		//multiplier on engine subsystem
	float elec_weap_mult;		//multiplier on weapon subsystem and turrets
	float elec_beam_mult;		//used instead of elec_weap_mult if turret is a beam turret
	float elec_sensors_mult;	//multiplier on sensors and awacs
	int elec_randomness;		//disruption time lasts + or - this value from whats calculated.  time in milliseconds
	int elec_use_new_style;		//use new style electronics parameters

	//local ssm info
	int lssm_warpout_delay;			//delay between launch and warpout (ms)
	int lssm_warpin_delay;			//delay between warpout and warpin (ms)
	float lssm_stage5_vel;			//velocity during final stage
	float lssm_warpin_radius;
	float lssm_lock_range;

	float field_of_fire;			//cone the weapon will fire in, 0 is strait all the time-Bobboau
	float fof_spread_rate;			//How quickly the FOF will spread for each shot (primary weapons only, this doesn't really make sense for turrets)
	float fof_reset_rate;			//How quickly the FOF spread will reset over time (primary weapons only, this doesn't really make sense for turrets)
	float max_fof_spread;			//The maximum fof increase that the shots can spread to
	int	  shots;					//the number of shots that will be fired at a time, 
									//only realy usefull when used with FOF to make a shot gun effect
									//now also used for weapon point cycleing

	// Countermeasure information
	float cm_aspect_effectiveness;
	float cm_heat_effectiveness;
	float cm_effective_rad;

    // *
               
    int SwarmWait;                  // *Swarm firewait, default is 150  -Et1

    float WeaponMinRange;           // *Minimum weapon range, default is 0 -Et1


	float weapon_submodel_rotate_accell;
	float weapon_submodel_rotate_vel;
	
	int damage_type_idx;
	int damage_type_idx_sav;	// stored value from table used to reset damage_type_idx

	int armor_type_idx;	// Weapon armor type


	// transparency/alpha info
	float alpha_max;			// maximum alpha value to use
	float alpha_min;			// minimum alpha value to use
	float alpha_cycle;			// cycle between max and min by this much each frame

	int weapon_hitpoints;

	int	burst_shots;
	float burst_delay;
	int burst_flags;

	// Thruster effects
	generic_anim	thruster_flame;
	generic_anim	thruster_glow;
	float			thruster_glow_factor;

	float			target_lead_scaler;
	int				targeting_priorities[32];
	int				num_targeting_priorities;

	// the optional pattern of weapons that this weapon will fire
	size_t			num_substitution_patterns;
	int				weapon_substitution_pattern[MAX_SUBSTITUTION_PATTERNS]; //weapon_indexes
	char			weapon_substitution_pattern_names[MAX_SUBSTITUTION_PATTERNS][NAME_LENGTH]; // weapon names so that we can generate the indexs after sort

	int			score; //Optional score for destroying the weapon

	int hud_tracking_snd; // Sound played when this weapon tracks a target
	int hud_locked_snd; // Sound played when this weapon locked onto a target
	int hud_in_flight_snd; // Sound played while the weapon is in flight
	InFlightSoundType in_flight_play_type; // The status when the sound should be played
} weapon_info;

// Data structure to track the active missiles
typedef struct missile_obj {
	missile_obj *next, *prev;
	int			flags, objnum;
} missile_obj;
extern missile_obj Missile_obj_list;

// WEAPON EXPLOSION INFO
#define MAX_WEAPON_EXPL_LOD						4

typedef struct weapon_expl_lod {
	char	filename[MAX_FILENAME_LEN];
	int	bitmap_id;
	int	num_frames;
	int	fps;

	weapon_expl_lod( ) 
		: bitmap_id( -1 ), num_frames( 0 ), fps( 0 )
	{ 
		filename[ 0 ] = 0;
	}
} weapon_expl_lod;

typedef struct weapon_expl_info	{
	int					lod_count;	
	weapon_expl_lod		lod[MAX_WEAPON_EXPL_LOD];
} weapon_expl_info;

class weapon_explosions
{
private:
	SCP_vector<weapon_expl_info> ExplosionInfo;
	int GetIndex(char *filename);

public:
	weapon_explosions();

	int Load(char *filename = NULL, int specified_lods = MAX_WEAPON_EXPL_LOD);
	int GetAnim(int weapon_expl_index, vec3d *pos, float size);
	void PageIn(int idx);
};

extern weapon_explosions Weapon_explosions;

extern weapon_info Weapon_info[MAX_WEAPON_TYPES];

extern int Num_weapon_types;			// number of weapons in the game
extern int Num_weapons;
extern int First_secondary_index;
extern int Default_cmeasure_index;

extern int Num_player_weapon_precedence;				// Number of weapon types in Player_weapon_precedence
extern int Player_weapon_precedence[MAX_WEAPON_TYPES];	// Array of weapon types, precedence list for player weapon selection

#define WEAPON_INDEX(wp)				(wp-Weapons)
#define WEAPON_INFO_INDEX(wip)		(wip-Weapon_info)


int weapon_info_lookup(const char *name = NULL);
void weapon_init();					// called at game startup
void weapon_close();				// called at game shutdown
void weapon_level_init();			// called before the start of each level
void weapon_render(object * obj);
void weapon_delete( object * obj );
void weapon_process_pre( object *obj, float frame_time);
void weapon_process_post( object *obj, float frame_time);

//Call before weapons_page_in to mark a weapon as used
void weapon_mark_as_used(int weapon_id);

// Group_id:  If you should quad lasers, they should all have the same group id.  
// This will be used to optimize lighting, since each group only needs to cast one light.
// Call this to get a new group id, then pass it to each weapon_create call for all the
// weapons in the group.   Number will be between 0 and WEAPON_MAX_GROUP_IDS and will
// get reused.
int weapon_create_group_id();

// How many unique groups of weapons there can be at one time. 
#define WEAPON_MAX_GROUP_IDS 256

// Passing a group_id of -1 means it isn't in a group.  See weapon_create_group_id for more 
// help on weapon groups.
int weapon_create( vec3d * pos, matrix * orient, int weapon_type, int parent_obj, int group_id=-1, int is_locked = 0, int is_spawned = 0, float fof_cooldown = 0.0f, ship_subsys * src_turret = NULL);
void weapon_set_tracking_info(int weapon_objnum, int parent_objnum, int target_objnum, int target_is_locked = 0, ship_subsys *target_subsys = NULL);

// for weapons flagged as particle spewers, spew particles. wheee
void weapon_maybe_spew_particle(object *obj);

bool weapon_armed(weapon *wp, bool hit_target);
void weapon_hit( object * weapon_obj, object * other_obj, vec3d * hitpos, int quadrant = -1 );
int cmeasure_name_lookup(char *name);
void spawn_child_weapons( object *objp );

// call to detonate a weapon. essentially calls weapon_hit() with other_obj as NULL, and sends a packet in multiplayer
void weapon_detonate(object *objp);

void	weapon_area_apply_blast(vec3d *force_apply_pos, object *ship_objp, vec3d *blast_pos, float blast, int make_shockwave);
int	weapon_area_calc_damage(object *objp, vec3d *pos, float inner_rad, float outer_rad, float max_blast, float max_damage,
										float *blast, float *damage, float limit);

void	missile_obj_list_rebuild();	// called by save/restore code only
missile_obj *missile_obj_return_address(int index);
void find_homing_object_cmeasures();

// THE FOLLOWING FUNCTION IS IN SHIP.CPP!!!!
// JAS - figure out which thruster bitmap will get rendered next
// time around.  ship_render needs to have shipp->thruster_bitmap set to
// a valid bitmap number, or -1 if we shouldn't render thrusters.
// This does basically the same thing as ship_do_thruster_frame, except it
// operates on a weapon.   This is in the ship code because it needs
// the same thruster animation info as the ship stuff, and I would
// rather extern this one function than all the thruster animation stuff.
void ship_do_weapon_thruster_frame( weapon *weaponp, object *objp, float frametime );

// call to get the "color" of the laser at the given moment (since glowing lasers can cycle colors)
void weapon_get_laser_color(color *c, object *objp);

void weapon_hit_do_sound(object *hit_obj, weapon_info *wip, vec3d *hitpos, bool is_armed);

void weapon_do_electronics_effect(object *ship_objp, vec3d *blast_pos, int wi_index);

// return a scale factor for damage which should be applied for 2 collisions
float weapon_get_damage_scale(weapon_info *wip, object *wep, object *target);

// Pauses all running weapon sounds
void weapon_pause_sounds();

// Unpauses all running weapon sounds
void weapon_unpause_sounds();

#endif
