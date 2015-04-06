/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __ASTEROID_H__
#define __ASTEROID_H__

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"		// for NAME_LENGTH


class object;
class polymodel;
struct collision_info_struct;

#define	MAX_ASTEROIDS			512

#define NUM_DEBRIS_SIZES		3
#define	NUM_DEBRIS_POFS			3				// Number of POFs per debris size

#define	ASTEROID_TYPE_SMALL		0
#define	ASTEROID_TYPE_MEDIUM	1
#define	ASTEROID_TYPE_LARGE		2

// these should always be equal for the benefit of generic asteroids (c.f. asteroid_page_in)
#define	MAX_ACTIVE_DEBRIS_TYPES		NUM_DEBRIS_SIZES

// Goober5000 - currently same as MAX_SHIP_DETAIL_LEVELS (put here to avoid an #include)
#define MAX_ASTEROID_DETAIL_LEVELS	5


// Data structure to track the active asteroids
typedef struct asteroid_obj {
	asteroid_obj *next, *prev;
	int flags, objnum;
} asteroid_obj;
extern asteroid_obj Asteroid_obj_list;


// Data structure for determining a type and amount of other asteroids an
// asteroid will split to when destroyed.
typedef struct asteroid_split_info {
	int		asteroid_type;
	int		min;
	int		max;
} asteroid_split_info;

class asteroid_info
{
public:
	char		name[NAME_LENGTH];								// name for the asteroid
	char		pof_files[NUM_DEBRIS_POFS][MAX_FILENAME_LEN];	// POF files to load/associate with ship
	int			num_detail_levels;								// number of detail levels for this ship
	int			detail_distance[MAX_ASTEROID_DETAIL_LEVELS];	// distance to change detail levels at
	float		max_speed;										// cap on speed for asteroid
	int			damage_type_idx;								//Damage type of the asteroid
	int			damage_type_idx_sav;							// stored value from table used to reset damage_type_idx
	float		inner_rad;										// radius within which maximum area effect damage is applied
	float		outer_rad;										// radius at which no area effect damage is applied
	float		damage;											// maximum damage applied from area effect explosion
	float		blast;											// maximum blast impulse from area effect explosion
	float		initial_asteroid_strength;						// starting strength of asteroid
	SCP_vector< asteroid_split_info > split_info;
	polymodel	*modelp[NUM_DEBRIS_POFS];
	int			model_num[NUM_DEBRIS_POFS];
	SCP_vector<int> explosion_bitmap_anims;
	float		fireball_radius_multiplier;						// the model radius is multiplied by this to determine the fireball size

	asteroid_info( )
		: num_detail_levels( 0 ), max_speed( 0 ), damage_type_idx( 0 ),
		  damage_type_idx_sav( -1 ), inner_rad( 0 ), outer_rad( 0 ),
		  damage( 0 ), blast( 0 ), initial_asteroid_strength( 0 ),
		  fireball_radius_multiplier( -1 )
	{
		name[ 0 ] = 0;
		memset( detail_distance, 0, sizeof( detail_distance ) );

		for (int i = 0; i < NUM_DEBRIS_POFS; i++)
		{
			modelp[i] = NULL;
			model_num[i] = -1;
			pof_files[i][0] = 0;
		}
	}
};


#define	AF_USED					(1<<0)			// Set means used.

typedef	struct asteroid {
	int		flags;
	int		objnum;
	int		asteroid_type;		// 0..MAX_DEBRIS_TYPES
	int		asteroid_subtype;	// Index in asteroid_info for modelnum and modelp
	int		check_for_wrap;		// timestamp to check for asteroid wrapping around field
	int		check_for_collide;	// timestamp to check for asteroid colliding with escort ships
	int		final_death_time;	// timestamp to swap in new models after explosion starts
	int		collide_objnum;		// set to objnum that asteroid will be impacting soon
	int		collide_objsig;		// object signature corresponding to collide_objnum
	vec3d	death_hit_pos;		// hit pos that caused death
	int		target_objnum;		// Yes, hah!  Asteroids can have targets.  See asteroid_aim_at_target().
} asteroid;


// TYPEDEF FOR DEBRIS TYPE
typedef enum {
	DG_ASTEROID,
	DG_SHIP
} debris_genre_t;

// TYPEDEF FOR FIELD TYPE
typedef enum {
	FT_ACTIVE,
	FT_PASSIVE
} field_type_t;

typedef	struct asteroid_field {
	vec3d	min_bound;					// Minimum range of field.
	vec3d	max_bound;					// Maximum range of field.
	float	bound_rad;
	int		has_inner_bound;
	vec3d	inner_min_bound;
	vec3d	inner_max_bound;
	vec3d	vel;						// Average asteroid moves at this velocity.
	float		speed;					// Average speed of field
	int		num_initial_asteroids;		// Number of asteroids at creation.
	field_type_t		field_type;		// active throws and wraps, passive does not
	debris_genre_t	debris_genre;		// type of debris (ship or asteroid)  [generic type]
	int				field_debris_type[MAX_ACTIVE_DEBRIS_TYPES];	// one of the debris type defines above
} asteroid_field;

extern SCP_vector< asteroid_info > Asteroid_info;
extern asteroid Asteroids[MAX_ASTEROIDS];
extern asteroid_field	Asteroid_field;

extern int	Num_asteroids;
extern int	Asteroids_enabled;
extern char		Asteroid_icon_closeup_model[NAME_LENGTH];	// model for asteroid field briefing icon rendering
extern vec3d	Asteroid_icon_closeup_position;  // closeup position for asteroid field briefing icon rendering
extern float	Asteroid_icon_closeup_zoom;		 // zoom position for asteroid field briefing icon rendering

void	asteroid_init();
void	asteroid_level_init();
void	asteroid_level_close();
void	asteroid_create_all();
void	asteroid_render( object *asteroid_objp );
void	asteroid_delete( object *asteroid_objp );
void	asteroid_process_pre( object *asteroid_objp, float frame_time);
void	asteroid_process_post( object *asteroid_objp, float frame_time);
int	asteroid_check_collision( object *asteroid_objp, object * other_obj, vec3d * hitpos, collision_info_struct *asteroid_hit_info=NULL );
void	asteroid_hit( object *asteroid_objp, object *other_objp, vec3d *hitpos, float damage );
int	asteroid_count();
int	asteroid_collide_objnum(object *asteroid_objp);
float asteroid_time_to_impact(object *asteroid_objp);
void	asteroid_show_brackets();
void	asteroid_target_closest_danger();
int	asteroid_get_random_in_cone(vec3d *pos, vec3d *dir, float ang, int danger = 0);

// need to extern for multiplayer
void asteroid_sub_create(object *parent_objp, int asteroid_type, vec3d *relvec);

void asteroid_frame();

#endif	// __ASTEROID_H__
