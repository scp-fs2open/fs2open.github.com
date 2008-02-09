/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Starfield/Nebula.cpp $
 * $Revision: 2.6 $
 * $Date: 2004-07-26 20:47:53 $
 * $Author: Kazan $
 *
 * Code to load & display nebulas
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5  2004/07/12 16:33:07  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.4  2004/03/05 09:02:07  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2004/02/15 06:02:32  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.2  2003/11/11 17:59:27  phreak
 * made some g3_start_instance_matrix calls not affect any of the HT&L graphics
 * calls since they were called for internal calculations for 2D stuff
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 14    9/01/99 11:26p Dave
 * Fixed release build warnings.
 * 
 * 13    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 12    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 11    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 10    6/22/99 7:03p Dave
 * New detail options screen.
 * 
 * 9     3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 8     12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 7     12/08/98 9:36a Dave
 * Almost done nebula effect for D3D. Looks 85% as good as Glide.
 * 
 * 6     12/07/98 5:51p Dave
 * Finally got d3d fog working! Now we just need to tweak values.
 * 
 * 5     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 4     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 3     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 15    5/13/98 2:53p John
 * Made subspace effect work under software.  Had to add new inner loop to
 * tmapper.  Added glows to end of subspace effect.  Made subspace effect
 * levels use gamepalette-subspace palette.
 * 
 * 14    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 13    3/22/98 11:02a John
 * Made a bunch of the detail levels actually do something
 * 
 * 12    2/22/98 12:19p John
 * Externalized some strings
 * 
 * 11    1/23/98 5:08p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 10    1/10/98 1:14p John
 * Added explanation to debug console commands
 * 
 * 9     12/21/97 4:33p John
 * Made debug console functions a class that registers itself
 * automatically, so you don't need to add the function to
 * debugfunctions.cpp.  
 * 
 * 8     11/25/97 11:40a Hoffoss
 * Added support for nebula placement editing.
 * 
 * 7     11/24/97 12:04p John
 * 
 * 6     11/21/97 2:55p Hoffoss
 * Added Nebula support to Fred.  Implemented loading and saving nebula
 * info to/from mission files.
 * 
 * 5     11/21/97 11:32a John
 * Added nebulas.   Fixed some warpout bugs.
 * 
 * 4     11/19/97 10:47p Adam
 * upped MAX_TRIS to 200.  Whaddya think this is, I-War?
 * 
 * 3     11/16/97 2:41p John
 * added a debug function to load a nebula into the game.
 * 
 * 2     11/16/97 2:29p John
 * added versioning to nebulas; put nebula code into freespace.
 * 
 * 1     11/16/97 1:14p John
 *
 * $NoKeywords: $
 */

#include "starfield/nebula.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "cfile/cfile.h"



#define MAX_TRIS 200
#define MAX_POINTS 300

static int neb_w = 0, neb_h = 0;

static int nebula_inited = 0;
static int num_pts = 0;

static vector nebula_vecs[MAX_POINTS];
static vertex nebula_verts[MAX_POINTS];

static float scale_factor = 1.0f;

static int num_tris = 0;
static int tri[MAX_TRIS][3];

static int Nebula_loaded = 0;
static angles Nebula_pbh;
static matrix Nebula_orient;

int Nebula_pitch;
int Nebula_bank;
int Nebula_heading;

void nebula_close()
{
	if (!Nebula_loaded) return;

	Nebula_loaded = 0;
}

#define NEBULA_FILE_ID NOX("NEBU")
#define NEBULA_MAJOR_VERSION 1		// Can be 1-?
#define NEBULA_MINOR_VERSION 0		// Can be 0-99

// given:
// u,v in range 0-1

void project_2d_onto_sphere( vector *pnt, float u, float v )
{
	float a,x,y,z,s;

	a = PI * (2.0f * u - 1.0f );
	z = 2.0f * v - 1.0f;	
	s = scale_factor * fl_sqrt( 1.0f - z*z );
	x = s * (float)cos(a);
	y = s * (float)sin(a);
	pnt->xyz.x = x;
	pnt->xyz.y = y;
	pnt->xyz.z = z;
}

// Version 199 mean major version=1, minor=99.
// Changing major means no longer compatible.
// Revision history:
// 1.00 - initial version

// returns 0 if failed
int load_nebula_sub(char *filename)
{
	CFILE *fp;
	char id[16];
	int version, major, minor;

	fp = cfopen(filename, "rb");

	if ( !fp )	{
		return 0;
	}

	// ID of NEBU
	cfread( id, 4, 1, fp );	
	if ( strncmp( id, NEBULA_FILE_ID, 4))	{
		mprintf(( "Not a valid nebula file.\n" ));
		return 0;
	} 
	cfread( &version, sizeof(int), 1, fp );
	major = version / 100;
	minor = version % 100;

	if ( major != NEBULA_MAJOR_VERSION )	{
		mprintf(( "An out of date nebula file.\n" ));
		return 0;
	}	

	cfread( &num_pts, sizeof(int), 1, fp );
	Assert( num_pts < MAX_POINTS );
	cfread( &num_tris, sizeof(int), 1, fp );
	Assert( num_tris < MAX_TRIS );

	int i;
	for (i=0; i<num_pts; i++ )	{
		float xf, yf;
		int l;

		cfread( &xf, sizeof(float), 1, fp );
		cfread( &yf, sizeof(float), 1, fp );
		cfread( &l, sizeof(int), 1, fp );
		project_2d_onto_sphere( &nebula_vecs[i], 1.0f - xf, yf );
		vm_vec_scale( &nebula_vecs[i], 10.0f );
		nebula_verts[i].b = ubyte((l*255)/31);

		// throw in some randomness to the nebula vertices depth
	}

	for (i=0; i<num_tris; i++ )	{
		cfread( &tri[i][0], sizeof(int), 1, fp );
		cfread( &tri[i][1], sizeof(int), 1, fp );
		cfread( &tri[i][2], sizeof(int), 1, fp );
	}

	cfclose(fp);

	return 1;
}

void nebula_init( char *filename, int pitch, int bank, int heading )
{
	angles a;

	a.p = ANG_TO_RAD((float) pitch);
	a.b = ANG_TO_RAD((float) bank);
	a.h = ANG_TO_RAD((float) heading);
	nebula_init(filename, &a);
}

void nebula_init( char *filename, angles * pbh )
{
	if ( Nebula_loaded )	{
		nebula_close();
	}

	if ( load_nebula_sub( cf_add_ext(filename, NOX(".neb")) ) ) {
		Nebula_loaded = 1;
	}

	if ( pbh ) {
		Nebula_pbh = *pbh;
		vm_angles_2_matrix(&Nebula_orient, &Nebula_pbh );

	} else {
		Nebula_pbh.p = 0.0f;
		Nebula_pbh.b = 0.0f;
		Nebula_pbh.h = 0.0f;
		Nebula_orient = vmd_identity_matrix;
	}
}

void nebula_render()
{
	int i;
	// int r, g, b;

	// no nebula for you!
	return;

	if ( !Nebula_loaded ) {
		return;
	}

	if ( !Detail.planets_suns )	{
		return;
	}	

	// Rotate the nebula.
	g3_start_instance_matrix( NULL, &Nebula_orient, (gr_screen.mode == GR_DIRECT3D));

	for (i=0; i<num_pts; i++ )	{
		g3_rotate_faraway_vertex( &nebula_verts[i], &nebula_vecs[i] );
		g3_project_vertex( &nebula_verts[i] );
	}

	int saved_gr_zbuffering = 	gr_zbuffer_get();

	gr_zbuffer_set(GR_ZBUFF_NONE);

	for (i=0; i<num_tris; i++ ) {

		vertex * verts[3];

		verts[0] = &nebula_verts[tri[i][0]];
		verts[1] = &nebula_verts[tri[i][1]];
		verts[2] = &nebula_verts[tri[i][2]];

		g3_draw_poly(3, verts, TMAP_FLAG_RAMP | TMAP_FLAG_GOURAUD | TMAP_FLAG_NEBULA );
	}		

	g3_done_instance((gr_screen.mode == GR_DIRECT3D));

	gr_zbuffer_set(saved_gr_zbuffering);

	// always switch off fogging for good measure
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode == NEB2_RENDER_NONE)){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}
}

DCF(nebula,"Loads a different nebula")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING|ARG_NONE);
		if ( Dc_arg_type == ARG_NONE )	{
			nebula_close();
		} else {
			nebula_init( Dc_arg );
		}
	}
	if ( Dc_help )	{
		dc_printf( "Usage: nebula filename\nLoads the nebula file. No filename takes away nebula\n" );
	}	
}



