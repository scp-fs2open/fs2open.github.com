/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Lighting/Lighting.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:24 $
 * $Author: penguin $
 *
 * Include file for lighting functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:09  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 3     5/09/99 6:00p Dave
 * Lots of cool new effects. E3 build tweaks.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 13    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 12    2/13/98 5:00p John
 * Made lighting push functions return number of releveent lights.
 * 
 * 11    1/29/98 8:14a John
 * Added support for RGB lighting
 * 
 * 10    1/23/98 5:08p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 9     12/12/97 3:02p John
 * First Rev of Ship Shadows
 * 
 * 8     11/07/97 7:24p John
 * changed lighting to take two ranges.
 * In textest, added test code to draw nebulas
 * 
 * 7     11/04/97 9:19p John
 * Optimized dynamic lighting more.
 * 
 * 6     10/29/97 5:05p John
 * Changed dynamic lighting to only rotate and calculate lighting for
 * point lights that are close to an object.  Changed lower framerate cap
 * from 4 to .5.
 * 
 * 5     4/08/97 5:18p John
 * First rev of decent (dynamic, correct) lighting in FreeSpace.
 * 
 * 4     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 * 
 * 3     1/30/97 9:35a Hoffoss
 * Added header for files.
 *
 * $NoKeywords: $
 */

#ifndef _LIGHTING_H
#define _LIGHTING_H

// Light stuff works like this:
// At the start of the frame, call light_reset.
// For each light source, call light_add_??? functions.
// To calculate lighting, do:
// call light_filter_reset or light_filter.
// set up matrices with g3 functions
// call light_rotatate_all to rotate all valid
// lights into current coordinates.
// call light_apply to fill in lighting for a point.

void light_reset();
void light_set_ambient(float ambient_light);

// Intensity - how strong the light is.  1.0 will cast light around 5meters or so.
// r,g,b - only used for colored lighting. Ignored currently.
void light_add_directional( vector *dir, float intensity, float r, float g, float b );
void light_add_point( vector * pos, float r1, float r2, float intensity, float r, float g, float b, int ignore_objnum );
void light_add_point_unique( vector * pos, float r1, float r2, float intensity, float r, float g, float b, int affected_objnum);
void light_add_tube(vector *p0, vector *p1, float r1, float r2, float intensity, float r, float g, float b, int affected_objnum);
void light_rotate_all();

// Reset the list of lights to point to all lights.
void light_filter_reset();

// Makes a list of only the lights that will affect
// the sphere specified by 'pos' and 'rad' and 'objnum'.
// Returns number of lights active.
int light_filter_push( int objnum, vector *pos, float rad );
int light_filter_push_box( vector *min, vector *max );
void light_filter_pop();

// Applies light to a vertex.   In order for this to work, 
// it assumes that one of light_filter or light_filter_reset
// have been called.  It only uses 'vert' to fill in it's light
// fields.  'pos' is position of point, 'norm' is the norm.
ubyte light_apply( vector *pos, vector * norm, float static_light_val );

// Same as above only does RGB.
void light_apply_rgb( ubyte *param_r, ubyte *param_g, ubyte *param_b, vector *pos, vector * norm, float static_light_val );

// return the # of global light sources
int light_get_global_count();

// Fills direction of global light source N in pos.
// Returns 0 if there is no global light.
int light_get_global_dir(vector *pos, int n);

// Set to non-zero if we're in a shadow.
void light_set_shadow( int state );


#endif
