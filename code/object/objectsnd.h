/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Object/ObjectSnd.h $
 * $Revision: 2.5 $
 * $Date: 2005-07-13 03:35:31 $
 * $Author: Goober5000 $
 *
 * Header file for managing object-linked persistant sounds
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.4  2005/04/25 00:28:58  wmcoolmon
 * subsystem sounds
 *
 * Revision 2.3  2005/04/05 05:53:21  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.2  2004/08/11 05:06:29  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.1  2004/03/05 09:01:57  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:27  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:11  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     7/01/99 4:23p Dave
 * Full support for multiple linked ambient engine sounds. Added "big
 * damage" flag.
 * 
 * 3     7/01/99 11:44a Dave
 * Updated object sound system to allow multiple obj sounds per ship.
 * Added hit-by-beam sound. Added killed by beam sound.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 11    3/17/98 5:55p Lawrance
 * Support object-linked sounds for asteroids.
 * 
 * 10    9/03/97 5:02p Lawrance
 * add engine stuttering when a ship is dying
 * 
 * 9     7/14/97 12:04a Lawrance
 * make Obj_snd_enabled visible
 * 
 * 8     6/09/97 11:50p Lawrance
 * integrating DirectSound3D
 * 
 * 7     6/06/97 4:13p Lawrance
 * use an index instead of a pointer for object-linked sounds
 * 
 * 6     6/05/97 1:07a Lawrance
 * changes to support sound interface
 * 
 * 5     6/02/97 1:50p Lawrance
 * supporting integration with Direct3D
 * 
 * 4     5/09/97 4:33p Lawrance
 * doppler effects
 * 
 * 3     5/09/97 9:41a Lawrance
 * added #ifndef to avoid multiple inclusions
 * 
 * 2     5/08/97 4:30p Lawrance
 * split off object sound stuff into separate file
 *
 * $NoKeywords: $
 */

#ifndef __OBJECTSND_H__
#define __OBJECTSND_H__

#define	OS_USED					(1<<0)
#define	OS_DS3D					(1<<1)
#define OS_MAIN					(1<<2)		// "main" sound. attentuation does not apply until outside the radius of the object
#define OS_TURRET_BASE_ROTATION	(1<<3)
#define OS_TURRET_GUN_ROTATION	(1<<4)
#define OS_SUBSYS_ALIVE			(1<<5)
#define OS_SUBSYS_DEAD			(1<<6)
#define OS_SUBSYS_DAMAGED		(1<<7)
#define OS_SUBSYS_ROTATION		(1<<8)

struct vec3d;

extern int Obj_snd_enabled;

void	obj_snd_level_init();
void	obj_snd_level_close();
void	obj_snd_do_frame();

// pos is the position of the sound source in the object's frame of reference.
// so, if the objp->pos was at the origin, the pos passed here would be the exact
// model coords of the location of the engine
// by passing vmd_zero_vector here, you get a sound centered directly on the object
// NOTE : if main is true, the attentuation factors don't apply if you're within the radius of the object
int	obj_snd_assign(int objnum, int sndnum, vec3d *pos, int main, int flags=0, struct ship_subsys *associated_sub=NULL);

//Delete specific persistent sound on object
void obj_snd_delete(int objnum, int index);

// if sndnum is not -1, deletes all instances of the given sound within the object
void	obj_snd_delete_type(int objnum, int sndnum = -1, struct ship_subsys *ss = NULL);

void	obj_snd_delete_all();
void	obj_snd_stop_all();
int	obj_snd_is_playing(int objnum, int index);
int	obj_snd_return_instance(int objnum, int index);
int obj_snd_update_offset(int objnum, int index, vec3d *new_offset);

#endif
