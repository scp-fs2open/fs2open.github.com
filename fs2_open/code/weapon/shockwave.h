/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Weapon/Shockwave.h $
 * $Revision: 2.8 $
 * $Date: 2005-07-24 06:01:37 $
 * $Author: wmcoolmon $
 *
 * Header file for creating and managing shockwaves
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.7  2005/07/24 00:32:45  wmcoolmon
 * Synced 3D shockwaves' glowmaps with the model, tossed in some medals.tbl
 * support for the demo/FS1
 *
 * Revision 2.6  2005/07/13 03:35:30  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.5  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.4  2004/08/11 05:06:36  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.3  2004/03/05 09:01:54  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/08/22 07:35:09  bobboau
 * specular code should be bugless now,
 * cell shadeing has been added activated via the comand line '-cell',
 * 3D shockwave models, and a transparency method I'm calling edge and center alpha that could be usefull for other things, ask for details
 *
 * Revision 2.1  2002/08/01 01:41:11  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     8/27/99 1:34a Andsager
 * Modify damage by shockwaves for BIG|HUGE ships.  Modify shockwave damge
 * when weapon blows up.
 * 
 * 6     7/18/99 12:32p Dave
 * Randomly oriented shockwaves.
 * 
 * 5     3/23/99 2:29p Andsager
 * Fix shockwaves for kamikazi and Fred defined.  Collect together
 * shockwave_create_info struct.
 * 
 * 4     2/26/99 4:14p Dave
 * Put in the ability to have multiple shockwaves for ships.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 11    4/15/98 10:17p Mike
 * Training mission #5.
 * Fix application of subsystem damage.
 * 
 * 10    2/26/98 10:08p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 9     2/20/98 8:33p Lawrance
 * Add function to query shockwave max radius
 * 
 * 8     2/14/98 4:43p Lawrance
 * add shockwave_weapon_index() to interface
 * 
 * 7     2/02/98 8:47a Andsager
 * Ship death area damage applied as instantaneous damage for small ships
 * and shockwaves for large (>50 radius) ships.
 * 
 * 6     7/16/97 3:50p Lawrance
 * render shockwaves first, to fake transparency
 * 
 * 5     7/16/97 2:52p Lawrance
 * make shockwaves objects
 * 
 * 4     7/15/97 7:26p Lawrance
 * make shockwave blast persist over time
 * 
 * 3     7/09/97 1:56p Lawrance
 * add savegame support for shockwaves
 * 
 * 2     7/08/97 6:00p Lawrance
 * implementing shockwaves
 * 
 * 1     7/08/97 1:30p Lawrance
 *
 * $NoKeywords: $
 */

#ifndef __SHOCKWAVE_H__
#define __SHOCKWAVE_H__

#include "globalincs/pstypes.h"

struct object;

#define	SW_USED				(1<<0)
#define	SW_WEAPON			(1<<1)
#define	SW_SHIP_DEATH		(1<<2)
#define	SW_WEAPON_KILL		(1<<3)	// Shockwave created when weapon destroyed by another

#define	MAX_SHOCKWAVES					16
#define	MAX_SHOCKWAVE_TYPES				8
#define NUM_DEFAULT_SHOCKWAVE_TYPES		1
#define	SW_MAX_OBJS_HIT	64

typedef struct shockwave_info
{
	int	bitmap_id;
	int	num_frames;
	int	fps;
} shockwave_info;

typedef struct shockwave {
	shockwave	*next, *prev;
	int			flags;
	int			objnum;					// index into Objects[] for shockwave
	int			num_objs_hit;
	int			obj_sig_hitlist[SW_MAX_OBJS_HIT];
	float		speed, radius;
	float		inner_radius, outer_radius, damage;
	int			weapon_info_index;	// -1 if shockwave not caused by weapon	
	vec3d		pos;
	float		blast;					// amount of blast to apply
	int			next_blast;				// timestamp for when to apply next blast damage
	int			shockwave_info_index;
	int			current_bitmap;
	float		time_elapsed;			// in seconds
	float		total_time;				// total lifetime of animation in seconds
	int			delay_stamp;			// for delayed shockwaves
	angles		rot_angles;
	int			model;
} shockwave;

typedef struct shockwave_create_info {
	float inner_rad;
	float outer_rad;
	float damage;
	float blast;
	float speed;
	angles rot_angles;
} shockwave_create_info;

extern shockwave			Shockwaves[MAX_SHOCKWAVES];
extern shockwave_info	Shockwave_info[MAX_SHOCKWAVE_TYPES];

void shockwave_close();
void shockwave_level_init();
void shockwave_level_close();
void shockwave_delete(object *objp);
void shockwave_move_all(float frametime);
int shockwave_create(int parent_objnum, vec3d *pos, shockwave_create_info *sci, int flag, int delay = -1, int model = -1, int info_index=-1);
void shockwave_render(object *objp);
int shockwave_weapon_index(int index);
float shockwave_max_radius(int index);

//Use to add a shockwave and get its info_index
int shockwave_add(char *bm_name);

#endif /* __SHOCKWAVE_H__ */
