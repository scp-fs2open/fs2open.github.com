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

#include "globalincs/globals.h" // for NAME_LENGTH
#include "globalincs/pstypes.h"

#include "actions/Program.h"
#include "gamesnd/gamesnd.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "model/model_flags.h"
#include "object/object.h"
#include "ship/ship_flags.h"

class object;
class ship_info;
class model_render_params;

extern flag_def_list model_render_flags[];
extern int model_render_flags_size;

#define MAX_DEBRIS_OBJECTS	32
#define MAX_MODEL_DETAIL_LEVELS	8
#define MAX_PROP_LEN			256
#define MAX_NAME_LEN			32
#define MAX_ARC_EFFECTS		8

// For POF compatibility reasons, the first four of these should not be renumbered.
#define MOVEMENT_TYPE_NONE			-1
#define MOVEMENT_TYPE_UNUSED		0	// previously MOVEMENT_TYPE_POS
#define MOVEMENT_TYPE_REGULAR		1	// previously MOVEMENT_TYPE_ROT
#define MOVEMENT_TYPE_TURRET		2	// for turrets only
#define MOVEMENT_TYPE_TRIGGERED		3
#define MOVEMENT_TYPE_INTRINSIC		4	// intrinsic (non-subsystem-based)


// DA 11/13/98 Reordered to account for difference between max and game
#define MOVEMENT_AXIS_NONE		-1
#define MOVEMENT_AXIS_X			0
#define MOVEMENT_AXIS_Y			2
#define MOVEMENT_AXIS_Z			1
#define MOVEMENT_AXIS_OTHER		3

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
extern const char *Subsystem_types[SUBSYSTEM_MAX];

#define MAX_TFP						10				// maximum number of turret firing points

#define MAX_SPLIT_PLANE				5				// number of artist specified split planes (used in big ship explosions)

// Data specific to a particular instance of a submodel.
struct submodel_instance
{
	float	cur_angle = 0.0f;							// The current angle this thing is turned to.
	float	prev_angle = 0.0f;
	float	turret_idle_angle = 0.0f;				// If this is a turret, this is the expected idling angle of the submodel

	float	current_turn_rate = 0.0f;
	float	desired_turn_rate = 0.0f;
	float	turn_accel = 0.0f;
	int		turn_step_zero_timestamp = timestamp();		// timestamp determines when next step is to begin (for stepped rotation)

	vec3d	point_on_axis = vmd_zero_vector;		// in ship RF
	bool	axis_set = false;

	bool	blown_off = false;						// If set, this subobject is blown off
	bool	collision_checked = false;

	// These fields are the true standard reference for submodel rotation.  They should seldom be read directly
	// and should almost never be written directly.  In most cases, coders should prefer cur_angle and prev_angle.
	matrix	canonical_orient = vmd_identity_matrix;
	matrix	canonical_prev_orient = vmd_identity_matrix;

	// --- these fields used to be in bsp_info ---

	// Electrical Arc Effect Info
	// Sets a spark for this submodel between vertex v1 and v2	
	int		num_arcs = 0;											// See model_add_arc for more info	
	vec3d	arc_pts[MAX_ARC_EFFECTS][2];
	ubyte		arc_type[MAX_ARC_EFFECTS];							// see MARC_TYPE_* defines

	//SMI-Specific movement axis. Only valid in MOVEMENT_TYPE_TRIGGERED.
	vec3d	rotation_axis;

	submodel_instance()
	{
		memset(&arc_pts, 0, MAX_ARC_EFFECTS * 2 * sizeof(vec3d));
		memset(&arc_type, 0, MAX_ARC_EFFECTS * sizeof(ubyte));
	}
};

// Data specific to a particular instance of a model.
struct polymodel_instance
{
	int id = -1;							// global model_instance num index
	int model_num = -1;						// global model num index, same as polymodel->id
	submodel_instance *submodel = nullptr;	// array of submodel instances; mirrors the polymodel->submodel array

	int objnum;								// id of the object using this pmi, or -1 if no object (e.g. skybox) 
};

#define MAX_MODEL_SUBSYSTEMS		200				// used in ships.cpp (only place?) for local stack variable DTP; bumped to 200
													// when reading in ships.tbl

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
class model_subsystem {					/* contains rotation rate info */
public:
	flagset<Model::Subsystem_Flags>	flags;	    // See model_flags.h
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
    char	crewspot[MAX_NAME_LEN];	    		// unique identifying name for this turret -- used to assign AI class and multiplayer people
	vec3d	turret_norm;						//	direction this turret faces (i.e. the normal to the turret base, or the center of the field of view)
	float	turret_fov;							//	dot of turret_norm:vec_to_enemy > this means can see
	float	turret_max_fov;						//  dot of turret_norm:vec_to_enemy <= this means barrels can elevate up to the target
	float	turret_base_fov;						//  turret's base's fov
	int		turret_num_firing_points;			// number of firing points on this turret
	vec3d	turret_firing_point[MAX_TFP];		//	in parent object's reference frame, point from which to fire.
	int		turret_gun_sobj;					// Which subobject in this model the firing points are linked to.
	float	turret_turning_rate;				// How fast the turret turns. Read from ships.tbl
	gamesnd_id	turret_base_rotation_snd;				// Sound to make when the turret moves
	float	turret_base_rotation_snd_mult;			// Volume multiplier for the turret sounds
	gamesnd_id		turret_gun_rotation_snd;				// Sound to make when the turret moves
	float	turret_gun_rotation_snd_mult;			// Volume multiplier for the turret sounds


	//Sound stuff
	gamesnd_id	alive_snd;		//Sound to make while the subsystem is not-dead
	gamesnd_id	dead_snd;		//Sound to make when the subsystem is dead.
	gamesnd_id	rotation_snd;	//Sound to make when the subsystem is rotating. (ie turrets)

	// engine wash info
	struct engine_wash_info		*engine_wash_pointer;					// index into Engine_wash_info

	// rotation specific info
	int			weapon_rotation_pbank;				// weapon-controlled rotation - Goober5000
	stepped_rotation_t	*stepped_rotation;			// turn rotation struct

	// AWACS specific information
	float		awacs_intensity;						// awacs intensity of this subsystem
	float		awacs_radius;							// radius of effect of the AWACS

	int		scan_time;							// overrides ship class scan time if >0

	int		primary_banks[MAX_SHIP_PRIMARY_BANKS];					// default primary weapons -hoffoss
	int		primary_bank_capacity[MAX_SHIP_PRIMARY_BANKS];		// capacity of a bank - Goober5000
	int		secondary_banks[MAX_SHIP_SECONDARY_BANKS];				// default secondary weapons -hoffoss
	int		secondary_bank_capacity[MAX_SHIP_SECONDARY_BANKS];	// capacity of a bank -hoffoss
	int		path_num;								// path index into polymodel .paths array.  -2 if none exists, -1 if not defined

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

	actions::ProgramSet beam_warmdown_program;

    void reset();

    model_subsystem();
};

typedef struct model_special {
	struct	model_special *next, *prev;		// for using as a linked list
	int		bank;										// used for sequencing gun/missile backs. approach/docking points
	int		slot;										// location for gun or missile in this bank
	vec3d	pnt;										// point where this special submodel thingy is at
	vec3d	norm;										// normal for the special submodel thingy
} model_special;

// model arc types
#define MARC_TYPE_DAMAGED					0		// blue lightning arcs for when the ship is damaged
#define MARC_TYPE_EMP						1		// EMP blast type arcs
#define MARC_TYPE_SHIP						2		// arcing lightning thats intrinsically part of the ship

#define MAX_LIVE_DEBRIS	7

typedef struct model_tmap_vert {
	ushort vertnum;
	ushort normnum;
	float u,v;
} model_tmap_vert;

struct bsp_collision_node {
	vec3d min;
	vec3d max;

	int back;
	int front;

	int leaf;
};

struct bsp_collision_leaf {
	vec3d plane_pnt;
	vec3d plane_norm;
	float face_rad;
	int vert_start;
	ubyte num_verts;
	ubyte tmap_num;

	int next;
};

struct bsp_collision_tree {
	bsp_collision_node *node_list;
	int n_nodes;

	bsp_collision_leaf *leaf_list;
	int n_leaves;

	model_tmap_vert *vert_list;
	vec3d *point_list;

	int n_verts;
	bool used;
};

class bsp_info
{
public:
	bsp_info()
		: bsp_data_size(0), bsp_data(nullptr), collision_tree_index(-1),
		rad(0.0f), my_replacement(-1), i_replace(-1), num_live_debris(0),
		parent(-1), num_children(0), first_child(-1), next_sibling(-1), num_details(0),
		outline_buffer(nullptr), n_verts_outline(0), render_sphere_radius(0.0f), use_render_box(0),	use_render_sphere(0)
	{
		name[0] = 0;
		lod_name[0] = 0;

		offset = geometric_center = min = max = render_box_min = render_box_max = render_box_offset = render_sphere_offset = vmd_zero_vector;
		frame_of_reference = vmd_identity_matrix;

		memset(&bounding_box, 0, 8 * sizeof(vec3d));
		memset(&live_debris, 0, MAX_LIVE_DEBRIS * sizeof(int));
		memset(&details, 0, MAX_MODEL_DETAIL_LEVELS * sizeof(int));
	}

	char	name[MAX_NAME_LEN];						// name of the subsystem.  Probably displayed on HUD

	int		rotation_type = MOVEMENT_TYPE_NONE;
	vec3d	rotation_axis = vmd_zero_vector;		// which axis this subobject rotates on.
	int		rotation_axis_id = MOVEMENT_AXIS_NONE;	// for optimization

	float	default_turn_rate = 0.0f;
	float	default_turn_accel = 0.0f;

	flagset<Model::Submodel_flags> flags;

	int subsys_num = -1;			// subsystem number; index to ship_info->subsystems; the counterpart of model_subsystem->subobj_num

	vec3d	offset;					// 3d offset from parent object
	matrix	frame_of_reference;		// used to be called 'orientation' - this is just used for setting the rotation axis and the animation angles

	int		bsp_data_size;
	ubyte		*bsp_data;

	int collision_tree_index;

	vec3d	geometric_center;		// geometric center of this subobject.  In the same Frame Of 
	                              //  Reference as all other vertices in this submodel. (Relative to pivot point)
	float		rad;						// radius for each submodel

	vec3d	min;						// The min point of this object's geometry
	vec3d	max;						// The max point of this object's geometry
	vec3d	bounding_box[8];		// calculated fron min/max

	int		my_replacement;		// If not -1 this subobject is what should get rendered instead of this one
	int		i_replace;				// If this is not -1, then this subobject will replace i_replace when it is damaged

	int		num_live_debris;		// num live debris models assocaiated with a submodel
	int		live_debris[MAX_LIVE_DEBRIS];	// array of live debris submodels for a submodel

	// Tree info
	int		parent;					// what is parent for each submodel, -1 if none
	int		num_children;			// How many children this model has
	int		first_child;			// The first_child of this model, -1 if none
	int		next_sibling;			// This submodel's next sibling, -1 if none

	int		num_details;			// How many submodels are lower detail "mirrors" of this submodel
	int		details[MAX_MODEL_DETAIL_LEVELS];		// A list of all the lower detail "mirrors" of this submodel

	// buffers used by HT&L
	vertex_buffer buffer;
	vertex_buffer trans_buffer;

	vertex *outline_buffer;
	uint n_verts_outline;

	vec3d	render_box_min;
	vec3d	render_box_max;
	vec3d	render_box_offset;
	float	render_sphere_radius;
	vec3d	render_sphere_offset;
	int		use_render_box;			// 0==do nothing, 1==only render this object if you are inside the box, -1==only if you're outside
	int		use_render_sphere;		// 0==do nothing, 1==only render this object if you are inside the sphere, -1==only if you're outside

	char	lod_name[MAX_NAME_LEN];	//FUBAR:  Name to be used for LOD naming comparison to preserve compatibility with older tables.  Only used on LOD0 

	int		look_at_submodel = -1;		//VA - number of the submodel to be looked at by this submodel (-1 if none)
	float	look_at_offset = -1.0f;		//angle in radians that the submodel should be turned from its initial orientation to be considered "looking at" something (-1 if set on first eval)
};

#define MP_TYPE_UNUSED 0
#define MP_TYPE_SUBSYS 1

typedef struct mp_vert {
	vec3d		pos;				// xyz coordinates of vertex in object's frame of reference
	int			nturrets;		// number of turrets guarding this vertex
	int			*turret_ids;	// array of indices into ship_subsys linked list (can't index using [] though)
	float			radius;			// How far the closest obstruction is from this vertex
} mp_vert;

typedef struct model_path {
	char			name[MAX_NAME_LEN];					// name of the path.  Should be unique
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

// info for gun and missile banks.  Also used for docking points.  There should always
// only be two slots for each docking bay

struct w_bank
{
	int		num_slots = 0;
	vec3d	*pnt = nullptr;
	vec3d	*norm = nullptr;
	float   *external_model_angle_offset = nullptr;

	~w_bank()
	{
		delete[] pnt;
		delete[] norm;
		delete[] external_model_angle_offset;
	}
};

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

#define PULSE_SIN 1
#define PULSE_COS 2
#define PULSE_TRI 3
#define PULSE_SHIFTTRI 4

typedef struct glow_point_bank {  // glow bank structure -Bobboau
	int			type;
	int			glow_timestamp; 
	int			on_time; 
	int			off_time; 
	int			disp_time; 
	bool		is_on;
	int			submodel_parent; 
	int			LOD; 
	int			num_points; 
	glow_point	*points;
	int			glow_bitmap; 
	int			glow_neb_bitmap; 
} glow_point_bank;

typedef struct glow_point_bank_override {
	char		name[33];
	int			type;
	int			on_time; 
	int			off_time; 
	int			disp_time; 
	int			glow_bitmap; 
	int			glow_neb_bitmap;
	bool		is_on;

	bool		type_override;
	bool		on_time_override; 
	bool		off_time_override; 
	bool		disp_time_override; 
	bool		glow_bitmap_override;

	ubyte		pulse_type;
	int			pulse_period;
	float		pulse_amplitude;
	float		pulse_bias;
	float		pulse_exponent;
	bool		is_lightsource;
	float		radius_multi;
	vec3d		light_color;
	vec3d		light_mix_color;
	bool		lightcone;
	float		cone_angle;
	float		cone_inner_angle;
	vec3d		cone_direction;
	bool		dualcone;
	bool		rotating;
	vec3d		rotation_axis;
	float		rotation_speed;

	bool		pulse_period_override;
} glow_point_bank_override;

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
	int		parent_submodel;			// if this dockpoint should be relative to a submodel instead of the main model
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
struct shield_info {
	int				nverts;
	int				ntris;
	shield_vertex	*verts;
	shield_tri		*tris;

	gr_buffer_handle buffer_id;
	int buffer_n_verts;
	vertex_layout layout;

	shield_info() : nverts(0), ntris(0), verts(NULL), tris(NULL), buffer_id(-1), buffer_n_verts(0), layout() {	}
};

#define BSP_LIGHT_TYPE_WEAPON 1
#define BSP_LIGHT_TYPE_THRUSTER 2

typedef struct bsp_light {
	vec3d			pos;
	int				type;		// See BSP_LIGHT_TYPE_?? for values
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
#define MAX_INS_VECS					81
#define MAX_INS_FACES				128
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

#define PM_FLAG_ALLOW_TILING			(1<<0)					// Allow texture tiling
#define PM_FLAG_AUTOCEN					(1<<1)					// contains autocentering info	
#define PM_FLAG_TRANS_BUFFER			(1<<2)					// render transparency buffer
#define PM_FLAG_BATCHED					(1<<3)					// this model can be batch rendered
#define PM_FLAG_HAS_INTRINSIC_MOTION	(1<<4)					// whether this model has an intrinsic rotation or translation submodel somewhere

// Goober5000
class texture_info
{
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

	int LoadTexture(const char *filename, const char *dbg_name = "<UNKNOWN>");

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
#define TM_SPEC_GLOSS_TYPE	6		// optional reflectance map (specular and gloss)
#define TM_AMBIENT_TYPE		7		// optional ambient occlusion map with ambient occlusion and cavity occlusion factors for red and green channels.
#define TM_NUM_TYPES		8		//WMC - Number of texture_info objects in texture_map
									//Used by scripting - if you change this, do a search
									//to update switch() statement in lua.cpp
// taylor
//WMC - OOPified
class texture_map
{
public:
	texture_info textures[TM_NUM_TYPES];

	bool is_ambient;
	bool is_transparent;

	int FindTexture(int bm_handle);
	int FindTexture(const char* name);

	void PageIn();
	void PageOut(bool release);

	void Clear();
	void ResetToOriginal();

	texture_map()
		: is_ambient(false), is_transparent(false)
	{}
};

#define MAX_REPLACEMENT_TEXTURES MAX_MODEL_TEXTURES * TM_NUM_TYPES

// Goober5000 - since we need something < 0
#define REPLACE_WITH_INVISIBLE	-47

//used to describe a polygon model
// NOTE: Because WMC OOPified the textures, this must now be treated as a class, rather than a struct.
//       Additionally, a lot of model initialization and de-initialization is currently done in model_load or model_unload.
class polymodel
{
public:
	// initialize to 0 and NULL because previously a memset was used
	polymodel()
		: id(-1), version(0), flags(0), n_detail_levels(0), num_debris_objects(0), n_models(0), num_lights(0), lights(NULL),
		n_view_positions(0), rad(0.0f), core_radius(0.0f), n_textures(0), submodel(NULL), n_guns(0), n_missiles(0), n_docks(0),
		n_thrusters(0), gun_banks(NULL), missile_banks(NULL), docking_bays(NULL), thrusters(NULL), ship_bay(NULL), shield(),
		shield_collision_tree(NULL), sldc_size(0), n_paths(0), paths(NULL), mass(0), num_xc(0), xc(NULL), num_split_plane(0),
		num_ins(0), used_this_mission(0), n_glow_point_banks(0), glow_point_banks(nullptr),
		vert_source()
	{
		filename[0] = 0;
		mins = maxs = autocenter = center_of_mass = vmd_zero_vector;
		moment_of_inertia = vmd_identity_matrix;

		memset(&detail, 0, MAX_MODEL_DETAIL_LEVELS * sizeof(int));
		memset(&detail_depth, 0, MAX_MODEL_DETAIL_LEVELS * sizeof(float));
		memset(&debris_objects, 0, MAX_DEBRIS_OBJECTS * sizeof(int));
		memset(&bounding_box, 0, 8 * sizeof(vec3d));
		memset(&view_positions, 0, MAX_EYES * sizeof(eye));
		memset(&octants, 0, 8 * sizeof(model_octant));
		memset(&split_plane, 0, MAX_SPLIT_PLANE * sizeof(float));
		memset(&ins, 0, MAX_MODEL_INSIGNIAS * sizeof(insignia));

#ifndef NDEBUG
		ram_used = 0;
		debug_info_size = 0;
		debug_info = NULL;
#endif
	}


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
	SCP_vector<vec3d>		shield_points;

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

	indexed_vertex_source vert_source;
	
	vertex_buffer detail_buffers[MAX_MODEL_DETAIL_LEVELS];
};

// Iterate over a submodel tree, starting at the given submodel root node, and running the given function for each node.  The function's signature should be:
//
// void func(int submodel, int level, bool isLeaf);
//
// The "level" parameter indicates how deep the nesting level is, and the "isLeaf" parameter indicates whether the submodel is a leaf node.
// To find the model's detail0 root node, use pm->submodel[pm->detail[0]].
template <typename Func>
void model_iterate_submodel_tree(polymodel* pm, int submodel, Func func, int level = 0)
{
	Assertion(pm != nullptr, "pm must not be null!");
	Assertion(submodel >= 0 && submodel < pm->n_models, "submodel must be in range!");

	auto child = pm->submodel[submodel].first_child;

	func(submodel, level, child < 0);

	while (child >= 0)
	{
		model_iterate_submodel_tree(pm, child, func, level + 1);
		child = pm->submodel[child].next_sibling;
	}
}

// Call once to initialize the model system
void model_init();

// call to unload a model (works like bm_unload()), "force" SHOULD NEVER BE SET outside of modelread.cpp!!!!
void model_unload(int modelnum, int force = 0);

// Call to free all existing models
void model_free_all();
void model_instance_free_all();

// Loads a model from disk and returns the model number it loaded into.
int model_load(const char *filename, int n_subsystems, model_subsystem *subsystems, int ferror = 1, int duplicate = 0);

int model_create_instance(int objnum, int model_num);
void model_delete_instance(int model_instance_num);

// Goober5000
void model_load_texture(polymodel *pm, int i, char *file);

// Returns a pointer to the polymodel structure for model 'n'
polymodel *model_get(int model_num);

polymodel_instance* model_get_instance(int model_instance_num);

// routine to copy subsystems.  Must be called when subsystems sets are the same -- see ship.cpp
void model_copy_subsystems(int n_subsystems, model_subsystem *d_sp, model_subsystem *s_sp);

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color(int r, int g, int b);

// IF MR_LOCK_DETAIL is set, then it will always draw detail level 'n'
// This defaults to 0. (0=highest, larger=lower)
void model_set_detail_level(int n);

// Flags you can pass to model_render
#define MR_NORMAL					(0)			// Draw a normal object
#define MR_SHOW_OUTLINE				(1<<0)		// Draw the object in outline mode. Color specified by model_set_outline_color
#define MR_SKYBOX					(1<<1)		// Draw as a skybox
#define MR_DESATURATED				(1<<2)		// Draw model in monochrome using outline color
#define MR_STENCIL_WRITE			(1<<3)		// Write stencil buffere where the model was rendered
#define MR_STENCIL_READ				(1<<4)		// Only draw pixels of the model where the stencil buffer has the right value (see MR_STENCIL_WRITE)
#define MR_SHOW_THRUSTERS			(1<<5)		// Show the engine thrusters. See model_set_thrust for how long it draws.
#define MR_NO_COLOR_WRITES			(1<<6)		// Don't write anything to the color buffers (used when setting the stencil buffer)
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
#define MR_EMPTY_SLOT3				(1<<17)		// draw bay paths
#define MR_ALL_XPARENT				(1<<18)		// render it fully transparent
#define MR_NO_ZBUFFER				(1<<19)		// switch z-buffering off completely 
#define MR_NO_CULL					(1<<20)		// don't cull backfacing poly's
#define MR_EMPTY_SLOT4				(1<<21)		// force a given texture to always be used
#define MR_NO_BATCH					(1<<22)		// don't use submodel batching when rendering
#define MR_EDGE_ALPHA				(1<<23)		// makes norms that are faceing away from you render more transparent -Bobboau
#define MR_CENTER_ALPHA				(1<<24)		// oposite of above -Bobboau
#define MR_NO_FOGGING				(1<<25)		// Don't fog - taylor
#define MR_SHOW_OUTLINE_HTL			(1<<26)		// Show outlines (wireframe view) using HTL method
#define MR_NO_GLOWMAPS				(1<<27)		// disable rendering of glowmaps - taylor
#define MR_FULL_DETAIL				(1<<28)		// render all valid objects, particularly ones that are otherwise in/out of render boxes - taylor
#define MR_FORCE_CLAMP				(1<<29)		// force clamp - Hery
#define MR_EMPTY_SLOT5				(1<<30)		// Use a animated Shader - Valathil
#define MR_ATTACHED_MODEL			(1<<31)		// Used for attached weapon model lodding

#define MR_DEBUG_PIVOTS				(1<<0)		// Show the pivot points
#define MR_DEBUG_PATHS				(1<<1)		// Show the paths associated with a model
#define MR_DEBUG_RADIUS				(1<<2)		// Show the radius around the object
#define MR_DEBUG_SHIELDS			(1<<3)		// Show the shield mesh
#define MR_DEBUG_BAY_PATHS			(1<<4)		// draw bay paths
#define MR_DEBUG_NO_DIFFUSE			(1<<5)
#define MR_DEBUG_NO_SPEC			(1<<6)
#define MR_DEBUG_NO_NORMAL			(1<<7)
#define MR_DEBUG_NO_ENV				(1<<8)
#define MR_DEBUG_NO_GLOW			(1<<9)
#define MR_DEBUG_NO_HEIGHT			(1<<10)
#define MR_DEBUG_NO_AMBIENT			(1<<11)
#define MR_DEBUG_NO_MISC			(1<<12)
#define MR_DEBUG_NO_REFLECT			(1<<13)

//Defines for the render parameter of model_render, model_really_render and model_render_buffers
#define MODEL_RENDER_OPAQUE 1
#define MODEL_RENDER_TRANS 2
#define MODEL_RENDER_ALL 3

// Returns the radius of a model
float model_get_radius(int modelnum);
float submodel_get_radius(int modelnum, int submodelnum);

// Returns the core radius (smallest dimension of hull's bounding box, used for collision detection with big ships only)
float model_get_core_radius(int modelnum);

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
extern int model_rotate_gun(object *objp, polymodel *pm, polymodel_instance *pmi, ship_subsys *ss, vec3d *dst, bool reset = false);

// Rotates the angle of a submodel.  Use this so the right unlocked axis
// gets stuffed.
extern void submodel_canonicalize(bsp_info *sm, submodel_instance *smi, bool clamp);
extern void submodel_rotate(model_subsystem *psub, submodel_instance *smi);
extern void submodel_rotate(bsp_info *sm, submodel_instance *smi);

// Rotates the angle of a submodel.  Use this so the right unlocked axis
// gets stuffed.  Does this for stepped rotations
void submodel_stepped_rotate(model_subsystem *psub, submodel_instance *smi);

// ------- submodel transformations -------

// Goober5000
// For a submodel, return its overall offset from the main model.
extern void model_find_submodel_offset(vec3d *outpnt, const polymodel *pm, int sub_model_num);

// Given a point in a submodel's local frame of reference, transform it to a global frame of reference.
// If objorient and objpos are supplied, this will be world space; otherwise it will be the model's space.
extern void model_local_to_global_point(vec3d *outpnt, const vec3d *mpnt, int model_num, int submodel_num, const matrix *objorient = nullptr, const vec3d *objpos = nullptr);
// Given a point in a submodel's local frame of reference, transform it to a global frame of reference.
// If objorient and objpos are supplied, this will be world space; otherwise it will be the model's space.
extern void model_local_to_global_point(vec3d *outpnt, const vec3d *mpnt, const polymodel *pm, int submodel_num, const matrix *objorient = nullptr, const vec3d *objpos = nullptr);

// Given a point in a submodel's local frame of reference, transform it to a global frame of reference, taking into account submodel rotations.
// If objorient and objpos are supplied, this will be world space; otherwise it will be the model's space.
extern void model_instance_local_to_global_point(vec3d *outpnt, const vec3d *mpnt, int model_instance_num, int submodel_num, const matrix *objorient = nullptr, const vec3d *objpos = nullptr);
// Given a point in a submodel's local frame of reference, transform it to a global frame of reference, taking into account submodel rotations.
// If objorient and objpos are supplied, this will be world space; otherwise it will be the model's space.
extern void model_instance_local_to_global_point(vec3d *outpnt, const vec3d *mpnt, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *objorient = nullptr, const vec3d *objpos = nullptr);

// Given a direction (or normal) in a submodel's local frame of reference, transform it to a global frame of reference.
// If objorient and objpos are supplied, this will be world space; otherwise it will be the model's space.
extern void model_local_to_global_dir(vec3d *out_dir, const vec3d *in_dir, int model_num, int submodel_num, const matrix *objorient = nullptr);
// Given a direction (or normal) in a submodel's local frame of reference, transform it to a global frame of reference.
// If objorient and objpos are supplied, this will be world space; otherwise it will be the model's space.
extern void model_local_to_global_dir(vec3d *out_dir, const vec3d *in_dir, const polymodel *pm, int submodel_num, const matrix *objorient = nullptr);

// Given a direction (or normal) in a submodel's local frame of reference, transform it to a global frame of reference, taking into account submodel rotations..
// If objorient and objpos are supplied, this will be world space; otherwise it will be the model's space.
extern void model_instance_local_to_global_dir(vec3d *out_dir, const vec3d *in_dir, int model_instance_num, int submodel_num, const matrix *objorient = nullptr, bool use_submodel_parent = false);
// Given a direction (or normal) in a submodel's local frame of reference, transform it to a global frame of reference, taking into account submodel rotations..
// If objorient and objpos are supplied, this will be world space; otherwise it will be the model's space.
extern void model_instance_local_to_global_dir(vec3d *out_dir, const vec3d *in_dir, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *objorient = nullptr);

// Given a point in world coordinates, transform it to a submodel's local frame of reference.
// This is special purpose code, specific for model collision.
// NOTE - this code ASSUMES submodel is 1 level down from hull (detail[0])
// TODO - fix this function to work for all submodel levels
extern void model_instance_world_to_local_point(vec3d *out, vec3d *world_pt, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *orient, const vec3d *pos);

// Combines model_instance_local_to_global_point and model_instance_local_to_global_dir into one function.
extern void model_instance_local_to_global_point_dir(vec3d *outpnt, vec3d *outnorm, const vec3d *submodel_pnt, const vec3d *submodel_norm, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *objorient = nullptr, const vec3d *objpos = nullptr);

// Combines model_instance_local_to_global_point and the matrix equivalent of model_instance_local_to_global_dir into one function.
extern void model_instance_local_to_global_point_orient(vec3d *outpnt, matrix *outorient, const vec3d *submodel_pnt, const matrix *submodel_orient, const polymodel *pm, const polymodel_instance *pmi, int submodel_num, const matrix *objorient = nullptr, const vec3d *objpos = nullptr);

// ------- end of submodel transformations -------

// Given a polygon model index, find a list of moving submodels to be used for collision
void model_get_moving_submodel_list(SCP_vector<int> &submodel_vector, const object *objp);

// Given a polygon model index, get a list of a model tree starting from that index
void model_get_submodel_tree_list(SCP_vector<int> &submodel_vector, const polymodel *pm, int mn);

// For a rotating submodel, find a point on the axis
void model_init_submodel_axis_pt(polymodel *pm, polymodel_instance *pmi, int submodel_num);

// Clears all the submodel instances stored in a model to their defaults.
extern void model_clear_instance(int model_num);

// Sets rotating submodel turn info to that stored in model
extern void model_set_submodel_instance_motion_info(bsp_info *sm, submodel_instance *smi);

// Sets the submodel instance data in a submodel
extern void model_set_up_techroom_instance(ship_info *sip, int model_instance_num);

void model_replicate_submodel_instance(polymodel *pm, polymodel_instance *pmi, int submodel_num, flagset<Ship::Subsystem_Flags>& flags);

// Adds an electrical arcing effect to a submodel
void model_instance_clear_arcs(polymodel *pm, polymodel_instance *pmi);
void model_instance_add_arc(polymodel *pm, polymodel_instance *pmi, int sub_model_num, vec3d *v1, vec3d *v2, int arc_type);

// Gets two random points on the surface of a submodel
extern void submodel_get_two_random_points(int model_num, int submodel_num, vec3d *v1, vec3d *v2, vec3d *n1 = NULL, vec3d *n2 = NULL);

extern void submodel_get_two_random_points_better(int model_num, int submodel_num, vec3d * v1, vec3d * v2, int seed = -1);

// gets the average position of the mesh at a particular z slice, approximately
void submodel_get_cross_sectional_avg_pos(int model_num, int submodel_num, float z_slice_pos, vec3d* pos);
// generates a random position more or less inside-ish a mesh at a particular z slice
void submodel_get_cross_sectional_random_pos(int model_num, int submodel_num, float z_slice_pos, vec3d* pos);
  
extern int model_find_submodel_index(int modelnum, const char *name);

// gets the index into the docking_bays array of the specified type of docking point
// Returns the index.  second functions returns the index of the docking bay with
// the specified name
extern int model_find_dock_index(int modelnum, int dock_type, int index_to_start_at = 0);
extern int model_find_dock_name_index(int modelnum, const char* name);

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

// Returns number of polygons in a submodel;
int submodel_get_num_polys(int model_num, int submodel_num);


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
	int		lod;				// Which detail level of the submodel to check instead
	
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
	bool		edge_hit;			// Set if an edge got hit.  Only valid if MC_CHECK_THICK is set.	
	ubyte		*f_poly;				// pointer to flat poly where we intersected
	ubyte		*t_poly;				// pointer to tmap poly where we intersected
	bsp_collision_leaf *bsp_leaf;

										// flags can be changed for the case of sphere check finds an edge hit
} mc_info;

inline void mc_info_init(mc_info *mc)
{
	mc->model_instance_num = -1;
	mc->model_num = -1;
	mc->submodel_num = -1;
	mc->orient = nullptr;
	mc->pos = nullptr;
	mc->p0 = nullptr;
	mc->p1 = nullptr;
	mc->flags = 0;
	mc->lod = 0;
	mc->radius = 0;
	mc->num_hits = 0; 
	mc->hit_dist = 0;
	mc->hit_point = vmd_zero_vector;
	mc->hit_point_world = vmd_zero_vector;
	mc->hit_submodel = -1;
	mc->hit_bitmap = -1;
	mc->hit_u = 0; mc->hit_v = 0;
	mc->shield_hit_tri = -1;
	mc->hit_normal = vmd_zero_vector;
	mc->edge_hit = false;
	mc->f_poly = nullptr;
	mc->t_poly = nullptr;
	mc->bsp_leaf = nullptr;
}


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

int model_collide(mc_info *mc_info_obj);
void model_collide_parse_bsp(bsp_collision_tree *tree, void *model_ptr, int version);

bsp_collision_tree *model_get_bsp_collision_tree(int tree_index);
void model_remove_bsp_collision_tree(int tree_index);
int model_create_bsp_collision_tree();

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

typedef struct mst_info {
	int primary_bitmap;
	int primary_glow_bitmap;
	int secondary_glow_bitmap;
	int tertiary_glow_bitmap;
	int distortion_bitmap;

	bool use_ab;
	float glow_noise;
	vec3d rotvel;
	vec3d length;

	float glow_rad_factor;
	float secondary_glow_rad_factor;
	float tertiary_glow_rad_factor;
	float glow_length_factor;
	float distortion_rad_factor;
	float distortion_length_factor;
	bool draw_distortion;

	mst_info() : primary_bitmap(-1), primary_glow_bitmap(-1), secondary_glow_bitmap(-1), tertiary_glow_bitmap(-1), distortion_bitmap(-1),
					use_ab(false), glow_noise(1.0f), rotvel(vmd_zero_vector), length(vmd_zero_vector), glow_rad_factor(1.0f),
					secondary_glow_rad_factor(1.0f), tertiary_glow_rad_factor(1.0f), glow_length_factor(1.0f), distortion_rad_factor(1.0f), distortion_length_factor(1.0f),
					draw_distortion(true)
				{}
} mst_info;

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
float model_find_closest_point(vec3d *outpnt, int model_num, int submodel_num, const matrix *orient, const vec3d *pos, const vec3d *eye_pos);

// Like the above, but finds the closest two points to each other.
float model_find_closest_points(vec3d *outpnt1, int model_num1, int submodel_num1, const matrix *orient1, const vec3d *pos1, vec3d *outpnt2, int model_num2, int submodel_num2, const matrix *orient2, const vec3d *pos2);

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

void model_do_intrinsic_motions(object *objp);

int model_should_render_engine_glow(int objnum, int bank_obj);

bool model_get_team_color(team_color *clr, const SCP_string &team, const SCP_string &secondaryteam, fix timestamp, int fadetime);

void moldel_calc_facing_pts( vec3d *top, vec3d *bot, vec3d *fvec, vec3d *pos, float w, float z_add, vec3d *Eyeposition );

void model_draw_debug_points( polymodel *pm, bsp_info * submodel, uint flags );

void model_render_shields( polymodel * pm, uint flags );

void model_draw_paths_htl( int model_num, uint flags );

void model_draw_bay_paths_htl(int model_num);

bool model_interp_config_buffer(indexed_vertex_source *vert_src, vertex_buffer *vb, bool update_ibuffer_only);
bool model_interp_pack_buffer(indexed_vertex_source *vert_src, vertex_buffer *vb);
void model_interp_submit_buffers(indexed_vertex_source *vert_src, size_t vertex_stride);
void model_allocate_interp_data(int n_verts = 0, int n_norms = 0);

void glowpoint_init();
SCP_vector<glow_point_bank_override>::iterator get_glowpoint_bank_override_by_name(const char* name);
extern SCP_vector<glow_point_bank_override> glowpoint_bank_overrides;

#endif // _MODEL_H
