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
 * $Revision: 2.3 $
 * $Date: 2003-10-15 22:03:23 $
 * $Author: Kazan $
 *
 * Header file for asteroids
 *
 * $Log: not supported by cvs2svn $
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

#include "ship/ship.h"
#include "parse/parselo.h"		// for NAME_LENGTH

struct object;
struct polymodel;
struct collision_info_struct;

#define	MAX_ASTEROIDS			256

// DEBRIS TYPES


#if defined(MORE_SPECIES)
#define DEBRIS_NAME_MASK 0x000f
// -----------------------------------
// Kazan Extended Species version
// -----------------------------------
#define	MAX_DEBRIS_TYPES		27

#define	ASTEROID_TYPE_SMALL		0
#define	ASTEROID_TYPE_MEDIUM	1
#define	ASTEROID_TYPE_BIG		2
// terran
#define	DEBRIS_TERRAN_SMALL		3
#define	DEBRIS_TERRAN_MEDIUM	4
#define	DEBRIS_TERRAN_LARGE		5
// vasudan
#define	DEBRIS_VASUDAN_SMALL	6
#define	DEBRIS_VASUDAN_MEDIUM	7
#define	DEBRIS_VASUDAN_LARGE	8
// shivan
#define	DEBRIS_SHIVAN_SMALL		9
#define	DEBRIS_SHIVAN_MEDIUM	10
#define	DEBRIS_SHIVAN_LARGE		11
// ancient
#define	DEBRIS_ANCIENT_SMALL	12
#define	DEBRIS_ANCIENT_MEDIUM	13
#define	DEBRIS_ANCIENT_LARGE	14
// user1
#define	DEBRIS_USER1_SMALL		15
#define	DEBRIS_USER1_MEDIUM		16
#define	DEBRIS_USER1_LARGE		17
// user2
#define	DEBRIS_USER2_SMALL		18
#define	DEBRIS_USER2_MEDIUM		19
#define	DEBRIS_USER2_LARGE		20
// user3
#define	DEBRIS_USER3_SMALL		21
#define	DEBRIS_USER3_MEDIUM		22
#define	DEBRIS_USER3_LARGE		23
// user4
#define	DEBRIS_USER4_SMALL		24
#define	DEBRIS_USER4_MEDIUM		25
#define	DEBRIS_USER4_LARGE		26
#else
// -----------------------------------
// Old Volition Version
// -----------------------------------
#define	MAX_DEBRIS_TYPES		12
//
#define	ASTEROID_TYPE_SMALL		0
#define	ASTEROID_TYPE_MEDIUM	1
#define	ASTEROID_TYPE_BIG		2
//
#define	DEBRIS_TERRAN_SMALL		3
#define	DEBRIS_TERRAN_MEDIUM	4
#define	DEBRIS_TERRAN_LARGE		5
//
#define	DEBRIS_VASUDAN_SMALL	6
#define	DEBRIS_VASUDAN_MEDIUM	7
#define	DEBRIS_VASUDAN_LARGE	8
//
#define	DEBRIS_SHIVAN_SMALL		9
#define	DEBRIS_SHIVAN_MEDIUM	10
#define	DEBRIS_SHIVAN_LARGE		11
// END DEBRIS TYPES
#endif

typedef struct debris_struct {
	int index;
	char *name;
} debris_struct;

// Data structure to track the active asteroids
typedef struct asteroid_obj {
	asteroid_obj *next, *prev;
	int flags, objnum;
} asteroid_obj;
extern asteroid_obj Asteroid_obj_list;


extern debris_struct Field_debris_info[];

#define	MAX_ASTEROID_POFS			3				// Max number of POFs per asteroid type

#define	AF_USED					(1<<0)			//	Set means used.

typedef struct asteroid_info {
	char			name[NAME_LENGTH];									// name for the asteroid
	char			pof_files[MAX_ASTEROID_POFS][NAME_LENGTH];	// POF files to load/associate with ship
	int			num_detail_levels;									// number of detail levels for this ship
	int			detail_distance[MAX_SHIP_DETAIL_LEVELS];		// distance to change detail levels at
	float			max_speed;												// cap on speed for asteroid
	float			inner_rad;												// radius within which maximum area effect damage is applied
	float			outer_rad;												// radius at which no area effect damage is applied
	float			damage;													// maximum damage applied from area effect explosion
	float			blast;													// maximum blast impulse from area effect explosion									
	float			initial_asteroid_strength;								// starting strength of asteroid
	polymodel	*modelp[MAX_ASTEROID_POFS];
	int			model_num[MAX_ASTEROID_POFS];
} asteroid_info;

typedef	struct asteroid {
	int		flags;
	int		objnum;
	int		type;						//	In 0..Num_asteroid_types
	int		asteroid_subtype;		// Which index into asteroid_info for modelnum and modelp
	int		check_for_wrap;		//	timestamp to check for asteroid wrapping around field
	int		check_for_collide;	// timestamp to check for asteroid colliding with escort ships
	int		final_death_time;		// timestamp to swap in new models after explosion starts
	int		collide_objnum;		// set to objnum that asteroid will be impacting soon
	int		collide_objsig;		// object signature corresponding to collide_objnum
	vector	death_hit_pos;			// hit pos that caused death
	int		target_objnum;			//	Yes, hah!  Asteroids can have targets.  See asteroid_aim_at_target().
} asteroid;

// TYPEDEF FOR SPECIES OF DEBRIS - BITFIELD
#define DS_TERRAN		0x01
#define DS_VASUDAN		0x02
#define DS_SHIVAN		0x04
#define DS_ANCIENT		0x08
#define DS_USER1		0x10
#define DS_USER2		0x20
#define DS_USER3		0x40
#define DS_USER4		0x80

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

#define	MAX_ACTIVE_DEBRIS_TYPES	3

typedef	struct asteroid_field {
	vector	min_bound;						//	Minimum range of field.
	vector	max_bound;						//	Maximum range of field.
	int		has_inner_bound;
	vector	inner_min_bound;
	vector	inner_max_bound;
	vector	vel;								//	Average asteroid moves at this velocity.
	float		speed;							// Average speed of field
	int		num_initial_asteroids;		//	Number of asteroids at creation.
	field_type_t		field_type;			// active throws and wraps, passive does not
	debris_genre_t	debris_genre;		// type of debris (ship or asteroid)  [generic type]
	int				field_debris_type[MAX_ACTIVE_DEBRIS_TYPES];	// one of the debris type defines above
} asteroid_field;

extern asteroid_info Asteroid_info[MAX_DEBRIS_TYPES];
extern asteroid Asteroids[MAX_ASTEROIDS];
extern asteroid_field	Asteroid_field;

extern int	Num_asteroid_types;
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
int	asteroid_check_collision( object *asteroid_objp, object * other_obj, vector * hitpos, collision_info_struct *asteroid_hit_info=NULL );
void	asteroid_hit( object *asteroid_objp, object *other_objp, vector *hitpos, float damage );
int	asteroid_count();
int	asteroid_collide_objnum(object *asteroid_objp);
float asteroid_time_to_impact(object *asteroid_objp);
void	asteroid_show_brackets();
void	asteroid_target_closest_danger();
int	asteroid_get_random_in_cone(vector *pos, vector *dir, float ang, int danger = 0);

// need to extern for multiplayer
void asteroid_sub_create(object *parent_objp, int asteroid_type, vector *relvec);

void asteroid_frame();

#endif	// __ASTEROID_H__
