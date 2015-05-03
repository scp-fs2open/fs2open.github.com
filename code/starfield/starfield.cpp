/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <limits.h>

#include "math/vecmat.h"
#include "render/3d.h"
#include "starfield/starfield.h"
#include "freespace2/freespace.h"
#include "io/timer.h"
#include "starfield/nebula.h"
#include "lighting/lighting.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "starfield/supernova.h"
#include "cmdline/cmdline.h"
#include "parse/parselo.h"
#include "hud/hud.h"
#include "hud/hudtarget.h"
#include "debugconsole/console.h"


#define MAX_DEBRIS_VCLIPS			4
#define DEBRIS_ROT_MIN				10000
#define DEBRIS_ROT_RANGE			8
#define DEBRIS_ROT_RANGE_SCALER		10000
#define RND_MAX_MASK				0x3fff
#define HALF_RND_MAX				0x2000

typedef struct {
	vec3d pos;
	vec3d last_pos;
	int active;
	int vclip;
	float size;
} old_debris;

const int MAX_DEBRIS = 200;
const int MAX_STARS = 2000;
const float MAX_DIST = 50.0f;
const float MAX_DIST_RANGE = 60.0f;
const float MIN_DIST_RANGE = 14.0f;
const float BASE_SIZE = 0.12f;
float BASE_SIZE_NEB = 0.5f;

static int Subspace_model_inner = -1;
static int Subspace_model_outer = -1;
static int Rendering_to_env = 0;

int Num_stars = 500;
fix starfield_timestamp = 0;

#define MAX_FLARE_COUNT 10
#define MAX_FLARE_BMP 6

typedef struct flare_info {
	float pos;
	float scale;
	int tex_num;
} flare_info;

typedef struct flare_bitmap {
	char filename[MAX_FILENAME_LEN];
	int bitmap_id;
} flare_bitmap;


// global info (not individual instances)
typedef struct starfield_bitmap {
	char filename[MAX_FILENAME_LEN];				// bitmap filename
	char glow_filename[MAX_FILENAME_LEN];			// only for suns
	int bitmap_id;									// bitmap handle
	int n_frames;
	int fps;
	int glow_bitmap;								// only for suns
	int glow_n_frames;
	int glow_fps;
	int xparent;
	float r, g, b, i, spec_r, spec_g, spec_b;		// only for suns
	int glare;										// only for suns
	int flare;										// Is there a lens-flare for this sun?
	flare_info flare_infos[MAX_FLARE_COUNT];		// each flare can use a texture in flare_bmp, with different scale
	flare_bitmap flare_bitmaps[MAX_FLARE_BMP];		// bitmaps for different lens flares (can be re-used)
	int n_flares;									// number of flares actually used
	int n_flare_bitmaps;							// number of flare bitmaps available
	int used_this_level;
	int preload;
} starfield_bitmap;

// starfield bitmap instance
typedef struct starfield_bitmap_instance {
	float scale_x, scale_y;							// x and y scale
	int div_x, div_y;								// # of x and y divisions
	angles ang;										// angles from FRED
	int star_bitmap_index;							// index into starfield_bitmap array
	int n_verts;
	vertex *verts;

	starfield_bitmap_instance() : scale_x(1.0f), scale_y(1.0f), div_x(1), div_y(1), star_bitmap_index(0), n_verts(0), verts(NULL) {
		ang.p = 0.0f;
		ang.b = 0.0f;
		ang.h = 0.0f;
    }
} starfield_bitmap_instance;

// for drawing cool stuff on the background - comes from a table
static SCP_vector<starfield_bitmap> Starfield_bitmaps;
static SCP_vector<starfield_bitmap_instance> Starfield_bitmap_instances;

// sun bitmaps and sun glow bitmaps
static SCP_vector<starfield_bitmap> Sun_bitmaps;
static SCP_vector<starfield_bitmap_instance> Suns;

// Goober5000
int Num_backgrounds = 0;
int Cur_background = -1;
background_t Backgrounds[MAX_BACKGROUNDS];

int last_stars_filled = 0;
color star_colors[8];
color star_aacolors[8];

typedef struct star {
	vec3d pos;
	vec3d last_star_pos;
	color col;
} star;

typedef struct vDist {
	int x;
	int y;
} vDist;

star Stars[MAX_STARS];

old_debris odebris[MAX_DEBRIS];


typedef struct debris_vclip {
	int	bm;
	int	nframes;
	char  name[MAX_FILENAME_LEN];
} debris_vclip;
extern debris_vclip Debris_vclips_normal[];
extern debris_vclip Debris_vclips_nebula[];
extern debris_vclip *Debris_vclips;

//XSTR:OFF
debris_vclip Debris_vclips_normal[MAX_DEBRIS_VCLIPS] = { { -1, -1, "debris01" }, { -1, -1, "debris02" }, { -1, -1, "debris03" }, { -1, -1, "debris04" } };
debris_vclip Debris_vclips_nebula[MAX_DEBRIS_VCLIPS] = { { -1, -1, "nebdeb01" }, { -1, -1, "nebdeb02" }, { -1, -1, "nebdeb03" }, { -1, -1, "nebdeb04" } };
debris_vclip *Debris_vclips = Debris_vclips_normal;
//XSTR:ON

int stars_debris_loaded = 0;	// 0 = not loaded, 1 = normal vclips, 2 = nebula vclips

// background data
int Stars_background_inited = 0;			// if we're inited
int Nmodel_num = -1;							// model num
matrix Nmodel_orient = IDENTITY_MATRIX;			// model orientation
int Nmodel_flags = DEFAULT_NMODEL_FLAGS;		// model flags
int Nmodel_bitmap = -1;						// model texture

int Num_debris_normal = 0;
int Num_debris_nebula = 0;

bool Dynamic_environment = false;
bool Motion_debris_override = false;

void stars_release_debris_vclips(debris_vclip *vclips)
{
	int i;

	if (vclips == NULL)
		return;

	for (i = 0; i < MAX_DEBRIS_VCLIPS; i++) {
		if ( (vclips[i].bm >= 0) && bm_release(vclips[i].bm) ) {
			vclips[i].bm = -1;
			vclips[i].nframes = -1;
		}
	}
}

void stars_load_debris_vclips(debris_vclip *vclips)
{
	int i;

	if (vclips == NULL) {
		Int3();
		return;
	}

	for (i = 0; i < MAX_DEBRIS_VCLIPS; i++) {
		vclips[i].bm = bm_load_animation( vclips[i].name, &vclips[i].nframes, NULL, NULL, 1 );

		if ( vclips[i].bm < 0 ) {
			// try loading it as a single bitmap
			vclips[i].bm = bm_load(Debris_vclips[i].name);
			vclips[i].nframes = 1;

			if (vclips[i].bm <= 0) {
				Error( LOCATION, "Couldn't load animation/bitmap '%s'\n", vclips[i].name );
			}
		}
	}
}

void stars_load_debris(int fullneb = 0)
{
	if (Cmdline_nomotiondebris) {
		return;
	}

	// if we're in nebula mode
	if ( fullneb && (stars_debris_loaded != 2) ) {
		stars_release_debris_vclips(Debris_vclips);
		stars_load_debris_vclips(Debris_vclips_nebula);
		Debris_vclips = Debris_vclips_nebula;
		stars_debris_loaded = 2;
	} else if (stars_debris_loaded != 1) {
		stars_release_debris_vclips(Debris_vclips);
		stars_load_debris_vclips(Debris_vclips_normal);
		Debris_vclips = Debris_vclips_normal;
		stars_debris_loaded = 1;
	}
}

const int MAX_PERSPECTIVE_DIVISIONS = 5;
const float p_phi = 10.0f, p_theta = 10.0f;

extern void stars_project_2d_onto_sphere( vec3d *pnt, float rho, float phi, float theta );

static void starfield_create_bitmap_buffer(const int si_idx)
{
	vec3d s_points[MAX_PERSPECTIVE_DIVISIONS+1][MAX_PERSPECTIVE_DIVISIONS+1];
	vec3d t_points[MAX_PERSPECTIVE_DIVISIONS+1][MAX_PERSPECTIVE_DIVISIONS+1];

	vertex v[4];
	matrix m, m_bank;
	int idx, s_idx;
	float ui, vi;
	angles bank_first;

	starfield_bitmap_instance *sbi = &Starfield_bitmap_instances[si_idx];

	angles *a = &sbi->ang;
	float scale_x = sbi->scale_x;
	float scale_y = sbi->scale_y;
	int div_x = sbi->div_x;
	int div_y = sbi->div_y;

	// cap division values
//	div_x = div_x > MAX_PERSPECTIVE_DIVISIONS ? MAX_PERSPECTIVE_DIVISIONS : div_x;
	div_x = 1;
	div_y = div_y > MAX_PERSPECTIVE_DIVISIONS ? MAX_PERSPECTIVE_DIVISIONS : div_y;

	if (sbi->verts != NULL) {
		delete [] sbi->verts;
	}

	sbi->verts = new(std::nothrow) vertex[div_x * div_y * 6];

	if (sbi->verts == NULL) {
		sbi->star_bitmap_index = -1;
		return;
	}

	sbi->n_verts = div_x * div_y * 6;

	// texture increment values
	ui = 1.0f / (float)div_x;
	vi = 1.0f / (float)div_y;	

	// adjust for aspect ratio
	//scale_x *= (gr_screen.clip_aspect + 0.55f); // fudge factor
	//scale_x *= (gr_screen.clip_aspect + (0.7333333f/gr_screen.clip_aspect)); // fudge factor
	scale_x *= 1.883333f; //fudge factor

	float s_phi = 0.5f + (((p_phi * scale_x) / 360.0f) / 2.0f);
	float s_theta = (((p_theta * scale_y) / 360.0f) / 2.0f);
	float d_phi = -(((p_phi * scale_x) / 360.0f) / (float)(div_x));
	float d_theta = -(((p_theta * scale_y) / 360.0f) / (float)(div_y));

	// bank matrix
	bank_first.p = 0.0f;
	bank_first.b = a->b;
	bank_first.h = 0.0f;
	vm_angles_2_matrix(&m_bank, &bank_first);

	// convert angles to matrix
	float b_save = a->b;
	a->b = 0.0f;
	vm_angles_2_matrix(&m, a);
	a->b = b_save;

	// generate the bitmap points
	for(idx=0; idx<=div_x; idx++) {
		for(s_idx=0; s_idx<=div_y; s_idx++) {
			// get world spherical coords
			stars_project_2d_onto_sphere(&s_points[idx][s_idx], 1000.0f, s_phi + ((float)idx*d_phi), s_theta + ((float)s_idx*d_theta));
			
			// bank the bitmap first
			vm_vec_rotate(&t_points[idx][s_idx], &s_points[idx][s_idx], &m_bank);

			// rotate on the sphere
			vm_vec_rotate(&s_points[idx][s_idx], &t_points[idx][s_idx], &m);
		}
	}

	memset(v, 0, sizeof(vertex) * 4);

	int j = 0;

	vertex *verts = sbi->verts;

	for (idx = 0; idx < div_x; idx++) {
		for (s_idx = 0; s_idx < div_y; s_idx++) {
			// stuff texture coords
			v[0].texture_position.u = ui * float(idx);
			v[0].texture_position.v = vi * float(s_idx);

			v[1].texture_position.u = ui * float(idx+1);
			v[1].texture_position.v = vi * float(s_idx);

			v[2].texture_position.u = ui * float(idx+1);
			v[2].texture_position.v = vi * float(s_idx+1);

			v[3].texture_position.u = ui * float(idx);
			v[3].texture_position.v = vi * float(s_idx+1);

			g3_transfer_vertex(&v[0], &s_points[idx][s_idx]);
			g3_transfer_vertex(&v[1], &s_points[idx+1][s_idx]);
			g3_transfer_vertex(&v[2], &s_points[idx+1][s_idx+1]);
			g3_transfer_vertex(&v[3], &s_points[idx][s_idx+1]);

			// poly 1
			verts[j++] = v[0];
			verts[j++] = v[1];
			verts[j++] = v[2];
			// poly 2
			verts[j++] = v[0];
			verts[j++] = v[2];
			verts[j++] = v[3];
		}
	}

	Assert( j == sbi->n_verts );
}

// take the Starfield_bitmap_instances[] and make all the vertex buffers that you'll need to draw it 
static void starfield_generate_bitmap_buffers()
{
	int idx;

	int sb_instances = (int)Starfield_bitmap_instances.size();

	for (idx = 0; idx < sb_instances; idx++) {
		if (Starfield_bitmap_instances[idx].star_bitmap_index < 0) {
			continue;
		}

		starfield_create_bitmap_buffer(idx);
	}
}

static void starfield_bitmap_entry_init(starfield_bitmap *sbm)
{
	int i;

	Assert( sbm != NULL );

	memset( sbm, 0, sizeof(starfield_bitmap) );

	sbm->bitmap_id = -1;
	sbm->glow_bitmap = -1;
	sbm->glow_n_frames = 1;

	for (i = 0; i < MAX_FLARE_BMP; i++) {
		sbm->flare_bitmaps[i].bitmap_id = -1;
	}

	for (i = 0; i < MAX_FLARE_COUNT; i++) {
		sbm->flare_infos[i].tex_num = -1;
	}
}

#define CHECK_END() {	\
	if (in_check) {	\
		required_string("#end");	\
		in_check = false;	\
		run_count = 0;	\
	}	\
}

void parse_startbl(const char *filename)
{
	char name[MAX_FILENAME_LEN], tempf[16];
	starfield_bitmap sbm;
	int idx;
	bool in_check = false;
	int rc = -1;
	int run_count = 0;

	try
	{
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();

		// freaky! ;)
		while (!check_for_eof()) {
			while ((rc = optional_string_either("$Bitmap:", "$BitmapX:")) != -1) {
				in_check = true;

				starfield_bitmap_entry_init(&sbm);

				stuff_string(sbm.filename, F_NAME, MAX_FILENAME_LEN);
				sbm.xparent = rc;  // 0 == intensity alpha bitmap,  1 == green xparency bitmap

				if ((idx = stars_find_bitmap(sbm.filename)) >= 0) {
					if (sbm.xparent == Starfield_bitmaps[idx].xparent) {
						if (!Parsing_modular_table)
							Warning(LOCATION, "Starfield bitmap '%s' listed more than once!!  Only using the first entry!", sbm.filename);
					}
					else {
						Warning(LOCATION, "Starfield bitmap '%s' already listed as a %sxparent bitmap!!  Only using the xparent version!",
							(rc) ? "xparent" : "non-xparent", (rc) ? "xparent" : "non-xparent", sbm.filename);
					}
				}
				else {
					Starfield_bitmaps.push_back(sbm);
				}
			}

			CHECK_END();

			while (optional_string("$Sun:")) {
				in_check = true;

				starfield_bitmap_entry_init(&sbm);

				stuff_string(sbm.filename, F_NAME, MAX_FILENAME_LEN);

				// associated glow
				required_string("$Sunglow:");
				stuff_string(sbm.glow_filename, F_NAME, MAX_FILENAME_LEN);

				// associated lighting values
				required_string("$SunRGBI:");
				stuff_float(&sbm.r);
				stuff_float(&sbm.g);
				stuff_float(&sbm.b);
				stuff_float(&sbm.i);

				if (optional_string("$SunSpecularRGB:")) {
					stuff_float(&sbm.spec_r);
					stuff_float(&sbm.spec_g);
					stuff_float(&sbm.spec_b);
				}
				else {
					sbm.spec_r = sbm.r;
					sbm.spec_g = sbm.g;
					sbm.spec_b = sbm.b;
				}

				// lens flare stuff
				if (optional_string("$Flare:")) {
					sbm.flare = 1;

					required_string("+FlareCount:");
					stuff_int(&sbm.n_flares);

					// if there's a flare, it has to have at least one texture
					required_string("$FlareTexture1:");
					stuff_string(sbm.flare_bitmaps[0].filename, F_NAME, MAX_FILENAME_LEN);

					sbm.n_flare_bitmaps = 1;

					for (idx = 1; idx < MAX_FLARE_BMP; idx++) {
						// allow 9999 textures (theoretically speaking, that is)
						sprintf(tempf, "$FlareTexture%d:", idx + 1);

						if (optional_string(tempf)) {
							sbm.n_flare_bitmaps++;
							stuff_string(sbm.flare_bitmaps[idx].filename, F_NAME, MAX_FILENAME_LEN);
						}
						//	else break; //don't allow flaretexture1 and then 3, etc.
					}

					required_string("$FlareGlow1:");

					required_string("+FlareTexture:");
					stuff_int(&sbm.flare_infos[0].tex_num);

					required_string("+FlarePos:");
					stuff_float(&sbm.flare_infos[0].pos);

					required_string("+FlareScale:");
					stuff_float(&sbm.flare_infos[0].scale);

					sbm.n_flares = 1;

					for (idx = 1; idx < MAX_FLARE_COUNT; idx++) {
						// allow a lot of glows
						sprintf(tempf, "$FlareGlow%d:", idx + 1);

						if (optional_string(tempf)) {
							sbm.n_flares++;

							required_string("+FlareTexture:");
							stuff_int(&sbm.flare_infos[idx].tex_num);

							required_string("+FlarePos:");
							stuff_float(&sbm.flare_infos[idx].pos);

							required_string("+FlareScale:");
							stuff_float(&sbm.flare_infos[idx].scale);
						}
						//	else break; //don't allow "flare 1" and then "flare 3"
					}
				}

				sbm.glare = !optional_string("$NoGlare:");

				sbm.xparent = 1;

				if ((idx = stars_find_sun(sbm.filename)) >= 0) {
					if (Parsing_modular_table)
						Sun_bitmaps[idx] = sbm;
					else
						Warning(LOCATION, "Sun bitmap '%s' listed more than once!!  Only using the first entry!", sbm.filename);
				}
				else {
					Sun_bitmaps.push_back(sbm);
				}
			}

			CHECK_END();

			// normal debris pieces
			while (optional_string("$Debris:")) {
				in_check = true;

				stuff_string(name, F_NAME, MAX_FILENAME_LEN);

				if (Num_debris_normal < MAX_DEBRIS_VCLIPS) {
					strcpy_s(Debris_vclips_normal[Num_debris_normal++].name, name);
				}
				else {
					Warning(LOCATION, "Could not load normal motion debris '%s'; maximum of %d exceeded.", name, MAX_DEBRIS_VCLIPS);
				}
			}

			CHECK_END();

			// nebula debris pieces
			while (optional_string("$DebrisNeb:")) {
				in_check = true;

				stuff_string(name, F_NAME, MAX_FILENAME_LEN);

				if (Num_debris_nebula < MAX_DEBRIS_VCLIPS) {
					strcpy_s(Debris_vclips_nebula[Num_debris_nebula++].name, name);
				}
				else {
					Warning(LOCATION, "Could not load nebula motion debris '%s'; maximum of %d exceeded.", name, MAX_DEBRIS_VCLIPS);
				}
			}

			CHECK_END();

			// since it's possible for some idiot to have a tbl screwed up enough
			// that this ends up in an endless loop, give an opportunity to advance
			// through the file no matter what, because even the retail tbl has an
			// extra "#end" line in it.
			if (optional_string("#end") || (run_count++ > 5)) {
				run_count = 0;
				advance_to_eoln(NULL);
			}
		}
	}
	catch (const parse::ParseException& e)
	{
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

void stars_load_all_bitmaps()
{
	int idx, i;
	starfield_bitmap *sb = NULL;
	static int Star_bitmaps_loaded = 0;

	if (Star_bitmaps_loaded)
		return;

	// pre-load all starfield bitmaps.  ONLY SHOULD DO THIS FOR FRED!!
	// this can get nasty when a lot of bitmaps are in use so spare it for
	// the normal game and only do this in FRED
	int mprintf_count = 0;
	for (idx = 0; idx < (int)Starfield_bitmaps.size(); idx++) {
		sb = &Starfield_bitmaps[idx];

		if (sb->bitmap_id < 0) {
			sb->bitmap_id = bm_load(sb->filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, NULL, 1);

				if (sb->bitmap_id < 0) {
					mprintf(("Unable to load starfield bitmap: '%s'!\n", sb->filename));
					mprintf_count++;
				}
			}
		}
	}
	if (mprintf_count > 0) {
		Warning(LOCATION, "Unable to load %d starfield bitmap(s)!\n", mprintf_count);
	}

	for (idx = 0; idx < (int)Sun_bitmaps.size(); idx++) {
		sb = &Sun_bitmaps[idx];

		// normal bitmap
		if (sb->bitmap_id < 0) {
			sb->bitmap_id = bm_load(sb->filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, NULL, 1);

				if (sb->bitmap_id < 0) {
					Warning(LOCATION, "Unable to load sun bitmap: '%s'!\n", sb->filename);
				}
			}
		}

		// glow bitmap
		if (sb->glow_bitmap < 0) {
			sb->glow_bitmap = bm_load(sb->glow_filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->glow_bitmap < 0) {
				sb->glow_bitmap = bm_load_animation(sb->glow_filename, &sb->glow_n_frames, &sb->glow_fps, NULL, 1);

				if (sb->glow_bitmap < 0) {
					Warning(LOCATION, "Unable to load sun glow bitmap: '%s'!\n", sb->glow_filename);
				}
			}
		}

		if (sb->flare) {
			for (i = 0; i < MAX_FLARE_BMP; i++) {
				if ( !strlen(sb->flare_bitmaps[i].filename) )
					continue;

				if (sb->flare_bitmaps[i].bitmap_id < 0) {
					sb->flare_bitmaps[i].bitmap_id = bm_load(sb->flare_bitmaps[i].filename);

					if (sb->flare_bitmaps[i].bitmap_id < 0) {
						Warning(LOCATION, "Unable to load sun flare bitmap: '%s'!\n", sb->flare_bitmaps[i].filename);
						continue;
					}
				}
			}
		}
	}

	Star_bitmaps_loaded = 1;
}

void stars_clear_instances()
{
	for (uint i = 0; i < Starfield_bitmap_instances.size(); i++) {
		delete [] Starfield_bitmap_instances[i].verts;
		Starfield_bitmap_instances[i].verts = NULL;
	}

	Starfield_bitmap_instances.clear();
	Suns.clear();
}

// call on game startup
void stars_init()
{
	// starfield bitmaps
	Num_debris_normal = 0;
	Num_debris_nebula = 0;

	// parse stars.tbl
	parse_startbl("stars.tbl");

	parse_modular_table("*-str.tbm", parse_startbl);
}

// call only from game_shutdown()!!
void stars_close()
{
	stars_clear_instances();

	// any other code goes here
}

// called before mission parse so we can clear out all of the old stuff
void stars_pre_level_init(bool clear_backgrounds)
{
	uint idx, i;
	starfield_bitmap *sb = NULL;

	if (clear_backgrounds)
	{
		// Goober5000 - clear entire array, including backgrounds that will be unused
		for (i = 0; i < MAX_BACKGROUNDS; i++)
		{
			Backgrounds[i].suns.clear();
			Backgrounds[i].bitmaps.clear();
		}
	}

	stars_clear_instances();

	stars_set_background_model(NULL, NULL);
	stars_set_background_orientation();

	// mark all starfield and sun bitmaps as unused for this mission and release any current bitmaps
	// NOTE: that because of how we have to load the bitmaps it's important to release all of
	//       them first thing rather than after we have marked and loaded only what's needed
	// NOTE2: there is a reason that we don't check for release before setting the handle to -1 so
	//        be aware that this is NOT a bug. also, bmpman should NEVER return 0 as a valid handle!
	if ( !Fred_running ) {
		for (idx = 0; idx < Starfield_bitmaps.size(); idx++) {
			sb = &Starfield_bitmaps[idx];

			if (sb->bitmap_id > 0) {
				bm_release(sb->bitmap_id);
				sb->bitmap_id = -1;
			}

			sb->used_this_level = 0;
			sb->preload = 0;
		}

		for (idx = 0; idx < Sun_bitmaps.size(); idx++) {
			sb = &Sun_bitmaps[idx];

			if (sb->bitmap_id > 0) {
				bm_release(sb->bitmap_id);
				sb->bitmap_id = -1;
			}

			if (sb->glow_bitmap > 0) {
				bm_release(sb->glow_bitmap);
				sb->glow_bitmap = -1;
			}

			for (i = 0; i < MAX_FLARE_BMP; i++) {
				if (sb->flare_bitmaps[i].bitmap_id > 0) {
					bm_release(sb->flare_bitmaps[i].bitmap_id);
					sb->flare_bitmaps[i].bitmap_id = -1;
				}
			}

			sb->used_this_level = 0;
			sb->preload = 0;
		}
	}

	Dynamic_environment = false;
	Motion_debris_override = false;
}

// call this in game_post_level_init() so we know whether we're running in full nebula mode or not
void stars_post_level_init()
{
	int i;
	vec3d v;
	float dist, dist_max;
	ubyte red,green,blue,alpha;


	stars_set_background_model(The_mission.skybox_model, NULL, The_mission.skybox_flags);
	stars_set_background_orientation(&The_mission.skybox_orientation);

	stars_load_debris( ((The_mission.flags & MISSION_FLAG_FULLNEB) || Nebula_sexp_used) );

// following code randomly distributes star points within a sphere volume, which
// avoids there being denser areas along the edges and in corners that we had in the
// old rectangular distribution scheme.
	dist_max = (float) (HALF_RND_MAX * HALF_RND_MAX);
	for (i=0; i<MAX_STARS; i++) {
		dist = dist_max;
		while (dist >= dist_max) {
			v.xyz.x = (float) ((myrand() & RND_MAX_MASK) - HALF_RND_MAX);
			v.xyz.y = (float) ((myrand() & RND_MAX_MASK) - HALF_RND_MAX);
			v.xyz.z = (float) ((myrand() & RND_MAX_MASK) - HALF_RND_MAX);

			dist = v.xyz.x * v.xyz.x + v.xyz.y * v.xyz.y + v.xyz.z * v.xyz.z;
		}
		vm_vec_copy_normalize(&Stars[i].pos, &v);

		{
			red= (ubyte)(myrand() % 63 +192);		//192-255
			green= (ubyte)(myrand() % 63 +192);		//192-255
			blue= (ubyte)(myrand() % 63 +192);		//192-255
			alpha = (ubyte)(myrand () % 192 + 24);	//24-216

			gr_init_alphacolor(&Stars[i].col, red, green, blue, alpha, AC_TYPE_BLEND);
		}

	}

	memset( &odebris, 0, sizeof(old_debris) * MAX_DEBRIS );

	
	for (i=0; i<8; i++ ) {
		ubyte intensity = (ubyte)((i + 1) * 24);
		gr_init_alphacolor(&star_aacolors[i], 255, 255, 255, intensity, AC_TYPE_BLEND );
		gr_init_color(&star_colors[i], intensity, intensity, intensity );
	}

	last_stars_filled = 0;

	// if we have no sun instances, create one
	if ( !Suns.size() ) {
		if ( !strlen(Sun_bitmaps[0].filename) ) {
			mprintf(("Trying to add default sun but no default exists!!\n"));
		} else {
			mprintf(("Adding default sun.\n"));

			starfield_bitmap_instance def_sun;

			// stuff some values
			def_sun.ang.h = fl_radians(60.0f);

			Suns.push_back(def_sun);
		}
	}

	// FRED doesn't do normal page_in stuff so we need to load up the bitmaps here instead
	if (Fred_running) {
		stars_load_all_bitmaps();
	}

	starfield_generate_bitmap_buffers();
}


extern object * Player_obj;

#define STAR_AMOUNT_DEFAULT 0.75f
#define STAR_DIM_DEFAULT 7800.0f
#define STAR_CAP_DEFAULT 75.0f
#define STAR_MAX_LENGTH_DEFAULT 0.04f		// 312

float Star_amount = STAR_AMOUNT_DEFAULT;
float Star_dim = STAR_DIM_DEFAULT;
float Star_cap = STAR_CAP_DEFAULT;
float Star_max_length = STAR_MAX_LENGTH_DEFAULT;

#define STAR_FLAG_TAIL			(1<<0)	// Draw a tail when moving
#define STAR_FLAG_DIM			(1<<1)	// Dim as you move
#define STAR_FLAG_ANTIALIAS		(1<<2)	// Draw the star using antialiased lines
#define STAR_FLAG_DEFAULT		(STAR_FLAG_TAIL | STAR_FLAG_DIM)

uint Star_flags = STAR_FLAG_DEFAULT;

//XSTR:OFF
DCF(stars,"Set parameters for starfield")
{
	SCP_string arg;
	float val_f;
	int val_i;

	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: stars keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "stars default   Resets stars to all default values\n" );
		dc_printf( "stars num X     Sets number of stars to X.  Between 0 and %d.\n", MAX_STARS );
		dc_printf( "stars tail X    Where X is the percent of 'tail' between 0 and 1.0\n" );
		dc_printf( "stars dim X     Where X is the amount stars dim between 0 and 255.0\n" );
		dc_printf( "stars cap X     Where X is the cap of dimming between 0 and 255.\n" );
		dc_printf( "stars len X     Where X is the cap of length.\n" );
		dc_printf( "stars m0        Macro0. Old 'pixel type' crappy stars. flags=none\n" );
		dc_printf( "stars m1        Macro1. (default) tail=.75, dim=20.0, cap=75.0, flags=dim,tail\n" );
		dc_printf( "stars m2        Macro2. tail=.75, dim=20.0, cap=75.0, flags=dim,tail,aa\n" );
		dc_printf( "stars flag X    Toggles flag X, where X is tail or dim or aa (aa=antialias)\n" );
		dc_printf( "\nHINT: set cap to 0 to get dim rate and tail down, then use\n" );
		dc_printf( "cap to keep the lines from going away when moving too fast.\n" );
		dc_printf( "\nUse '? stars' to see current values.\n" );
		return;	// don't print status if help is printed.  Too messy.
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf( "Num_stars: %d\n", Num_stars );
		dc_printf( "Tail: %.2f\n", Star_amount );
		dc_printf( "Dim : %.2f\n", Star_dim );
		dc_printf( "Cap : %.2f\n", Star_cap );
		dc_printf( "Max length: %.2f\n", Star_max_length );
		dc_printf( "Flags:\n" );
		dc_printf( "  Tail : %s\n", (Star_flags&STAR_FLAG_TAIL?"On":"Off") );
		dc_printf( "  Dim  : %s\n", (Star_flags&STAR_FLAG_DIM?"On":"Off") );
		dc_printf( "  Antialias: %s\n", (Star_flags&STAR_FLAG_ANTIALIAS?"On":"Off") );
		dc_printf( "\nTHESE AREN'T SAVED TO DISK, SO IF YOU TWEAK\n" );
		dc_printf( "THESE AND LIKE THEM, WRITE THEM DOWN!!\n" );
		return;
	}

	dc_stuff_string_white(arg);
	// "stars default" is handled by "stars m1"
	if (arg == "num") {
		dc_stuff_int(&val_i);

		CLAMP(val_i, 0, MAX_STARS);
		Num_stars = val_i;

		dc_printf("Num_stars set to %i\n", Num_stars);
	
	} else if (arg == "tail") {
		dc_stuff_float(&val_f);
		CLAMP(val_f, 0.0, 1.0);
		Star_amount = val_f;
		
		dc_printf("Star_amount set to %f\n", Star_amount);

	} else if (arg == "dim") {
		dc_stuff_float(&val_f);

		if (val_f > 0.0f ) {
			Star_dim = val_f;
			dc_printf("Star_dim set to %f\n", Star_dim);
		
		} else {
			dc_printf("Error: Star_dim value must be non-negative\n");
		}
	
	} else if (arg == "cap") {
		dc_stuff_float(&val_f);
		CLAMP(val_f, 0.0, 255);
		Star_cap = val_f;
		
		dc_printf("Star_cap set to %f\n", Star_cap);
	
	} else if (arg == "len") {
		dc_stuff_float(&Star_max_length);

		dc_printf("Star_max_length set to %f\n", Star_max_length);

	} else if (arg == "m0") {
		Star_amount = 0.0f;
		Star_dim = 0.0f;
		Star_cap = 0.0f;
		Star_flags = 0;
		Star_max_length = STAR_MAX_LENGTH_DEFAULT;

		dc_printf("Starfield set: Old 'pixel type' crappy stars. flags=none\n");
	
	} else if ((arg == "m1") || (arg == "default")) {
		Star_amount = STAR_AMOUNT_DEFAULT;
		Star_dim = STAR_DIM_DEFAULT;
		Star_cap = STAR_CAP_DEFAULT;
		Star_flags = STAR_FLAG_DEFAULT;
		Star_max_length = STAR_MAX_LENGTH_DEFAULT;

		dc_printf("Starfield set: (default) tail=.75, dim=20.0, cap=75.0, flags=dim,tail\n");

	} else if (arg == "m2") {
		Star_amount = 0.75f;
		Star_dim = 20.0f;
		Star_cap = 75.0f;
		Star_flags = STAR_FLAG_TAIL|STAR_FLAG_DIM|STAR_FLAG_ANTIALIAS;
		Star_max_length = STAR_MAX_LENGTH_DEFAULT;

		dc_printf("Starfield set: tail=.75, dim=20.0, cap=75.0, flags=dim,tail,aa\n");

	} else if (arg == "flag") {
		dc_stuff_string_white(arg);
		if (arg == "tail") {
			Star_flags ^= STAR_FLAG_TAIL;
		} else if (arg == "dim" ) {
			Star_flags ^= STAR_FLAG_DIM;
		} else if (arg == "aa" ) {
			Star_flags ^= STAR_FLAG_ANTIALIAS;
		} else {
			dc_printf("Error: unknown flag argument '%s'\n", arg.c_str());
		}

	} else {
		dc_printf("Error: Unknown argument '%s'", arg.c_str());
	}
}
//XSTR:ON

int reload_old_debris = 1;		// If set to one, then reload all the last_pos of the debris

// Call this if camera "cuts" or moves long distances
// so blur effect doesn't draw lines all over the screen.
void stars_camera_cut()
{
	last_stars_filled = 0;
	reload_old_debris = 1;
}

//#define TIME_STAR_CODE		// enable to time star code

extern int Sun_drew;

// get the world coords of the sun pos on the unit sphere.
void stars_get_sun_pos(int sun_n, vec3d *pos)
{
	vec3d temp;
	matrix rot;

	// sanity
	Assert( sun_n < (int)Suns.size() );

	if ( (sun_n >= (int)Suns.size()) || (sun_n < 0) ) {
		return;
	}

	// rotate the sun properly
	temp = vmd_zero_vector;
	temp.xyz.z = 1.0f;
	
	// rotation matrix
	vm_angles_2_matrix(&rot, &Suns[sun_n].ang);
	vm_vec_rotate(pos, &temp, &rot);
}

// draw sun
void stars_draw_sun(int show_sun)
{	
	int idx;
	vec3d sun_pos;
	vec3d sun_dir;
	vertex sun_vex;
	starfield_bitmap *bm;
	float local_scale;

	// should we even be here?
	if (!show_sun)
		return;

	// no suns drew yet
	Sun_drew = 0;

	// draw all suns
	int num_suns = (int)Suns.size();

	for (idx = 0; idx < num_suns; idx++) {
		// get the instance
		if (Suns[idx].star_bitmap_index < 0)
			return;

		bm = &Sun_bitmaps[Suns[idx].star_bitmap_index];

		// if no bitmap then bail...
		if (bm->bitmap_id < 0)
			continue;

		memset( &sun_vex, 0, sizeof(vertex) );

		// get sun pos
		sun_pos = vmd_zero_vector;
		sun_pos.xyz.y = 1.0f;
		stars_get_sun_pos(idx, &sun_pos);
		
		// get the direction
		sun_dir = sun_pos;
		vm_vec_normalize(&sun_dir);

		// add the light source corresponding to the sun, except when rendering to an envmap
		if ( !Rendering_to_env )
			light_add_directional(&sun_dir, bm->i, bm->r, bm->g, bm->b, bm->spec_r, bm->spec_g, bm->spec_b, true);

		// if supernova
		if ( supernova_active() && (idx == 0) )
			local_scale = 1.0f + (SUPERNOVA_SUN_SCALE * supernova_pct_complete());
		else
			local_scale = 1.0f;

		// draw the sun itself, keep track of how many we drew
		if (bm->fps) {
			gr_set_bitmap(bm->bitmap_id + ((timestamp() / (int)(bm->fps)) % bm->n_frames), GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);
		} else {
			gr_set_bitmap(bm->bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);
		}

		g3_rotate_faraway_vertex(&sun_vex, &sun_pos);

		if ( !g3_draw_bitmap(&sun_vex, 0, 0.05f * Suns[idx].scale_x * local_scale, TMAP_FLAG_TEXTURED) )
			Sun_drew++;
	}
}

// draw a star's lens-flare
void stars_draw_lens_flare(vertex *sun_vex, int sun_n)
{
	starfield_bitmap *bm;
	int i,j;
	float dx,dy;
	vertex flare_vex = *sun_vex; //copy over to flare_vex to get all sorts of properties

	Assert( sun_n < (int)Suns.size() );

	if ( (sun_n >= (int)Suns.size()) || (sun_n < 0) ) {
		return;
	}

	// get the instance
	if (Suns[sun_n].star_bitmap_index < 0) {
		return;
	} else {
		bm = &Sun_bitmaps[Suns[sun_n].star_bitmap_index];
	}

	if (!bm->flare)
		return;
	
	/* (dx,dy) is a 2d vector equal to two times the vector from the sun's
	position to the center fo the screen meaning it is the vector to the 
	opposite position on the screen. */
	dx = 2.0f*(i2fl(gr_screen.clip_right-gr_screen.clip_left)*0.5f - sun_vex->screen.xyw.x);
	dy = 2.0f*(i2fl(gr_screen.clip_bottom-gr_screen.clip_top)*0.5f - sun_vex->screen.xyw.y);

	for (j = 0; j < bm->n_flare_bitmaps; j++)
	{
		// if no bitmap then bail...
		if (bm->flare_bitmaps[j].bitmap_id < 0)
			continue;

		gr_set_bitmap(bm->flare_bitmaps[j].bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);

		for (i = 0; i < bm->n_flares; i++) {
			// draw sorted by texture, to minimize texture changes. not the most efficient way, but better than non-sorted
			if (bm->flare_infos[i].tex_num == j) {
//				gr_set_bitmap(bm->flare_bitmaps[bm->flare_infos[i].tex_num], GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);
				flare_vex.screen.xyw.x = sun_vex->screen.xyw.x + dx * bm->flare_infos[i].pos;
				flare_vex.screen.xyw.y = sun_vex->screen.xyw.y + dy * bm->flare_infos[i].pos;
				g3_draw_bitmap(&flare_vex, 0, 0.05f * bm->flare_infos[i].scale, TMAP_FLAG_TEXTURED);
			}
		}
	}
}

// draw the corresponding glow for sun_n
void stars_draw_sun_glow(int sun_n)
{
	starfield_bitmap *bm;
	vec3d sun_pos, sun_dir;
	vertex sun_vex;	
	float local_scale;

	// sanity
	//WMC - Dunno why this is getting hit...
	//Assert( sun_n < (int)Suns.size() );

	if ( (sun_n >= (int)Suns.size()) || (sun_n < 0) ) {
		return;
	}

	// get the instance
	if (Suns[sun_n].star_bitmap_index < 0)
		return;

	bm = &Sun_bitmaps[Suns[sun_n].star_bitmap_index];

	// if no bitmap then bail...
	if (bm->glow_bitmap < 0)
		return;

	memset( &sun_vex, 0, sizeof(vertex) );

	// get sun pos
	sun_pos = vmd_zero_vector;
	sun_pos.xyz.y = 1.0f;
	stars_get_sun_pos(sun_n, &sun_pos);	

	// get the direction
	sun_dir = sun_pos;
	vm_vec_normalize(&sun_dir);

	// if supernova
	if ( supernova_active() && (sun_n == 0) )
		local_scale = 1.0f + (SUPERNOVA_SUN_SCALE * supernova_pct_complete());
	else
		local_scale = 1.0f;

	// draw the sun itself, keep track of how many we drew
	if (bm->glow_fps) {
		gr_set_bitmap(bm->glow_bitmap + ((timestamp() / (int)(bm->glow_fps)) % bm->glow_n_frames), GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.5f);
	} else {
		gr_set_bitmap(bm->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.5f);
	}

	g3_rotate_faraway_vertex(&sun_vex, &sun_pos);
	int zbuff = gr_zbuffer_set(GR_ZBUFF_NONE);
	g3_draw_bitmap(&sun_vex, 0, 0.10f * Suns[sun_n].scale_x * local_scale, TMAP_FLAG_TEXTURED);

	if (bm->flare) {
		vec3d light_dir;
		vec3d local_light_dir;
		light_get_global_dir(&light_dir, sun_n);
		vm_vec_rotate(&local_light_dir, &light_dir, &Eye_matrix);
		float dot=vm_vec_dot( &light_dir, &Eye_matrix.vec.fvec );
		if (dot > 0.7f) // Only render the flares if the sun is reasonably near the center of the screen
			stars_draw_lens_flare(&sun_vex, sun_n);
	}

	gr_zbuffer_set(zbuff);
}


// draw bitmaps
void stars_draw_bitmaps(int show_bitmaps)
{
	int idx;
	int star_index;

	// should we even be here?
	if ( !show_bitmaps )
		return;

	// if we're in the nebula, don't render any backgrounds
	if (The_mission.flags & MISSION_FLAG_FULLNEB)
		return;

	// detail settings
	if ( !Detail.planets_suns )
		return;

	if ( !Cmdline_nohtl ) {
		gr_start_instance_matrix(&Eye_position, &vmd_identity_matrix);
	}

	// turn off culling
	int cull = gr_set_cull(0);

	// turn off zbuffering
	int saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	int sb_instances = (int)Starfield_bitmap_instances.size();

	for (idx = 0; idx < sb_instances; idx++) {
		// lookup the info index
		star_index = Starfield_bitmap_instances[idx].star_bitmap_index;

		if (star_index < 0) {
			continue;
		}

		// if no bitmap then bail...
		if (Starfield_bitmaps[star_index].bitmap_id < 0) {
			continue;
		}

		int tmap_flags = TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_FLAG_TRILIST | TMAP_HTL_3D_UNLIT;

		if (Starfield_bitmaps[star_index].xparent) {
			if (Starfield_bitmaps[star_index].fps) {
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap_id + ((timestamp() / (int)(Starfield_bitmaps[star_index].fps)) % Starfield_bitmaps[star_index].n_frames));		
			} else {
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap_id);
			}

			tmap_flags |= TMAP_FLAG_XPARENT;
		} else {
			if (Starfield_bitmaps[star_index].fps) {
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap_id + ((timestamp() / (int)(Starfield_bitmaps[star_index].fps)) % Starfield_bitmaps[star_index].n_frames), GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);	
			} else {
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap_id, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);	
			}
		}

		gr_render(Starfield_bitmap_instances[idx].n_verts, Starfield_bitmap_instances[idx].verts, tmap_flags);
	}

	// turn on culling
	gr_set_cull(cull);

	// restore zbuffer
	gr_zbuffer_set(saved_zbuffer_mode);

	if ( !Cmdline_nohtl )
		gr_end_instance_matrix();
}

extern int Interp_subspace;
extern float Interp_subspace_offset_u;
extern float Interp_subspace_offset_u;
extern float Interp_subspace_offset_v;

float subspace_offset_u = 0.0f;
float subspace_offset_u_inner = 0.0f;
float subspace_offset_v = 0.0f;

float subspace_u_speed = 0.07f;			// how fast u changes
float subspace_v_speed = 0.05f;			// how fast v changes

int Subspace_glow_bitmap = -1;

float Subspace_glow_frame = 0.0f;
float Subspace_glow_rate = 1.0f;


//XSTR:OFF
DCF(subspace_set,"Set parameters for subspace effect")
{
	SCP_string arg;
	float value;

	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: subspace [--status] <axis> <speed>\n");
		dc_printf("[--status] -- Displays the current speeds for both axes\n");
		dc_printf("<axis>  -- May be either 'u' or 'v', and corresponds to the texture axis\n");
		dc_printf("<speed> -- is the speed along the axis that the texture is moved\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf( "u: %.2f\n", subspace_u_speed );
		dc_printf( "v: %.2f\n", subspace_v_speed );
		return;
	}

	dc_stuff_string_white(arg);
	if (arg == "u") {
		dc_stuff_float(&value);

		if ( value < 0.0f ) {
			dc_printf("Error: speed must be non-negative");
			return;
		}
		subspace_u_speed = value;

	} else if (arg == "v") {
		dc_stuff_float(&value);

		if (value < 0.0f) {
			dc_printf("Error: speed must be non-negative");
			return;
		}
		subspace_v_speed = value;

	} else {
		dc_printf("Error: Unknown axis '%s'", arg.c_str());
	}
}
//XSTR:ON

void subspace_render()
{
	int framenum = 0;

	if ( Subspace_model_inner == -1 )	{
		Subspace_model_inner = model_load( "subspace_small.pof", 0, NULL );
		Assert(Subspace_model_inner >= 0);
	}

	if ( Subspace_model_outer == -1 )	{
		Subspace_model_outer = model_load( "subspace_big.pof", 0, NULL );
		Assert(Subspace_model_outer >= 0);
	}

	if ( Subspace_glow_bitmap == -1 )	{
		Subspace_glow_bitmap = bm_load( NOX("SunGlow01"));
		Assert(Subspace_glow_bitmap >= 0);
	}

	if ( !Rendering_to_env ) {
		Subspace_glow_frame += flFrametime * 1.0f;

		float total_time = i2fl(NOISE_NUM_FRAMES) / 15.0f;

		// Sanity checks
		if ( Subspace_glow_frame < 0.0f )
			Subspace_glow_frame = 0.0f;
		if ( Subspace_glow_frame > 100.0f )
			Subspace_glow_frame = 0.0f;

		while ( Subspace_glow_frame > total_time ) {
			Subspace_glow_frame -= total_time;
		}

		framenum = fl2i( (Subspace_glow_frame*NOISE_NUM_FRAMES) / total_time );

		if ( framenum < 0 )
			framenum = 0;
		if ( framenum >= NOISE_NUM_FRAMES )
			framenum = NOISE_NUM_FRAMES-1;

		subspace_offset_u += flFrametime*subspace_u_speed;
		if (subspace_offset_u > 1.0f ) {
			subspace_offset_u -= 1.0f;
		}

		subspace_offset_u_inner += flFrametime*subspace_u_speed*3.0f;
		if (subspace_offset_u > 1.0f ) {
			subspace_offset_u -= 1.0f;
		}

		subspace_offset_v += flFrametime*subspace_v_speed;
		if (subspace_offset_v > 1.0f ) {
			subspace_offset_v -= 1.0f;
		}
	}


	matrix tmp;
	angles angs = { 0.0f, 0.0f, 0.0f };

	angs.b = subspace_offset_v * PI2;

	vm_angles_2_matrix(&tmp,&angs);

	int saved_gr_zbuffering = gr_zbuffer_get();

	gr_zbuffer_set(GR_ZBUFF_NONE);

	int render_flags = MR_NO_LIGHTING | MR_ALL_XPARENT;

	Interp_subspace = 1;
	Interp_subspace_offset_u = 1.0f - subspace_offset_u;
	Interp_subspace_offset_v = 0.0f;

	model_set_alpha(1.0f);

	if (!Cmdline_nohtl)
		gr_set_texture_panning(Interp_subspace_offset_v, Interp_subspace_offset_u, true);

	model_render( Subspace_model_outer, &tmp, &Eye_position, render_flags );	//MR_NO_CORRECT|MR_SHOW_OUTLINE

	if (!Cmdline_nohtl)
		gr_set_texture_panning(0, 0, false);

	Interp_subspace = 1;
	Interp_subspace_offset_u = 1.0f - subspace_offset_u_inner;
	Interp_subspace_offset_v = 0.0f;

	angs.b = -subspace_offset_v * PI2;

	vm_angles_2_matrix(&tmp,&angs);

	model_set_outline_color(255,255,255);

	model_set_alpha(1.0f);

	if (!Cmdline_nohtl)
		gr_set_texture_panning(Interp_subspace_offset_v, Interp_subspace_offset_u, true);

	model_render( Subspace_model_inner, &tmp, &Eye_position, render_flags  );	//MR_NO_CORRECT|MR_SHOW_OUTLINE

	if (!Cmdline_nohtl)
		gr_set_texture_panning(0, 0, false);

	//Render subspace glows here and not as thrusters - Valathil 
	vec3d glow_pos;
	vertex glow_vex;	

	glow_pos.xyz.x = 0.0f;
	glow_pos.xyz.y = 0.0f;
	glow_pos.xyz.z = 100.0f;

	gr_set_bitmap(Subspace_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);
	g3_rotate_faraway_vertex(&glow_vex, &glow_pos);
	g3_draw_bitmap(&glow_vex, 0, 17.0f + 0.5f * Noise[framenum], TMAP_FLAG_TEXTURED);

	glow_pos.xyz.z = -100.0f;

	g3_rotate_faraway_vertex(&glow_vex, &glow_pos);
	g3_draw_bitmap(&glow_vex, 0, 17.0f + 0.5f * Noise[framenum], TMAP_FLAG_TEXTURED);

	Interp_subspace = 0;
	gr_zbuffer_set(saved_gr_zbuffering);
}


#ifdef NEW_STARS
//each star is actualy a line
//so each star point is actualy two points
struct star_point {
	star_point() {
		memset(p1.color,INT_MAX,4);
		p2.color[0]=64;
		p2.color[1]=64;
		p2.color[2]=64;
		p2.color[3]=64;
	}
	colored_vector p1,p2;
	void set(vertex* P1,vertex* P2, color*Col) {
		p1.vec.set_screen_vert(*P1);
		p2.vec.set_screen_vert(*P2);
		if(gr_screen.mode == GR_OPENGL)
			memcpy(p1.color, &Col->red,3);
		else {
			memcpy(&p1.color[1], &Col->red,3);
		}
	};
};

class star_point_list {
	int n_points;
	star_point *point_list;
public:
	star_point_list():n_points(0),point_list(NULL){};
	~star_point_list(){if(point_list)delete[]point_list;};

	void allocate(int size){
		if(size<=n_points)return;
		if(point_list)delete[]point_list;
		point_list = new star_point[size];
		n_points = size;
	}
	star_point* operator[] (int idx) {
		return &point_list[idx];
	}
	colored_vector* get_buffer(){return &point_list->p1;};
};

star_point_list star_list;

void new_stars_draw_stars()//don't use me yet, I'm still haveing my API interface figured out-Bobboau
{
	star_list.allocate(Num_stars);

	//Num_stars = 1;
	int i;
	star *sp;
	float dist = 0.0f;
	float ratio;
	int color;
	float colorf;
	vertex p1, p2;
	int can_draw = 1;

	if ( !last_stars_filled ) {
		for (sp=Stars,i=0; i<Num_stars; i++, sp++ ) {
			vertex p2;
			g3_rotate_faraway_vertex(&p2, &sp->pos);
			sp->last_star_pos.xyz.x = p2.x;
			sp->last_star_pos.xyz.y = p2.y;
			sp->last_star_pos.xyz.z = p2.z;
		}
	}

	int tmp_num_stars;

	tmp_num_stars = (Detail.num_stars*Num_stars)/MAX_DETAIL_LEVEL;
	if (tmp_num_stars < 0 ) {
		tmp_num_stars = 0;
	} else if ( tmp_num_stars > Num_stars ) {
		tmp_num_stars = Num_stars;
	}
	
	int stars_to_draw = 0;
	for (sp=Stars,i=0; i<tmp_num_stars; i++, sp++ ) {
		can_draw=1;
		memset(&p1, 0, sizeof(vertex));
		memset(&p2, 0, sizeof(vertex));

		// This makes a star look "proper" by not translating the
		// point around the viewer's eye before rotation.  In other
		// words, when the ship translates, the stars do not change.

		g3_rotate_faraway_vertex(&p2, &sp->pos);
		if ( p2.codes ) {
			can_draw = 0;
		} else {
			g3_project_vertex(&p2);
			if ( p2.flags & PF_OVERFLOW ) {
				can_draw = 0;
			}
		}

		

		if ( can_draw && (Star_flags & (STAR_FLAG_TAIL|STAR_FLAG_DIM)) ) {

			dist = vm_vec_dist_quick( &sp->last_star_pos, (vec3d *)&p2.x );

			if ( dist > Star_max_length ) {
 				ratio = Star_max_length / dist;
				dist = Star_max_length;
			} else {
				ratio = 1.0f;
			}
			
			ratio *= Star_amount;

			p1.x = p2.x + (sp->last_star_pos.xyz.x-p2.x)*ratio;
			p1.y = p2.y + (sp->last_star_pos.xyz.y-p2.y)*ratio;
			p1.z = p2.z + (sp->last_star_pos.xyz.z-p2.z)*ratio;

			p1.flags = 0;	// not projected
			g3_code_vertex( &p1 );

			if ( p1.codes ) {
				can_draw = 0;
			} else {
				g3_project_vertex(&p1);
				if ( p1.flags & PF_OVERFLOW ) {
					can_draw = 0;
				}
			}
		}

		sp->last_star_pos.xyz.x = p2.x;
		sp->last_star_pos.xyz.y = p2.y;
		sp->last_star_pos.xyz.z = p2.z;

		if ( !can_draw )	continue;

		if ( Star_flags & STAR_FLAG_DIM )	{
			colorf = 255.0f - dist*Star_dim;
			if ( colorf < Star_cap )
				colorf = Star_cap;
			color = (fl2i(colorf)*(i&7))/256;
		} else {
			color = i & 7;
		}

		p1.sx-=1.5f;
		p1.sy-=1.5f;
		p2.sx+=1.5f;
		p2.sy+=1.5f;
		star_list[stars_to_draw++]->set(&p2,&p1,&sp->col);
	}
	gr_set_color_fast( &Stars->col );
	gr_zbuffer_set(GR_ZBUFF_NONE);
	gr_draw_line_list(star_list.get_buffer(), stars_to_draw);
	
}
#endif	// NEW_STARS


void stars_draw_stars()
{
	int i;
	star *sp;
	float dist = 0.0f;
	float ratio;
	vDist vDst;
	vertex p1, p2;
	int can_draw = 1;

	if ( !last_stars_filled ) {
		for (i = 0; i < Num_stars; i++) {
			g3_rotate_faraway_vertex(&p2, &Stars[i].pos);
			Stars[i].last_star_pos = p2.world;
		}
	}

	int tmp_num_stars = 0;

	tmp_num_stars = (Detail.num_stars * Num_stars) / MAX_DETAIL_LEVEL;
	CLAMP(tmp_num_stars, 0, Num_stars);


	for (i = 0; i < tmp_num_stars; i++) {
		sp = &Stars[i];

		can_draw = 1;
		memset(&p1, 0, sizeof(vertex));
		memset(&p2, 0, sizeof(vertex));

		// This makes a star look "proper" by not translating the
		// point around the viewer's eye before rotation.  In other
		// words, when the ship translates, the stars do not change.

		g3_rotate_faraway_vertex(&p2, &sp->pos);
		if ( p2.codes )	{
			can_draw = 0;
		} else {
			g3_project_vertex(&p2);
			if ( p2.flags & PF_OVERFLOW ) {
				can_draw = 0;
			}
		}

		if ( can_draw && (Star_flags & (STAR_FLAG_TAIL|STAR_FLAG_DIM)) ) {
			dist = vm_vec_dist_quick( &sp->last_star_pos, &p2.world );

			if ( dist > Star_max_length ) {
 				ratio = Star_max_length / dist;
				dist = Star_max_length;
			} else {
				ratio = 1.0f;
			}
			
			ratio *= Star_amount;

			vm_vec_sub(&p1.world, &sp->last_star_pos, &p2.world);
			vm_vec_scale(&p1.world, ratio);
			vm_vec_add2(&p1.world, &p2.world);

			p1.flags = 0;	// not projected
			g3_code_vertex( &p1 );

			if ( p1.codes )	{
				can_draw = 0;
			} else {
				g3_project_vertex(&p1);
				if ( p1.flags & PF_OVERFLOW ) {
					can_draw = 0;
				}
			}
		}

		sp->last_star_pos = p2.world;

		if ( !can_draw )
			continue;

		gr_set_color_fast( &sp->col );

		vDst.x = fl2i(p1.screen.xyw.x) - fl2i(p2.screen.xyw.x);
		vDst.y = fl2i(p1.screen.xyw.y) - fl2i(p2.screen.xyw.y);

		if ( ((vDst.x * vDst.x) + (vDst.y * vDst.y)) <= 4 ) {
			p1.screen.xyw.x = p2.screen.xyw.x + 1.0f;
			p1.screen.xyw.y = p2.screen.xyw.y;
		}

		gr_aaline(&p1, &p2);
	}
}

void stars_draw_debris()
{
	int i;
	float vdist;
	vec3d tmp;
	vertex p;

	extern bool Motion_debris_override;

	if (Motion_debris_override)
		return;

	gr_set_color( 0, 0, 0 );

	// turn off fogging
	if (The_mission.flags & MISSION_FLAG_FULLNEB) {
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	old_debris * d = odebris; 

	for (i=0; i<MAX_DEBRIS; i++, d++ ) {
		if (!d->active)	{
			d->pos.xyz.x = f2fl(myrand() - RAND_MAX_2);
			d->pos.xyz.y = f2fl(myrand() - RAND_MAX_2);
			d->pos.xyz.z = f2fl(myrand() - RAND_MAX_2);

			vm_vec_normalize(&d->pos);

			vm_vec_scale(&d->pos, MAX_DIST);
			vm_vec_add2(&d->pos, &Eye_position );
			d->active = 1;
			d->vclip = i % MAX_DEBRIS_VCLIPS;	//rand()

			// if we're in full neb mode
			if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)) {
				d->size = i2fl(myrand() % 4)*BASE_SIZE_NEB;
			} else {
				d->size = i2fl(myrand() % 4)*BASE_SIZE;
			}

			vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );
		}

		if ( reload_old_debris ) {
			vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );
		}
			
		g3_rotate_vertex(&p, &d->pos);

		if (p.codes == 0) {
			int frame = Missiontime / (DEBRIS_ROT_MIN + (i % DEBRIS_ROT_RANGE) * DEBRIS_ROT_RANGE_SCALER);
			frame %= Debris_vclips[d->vclip].nframes;

			if ( (The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE) ) {
				gr_set_bitmap( Debris_vclips[d->vclip].bm + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.3f);
			} else {
				gr_set_bitmap( Debris_vclips[d->vclip].bm + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);
			}

			vm_vec_add( &tmp, &d->last_pos, &Eye_position );
			g3_draw_laser( &d->pos,d->size,&tmp,d->size, TMAP_FLAG_TEXTURED|TMAP_FLAG_XPARENT, 25.0f );
		}

		vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );

		vdist = vm_vec_mag_quick(&d->last_pos);

		if ( (vdist < MIN_DIST_RANGE) || (vdist > MAX_DIST_RANGE) )
			d->active = 0;
	}

	reload_old_debris = 0;
}

void stars_draw(int show_stars, int show_suns, int show_nebulas, int show_subspace, int env)
{
	int gr_zbuffering_save = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE); 

	Rendering_to_env = env;

	if (show_subspace)
		subspace_render();

	if (Num_stars >= MAX_STARS)
		Num_stars = MAX_STARS;

#ifdef TIME_STAR_CODE
	fix xt1, xt2;
	xt1 = timer_get_fixed_seconds();
#endif

	if ( show_nebulas && (Game_detail_flags & DETAIL_FLAG_NEBULAS) && (Neb2_render_mode != NEB2_RENDER_POF) && (Neb2_render_mode != NEB2_RENDER_LAME))	{
		nebula_render();
	}

	// draw background stuff
	if ( (Neb2_render_mode != NEB2_RENDER_POLY) && (Neb2_render_mode != NEB2_RENDER_LAME) && show_stars ) {
		// semi-hack, do we don't fog the background
		int neb_save = Neb2_render_mode;
		Neb2_render_mode = NEB2_RENDER_NONE;
		stars_draw_background();
		Neb2_render_mode = neb_save;
	}
	else if ( !show_subspace ) // don't render the background pof when rendering subspace
	{
		stars_draw_background();
	}

	if ( !env && show_stars && (Nmodel_num < 0) && (Game_detail_flags & DETAIL_FLAG_STARS) && !(The_mission.flags & MISSION_FLAG_FULLNEB) && (supernova_active() < 3) ) {
		stars_draw_stars();
	}

	last_stars_filled = 1;

#ifdef TIME_STAR_CODE
	xt2 = timer_get_fixed_seconds();
	mprintf(( "Stars: %d\n", xt2-xt1 ));
#endif

	if ( !Rendering_to_env && (Game_detail_flags & DETAIL_FLAG_MOTION) && (!Fred_running) && (supernova_active() < 3) && (!Cmdline_nomotiondebris) )	{
		stars_draw_debris();
	}

	//if we're not drawing them, quit here
	if (show_suns) {
		stars_draw_sun( show_suns );
		stars_draw_bitmaps( show_suns );
	}

	gr_zbuffer_set( gr_zbuffering_save );
	Rendering_to_env = 0;
}

void stars_preload_sun_bitmap(char *fname)
{
	int idx;

	if (fname == NULL)
		return;

	idx = stars_find_sun(fname);

	if (idx == -1) {
		return;
	}

	Sun_bitmaps[idx].preload = 1;
}

void stars_preload_background_bitmap(char *fname)
{
	int idx;

	if (fname == NULL)
		return;

	idx = stars_find_bitmap(fname);

	if (idx == -1) {
		return;
	}

	Starfield_bitmaps[idx].preload = 1;
}

void stars_page_in()
{
	int idx, i;
	starfield_bitmap_instance *sbi;
	starfield_bitmap *sb;

	// Initialize the subspace stuff

	if ( Game_subspace_effect )	{
		Subspace_model_inner = model_load( "subspace_small.pof", 0, NULL );
		Assert(Subspace_model_inner >= 0);
		Subspace_model_outer = model_load( "subspace_big.pof", 0, NULL );
		Assert(Subspace_model_outer >= 0);

		polymodel *pm;
		
		pm = model_get(Subspace_model_inner);
		
		nprintf(( "Paging", "Paging in textures for subspace effect.\n" ));

		for (idx = 0; idx < pm->n_textures; idx++) {
			pm->maps[idx].PageIn();
		}

		pm = model_get(Subspace_model_outer);
		
		nprintf(( "Paging", "Paging in textures for subspace effect.\n" ));

		for (idx = 0; idx < pm->n_textures; idx++) {
			pm->maps[idx].PageIn();
		}

		if (Subspace_glow_bitmap < 0) {
			Subspace_glow_bitmap = bm_load( NOX("SunGlow01"));
		}

		bm_page_in_xparent_texture(Subspace_glow_bitmap);
	} else {
		Subspace_model_inner = -1;
		Subspace_model_outer = -1;

		if (Subspace_glow_bitmap > 0) {
			bm_release(Subspace_glow_bitmap);
			Subspace_glow_bitmap = -1;
		}
	}

	// extra SEXP related checks to preload anything that might get used from there
	for (idx = 0; idx < (int)Starfield_bitmaps.size(); idx++) {
		sb = &Starfield_bitmaps[idx];

		if (sb->used_this_level)
			continue;

		if (sb->preload) {
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load(sb->filename);

				// maybe didn't load a static image so try for an animated one
				if (sb->bitmap_id < 0) {
					sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, NULL, 1);

					if (sb->bitmap_id < 0) {
						Warning(LOCATION, "Unable to load starfield bitmap: '%s'!\n", sb->filename);
					}
				}
			}

			// this happens whether it loaded properly or not, no harm should come from it
			if (sb->xparent) {
				bm_page_in_xparent_texture(sb->bitmap_id);
			} else {
				bm_page_in_texture(sb->bitmap_id);
			}

			sb->used_this_level++;
		}
	}

	for (idx = 0; idx < (int)Sun_bitmaps.size(); idx++) {
		sb = &Sun_bitmaps[idx];

		if (sb->used_this_level)
			continue;

		if (sb->preload) {
			// normal bitmap
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load(sb->filename);

				// maybe didn't load a static image so try for an animated one
				if (sb->bitmap_id < 0) {
					sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, NULL, 1);

					if (sb->bitmap_id < 0) {
						Warning(LOCATION, "Unable to load sun bitmap: '%s'!\n", sb->filename);
					}
				}
			}

			// glow bitmap
			if (sb->glow_bitmap < 0) {
				sb->glow_bitmap = bm_load(sb->glow_filename);

				// maybe didn't load a static image so try for an animated one
				if (sb->glow_bitmap < 0) {
					sb->glow_bitmap = bm_load_animation(sb->glow_filename, &sb->glow_n_frames, &sb->glow_fps, NULL, 1);

					if (sb->glow_bitmap < 0) {
						Warning(LOCATION, "Unable to load sun glow bitmap: '%s'!\n", sb->glow_filename);
					}
				}
			}

			if (sb->flare) {
				for (i = 0; i < MAX_FLARE_BMP; i++) {
					if ( !strlen(sb->flare_bitmaps[i].filename) )
						continue;

					if (sb->flare_bitmaps[i].bitmap_id < 0) {
						sb->flare_bitmaps[i].bitmap_id = bm_load(sb->flare_bitmaps[i].filename);

						if (sb->flare_bitmaps[i].bitmap_id < 0) {
							Warning(LOCATION, "Unable to load sun flare bitmap: '%s'!\n", sb->flare_bitmaps[i].filename);
							continue;
						}
					}

					bm_page_in_texture(sb->flare_bitmaps[i].bitmap_id);
				}
			}

			bm_page_in_texture(sb->bitmap_id);
			bm_page_in_texture(sb->glow_bitmap);

			sb->used_this_level++;
		}
	}

	// load and page in needed starfield bitmaps
	for (idx = 0; idx < (int)Starfield_bitmap_instances.size(); idx++) {
		sbi = &Starfield_bitmap_instances[idx];

		if (sbi->star_bitmap_index < 0)
			continue;

		sb = &Starfield_bitmaps[sbi->star_bitmap_index];

		if (sb->used_this_level)
			continue;

		if (sb->bitmap_id < 0 ) {
			sb->bitmap_id = bm_load(sb->filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, NULL, 1);

				if (sb->bitmap_id < 0) {
					Warning(LOCATION, "Unable to load starfield bitmap: '%s'!\n", sb->filename);
				}
			}
		}

		// this happens whether it loaded properly or not, no harm should come from it
		if (sb->xparent) {
			bm_page_in_xparent_texture(sb->bitmap_id);
		} else {
			bm_page_in_texture(sb->bitmap_id);
		}

		sb->used_this_level++;
	}

	// now for sun bitmaps and glows
	for (idx = 0; idx < (int)Suns.size(); idx++) {
		sbi = &Suns[idx];

		if (sbi->star_bitmap_index < 0)
			continue;

		sb = &Sun_bitmaps[sbi->star_bitmap_index];

		if (sb->used_this_level)
			continue;

		// normal bitmap
		if (sb->bitmap_id < 0) {
			sb->bitmap_id = bm_load(sb->filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->bitmap_id < 0) {
				sb->bitmap_id = bm_load_animation(sb->filename, &sb->n_frames, &sb->fps, NULL, 1);

				if (sb->bitmap_id < 0) {
					Warning(LOCATION, "Unable to load sun bitmap: '%s'!\n", sb->filename);
				}
			}
		}

		// glow bitmap
		if (sb->glow_bitmap < 0) {
			sb->glow_bitmap = bm_load(sb->glow_filename);

			// maybe didn't load a static image so try for an animated one
			if (sb->glow_bitmap < 0) {
				sb->glow_bitmap = bm_load_animation(sb->glow_filename, &sb->glow_n_frames, &sb->glow_fps, NULL, 1);

				if (sb->glow_bitmap < 0) {
					Warning(LOCATION, "Unable to load sun glow bitmap: '%s'!\n", sb->glow_filename);
				}
			}
		}

		if (sb->flare) {
			for (i = 0; i < MAX_FLARE_BMP; i++) {
				if ( !strlen(sb->flare_bitmaps[i].filename) )
					continue;

				if (sb->flare_bitmaps[i].bitmap_id < 0) {
					sb->flare_bitmaps[i].bitmap_id = bm_load(sb->flare_bitmaps[i].filename);

					if (sb->flare_bitmaps[i].bitmap_id < 0) {
						Warning(LOCATION, "Unable to load sun flare bitmap: '%s'!\n", sb->flare_bitmaps[i].filename);
						continue;
					}
				}

				bm_page_in_texture(sb->flare_bitmaps[i].bitmap_id);
			}
		}

		bm_page_in_texture(sb->bitmap_id);
		bm_page_in_texture(sb->glow_bitmap);

		sb->used_this_level++;
	}


	if (Cmdline_nomotiondebris)
		return;

	for (idx = 0; idx < MAX_DEBRIS_VCLIPS; idx++) {
		bm_page_in_xparent_texture(Debris_vclips[idx].bm, Debris_vclips[idx].nframes);
	}	
}

// background nebula models and planets
void stars_draw_background()
{	
	if (Nmodel_num < 0)
		return;

	if (Nmodel_bitmap >= 0) {
		model_set_forced_texture(Nmodel_bitmap);
		Nmodel_flags |= MR_FORCE_TEXTURE;
	}

	// draw the model at the player's eye with no z-buffering
	model_set_alpha(1.0f);

	model_render(Nmodel_num, &Nmodel_orient, &Eye_position, Nmodel_flags, -1, -1, NULL, true);

	if (Nmodel_bitmap >= 0) {
		model_set_forced_texture(-1);
		Nmodel_flags &= ~MR_FORCE_TEXTURE;
	}
}

// call this to set a specific model as the background model
void stars_set_background_model(char *model_name, char *texture_name, int flags)
{
	if (Nmodel_bitmap >= 0) {
		bm_unload(Nmodel_bitmap);
		Nmodel_bitmap = -1;
	}
	
	if (Nmodel_num >= 0) {
		model_unload(Nmodel_num);
		Nmodel_num = -1;
	}

	Nmodel_flags = flags;

	if ( (model_name == NULL) || (*model_name == '\0') )
		return;

	Nmodel_num = model_load(model_name, 0, NULL, -1);
	Nmodel_bitmap = bm_load(texture_name);

	if (Nmodel_num >= 0)
		model_page_in_textures(Nmodel_num);
}

// call this to set a specific orientation for the background
void stars_set_background_orientation(matrix *orient)
{
	if (orient == NULL) {
		vm_set_identity(&Nmodel_orient);
	} else {
		Nmodel_orient = *orient;
	}
}

// lookup a starfield bitmap, return index or -1 on fail
int stars_find_bitmap(char *name)
{
	int idx;

	if (name == NULL)
		return -1;

	// lookup
	for (idx = 0; idx < (int)Starfield_bitmaps.size(); idx++) {
		if ( !stricmp(name, Starfield_bitmaps[idx].filename) ) {
			return idx;
		}
	}

	// not found 
	return -1;
}

// lookup a sun by bitmap filename, return index or -1 on fail
int stars_find_sun(char *name)
{
	int idx;

	if (name == NULL)
		return -1;

	// lookup
	for (idx = 0; idx < (int)Sun_bitmaps.size(); idx++) {
		if ( !stricmp(name, Sun_bitmaps[idx].filename) ) {
			return idx;
		}
	}

	// not found 
	return -1;
}

// add an instance for a sun (something actually used in a mission)
// NOTE that we assume a duplicate is ok here
int stars_add_sun_entry(starfield_list_entry *sun_ptr)
{
	int idx, i;
	starfield_bitmap_instance sbi;

	Assert(sun_ptr != NULL);

	// copy information
	sbi.ang.p = sun_ptr->ang.p;
	sbi.ang.b = sun_ptr->ang.b;
	sbi.ang.h = sun_ptr->ang.h;
	sbi.scale_x = sun_ptr->scale_x;
	sbi.scale_y = sun_ptr->scale_y;
	sbi.div_x = sun_ptr->div_x;
	sbi.div_y = sun_ptr->div_y;

	idx = stars_find_sun(sun_ptr->filename);

	if (idx == -1) {
		Warning(LOCATION, "Trying to add a sun '%s' that does not exist in stars.tbl!", sun_ptr->filename);
		return -1;
	}

	sbi.star_bitmap_index = idx;

	// make sure any needed bitmaps are loaded
	if (Sun_bitmaps[idx].bitmap_id < 0) {
		// normal bitmap
		Sun_bitmaps[idx].bitmap_id = bm_load(Sun_bitmaps[idx].filename);

			// maybe didn't load a static image so try for an animated one
		if (Sun_bitmaps[idx].bitmap_id < 0) {
			Sun_bitmaps[idx].bitmap_id = bm_load_animation(Sun_bitmaps[idx].filename, &Sun_bitmaps[idx].n_frames, &Sun_bitmaps[idx].fps, NULL, 1);

			if (Sun_bitmaps[idx].bitmap_id < 0) {
				// failed
				return -1;
			}
		}

		// glow bitmap
		if (Sun_bitmaps[idx].glow_bitmap < 0) {
			Sun_bitmaps[idx].glow_bitmap = bm_load(Sun_bitmaps[idx].glow_filename);

			// maybe didn't load a static image so try for an animated one
			if (Sun_bitmaps[idx].glow_bitmap < 0) {
				Sun_bitmaps[idx].glow_bitmap = bm_load_animation(Sun_bitmaps[idx].glow_filename, &Sun_bitmaps[idx].glow_n_frames, &Sun_bitmaps[idx].glow_fps, NULL, 1);

				if (Sun_bitmaps[idx].glow_bitmap < 0) {
					Warning(LOCATION, "Unable to load sun glow bitmap: '%s'!\n", Sun_bitmaps[idx].glow_filename);
				}
			}
		}

		if (Sun_bitmaps[idx].flare) {
			for (i = 0; i < MAX_FLARE_BMP; i++) {
				flare_bitmap* fbp = &Sun_bitmaps[idx].flare_bitmaps[i];
				if ( !strlen(fbp->filename) )
					continue;

				if (fbp->bitmap_id < 0) {
					fbp->bitmap_id = bm_load(fbp->filename);

					if (fbp->bitmap_id < 0) {
						Warning(LOCATION, "Unable to load sun flare bitmap: '%s'!\n", Sun_bitmaps[idx].flare_bitmaps[i].filename);
						continue;
					}
				}
			}
		}
	}

	// now check if we can make use of a previously discarded instance entry
	// this should never happen with FRED
	if ( !Fred_running ) {
		for (i = 0; i < (int)Suns.size(); i++) {
			if ( Suns[i].star_bitmap_index < 0 ) {
				Suns[i] = sbi;
				return i;
			}
		}
	}

	// ... or add a new one 
	Suns.push_back(sbi);

	return Suns.size() - 1;
}

// add an instance for a starfield bitmap (something actually used in a mission)
// NOTE that we assume a duplicate is ok here
int stars_add_bitmap_entry(starfield_list_entry *sle)
{
	int idx;
	starfield_bitmap_instance sbi;

	Assert(sle != NULL);

	// copy information
	sbi.ang.p = sle->ang.p;
	sbi.ang.b = sle->ang.b;
	sbi.ang.h = sle->ang.h;
	sbi.scale_x = sle->scale_x;
	sbi.scale_y = sle->scale_y;
	sbi.div_x = sle->div_x;
	sbi.div_y = sle->div_y;

	idx = stars_find_bitmap(sle->filename);

	if (idx == -1) {
		Warning(LOCATION, "Trying to add a bitmap '%s' that does not exist in stars.tbl!", sle->filename);
		return -1;
	}

	sbi.star_bitmap_index = idx;

	// make sure any needed bitmaps are loaded
	if (Starfield_bitmaps[idx].bitmap_id < 0) {
		Starfield_bitmaps[idx].bitmap_id = bm_load(Starfield_bitmaps[idx].filename);

		// maybe didn't load a static image so try for an animated one
		if (Starfield_bitmaps[idx].bitmap_id < 0) {
			Starfield_bitmaps[idx].bitmap_id = bm_load_animation(Starfield_bitmaps[idx].filename, &Starfield_bitmaps[idx].n_frames, &Starfield_bitmaps[idx].fps, NULL, 1);

			if (Starfield_bitmaps[idx].bitmap_id < 0) {
				// failed
				return -1;
			}
		}
	}

	// now check if we can make use of a previously discarded instance entry
	for (int i = 0; i < (int)Starfield_bitmap_instances.size(); i++) {
		if ( Starfield_bitmap_instances[i].star_bitmap_index < 0 ) {
			// starfield_update_index_buffers(i, 0);
			Starfield_bitmap_instances[i] = sbi;
			starfield_create_bitmap_buffer(i);
			return i;
		}
	}

	// ... or add a new one
	Starfield_bitmap_instances.push_back(sbi);
	starfield_create_bitmap_buffer(Starfield_bitmap_instances.size() - 1);

	return Starfield_bitmap_instances.size() - 1;
}

// get the number of entries that each vector contains
// "is_a_sun" will get sun instance counts, otherwise it gets normal starfield bitmap instance counts
// "bitmap_count" will get number of starfield_bitmap entries rather than starfield_bitmap_instance entries
int stars_get_num_entries(bool is_a_sun, bool bitmap_count)
{
	// try for instance counts first
	if (!bitmap_count) {
		if (is_a_sun) {
			return (int)Suns.size();
		} else {
			return (int)Starfield_bitmap_instances.size();
		}
	}
	// looks like we want bitmap counts (probably only FRED uses this)
	else {
		if (is_a_sun) {
			return (int)Sun_bitmaps.size();
		} else {
			return (int)Starfield_bitmaps.size();
		}
	}
}


// get a starfield_bitmap entry providing only an instance
starfield_bitmap *stars_get_bitmap_entry(int index, bool is_a_sun)
{
	int max_index = (is_a_sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	//WMC - Commented out because it keeps happening, and I don't know what this means.
	//Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return NULL;

	if (is_a_sun && (Suns[index].star_bitmap_index >= 0)) {
		return &Sun_bitmaps[Suns[index].star_bitmap_index];
	} else if (!is_a_sun && (Starfield_bitmap_instances[index].star_bitmap_index >= 0)) {
		return &Starfield_bitmaps[Starfield_bitmap_instances[index].star_bitmap_index];
	}

	return NULL;
}

bool stars_sun_has_glare(int index)
{
	starfield_bitmap *sb = stars_get_bitmap_entry(index, true);
	return (sb && sb->glare);
}

// get a starfield_bitmap_instance, obviously
starfield_bitmap_instance *stars_get_instance(int index, bool is_a_sun)
{
	int max_index = (is_a_sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return NULL;

	if (is_a_sun) {
		return &Suns[index];
	} else {
		return &Starfield_bitmap_instances[index];
	}
}

// set an instace to not render
void stars_mark_instance_unused(int index, bool is_a_sun)
{
	int max_index = (is_a_sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return;

	if (is_a_sun) {
		Suns[index].star_bitmap_index =  -1;
	} else {
		Starfield_bitmap_instances[index].star_bitmap_index = -1;
	}

	if ( !is_a_sun ) {
		delete [] Starfield_bitmap_instances[index].verts;
		Starfield_bitmap_instances[index].verts = NULL;
	}
}

// retrieves the name from starfield_bitmap for the instance index
// NOTE: it's unsafe to return NULL here so use <none> for invalid entries
const char *stars_get_name_from_instance(int index, bool is_a_sun)
{
	int max_index = (is_a_sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return NOX("<none>");

	if (is_a_sun && (Suns[index].star_bitmap_index >= 0)) {
		return Sun_bitmaps[Suns[index].star_bitmap_index].filename;
	} else if (!is_a_sun && (Starfield_bitmap_instances[index].star_bitmap_index >= 0)) {
		return Starfield_bitmaps[Starfield_bitmap_instances[index].star_bitmap_index].filename;
	}

	return NOX("<none>");
}

// WMC/Goober5000
void stars_set_nebula(bool activate)
{
	if (activate)
	{
		The_mission.flags |= MISSION_FLAG_FULLNEB;
		Toggle_text_alpha = TOGGLE_TEXT_NEBULA_ALPHA;
		HUD_contrast = 1;
		if(Cmdline_nohtl || Fred_running) {
			Neb2_render_mode = NEB2_RENDER_POF;
			stars_set_background_model(BACKGROUND_MODEL_FILENAME, Neb2_texture_name);
			stars_set_background_orientation();
		} else {
			Neb2_render_mode = NEB2_RENDER_HTL;
		}
		Debris_vclips = Debris_vclips_nebula;
		neb2_eye_changed();
	}
	else
	{
		The_mission.flags &= ~MISSION_FLAG_FULLNEB;
		Toggle_text_alpha = TOGGLE_TEXT_NORMAL_ALPHA;
		Neb2_render_mode = NEB2_RENDER_NONE;
		Debris_vclips = Debris_vclips_normal;
		HUD_contrast = 0;
	}
}

// retrieves the name from starfield_bitmap, really only used by FRED2
// NOTE: it is unsafe to return NULL here, but because that's bad anyway it really shouldn't happen, so we do return NULL.
const char *stars_get_name_FRED(int index, bool is_a_sun)
{
	if (!Fred_running)
		return NULL;

	int max_index = (is_a_sun) ? (int)Sun_bitmaps.size() : (int)Starfield_bitmaps.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return NULL;

	if (is_a_sun) {
		return Sun_bitmaps[index].filename;
	} else {
		return Starfield_bitmaps[index].filename;
	}
}

// modify an existing starfield bitmap instance, or add a new one if needed
void stars_modify_entry_FRED(int index, const char *name, starfield_list_entry *sbi_new, bool is_a_sun)
{
	if (!Fred_running)
		return;

	starfield_bitmap_instance sbi;
	int idx;
	int add_new = index > ((is_a_sun) ? (int)Sun_bitmaps.size() : (int)Starfield_bitmaps.size());

	Assert( index >= 0 );
	Assert( sbi_new != NULL );

    // copy information
    sbi.ang.p = sbi_new->ang.p;
	sbi.ang.b = sbi_new->ang.b;
	sbi.ang.h = sbi_new->ang.h;
	sbi.scale_x = sbi_new->scale_x;
	sbi.scale_y = sbi_new->scale_y;
	sbi.div_x = sbi_new->div_x;
	sbi.div_y = sbi_new->div_y;

	if (is_a_sun) {
		idx = stars_find_sun((char*)name);
	} else {
		idx = stars_find_bitmap((char*)name);
	}

	// this shouldn't ever happen from FRED since you select the name from a list of those available
	if (idx == -1)
		return;

	sbi.star_bitmap_index = idx;

	if (add_new) {
		if (is_a_sun) {
			Suns.push_back( sbi );
		} else {
			Starfield_bitmap_instances.push_back( sbi );
		}
	} else {
		if (is_a_sun) {
			Suns[index] = sbi;
		} else {
			Starfield_bitmap_instances[index] = sbi;
		}
	}

	if ( !is_a_sun ) {
		starfield_create_bitmap_buffer(index);
	}
}

// erase an instance, note that this is very slow so it should only be done in FRED
void stars_delete_entry_FRED(int index, bool is_a_sun)
{
	if (!Fred_running)
		return;

	int max_index = (is_a_sun) ? (int)Suns.size() : (int)Starfield_bitmap_instances.size();

	Assert( (index >= 0) && (index < max_index) );

	if ( (index < 0) || (index >= max_index) )
		return;

	if (is_a_sun) {
		Suns.erase( Suns.begin() + index );
	} else {
		Starfield_bitmap_instances.erase( Starfield_bitmap_instances.begin() + index );
	}
}

// Goober5000
void stars_load_first_valid_background()
{
	int background_idx = stars_get_first_valid_background();

#ifndef NDEBUG
	if (background_idx < 0 && !Fred_running)
	{
		int i;
		bool at_least_one_bitmap = false;
		for (i = 0; i < Num_backgrounds; i++)
		{
			if (Backgrounds[i].bitmaps.size() > 0)
				at_least_one_bitmap = true;
		}

		if (at_least_one_bitmap)
		{
			if (Num_backgrounds == 1)
				Warning(LOCATION, "Unable to find a sufficient number of bitmaps for this mission's background.  The background will not be displayed.");	
			else if (Num_backgrounds > 1)
				Warning(LOCATION, "Unable to find a sufficient number of bitmaps for any background listed in this mission.  No background will be displayed.");
		}
	}
#endif

	stars_load_background(background_idx);
}

// Goober5000
int stars_get_first_valid_background()
{
	uint i, j;

	if (Num_backgrounds == 0)
		return -1;

	// get the first background with > 50% of its suns and > 50% of its bitmaps present
	for (i = 0; i < (uint)Num_backgrounds; i++)
	{
		uint total_suns = 0;
		uint total_bitmaps = 0;
		background_t *background = &Backgrounds[i];

		for (j = 0; j < background->suns.size(); j++)
		{
			if (stars_find_sun(background->suns[j].filename) >= 0)
				total_suns++;
		}

		for (j = 0; j < background->bitmaps.size(); j++)
		{
			if (stars_find_bitmap(background->bitmaps[j].filename) >= 0)
				total_bitmaps++;
		}

		// add 1 so rounding will work properly
		if ((total_suns >= (background->suns.size() + 1) / 2) && (total_bitmaps >= (background->bitmaps.size() + 1) / 2))
			return i;
	}

	// didn't find a valid entry
	return -1;
}

// Goober5000
void stars_load_background(int background_idx)
{
	uint j;

	stars_clear_instances();
	Cur_background = background_idx;

	if (Cur_background >= 0)
	{
		background_t *background = &Backgrounds[Cur_background];

		int failed_suns = 0;
		for (j = 0; j < background->suns.size(); j++)
		{
			if ((stars_add_sun_entry(&background->suns[j]) < 0) && !Fred_running)
			{
				nprintf(("General", "Failed to add sun '%s' to the mission!", background->suns[j].filename));
				failed_suns++;
			}
		}
		if (failed_suns > 0)
			Warning(LOCATION, "Failed to add %d sun bitmaps to the mission!", failed_suns);

		int failed_stars = 0;
		for (j = 0; j < background->bitmaps.size(); j++)
		{
			if ((stars_add_bitmap_entry(&background->bitmaps[j]) < 0) && !Fred_running)
			{
				nprintf(("General", "Failed to add starfield bitmap '%s' to the mission!", background->bitmaps[j].filename));
				failed_stars++;
			}
		}
		if (failed_stars > 0)
			Warning(LOCATION, "Failed to add %d starfield bitmaps to the mission!", failed_stars);
	}
}

// Goober5000
void stars_copy_background(background_t *dest, background_t *src)
{
	dest->suns.assign(src->suns.begin(), src->suns.end());
	dest->bitmaps.assign(src->bitmaps.begin(), src->bitmaps.end());
}

// Goober5000
void stars_swap_backgrounds(int idx1, int idx2)
{
	background_t temp;
	stars_copy_background(&temp, &Backgrounds[idx1]);
	stars_copy_background(&Backgrounds[idx1], &Backgrounds[idx2]);
	stars_copy_background(&Backgrounds[idx2], &temp);
}

// Goober5000
bool stars_background_empty(int idx)
{
	return !(Backgrounds[idx].suns.size() > 0 || Backgrounds[idx].bitmaps.size() > 0);
}

// Goober5000
void stars_pack_backgrounds()
{
	int i, j;

	// move all empty entries to the end, and recount
	Num_backgrounds = 0;
	for (i = 0; i < MAX_BACKGROUNDS; i++)
	{
		if (!stars_background_empty(i))
		{
			Num_backgrounds++;
			continue;
		}

		for (j = i + 1; j < MAX_BACKGROUNDS; j++)
		{
			if (!stars_background_empty(j))
			{
				stars_swap_backgrounds(i, j);
				Num_backgrounds++;
				break;
			}
		}
	}
}
