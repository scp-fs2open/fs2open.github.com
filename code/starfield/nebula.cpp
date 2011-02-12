/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "starfield/nebula.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "cfile/cfile.h"



#define MAX_TRIS 200
#define MAX_POINTS 300

//static int neb_w = 0, neb_h = 0;

//static int nebula_inited = 0;
static int num_pts = 0;

static vec3d nebula_vecs[MAX_POINTS];
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

void project_2d_onto_sphere( vec3d *pnt, float u, float v )
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

	if (Fred_running) {
	// no nebula for you!
	return;
	}

	if ( !Nebula_loaded ) {
		return;
	}

	if ( !Detail.planets_suns )	{
		return;
	}	

	// Rotate the nebula.
	g3_start_instance_matrix( NULL, &Nebula_orient, false);

	for (i=0; i<num_pts; i++ )	{
		g3_rotate_faraway_vertex( &nebula_verts[i], &nebula_vecs[i] );
		g3_project_vertex( &nebula_verts[i] );
	}

	int saved_gr_zbuffering = gr_zbuffer_get();

	gr_zbuffer_set(GR_ZBUFF_NONE);

	for (i=0; i<num_tris; i++ ) {

		vertex * verts[3];

		verts[0] = &nebula_verts[tri[i][0]];
		verts[1] = &nebula_verts[tri[i][1]];
		verts[2] = &nebula_verts[tri[i][2]];

		g3_draw_poly(3, verts, TMAP_FLAG_RAMP | TMAP_FLAG_GOURAUD | TMAP_FLAG_NEBULA );
	}		

	g3_done_instance(false);

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



