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
#include "weapon/shockwave.h"
#include "species_defs/species_defs.h"
#include "globalincs/pstypes.h"
#include "fireball/fireballs.h"
#include "hud/hud.h"

#include <string>

struct object;
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
#define	MAX_REINFORCEMENTS		10


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

// ship weapon flags
#define SW_FLAG_BEAM_FREE					(1<<0)							// if this is a beam weapon, its free to fire
#define SW_FLAG_TURRET_LOCK				(1<<1)							//	is this turret is free to fire or locked
#define SW_FLAG_TAGGED_ONLY				(1<<2)							// only fire if target is tagged

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

	// ballistic primary support - by Goober5000
	int primary_bank_ammo[MAX_SHIP_PRIMARY_BANKS];			// Number of missiles left in primary bank
	int primary_bank_start_ammo[MAX_SHIP_PRIMARY_BANKS];	// Number of missiles starting in primary bank
	int primary_bank_capacity[MAX_SHIP_PRIMARY_BANKS];		// Max number of projectiles in bank
	int primary_next_slot[MAX_SHIP_PRIMARY_BANKS];			// Next slot to fire in the bank
	int primary_bank_rearm_time[MAX_SHIP_PRIMARY_BANKS];	// timestamp which indicates when bank can get new projectile

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

	int flags;								// see SW_FLAG_* defines above
	ubyte primary_animation_position[MAX_SHIP_PRIMARY_BANKS];
	ubyte secondary_animation_position[MAX_SHIP_SECONDARY_BANKS];
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

struct ArmorDamageType
{
	friend class ArmorType;
private:
	//Rather than make an extra struct,
	//I just made two arrays
	int					DamageTypeIndex;
	SCP_vector<int>	Calculations;
	SCP_vector<float>	Arguments;
	float				shieldpierce_pct;

	// piercing effect data
	float				piercing_start_pct;
	int					piercing_type;

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
	float GetDamage(float damage_applied, int in_damage_type_idx);
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

typedef struct cockpit_display_info {
	char name[MAX_FILENAME_LEN];
	char filename[MAX_FILENAME_LEN];
	char fg_filename[MAX_FILENAME_LEN];
	char bg_filename[MAX_FILENAME_LEN];
	int offset[2];
	int size[2];
} cockpit_display_info;

// Goober5000
#define SSF_CARGO_REVEALED		(1 << 0)
#define SSF_UNTARGETABLE		(1 << 1)
#define SSF_NO_SS_TARGETING     (1 << 2)

//nuke
#define SSF_HAS_FIRED		    (1 << 3)		//used by scripting to flag a turret as having been fired
#define SSF_FOV_REQUIRED		(1 << 4)
#define SSF_FOV_EDGE_CHECK		(1 << 5)

#define SSF_NO_REPLACE			(1 << 6)		// prevents 'destroyed' submodel from being rendered if subsys is destroyed.
#define SSF_NO_LIVE_DEBRIS		(1 << 7)		// prevents subsystem from generating live debris
#define SSF_VANISHED			(1 << 8)		// allows subsystem to be made to disappear without a trace (for swapping it for a true model for example.
#define SSF_MISSILES_IGNORE_IF_DEAD	(1 << 9)	// forces homing missiles to target hull if subsystem is dead before missile hits it.
#define SSF_ROTATES				(1 << 10)
#define SSF_DAMAGE_AS_HULL		(1 << 11)		// Applies armor damage instead of subsystem damge. - FUBAR
#define SSF_NO_AGGREGATE		(1 << 12)		// exclude this subsystem from the aggregate subsystem-info tracking - Goober5000


// Wanderer 
#define SSSF_ALIVE					(1 << 0)		// subsystem has active alive sound
#define SSSF_DEAD					(1 << 1)		// subsystem has active dead sound
#define SSSF_ROTATE					(1 << 2)		// subsystem has active rotation sound
#define SSSF_TURRET_ROTATION		(1 << 3)		// rotation sound to be scaled like turrets do

// structure definition for a linked list of subsystems for a ship.  Each subsystem has a pointer
// to the static data for the subsystem.  The obj_subsystem data is defined and read in the model
// code.  Other dynamic data (such as current_hits) should remain in this structure.
typedef	struct ship_subsys {
	struct ship_subsys *next, *prev;				//	Index of next and previous objects in list.
	model_subsystem *system_info;					// pointer to static data for this subsystem -- see model.h for definition

	char		sub_name[NAME_LENGTH];					//WMC - Name that overrides name of original
	float		current_hits;							// current number of hits this subsystem has left.
	float		max_hits;

	int flags;						// Goober5000

	int subsys_guardian_threshold;	// Goober5000
	int armor_type_idx;				// FUBAR

	// turret info
	//Important -WMC
	//With the new turret code, indexes run from 0 to MAX_SHIP_WEAPONS; a value of MAX_SHIP_PRIMARY_WEAPONS
	//or higher, an index into the turret weapons is considered to be an index into the secondary weapons
	//for much of the code. See turret_next_weap_fire_stamp.

	//Note that turret_next_weap_fire_stamp is officially a hack, because turrets use all this crap
	//ideally, they should make use of the ship_weapon structure below
	//int		turret_next_weap_fire_stamp[MAX_SHIP_WEAPONS];	//Fire stamps for all weapons on this turret
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

	int		turret_pick_big_attack_point_timestamp;	//	Next time to pick an attack point for this turret
	vec3d	turret_big_attack_point;			//	local coordinate of point for this turret to attack on enemy

	ubyte	turret_animation_position;
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

	triggered_rotation trigger;		//the actual currently running animation and assosiated states

	float points_to_target;
	float base_rotation_rate_pct;
	float gun_rotation_rate_pct;

	// still going through these...
	int subsys_snd_flags;

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
} ship_subsys;

// structure for subsystems which tells us the total count of a particular type of subsystem (i.e.
// we might have 3 engines), and the relative strength of the subsystem.  The #defines in model.h
// for SUBSYSTEM_xxx will be used as indices into this array.
typedef struct ship_subsys_info {
	int	type_count;					// number of subsystems of type on this ship;
	float aggregate_max_hits;		// maximum number of hits for all subsystems of this type.
	float aggregate_current_hits;	// current count of hits for all subsystems of this type.	
} ship_subsys_info;


// states for the flags variable within the ship structure
// low bits are for mission file savable flags..
// FRED needs these to be the low-order bits with no holes,
// because it indexes into an array, Hoffoss says.
#define	SF_IGNORE_COUNT			(1 << 0)		// ignore this ship when counting ship types for goals
#define	SF_REINFORCEMENT			(1 << 1)		// this ship is a reinforcement ship
#define	SF_ESCORT					(1 << 2)		// this ship is an escort ship
#define	SF_NO_ARRIVAL_MUSIC		(1 << 3)		// don't play arrival music when ship arrives
#define	SF_NO_ARRIVAL_WARP		(1 << 4)		// no arrival warp in effect
#define	SF_NO_DEPARTURE_WARP		(1 << 5)		// no departure warp in effect
#define	SF_LOCKED					(1 << 6)		// can't manipulate ship in loadout screens
//#define	SF_INVULNERABLE			(1 << 7)

// high bits are for internal flags not saved to mission files
// Go from bit 31 down to bit 3
#define	SF_KILL_BEFORE_MISSION	(1 << 31)
#define	SF_DYING						(1 << 30)
#define	SF_DISABLED					(1 << 29)
#define	SF_DEPART_WARP				(1 << 28)	// ship is departing via warp-out
#define	SF_DEPART_DOCKBAY			(1 << 27)	// ship is departing via docking bay
#define	SF_ARRIVING_STAGE_1		(1 << 26)	// ship is arriving. In other words, doing warp in effect, stage 1
#define	SF_ARRIVING_STAGE_2		(1 << 25)	// ship is arriving. In other words, doing warp in effect, stage 2
#define  SF_ARRIVING             (SF_ARRIVING_STAGE_1|SF_ARRIVING_STAGE_2)
#define	SF_ENGINES_ON				(1 << 24)	// engines sound should play if set
#define	SF_DOCK_LEADER			(1 << 23)	// Goober5000 - this guy is in charge of everybody he's docked to
#define	SF_CARGO_REVEALED			(1 << 22)	// ship's cargo is revealed to all friendly ships
#define	SF_FROM_PLAYER_WING		(1	<< 21)	// set for ships that are members of any player starting wing
#define	SF_PRIMARY_LINKED			(1 << 20)	// ships primary weapons are linked together
#define	SF_SECONDARY_DUAL_FIRE	(1 << 19)	// ship is firing two missiles from the current secondary bank
#define	SF_WARP_BROKEN				(1	<< 18)	// set when warp drive is not working, but is repairable
#define	SF_WARP_NEVER				(1	<< 17)	// set when ship can never warp
#define	SF_TRIGGER_DOWN			(1 << 16)	// ship has its "trigger" held down
#define	SF_AMMO_COUNT_RECORDED	(1	<<	15)	// we've recorded the initial secondary weapon count (which is used to limit support ship rearming)
#define	SF_HIDDEN_FROM_SENSORS	(1	<< 14)	// ship doesn't show up on sensors, blinks in/out on radar
#define	SF_SCANNABLE				(1	<< 13)	// ship is "scannable".  Play scan effect and report as "Scanned" or "not scanned".
#define	SF_WARPED_SUPPORT			(1 << 12)	// set when this is a support ship which was warped in automatically
#define	SF_EXPLODED					(1 << 11)	// ship has exploded (needed for kill messages)
#define	SF_SHIP_HAS_SCREAMED		(1 << 10)	// ship has let out a death scream
#define	SF_RED_ALERT_STORE_STATUS (1 << 9)	// ship status should be stored/restored if red alert mission
#define	SF_VAPORIZE					(1<<8)		// ship is vaporized by beam - alternative death sequence

// MWA -- don't go below whatever bitfield is used for Fred above (currently 7)!!!!

#define	SF_DEPARTING				(SF_DEPART_WARP | SF_DEPART_DOCKBAY)				// ship is departing
#define	SF_CANNOT_WARP				(SF_WARP_BROKEN | SF_WARP_NEVER | SF_DISABLED)	// ship cannot warp out


#define DEFAULT_SHIP_PRIMITIVE_SENSOR_RANGE		10000	// Goober5000


// Bits for ship.flags2
#define SF2_PRIMITIVE_SENSORS				(1<<0)		// Goober5000 - primitive sensor display
#define SF2_FRIENDLY_STEALTH_INVIS			(1<<1)		// Goober5000 - when stealth, don't appear on radar even if friendly
#define SF2_STEALTH							(1<<2)		// Goober5000 - is this particular ship stealth
#define SF2_DONT_COLLIDE_INVIS				(1<<3)		// Goober5000 - is this particular ship don't-collide-invisible
#define SF2_NO_SUBSPACE_DRIVE				(1<<4)		// Goober5000 - this ship has no subspace drive
#define SF2_NAVPOINT_CARRY					(1<<5)		// Kazan      - This ship autopilots with the player
#define SF2_AFFECTED_BY_GRAVITY				(1<<6)		// Goober5000 - ship affected by gravity points
#define SF2_TOGGLE_SUBSYSTEM_SCANNING		(1<<7)		// Goober5000 - switch whether subsystems are scanned
#define SF2_NO_BUILTIN_MESSAGES				(1<<8)		// Karajorma - ship should not send built-in messages
#define SF2_PRIMARIES_LOCKED				(1<<9)		// Karajorma - This ship can't fire primary weapons
#define SF2_SECONDARIES_LOCKED				(1<<10)		// Karajorma - This ship can't fire secondary weapons
#define SF2_GLOWMAPS_DISABLED				(1<<11)		// taylor - to disable glow maps
#define SF2_NO_DEATH_SCREAM					(1<<12)		// Goober5000 - for WCS
#define SF2_ALWAYS_DEATH_SCREAM				(1<<13)		// Goober5000 - for WCS
#define SF2_NAVPOINT_NEEDSLINK				(1<<14)		// Kazan	- This ship requires "linking" for autopilot (when player ship gets within specified distance SF2_NAVPOINT_NEEDSLINK is replaced by SF2_NAVPOINT_CARRY)
#define SF2_HIDE_SHIP_NAME					(1<<15)		// Karajorma - Hides the ships name (like the -wcsaga command line used to but for any selected ship)
#define SF2_AFTERBURNER_LOCKED				(1<<16)		// KeldorKatarn - This ship can't use its afterburners
#define SF2_SET_CLASS_DYNAMICALLY			(1<<18)		// Karajorma - This ship should have its class assigned rather than simply read from the mission file 
#define SF2_LOCK_ALL_TURRETS_INITIALLY		(1<<19)		// Karajorma - Lock all turrets on this ship at mission start or on arrival
#define SF2_FORCE_SHIELDS_ON				(1<<20)
#define SF2_NO_ETS							(1<<21)		// The E - This ship does not have an ETS

// If any of these bits in the ship->flags are set, ignore this ship when targetting
extern int TARGET_SHIP_IGNORE_FLAGS;

#define MAX_DAMAGE_SLOTS	32
#define MAX_SHIP_ARCS		2		// How many "arcs" can be active at once... Must be less than MAX_ARC_EFFECTS in model.h. 
#define NUM_SUB_EXPL_HANDLES	2	// How many different big ship sub explosion sounds can be played.

#define MAX_SHIP_CONTRAILS		12
#define MAX_MAN_THRUSTERS	128

typedef struct ship_spark {
	vec3d pos;			// position of spark in the submodel's RF
	int submodel_num;	// which submodel is making the spark
	int end_time;
} ship_spark;

#define AWACS_WARN_NONE		(1 << 0)
#define AWACS_WARN_25		(1 << 1)
#define AWACS_WARN_75		(1 << 2)

typedef struct ship {
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
	// corkscrew missile stuff
	ubyte num_corkscrew_to_fire;						// # of corkscrew missiles lef to fire
	// END PACK

	// targeting laser info
	int targeting_laser_objnum;					// -1 if invalid, beam object # otherwise
	// corkscrew missile stuff
	int next_corkscrew_fire;						// next time to fire a corkscrew missile

	int	final_death_time;				// Time until big fireball starts
	int	death_time;				// Time until big fireball starts
	int	end_death_time;				// Time until big fireball starts
	int	really_final_death_time;	// Time until ship breaks up and disappears
	vec3d	deathroll_rotvel;			// Desired death rotational velocity

	WarpEffect *warpin_effect;
	WarpEffect *warpout_effect;
/*
	int start_warp_time;
	int	final_warp_time;	// pops when ship is completely warped out or warped in.  Used for both warp in and out.
	int warp_anim;
	int warp_anim_nframes;
	int warp_anim_fps;
	vec3d	warp_effect_pos;		// where the warp in effect comes in at
	vec3d	warp_effect_fvec;		// The warp in effect's forward vector
	float warp_radius;
	int warp_stage;			//WMC - stage for warp, used by WT_SWEEPER
	float warp_width;
	float warp_height;
*/
	int	next_fireball;

	int	next_hit_spark;
	int	num_hits;			//	Note, this is the number of spark emitter positions!
	ship_spark	sparks[MAX_SHIP_HITS];
	
	bool use_special_explosion; 
	float special_exp_damage;					// new special explosion/hitpoints system
	float special_exp_blast;
	float special_exp_inner;
	float special_exp_outer;
	bool use_shockwave;
	float special_exp_shockwave_speed;
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

	uint	flags;								// flag variable to contain ship state (see SF_ #defines)
	uint	flags2;								// another flag variable (see SF2_ #defines)
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

	float		wash_intensity;
	vec3d	wash_rot_axis;
	int		wash_timestamp;

	// store blast information about shockwaves that hit the ship
//	ship_shockwave	sw;

	int	num_swarm_missiles_to_fire;	// number of swarm missiles that need to be launched
	int	next_swarm_fire;					// timestamp of next swarm missile to fire
	int	next_swarm_path;					// next path number for swarm missile to take
	int	num_turret_swarm_info;			// number of turrets in process of launching swarm

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
	ubyte	awacs_warning_flag;

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
	int *cockpit_replacement_textures;

	SCP_vector<cockpit_display> displays;

	// Goober5000 - index into pm->view_positions[]
	// apparently, early in FS1 development, there was a field called current_eye_index
	// that had this same functionality
	int current_viewpoint;

	//WMC - this one
	//camid ship_camera;

	trail *ABtrail_ptr[MAX_SHIP_CONTRAILS];		//after burner trails -Bobboau
	trail_info ab_info[MAX_SHIP_CONTRAILS];
	int ab_count;

	// glow points
	SCP_vector<bool> glow_point_bank_active;

	//cloaking stuff
	vec3d texture_translation_key;		//translate the texture matrix for a cool effect
	vec3d current_translation;
	int cloak_stage;
	fix time_until_full_cloak;
	int cloak_alpha;
	fix time_until_uncloak;

	int last_fired_point[MAX_SHIP_PRIMARY_BANKS]; //for fire point cylceing

	// fighter bay door stuff, parent side
	int bay_doors_anim_done_time;		// ammount of time to transition from one animation state to another
	ubyte bay_doors_status;			// anim status of the bay doors (closed/not-animating, opening, open/not-animating)
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
/*
	flash_ball	*debris_flare;
	int n_debris_flare;
	float flare_life;
	int flare_bm;
	*/

	SCP_vector<alt_class> s_alt_classes;	

	int ship_iff_color[MAX_IFFS][MAX_IFFS];

	int ammo_low_complaint_count;				// number of times this ship has complained about low ammo
	int armor_type_idx;
	int shield_armor_type_idx;
	int collision_damage_type_idx;
	int debris_damage_type_idx;

	int model_instance_num;
} ship;

struct ai_target_priority {
	char name[NAME_LENGTH];

	int obj_type;
	SCP_vector <int> ship_type;
	SCP_vector <int> ship_class;
	SCP_vector <int> weapon_class;

	unsigned int obj_flags;
	int sif_flags;
	int sif2_flags;
	int wif_flags;
	int wif2_flags;
};

extern SCP_vector <ai_target_priority> Ai_tp_list;

void parse_ai_target_priorities();
void parse_weapon_targeting_priorities();
ai_target_priority init_ai_target_priorities();

// structure and array def for ships that have exited the game.  Keeps track of certain useful
// information.
#define SEF_DESTROYED			(1<<0)
#define SEF_DEPARTED				(1<<1)
#define SEF_CARGO_KNOWN			(1<<2)
#define SEF_PLAYER_DELETED		(1<<3)			// ship deleted by a player in ship select
#define SEF_BEEN_TAGGED			(1<<4)
#define SEF_RED_ALERT_CARRY	(1<<5)

typedef struct exited_ship {
	char	ship_name[NAME_LENGTH];
	int		obj_signature;
	int		ship_class;
	int		team;
	int		flags;
	fix		time;
	int		hull_strength;
	fix		time_cargo_revealed;
	char	cargo1;
	float damage_ship[MAX_DAMAGE_SLOTS];		// A copy of the arrays from the ship so that we can figure out what damaged it
	int   damage_ship_id[MAX_DAMAGE_SLOTS];

	exited_ship()
		: team( 0 ), flags( 0 ), time( 0 ), hull_strength( 0 ),
		  time_cargo_revealed( 0 ), cargo1( 0 )
	{ 
		ship_name[ 0 ] = '\0';
		obj_signature = -1;
		ship_class = -1; 
		memset( damage_ship, 0, sizeof( damage_ship ) );
		memset( damage_ship_id, 0, sizeof( damage_ship_id ) );
	}
} exited_ship;

extern SCP_vector<exited_ship> Ships_exited;

// a couple of functions to get at the data
extern void ship_add_exited_ship( ship *shipp, int reason );
extern int ship_find_exited_ship_by_name( char *name );
extern int ship_find_exited_ship_by_signature( int signature);

#define	SIF_NO_COLLIDE				(1 << 0)
#define	SIF_PLAYER_SHIP				(1 << 1)
#define	SIF_DEFAULT_PLAYER_SHIP		(1 << 2)
#define	SIF_PATH_FIXUP				(1 << 3)		// when set, path verts have been set for this ship's model
#define	SIF_SUPPORT					(1 << 4)		// this ship can perform repair/rearm functions
#define	SIF_AFTERBURNER				(1 << 5)		// this ship has afterburners
#define SIF_BALLISTIC_PRIMARIES		(1 << 6)		// this ship can equip ballistic primaries - Goober5000

// If you add a new ship type, then please add the appropriate type in the ship_count
// structure later in this file!!! and let MWA know!!
#define	SIF_CARGO					(1 << 7)		// is this ship a cargo type ship -- used for docking purposes
#define	SIF_FIGHTER					(1 << 8)		// this ship is a fighter
#define	SIF_BOMBER					(1 << 9)		// this ship is a bomber
#define	SIF_CRUISER					(1 << 10)		// this ship is a cruiser
#define	SIF_FREIGHTER				(1 << 11)	// this ship is a freighter
#define	SIF_CAPITAL					(1 << 12)	// this ship is a capital/installation ship
#define	SIF_TRANSPORT				(1 << 13)	// this ship is a transport
#define	SIF_NAVBUOY					(1 << 14)	// AL 11-24-97: this is a navbuoy
#define	SIF_SENTRYGUN				(1 << 15)	// AL 11-24-97: this is a navbuoy with turrets
#define	SIF_ESCAPEPOD				(1 << 16)	// AL 12-09-97: escape pods that fire from big ships
#define	SIF_NO_SHIP_TYPE			(1 << 17)	// made distinct to help trap errors

#define	SIF_SHIP_COPY				(1 << 18)	// this ship is a copy of another ship in the table -- meaningful for scoring and possible other things
#define	SIF_IN_TECH_DATABASE		(1 << 19)	// is ship type to be listed in the tech database?
#define	SIF_IN_TECH_DATABASE_M		(1 << 20)	// is ship type to be listed in the tech database for multiplayer?

#define	SIF_STEALTH					(1 << 21)	// the ship has stealth capabilities
#define	SIF_SUPERCAP				(1 << 22)	// the ship is a supercap
#define	SIF_DRYDOCK					(1 << 23)	// the ship is a drydock
#define	SIF_SHIP_CLASS_DONT_COLLIDE_INVIS	(1 << 24)	// Don't collide with this ship's invisible polygons

#define	SIF_BIG_DAMAGE				(1 << 25)	// this ship is classified as a big damage ship
#define	SIF_HAS_AWACS				(1 << 26)	// ship has an awacs subsystem

#define	SIF_CORVETTE				(1 << 27)	// corvette class (currently this only means anything for briefing icons)
#define	SIF_GAS_MINER				(1 << 28)	// also just for briefing icons
#define	SIF_AWACS					(1 << 29)	// ditto

#define	SIF_KNOSSOS_DEVICE			(1 << 30)	// this is the knossos device

#define	SIF_NO_FRED					(1 << 31)	// not available in fred

// flags2 list. If this is updated MAX_SHIP_FLAGS must also be updated!
#define SIF2_DEFAULT_IN_TECH_DATABASE		(1 << 0)	// default in tech database - Goober5000
#define SIF2_DEFAULT_IN_TECH_DATABASE_M		(1 << 1)	// ditto - Goober5000
#define SIF2_FLASH							(1 << 2)	// makes a flash when it explodes
#define SIF2_SHOW_SHIP_MODEL				(1 << 3)	// Show ship model even in first person view
#define SIF2_SURFACE_SHIELDS                (1 << 4)    // _argv[-1], 16 Jan 2005: Enable surface shields for this ship.
#define SIF2_GENERATE_HUD_ICON				(1 << 5)	// Enable generation of a HUD shield icon
#define SIF2_DISABLE_WEAPON_DAMAGE_SCALING	(1 << 6)	// WMC - Disable weapon scaling based on flags
#define SIF2_GUN_CONVERGENCE				(1 << 7)	// WMC - Gun convergence based on model weapon norms.
#define SIF2_NO_THRUSTER_GEO_NOISE			(1 << 8)	// Echelon9 - No thruster geometry noise.
#define SIF2_INTRINSIC_NO_SHIELDS			(1 << 9)	// Chief - disables shields for this ship even without No Shields in mission.
#define SIF2_NO_PRIMARY_LINKING				(1 << 10)	// Chief - slated for 3.7 originally, but this looks pretty simple to implement.
#define SIF2_NO_PAIN_FLASH					(1 << 11)	// The E - disable red pain flash
#define SIF2_ALLOW_LANDINGS					(1 << 12)	// SUSHI: Automatically set if any subsystems allow landings (as a shortcut)
#define SIF2_NO_ETS							(1 << 13)	// The E - No ETS on this ship class
// !!! IF YOU ADD A FLAG HERE BUMP MAX_SHIP_FLAGS !!!
#define	MAX_SHIP_FLAGS	14		//	Number of distinct flags for flags field in ship_info struct
#define	SIF_DEFAULT_VALUE		0
#define SIF2_DEFAULT_VALUE		0

#define	SIF_ALL_SHIP_TYPES		(SIF_CARGO | SIF_FIGHTER | SIF_BOMBER | SIF_CRUISER | SIF_FREIGHTER | SIF_CAPITAL | SIF_TRANSPORT | SIF_SUPPORT | SIF_NO_SHIP_TYPE | SIF_NAVBUOY | SIF_SENTRYGUN | SIF_ESCAPEPOD | SIF_SUPERCAP | SIF_CORVETTE | SIF_GAS_MINER | SIF_AWACS | SIF_KNOSSOS_DEVICE)
#define	SIF_SMALL_SHIP				(SIF_FIGHTER | SIF_BOMBER | SIF_SUPPORT | SIF_ESCAPEPOD )
#define	SIF_BIG_SHIP				(SIF_CRUISER | SIF_FREIGHTER | SIF_TRANSPORT | SIF_CORVETTE | SIF_GAS_MINER | SIF_AWACS)
#define	SIF_HUGE_SHIP				(SIF_CAPITAL | SIF_SUPERCAP | SIF_DRYDOCK | SIF_KNOSSOS_DEVICE)
#define	SIF_NOT_FLYABLE			(SIF_CARGO | SIF_NAVBUOY | SIF_SENTRYGUN)		// AL 11-24-97: this useful to know for targeting reasons
#define	SIF_HARMLESS				(SIF_CARGO | SIF_NAVBUOY | SIF_ESCAPEPOD)		// AL 12-3-97: ships that are not a threat
// for ships of this type, we make beam weapons miss a little bit otherwise they'd be way too powerful
#define	SIF_BEAM_JITTER			(SIF_CARGO | SIF_FIGHTER | SIF_BOMBER | SIF_FREIGHTER | SIF_TRANSPORT | SIF_SENTRYGUN | SIF_NAVBUOY | SIF_ESCAPEPOD)

// masks for preventing only non flag entry SIF flags from being cleared
#define SIF_MASK				SIF_AFTERBURNER
#define SIF2_MASK				0

#define REGULAR_WEAPON	(1<<0)
#define DOGFIGHT_WEAPON (1<<1)

#define AIM_FLAG_AUTOAIM				(1 << 0)	// has autoaim
#define AIM_FLAG_AUTO_CONVERGENCE		(1 << 1)	// has automatic convergence
#define AIM_FLAG_STD_CONVERGENCE		(1 << 2)	// has standard - ie. non-automatic - convergence
#define AIM_FLAG_AUTOAIM_CONVERGENCE	(1 << 3)	// has autoaim with convergence

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

#define STI_MSG_COUNTS_FOR_ALONE		(1<<0)
#define STI_MSG_PRAISE_DESTRUCTION		(1<<1)

#define STI_HUD_HOTKEY_ON_LIST			(1<<0)
#define STI_HUD_TARGET_AS_THREAT		(1<<1)
#define STI_HUD_SHOW_ATTACK_DIRECTION	(1<<2)
#define STI_HUD_NO_CLASS_DISPLAY		(1<<3)

#define STI_SHIP_SCANNABLE				(1<<0)
#define STI_SHIP_WARP_PUSHES			(1<<1)
#define STI_SHIP_WARP_PUSHABLE			(1<<2)
#define STI_TURRET_TGT_SHIP_TGT			(1<<3)

#define STI_WEAP_BEAMS_EASILY_HIT		(1<<0)
#define STI_WEAP_NO_HUGE_IMPACT_EFF		(1<<1)

#define STI_AI_ACCEPT_PLAYER_ORDERS		(1<<0)
#define STI_AI_AUTO_ATTACKS				(1<<1)
#define STI_AI_ATTEMPT_BROADSIDE		(1<<2)
#define STI_AI_GUARDS_ATTACK			(1<<3)
#define STI_AI_TURRETS_ATTACK			(1<<4)
#define STI_AI_CAN_FORM_WING			(1<<5)
#define STI_AI_PROTECTED_ON_CRIPPLE		(1<<6)

typedef struct ship_type_info {
	char name[NAME_LENGTH];

	//Messaging?
	int message_bools;

	//HUD
	int hud_bools;

	//Ship
	int ship_bools;	//For lack of better term
	float debris_max_speed;

	//Weapons
	int weapon_bools;
	float ff_multiplier;
	float emp_multiplier;

	//Fog
	float fog_start_dist;
	float fog_complete_dist;

	//AI
	int	ai_valid_goals;
	int ai_player_orders;
	int ai_bools;
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
		: message_bools( 0 ), hud_bools( 0 ), ship_bools( 0 ), weapon_bools( 0 ),
		  debris_max_speed( 0.f ), ff_multiplier( 0.f ), emp_multiplier( 0.f ),
		  fog_start_dist( 0.f ), fog_complete_dist( 0.f ),
		  ai_valid_goals( 0 ), ai_player_orders( 0 ), ai_bools( 0 ), ai_active_dock( 0 ), ai_passive_dock( 0 ),
		  vaporize_chance( 0.f )

	{
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

#define MT_BANK_RIGHT		(1<<0)
#define MT_BANK_LEFT		(1<<1)
#define MT_PITCH_UP			(1<<2)
#define MT_PITCH_DOWN		(1<<3)
#define MT_ROLL_RIGHT		(1<<4)
#define MT_ROLL_LEFT		(1<<5)
#define MT_SLIDE_RIGHT		(1<<6)
#define MT_SLIDE_LEFT		(1<<7)
#define MT_SLIDE_UP			(1<<8)
#define MT_SLIDE_DOWN		(1<<9)
#define MT_FORWARD			(1<<10)
#define MT_REVERSE			(1<<11)

typedef struct man_thruster {
	int use_flags;

	int start_snd;
	int loop_snd;
	int stop_snd;

	int tex_id;
	int tex_nframes;
	int tex_fps;
	float length;
	float radius;

	vec3d pos, norm;
	man_thruster()
		: use_flags( 0 ), tex_nframes( 0 ), tex_fps( 0 ), length( 0. ), radius( 0. )
	{
		tex_id=-1;
		start_snd=-1;
		loop_snd=-1;
		stop_snd=-1;
		memset( &pos, 0, sizeof( vec3d ) );
		memset( &norm, 0, sizeof( vec3d ) );
	}
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

// The real FreeSpace ship_info struct.
typedef struct ship_info {
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
	vec3d	afterburner_max_vel;				//	max velocity of the ship in the linear directions when afterburners are engaged -- read from ships.tbl
	vec3d	max_rotvel;							// maximum rotational velocity
	vec3d	rotation_time;						// time to rotate in x/y/z dimension traveling at max rotvel
	float		srotation_time;					//	scalar, computed at runtime as (rotation_time.x + rotation_time.y)/2
	float		max_rear_vel;						// max speed ship can go backwards.
	float		forward_accel;
	float		afterburner_forward_accel;		// forward acceleration with afterburner engaged
	float		forward_decel;
	float		slide_accel;
	float		slide_decel;

	char		warpin_anim[MAX_FILENAME_LEN];
	float		warpin_radius;
	int			warpin_snd_start;
	int			warpin_snd_end;
	float		warpin_speed;
	int			warpin_time;	//in ms
	int			warpin_type;

	char		warpout_anim[MAX_FILENAME_LEN];
	float		warpout_radius;
	int			warpout_snd_start;
	int			warpout_snd_end;
	float		warpout_speed;
	int			warpout_time;	//in ms
	int			warpout_type;

	float		warpout_player_speed;

	int		flags;							//	See SIF_xxxx - changed to uint by Goober5000, changed back by Zacam
	int		flags2;							//	See SIF2_xxxx - added by Goober5000, changed by Zacam
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
	float		afterburner_fuel_capacity;		// maximum afterburner fuel that can be stored
	float		afterburner_burn_rate;			// rate in fuel/second that afterburner consumes fuel
	float		afterburner_recover_rate;		//	rate in fuel/second that afterburner recovers fuel
	//SparK: reverse afterburner
	float		afterburner_max_reverse_vel;
	float		afterburner_reverse_accel;

	int		cmeasure_type;						// Type of countermeasures this ship carries
	int		cmeasure_max;						//	Number of charges of countermeasures this ship can hold.

	int num_primary_banks;										// Actual number of primary banks (property of model)
	int num_secondary_banks;									//	Actual number of secondary banks (property of model)
	int primary_bank_weapons[MAX_SHIP_PRIMARY_BANKS];			// Weapon_info[] index for the weapon in the bank
	
	// Goober5000's ballistic conversion
	int primary_bank_ammo_capacity[MAX_SHIP_PRIMARY_BANKS];	// Capacity of primary ballistic bank
	
	int secondary_bank_weapons[MAX_SHIP_SECONDARY_BANKS];	// Weapon_info[] index for the weapon in the bank
	int secondary_bank_ammo_capacity[MAX_SHIP_SECONDARY_BANKS];	// Capacity of bank (not number of missiles)

	float	max_hull_strength;				// Max hull strength of this class of ship.
	float	max_shield_strength;

	float	hull_repair_rate;				//How much of the hull is repaired every second
	float	subsys_repair_rate;		//How fast 

	float	sup_hull_repair_rate;
	float	sup_shield_repair_rate;
	float	sup_subsys_repair_rate;

	int engine_snd;							// handle to engine sound for ship (-1 if no engine sound)

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
	float		thruster01_glow_rad_factor;
	float		thruster02_glow_rad_factor;
	float		thruster03_glow_rad_factor;
	float		thruster02_glow_len_factor;

	int splodeing_texture;
	char splodeing_texture_name[MAX_FILENAME_LEN];

	bool draw_primary_models[MAX_SHIP_PRIMARY_BANKS];
	bool draw_secondary_models[MAX_SHIP_SECONDARY_BANKS];
	bool draw_models; //any weapon mode will be drawn
	float weapon_model_draw_distance;
	
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

	int num_maneuvering;
	man_thruster maneuvering[MAX_MAN_THRUSTERS];

	int radar_image_2d_idx;
	int radar_image_size;
	float radar_projection_size_mult;

	int ship_iff_info[MAX_IFFS][MAX_IFFS];

	int aiming_flags;
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
} ship_info;

extern int Num_wings;
extern ship Ships[MAX_SHIPS];
extern ship	*Player_ship;

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
	
	engine_wash_info();
} engine_wash_info;

extern SCP_vector<engine_wash_info> Engine_wash_info;

// flags defined for wings
#define MAX_WING_FLAGS				8				// total number of flags in the wing structure -- used for parsing wing flags
#define WF_WING_GONE					(1<<0)		// all ships were either destroyed or departed
#define WF_WING_DEPARTING			(1<<1)		// wing's departure cue turned true
#define WF_IGNORE_COUNT				(1<<2)		// ignore all ships in this wing for goal counting purposes.
#define WF_REINFORCEMENT			(1<<3)		// is this wing a reinforcement wing
#define WF_RESET_REINFORCEMENT	(1<<4)		// needed when we need to reset the wing's reinforcement flag (after calling it in)
#define WF_NO_ARRIVAL_MUSIC		(1<<5)		// don't play arrival music when wing arrives
#define WF_EXPANDED					(1<<6)		// wing expanded in hotkey select screen
#define WF_NO_ARRIVAL_MESSAGE		(1<<7)		// don't play any arrival message
#define WF_NO_ARRIVAL_WARP			(1<<8)		// don't play warp effect for any arriving ships in this wing.
#define WF_NO_DEPARTURE_WARP		(1<<9)		// don't play warp effect for any departing ships in this wing.
#define WF_NO_DYNAMIC				(1<<10)		// members of this wing relentlessly pursue their ai goals
#define WF_DEPARTURE_ORDERED		(1<<11)		// departure of this wing was ordered by player
#define WF_NEVER_EXISTED			(1<<12)		// this wing never existed because something prevented it from being created (like its mother ship being destroyed)
#define WF_NAV_CARRY				(1<<13)		// Kazan - Wing has nav-carry-status

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

	int flags;

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

//Template stuff, here's as good a place as any.
int parse_ship_values(ship_info* sip, bool isTemplate, bool first_time, bool replace);
extern int ship_template_lookup(char *name = NULL);
void parse_ship_particle_effect(ship_info* sip, particle_effect* pe, char *id_string);

extern int ship_info_lookup(char *name = NULL);
extern int ship_name_lookup(char *name, int inc_players = 0);	// returns the index into Ship array of name
extern int ship_type_name_lookup(char *name);

extern int wing_lookup(char *name);

// returns 0 if no conflict, 1 if conflict, -1 on some kind of error with wing struct
extern int wing_has_conflicting_teams(int wing_index);

// next function takes optional second parameter which says to ignore the current count of ships
// in the wing -- used to tell is the wing exists or not, not whether it exists and has ships currently
// present.
extern int wing_name_lookup(char *name, int ignore_count = 0);

extern int Player_ship_class;

#define MAX_PLAYER_SHIP_CHOICES	15
/*
extern int Num_player_ship_precedence;				// Number of ship types in Player_ship_precedence
extern int Player_ship_precedence[MAX_PLAYER_SHIP_CHOICES];	// Array of ship types, precedence list for player ship/wing selection
*/

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
extern void ship_get_eye( vec3d *eye_pos, matrix *eye_orient, object *obj, bool do_slew = true );		// returns in eye the correct viewing position for the given object
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

extern object *ship_find_repair_ship( object *requester );
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
extern int get_quadrant(vec3d *hit_pnt);						//	Return quadrant num of last hit ponit.

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
int ship_get_SIF(ship *shipp);
int ship_get_SIF(int sh);

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

int is_support_allowed(object *objp);

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

// given a ship info type, return a species
int ship_get_species_by_type(int ship_info_index);

// return the length of the ship class
float ship_class_get_length(ship_info *sip);

// Goober5000 - used by change-ai-class
extern void ship_set_new_ai_class(int ship_num, int new_ai_class);
extern void ship_subsystem_set_new_ai_class(int ship_num, char *subsystem, int new_ai_class);

// wing squad logos - Goober5000
extern void wing_load_squad_bitmap(wing *w);

// Goober5000 - needed by new hangar depart code
extern int ship_has_dock_bay(int shipnum);

// Goober5000 - needed by new hangar depart code
extern int ship_get_ship_with_dock_bay(int team);

// Goober5000 - moved here from hudbrackets.cpp
extern int ship_subsys_is_fighterbay(ship_subsys *ss);

// Goober5000
extern int ship_fighterbays_all_destroyed(ship *shipp);

// Goober5000
extern int ship_subsys_takes_damage(ship_subsys *ss);

//phreak
extern int ship_fire_tertiary(object *objp);

// Goober5000 - handles submodel rotation, incorporating conditions such as gun barrels when firing
extern void ship_do_submodel_rotation(ship *shipp, model_subsystem *psub, ship_subsys *pss);

// Goober5000 - shortcut hud stuff
extern int ship_has_energy_weapons(ship *shipp);
extern int ship_has_engine_power(ship *shipp);

// Swifty - Cockpit displays
void ship_init_cockpit_displays(ship *shipp);
void ship_add_cockpit_display(ship *shipp, cockpit_display_info *display, int cockpit_model_num);
void ship_clear_cockpit_displays(ship *shipp);
void ship_set_hud_cockpit_targets(ship *shipp);
void ship_clear_hud_cockpit_targets(ship *shipp);
void ship_render_backgrounds_cockpit_display(ship *shipp);
void ship_render_foregrounds_cockpit_display(ship *shipp);

//WMC - Warptype stuff
int warptype_match(char *p);

// Goober5000
int ship_starting_wing_lookup(char *wing_name);
int ship_squadron_wing_lookup(char *wing_name);
int ship_tvt_wing_lookup(char *wing_name);

// Goober5000
int ship_class_compare(int ship_class_1, int ship_class_2);

int armor_type_get_idx(char* name);

void armor_init();

int thruster_glow_anim_load(generic_anim *ga);

#endif
