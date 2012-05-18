/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MODEL_H
#define _MODEL_H

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"	// for NAME_LENGTH
#include "graphics/2d.h"
#include "object/object.h"

struct object;

extern flag_def_list model_render_flags[];
extern int model_render_flags_size;

#define MAX_DEBRIS_OBJECTS	32
#define MAX_MODEL_DETAIL_LEVELS	8
#define MAX_PROP_LEN			256
#define MAX_NAME_LEN			32
#define MAX_ARC_EFFECTS		8

#define MOVEMENT_TYPE_NONE				-1
#define MOVEMENT_TYPE_POS				0
#define MOVEMENT_TYPE_ROT				1
#define MOVEMENT_TYPE_ROT_SPECIAL	2		// for turrets only
#define MOVEMENT_TYPE_TRIGGERED			3	//triggered rotation


// DA 11/13/98 Reordered to account for difference between max and game
#define MOVEMENT_AXIS_NONE	-1
#define MOVEMENT_AXIS_X		0
#define MOVEMENT_AXIS_Y		2
#define MOVEMENT_AXIS_Z		1

// defines for special objects like gun and missile points, docking point, etc
// Hoffoss: Please make sure that subsystem NONE is always index 0, and UNKNOWN is
// always the last subsystem in the list.  Also, make sure that MAX is correct.
// Otherwise, problems will arise in Fred.

#define SUBSYSTEM_NONE				0
#define SUBSYSTEM_ENGINE			1
#define SUBSYSTEM_TURRET			2
#define SUBSYSTEM_RADAR				3
#define SUBSYSTEM_NAVIGATION		4
#define SUBSYSTEM_COMMUNICATION	5
#define SUBSYSTEM_WEAPONS			6
#define SUBSYSTEM_SENSORS			7
#define SUBSYSTEM_SOLAR				8
#define SUBSYSTEM_GAS_COLLECT		9
#define SUBSYSTEM_ACTIVATION		10
#define SUBSYSTEM_UNKNOWN			11
#define SUBSYSTEM_MAX				12				//	maximum value for subsystem_xxx, for error checking

// Goober5000
extern char *Subsystem_types[SUBSYSTEM_MAX];

#define MAX_TFP						10				// maximum number of turret firing points

#define MAX_SPLIT_PLANE				5				// number of artist specified split planes (used in big ship explosions)

// Data specific to a particular instance of a submodel.  This gets stuffed/unstuffed using
// the model_clear_instance, model_set_instance, model_get_instance functions.
typedef struct submodel_instance_info {
	int		blown_off;								// If set, this subobject is blown off
	angles	angs;										// The current angle this thing is turned to.
	angles	prev_angs;
	vec3d	pt_on_axis;								// in ship RF
	float		cur_turn_rate;
	float		desired_turn_rate;
	float		turn_accel;
	int		axis_set;
	int		step_zero_timestamp;		// timestamp determines when next step is to begin (for stepped rotation)
} submodel_instance_info;

typedef struct submodel_instance {
	angles angs;
	angles prev_angs;
	vec3d mc_base;
	matrix mc_orient;
	bool collision_checked;
	bool blown_off;
} submodel_instance;

// Data specific to a particular instance of a model.
typedef struct polymodel_instance {
	int model_num;					// global model num index, same as polymodel->id
	int root_submodel_num;			// unused?
	submodel_instance *submodel;	// array of submodel instances; mirrors the polymodel->submodel array
} polymodel_instance;

#define MAX_MODEL_SUBSYSTEMS		200				// used in ships.cpp (only place?) for local stack variable DTP; bumped to 200
													// when reading in ships.tbl

#define MSS_FLAG_ROTATES			(1 << 0)		// This means the object rotates automatically with "turn_rate"
#define MSS_FLAG_STEPPED_ROTATE		(1 << 1)		// This means that the rotation occurs in steps
#define MSS_FLAG_AI_ROTATE			(1 << 2)		// This means that the rotation is controlled by ai
#define MSS_FLAG_CREWPOINT			(1 << 3)		// If set, this is a crew point.
#define MSS_FLAG_TURRET_MATRIX		(1 << 4)		// If set, this has it's turret matrix created correctly.
#define MSS_FLAG_AWACS				(1 << 5)		// If set, this subsystem has AWACS capability
#define MSS_FLAG_ARTILLERY			(1 << 6)		// if this rotates when weapons are fired - Goober5000
#define MSS_FLAG_TRIGGERED			(1 << 7)		// rotates when triggered by something
#define MSS_FLAG_UNTARGETABLE		(1 << 8)		// Goober5000
#define MSS_FLAG_CARRY_NO_DAMAGE	(1 << 9)		// WMC
#define MSS_FLAG_USE_MULTIPLE_GUNS	(1 << 10)		// WMC
#define MSS_FLAG_FIRE_ON_NORMAL		(1 << 11)		// forces a turret to fire down its normal vecs
#define MSS_FLAG_TURRET_HULL_CHECK	(1 << 12)		// makes the turret check to see if it's going to shoot through it's own hull before fireing - Bobboau
#define MSS_FLAG_TURRET_FIXED_FP	(1 << 13)		// forces turret (when defined with multiple weapons) to prevent the firepoints from alternating
#define MSS_FLAG_TURRET_SALVO		(1 << 14)		// forces turret to fire salvos (all guns simultaneously) - independent targeting
#define MSS_FLAG_FIRE_ON_TARGET		(1 << 15)		// prevents turret from firing unless it is pointing at the firingpoints are pointing at the target
#define MSS_FLAG_NO_SS_TARGETING	(1 << 16)		// toggles the subsystem targeting for the turret
#define MSS_FLAG_TURRET_RESET_IDLE	(1 << 17)		// makes turret reset to their initial position if the target is out of field of view
#define MSS_FLAG_TURRET_ALT_MATH	(1 << 18)		// tells the game to use additional calculations should turret have a defined y fov
#define MSS_FLAG_DUM_ROTATES		(1 << 19)		// Bobboau
#define MSS_FLAG_CARRY_SHOCKWAVE	(1 << 20)		// subsystem - even with 'carry no damage' flag - will carry shockwave damage to the hull
#define MSS_FLAG_ALLOW_LANDING		(1 << 21)		// This subsystem can be landed on
#define MSS_FLAG_FOV_EDGE_CHECK		(1 << 22)		// Tells the game to use better FOV edge checking with this turret
#define MSS_FLAG_FOV_REQUIRED		(1 << 23)		// Tells game not to allow this turret to attempt targeting objects out of FOV
#define MSS_FLAG_NO_REPLACE			(1 << 24)		// set the subsys not to draw replacement ('destroyed') model
#define MSS_FLAG_NO_LIVE_DEBRIS		(1 << 25)		// sets the subsys not to release live debris
#define MSS_FLAG_IGNORE_IF_DEAD		(1 << 26)		// tells homing missiles to ignore the subsys if its dead and home on to hull instead of earlier subsys pos
#define MSS_FLAG_ALLOW_VANISHING	(1 << 27)		// allows subsystem to vanish (prevents explosions & sounds effects from being played)
#define MSS_FLAG_DAMAGE_AS_HULL		(1 << 28)		// applies armor damage to subsystem instead of subsystem damage - FUBAR
#define MSS_FLAG_TURRET_LOCKED      (1 << 29)       // Turret starts locked by default - Sushi
#define MSS_FLAG_NO_AGGREGATE		(1 << 30)		// Don't include with aggregate subsystem types - Goober5000
#define MSS_FLAG_TURRET_ANIM_WAIT   (1 << 31)		// Turret won't fire until animation is complete - Sushi

#define MSS_FLAG2_PLAYER_TURRET_SOUND			 (1 << 0)
#define MSS_FLAG2_TURRET_ONLY_TARGET_IF_CAN_FIRE (1 << 1)	// Turrets only target things they're allowed to shoot at (e.g. if check-hull fails, won't keep targeting)
#define MSS_FLAG2_NO_DISAPPEAR					 (1 << 2)	// Submodel won't disappear when subsystem destroyed
#define MSS_FLAG2_COLLIDE_SUBMODEL				 (1 << 3)	// subsystem takes damage only from hits which impact the associated submodel

#define NUM_SUBSYSTEM_FLAGS			33

// definition of stepped rotation struct
typedef struct stepped_rotation {
	int num_steps;				// number of steps in complete revolution
	float fraction;			// fraction of time in step spent in accel
	float t_transit;			// time spent moving from one step to next
	float t_pause;				// time at rest between steps
	float max_turn_rate;		// max turn rate going betweens steps
	float max_turn_accel;	// max accel going between steps
} stepped_rotation_t;

struct queued_animation;

// definition for model subsystems.
typedef struct model_subsystem {					/* contains rotation rate info */

	uint	flags;								// See MSS_FLAG_* defines above
	uint	flags2;
	char	name[MAX_NAME_LEN];					// name of the subsystem.  Probably displayed on HUD
	char	subobj_name[MAX_NAME_LEN];			// Temporary (hopefully) parameter used to match stuff in ships.tbl
	char	alt_sub_name[NAME_LENGTH];			// Karajorma - Name that overrides name of original
	char	alt_dmg_sub_name[NAME_LENGTH];		// Name for the damage popup subsystems, allows for translation
	int		subobj_num;							// subobject number (from bspgen) -- used to match subobjects of subsystems to these entries; index to polymodel->submodel
	int		model_num;							// Which model this is attached to (i.e. the polymodel[] index); same as polymodel->id
	int		type;								// type. see SUBSYSTEM_* types above.  A generic type thing
	vec3d	pnt;								// center point of this subsystem
	float	radius;								// the extent of the subsystem
	float	max_subsys_strength;				// maximum hits of this subsystem
	int		armor_type_idx;						// Armor type on teh subsystem -C

	//	The following items are specific to turrets and will probably be moved to
	//	a separate struct so they don't take up space for all subsystem types.
	char	crewspot[MAX_NAME_LEN];				// unique identifying name for this turret -- used to assign AI class and multiplayer people
	vec3d	turret_norm;						//	direction this turret faces
	matrix	turret_matrix;						// turret_norm converted to a matrix.
	float	turret_fov;							//	dot of turret_norm:vec_to_enemy > this means can see
	float	turret_max_fov;						//  dot of turret_norm:vec_to_enemy <= this means barrels can elevate up to the target
	float	turret_y_fov;						//  turret's base's fov
	int		turret_num_firing_points;			// number of firing points on this turret
	vec3d	turret_firing_point[MAX_TFP];		//	in parent object's reference frame, point from which to fire.
	int		turret_gun_sobj;					// Which subobject in this model the firing points are linked to.
	float	turret_turning_rate;				// How fast the turret turns. Read from ships.tbl
	int		turret_base_rotation_snd;				// Sound to make when the turret moves
	float	turret_base_rotation_snd_mult;			// Volume multiplier for the turret sounds
	int		turret_gun_rotation_snd;				// Sound to make when the turret moves
	float	turret_gun_rotation_snd_mult;			// Volume multiplier for the turret sounds


	//Sound stuff
	int		alive_snd;		//Sound to make while the subsystem is not-dead
	int		dead_snd;		//Sound to make when the subsystem is dead.
	int		rotation_snd;	//Sound to make when the subsystem is rotating. (ie turrets)

	// engine wash info
	struct engine_wash_info		*engine_wash_pointer;					// index into Engine_wash_info

	// rotation specific info
	float		turn_rate;							// The turning rate of this subobject, if MSS_FLAG_ROTATES is set.
	int			weapon_rotation_pbank;				// weapon-controlled rotation - Goober5000
	stepped_rotation_t	*stepped_rotation;			// turn rotation struct

	// AWACS specific information
	float		awacs_intensity;						// awacs intensity of this subsystem
	float		awacs_radius;							// radius of effect of the AWACS

	int		primary_banks[MAX_SHIP_PRIMARY_BANKS];					// default primary weapons -hoffoss
	int		primary_bank_capacity[MAX_SHIP_PRIMARY_BANKS];		// capacity of a bank - Goober5000
	int		secondary_banks[MAX_SHIP_SECONDARY_BANKS];				// default secondary weapons -hoffoss
	int		secondary_bank_capacity[MAX_SHIP_SECONDARY_BANKS];	// capacity of a bank -hoffoss
	int		path_num;								// path index into polymodel .paths array.  -2 if none exists, -1 if not defined

	int n_triggers;
	queued_animation *triggers;		//all the triggered animations associated with this object

	int		turret_reset_delay;

	// target priority setting for turrets
	int      target_priority[32];
	int      num_target_priorities;

	float	optimum_range;
	float	favor_current_facing;

	float	turret_rof_scaler;

	//Per-turret ownage settings - SUSHI
	int turret_max_bomb_ownage; 
	int turret_max_target_ownage; 
} model_subsystem;

typedef struct model_special {
	struct	model_special *next, *prev;		// for using as a linked list
	int		bank;										// used for sequencing gun/missile backs. approach/docking points
	int		slot;										// location for gun or missile in this bank
	vec3d	pnt;										// point where this special submodel thingy is at
	vec3d	norm;										// normal for the special submodel thingy
} model_special;

// model arc types
#define MARC_TYPE_NORMAL					0		// standard freespace 1 blue lightning arcs
#define MARC_TYPE_EMP						1		// EMP blast type arcs

#define MAX_LIVE_DEBRIS	7


// IBX stuff
typedef struct IBX {
	CFILE *read;		// reads, if an IBX file already exists
	CFILE *write;		// writes, if new file created
	int size;			// file size used to make sure an IBX contains enough data for the whole model
	int version;		// IBX file version to use: v1 is USHORT only, v2 can mix USHORT and UINT
	char name[MAX_FILENAME_LEN];	// filename of the ibx, this is used in case a safety check fails and we delete the file
} IBX;



typedef struct bsp_info {
	char		name[MAX_NAME_LEN];	// name of the subsystem.  Probably displayed on HUD
	int		movement_type;			// -1 if no movement, otherwise rotational or positional movement -- subobjects only
	int		movement_axis;			// which axis this subobject moves or rotates on.
	bool	can_move;				// If true, the position and/or orientation of this submodel can change due to rotation of itself OR a parent

	vec3d	offset;					// 3d offset from parent object
	matrix	orientation;			// 3d orientation relative to parent object

	int		bsp_data_size;
	ubyte		*bsp_data;

	vec3d	geometric_center;		// geometric center of this subobject.  In the same Frame Of 
	                              //  Reference as all other vertices in this submodel. (Relative to pivot point)
	float		rad;						// radius for each submodel

	vec3d	min;						// The min point of this object's geometry
	vec3d	max;						// The max point of this object's geometry
	vec3d	bounding_box[8];		// caclulated fron min/max

	int		blown_off;				// If set, this subobject is blown off. Stuffed by model_set_instance
	int		my_replacement;		// If not -1 this subobject is what should get rendered instead of this one
	int		i_replace;				// If this is not -1, then this subobject will replace i_replace when it is damaged
	angles	angs;					// The rotation angles of this subobject (Within its own orientation, NOT relative to parent - KeldorKatarn)

	int		is_live_debris;		// whether current submodel is a live debris model
	int		num_live_debris;		// num live debris models assocaiated with a submodel
	int		live_debris[MAX_LIVE_DEBRIS];	// array of live debris submodels for a submodel

	submodel_instance_info	*sii;	// stuff needed for collision from rotations

	int		is_thruster;
	int		is_damaged;

	// Tree info
	int		parent;					// what is parent for each submodel, -1 if none
	int		num_children;			// How many children this model has
	int		first_child;			// The first_child of this model, -1 if none
	int		next_sibling;			// This submodel's next sibling, -1 if none

	int		num_details;			// How many submodels are lower detail "mirrors" of this submodel
	int		details[MAX_MODEL_DETAIL_LEVELS];		// A list of all the lower detail "mirrors" of this submodel

	// Electrical Arc Effect Info
	// Sets a spark for this submodel between vertex v1 and v2	
	int		num_arcs;												// See model_add_arc for more info	
	vec3d	arc_pts[MAX_ARC_EFFECTS][2];	
	ubyte		arc_type[MAX_ARC_EFFECTS];							// see MARC_TYPE_* defines
	
	// buffers used by HT&L
	vertex_buffer buffer;

	vec3d	render_box_min;
	vec3d	render_box_max;
	float	render_sphere_radius;
	vec3d	render_sphere_offset;
	int		use_render_box;			// 0==do nothing, 1==only render this object if you are inside the box, -1==only if you're outside
	int		use_render_sphere;		// 0==do nothing, 1==only render this object if you are inside the sphere, -1==only if you're outside
	bool	gun_rotation;			// for animated weapon models
	bool	no_collisions;			// for $no_collisions property - kazan
	bool	nocollide_this_only;	//SUSHI: Like no_collisions, but not recursive. For the "replacement" collision model scheme.
	bool	collide_invisible;		//SUSHI: If set, this submodel should allow collisions for invisible textures. For the "replacement" collision model scheme.
	bool	force_turret_normal;	//Wanderer: Sets the turret uvec to override any input of for turret normal.
	char	lod_name[MAX_NAME_LEN];	//FUBAR:  Name to be used for LOD naming comparison to preserve compatibility with older tables.  Only used on LOD0 
	bool	attach_thrusters;		//zookeeper: If set and this submodel or any of its parents rotates, also rotates associated thrusters.
	float		dumb_turn_rate;

	/* If you've got a better way to do this, please implement it! */
	void Reset( )
	{
		name[ 0 ] = '\0';
		movement_type = 0;
		movement_axis = 0;
		can_move = false;
		
		bsp_data_size = 0;
		blown_off = 0;
		my_replacement = 0;
		i_replace = 0;
		is_live_debris = 0;
		num_live_debris = 0;
		sii = NULL;
		is_thruster = 0;
		is_damaged = 0;
		parent = 0;
		num_children = 0;
		first_child = 0;
		next_sibling = 0;
		num_details = 0;
		num_arcs = 0;
		render_sphere_radius = 0;
		use_render_box = 0;
		use_render_sphere = 0;
		gun_rotation = false;
		no_collisions = false;
		nocollide_this_only = false;
		collide_invisible = false;
		force_turret_normal = false;
		dumb_turn_rate = 0.f;
		bsp_data = NULL;
		rad = 0.f;
		lod_name[ 0 ] = '\0';
		attach_thrusters = false;

		/* Compound types */
		memset( live_debris, 0, sizeof( live_debris ) );
		memset( details, 0, sizeof( details ) );
		memset( &geometric_center, 0, sizeof( geometric_center ) );
		memset( &offset, 0, sizeof( offset ) );
		memset( &orientation, 0, sizeof( orientation ) );
		memset( &min, 0, sizeof( min ) );
		memset( &max, 0, sizeof( max ) );
		memset( bounding_box, 0, sizeof( bounding_box ) );
		memset( &angs, 0, sizeof( angs ) );
		memset( arc_pts, 0, sizeof( arc_pts ) );
		memset( arc_type, 0, sizeof( arc_type ) );
		memset( &render_box_min, 0, sizeof( render_box_min ) );
		memset( &render_box_max, 0, sizeof( render_box_max ) );
		memset( &render_sphere_offset, 0, sizeof( render_sphere_offset ) );

		buffer.clear( );
	}
} bsp_info;

void parse_triggersint(int &n_trig, queued_animation **triggers, char *props);

#define MP_TYPE_UNUSED 0
#define MP_TYPE_SUBSYS 1

typedef struct mp_vert {
	vec3d		pos;				// xyz coordinates of vertex in object's frame of reference
	int			nturrets;		// number of turrets guarding this vertex
	int			*turret_ids;	// array of indices into ship_subsys linked list (can't index using [] though)
	float			radius;			// How far the closest obstruction is from this vertex
} mp_vert;

typedef struct model_path {
	char			name[MAX_NAME_LEN];					// name of the subsystem.  Probably displayed on HUD
	char			parent_name[MAX_NAME_LEN];			// parent name of submodel that path is linked to in POF
	int			parent_submodel;
	int			nverts;
	mp_vert		*verts;
	int			goal;			// Which of the verts is the one closest to the goal of this path
	int			type;			// What this path takes you to... See MP_TYPE_??? defines above for details
	int			value;		// This depends on the type.
									// For MP_TYPE_UNUSED, this means nothing.
									// For MP_TYPE_SUBSYS, this is the subsystem number this path takes you to.
} model_path;

typedef struct model_tmap_vert {
	ushort vertnum;
	ushort normnum;
	float u,v;
} model_tmap_vert;

// info for gun and missile banks.  Also used for docking points.  There should always
// only be two slots for each docking bay

#define MAX_SLOTS		25

typedef struct w_bank {
	int		num_slots;
	vec3d	pnt[MAX_SLOTS];
	vec3d	norm[MAX_SLOTS];
	float		radius[MAX_SLOTS];
} w_bank;

struct glow_point{
	vec3d	pnt;
	vec3d	norm;
	float	radius;
};

typedef struct thruster_bank {
	int		num_points;
	glow_point *points;

	// Engine wash info
	struct engine_wash_info	*wash_info_pointer;		// index into Engine_wash_info
	int		obj_num;		// what subsystem number this bank is on; index to ship_info->subsystems
	int		submodel_num;	// what submodel number this bank is on; index to polymodel->submodel/polymodel_instance->submodel
} thruster_bank;

typedef struct glow_point_bank {  // glow bank structure -Bobboau
	int			type;
	int			glow_timestamp; 
	int			on_time; 
	int			off_time; 
	int			disp_time; 
	int			is_on; 
	int			is_active; 
	int			submodel_parent; 
	int			LOD; 
	int			num_points; 
	glow_point	*points;
	int			glow_bitmap; 
	int			glow_neb_bitmap; 
} glow_point_bank;

// defines for docking bay things.  The types are essentially flags since docking bays can probably
// be used for multiple things in some cases (i.e. rearming and general docking)
//WMC - IMPORTANT, update Dock_type_names array if you add a new one of these
extern flag_def_list Dock_type_names[];
extern int Num_dock_type_names;

#define DOCK_TYPE_CARGO				(1<<0)
#define DOCK_TYPE_REARM				(1<<1)
#define DOCK_TYPE_GENERIC			(1<<2)

#define MAX_DOCK_SLOTS	2

typedef struct dock_bay {
	int		num_slots;
	int		type_flags;					// indicates what this docking bay can be used for (i.e. cargo/rearm, etc)
	int		num_spline_paths;			// number of spline paths which lead to this docking bay
	int		*splines;					// array of indices into the Spline_path array
	char		name[MAX_NAME_LEN];		// name of this docking location
	vec3d	pnt[MAX_DOCK_SLOTS];
	vec3d	norm[MAX_DOCK_SLOTS];
} dock_bay;

// struct that holds the indicies into path information associated with a fighter bay on a capital ship
// NOTE: Fighter bay paths are identified by the path_name $bayN (where N is numbered from 1).
//			Capital ships only have ONE fighter bay on the entire ship
// NOTE: MAX_SHIP_BAY_PATHS cannot be bumped higher than 31 without rewriting the arrival/departure flag logic.
#define MAX_SHIP_BAY_PATHS		31
typedef struct ship_bay {
	int	num_paths;							// how many paths are associated with the model's fighter bay
	int	path_indexes[MAX_SHIP_BAY_PATHS];	// index into polymodel->paths[] array
	int	arrive_flags;	// bitfield, set to 1 when that path number is reserved for an arrival
	int	depart_flags;	// bitfield, set to 1 when that path number is reserved for a departure
} ship_bay_t;

// three structures now used for representing shields.
// shield_tri structure stores information concerning each face of the shield.
// verts indexes into the verts array in the higher level structure
// neighbors indexes into the tris array in the higher level structure
typedef struct shield_tri {
  int used;
  int verts[3];			// 3 indices into vertex list of the shield.  list found in shield_info struct
  int neighbors[3];		// indices into array of triangles. neighbor = shares edge.  list found in shield_info struct
  vec3d norm;				// norm of this triangle
} shield_tri;

// a list of these shield_vertex structures comprimises the vertex list of the shield.
// The verts array in the shield_tri structure points to one of these members
typedef struct shield_vertex {
	vec3d	pos;
	float		u,v;
} shield_vertex;

// the high level shield structure.  A ship without any shield has nverts and ntris set to 0.
// The vertex list and the tris list are used by the shield_tri structure
typedef struct shield_info {
	int				nverts;
	int				ntris;
	shield_vertex	*verts;
	shield_tri		*tris;
} shield_info;

#define BSP_LIGHT_TYPE_WEAPON 1
#define BSP_LIGHT_TYPE_THRUSTER 2

typedef struct bsp_light {
	vec3d			pos;
	int				type;		// See BSP_LIGHT_TYPE_?? for values
	float				value;	// How much to light up this light.  0-1.
} bsp_light;

// model_octant - There are 8 of these per model.  They are a handy way to categorize
// a lot of model properties to get some easy 8x optimizations for model stuff.
typedef struct model_octant {
	vec3d		min, max;				// The bounding box that makes up this octant defined as 2 points.
	int			nverts;					// how many vertices are in this octant
	vec3d		**verts;					// The vertices in this octant in the high-res hull.  A vertex can only be in one octant.
	int			nshield_tris;			// how many shield triangles are in the octant
	shield_tri	**shield_tris;			// the shield triangles that make up this octant. A tri could be in multiple octants.
} model_octant;

#define MAX_EYES	10

typedef struct eye {
	int		parent;			// parent's subobject number
	vec3d	pnt;				// the point for the eye
	vec3d	norm;				// direction the eye faces.  Not used with first eye since player orient is used
} eye;

typedef struct cross_section {
	float z;
	float radius;
} cross_section;

#define MAX_MODEL_INSIGNIAS		6
#define MAX_INS_FACE_VECS			3
#define MAX_INS_VECS					20
#define MAX_INS_FACES				10
typedef struct insignia {
	int detail_level;
	int num_faces;					
	int faces[MAX_INS_FACES][MAX_INS_FACE_VECS];		// indices into the vecs array	
	float u[MAX_INS_FACES][MAX_INS_FACE_VECS];		// u tex coords on a per-face-per-vertex basis
	float v[MAX_INS_FACES][MAX_INS_FACE_VECS];		// v tex coords on a per-face-per-vertex bases
	vec3d vecs[MAX_INS_VECS];								// vertex list	
	vec3d offset;	// global position offset for this insignia
	vec3d norm[MAX_INS_VECS]	;					//normal of the insignia-Bobboau
} insignia;

#define PM_FLAG_ALLOW_TILING		(1<<0)					// Allow texture tiling
#define PM_FLAG_AUTOCEN				(1<<1)					// contains autocentering info	

// Goober5000
class texture_info {
private:
	int original_texture;	// what gets read in from file
	int texture;			// what texture you draw with; reset to original_textures by model_set_instance

	//WMC - Removed unneeded struct and is_anim to clean this up.
	//If num_frames is < 2, it doesn't need to be treated like an animation.
	int num_frames;
	float total_time;		// in seconds
public:
	texture_info();
	texture_info(int bm_handle);
	void clear();

	int GetNumFrames();
	int GetOriginalTexture();
	int GetTexture();
	float GetTotalTime();

	int LoadTexture(char *filename, char *dbg_name);

	void PageIn();
	void PageOut(bool release);

	int ResetTexture();
	int SetTexture(int n_tex);
};

#define TM_BASE_TYPE		0		// the standard base map
#define TM_GLOW_TYPE		1		// optional glow map
#define TM_SPECULAR_TYPE	2		// optional specular map
#define TM_NORMAL_TYPE		3		// optional normal map
#define TM_HEIGHT_TYPE		4		// optional height map (for parallax mapping)
#define TM_MISC_TYPE		5		// optional utility map
#define TM_NUM_TYPES		6		//WMC - Number of texture_info objects in texture_map
									//Used by scripting - if you change this, do a search
									//to update switch() statement in lua.cpp
// taylor
//WMC - OOPified
class texture_map {
public:
	texture_info textures[TM_NUM_TYPES];

	bool is_ambient;
	bool is_transparent;
public:
	int FindTexture(int bm_handle);
	int FindTexture(char *name);

	void PageIn();
	void PageOut(bool release);

	void Reset();
};

#define MAX_REPLACEMENT_TEXTURES MAX_MODEL_TEXTURES * TM_NUM_TYPES

//used to describe a polygon model
typedef struct polymodel {
	int			id;				// what the polygon model number is.  (Index in Polygon_models)
	int			version;
	char			filename[FILESPEC_LENGTH];

	uint			flags;			// 1=allow tiling
	int			n_detail_levels;
	int			detail[MAX_MODEL_DETAIL_LEVELS];
	float			detail_depth[MAX_MODEL_DETAIL_LEVELS];

	int			num_debris_objects;
	int			debris_objects[MAX_DEBRIS_OBJECTS];

	int			n_models;

	vec3d		mins,maxs;							//min,max for whole model
	vec3d		bounding_box[8];

	int			num_lights;							// how many lights there are
	bsp_light *	lights;								// array of light info

	int			n_view_positions;					// number of viewing positions available on this ship
	eye			view_positions[MAX_EYES];		//viewing positions.  Default to {0,0,0}. in location 0

	vec3d		autocenter;							// valid only if PM_FLAG_AUTOCEN is set

	float			rad;									// The radius of everything in the model; shields, thrusters.
	float			core_radius;						// The radius to be used for collision detection in small ship vs big ships.
															// This is equal to 1/2 of the smallest dimension of the hull's bounding box.
	// texture maps for model
	int n_textures;
	texture_map	maps[MAX_MODEL_TEXTURES];
	
	bsp_info		*submodel;							// an array of size n_models of submodel info.

	// linked lists for special polygon types on this model.  Most ships I think will have most
	// of these.  (most ships however, probably won't have approach points).
	int			n_guns;								// number of primary gun points (not counting turrets)
	int			n_missiles;							// number of secondary missile points (not counting turrets)
	int			n_docks;								// number of docking points
	int			n_thrusters;						// number of thrusters on this ship.
	w_bank		*gun_banks;							// array of gun banks
	w_bank		*missile_banks;					// array of missile banks
	dock_bay		*docking_bays;						// array of docking point pairs
	thruster_bank		*thrusters;							// array of thruster objects -- likely to change in the future
	ship_bay_t		*ship_bay;							// contains path indexes for ship bay approach/depart paths

	shield_info	shield;								// new shield information
	ubyte	*shield_collision_tree;
	int		sldc_size;

	int			n_paths;
	model_path	*paths;

	// physics info
	float			mass;
	vec3d		center_of_mass;
	matrix		moment_of_inertia;
	
	model_octant	octants[8];

	int num_xc;				// number of cross sections
	cross_section* xc;	// pointer to array of cross sections (used in big ship explosions)

	int num_split_plane;	// number of split planes
	float split_plane[MAX_SPLIT_PLANE];	// actual split plane z coords (for big ship explosions)

	insignia		ins[MAX_MODEL_INSIGNIAS];
	int			num_ins;

#ifndef NDEBUG
	int			ram_used;		// How much RAM this model uses
	int			debug_info_size;
	char			*debug_info;
#endif

	int used_this_mission;		// used for page-in system, how many times this model has been loaded per mission - taylor

	int n_glow_point_banks;						// number of glow points on this ship. -Bobboau
	glow_point_bank *glow_point_banks;			// array of glow objects -Bobboau

	float gun_submodel_rotation;

	int vertex_buffer_id;			// HTL vertex buffer id
} polymodel;

// Call once to initialize the model system
void model_init();

// call to unload a model (works like bm_unload()), "force" SHOULD NEVER BE SET outside of modelread.cpp!!!!
void model_unload(int modelnum, int force = 0);

// Call to free all existing models
void model_free_all();
void model_instance_free_all();

// Loads a model from disk and returns the model number it loaded into.
int model_load(char *filename, int n_subsystems, model_subsystem *subsystems, int ferror = 1, int duplicate = 0);

int model_create_instance(int model_num, int submodel_num = -1);
void model_delete_instance(int model_instance_num);

// Goober5000
void model_load_texture(polymodel *pm, int i, char *file);

// Returns a pointer to the polymodel structure for model 'n'
polymodel *model_get(int model_num);

polymodel_instance* model_get_instance(int model_instance_num);

// routine to copy susbsystems.  Must be called when subsystems sets are the same -- see ship.cpp
void model_copy_subsystems(int n_subsystems, model_subsystem *d_sp, model_subsystem *s_sp);

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color(int r, int g, int b);

void model_set_outline_color_fast(void *outline_color);

// IF MR_LOCK_DETAIL is set, then it will always draw detail level 'n'
// This defaults to 0. (0=highest, larger=lower)
void model_set_detail_level(int n);

// Flags you can pass to model_render
#define MR_NORMAL					(0)			// Draw a normal object
#define MR_SHOW_OUTLINE				(1<<0)		// Draw the object in outline mode. Color specified by model_set_outline_color
#define MR_SHOW_PIVOTS				(1<<1)		// Show the pivot points
#define MR_SHOW_PATHS				(1<<2)		// Show the paths associated with a model
#define MR_SHOW_RADIUS				(1<<3)		// Show the radius around the object
#define MR_SHOW_SHIELDS				(1<<4)		// Show the shield mesh
#define MR_SHOW_THRUSTERS			(1<<5)		// Show the engine thrusters. See model_set_thrust for how long it draws.
#define MR_LOCK_DETAIL				(1<<6)		// Only draw the detail level defined in model_set_detail_level
#define MR_NO_POLYS					(1<<7)		// Don't draw the polygons.
#define MR_NO_LIGHTING				(1<<8)		// Don't perform any lighting on the model.
#define MR_NO_TEXTURING				(1<<9)		// Draw textures as flat-shaded polygons.
#define MR_NO_CORRECT				(1<<10)		// Don't to correct texture mapping
#define MR_NO_SMOOTHING				(1<<11)		// Don't perform smoothing on vertices.
#define MR_IS_ASTEROID				(1<<12)		// When set, treat this as an asteroid.  
#define MR_IS_MISSILE				(1<<13)		// When set, treat this as a missilie.  No lighting, small thrusters.
#define MR_SHOW_OUTLINE_PRESET		(1<<14)		// Draw the object in outline mode. Color assumed to be set already.	
#define MR_SHOW_INVISIBLE_FACES		(1<<15)		// Show invisible faces as green...
#define MR_AUTOCENTER				(1<<16)		// Always use the center of the hull bounding box as the center, instead of the pivot point
#define MR_BAY_PATHS				(1<<17)		// draw bay paths
#define MR_ALL_XPARENT				(1<<18)		// render it fully transparent
#define MR_NO_ZBUFFER				(1<<19)		// switch z-buffering off completely 
#define MR_NO_CULL					(1<<20)		// don't cull backfacing poly's
#define MR_FORCE_TEXTURE			(1<<21)		// force a given texture to always be used
#define MR_FORCE_LOWER_DETAIL		(1<<22)		// force the model to draw 1 LOD lower, if possible
#define MR_EDGE_ALPHA				(1<<23)		// makes norms that are faceing away from you render more transparent -Bobboau
#define MR_CENTER_ALPHA				(1<<24)		// oposite of above -Bobboau
#define MR_NO_FOGGING				(1<<25)		// Don't fog - taylor
#define MR_SHOW_OUTLINE_HTL			(1<<26)		// Show outlines (wireframe view) using HTL method
#define MR_NO_GLOWMAPS				(1<<27)		// disable rendering of glowmaps - taylor
#define MR_FULL_DETAIL				(1<<28)		// render all valid objects, particularly ones that are otherwise in/out of render boxes - taylor
#define MR_FORCE_CLAMP				(1<<29)		// force clamp - Hery
#define MR_ANIMATED_SHADER			(1<<30)		// Use a animated Shader - Valathil

// Renders a model and all it's submodels.
// See MR_? defines for values for flags
void model_render(int model_num, matrix *orient, vec3d * pos, uint flags = MR_NORMAL, int objnum = -1, int lighting_skip = -1, int *replacement_textures = NULL);

// Renders just one particular submodel on a model.
// See MR_? defines for values for flags
void submodel_render(int model_num,int submodel_num, matrix *orient, vec3d * pos, uint flags = MR_NORMAL, int objnum = -1, int *replacement_textures = NULL);

// Returns the radius of a model
float model_get_radius(int modelnum);
float submodel_get_radius(int modelnum, int submodelnum);

// Returns the core radius (smallest dimension of hull's bounding box, used for collision detection with big ships only)
float model_get_core_radius(int modelnum);

// Returns zero is x1,y1,x2,y2 are valid
// returns 1 for invalid model, 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
// This function just looks at the radius, and not the orientation, so the
// bounding box won't change depending on the obj's orient.
extern int model_find_2d_bound(int model_num,matrix *orient, vec3d * pos,int *x1, int *y1, int *x2, int *y2);

// Returns zero is x1,y1,x2,y2 are valid
// returns 1 for invalid model, 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
// This function looks at the object's bounding box and it's orientation,
// so the bounds will change as the object rotates, to give the minimum bouding
// rect.
extern int model_find_2d_bound_min(int model_num,matrix *orient, vec3d * pos,int *x1, int *y1, int *x2, int *y2);

// Returns zero is x1,y1,x2,y2 are valid
// returns 1 for invalid model, 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
// This function looks at the object's bounding box and it's orientation,
// so the bounds will change as the object rotates, to give the minimum bouding
// rect.
int submodel_find_2d_bound_min(int model_num,int submodel, matrix *orient, vec3d * pos,int *x1, int *y1, int *x2, int *y2);


// Returns zero is x1,y1,x2,y2 are valid
// Returns 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
// This function just looks at the radius, and not the orientation, so the
// bounding box won't change depending on the obj's orient.
int subobj_find_2d_bound(float radius, matrix *orient, vec3d * pos,int *x1, int *y1, int *x2, int *y2);

// stats variables
#ifndef NDEBUG
extern int modelstats_num_polys;
extern int modelstats_num_polys_drawn;
extern int modelstats_num_verts;
extern int modelstats_num_sortnorms;
#endif

// Tries to move joints so that the turret points to the point dst.
// turret1 is the angles of the turret, turret2 is the angles of the gun from turret
extern int model_rotate_gun(int model_num, model_subsystem *turret, matrix *orient, angles *base_angles, angles *gun_angles, vec3d *pos, vec3d *dst, int obj_idx, bool reset = false);

// Gets and sets turret rotation matrix
extern void model_make_turret_matrix(int model_num, model_subsystem * turret );

// Rotates the angle of a submodel.  Use this so the right unlocked axis
// gets stuffed.
extern void submodel_rotate(model_subsystem *psub, submodel_instance_info * sii);

// Rotates the angle of a submodel.  Use this so the right unlocked axis
// gets stuffed.  Does this for stepped rotations
void submodel_stepped_rotate(model_subsystem *psub, submodel_instance_info *sii);

// Goober5000
// For a submodel, return its overall offset from the main model.
extern void model_find_submodel_offset(vec3d *outpnt, int model_num, int sub_model_num);

// Given a point (pnt) that is in sub_model_num's frame of
// reference, and given the object's orient and position, 
// return the point in 3-space in outpnt.
extern void model_find_world_point(vec3d * outpnt, vec3d *mpnt,int model_num, int sub_model_num, matrix * objorient, vec3d * objpos);
void model_instance_find_world_point(vec3d * outpnt, vec3d *mpnt, int model_num, int model_instance_num, int sub_model_num, matrix * objorient, vec3d * objpos );

// Given a point in the world RF, find the corresponding point in the model RF.
// This is special purpose code, specific for model collision.
// NOTE - this code ASSUMES submodel is 1 level down from hull (detail[0])
void world_find_model_point(vec3d *out, vec3d *world_pt, polymodel *pm, int submodel_num, matrix *orient, vec3d *pos);

void world_find_model_instance_point(vec3d *out, vec3d *world_pt, polymodel *pm, polymodel_instance *pmi, int submodel_num, matrix *orient, vec3d *pos);

extern void find_submodel_instance_point(vec3d *outpnt, object *ship_obj, int submodel_num);
extern void find_submodel_instance_point_normal(vec3d *outpnt, vec3d *outnorm, object *ship_obj, int submodel_num, vec3d *submodel_pnt, vec3d *submodel_norm);
extern void find_submodel_instance_world_point(vec3d *outpnt, object *ship_obj, int submodel_num);

// Given a polygon model index, find a list of rotating submodels to be used for collision
void model_get_rotating_submodel_list(SCP_vector<int> *submodel_vector, object *objp);

// For a rotating submodel, find a point on the axis
void model_init_submodel_axis_pt(submodel_instance_info *sii, int model_num, int submodel_num);

// Given a direction (pnt) that is in sub_model_num's frame of
// reference, and given the object's orient and position, 
// return the point in 3-space in outpnt.
extern void model_find_world_dir(vec3d * out_dir, vec3d *in_dir,int model_num, int sub_model_num, matrix * objorient, vec3d * objpos);
extern void model_instance_find_world_dir(vec3d * out_dir, vec3d *in_dir,int model_num, int model_instance_num, int sub_model_num, matrix * objorient, vec3d * objpos);

// Clears all the submodel instances stored in a model to their defaults.
extern void model_clear_instance(int model_num);

void model_clear_submodel_instance( submodel_instance *sm_instance );
void model_clear_submodel_instances( int model_instance_num );

// Sets rotating submodel turn info to that stored in model
void model_set_instance_info(submodel_instance_info *sii, float turn_rate, float turn_accel);

// Clears all the values in a particular instance to their defaults.
extern void model_clear_instance_info(submodel_instance_info * sii);

// Sets the submodel instance data in a submodel
extern void model_set_instance(int model_num, int sub_model_num, submodel_instance_info * sii, int flags = 0 );

void model_update_instance(int model_instance_num, int sub_model_num, submodel_instance_info *sii);
void model_instance_dumb_rotation(int model_instance_num);

// Adds an electrical arcing effect to a submodel
void model_add_arc(int model_num, int sub_model_num, vec3d *v1, vec3d *v2, int arc_type);

// Fills in an array with points from a model.  Only gets up to max_num verts.
// Returns number of verts found
extern int submodel_get_points(int model_num, int submodel_num, int max_num, vec3d **nts);

// Gets two random points on the surface of a submodel
extern void submodel_get_two_random_points(int model_num, int submodel_num, vec3d *v1, vec3d *v2, vec3d *n1 = NULL, vec3d *n2 = NULL);

// gets the index into the docking_bays array of the specified type of docking point
// Returns the index.  second functions returns the index of the docking bay with
// the specified name
extern int model_find_dock_index(int modelnum, int dock_type, int index_to_start_at = 0);
extern int model_find_dock_name_index(int modelnum, char *name);

// returns the actual name of a docking point on a model, needed by Fred.
char *model_get_dock_name(int modelnum, int index);

// returns number of docking points for a model
int model_get_num_dock_points(int modelnum);
int model_get_dock_index_type(int modelnum, int index);

// get all the different docking point types on a model
int model_get_dock_types(int modelnum);

// Goober5000
// returns index in [0, MAX_SHIP_BAY_PATHS)
int model_find_bay_path(int modelnum, char *bay_path_name);

// Returns number of verts in a submodel;
int submodel_get_num_verts(int model_num, int submodel_num);

// Returns number of polygons in a submodel;
int submodel_get_num_polys(int model_num, int submodel_num);

// Given a vector that is in sub_model_num's frame of
// reference, and given the object's orient and position,
// return the vector in the model's frame of reference.
void model_find_obj_dir(vec3d *w_vec, vec3d *m_vec, object *ship_obj, int sub_model_num);
void model_instance_find_obj_dir(vec3d *w_vec, vec3d *m_vec, object *ship_obj, int sub_model_num);


// This is the interface to model_check_collision.  Rather than passing all these
// values and returning values in globals, just fill in a temporary variable with
// the input values and call model_check_collision
typedef struct mc_info {
	// Input values
	int		model_instance_num;
	int		model_num;			// What model to check
	int		submodel_num;		// What submodel to check if MC_SUBMODEL is set
	matrix	*orient;				// The orient of the model
	vec3d	*pos;					// The pos of the model in world coordinates
	vec3d	*p0;					// The starting point of the ray (sphere) to check
	vec3d	*p1;					// The ending point of the ray (sphere) to check
	int		flags;				// Flags that the model_collide code looks at.  See MC_??? defines
	float		radius;				// If MC_CHECK_THICK is set, checks a sphere moving with the radius.
	
	// Return values
	int		num_hits;			// How many collisions were found
	float		hit_dist;			// The distance from p0 to hitpoint
	vec3d	hit_point;			// Where the collision occurred at in hit_submodel's coordinate system
	vec3d	hit_point_world;	// Where the collision occurred at in world coordinates
	int		hit_submodel;		// Which submodel got hit.
	int		hit_bitmap;			// Which texture got hit.  -1 if not a textured poly
	float		hit_u, hit_v;		// Where on hit_bitmap the ray hit.  Invalid if hit_bitmap < 0
	int		shield_hit_tri;	// Which triangle on the shield got hit or -1 if none
	vec3d	hit_normal;			//	Vector normal of polygon of collision.  (This is in submodel RF)
	int		edge_hit;			// Set if an edge got hit.  Only valid if MC_CHECK_THICK is set.	
	ubyte		*f_poly;				// pointer to flat poly where we intersected
	ubyte		*t_poly;				// pointer to tmap poly where we intersected
		
										// flags can be changed for the case of sphere check finds an edge hit
	mc_info()
	{
		// Echelon9 - BIG WARNING.
        // Using memset() as a constructor is rarely correct in C++
        // If mc_info ever becomes non-POD type, this memset() will hose the virtual table
        memset(this, 0, sizeof(*this));
	}

	mc_info(const mc_info& other)
	{
		this->model_instance_num = other.model_instance_num;
		this->model_num = other.model_num;
		this->submodel_num = other.submodel_num;
		this->orient = other.orient;
		this->pos = other.pos;
		this->p0 = other.p0;
		this->p1 = other.p1;
		this->flags = other.flags;
		this->radius = other.radius;

		this->num_hits = other.num_hits;
		this->hit_dist = other.hit_dist;
		this->hit_point = other.hit_point;
		this->hit_point_world = other.hit_point_world;
        this->hit_submodel = other.hit_submodel;
        this->hit_bitmap = other.hit_bitmap;
		this->hit_u = other.hit_u;
		this->hit_v = other.hit_v;
		this->shield_hit_tri = other.shield_hit_tri;
		this->hit_normal = other.hit_normal;
		this->edge_hit = other.edge_hit;
		this->f_poly = other.f_poly;
		this->t_poly = other.t_poly;
	}
} mc_info;



//======== MODEL_COLLIDE ============

//	Model Collision flags, used in model_collide()
#define MC_CHECK_MODEL			(1<<0)			// Check the polygons in the model.
#define MC_CHECK_SHIELD			(1<<1)			//	check for collision against shield, if it exists.
#define MC_ONLY_SPHERE			(1<<2)			// Only check bounding sphere. Not accurate, but fast.  
															// NOTE!  This doesn't set hit_point correctly with MC_CHECK_SPHERELINE
#define MC_ONLY_BOUND_BOX		(1<<3)			// Only check bounding boxes.  Pretty accurate and slower than MC_ONLY_SPHERE.
															// Checks the rotatated bounding box of each submodel.  
															// NOTE!  This doesn't set hit_point correctly with MC_CHECK_SPHERELINE
#define MC_CHECK_RAY				(1<<4)			// Checks a ray from p0 *through* p1 on to infinity
#define MC_CHECK_SPHERELINE	(1<<5)			// Checks a moving sphere rather than just a ray.  Radius
#define MC_SUBMODEL				(1<<6)			// If this is set, only check the submodel specified in mc->submodel_num. Use with MC_CHECK_MODEL
#define MC_SUBMODEL_INSTANCE	(1<<7)			// Check submodel and its children (of a rotating submodel)
#define MC_CHECK_INVISIBLE_FACES (1<<8)		// Check the invisible faces.


/*
   Checks to see if a vector from p0 to p0 collides with a model of
   type 'model_num' at 'orient' 'pos'.

   Returns the number of polys that were hit.  Zero is none, obviously.
  	Return true if a collision with hull (or shield, if MC_CHECK_SHIELD set), 
	else return false.

   If it did it one or more, then hitpt is the closest 3d point that the
   vector hit.  See the MC_? defines for flag values.

   Model_collide can test a sphere against either (1) shield or (2) model.

   To check a sphere, set the radius of sphere in mc_info structure and
   set the flag MC_CHECK_SPHERE.

   Here is a sample for how to use:
  
	mc_info mc;

	mc.model_num = ???;			// Fill in the model to check
	mc.orient = &obj->orient;	// The object's orient
	mc.pos = &obj->pos;			// The object's position
	mc.p0 = &p0;					// Point 1 of ray to check
	mc.p1 = &p1;					// Point 2 of ray to check
	mc.flags = MC_CHECK_MODEL;	// flags

** TO COLLIDE AGAINST A LINE SEGMENT

  model_collide(&mc);
	if (mc.num_hits) {		
		// We hit submodel mc.hit_submodel on texture mc.hitbitmap,
		// at point mc.hit_point_world, with uv's of mc.hit_u, mc.hit_v.
	}

** TO COLLIDE AGAINST A SPHERE
	mc.flags |= MC_CHECK_SPHERELINE;
	mc.radius = radius;

	model_collide(&mc, radius);
	if (mc.num_hits) {		
		// We hit submodel mc.hit_submodel on texture mc.hitbitmap,
		// at point mc.hit_point_world, with uv's of mc.hit_u, mc.hit_v.
		// Check (mc.edge_hit) to see if we hit an edge
	}
*/

int model_collide(mc_info * mc_info);

void model_collide_preprocess(matrix *orient, int model_instance_num);

// Sets the submodel instance data in a submodel
// If show_damaged is true it shows only damaged submodels.
// If it is false it shows only undamaged submodels.
void model_show_damaged(int model_num, int show_damaged);


//=========================== MODEL OCTANT STUFF ================================

//  Models are now divided into 8 octants.    Shields too.
//  This made the collision code faster.   Shield is 4x and ship faces
//  are about 2x faster.

//  Before, calling model_collide with flags=0 didn't check the shield
//  but did check the model itself.   Setting the shield flags caused
//  the shield to get check along with the ship.
//  Now, you need to explicitly tell the model_collide code to check
//  the model, so you can check the model or shield or both.

//  If you need to check them both, do it in one call; this saves some
//  time.    If checking the shield is sufficient for determining 
//  something   (like if it is under the hud) then use just shield 
//  check, it is at least 5x faster than checking the model itself.


// Model octant ordering - this is a made up ordering, but it makes sense.
// X Y Z  index description
// - - -  0     left bottom rear
// - - +  1     left bottom front
// - + -  2     left top rear
// - + +  3     left top front
// + - -  4     right bottom rear
// + - +  5     right bottom front
// + + -  6     right top rear
// + + +  7     right top front

// Returns which octant point 'pnt' is closet to. This will always return 
// a valid octant (0-7) since the point doesn't have to be in an octant.
// If model_orient and/or model_pos are NULL, pnt is assumed to already 
// be rotated into the model's local coordinates.  
// If oct is not null, it will be filled in with a pointer to the octant
// data.
int model_which_octant_distant(vec3d *pnt, int model_num,matrix *model_orient, vec3d * model_pos, model_octant **oct);

// Returns which octant point 'pnt' is in. This might return
// -1 if the point isn't in any octant.
// If model_orient and/or model_pos are NULL, pnt is assumed to already 
// be rotated into the model's local coordinates.
// If oct is not null, it will be filled in with a pointer to the octant
// data.  Or NULL if the pnt isn't in the octant.
int model_which_octant(vec3d *pnt, int model_num,matrix *model_orient, vec3d * model_pos, model_octant **oct);

typedef struct mst_info {
	int primary_bitmap;
	int primary_glow_bitmap;
	int secondary_glow_bitmap;
	int tertiary_glow_bitmap;
	int distortion_bitmap;

	bool use_ab;
	float glow_noise;
	const vec3d *rotvel;
	vec3d length;

	float glow_rad_factor;
	float secondary_glow_rad_factor;
	float tertiary_glow_rad_factor;
	float glow_length_factor;
	float distortion_rad_factor;
	float distortion_length_factor;
	bool draw_distortion;

	mst_info() : primary_bitmap(-1), primary_glow_bitmap(-1), secondary_glow_bitmap(-1), tertiary_glow_bitmap(-1),
					use_ab(false), glow_noise(1.0f), rotvel(NULL), length(vmd_zero_vector), glow_rad_factor(1.0f),
					secondary_glow_rad_factor(1.0f), tertiary_glow_rad_factor(1.0f), glow_length_factor(1.0f), distortion_rad_factor(1.0f), distortion_length_factor(1.0f)
				{}
} mst_info;


//Valathil - Buffer struct for transparent object sorting
typedef struct transparent_object {
	int blend_filter;
	float alpha;
	int texture;
	int glow_map;
	int spec_map;
	int norm_map;
	int height_map;
	int misc_map;
	vertex_buffer *buffer;
	unsigned int tmap_flags;
	int i;
	vec3d scale;
} transparent_object;

typedef struct transparent_submodel {
	bsp_info *model;
	matrix orient;
	bool is_submodel;
	bool pop_matrix;
	SCP_vector<transparent_object> transparent_objects;
} transparent_submodel;
// scale the engines thrusters by this much
// Only enabled if MR_SHOW_THRUSTERS is on
void model_set_thrust(int model_num = -1, mst_info *mst = NULL);


//=======================================================================================
// Finds the closest point on a model to a point in space.  Actually only finds a point
// on the bounding box of the model.    
// Given:
//   model_num      Which model
//   submodel_num   Which submodel, -1 for hull
//   orient         Orientation of the model
//   pos            Position of the model
//   eye_pos        Point that you want to find the closest point to
// Returns:
//   distance from eye_pos to closest_point.  0 means eye_pos is 
//   on or inside the bounding box.
//   Also fills in outpnt with the actual closest point.
float model_find_closest_point(vec3d *outpnt, int model_num, int submodel_num, matrix *orient, vec3d * pos, vec3d *eye_pos);

// set the insignia bitmap to be used when rendering a ship with an insignia (-1 switches it off altogether)
void model_set_insignia_bitmap(int bmap);

// set model transparency for use with MR_ALL_XPARENT
void model_set_alpha(float alpha);

// set the forces bitmap
void model_set_forced_texture(int bmap);

// see if the given texture is used by the passed model. 0 if not used, 1 if used, -1 on error
int model_find_texture(int model_num, int bitmap);

// find closest point on extended bounding box (the bounding box plus all the planes that make it up)
// returns closest distance to extended box
// positive return value means start_point is outside extended box
// displaces closest point an optional amount delta to the outside of the box
// closest_box_point can be NULL.
float get_world_closest_box_point_with_delta(vec3d *closest_box_point, object *box_obj, vec3d *start_point, int *is_inside, float delta);

// given a newly loaded model, page in all textures
void model_page_in_textures(int modelnum, int ship_info_index = -1);

// given a model, unload all of its textures
void model_page_out_textures(int model_num, bool release = false);

void model_set_warp_globals(float scale_x = 1.0f, float scale_y = 1.0f, float scale_z = 1.0f, int bitmap_id = -1, float alpha = -1.0f);

void model_set_replacement_textures(int *replacement_textures);

void model_setup_cloak(vec3d *shift, int full_cloak, int alpha);
void model_finish_cloak(int full_cloak);

void model_do_dumb_rotation(int modelnum); //Bobboau
#endif // _MODEL_H
