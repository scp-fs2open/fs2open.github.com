/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _SHIP_H
#define _SHIP_H



#include "globalincs/globals.h"		// for defintions of token lengths -- maybe move this elsewhere later (Goober5000 - moved to globals.h)
#include "graphics/2d.h"			// for color def
#include "model/model.h"
#include "model/modelanim.h"
#include "palman/palman.h"
#include "weapon/trails.h"
#include "ai/ai.h"
#include "network/multi_obj.h"
#include "hud/hudparse.h"
#include "render/3d.h"
#include "radar/radarsetup.h"
#include "weapon/shockwave.h"
#include "species_defs/species_defs.h"
#include "globalincs/pstypes.h"
#include "fireball/fireballs.h"
#include "hud/hud.h"
#include "ship/ship_flags.h"
#include "weapon/weapon_flags.h"

#include <string>

class object;
class WarpEffect;

//	Part of the player died system.
extern vec3d	Original_vec_to_deader;

//	States for player death sequence, stuffed in Player_died_state.
#define	PDS_NONE		1
#define	PDS_DIED		2
#define	PDS_EJECTED	3

#define SHIP_GUARDIAN_THRESHOLD_DEFAULT	1	// Goober5000

#define	HULL_DAMAGE_THRESHOLD_PERCENT	0.25f	//	Apply damage to hull, not shield if shield < this

// the #defines below are to avoid round-off errors
#define WEAPON_RESERVE_THRESHOLD		0.01f	// energy threshold where ship is considered to have no weapon energy system
#define SUBSYS_MAX_HITS_THRESHOLD		0.01f	// max_hits threshold where subsys is considered to take damage

#define	HP_SCALE						1.2			//	1.2 means die when 20% of hits remaining
#define	MAX_SHIP_HITS				8				// hits to kill a ship
#define	MAX_SHIP_DETAIL_LEVELS	5				// maximum detail levels that a ship can render at
#define	MAX_REINFORCEMENTS		32


// defines for 'direction' parameter of ship_select_next_primary()
#define CYCLE_PRIMARY_NEXT		0
#define CYCLE_PRIMARY_PREV		1

#define BANK_1		0
#define BANK_2		1
#define BANK_3		2
#define BANK_4		3
#define BANK_5		4
#define BANK_6		5
#define BANK_7		6
#define BANK_8		7
#define BANK_9		8

#define TYPE_ATTACK_PROTECT	0
#define TYPE_REPAIR_REARM		1

#define MAX_REINFORCEMENT_MESSAGES	5

#define RF_IS_AVAILABLE			(1<<0)			// reinforcement is now available

typedef struct {
	char	name[NAME_LENGTH];	// ship or wing name (ship and wing names don't collide)
	int	type;						// what operations this reinforcement unit can perform
	int	uses;						// number of times reinforcemnt unit can be used
	int	num_uses;				// number of times this reinforcement was actually used
	int	arrival_delay;			// how long after called does this reinforcement appear
	int	flags;
	char	no_messages[MAX_REINFORCEMENT_MESSAGES][NAME_LENGTH];		// list of messages to possibly send when calling for reinforcement not available
	char	yes_messages[MAX_REINFORCEMENT_MESSAGES][NAME_LENGTH];	// list of messages to acknowledge reinforcement on the way
} reinforcements;

typedef struct ship_weapon {
	int num_primary_banks;					// Number of primary banks (same as model)
	int num_secondary_banks;				// Number of secondary banks (same as model)
	int num_tertiary_banks;

	int primary_bank_weapons[MAX_SHIP_PRIMARY_BANKS];			// Weapon_info[] index for the weapon in the bank
	int secondary_bank_weapons[MAX_SHIP_SECONDARY_BANKS];	// Weapon_info[] index for the weapon in the bank

	int current_primary_bank;			// currently selected primary bank
	int current_secondary_bank;		// currently selected secondary bank
	int current_tertiary_bank;

	int previous_primary_bank;
	int previous_secondary_bank;		// currently selected secondary bank

	int next_primary_fire_stamp[MAX_SHIP_PRIMARY_BANKS];			// next time this primary bank can fire
	int last_primary_fire_stamp[MAX_SHIP_PRIMARY_BANKS];			// last time this primary bank fired (mostly used by SEXPs)
	int next_secondary_fire_stamp[MAX_SHIP_SECONDARY_BANKS];		// next time this secondary bank can fire
	int last_secondary_fire_stamp[MAX_SHIP_SECONDARY_BANKS];		// last time this secondary bank fired (mostly used by SEXPs)
	int next_tertiary_fire_stamp;
	int last_primary_fire_sound_stamp[MAX_SHIP_PRIMARY_BANKS];		// trailing end of the last time this primary bank was fired, for purposes of timing the pre-launch sound

	// ballistic primary support - by Goober5000
	int primary_bank_ammo[MAX_SHIP_PRIMARY_BANKS];			// Number of missiles left in primary bank
	int primary_bank_start_ammo[MAX_SHIP_PRIMARY_BANKS];	// Number of missiles starting in primary bank
	int primary_bank_capacity[MAX_SHIP_PRIMARY_BANKS];		// Max number of projectiles in bank
	int primary_next_slot[MAX_SHIP_PRIMARY_BANKS];			// Next slot to fire in the bank
	int primary_bank_rearm_time[MAX_SHIP_PRIMARY_BANKS];	// timestamp which indicates when bank can get new projectile
	// end ballistic primary support

	float primary_bank_fof_cooldown[MAX_SHIP_PRIMARY_BANKS];      // SUSHI: Current FOF cooldown level for the primary weapon

	// dynamic weapon linking - by RSAXVC
	int primary_bank_slot_count[MAX_SHIP_PRIMARY_BANKS];	// Fire this many slots at a time
	// end dynamic weapon linking

	int secondary_bank_ammo[MAX_SHIP_SECONDARY_BANKS];			// Number of missiles left in secondary bank
	int secondary_bank_start_ammo[MAX_SHIP_SECONDARY_BANKS];	// Number of missiles starting in secondary bank
	int secondary_bank_capacity[MAX_SHIP_SECONDARY_BANKS];		// Max number of missiles in bank
	int secondary_next_slot[MAX_SHIP_SECONDARY_BANKS];			// Next slot to fire in the bank
	int secondary_bank_rearm_time[MAX_SHIP_SECONDARY_BANKS];	// timestamp which indicates when bank can get new missile

	int tertiary_bank_ammo;			// Number of shots left tertiary bank
	int tertiary_bank_start_ammo;	// Number of shots starting in tertiary bank
	int tertiary_bank_capacity;		// Max number of shots in bank
	int tertiary_bank_rearm_time;	// timestamp which indicates when bank can get new something (used for ammopod or boostpod)

	int last_fired_weapon_index;		//	Index of last fired secondary weapon.  Used for remote detonates.
	int last_fired_weapon_signature;	//	Signature of last fired weapon.
	int detonate_weapon_time;			//	time at which last fired weapon can be detonated
	int ai_class;

	flagset<Ship::Weapon_Flags> flags;								// see SW_FLAG_* defines above
	EModelAnimationPosition primary_animation_position[MAX_SHIP_PRIMARY_BANKS];
	EModelAnimationPosition secondary_animation_position[MAX_SHIP_SECONDARY_BANKS];
	int primary_animation_done_time[MAX_SHIP_PRIMARY_BANKS];
	int  secondary_animation_done_time[MAX_SHIP_SECONDARY_BANKS];

	int	burst_counter[MAX_SHIP_PRIMARY_BANKS + MAX_SHIP_SECONDARY_BANKS];
	int external_model_fp_counter[MAX_SHIP_PRIMARY_BANKS + MAX_SHIP_SECONDARY_BANKS];

	size_t primary_bank_pattern_index[MAX_SHIP_PRIMARY_BANKS];
	size_t secondary_bank_pattern_index[MAX_SHIP_SECONDARY_BANKS];
} ship_weapon;

//**************************************************************
//WMC - Damage type handling code

int damage_type_add(char *name);

//**************************************************************
//WMC - Armor stuff

// Nuke: some defines for difficulty scaling type
#define ADT_DIFF_SCALE_BAD_VAL	-1 // error mode 
#define ADT_DIFF_SCALE_FIRST	0
#define ADT_DIFF_SCALE_LAST		1
#define ADT_DIFF_SCALE_MANUAL	2 // this is the user defined mode where the modder has to handle difficulty scaling in their calculations

// Nuke: +value: replacing constants
// these are stored as altArguments, positive values mean storage idxes and -1 means not used, anything below that is fair game
#define AT_CONSTANT_NOT_USED	-1	// will probibly never get used
#define AT_CONSTANT_BAD_VAL		-2	// this conveys table error to the user 
#define AT_CONSTANT_BASE_DMG	-3	// what the damage was at start of calculations
#define AT_CONSTANT_CURRENT_DMG	-4	// what the damage currently is
#define AT_CONSTANT_DIFF_FACTOR	-5	// difficulty factor (by default 0.2 (easy) to 1.0 (insane))
#define AT_CONSTANT_RANDOM		-6	// number between 0 and 1 (redundant but saves a calculation)
#define AT_CONSTANT_PI			-7	// because everyone likes pi

struct ArmorDamageType
{
	friend class ArmorType;
private:
	//Rather than make an extra struct,
	//I just made two arrays
	int					DamageTypeIndex;
	SCP_vector<int>	Calculations;
	SCP_vector<float>	Arguments;
	SCP_vector<int>		altArguments;		// Nuke: to facilitate optional importation of data in place of +value: tag -nuke 
	float				shieldpierce_pct;

	// piercing effect data
	float				piercing_start_pct;
	int					piercing_type;
	// Nuke: difficulty scale type
	int					difficulty_scale_type;

public:
	void clear();
};

class ArmorType
{
private:
	char Name[NAME_LENGTH];

	SCP_vector<ArmorDamageType> DamageTypes;
public:
	ArmorType(char* in_name);
	int flags;

	//Get
	char *GetNamePtr(){return Name;}
	bool IsName(char *in_name){return (stricmp(in_name,Name)==0);}
	float GetDamage(float damage_applied, int in_damage_type_idx, float diff_dmg_scale);
	float GetShieldPiercePCT(int damage_type_idx);
	int GetPiercingType(int damage_type_idx);
	float GetPiercingLimit(int damage_type_idx);
	
	//Set
	void ParseData();
};

extern SCP_vector<ArmorType> Armor_types;

//**************************************************************
//WMC - Damage type handling code

typedef struct DamageTypeStruct
{
	char name[NAME_LENGTH];
} DamageTypeStruct;

extern SCP_vector<DamageTypeStruct>	Damage_types;

#define SAF_IGNORE_SS_ARMOR			(1 << 0)		// hull armor is applied regardless of the subsystem armor for hull damage

#define SADTF_PIERCING_NONE			0				// no piercing effects, no beam tooling
#define SADTF_PIERCING_DEFAULT		1				// piercing effects, beam tooling
#define SADTF_PIERCING_RETAIL		2				// no piercing effects, beam tooling

//SUSHI: Damage lightning types. SLT = Ship Lighting Type.
#define SLT_NONE	0
#define SLT_DEFAULT	1

#define NUM_TURRET_ORDER_TYPES		3
extern char *Turret_target_order_names[NUM_TURRET_ORDER_TYPES];	//aiturret.cpp

// Swifty: Cockpit displays
typedef struct cockpit_display {
	int target;
	int source;
	int foreground;
	int background;
	int offset[2];
	int size[2];
	char name[MAX_FILENAME_LEN];
} cockpit_display;

extern SCP_vector<cockpit_display> Player_displays;

typedef struct cockpit_display_info {
	char name[MAX_FILENAME_LEN];
	char filename[MAX_FILENAME_LEN];
	char fg_filename[MAX_FILENAME_LEN];
	char bg_filename[MAX_FILENAME_LEN];
	int offset[2];
	int size[2];
} cockpit_display_info;

// structure definition for a linked list of subsystems for a ship.  Each subsystem has a pointer
// to the static data for the subsystem.  The obj_subsystem data is defined and read in the model
// code.  Other dynamic data (such as current_hits) should remain in this structure.
class ship_subsys
{
public:
	class ship_subsys *next, *prev;				//	Index of next and previous objects in list.
	model_subsystem *system_info;					// pointer to static data for this subsystem -- see model.h for definition

	int			parent_objnum;						// objnum of the parent ship

	char		sub_name[NAME_LENGTH];					//WMC - Name that overrides name of original
	float		current_hits;							// current number of hits this subsystem has left.
	float		max_hits;

	flagset<Ship::Subsystem_Flags> flags;						// Goober5000

	int subsys_guardian_threshold;	// Goober5000
	int armor_type_idx;				// FUBAR

	// turret info
	//Important -WMC
	//With the new turret code, indexes run from 0 to MAX_SHIP_WEAPONS; a value of MAX_SHIP_PRIMARY_WEAPONS
	//or higher, an index into the turret weapons is considered to be an index into the secondary weapons
	//for much of the code. See turret_next_weap_fire_stamp.

	int		turret_best_weapon;				// best weapon for current target; index into prim/secondary banks
	vec3d	turret_last_fire_direction;		//	direction pointing last time this turret fired
	int		turret_next_enemy_check_stamp;	//	time at which to next look for a new enemy.
	int		turret_next_fire_stamp;				// next time this turret can fire
	int		turret_enemy_objnum;					//	object index of ship this turret is firing upon
	int		turret_enemy_sig;						//	signature of object ship this turret is firing upon
	int		turret_next_fire_pos;				// counter which tells us which gun position to fire from next
	float	turret_time_enemy_in_range;		//	Number of seconds enemy in view cone, accuracy improves over time.
	int		turret_targeting_order[NUM_TURRET_ORDER_TYPES];	//Order that turrets target different types of things.
	float	optimum_range;					        
	float	favor_current_facing;					        
	ship_subsys	*targeted_subsys;					//	subsystem this turret is attacking
	bool	scripting_target_override;
	int		last_fired_weapon_info_index;		// which weapon class was last fired

	int		turret_pick_big_attack_point_timestamp;	//	Next time to pick an attack point for this turret
	vec3d	turret_big_attack_point;			//	local coordinate of point for this turret to attack on enemy

	EModelAnimationPosition	turret_animation_position;
	int		turret_animation_done_time;

	// swarm (rapid fire) info
	int		turret_swarm_info_index[MAX_TFP];	
	int		turret_swarm_num;	

	// awacs info
	float		awacs_intensity;
	float		awacs_radius;

	ship_weapon	weapons;

	// Data the renderer needs for ship instance specific data, like
	// angles and if it is blown off or not.
	// There are 2 of these because turrets need one for the turret and one for the barrel.
	// Things like radar dishes would only use one.
	submodel_instance_info	submodel_info_1;		// Instance data for main turret or main object
	submodel_instance_info	submodel_info_2;		// Instance data for turret guns, if there is one

	int disruption_timestamp;							// time at which subsystem isn't disrupted

	int subsys_cargo_name;			// cap ship cargo on subsys
	fix time_subsys_cargo_revealed;	// added by Goober5000

	int triggered_rotation_index;		//the actual currently running animation and assosiated states

	float points_to_target;
	float base_rotation_rate_pct;
	float gun_rotation_rate_pct;

	// still going through these...
	flagset<Ship::Subsys_Sound_Flags> subsys_snd_flags;

	int      rotation_timestamp;
	matrix   world_to_turret_matrix;

	// target priority setting for turrets
	int      target_priority[32];
	int      num_target_priorities;

	//SUSHI: Fields for max_turret_aim_update_delay
	//Only used when targeting small ships
	fix		next_aim_pos_time;
	vec3d	last_aim_enemy_pos;
	vec3d	last_aim_enemy_vel;

	//scaler for setting adjusted turret rof
	float	rof_scaler;
	float	turn_rate;

	//Per-turret ownage settings - SUSHI
	int turret_max_bomb_ownage; 
	int turret_max_target_ownage;

	ship_subsys()
		: next(NULL), prev(NULL)
	{}

	void clear();
};

// structure for subsystems which tells us the total count of a particular type of subsystem (i.e.
// we might have 3 engines), and the relative strength of the subsystem.  The #defines in model.h
// for SUBSYSTEM_xxx will be used as indices into this array.
typedef struct ship_subsys_info {
	int	type_count;					// number of subsystems of type on this ship;
	float aggregate_max_hits;		// maximum number of hits for all subsystems of this type.
	float aggregate_current_hits;	// current count of hits for all subsystems of this type.	
} ship_subsys_info;

// Karajorma - Used by the alter-ship-flag SEXP as an alternative to having lots of ship flag SEXPs
typedef struct ship_flag_name {
	Ship::Ship_Flags flag;							// the actual ship flag constant as given by the define below
	char flag_name[TOKEN_LENGTH];		// the name written to the mission file for its corresponding parse_object flag
} ship_flag_name;

#define MAX_SHIP_FLAG_NAMES					17
extern ship_flag_name Ship_flag_names[];

#define DEFAULT_SHIP_PRIMITIVE_SENSOR_RANGE		10000	// Goober5000

#define MAX_DAMAGE_SLOTS	32
#define MAX_SHIP_ARCS		2		// How many "arcs" can be active at once... Must be less than MAX_ARC_EFFECTS in model.h. 
#define NUM_SUB_EXPL_HANDLES	2	// How many different big ship sub explosion sounds can be played.

#define MAX_SHIP_CONTRAILS		24
#define MAX_MAN_THRUSTERS	128

typedef struct ship_spark {
	vec3d pos;			// position of spark in the submodel's RF
	int submodel_num;	// which submodel is making the spark
	int end_time;
} ship_spark;

// NOTE: Can't be treated as a struct anymore, since it has STL data structures in its object tree!
class ship
{
public:
	int	objnum;
	int	ai_index;			// Index in Ai_info of ai_info associated with this ship.
	int	ship_info_index;	// Index in ship_info for this ship
	int	hotkey;
	int	escort_priority;
	int	score;
	float assist_score_pct;
	int	respawn_priority;
	
	// BEGIN PACK ubytes and chars
	ubyte	pre_death_explosion_happened;		// If set, it means the 4 or 5 smaller explosions 
	ubyte wash_killed;
	char	cargo1;

	// ship wing status info
	char	wing_status_wing_index;			// wing index (0-4) in wingman status gauge
	char	wing_status_wing_pos;			// wing position (0-5) in wingman status gauge

	// alternate indexes
	int	 alt_type_index;								// only used for display purposes (read : safe)
	int	 callsign_index;								// ditto

	// targeting laser info
	char targeting_laser_bank;						// -1 if not firing, index into polymodel gun points if it _is_ firing
	int targeting_laser_objnum;					// -1 if invalid, beam object # otherwise

	// corkscrew missile stuff
	ubyte num_corkscrew_to_fire;						// # of corkscrew missiles lef to fire
	int corkscrew_missile_bank;
	int next_corkscrew_fire;						// next time to fire a corkscrew missile
	// END PACK

	int	final_death_time;				// Time until big fireball starts
	int	death_time;				// Time until big fireball starts
	int	end_death_time;				// Time until big fireball starts
	int	really_final_death_time;	// Time until ship breaks up and disappears
	vec3d	deathroll_rotvel;			// Desired death rotational velocity

	WarpEffect *warpin_effect;
	WarpEffect *warpout_effect;

	int	next_fireball;

	int	next_hit_spark;
	int	num_hits;			//	Note, this is the number of spark emitter positions!
	ship_spark	sparks[MAX_SHIP_HITS];
	
	bool use_special_explosion; 
	int special_exp_damage;					// new special explosion/hitpoints system
	int special_exp_blast;
	int special_exp_inner;
	int special_exp_outer;
	bool use_shockwave;
	int special_exp_shockwave_speed;
	int special_exp_deathroll_time;

	int	special_hitpoints;
	int	special_shield;

	float ship_max_shield_strength;
	float ship_max_hull_strength;

	int ship_guardian_threshold;	// Goober5000 - now also determines whether ship is guardian'd


	char	ship_name[NAME_LENGTH];

	int	team;				//	Which team it's on, HOSTILE, FRIENDLY, UNKNOWN, NEUTRAL
	
	fix	time_cargo_revealed;					// time at which the cargo was revealed

	int	arrival_location;
	int	arrival_distance;		// how far away this ship should arrive
	int	arrival_anchor;			// name of object this ship arrives near (or in front of)
	int	arrival_path_mask;		// Goober5000 - possible restrictions on which bay paths to use
	int	arrival_cue;
	int	arrival_delay;

	int	departure_location;		// depart to hyperspace or someplace else (like docking bay)
	int	departure_anchor;		// when docking bay -- index of ship to use
	int departure_path_mask;	// Goober5000 - possible restrictions on which bay paths to use
	int	departure_cue;			// sexpression to eval when departing
	int	departure_delay;		// time in seconds after sexp is true that we delay.

	int	wingnum;								// wing number this ship is in.  -1 if in no wing, Wing array index otherwise
	int	orders_accepted;					// set of orders this ship will accept from the player.

	// Subsystem fields.  The subsys_list is a list of all subsystems (which might include multiple types
	// of a particular subsystem, like engines).  The subsys_info struct is information for particular
	// types of subsystems.  (i.e. the list might contain 3 engines.  There will be one subsys_info entry
	// describing the state of all engines combined) -- MWA 4/1/97
	ship_subsys	subsys_list;									//	linked list of subsystems for this ship.
	ship_subsys	*last_targeted_subobject[MAX_PLAYERS];	// Last subobject that has been targeted.  NULL if none;(player specific)
	ship_subsys_info	subsys_info[SUBSYSTEM_MAX];		// info on particular generic types of subsystems	

	float	*shield_integrity;					//	Integrity at each triangle in shield mesh.

	// ETS fields
	int	shield_recharge_index;			// index into array holding the shield recharge rate
	int	weapon_recharge_index;			// index into array holding the weapon recharge rate
	int	engine_recharge_index;			// index into array holding the engine recharge rate
	float	weapon_energy;						// Number of EUs in energy reserves
	float	current_max_speed;				// Max ship speed (based on energy diverted to engines)
	int	next_manage_ets;					// timestamp for when ai can next modify ets ( -1 means never )

	flagset<Ship::Ship_Flags> flags;

	int	reinforcement_index;				// index into reinforcement struct or -1
	
	float	afterburner_fuel;					// amount of afterburner fuel remaining (capacity is stored
													// as afterburner_fuel_capacity in ship_info).

	int cmeasure_count;						//	Number of charges of countermeasures this ship can hold.
	int current_cmeasure;					//	Currently selected countermeasure.

	int cmeasure_fire_stamp;				//	Time at which can fire countermeasure.

	float	target_shields_delta;			//	Target for shield recharge system.
	float	target_weapon_energy_delta;	//	Target for recharge system.
	ship_weapon	weapons;

	int	shield_hits;						//	Number of hits on shield this frame.

	SCP_vector<vec3d>	shield_points;

	float		wash_intensity;
	vec3d	wash_rot_axis;
	int		wash_timestamp;

	int	num_swarm_missiles_to_fire;	// number of swarm missiles that need to be launched
	int	next_swarm_fire;					// timestamp of next swarm missile to fire
	int	next_swarm_path;					// next path number for swarm missile to take
	int	num_turret_swarm_info;			// number of turrets in process of launching swarm
	int swarm_missile_bank;				// The missilebank the swarm was originally launched from

	int	group;								// group ship is in, or -1 if none.  Fred thing
	int	death_roll_snd;					// id of death roll sound, may need to be stopped early	
	int	ship_list_index;					// index of ship in Ship_objs[] array

	int	thruster_bitmap;					// What frame the current thruster bitmap is at for this ship
	float	thruster_frame;					// Used to keep track of which frame the animation should be on.

	int	thruster_glow_bitmap;			// What frame the current thruster engine glow bitmap is at for this ship
	float	thruster_glow_frame;				// Used to keep track of which frame the engine glow animation should be on.
	float	thruster_glow_noise;				// Noise for current frame

	int	thruster_secondary_glow_bitmap;		// Bobboau
	int	thruster_tertiary_glow_bitmap;		// Bobboau
	int	thruster_distortion_bitmap;			// Valathil

	int	next_engine_stutter;				// timestamp to time the engine stuttering when a ship dies

	fix base_texture_anim_frametime;		// Goober5000 - zero mark for texture animations

	float total_damage_received;        // total damage received (for scoring purposes)
	float damage_ship[MAX_DAMAGE_SLOTS];    // damage applied from each player
	int   damage_ship_id[MAX_DAMAGE_SLOTS]; // signature of the damager (corresponds to each entry in damage_ship)
	int	persona_index;						// which persona is this guy.

	int	subsys_disrupted_flags;					// bitflags used to check if SUBYSTEM_* is disrupted or not
	int	subsys_disrupted_check_timestamp;	// timer to control how oftern flags are set/cleared in subsys_disrupted_flags

	uint	create_time;						// time ship was created, set by gettime()

	// keep multiplayer specific stuff below this point	
	int	ts_index;							// index into the team select and Wss_slots array (or -1 if not in one of those arrays)

	int	large_ship_blowup_index;			// -1 if not a large ship exploding, else this is an index used by the shipfx large ship exploding code.
	int	sub_expl_sound_handle[NUM_SUB_EXPL_HANDLES];


	// Stuff for showing electrical arcs on damaged ships
	vec3d	arc_pts[MAX_SHIP_ARCS][2];			// The endpoints of each arc
	int		arc_timestamp[MAX_SHIP_ARCS];		// When this times out, the spark goes away.  -1 is not used
	ubyte		arc_type[MAX_SHIP_ARCS];			// see MARC_TYPE_* defines in model.h
	int		arc_next_time;							// When the next arc will be created.	

	// emp missile stuff
	float emp_intensity;								// <= 0.0f if no emp effect present
	float emp_decr;									// how much to decrement EMP effect per second for this ship

	// contrail stuff
	trail *trail_ptr[MAX_SHIP_CONTRAILS];	

	// tag stuff
	float tag_total;									// total tag time
	float tag_left;									// total tag remaining	
	fix	time_first_tagged;
	float level2_tag_total;							// total tag time
	float level2_tag_left;							// total tag remaining	

	// old-style object update stuff
	np_update		np_updates[MAX_PLAYERS];	// for both server and client

	// lightning timestamp
	int lightning_stamp;

	// AWACS warning flag
	flagset<Ship::Awacs_Warning_Flags>	awacs_warning_flag;

	// Special warp objnum (warping at knossos)
	int special_warpin_objnum;
	int special_warpout_objnum;

	ship_subsys fighter_beam_turret_data;		//a fake subsystem that pretends to be a turret for fighter beams
	model_subsystem beam_sys_info;
	int was_firing_last_frame[MAX_SHIP_PRIMARY_BANKS];

	// Goober5000 - range of primitive sensors
	int primitive_sensor_range;
	
	// Goober5000 - revised nameplate implementation
	int *ship_replacement_textures;

	// Goober5000 - index into pm->view_positions[]
	// apparently, early in FS1 development, there was a field called current_eye_index
	// that had this same functionality
	int current_viewpoint;

	trail *ABtrail_ptr[MAX_SHIP_CONTRAILS];		//after burner trails -Bobboau
	trail_info ab_info[MAX_SHIP_CONTRAILS];
	int ab_count;

	// glow points
	std::deque<bool> glow_point_bank_active;

	//Animated Shader effects
	int shader_effect_num;
	int shader_effect_duration;
	int shader_effect_start_time;
	bool shader_effect_active;

	int last_fired_point[MAX_SHIP_PRIMARY_BANKS]; //for fire point cylceing
	ship_subsys *last_fired_turret; // which turret has fired last

	// fighter bay door stuff, parent side
	int bay_doors_anim_done_time;		// ammount of time to transition from one animation state to another
	EModelAnimationPosition bay_doors_status;			// anim status of the bay doors (closed/not-animating, opening, open/not-animating)
	int bay_doors_wanting_open;		// how many ships want/need the bay door open

	// figther bay door stuff, client side
	ubyte bay_doors_launched_from;	// the bay door that I launched from
	bool bay_doors_need_open;		// keep track of whether I need the door open or not
	int bay_doors_parent_shipnum;	// our parent ship, what we are entering/leaving
	
	float secondary_point_reload_pct[MAX_SHIP_SECONDARY_BANKS][MAX_SLOTS];	//after fireing a secondary it takes some time for that secondary weapon to reload, this is how far along in that proces it is (from 0 to 1)
	float primary_rotate_rate[MAX_SHIP_PRIMARY_BANKS];
	float primary_rotate_ang[MAX_SHIP_PRIMARY_BANKS];

	int thrusters_start[MAX_MAN_THRUSTERS];		//Timestamp of when thrusters started
	int thrusters_sounds[MAX_MAN_THRUSTERS];	//Sound index for thrusters

	SCP_vector<alt_class> s_alt_classes;	

	int ship_iff_color[MAX_IFFS][MAX_IFFS];

	int ammo_low_complaint_count;				// number of times this ship has complained about low ammo
	int armor_type_idx;
	int shield_armor_type_idx;
	int collision_damage_type_idx;
	int debris_damage_type_idx;
	ushort debris_net_sig;						// net signiture of the first piece of debris this ship has

	int model_instance_num;

	fix time_created;

	fix radar_visible_since; // The first time this ship was visible on the radar. Gets reset when ship is not visible anymore
	fix radar_last_contact; // The last time this ship appeared on the radar. When it is currently visible this has the value if Missiontime

	RadarVisibility radar_last_status; // Last radar status
	RadarVisibility radar_current_status; // Current radar status

	SCP_string team_name;
	SCP_string secondary_team_name;	//If the change-team-color sexp is used, these fields control the fading behaviour
	fix team_change_timestamp;
	int team_change_time;

	float autoaim_fov;

	// reset to a completely blank ship
	void clear();
};

struct ai_target_priority {
	char name[NAME_LENGTH];

	int obj_type;
	SCP_vector <int> ship_type;
	SCP_vector <int> ship_class;
	SCP_vector <int> weapon_class;

	flagset<Object::Object_Flags> obj_flags;
	flagset<Ship::Info_Flags>* sif_flags;
	flagset<Weapon::Info_Flags>* wif_flags;
};

extern SCP_vector <ai_target_priority> Ai_tp_list;

void parse_ai_target_priorities();
void parse_weapon_targeting_priorities();
ai_target_priority init_ai_target_priorities();

typedef struct exited_ship {
	char	ship_name[NAME_LENGTH];
	int		obj_signature;
	int		ship_class;
	int		team;
	flagset<Ship::Exit_Flags> flags;
	fix		time;
	int		hull_strength;
	fix		time_cargo_revealed;
	char	cargo1;
	float damage_ship[MAX_DAMAGE_SLOTS];		// A copy of the arrays from the ship so that we can figure out what damaged it
	int   damage_ship_id[MAX_DAMAGE_SLOTS];
} exited_ship;

extern SCP_vector<exited_ship> Ships_exited;

// a couple of functions to get at the data
extern void ship_add_exited_ship( ship *shipp, Ship::Exit_Flags reason );
extern int ship_find_exited_ship_by_name( char *name );
extern int ship_find_exited_ship_by_signature( int signature);

#define REGULAR_WEAPON	(1<<0)
#define DOGFIGHT_WEAPON (1<<1)

typedef struct thruster_particles {
	generic_anim thruster_bitmap;
	float		min_rad;
	float		max_rad;
	int			n_high;
	int			n_low;
	float		variance;
} thruster_particles;

typedef struct particle_effect {
	int				n_low;
	int				n_high;
	float			min_rad;
	float			max_rad;
	float			min_life;
	float			max_life;
	float			min_vel;
	float			max_vel;
	float			variance;
} particle_effect;

typedef struct ship_type_info {
	char name[NAME_LENGTH];

	//Messaging?
	flagset<Ship::Type_Info_Msg> message_bools;

	//HUD
	flagset<Ship::Type_Info_Hud> hud_bools;

	//Ship
	flagset<Ship::Type_Info_Ship> ship_bools;	//For lack of better term
	float debris_max_speed;

	//Weapons
	flagset<Ship::Type_Info_Weapon> weapon_bools;
	float ff_multiplier;
	float emp_multiplier;

	//Fog
	float fog_start_dist;
	float fog_complete_dist;

	//AI
	int	ai_valid_goals;
	int ai_player_orders;
	flagset<Ship::Type_Info_AI> ai_bools;
	int ai_active_dock;
	int ai_passive_dock;
	SCP_vector<int> ai_actively_pursues;
	SCP_vector<int> ai_cripple_ignores;

	//Explosions
	float vaporize_chance;

	//Resources
	SCP_vector<int> explosion_bitmap_anims;

	//Regen values - need to be converted after all types have loaded
	SCP_vector<SCP_string> ai_actively_pursues_temp;
	SCP_vector<SCP_string> ai_cripple_ignores_temp;

	ship_type_info( )
		: debris_max_speed( 0.f ),
		  ff_multiplier( 0.f ), emp_multiplier( 0.f ),
		  fog_start_dist( 0.f ), fog_complete_dist( 0.f ),
		  ai_valid_goals( 0 ), ai_player_orders( 0 ), ai_active_dock( 0 ), ai_passive_dock( 0 ),
		  vaporize_chance( 0.f )

	{
		message_bools.reset();
		hud_bools.reset();
		ship_bools.reset();
		weapon_bools.reset();
		ai_bools.reset();

		name[ 0 ] = '\0';
	}
} ship_type_info;

extern SCP_vector<ship_type_info> Ship_types;

struct man_thruster_renderer {
	int bmap_id;
	geometry_batcher man_batcher;

	man_thruster_renderer(int id){bmap_id = id;}
};

extern SCP_vector<man_thruster_renderer> Man_thrusters;

typedef struct man_thruster {
	flagset<Ship::Thruster_Flags> use_flags;

	int start_snd;
	int loop_snd;
	int stop_snd;

	int tex_id;
	int tex_nframes;
	int tex_fps;
	float length;
	float radius;

	vec3d pos, norm;
} man_thruster;

//Warp type defines
#define WT_DEFAULT					0
#define WT_KNOSSOS					1
#define WT_DEFAULT_THEN_KNOSSOS		2
#define WT_IN_PLACE_ANIM			3
#define WT_SWEEPER					4
#define WT_HYPERSPACE				5

// Holds variables for collision physics (Gets its own struct purely for clarity purposes)
// Most of this only really applies properly to small ships
typedef struct ship_collision_physics {
	// Collision physics definitions: how a ship responds to collisions
	float both_small_bounce;	// Bounce factor when both ships are small
								// This currently only comes into play if one ship is the player... 
								// blame retail for that.
	float bounce;				// Bounce factor for all other cases
	float friction;				// Controls lateral velocity lost when colliding with a large ship
	float rotation_factor;		// Affects the rotational energy of collisions... TBH not sure how. 

	// Speed & angle constraints for a smooth landing
	// Note that all angles are stored as a dotproduct between normalized vectors instead. This saves us from having
	// to do a lot of dot product calculations later.
	float landing_max_z;		
	float landing_min_z;
	float landing_min_y;
	float landing_max_x;
	float landing_max_angle;
	float landing_min_angle;
	float landing_max_rot_angle;

	// Speed & angle constraints for a "rough" landing (one with normal collision consequences, but where 
	// the ship is still reoriented towards its resting orientation)
	float reorient_max_z;
	float reorient_min_z;
	float reorient_min_y;
	float reorient_max_x;
	float reorient_max_angle;
	float reorient_min_angle;
	float reorient_max_rot_angle;

	// Landing response parameters
	float reorient_mult;		// How quickly the ship will reorient towards it's resting position
	float landing_rest_angle;	// The vertical angle where the ship's orientation comes to rest
	int landing_sound_idx;		//Sound to play on successful landing collisions

} ship_collision_physics;

typedef struct path_metadata {
	vec3d departure_rvec;
} path_metadata;

// The real FreeSpace ship_info struct.
// NOTE: Can't be treated as a struct anymore, since it has STL data structures in its object tree!
class ship_info
{
public:
	char		name[NAME_LENGTH];				// name for the ship
	char		alt_name[NAME_LENGTH];			// display another name for the ship
	char		short_name[NAME_LENGTH];		// short name, for use in the editor?
	int			species;								// which species this craft belongs to
	int			class_type;						//For type table

	char		*type_str;							// type string used by tooltips
	char		*maneuverability_str;			// string used by tooltips
	char		*armor_str;							// string used by tooltips
	char		*manufacturer_str;				// string used by tooltips
	char		*desc;								// string used by tooltips
	char		*tech_desc;							// string used by tech database
	char		tech_title[NAME_LENGTH];			// ship's name (in tech database)

	char     *ship_length;						// string used by multiplayer ship desc
	char     *gun_mounts;			         // string used by multiplayer ship desc
	char     *missile_banks;					// string used by multiplayer ship desc

	char		cockpit_pof_file[MAX_FILENAME_LEN];	// POF file for cockpit view
	vec3d		cockpit_offset;
	char		pof_file[MAX_FILENAME_LEN];			// POF file to load/associate with ship
	char		pof_file_hud[MAX_FILENAME_LEN];		// POF file to load for the HUD target box
	int		num_detail_levels;				// number of detail levels for this ship
	int		detail_distance[MAX_SHIP_DETAIL_LEVELS];					// distance to change detail levels at
	int		cockpit_model_num;					// cockpit model
	int		model_num;							// ship model
	int		model_num_hud;						// model to use when rendering to the HUD (eg, mini supercap)
	int		hud_target_lod;						// LOD to use for rendering to the HUD targetbox (if not already using special HUD model)
	float		density;								// density of the ship in g/cm^3 (water  = 1)
	float		damp;									// drag
	float		rotdamp;								// rotational drag
	float		delta_bank_const;
	vec3d	max_vel;								//	max velocity of the ship in the linear directions -- read from ships.tbl
	vec3d	max_rotvel;							// maximum rotational velocity
	vec3d	rotation_time;						// time to rotate in x/y/z dimension traveling at max rotvel
	float		srotation_time;					//	scalar, computed at runtime as (rotation_time.x + rotation_time.y)/2
	float		max_rear_vel;						// max speed ship can go backwards.
	float		forward_accel;
	float		forward_decel;
	float		slide_accel;
	float		slide_decel;

	char		warpin_anim[MAX_FILENAME_LEN];
	float		warpin_radius;
	int			warpin_snd_start;
	int			warpin_snd_end;
	float		warpin_speed;
	int			warpin_time;	//in ms
	float		warpin_decel_exp;
	int			warpin_type;

	char		warpout_anim[MAX_FILENAME_LEN];
	float		warpout_radius;
	int			warpout_snd_start;
	int			warpout_snd_end;
	int			warpout_engage_time;	//in ms
	float		warpout_speed;
	int			warpout_time;	//in ms
	float		warpout_accel_exp;
	int			warpout_type;

	float		warpout_player_speed;

	flagset<Ship::Info_Flags> flags;
	int		ai_class;							//	Index into Ai_classes[].  Defined in ai.tbl
	float		max_speed, min_speed, max_accel;

	//Collision
	int						collision_damage_type_idx;
	ship_collision_physics	collision_physics;

	// ship explosion info
	shockwave_create_info shockwave;
	int	explosion_propagates;				// If true, then the explosion propagates
	float big_exp_visual_rad;				//SUSHI: The visual size of the main explosion
	float prop_exp_rad_mult;				// propagating explosions radius multiplier
	float death_roll_r_mult;
	float death_fx_r_mult;
	float death_roll_time_mult;
	int death_roll_base_time;
	int death_fx_count;
	int	shockwave_count;					// the # of total shockwaves
	SCP_vector<int> explosion_bitmap_anims;
	float vaporize_chance;					

	particle_effect		impact_spew;
	particle_effect		damage_spew;
	particle_effect		split_particles;
	particle_effect		knossos_end_particles;
	particle_effect		regular_end_particles;

	//Debris stuff
	float			debris_min_lifetime;
	float			debris_max_lifetime;
	float			debris_min_speed;
	float			debris_max_speed;
	float			debris_min_rotspeed;
	float			debris_max_rotspeed;
	int				debris_damage_type_idx;
	float			debris_min_hitpoints;
	float			debris_max_hitpoints;
	float			debris_damage_mult;
	float			debris_arc_percent;

	// subsystem information
	int		n_subsystems;						// this number comes from ships.tbl
	model_subsystem *subsystems;				// see model.h for structure definition

	// Energy Transfer System fields
	float		power_output;					// power output of ships reactor (EU/s)
	float		max_overclocked_speed;			// max speed when 100% power output sent to engines
	float		max_weapon_reserve;				// maximum energy that can be stored for primary weapon usage
	float		max_shield_regen_per_second;	// Goober5000 - max percent/100 of shield energy regenerated per second
	float		max_weapon_regen_per_second;	// Goober5000 - max percent/100 of weapon energy regenerated per second

	// Afterburner fields
	vec3d		afterburner_max_vel;				//	max velocity of the ship in the linear directions when afterburners are engaged -- read from ships.tbl
	float		afterburner_forward_accel;		// forward acceleration with afterburner engaged
	float		afterburner_fuel_capacity;		// maximum afterburner fuel that can be stored
	float		afterburner_burn_rate;			// rate in fuel/second that afterburner consumes fuel
	float		afterburner_recover_rate;		//	rate in fuel/second that afterburner recovers fuel
	//SparK: reverse afterburner
	float		afterburner_max_reverse_vel;
	float		afterburner_reverse_accel;

	int		cmeasure_type;						// Type of countermeasures this ship carries
	int		cmeasure_max;						//	Number of charges of countermeasures this ship can hold.

	int num_primary_banks;										// Actual number of primary banks (property of model)
	int primary_bank_weapons[MAX_SHIP_PRIMARY_BANKS];			// Weapon_info[] index for the weapon in the bank	
	// Goober5000's ballistic conversion
	int primary_bank_ammo_capacity[MAX_SHIP_PRIMARY_BANKS];	// Capacity of primary ballistic bank

	int num_secondary_banks;									//	Actual number of secondary banks (property of model)
	int secondary_bank_weapons[MAX_SHIP_SECONDARY_BANKS];	// Weapon_info[] index for the weapon in the bank
	int secondary_bank_ammo_capacity[MAX_SHIP_SECONDARY_BANKS];	// Capacity of bank (not number of missiles)

	bool draw_primary_models[MAX_SHIP_PRIMARY_BANKS];
	bool draw_secondary_models[MAX_SHIP_SECONDARY_BANKS];
	float weapon_model_draw_distance;

	float	max_hull_strength;				// Max hull strength of this class of ship.
	float	max_shield_strength;
	float	auto_shield_spread;
	bool	auto_shield_spread_bypass;
	int		auto_shield_spread_from_lod;

	int		shield_point_augment_ctrls[4];	// Re-mapping of shield augmentation controls for model point shields

	float	hull_repair_rate;				//How much of the hull is repaired every second
	float	subsys_repair_rate;		//How fast 

	float	sup_hull_repair_rate;
	float	sup_shield_repair_rate;
	float	sup_subsys_repair_rate;

	vec3d	closeup_pos;					// position for camera when using ship in closeup view (eg briefing and hud target monitor)
	float		closeup_zoom;					// zoom when using ship in closeup view (eg briefing and hud target monitor)

	int		allowed_weapons[MAX_WEAPON_TYPES];	// array which specifies which weapons can be loaded out by the
												// player during weapons loadout.

	// Goober5000 - fix for restricted banks mod
	int restricted_loadout_flag[MAX_SHIP_WEAPONS];
	int allowed_bank_restricted_weapons[MAX_SHIP_WEAPONS][MAX_WEAPON_TYPES];

	ubyte	shield_icon_index;				// index to locate ship-specific animation frames for the shield on HUD
	char	icon_filename[MAX_FILENAME_LEN];	// filename for icon that is displayed in ship selection
	char	anim_filename[MAX_FILENAME_LEN];	// filename for animation that plays in ship selection
	char	overhead_filename[MAX_FILENAME_LEN];	// filename for animation that plays weapons loadout
	int 	selection_effect;

	int bii_index_ship;						// if this ship has a briefing icon that overrides the normal icon set
	int bii_index_ship_with_cargo;
	int bii_index_wing;
	int bii_index_wing_with_cargo;

	int	score;								// default score for this ship

	int	scan_time;							// time to scan this ship (in ms)

	// contrail info
	trail_info ct_info[MAX_SHIP_CONTRAILS];	
	int ct_count;

	// rgb non-dimming pixels for this ship type
	int num_nondark_colors;
	ubyte nondark_colors[MAX_NONDARK_COLORS][3];

	// rgb shield color
	ubyte shield_color[3];

	// HW2-style team coloring
	bool uses_team_colors;
	SCP_string default_team_name;

	// optional afterburner trail values
	generic_bitmap afterburner_trail;
	float afterburner_trail_width_factor;
	float afterburner_trail_alpha_factor;
	float afterburner_trail_life;
	int afterburner_trail_faded_out_sections;

	// thruster particles
	SCP_vector<thruster_particles> normal_thruster_particles;
	SCP_vector<thruster_particles> afterburner_thruster_particles;

	// Bobboau's extra thruster stuff
	thrust_pair			thruster_flame_info;
	thrust_pair			thruster_glow_info;
	thrust_pair_bitmap	thruster_secondary_glow_info;
	thrust_pair_bitmap	thruster_tertiary_glow_info;
	thrust_pair_bitmap	thruster_distortion_info;

	float		thruster01_glow_rad_factor;
	float		thruster02_glow_rad_factor;
	float		thruster03_glow_rad_factor;
	float		thruster02_glow_len_factor;
	float		thruster_dist_rad_factor;
	float		thruster_dist_len_factor;

	bool		draw_distortion;

	int splodeing_texture;
	char splodeing_texture_name[MAX_FILENAME_LEN];

	// Goober5000
	SCP_vector<texture_replace> replacement_textures;

	
	int armor_type_idx;
	int shield_armor_type_idx;
	
	bool can_glide;
	float glide_cap;	//Backslash - for 'newtonian'-style gliding, the cap on velocity
	bool glide_dynamic_cap;	//SUSHI: Whether or not we are using a dynamic glide cap
	float glide_accel_mult;	//SUSHI: acceleration multiplier for glide mode
	bool use_newtonian_damp; //SUSHI: Whether or not to use newtonian dampening for this ship
	bool newtonian_damp_override; 

	float autoaim_fov;

	bool topdown_offset_def;
	vec3d topdown_offset;

	int engine_snd;							// handle to engine sound for ship (-1 if no engine sound)
	int glide_start_snd;					// handle to sound to play at the beginning of a glide maneuver (default is 0 for regular throttle down sound)
	int glide_end_snd;						// handle to sound to play at the end of a glide maneuver (default is 0 for regular throttle up sound)

	SCP_map<GameSoundsIndex, int> ship_sounds;			// specifies ship-specific sound indexes

	int num_maneuvering;
	man_thruster maneuvering[MAX_MAN_THRUSTERS];

	int radar_image_2d_idx;
	int radar_color_image_2d_idx;
	int radar_image_size;
	float radar_projection_size_mult;

	int ship_iff_info[MAX_IFFS][MAX_IFFS];

	flagset<Ship::Aiming_Flags> aiming_flags;
	float minimum_convergence_distance;
	float convergence_distance;
	vec3d convergence_offset;

	float emp_resistance_mod;

	float piercing_damage_draw_limit;

	int damage_lightning_type;

	SCP_vector<HudGauge*> hud_gauges;
	bool hud_enabled;
	bool hud_retail;

	SCP_vector<cockpit_display_info> displays;

	SCP_map<SCP_string, path_metadata> pathMetadata;
};

extern int Num_wings;
extern ship Ships[MAX_SHIPS];
extern ship	*Player_ship;
extern int	*Player_cockpit_textures;

// Data structure to track the active missiles
typedef struct ship_obj {
	ship_obj		 *next, *prev;
	int			flags, objnum;
} ship_obj;
extern ship_obj Ship_obj_list;

typedef struct engine_wash_info
{
	char		name[NAME_LENGTH];
	float		angle;			// half angle of cone around engine thruster
	float		radius_mult;	// multiplier for radius 
	float		length;			// length of engine wash, measured from thruster
	float		intensity;		// intensity of engine wash
	
} engine_wash_info;

extern SCP_vector<engine_wash_info> Engine_wash_info;

//	Defines a wing of ships.
typedef struct wing {
	char	name[NAME_LENGTH];
	char	wing_squad_filename[MAX_FILENAME_LEN];	// Goober5000
	int	reinforcement_index;					// index in reinforcement struct or -1
	int	hotkey;

	int	num_waves, current_wave;			// members for dealing with waves
	int	threshold;								// when number of ships in the wing reaches this number -- new wave

	fix	time_gone;								// time into the mission when this wing is officially gone.

	int	wave_count;								// max ships per wave (as defined by the number of ships in the ships list)
	int	total_arrived_count;					// count of number of ships that we have created, regardless of wave
	int red_alert_skipped_ships;				// Goober5000 - if we skipped over any indexes while creating red-alert ships
	int	current_count;							// count of number of ships actually in this wing -- used for limit in next array
	int	ship_index[MAX_SHIPS_PER_WING];	// index into ships array of all ships currently in the wing

	int	total_destroyed;						// total number of ships destroyed in the wing (including all waves)
	int	total_departed;						// total number of ships departed in this wing (including all waves)
	int total_vanished;						// total number of ships vanished in this wing (including all waves)

	int	special_ship;							// the leader of the wing.  An index into ship_index[].

	int	arrival_location;						// arrival and departure information for wings -- similar to info for ships
	int	arrival_distance;						// distance from some ship where this ship arrives
	int	arrival_anchor;						// name of object this ship arrives near (or in front of)
	int	arrival_path_mask;					// Goober5000 - possible restrictions on which bay paths to use
	int	arrival_cue;
	int	arrival_delay;

	int	departure_location;
	int	departure_anchor;						// name of object that we depart to (in case of dock bays)
	int departure_path_mask;				// Goober5000 - possible restrictions on which bay paths to use
	int	departure_cue;
	int	departure_delay;

	int	wave_delay_min;						// minimum number of seconds before new wave can arrive
	int	wave_delay_max;						// maximum number of seconds before new wave can arrive
	int	wave_delay_timestamp;				// timestamp used for delaying arrival of next wave

	flagset<Ship::Wing_Flags> flags;

	ai_goal	ai_goals[MAX_AI_GOALS];			// goals for the wing -- converted to ai_goal struct

	ushort	net_signature;						// starting net signature for ships in this wing. assiged at mission load time

	// Goober5000 - if this wing has a unique squad logo
	// it's specified for the wing rather than each individual ship to cut down on the amount
	// of stuff that needs to be sitting in memory at once - each ship uses the wing texture;
	// and it also makes practical sense: no wing has two different squadrons in it :)
	int wing_insignia_texture;
} wing;

extern wing Wings[MAX_WINGS];

extern int Starting_wings[MAX_STARTING_WINGS];
extern int Squadron_wings[MAX_SQUADRON_WINGS];
extern int TVT_wings[MAX_TVT_WINGS];

extern char Starting_wing_names[MAX_STARTING_WINGS][NAME_LENGTH];
extern char Squadron_wing_names[MAX_SQUADRON_WINGS][NAME_LENGTH];
extern char TVT_wing_names[MAX_TVT_WINGS][NAME_LENGTH];

extern int ai_paused;
extern int CLOAKMAP;

extern int Num_reinforcements;
extern int Num_ship_classes;
extern ship_info Ship_info[MAX_SHIP_CLASSES];
extern reinforcements Reinforcements[MAX_REINFORCEMENTS];

// structure definition for ship type counts.  Used to give a count of the number of ships
// of a particular type, and the number of times that a ship of that particular type has been
// killed.  When changing any info here, be sure to update the ship_type_names array in Ship.cpp
// the order of the types here MUST match the order of the types in the array
typedef struct ship_counts {
	int	total;
	int	killed;
	ship_counts(){total=0;killed=0;}
} ship_counts;

extern SCP_vector<ship_counts> Ship_type_counts;


// Use the below macros when you want to find the index of an array element in the
// Wings[] or Ships[] arrays.
#define WING_INDEX(wingp) (wingp-Wings)
#define SHIP_INDEX(shipp) (shipp-Ships)


extern void ship_init();				// called once	at game start
extern void ship_level_init();		// called before the start of each level

//returns -1 if failed
extern int ship_create(matrix * orient, vec3d * pos, int ship_type, char *ship_name = NULL);
extern void change_ship_type(int n, int ship_type, int by_sexp = 0);
extern void ship_model_change(int n, int ship_type);
extern void ship_process_pre( object * objp, float frametime );
extern void ship_process_post( object * objp, float frametime );
extern void ship_render( object * objp );
extern void ship_render_cockpit( object * objp);
extern void ship_render_show_ship_cockpit( object * objp);
extern void ship_delete( object * objp );
extern int ship_check_collision_fast( object * obj, object * other_obj, vec3d * hitpos );
extern int ship_get_num_ships();

#define SHIP_VANISHED			(1<<0)
#define SHIP_DESTROYED			(1<<1)
#define SHIP_DEPARTED_WARP		(1<<2)
#define SHIP_DEPARTED_BAY		(1<<3)
#define SHIP_DEPARTED			( SHIP_DEPARTED_BAY | SHIP_DEPARTED_WARP )
// Goober5000
extern void ship_cleanup(int shipnum, int cleanup_mode);
extern void ship_destroy_instantly(object *ship_obj, int shipnum);
extern void ship_actually_depart(int shipnum, int method = SHIP_DEPARTED_WARP);

extern int ship_fire_primary_debug(object *objp);	//	Fire the debug laser.
extern int ship_stop_fire_primary(object * obj);
extern int ship_fire_primary(object * objp, int stream_weapons, int force = 0);
extern int ship_fire_secondary(object * objp, int allow_swarm = 0 );
extern int ship_launch_countermeasure(object *objp, int rand_val = -1);

// for special targeting lasers
extern void ship_start_targeting_laser(ship *shipp);
extern void ship_stop_targeting_laser(ship *shipp);
extern void ship_process_targeting_lasers();

extern int ship_select_next_primary(object *objp, int direction);
extern int  ship_select_next_secondary(object *objp);

// Goober5000
extern int get_available_primary_weapons(object *objp, int *outlist, int *outbanklist);

extern int get_available_secondary_weapons(object *objp, int *outlist, int *outbanklist);
extern void ship_recalc_subsys_strength( ship *shipp );
extern int subsys_set(int objnum, int ignore_subsys_info = 0);
extern void physics_ship_init(object *objp);

//	Note: This is not a general purpose routine.
//	It is specifically used for targeting.
//	It only returns a subsystem position if it has shields.
//	Return true/false for subsystem found/not found.
//	Stuff vector *pos with absolute position.
extern int get_subsystem_pos(vec3d *pos, object *objp, ship_subsys *subsysp);

int parse_ship_values(ship_info* sip, bool first_time, bool replace);
void parse_ship_particle_effect(ship_info* sip, particle_effect* pe, char *id_string);

extern int ship_info_lookup(const char *name = NULL);
extern int ship_name_lookup(const char *name, int inc_players = 0);	// returns the index into Ship array of name
extern int ship_type_name_lookup(const char *name);

extern int wing_lookup(const char *name);

// returns 0 if no conflict, 1 if conflict, -1 on some kind of error with wing struct
extern int wing_has_conflicting_teams(int wing_index);

// next function takes optional second parameter which says to ignore the current count of ships
// in the wing -- used to tell is the wing exists or not, not whether it exists and has ships currently
// present.
extern int wing_name_lookup(const char *name, int ignore_count = 0);

// for generating a ship name for arbitrary waves/indexes of that wing... correctly handles the # character
extern void wing_bash_ship_name(char *ship_name, const char *wing_name, int index);

extern int Player_ship_class;

//	Do the special effect for energy dissipating into the shield for a hit.
//	model_num	= index in Polygon_models[]
//	centerp		= pos of object, sort of the center of the shield
//	tcp			= hit point, probably the global hit_point set in polygon_check_face
//	tr0			= index of polygon in shield pointer in polymodel.
extern void create_shield_explosion(int objnum, int model_num, matrix *orient, vec3d *centerp, vec3d *tcp, int tr0);

//	Initialize shield hit system.
extern void shield_hit_init();
extern void create_shield_explosion_all(object *objp);
extern void shield_frame_init();
extern void add_shield_point(int objnum, int tri_num, vec3d *hit_pos);
extern void add_shield_point_multi(int objnum, int tri_num, vec3d *hit_pos);
extern void shield_point_multi_setup();
extern void shield_hit_close();

void ship_draw_shield( object *objp);

float apply_damage_to_shield(object *objp, int quadrant, float damage);
float compute_shield_strength(object *objp);

// Returns true if the shield presents any opposition to something 
// trying to force through it.
// If quadrant is -1, looks at entire shield, otherwise
// just one quadrant
int ship_is_shield_up( object *obj, int quadrant );

//=================================================
// These two functions transfer instance specific angle
// data into and out of the model structure, which contains
// angles, but not for each instance of model being used. See
// the actual functions in ship.cpp for more details.
extern void ship_model_start(object *objp);
extern void ship_model_stop(object *objp);
void ship_model_update_instance(object *objp);

//============================================
extern int ship_find_num_crewpoints(object *objp);
extern int ship_find_num_turrets(object *objp);

extern void compute_slew_matrix(matrix *orient, angles *a);
//extern camid ship_set_eye( object *obj, int eye_index);
extern void ship_set_eye(object *obj, int eye_index);
extern void ship_get_eye( vec3d *eye_pos, matrix *eye_orient, object *obj, bool do_slew = true, bool from_origin = false);		// returns in eye the correct viewing position for the given object
//extern camid ship_get_followtarget_eye(object *obj);
extern ship_subsys *ship_get_indexed_subsys( ship *sp, int index, vec3d *attacker_pos = NULL );	// returns index'th subsystem of this ship
extern int ship_get_index_from_subsys(ship_subsys *ssp, int objnum, int error_bypass = 0);
extern int ship_get_subsys_index(ship *sp, char *ss_name, int error_bypass = 0);		// returns numerical index in linked list of subsystems
extern float ship_get_subsystem_strength( ship *shipp, int type );
extern ship_subsys *ship_get_subsys(ship *shipp, char *subsys_name);
extern int ship_get_num_subsys(ship *shipp);
extern ship_subsys *ship_get_closest_subsys_in_sight(ship *sp, int subsys_type, vec3d *attacker_pos);

//WMC
char *ship_subsys_get_name(ship_subsys *ss);
bool ship_subsys_has_instance_name(ship_subsys *ss);
void ship_subsys_set_name(ship_subsys *ss, char *n_name);

// subsys disruption
extern int ship_subsys_disrupted(ship_subsys *ss);
extern int ship_subsys_disrupted(ship *sp, int type);
extern void ship_subsys_set_disrupted(ship_subsys *ss, int time);

extern int	ship_do_rearm_frame( object *objp, float frametime );
extern float ship_calculate_rearm_duration( object *objp );
extern void	ship_wing_cleanup( int shipnum, wing *wingp );

extern int ship_find_repair_ship( object *requester_obj, object **ship_we_found = NULL );
extern void ship_close();	// called in game_shutdown() to free malloced memory


extern void ship_assign_sound_all();
extern void ship_assign_sound(ship *sp);

extern void ship_clear_ship_type_counts();
extern void ship_add_ship_type_count( int ship_info_index, int num );
extern void ship_add_ship_type_kill_count( int ship_info_index );

extern int ship_get_type(char* output, ship_info* sip);
extern int ship_get_default_orders_accepted( ship_info *sip );
extern int ship_query_general_type(int ship);
extern int ship_class_query_general_type(int ship_class);
extern int ship_query_general_type(ship *shipp);
extern int ship_docking_valid(int docker, int dockee);
extern int get_quadrant(vec3d *hit_pnt, object *shipobjp = NULL);	//	Return quadrant num of given hit point.

extern void ship_obj_list_rebuild();	// only called by save/restore code
extern int ship_query_state(char *name);

// Goober5000
int ship_primary_bank_has_ammo(int shipnum);	// check if current primary bank has ammo
int ship_secondary_bank_has_ammo(int shipnum);	// check if current secondary bank has ammo

int ship_engine_ok_to_warp(ship *sp);		// check if ship has engine power to warp
int ship_navigation_ok_to_warp(ship *sp);	// check if ship has navigation power to warp

int ship_return_subsys_path_normal(ship *sp, ship_subsys *ss, vec3d *gsubpos, vec3d *norm);
int ship_subsystem_in_sight(object* objp, ship_subsys* subsys, vec3d *eye_pos, vec3d* subsys_pos, int do_facing_check=1, float *dot_out=NULL, vec3d *vec_out=NULL);
ship_subsys *ship_return_next_subsys(ship *shipp, int type, vec3d *attacker_pos);

// defines and definition for function to get a random ship of a particular team (any ship,
// any ship but player ships, or only players)
#define SHIP_GET_ANY_SHIP				0
#define SHIP_GET_NO_PLAYERS				1
#define SHIP_GET_ONLY_PLAYERS			2
#define SHIP_GET_UNSILENCED				3	// Karajorma - Returns no_players that can send builtin messages.


extern int ship_get_random_team_ship(int team_mask, int flags = SHIP_GET_ANY_SHIP, float max_dist = 0.0f);
extern int ship_get_random_player_wing_ship(int flags = SHIP_GET_ANY_SHIP, float max_dist = 0.0f, int persona_index = -1, int get_first = 0, int multi_team = -1);
extern int ship_get_random_ship_in_wing(int wingnum, int flags = SHIP_GET_ANY_SHIP, float max_dist = 0.0f, int get_first = 0);

// return ship index
int ship_get_random_targetable_ship();

extern int ship_get_by_signature(int signature);

#ifndef NDEBUG
extern int Ai_render_debug_flag;
extern int Show_shield_mesh;
extern int Ship_auto_repair;	// flag to indicate auto-repair of subsystem should occur
#endif

void ship_subsystem_delete(ship *shipp);
void ship_set_default_weapons(ship *shipp, ship_info *sip);
float ship_quadrant_shield_strength(object *hit_objp, vec3d *hitpos);

int ship_dumbfire_threat(ship *sp);
int ship_lock_threat(ship *sp);

int	bitmask_2_bitnum(int num);
char	*ship_return_orders(char *outbuf, ship *sp);
char	*ship_return_time_to_goal(char *outbuf, ship *sp);

void	ship_maybe_warn_player(ship *enemy_sp, float dist);
void	ship_maybe_praise_player(ship *deader_sp);
void	ship_maybe_praise_self(ship *deader_sp, ship *killer_sp);
void	ship_maybe_ask_for_help(ship *sp);
void	ship_scream(ship *sp);
void	ship_maybe_scream(ship *sp);
void	ship_maybe_tell_about_rearm(ship *sp);
void	ship_maybe_tell_about_low_ammo(ship *sp);
void	ship_maybe_lament();

void ship_primary_changed(ship *sp);
void ship_secondary_changed(ship *sp);

// get the Ship_info flags for a given ship
flagset<Ship::Info_Flags> ship_get_SIF(ship *shipp);
flagset<Ship::Info_Flags> ship_get_SIF(int sh);

// get the ship type info (objecttypes.tbl)
ship_type_info *ship_get_type_info(object *objp);

extern void ship_do_cargo_revealed( ship *shipp, int from_network = 0 );
extern void ship_do_cargo_hidden( ship *shipp, int from_network = 0 );
extern void ship_do_cap_subsys_cargo_revealed( ship *shipp, ship_subsys *subsys, int from_network = 0);
extern void ship_do_cap_subsys_cargo_hidden( ship *shipp, ship_subsys *subsys, int from_network = 0);

float ship_get_secondary_weapon_range(ship *shipp);

// Goober5000
int primary_out_of_ammo(ship_weapon *swp, int bank);
int get_max_ammo_count_for_primary_bank(int ship_class, int bank, int ammo_type);

int get_max_ammo_count_for_bank(int ship_class, int bank, int ammo_type);

int is_support_allowed(object *objp, bool do_simple_check = false);

// Given an object and a turret on that object, return the actual firing point of the gun
// and its normal.   This uses the current turret angles.  We are keeping track of which
// gun to fire next in the ship specific info for this turret subobject.  Use this info
// to determine which position to fire from next.
//	Stuffs:
//		*gpos: absolute position of gun firing point
//		*gvec: vector fro *gpos to *targetp
void ship_get_global_turret_gun_info(object *objp, ship_subsys *ssp, vec3d *gpos, vec3d *gvec, int use_angles, vec3d *targetp);

//	Given an object and a turret on that object, return the global position and forward vector
//	of the turret.   The gun normal is the unrotated gun normal, (the center of the FOV cone), not
// the actual gun normal given using the current turret heading.  But it _is_ rotated into the model's orientation
//	in global space.
void ship_get_global_turret_info(object *objp, model_subsystem *tp, vec3d *gpos, vec3d *gvec);

// return 1 if objp is in fov of the specified turret, tp.  Otherwise return 0.
//	dist = distance from turret to center point of object
int object_in_turret_fov(object *objp, ship_subsys *ss, vec3d *tvec, vec3d *tpos, float dist);

// functions for testing fov.. returns true if fov test is passed.
bool turret_std_fov_test(ship_subsys *ss, vec3d *gvec, vec3d *v2e, float size_mod = 0);
bool turret_adv_fov_test(ship_subsys *ss, vec3d *gvec, vec3d *v2e, float size_mod = 0);
bool turret_fov_test(ship_subsys *ss, vec3d *gvec, vec3d *v2e, float size_mod = 0);

// function for checking adjusted turret rof
float get_adjusted_turret_rof(ship_subsys *ss);

// forcible jettison cargo from a ship
void object_jettison_cargo(object *objp, object *cargo_objp);

// get damage done by exploding ship, takes into account mods for individual ship
float ship_get_exp_damage(object* objp);

// get whether ship has shockwave, takes into account mods for individual ship
int ship_get_exp_propagates(ship *sp);

// get outer radius of damage, takes into account mods for individual ship
float ship_get_exp_outer_rad(object *ship_objp);

// externed by Goober5000
extern int ship_explode_area_calc_damage( vec3d *pos1, vec3d *pos2, float inner_rad, float outer_rad, float max_damage, float max_blast, float *damage, float *blast );

// returns whether subsys is allowed to have cargo
int valid_cap_subsys_cargo_list(char *subsys_name);

// determine turret status of a given subsystem, returns 0 for no turret, 1 for "fixed turret", 2 for "rotating" turret
int ship_get_turret_type(ship_subsys *subsys);

// get ship by object signature, returns OBJECT INDEX
int ship_get_by_signature(int sig);

// get the team of a reinforcement item
int ship_get_reinforcement_team(int r_index);

// determine if the given texture is used by a ship type. return ship info index, or -1 if not used by a ship
int ship_get_texture(int bitmap);

// page in bitmaps for all ships on a given level
void ship_page_in();

// Goober5000 - helper for above
void ship_page_in_textures(int ship_index = -1);

// fixer for above - taylor
void ship_page_out_textures(int ship_index, bool release = false);

// update artillery lock info
void ship_update_artillery_lock();

// checks if a world point is inside the extended bounding box of a ship
int check_world_pt_in_expanded_ship_bbox(vec3d *world_pt, object *objp, float delta_box);

// returns true if objp is ship and is tagged
int ship_is_tagged(object *objp);

// returns max normal speed of ship (overclocked / afterburned)
float ship_get_max_speed(ship *shipp);

// returns warpout speed of ship
float ship_get_warpout_speed(object *objp);

// returns true if ship is beginning to speed up in warpout
int ship_is_beginning_warpout_speedup(object *objp);

// return the length of the ship class
float ship_class_get_length(ship_info *sip);

// Goober5000 - used by change-ai-class
extern void ship_set_new_ai_class(int ship_num, int new_ai_class);
extern void ship_subsystem_set_new_ai_class(int ship_num, char *subsystem, int new_ai_class);

// wing squad logos - Goober5000
extern void wing_load_squad_bitmap(wing *w);

// Goober5000 - needed by new hangar depart code
extern bool ship_has_dock_bay(int shipnum);
extern bool ship_useful_for_departure(int shipnum, int path_mask = 0);
extern int ship_get_ship_for_departure(int team);

// Goober5000 - moved here from hudbrackets.cpp
extern bool ship_subsys_is_fighterbay(ship_subsys *ss);

// Goober5000
extern bool ship_fighterbays_all_destroyed(ship *shipp);

// Goober5000
extern bool ship_subsys_takes_damage(ship_subsys *ss);

//phreak
extern int ship_fire_tertiary(object *objp);

// Goober5000 - handles submodel rotation, incorporating conditions such as gun barrels when firing
extern void ship_do_submodel_rotation(ship *shipp, model_subsystem *psub, ship_subsys *pss);

// Goober5000 - shortcut hud stuff
extern int ship_has_energy_weapons(ship *shipp);
extern int ship_has_engine_power(ship *shipp);

// Swifty - Cockpit displays
void ship_init_cockpit_displays(ship *shipp);
void ship_add_cockpit_display(cockpit_display_info *display, int cockpit_model_num);
void ship_set_hud_cockpit_targets();
void ship_clear_cockpit_displays();
int ship_start_render_cockpit_display(int cockpit_display_num);
void ship_end_render_cockpit_display(int cockpit_display_num);

//WMC - Warptype stuff
int warptype_match(char *p);

// Goober5000
int ship_starting_wing_lookup(const char *wing_name);
int ship_squadron_wing_lookup(const char *wing_name);
int ship_tvt_wing_lookup(const char *wing_name);

// Goober5000
int ship_class_compare(int ship_class_1, int ship_class_2);

int armor_type_get_idx(char* name);

void armor_init();

int thruster_glow_anim_load(generic_anim *ga);

// Sushi - Path metadata
void init_path_metadata(path_metadata& metadata);


typedef struct ship_effect {
	char name[NAME_LENGTH];
	bool disables_rendering;
	bool invert_timer;
	int shader_effect;
} ship_effect;

extern SCP_vector<ship_effect> Ship_effects;

/**
 *  @brief Returns a ship-specific sound index
 *  
 *  @param objp An object pointer. Has to be of type OBJ_SHIP
 *  @param id A sound id as defined in gamsesnd.h. If the given id is unknown then the game_snd with the id as index is returned.
 *  
 *  @return An index into the Snds vector, if the specified index could not be found then the id itself will be returned
 */
int ship_get_sound(object *objp, GameSoundsIndex id);

/**
 *  @brief Specifies if a ship has a custom sound for the specified id
 *  
 *  @param objp An object pointer. Has to be of type OBJ_SHIP
 *  @param id A sound id as defined in gamsesnd.h
 *  
 *  @return True if this object has the specified sound, false otherwise
 */
bool ship_has_sound(object *objp, GameSoundsIndex id);

/**
 * @brief Returns the index of the default player ship
 *
 * @return An index into Ship_info[], location of the default player ship.
 */
int get_default_player_ship_index();

/**
 * Given a ship with bounding box and a point, find the closest point on the bbox
 *
 * @param ship_obj Object that has the bounding box (should be a ship)
 * @param start World position of the point being compared
 * @param box_pt OUTPUT PARAMETER: closest point on the bbox to start
 *
 * @return point is inside bbox, TRUE/1
 * @return point is outside bbox, FALSE/0
 */
int get_nearest_bbox_point(object *ship_obj, vec3d *start, vec3d *box_pt);


inline bool is_ship_arriving(ship* shipp) { return shipp->flags[Ship::Ship_Flags::Arriving_stage_1] || shipp->flags[Ship::Ship_Flags::Arriving_stage_2]; }
inline bool is_ship_departing(ship* shipp) { return shipp->flags[Ship::Ship_Flags::Depart_warp] || shipp->flags[Ship::Ship_Flags::Depart_dockbay]; }
inline bool is_small_ship(ship_info* sip) { return sip->flags[Ship::Info_Flags::Fighter] || sip->flags[Ship::Info_Flags::Bomber] || sip->flags[Ship::Info_Flags::Support] || sip->flags[Ship::Info_Flags::Escapepod]; }
inline bool is_big_ship(ship_info* sip) { return sip->flags[Ship::Info_Flags::Cruiser] || sip->flags[Ship::Info_Flags::Freighter] || sip->flags[Ship::Info_Flags::Transport] || sip->flags[Ship::Info_Flags::Corvette] || sip->flags[Ship::Info_Flags::Gas_miner] || sip->flags[Ship::Info_Flags::Awacs]; }
inline bool is_huge_ship(ship_info* sip) { return sip->flags[Ship::Info_Flags::Capital] || sip->flags[Ship::Info_Flags::Supercap] || sip->flags[Ship::Info_Flags::Drydock] || sip->flags[Ship::Info_Flags::Knossos_device]; }
inline bool is_flyable(ship_info* sip) { return !(sip->flags[Ship::Info_Flags::Cargo] || sip->flags[Ship::Info_Flags::Navbuoy] || sip->flags[Ship::Info_Flags::Escapepod]); }
inline bool is_harmless(ship_info* sip) { return !is_flyable(sip); }
inline bool is_fighter_bomber(ship_info* sip) { return sip->flags[Ship::Info_Flags::Fighter] || sip->flags[Ship::Info_Flags::Bomber]; }
inline bool is_big_huge(ship_info* sip) { return is_big_ship(sip) || is_huge_ship(sip); }
inline bool ship_cannot_warp(ship* shipp) { return shipp->flags[Ship::Ship_Flags::Warp_broken] || shipp->flags[Ship::Ship_Flags::Warp_never] || shipp->flags[Ship::Ship_Flags::Disabled]; }
inline bool is_dying_departing(ship* shipp) { return is_ship_departing(shipp) || shipp->flags[Ship::Ship_Flags::Dying]; }

extern flagset<Ship::Ship_Flags> Ignore_List;
inline bool should_be_ignored(ship* shipp) {
	return (shipp->flags & Ignore_List).any_set();
}

extern void set_default_ignore_list();

extern void toggle_ignore_list_flag(Ship::Ship_Flags flag);

// for ships of this type, we make beam weapons miss a little bit otherwise they'd be way too powerful
inline bool apply_beam_jitter(ship_info* sip) {
	return sip->flags[Ship::Info_Flags::Cargo] || sip->flags[Ship::Info_Flags::Fighter] || sip->flags[Ship::Info_Flags::Bomber] || sip->flags[Ship::Info_Flags::Freighter] || sip->flags[Ship::Info_Flags::Transport] || sip->flags[Ship::Info_Flags::Sentrygun] || sip->flags[Ship::Info_Flags::Navbuoy] || sip->flags[Ship::Info_Flags::Escapepod];
}

inline bool avoids_shockwaves(ship_info* sip) { return is_small_ship(sip); }

#endif
