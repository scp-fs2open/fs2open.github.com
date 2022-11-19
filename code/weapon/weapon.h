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

#include "globalincs/globals.h"
#include "globalincs/systemvars.h"
#include "globalincs/pstypes.h"

#include "actions/Program.h"
#include "decals/decals.h"
#include "gamesnd/gamesnd.h"
#include "graphics/2d.h"
#include "graphics/color.h"
#include "graphics/generic.h"
#include "model/model.h"
#include "model/modelanimation.h"
#include "particle/ParticleManager.h"
#include "weapon/beam.h"
#include "weapon/shockwave.h"
#include "weapon/swarm.h"
#include "weapon/trails.h"
#include "weapon/weapon_flags.h"
#include "model/modelrender.h"
#include "render/3d.h"

class object;
class ship_subsys;

#define	WP_UNUSED			-1
#define	WP_LASER			0		// PLEASE NOTE that this flag specifies ballistic primaries as well - Goober5000
#define	WP_MISSILE			1
#define	WP_BEAM				2
extern const char *Weapon_subtype_names[];
extern int Num_weapon_subtypes;

#define WRT_NONE	-1
#define	WRT_LASER	1
#define	WRT_POF		2

// constants for weapon lock acquire methods
#define WLOCK_PIXEL		0
#define WLOCK_TIMER		1

// constants for weapon lock restrictions
#define LR_CURRENT_TARGET				0		// Only lock current target and subsystem
#define LR_CURRENT_TARGET_SUBSYS		1		// 
#define LR_ANY_TARGETS					2

// enum for multilock object type restriction
enum class LR_Objecttypes { LRO_SHIPS, LRO_WEAPONS };

//particle names go here -nuke
#define PSPEW_NONE		-1			//used to disable a spew, useful for xmts
#define PSPEW_DEFAULT	0			//std fs2 pspew
#define PSPEW_HELIX		1			//q2 style railgun trail
#define PSPEW_SPARKLER	2			//random particles in every direction, can be sperical or ovoid
#define PSPEW_RING		3			//outward expanding ring
#define PSPEW_PLUME		4			//spewers arrayed within a radius for thruster style effects, may converge or scatter

#define MAX_PARTICLE_SPEWERS	4	//i figure 4 spewers should be enough for now -nuke

// scale factor for supercaps taking damage from weapons which are not "supercap" weapons
#define SUPERCAP_DAMAGE_SCALE			0.25f

// default amount of time to wait after firing before a remote detonated missile can be detonated
#define DEFAULT_REMOTE_DETONATE_TRIGGER_WAIT  0.5f

// default value weapon max range is set to if there's not a tabled range value.
#define WEAPON_DEFAULT_TABLED_MAX_RANGE 999999999.9f

#define MAX_SPAWN_TYPES_PER_WEAPON 5

enum class WeaponState : uint32_t
{
	INVALID,

	// States for laser weapons
	NORMAL, //!< For laser weapons that have only one state

	// Missile states following
	FREEFLIGHT, //!< The initial flight state where the missile is "unpowered"
	IGNITION, //!< The moment when the missile comes out of free flight
	HOMED_FLIGHT, //!< The missile is homing in on its target
	UNHOMED_FLIGHT, //!< The missile does not currently target an object
};
struct WeaponStateHash {
	size_t operator()(const WeaponState& state) const {
		return static_cast<size_t>(state);
	}
};

typedef struct weapon {
	int		weapon_info_index;			// index into weapon_info array
	int		objnum;							// object number for this weapon
	int		model_instance_num;				// model instance number, if we have any intrinsic-moving submodels
	int		team;								// The team of the ship that fired this
	int		species;							// The species of the ship that fired thisz
	float		lifeleft;						// life left on this weapon	
	vec3d	start_pos;

	int		target_num;						//	Object index of target
	int		target_sig;						//	So we know if the target is the same one we've been tracking
	fix		creation_time;					//	time at which created, stuffed Missiontime
	flagset<Weapon::Weapon_Flags> weapon_flags;					//	bit flags defining behavior, see WF_xxxx
	object*	homing_object;					//	object this weapon is homing on.
	ship_subsys*	homing_subsys;			// subsystem this weapon is homing on
	vec3d	homing_pos;						// world position missile is homing on
	std::unique_ptr<swarm_info>		swarm_info_ptr;	// index into swarm missile info, -1 if not WIF_SWARM
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
	float	laser_bitmap_frame;				// these are used to keep track of which frame the relevant animations should be on
	float   laser_headon_bitmap_frame;
	float	laser_glow_bitmap_frame;	
	float   laser_glow_headon_bitmap_frame;

	int		pick_big_attack_point_timestamp;	//	Timestamp at which to pick a new point to attack.
	vec3d	big_attack_point;				//	Target-relative location of attack point.

	SCP_vector<int>* cmeasure_ignore_list;
	int		cmeasure_timer;

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

	int next_spawn_time[MAX_SPAWN_TYPES_PER_WEAPON];		// used for continuous child spawn

	mc_info* collisionInfo; // The last collision of this weapon or NULL if it had none

	sound_handle hud_in_flight_snd_sig; // Signature of the sound played while the weapon is in flight

	WeaponState weapon_state; // The current state of the weapon

	float beam_per_shot_rot; // for type 5 beams
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

enum class Type5BeamRotAxis {
	STARTPOS_OFFSET,
	ENDPOS_OFFSET, 
	STARTPOS_NO_OFFSET,
	ENDPOS_NO_OFFSET,
	CENTER,
	UNSPECIFIED
};

enum class Type5BeamPos {
	RANDOM_OUTSIDE,
	RANDOM_INSIDE,
	CENTER,
	SAME_RANDOM
};

typedef struct type5_beam_info {
	bool no_translate;                           // true if the end pos parameters were left unspecified
	Type5BeamPos start_pos;						 // whether it starts from the center or a type 0 or 1 beam kind of random
	Type5BeamPos end_pos;						 // same as above but but with an extra 'same random as start' option
	vec3d start_pos_offset;                      // position simply added to start pos (possibly manipulated by the bools below)
	vec3d end_pos_offset;                        // position simply added to end pos (possibly manipulated by the bools below)
	vec3d start_pos_rand;                        // same as above but a randomly chosen between defined value for each axis and its negative
	vec3d end_pos_rand;
	bool target_orient_positions;                // if true, offsets are oriented relative to the target, else the shooter's pov
	bool target_scale_positions;                 // if true, offsets are scaled by target radius, else its a fixed span from the shooters pov
	                                             // regardless of distance
	float continuous_rot;                        // radians per sec rotation over beam lifetime
	Type5BeamRotAxis continuous_rot_axis;		 // axis around which do continuous rotation
	SCP_vector<float> burst_rot_pattern;         // radians to rotate for each beam in a burst, will also make spawned and ssb beams fire 
	                                             // this many beams simultaneously with the defined rotations
	Type5BeamRotAxis burst_rot_axis;			 // axis around which to do burst rotation
	float per_burst_rot;                         // radians to rotate for each burst, or each shot if no burst
	Type5BeamRotAxis per_burst_rot_axis;		 // axis around which to do per burst rotation
} type5_beam_info;

typedef struct beam_weapon_info {
	BeamType beam_type;						// beam type
	float beam_life;					// how long it lasts
	int beam_warmup;					// how long it takes to warmup (in ms)
	int beam_warmdown;					// how long it takes to warmdown (in ms)
	float beam_muzzle_radius;			// muzzle glow radius
	int beam_particle_count;			// beam spew particle count
	float beam_particle_radius;			// radius of beam particles
	float beam_particle_angle;			// angle of beam particle spew cone
	generic_anim beam_particle_ani;		// particle_ani
	SCP_map<int, std::array<float, NUM_SKILL_LEVELS>> beam_iff_miss_factor;	// magic # which makes beams miss more. by parent iff and player skill level
	gamesnd_id beam_loop_sound;				// looping beam sound
	gamesnd_id beam_warmup_sound;				// warmup sound
	gamesnd_id beam_warmdown_sound;			// warmdown sound
	int beam_num_sections;				// the # of visible "sections" on the beam
	generic_anim beam_glow;				// muzzle glow bitmap
	float glow_length;					// (DahBlount) determines the length the muzzle glow when using a directional glow
	bool directional_glow;				// (DahBlount) makes the muzzle glow render to a poly that is oriented along the direction of fire
	int beam_shots;						// # of shots the beam takes
	float beam_shrink_factor;			// what percentage of total beam lifetime when the beam starts shrinking
	float beam_shrink_pct;				// what percent/second the beam shrinks at
	float beam_initial_width;		    // what percentage of total beam width the beam has initially
	float beam_grow_factor;			    // what percentage of total beam lifetime when the beam starts growing
	float beam_grow_pct;				// what percent/second the beam grows at
	beam_weapon_section_info sections[MAX_BEAM_SECTIONS];	// info on the visible sections of the beam 	
	float range;						// how far it will shoot-Bobboau
	float damage_threshold;				// point at wich damage will start being atenuated from 0.0 to 1.0
	float beam_width;					// width of the beam (for certain collision checks)
	bool beam_light_flicker;			// If beam light is affected by the flickering
	bool beam_light_as_multiplier;		// If beam light is used directly or multiplies the section width
	flagset<Weapon::Beam_Info_Flags> flags;
	type5_beam_info t5info;              // type 5 beams only
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
	float	spawn_min_angle;					//  Angle of spawning 'deadzone' inside spawn angle. Default 0.
	float	spawn_interval;						//  How often to do continuous spawn, negative is no continuous spawn
	float   spawn_interval_delay;               //  A delay before starting continuous spawn
	float   spawn_chance;						//  Liklihood of spawning on every spawn interval
	particle::ParticleEffectHandle spawn_effect; // Effect for continuous spawnings
} spawn_weapon_info;

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

enum class LockRestrictionType
{
	TYPE, CLASS, SPECIES, IFF
};

typedef std::pair<LockRestrictionType, int> lock_restriction;

enum class HomingAcquisitionType {
	CLOSEST,
	RANDOM,
};

struct weapon_info
{
	char	name[NAME_LENGTH];				// name of this weapon
	char	display_name[NAME_LENGTH];		// display name of this weapon
	char	title[WEAPON_TITLE_LEN];		// official title of weapon (used by tooltips)
	char	*desc;								// weapon's description (used by tooltips)
	char	altSubsysName[NAME_LENGTH];        // rename turret to this if this is the turrets first weapon

	char	pofbitmap_name[MAX_FILENAME_LEN];	// Name of the pof representing this if POF, or bitmap filename if bitmap
	int		model_num;							// modelnum of weapon -- -1 if no model
	char	external_model_name[MAX_FILENAME_LEN];					//the model rendered on the weapon points of a ship
	int		external_model_num;					//the model rendered on the weapon points of a ship

	char	*tech_desc;								// weapon's description (in tech database)
	char	tech_anim_filename[MAX_FILENAME_LEN];	// weapon's tech room animation
	char	tech_title[NAME_LENGTH];			// weapon's name (in tech database)
	char	tech_model[MAX_FILENAME_LEN];		//Image to display in the techroom (TODO) or the weapon selection screen if the ANI isn't specified/missing

	int		hud_target_lod;						// LOD to use when rendering weapon model to the hud targetbox
	int		num_detail_levels;					// number of LODs defined in table (optional)
	int		detail_distance[MAX_MODEL_DETAIL_LEVELS]; // LOD distances define in table (optional)
	int		subtype;								// one of the WP_* macros above
	int		render_type;						//	rendering method, laser, pof, avi

	vec3d	closeup_pos;						// position for camera to set an offset for viewing the weapon model
	float	closeup_zoom;						// zoom when using weapon model in closeup view in loadout selection

	char hud_filename[MAX_FILENAME_LEN];			//Name of image to display on HUD in place of text
	int hud_image_index;					//teh index of the image

	generic_anim laser_bitmap;				// bitmap for a laser
	generic_anim laser_headon_bitmap;		// optional bitmap for when viewed from ahead/behind
	generic_anim laser_glow_bitmap;			// optional laser glow bitmap
	generic_anim laser_glow_headon_bitmap;  // optional headon for the glow
	float laser_headon_switch_rate;			// how smooth vs sudden the transition between the headon and main bitmap should be
	float laser_headon_switch_ang;			// at what angle

	float laser_length;
	color	laser_color_1;						// for cycling between glow colors
	color	laser_color_2;						// for cycling between glow colors
	float	laser_head_radius, laser_tail_radius;
	float	collision_radius_override;          // overrides the radius for the purposes of collision

	bool 		light_color_set;
	hdr_color 	light_color;		//For the light cast by the projectile
	float 		light_radius;

	float	max_speed;							// max speed of the weapon
	float	acceleration_time;					// how many seconds to reach max speed (secondaries only)
	float	vel_inherit_amount;					// how much of the parent ship's velocity is inherited (0.0..1.0)
	float	free_flight_time;
	float	free_flight_speed_factor;
	float mass;									// mass of the weapon
	float fire_wait;							// fire rate -- amount of time before you can refire the weapon
	float max_delay;							// max time to delay a shot (DahBlount)
	float min_delay;							// min time to delay a shot	(DahBlount)

	float	damage;								//	damage of weapon (for missile, damage within inner radius)
	float	damage_time;						// point in the lifetime of the weapon at which damage starts to attenuate. This applies to non-beam primaries. (DahBlount)
	float	atten_damage;							// The damage to attenuate to. (DahBlount)
	float	damage_incidence_max;				// dmg multipler when weapon hits dead-on (perpindicular)
	float	damage_incidence_min;				// dmg multipler when weapon hits glancing (parallel)

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

	flagset<Weapon::Info_Flags>	wi_flags;							//	bit flags defining behavior, see WIF_xxxx

	float turn_time;
	float turn_accel_time;
	float	cargo_size;							// cargo space taken up by individual weapon (missiles only)
	float rearm_rate;							// rate per second at which secondary weapons are loaded during rearming
	int		reloaded_per_batch;				    // number of munitions rearmed per batch
	float	weapon_range;						// max range weapon can be effectively fired.  (May be less than life * speed)
	float	optimum_range;						// causes ai fighters to prefer this distance when attacking with the weapon
	float weapon_min_range;           // *Minimum weapon range, default is 0 -Et1

	bool pierce_objects;
	bool spawn_children_on_pierce;

    // spawn weapons
    int num_spawn_weapons_defined;
    int maximum_children_spawned;		// An upper bound for the total number of spawned children, used by multi
    spawn_weapon_info spawn_info[MAX_SPAWN_TYPES_PER_WEAPON];

	// swarm count
	short swarm_count;						// how many swarm missiles are fired for this weapon
	int SwarmWait;                  // *Swarm firewait, default is 150  -Et1

	int target_restrict;
	LR_Objecttypes target_restrict_objecttypes;
	bool multi_lock;
	int max_seeking;						// how many seekers can be active at a time if multilock is enabled. A value of one will lock stuff up one by one.
	int max_seekers_per_target;			// how many seekers can be attached to a target.

	SCP_vector<lock_restriction> ship_restrict;                     // list of pairs of types of restrictions, and the specific restriction of that type
	SCP_vector<std::pair<LockRestrictionType, SCP_string>> ship_restrict_strings;    // the above but the specific restrictions are the parsed strings (instead of looked up indicies)

	bool trigger_lock;						// Trigger must be held down and released to lock and fire.
	bool launch_reset_locks;				// Lock indicators reset after firing

	HomingAcquisitionType auto_target_method;

	//	Specific to ASPECT homing missiles.
	int acquire_method;
	float	min_lock_time;						// minimum time (in seconds) to achieve lock
	int	lock_pixels_per_sec;				// pixels/sec moved while locking
	int	catchup_pixels_per_sec;			// pixels/sec moved while catching-up for a lock				
	int	catchup_pixel_penalty;			// number of extra pixels to move while locking as a penalty for catching up for a lock		
	float lock_fov;

	//	Specific to HEAT homing missiles.
	float	fov;

	// Seeker strength - for countermeasures overhaul.
	float seeker_strength;

	gamesnd_id pre_launch_snd;
	int	pre_launch_snd_min_interval;	//Minimum interval in ms between the last time the pre-launch sound was played and the next time it can play, as a limiter in case the player is pumping the trigger
	gamesnd_id	launch_snd;
	gamesnd_id	impact_snd;
	gamesnd_id disarmed_impact_snd;
	gamesnd_id	flyby_snd;							//	whizz-by sound, transmitted through weapon's portable atmosphere.
	gamesnd_id	ambient_snd;
	
	gamesnd_id hud_tracking_snd; // Sound played when the player is tracking a target with this weapon
	gamesnd_id hud_locked_snd; // Sound played when the player is locked onto a target with this weapon
	gamesnd_id hud_in_flight_snd; // Sound played while the player has this weapon in flight
	InFlightSoundType in_flight_play_type; // The status when the sound should be played

	// Specific to weapons with TRAILS:
	trail_info tr_info;			

	char	icon_filename[MAX_FILENAME_LEN];	// filename for icon that is displayed in weapon selection
	char	anim_filename[MAX_FILENAME_LEN];	// filename for animation that plays in weapon selection
	int 	selection_effect;

	float shield_impact_explosion_radius;

	particle::ParticleEffectHandle impact_weapon_expl_effect; // Impact particle effect
	particle::ParticleEffectHandle dinky_impact_weapon_expl_effect; // Dinky impact particle effect
	particle::ParticleEffectHandle flash_impact_weapon_expl_effect;

	particle::ParticleEffectHandle piercing_impact_effect;
	particle::ParticleEffectHandle piercing_impact_secondary_effect;

	// Particle effect for the various states, WeaponState::NORMAL is the state for the whole lifetime, even for missiles
	SCP_unordered_map<WeaponState, particle::ParticleEffectHandle, WeaponStateHash> state_effects;

	// EMP effect
	float emp_intensity;					// intensity of the EMP effect
	float emp_time;						// time of the EMP effect

	// Recoil effect
	float recoil_modifier;

	// Energy suck effect
	float weapon_reduce;					// how much energy removed from weapons systems
	float afterburner_reduce;			// how much energy removed from weapons systems

	// Vampirism Effect Multiplier
	float vamp_regen;					// Factor by which a vampiric weapon will multiply the healing done to the shooter

	// tag stuff
	float	tag_time;						// how long the tag lasts		
	int tag_level;							// tag level (1 - 3)

	// muzzle flash
	int muzzle_flash;						// muzzle flash stuff
	
	float field_of_fire;			//cone the weapon will fire in, 0 is strait all the time-Bobboau
	float fof_spread_rate;			//How quickly the FOF will spread for each shot (primary weapons only, this doesn't really make sense for turrets)
	float fof_reset_rate;			//How quickly the FOF spread will reset over time (primary weapons only, this doesn't really make sense for turrets)
	float max_fof_spread;			//The maximum fof increase that the shots can spread to
	int	  shots;					//the number of shots that will be fired at a time, 
									//only realy usefull when used with FOF to make a shot gun effect
									//now also used for weapon point cycleing

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

	// SSM
	int SSM_index;							// wich entry in the SSM,tbl it uses -Bobboau

	//local ssm info
	int lssm_warpout_delay;			//delay between launch and warpout (ms)
	int lssm_warpin_delay;			//delay between warpout and warpin (ms)
	float lssm_stage5_vel;			//velocity during final stage
	float lssm_warpin_radius;
	float lssm_lock_range;
	int lssm_warpeffect;		//Which fireballtype is used for the warp effect

	// Beam weapon effect	
	beam_weapon_info	b_info;			// this must be valid if the weapon is a beam weapon WIF_BEAM or WIF_BEAM_SMALL

	// now using new particle spew struct -nuke
	particle_spew_info particle_spewers[MAX_PARTICLE_SPEWERS];

	// Countermeasure information
	float cm_aspect_effectiveness;
	float cm_heat_effectiveness;
	float cm_effective_rad;
	float cm_detonation_rad;
	bool  cm_kill_single;       // should the countermeasure kill just the single decoyed missile within CMEASURE_DETONATE_DISTANCE?
	int   cmeasure_timer_interval;	// how many milliseconds between pulses
	int cmeasure_firewait;						// delay in milliseconds between countermeasure firing --wookieejedi
	bool cmeasure_use_firewait;					// if set to true, then countermeasure will use specified firewait instead of default --wookieejedi
	int cmeasure_failure_delay_multiplier_ai;	// multiplier for firewait between failed countermeasure launches, next launch try = this value * firewait  --wookieejedi
	int cmeasure_sucess_delay_multiplier_ai;	// multiplier for firewait between successful countermeasure launches, next launch try = this value * firewait  --wookieejedi

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

	int	burst_shots;	// always 1 less than the actual burst length; 0 = no burst, 1 = two-shot burst, 2 = 3-shot, etc
	float burst_delay;
	flagset<Weapon::Burst_Flags> burst_flags;

	// Thruster effects
	generic_anim	thruster_flame;
	generic_anim	thruster_glow;
	float			thruster_glow_factor;

	float			target_lead_scaler;
	int				num_targeting_priorities;
	int				targeting_priorities[32];

	// Optional weapon failures
	float failure_rate;
	SCP_string failure_sub_name;
	int failure_sub;

	// the optional pattern of weapons that this weapon will fire
	size_t			num_substitution_patterns;
	int				weapon_substitution_pattern[MAX_SUBSTITUTION_PATTERNS]; //weapon_indexes
	char			weapon_substitution_pattern_names[MAX_SUBSTITUTION_PATTERNS][NAME_LENGTH]; // weapon names so that we can generate the indexs after sort

	int			score; //Optional score for destroying the weapon

	
	SCP_map<SCP_string, SCP_string> custom_data;

	decals::creation_info impact_decal;

	actions::ProgramSet on_create_program;

	animation::ModelAnimationSet animations;

public:
	weapon_info();

    inline bool is_homing()        { return wi_flags[Weapon::Info_Flags::Homing_heat, Weapon::Info_Flags::Homing_aspect, Weapon::Info_Flags::Homing_javelin]; }
    inline bool is_locked_homing() { return wi_flags[Weapon::Info_Flags::Homing_aspect, Weapon::Info_Flags::Homing_javelin]; }
    inline bool hurts_big_ships()  { return wi_flags[Weapon::Info_Flags::Bomb, Weapon::Info_Flags::Beam, Weapon::Info_Flags::Huge, Weapon::Info_Flags::Big_only]; }
    inline bool is_interceptable() { return wi_flags[Weapon::Info_Flags::Fighter_Interceptable, Weapon::Info_Flags::Turret_Interceptable]; }

	const char* get_display_name() const;
	bool has_display_name() const;

	void reset();
};

extern special_flag_def_list_new<Weapon::Info_Flags, weapon_info*, flagset<Weapon::Info_Flags>&> Weapon_Info_Flags[];

extern const size_t num_weapon_info_flags;

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

extern SCP_vector<weapon_info> Weapon_info;

extern int Num_weapons;
extern int First_secondary_index;
extern int Default_cmeasure_index;

extern SCP_vector<int> Player_weapon_precedence;	// Vector of weapon types, precedence list for player weapon selection

#define WEAPON_INDEX(wp)			(int)(wp-Weapons)


int weapon_info_lookup(const char *name);
int weapon_info_get_index(weapon_info *wip);

inline int weapon_info_size()
{
	return static_cast<int>(Weapon_info.size());
}

void weapon_init();					// called at game startup
void weapon_close();				// called at game shutdown
void weapon_level_init();			// called before the start of each level
void weapon_render(object* obj, model_draw_list *scene);
void weapon_delete( object * obj );
void weapon_process_pre( object *obj, float frame_time);
void weapon_process_post( object *obj, float frame_time);

//Call before weapons_page_in to mark a weapon as used
void weapon_mark_as_used(int weapon_id);

// Helper functions for l_Weaponclass.isWeaponUsed
bool weapon_page_in(int weapon_type);
bool weapon_used(int weapon_type);

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

// gets the substitution pattern pointer for a given weapon
// src_turret may be null
size_t* get_pointer_to_weapon_fire_pattern_index(int weapon_type, int ship_idx, ship_subsys* src_turret);

// for weapons flagged as particle spewers, spew particles. wheee
void weapon_maybe_spew_particle(object *obj);

bool weapon_armed(weapon *wp, bool hit_target);
void weapon_hit( object * weapon_obj, object * other_obj, vec3d * hitpos, int quadrant = -1, vec3d* hitnormal = NULL );
void spawn_child_weapons( object *objp, int spawn_index_override = -1);

// call to detonate a weapon. essentially calls weapon_hit() with other_obj as NULL, and sends a packet in multiplayer
void weapon_detonate(object *objp);

void	weapon_area_apply_blast(vec3d *force_apply_pos, object *ship_objp, vec3d *blast_pos, float blast, int make_shockwave);
int	weapon_area_calc_damage(object *objp, vec3d *pos, float inner_rad, float outer_rad, float max_blast, float max_damage,
										float *blast, float *damage, float limit);

missile_obj *missile_obj_return_address(int index);
void find_homing_object_cmeasures(const SCP_vector<object*> &cmeasure_list);

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

void weapon_hit_do_sound(object *hit_obj, weapon_info *wip, vec3d *hitpos, bool is_armed, int quadrant);

void weapon_do_electronics_effect(object *ship_objp, vec3d *blast_pos, int wi_index);

int weapon_get_random_player_usable_weapon();

// return a scale factor for damage which should be applied for 2 collisions
float weapon_get_damage_scale(weapon_info *wip, object *wep, object *target);

// Pauses all running weapon sounds
void weapon_pause_sounds();

// Unpauses all running weapon sounds
void weapon_unpause_sounds();

// Called by hudartillery.cpp after SSMs have been parsed to make sure that $SSM: entries defined in weapons are valid.
void validate_SSM_entries();

void shield_impact_explosion(vec3d *hitpos, object *objp, float radius, int idx);

// Swifty - return number of max simultaneous locks 
int weapon_get_max_missile_seekers(weapon_info *wip);

// return if this weapon can lock on this target, based on its type, class, species or iff
bool weapon_target_satisfies_lock_restrictions(weapon_info *wip, object* target);

// return if this weapon has iff restrictions, and should ignore normal iff targeting restrictions
bool weapon_has_iff_restrictions(weapon_info* wip);

// whether secondary weapon wip on shooter is in range of target_world_pos
bool weapon_secondary_world_pos_in_range(object* shooter, weapon_info* wip, vec3d* target_world_pos);

// Return whether shooter is in range, fov, etc of target_subsys, for multilock
// also returns the dot to the subsys in out_dot
// While single target missiles will check these properties as well separately, this function is ONLY used by multilock
bool weapon_multilock_can_lock_on_subsys(object* shooter, object* target, ship_subsys* target_subsys, weapon_info* wip, float* out_dot = nullptr);

// Return whether shooter is in fov, etc of a target, for multilock
// does NOT check range
// also returns the dot to the subsys in out_dot
// While single target missiles will check these properties as well separately, this function is ONLY used by multilock
bool weapon_multilock_can_lock_on_target(object* shooter, object* target_objp, weapon_info* wip, float* out_dot = nullptr, bool checkWeapons = false);

// Return whether the weapon has a target it is currently homing on
bool weapon_has_homing_object(weapon* wp);


#endif
