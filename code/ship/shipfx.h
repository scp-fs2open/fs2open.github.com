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

struct object;
struct ship;
struct ship_info;
struct ship_subsys;
struct shockwave_create_info;
struct vec3d;
struct matrix;

// Make sparks fly off of ship n
// sn = spark number to spark, corrosponding to element in
//      ship->hitpos array.  If this isn't -1, it is a just
//      got hit by weapon spark, otherwise pick one randomally.
void shipfx_emit_spark( int n, int sn );

// Does the special effects to blow a subsystem off a ship
extern void shipfx_blow_off_subsystem(object *ship_obj,ship *ship_p,ship_subsys *subsys, vec3d *exp_center);


// Creates "ndebris" pieces of debris on random verts of the the "submodel" in the 
// ship's model.
extern void shipfx_blow_up_model(object *obj,int model, int submodel, int ndebris, vec3d *exp_center);

// put here for multiplayer purposes
void shipfx_blow_up_hull(object *obj,int model, vec3d *exp_center );


// =================================================
//          SHIP WARP IN EFFECT STUFF
// =================================================

// When a ship warps in, this gets called to start the effect
extern void shipfx_warpin_start( object *objp );

// During a ship warp in, this gets called each frame to move the ship
extern void shipfx_warpin_frame( object *objp, float frametime );

// When a ship warps out, this gets called to start the effect
extern void shipfx_warpout_start( object *objp );

// During a ship warp out, this gets called each frame to move the ship
extern void shipfx_warpout_frame( object *objp, float frametime );

// =================================================
//          SHIP SHADOW EFFECT STUFF
// =================================================

// Given point p0, in object's frame of reference, find if 
// it can see the sun.
int shipfx_point_in_shadow( vec3d *p0, matrix *src_orient, vec3d *src_pos, float radius );

// Given an ship see if it is in a shadow.
int shipfx_in_shadow( object * src_obj );

// Given world point see if it is in a shadow.
int shipfx_eye_in_shadow( vec3d *eye_pos, object *src_obj, int sun_n);


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

// Sets the flash lights in the model used by this
// ship to the appropriate values.  There might not
// be any flashes linked to this ship in which
// case this function does nothing.
void shipfx_flash_light_model(object *objp, int model_num);

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

void shipfx_large_blowup_render(ship *shipp);

void shipfx_debris_limit_speed(struct debris *db, ship *shipp);

// sound manager fore big ship sub explosions sounds
void do_sub_expl_sound(float radius, vec3d* sound_pos, int* sound_handle);

// do all shockwaves for a ship blowing up
void shipfx_do_shockwave_stuff(ship *shipp, shockwave_create_info *sci);


// =================================================
//          ELECTRICAL SPARKS ON DAMAGED SHIPS EFFECT STUFF
// =================================================
void shipfx_do_damaged_arcs_frame( ship *shipp );


// =================================================
//				NEBULA LIGHTNING.
// =================================================
void shipfx_do_lightning_frame( ship *shipp );

// engine wash level init
void shipfx_engine_wash_level_init();

// pause engine wash sounds
void shipfx_stop_engine_wash_sound();

// =====================================================
// CLOAKING
// =====================================================

//translate the texture matrix some
void shipfx_cloak_frame(ship *shipp, float frametime);
void shipfx_start_cloak(ship *shipp, int warmup = 5000, int recalc_transform = 0, int device=0);
void shipfx_stop_cloak(ship *shipp, int warpdown = 5000);
float shipfx_calc_visibility(object *obj, vec3d *view_pt);

#define WD_NONE		0
#define WD_WARP_IN	1
#define WD_WARP_OUT	2
float shipfx_calculate_warp_time(object *objp, int warp_dir);
float shipfx_calculate_warp_dist(object *objp);

//********************-----CLASS: WarpEffect-----********************//
class WarpEffect
{
protected:
	//core variables
	object	*objp;
	int		direction;

	//variables provided for expediency
	ship *shipp;
	ship_info *sip;
public:
	WarpEffect();
	WarpEffect(object *n_objp, int n_direction);
	virtual ~WarpEffect() {}

	void clear();
	bool isValid();

	virtual void pageIn();
	virtual void pageOut();

	virtual int warpStart();
	virtual int warpFrame(float frametime);
	virtual int warpShipClip();
	virtual int warpShipRender();
	virtual int warpEnd();

	//For VM_WARP_CHASE
	virtual int getWarpPosition(vec3d *output);
    virtual int getWarpOrientation(matrix *output);
};

//********************-----CLASS: WE_Default-----********************//
#define WE_DEFAULT_NUM_STAGES			2
class WE_Default : public WarpEffect
{
private:
	//portal object
	object *portal_objp;

	//Total data
	int total_time_start;
	int total_time_end;
	//Stage data
	int stage;
	int stage_time_start;
	int	stage_time_end;			// pops when ship is completely warped out or warped in.  Used for both warp in and out.

	//Data "storage"
	int stage_duration[WE_DEFAULT_NUM_STAGES+1];

	//sweeper polygon and clip effect
	vec3d	pos;
	vec3d	fvec;
	float	radius;

public:
	WE_Default(object *n_objp, int n_direction);

	int warpStart();
	int warpFrame(float frametime);
	int warpShipClip();
	int warpShipRender();

	int getWarpPosition(vec3d *output);
    int getWarpOrientation(matrix *output);
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

	geometry_batcher batcher;
	vec3d	autocenter;
	float	z_offset_max;
	float	z_offset_min;
	float	tube_radius;
	float   shockwave_radius;

	//*****Per-instance
	vec3d pos;
	//Sound
	float snd_range_factor;
	int snd_start;
	struct game_snd *snd_start_gs;
	int snd_end;
	struct game_snd *snd_end_gs;

public:
	WE_BSG(object *n_objp, int n_direction);
	~WE_BSG();

	virtual void pageIn();

	virtual int warpStart();
	virtual int warpFrame(float frametime);
	virtual int warpShipClip();
	virtual int warpShipRender();
	virtual int warpEnd();

	virtual int getWarpPosition(vec3d *output);
    virtual int getWarpOrientation(matrix *output);
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
	int snd;
	float snd_range_factor;
	struct game_snd *snd_gs;

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
	WE_Homeworld(object *n_objp, int n_direction);
	virtual ~WE_Homeworld();

	virtual int warpStart();
	virtual int warpFrame(float frametime);
	virtual int warpShipClip();
	virtual int warpShipRender();
	virtual int warpEnd();

	int getWarpPosition(vec3d *output);
    int getWarpOrientation(matrix *output);
};

//********************-----CLASS: WE_Hyperspace----********************//
class WE_Hyperspace : public WarpEffect
{
private:
	//Total data
	int total_time_start;
	int total_duration;
	int total_time_end;
	float accel_exp;
	float decel_exp;

	//sweeper polygon and clip effect
	vec3d	pos_final;
	float	scale_factor;
public:
	WE_Hyperspace(object *n_objp, int n_direction);

	virtual int warpStart();
	virtual int warpFrame(float frametime);
};


#endif
