/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _SHIPFX_H
#define _SHIPFX_H

#include "globalincs/pstypes.h"
#include "graphics/grbatch.h"
#include "model/modelrender.h"

class object;
class ship;
class ship_info;
struct game_snd;
class ship_subsys;
struct shockwave_create_info;
struct vec3d;
struct matrix;

// Make sparks fly off of ship n
// sn = spark number to spark, corrosponding to element in
//      ship->hitpos array.  If this isn't -1, it is a just
//      got hit by weapon spark, otherwise pick one randomally.
void shipfx_emit_spark( int n, int sn );

// Does the special effects to blow a subsystem off a ship
extern void shipfx_blow_off_subsystem(object *ship_obj, ship *ship_p, ship_subsys *subsys, vec3d *exp_center, bool no_explosion = false);

// Creates "ndebris" pieces of debris on random verts of the "submodel" in the 
// ship's model.
extern void shipfx_blow_up_model(object *obj, int submodel, int ndebris, vec3d *exp_center);


// =================================================
//          SHIP WARP IN EFFECT STUFF
// =================================================

// if we are specifying a Default warp with an index into fireball.tbl, use this flag
#define WT_DEFAULT_WITH_FIREBALL	(1<<31)

// if we have more than one flag defined above, this should mask all of them
#define WT_FLAG_MASK				~(1<<31)

// Warp type defines
#define WT_DEFAULT					0
#define WT_KNOSSOS					1
#define WT_DEFAULT_THEN_KNOSSOS		2
#define WT_IN_PLACE_ANIM			3
#define WT_SWEEPER					4
#define WT_HYPERSPACE				5

extern const char *Warp_types[];
extern int Num_warp_types;
extern int warptype_match(const char *p);

extern void ship_set_warp_effects(object *objp);

enum class WarpDirection { NONE, WARP_IN, WARP_OUT };


// When a ship warps in, this gets called to start the effect
extern void shipfx_warpin_start( object *objp );

// During a ship warp in, this gets called each frame to move the ship
extern void shipfx_warpin_frame( object *objp, float frametime );

// When a ship warps out, this gets called to start the effect
extern void shipfx_warpout_start( object *objp );

// During a ship warp out, this gets called each frame to move the ship
extern void shipfx_warpout_frame( object *objp, float frametime );


// =================================================
//				WARP PARAMS
// =================================================

// WarpParams allows per-ship customization of what was previously in ships.tbl
class WarpParams
{
public:
	WarpDirection	direction = WarpDirection::WARP_IN;

	char		anim[MAX_FILENAME_LEN];
	float		radius = 0.0f;
	gamesnd_id	snd_start;
	gamesnd_id	snd_end;
	float		speed = 0.0f;
	int			time = 0;					// in ms
	float		accel_exp = 1.0f;
	int			warp_type = WT_DEFAULT;

	// only valid for warpout
	int			warpout_engage_time = -1;	// in ms
	float		warpout_player_speed = 0.0f;

	WarpParams();
	bool operator==(const WarpParams &other);
	bool operator!=(const WarpParams &other);
};

extern SCP_vector<WarpParams> Warp_params;

extern int find_or_add_warp_params(const WarpParams &params);

extern float shipfx_calculate_warp_time(object *objp, WarpDirection warp_dir, float half_length, float warping_dist);


// =================================================
//          SHIP SHADOW EFFECT STUFF
// =================================================

// Given world point see if it is in a shadow.
bool shipfx_eye_in_shadow( vec3d *eye_pos, object *src_obj, int sun_n);


// =================================================
//          SHIP GUN FLASH EFFECT STUFF
// =================================================

// Resets the ship flash stuff. Call before
// each level.
void shipfx_flash_init();

// Given that a ship fired a weapon, light up the model
// accordingly.
// Set is_primary to non-zero if this is a primary weapon.
// Gun_pos should be in object's frame of reference, not world!!!
void shipfx_flash_create(object *objp, int model_num, vec3d *gun_pos, vec3d *gun_dir, int is_primary, int weapon_info_index);

// Does whatever processing needs to be done each frame.
void shipfx_flash_do_frame(float frametime);


// =================================================
//          LARGE SHIP EXPLOSION EFFECT STUFF
// =================================================

// Call between levels
void shipfx_large_blowup_level_init();

// Returns 0 if couldn't init
void shipfx_large_blowup_init(ship *shipp);

// Returns 1 when explosion is done
int shipfx_large_blowup_do_frame(ship *shipp, float frametime);

void shipfx_large_blowup_queue_render(model_draw_list *scene, ship* shipp);

void shipfx_debris_limit_speed(struct debris *db, ship *shipp);

// sound manager fore big ship sub explosions sounds
void do_sub_expl_sound(float radius, vec3d* sound_pos, sound_handle* sound_handle);

// do all shockwaves for a ship blowing up
void shipfx_do_shockwave_stuff(ship *shipp, shockwave_create_info *sci);


// =================================================
//          ELECTRICAL SPARKS ON DAMAGED SHIPS EFFECT STUFF
// =================================================
void shipfx_do_lightning_arcs_frame( ship *shipp );


// =================================================
//				NEBULA LIGHTNING
// =================================================
void shipfx_do_lightning_frame( ship *shipp );


// engine wash level init
void shipfx_engine_wash_level_init();

// pause engine wash sounds
void shipfx_stop_engine_wash_sound();


//********************-----CLASS: WarpEffect-----********************//
class WarpEffect
{
protected:
	//core variables
	object	*objp;
	WarpDirection	direction;

	//variables provided for expediency
	ship *shipp;
	ship_info *sip;
	WarpParams *params;

public:
	WarpEffect();
	WarpEffect(object *n_objp, WarpDirection n_direction);
	virtual ~WarpEffect() = default;

	void clear();
	bool isValid();

	virtual void pageIn();
	virtual void pageOut();

	virtual int warpStart();
	virtual int warpFrame(float frametime);
	virtual int warpShipClip(model_render_params *render_info);
	virtual int warpShipRender();
	virtual int warpEnd();

	//For VM_WARP_CHASE
	virtual int getWarpPosition(vec3d *output);
    virtual int getWarpOrientation(matrix *output);
};

bool point_is_clipped_by_warp(const vec3d* point, WarpEffect* warp_effect);

//********************-----CLASS: WE_Default-----********************//
#define WE_DEFAULT_NUM_STAGES			2
class WE_Default : public WarpEffect
{
private:
	//portal object
	object *portal_objp;

	//ship data
	vec3d actual_local_center;	// center of the ship, not necessarily the model origin
	float half_length;			// half the length of the ship, or the docked assembly
	float warping_dist;			// distance to go through the effect (which is the full length)
	float warping_time;			// time to go through the effect
	float warping_speed;		// speed to go through the effect

	void compute_warpout_stuff(float *warp_time, vec3d *warp_pos);

	//Total data
	int total_time_start;
	int total_time_end;
	//Stage data
	int stage_time_start;
	int	stage_time_end;			// pops when ship is completely warped out or warped in.  Used for both warp in and out.

	//Data "storage"
	int stage_duration[WE_DEFAULT_NUM_STAGES+1];

	//sweeper polygon and clip effect
	vec3d	pos;
	vec3d	fvec;
	float	radius;

public:
	WE_Default(object *n_objp, WarpDirection n_direction);

	int warpStart() override;
	int warpFrame(float frametime) override;
	int warpShipClip(model_render_params *render_info) override;
	int warpShipRender() override;

	int getWarpPosition(vec3d *output) override;
    int getWarpOrientation(matrix *output) override;
};

//********************-----CLASS: WE_BSG-----********************//
#define WE_BSG_NUM_STAGES				2
class WE_BSG : public WarpEffect
{
private:
	//Total data
	int total_time_start;
	int total_time_end;
	//Stage data
	int stage;
	int stage_time_start;
	int	stage_time_end;			// pops when ship is completely warped out or warped in.  Used for both warp in and out.

	//Data "storage"
	int stage_duration[WE_BSG_NUM_STAGES];

	//anim
	int anim;
	int anim_nframes;
	int anim_fps;
	int anim_total_time;
	int shockwave;
	int shockwave_nframes;
	int shockwave_fps;
	int shockwave_total_time;

	vec3d	autocenter;
	float	z_offset_max;
	float	z_offset_min;
	float	tube_radius;
	float   shockwave_radius;

	//*****Per-instance
	vec3d pos;
	//Sound
	float snd_range_factor;
	sound_handle snd_start;
	game_snd *snd_start_gs;
	sound_handle snd_end;
	game_snd *snd_end_gs;

public:
	WE_BSG(object *n_objp, WarpDirection n_direction);
	~WE_BSG() override;

	void pageIn() override;

	int warpStart() override;
	int warpFrame(float frametime) override;
	int warpShipClip(model_render_params *render_info) override;
	int warpShipRender() override;
	int warpEnd() override;

	int getWarpPosition(vec3d *output) override;
	int getWarpOrientation(matrix *output) override;
};

//********************-----CLASS: WE_Homeworld-----********************//
#define WE_HOMEWORLD_NUM_STAGES			6
class WE_Homeworld : public WarpEffect
{
private:
	//Total data
	int total_time_start;
	int total_time_end;
	//Stage data
	int stage;
	int stage_time_start;
	int	stage_time_end;			// pops when ship is completely warped out or warped in.  Used for both warp in and out.

	//Data "storage"
	int stage_duration[WE_HOMEWORLD_NUM_STAGES];

	//anim
	int anim;
	int anim_nframes;
	int anim_fps;

	//sound
	sound_handle snd;
	float snd_range_factor;
	game_snd *snd_gs;

	//sweeper polygon and clip effect
	vec3d	pos;
	vec3d	fvec;
	float	width;
	float	width_full;
	float	height;
	float	height_full;
	float	z_offset_min;
	float	z_offset_max;
public:
	WE_Homeworld(object *n_objp, WarpDirection n_direction);
	~WE_Homeworld() override;

	int warpStart() override;
	int warpFrame(float frametime) override;
	int warpShipClip(model_render_params *render_info) override;
	int warpShipRender() override;
	int warpEnd() override;

	int getWarpPosition(vec3d *output) override;
    int getWarpOrientation(matrix *output) override;
};

//********************-----CLASS: WE_Hyperspace----********************//
class WE_Hyperspace : public WarpEffect
{
private:
	//Total data
	int total_time_start;
	int total_duration;
	int total_time_end;
	float accel_or_decel_exp;
	float initial_velocity;

	//sweeper polygon and clip effect
	vec3d	pos_final;
	float	scale_factor;
	
	//Sound
	float snd_range_factor;
	sound_handle snd_start;
	game_snd *snd_start_gs;
	sound_handle snd_end;
	game_snd *snd_end_gs;	
	
public:
	WE_Hyperspace(object *n_objp, WarpDirection n_direction);

	int warpStart() override;
	int warpFrame(float frametime) override;
	int warpEnd() override;	
};


#endif
