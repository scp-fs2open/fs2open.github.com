/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Asteroid/Asteroid.h $
 * $Revision: 2.12 $
 * $Date: 2005-09-25 20:31:42 $
 * $Author: Goober5000 $
 *
 * Header file for asteroids
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.11  2005/09/25 08:25:14  Goober5000
 * Okay, everything should now work again. :p Still have to do a little more with the asteroids.
 * --Goober5000
 *
 * Revision 2.9  2005/07/13 02:50:48  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.8  2005/07/13 02:01:28  Goober5000
 * fixed a bunch of "issues" caused by me with the species stuff
 * --Goober5000
 *
 * Revision 2.7  2005/07/13 00:44:20  Goober5000
 * improved species support and removed need for #define
 * --Goober5000
 *
 * Revision 2.6  2005/04/05 05:53:14  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.5  2004/08/11 05:06:18  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.4  2004/03/05 09:01:53  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/10/15 22:03:23  Kazan
 * Da Species Update :D
 *
 * Revision 2.2  2003/04/29 01:03:22  Goober5000
 * implemented the custom hitpoints mod
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 12    6/09/99 2:55p Andsager
 * Allow multiple asteroid subtypes (of large, medium, small) and follow
 * family.
 * 
 * 11    6/07/99 1:18p Andsager
 * Make asteroids choose consistent texture from large to small.  Modify
 * fireball radius of dying asteroids.
 * 
 * 10    5/03/99 10:50p Andsager
 * Make Asteroid_obj_list.  Change get_nearest_turret_objnum() to use
 * Asteroid_obj_list, Ship_obj_list and Missile_obj_list vs.
 * obj_used_list.
 * 
 * 9     4/16/99 2:34p Andsager
 * Second pass on debris fields
 * 
 * 8     4/15/99 5:00p Andsager
 * Frist pass on Debris field
 * 
 * 7     3/31/99 9:51a Andsager
 * Add for generalization to debris field
 * 
 * 6     2/07/99 8:51p Andsager
 * Add inner bound to asteroid field.  Inner bound tries to stay astroid
 * free.  Wrap when within and don't throw at ships inside.
 * 
 * 5     1/20/99 6:04p Dave
 * Another bit of stuff for beam weapons. Ships will properly use them
 * now, although they're really deadly.
 * 
 * 4     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 3     10/16/98 1:22p Andsager
 * clean up header files
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 17    3/26/98 9:19a Lawrance
 * Support multiple asteroid pofs
 * 
 * 16    3/17/98 12:16a Allender
 * asteroids in multiplayer -- minor problems with position being correct
 * 
 * 15    3/14/98 1:44p Mike
 * Todolist items 3365..3368.  Make child asteroids collide properly.
 * Make asteroid throwing less bunchy, toss asteroids earlier, make
 * facing-ness not break mission balance.
 * 
 * 14    3/11/98 12:13a Lawrance
 * Auto-target asteroids if no hostile ships present
 * 
 * 13    3/08/98 4:16p Hoffoss
 * 
 * 12    3/07/98 3:48p Lawrance
 * Show offscreen indicators for asteroids
 * 
 * 11    3/04/98 11:59p Lawrance
 * create an asteroid.tbl, read all asteroid data from there
 * 
 * 10    3/04/98 4:11p Lawrance
 * Have area effects affect asteroids, have asteroids cast an area effect,
 * fix ship shockwaves
 * 
 * 9     3/03/98 12:48a Lawrance
 * Ensure collide_objnum is still valid each frame.
 * 
 * 8     3/02/98 11:35p Lawrance
 * Keep track of asteroids that will impact ships on the escort view.
 * 
 * 7     2/26/98 10:07p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 6     2/26/98 4:22p Lawrance
 * Change wrapping behavior, make vel random
 * 
 * 5     2/20/98 8:31p Lawrance
 * delay creation of sub-asteroids
 * 
 * 4     2/19/98 4:33p Lawrance
 * hook in new sounds and custom explosion animation
 * 
 * 3     2/19/98 12:46a Lawrance
 * Further work on asteroids.
 * 
 * 2     2/10/98 6:43p Lawrance
 * Moved asteroid code to a separate lib.
 * 
 * 1     2/10/98 6:05p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __ASTEROID_H__
#define __ASTEROID_H__

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"		// for NAME_LENGTH

struct object;
struct polymodel;
struct collision_info_struct;

#define	MAX_ASTEROIDS			256

#define NUM_DEBRIS_SIZES	3
#define	NUM_DEBRIS_POFS		3				// Number of POFs per debris size

#define	ASTEROID_TYPE_SMALL		0
#define	ASTEROID_TYPE_MEDIUM	1
#define	ASTEROID_TYPE_LARGE		2

// This is for the asteroid types plus DEBRIS_X_Y
// (X is each species and Y is SMALL, MEDIUM, and LARGE)
#define	MAX_DEBRIS_TYPES	((MAX_SPECIES + 1) * NUM_DEBRIS_SIZES)

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


typedef struct asteroid_info {
	char			name[NAME_LENGTH];								// name for the asteroid
	char			pof_files[NUM_DEBRIS_POFS][NAME_LENGTH];		// POF files to load/associate with ship
	int			num_detail_levels;									// number of detail levels for this ship
	int			detail_distance[MAX_ASTEROID_DETAIL_LEVELS];		// distance to change detail levels at
	float			max_speed;												// cap on speed for asteroid
	float			inner_rad;												// radius within which maximum area effect damage is applied
	float			outer_rad;												// radius at which no area effect damage is applied
	float			damage;													// maximum damage applied from area effect explosion
	float			blast;													// maximum blast impulse from area effect explosion									
	float			initial_asteroid_strength;								// starting strength of asteroid
	polymodel	*modelp[NUM_DEBRIS_POFS];
	int			model_num[NUM_DEBRIS_POFS];
} asteroid_info;


#define	AF_USED					(1<<0)			//	Set means used.

typedef	struct asteroid {
	int		flags;
	int		objnum;
	int		asteroid_type;		// 0..MAX_DEBRIS_TYPES
	int		asteroid_subtype;	// Index in asteroid_info for modelnum and modelp
	int		check_for_wrap;		//	timestamp to check for asteroid wrapping around field
	int		check_for_collide;	// timestamp to check for asteroid colliding with escort ships
	int		final_death_time;		// timestamp to swap in new models after explosion starts
	int		collide_objnum;		// set to objnum that asteroid will be impacting soon
	int		collide_objsig;		// object signature corresponding to collide_objnum
	vec3d	death_hit_pos;			// hit pos that caused death
	int		target_objnum;			//	Yes, hah!  Asteroids can have targets.  See asteroid_aim_at_target().
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
	vec3d	min_bound;						//	Minimum range of field.
	vec3d	max_bound;						//	Maximum range of field.
	int		has_inner_bound;
	vec3d	inner_min_bound;
	vec3d	inner_max_bound;
	vec3d	vel;								//	Average asteroid moves at this velocity.
	float		speed;							// Average speed of field
	int		num_initial_asteroids;		//	Number of asteroids at creation.
	field_type_t		field_type;			// active throws and wraps, passive does not
	debris_genre_t	debris_genre;		// type of debris (ship or asteroid)  [generic type]
	int				field_debris_type[MAX_ACTIVE_DEBRIS_TYPES];	// one of the debris type defines above
} asteroid_field;

extern asteroid_info Asteroid_info[MAX_DEBRIS_TYPES];
extern asteroid Asteroids[MAX_ASTEROIDS];
extern asteroid_field	Asteroid_field;

extern int	Num_asteroids;
extern int	Asteroids_enabled;

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
