/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/ds3d.h $
 * $Revision: 2.3 $
 * $Date: 2005-07-13 03:35:29 $
 * $Author: Goober5000 $
 *
 * Header file for interface to DirectSound3D
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.1  2004/08/11 05:06:34  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     5/23/99 8:11p Alanl
 * Added support for EAX
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 8     4/19/98 9:31p Lawrance
 * Use Aureal_enabled flag
 * 
 * 7     7/28/97 11:39a Lawrance
 * allow individual volume scaling on 3D buffers
 * 
 * 6     6/09/97 11:50p Lawrance
 * integrating DirectSound3D
 * 
 * 5     6/08/97 5:59p Lawrance
 * integrate DirectSound3D into sound system
 * 
 * 4     6/02/97 1:45p Lawrance
 * implementing hardware mixing
 * 
 * 3     5/29/97 4:02p Lawrance
 * listener interface in place
 * 
 * 2     5/29/97 12:03p Lawrance
 * creation of file to hold DirectSound3D specific code
 *
 * $NoKeywords: $
 */

#ifndef __DS3D_H__
#define __DS3D_H__

int	ds3d_init(int voice_manager_required);
void	ds3d_close();
int	ds3d_update_listener(vec3d *pos, vec3d *vel, matrix *orient);
int	ds3d_update_buffer(int channel, float min, float max, vec3d *pos, vec3d *vel);
int	ds3d_set_sound_cone(int channel, int inner_angle, int outer_angle, int vol);

#endif /* __DS3D_H__ */
