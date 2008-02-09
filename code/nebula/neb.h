/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Nebula/Neb.h $
 * $Revision: 2.4 $
 * $Date: 2004-08-11 05:06:29 $
 * $Author: Kazan $
 *
 * Nebula effect
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2004/03/05 09:02:07  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/10/14 17:39:16  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.1  2003/09/26 14:37:15  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 16    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 15    7/29/99 12:05a Dave
 * Nebula speed optimizations.
 * 
 * 14    7/26/99 10:12a Dave
 * Upped the max # of nebula backgrounds.
 * 
 * 13    5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _FS2_NEB2_EFFECT_HEADER_FILE
#define _FS2_NEB2_EFFECT_HEADER_FILE

// --------------------------------------------------------------------------------------------------------
// NEBULA DEFINES/VARS
//
#include "globalincs/pstypes.h"
#include "globalincs/globals.h"

struct ship;
struct object;

// fog near and far values for rendering the background nebula
extern float Neb_backg_fog_near;
extern float Neb_backg_fog_far;

// nebula rendering mode
#define NEB2_RENDER_NONE								0			// no rendering
#define NEB2_RENDER_POLY								1			// background is the old-school polygons
#define NEB2_RENDER_POF									2			// background is the nice pof file
#define NEB2_RENDER_LAME								3			// super simple nebula effect 
#define NEB2_RENDER_HTL									4			// We are using proper fogging now 
extern int Neb2_render_mode;

// the AWACS suppresion level for the nebula
extern float Neb2_awacs;

#define MAX_NEB2_POOFS				6

// poof names and flags (for fred)
extern char Neb2_poof_filenames[MAX_NEB2_POOFS][MAX_FILENAME_LEN];	
extern int Neb2_poof_flags;

#define MAX_NEB2_BITMAPS			10

// pof texture filenames
extern char Neb2_bitmap_filenames[MAX_NEB2_BITMAPS][MAX_FILENAME_LEN];

// texture to use for this level
extern char Neb2_texture_name[MAX_FILENAME_LEN];

// how many "slices" are in the current player nebuls
extern int Neb2_slices;

// nebula poofs
typedef struct cube_poof {
	vector	pt;				// point in space
	int		bmap;				// bitmap in space
	float		rot;				// rotation angle
	float		rot_speed;		// rotation speed
	float		flash;			// lightning flash
} cube_poof;
#define MAX_CPTS		5		// should always be <= slices
extern cube_poof Neb2_cubes[MAX_CPTS][MAX_CPTS][MAX_CPTS];


// --------------------------------------------------------------------------------------------------------
// NEBULA FUNCTIONS
//

// neb2 stuff (specific nebula densities) -----------------------------------

// initialize neb2 stuff at game startup
void neb2_init();

// set detail level
void neb2_set_detail_level(int level);

// initialize nebula stuff - call from game_post_level_init(), so the mission has been loaded
void neb2_level_init();

// shutdown nebula stuff
void neb2_level_close();

// create a nebula object, return objnum of the nebula or -1 on fail
// NOTE : in most cases you will want to pass -1.0f for outer_radius. Trust me on this
int neb2_create(vector *offset, int num_poofs, float inner_radius, float outer_radius, float max_poof_radius);

// delete a nebula object
void neb2_delete(object *objp);

// call before beginning all rendering
void neb2_render_setup(vector *eye_pos, matrix *eye_orient);

// renders a nebula object
void neb2_render(object *objp);

// preprocess the nebula object before simulation
void neb2_process_pre(object *objp);

// process the nebula object after simulating, but before rendering
void neb2_process_post(object *objp);

// render the player nebula
void neb2_render_player();

// call this when the player's viewpoint has changed, this will cause the code to properly reset
// the eye's local poofs
void neb2_eye_changed();

// get near and far fog values based upon object type and rendering mode
void neb2_get_fog_values(float *fnear, float *ffar, object *obj);

// given a position in space, return a value from 0.0 to 1.0 representing the fog level 
float neb2_get_fog_intensity(object *obj);
float neb2_get_fog_intensity(vector *pos);

// should we not render this object because its obscured by the nebula?
int neb2_skip_render(object *objp, float z_depth);

// extend LOD 
float neb2_get_lod_scale(int objnum);

// fogging stuff --------------------------------------------------

// get the color of the pixel in the small pre-rendered background nebula
void neb2_get_pixel(int x, int y, int *r, int *g, int *b);

// set the background color
void neb2_set_backg_color(int r, int g, int b);

// get the color to fog the background color to
void neb2_get_backg_color(int *r, int *g, int *b);

void neb2_get_fog_colour(unsigned char *r, unsigned char *g, unsigned char *b);


#endif
