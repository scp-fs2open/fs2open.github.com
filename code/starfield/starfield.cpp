/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Starfield/StarField.cpp $
 * $Revision: 2.3 $
 * $Date: 2002-10-19 19:29:29 $
 * $Author: bobboau $
 *
 * Code to handle and draw starfields, background space image bitmaps, floating
 * debris, etc.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2002/09/20 20:05:29  phreak
 * glare parser stuff in stars_init()
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/21 15:48:21  mharris
 * Changed "char *name" to "char name[]" since we modify the string (and
 * modifying a constant string breaks unix)
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 34    9/07/99 4:01p Dave
 * Fixed up a string.tbl paroblem (self destruct message). Make sure IPX
 * does everything properly (setting up address when binding). Remove
 * black rectangle background from UI_INPUTBOX.
 * 
 * 33    9/01/99 10:14a Dave
 * Pirate bob.
 * 
 * 32    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 31    8/19/99 10:59a Dave
 * Packet loss detection.
 * 
 * 30    7/27/99 3:52p Dave
 * Make star drawing a bit more robust to help lame D3D cards.
 * 
 * 29    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 28    7/13/99 2:01p Dave
 * Don't draw background bitmaps in the nebula.
 * 
 * 27    6/08/99 2:34p Jasenw
 * Made perspective bitmaps render in Fred.
 * 
 * 26    6/04/99 1:18p Dave
 * Fixed briefing model rendering problems. Made show background option in
 * fred toggle nebula rendering.
 * 
 * 25    6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 24    5/28/99 1:45p Dave
 * Fixed up perspective bitmap drawing.
 * 
 * 23    5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 22    5/11/99 10:03a Dave
 * Put a bunch of stuff into tables.
 * 
 * 21    5/11/99 9:10a Dave
 * Move default sun position.
 * 
 * 20    5/09/99 6:00p Dave
 * Lots of cool new effects. E3 build tweaks.
 * 
 * 19    4/26/99 8:49p Dave
 * Made all pof based nebula stuff full customizable through fred.
 * 
 * 18    4/25/99 7:43p Dave
 * Misc small bug fixes. Made sun draw properly.
 * 
 * 17    4/23/99 5:53p Dave
 * Started putting in new pof nebula support into Fred.
 * 
 * 16    4/07/99 6:22p Dave
 * Fred and Freespace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 15    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 14    3/20/99 5:09p Dave
 * Fixed release build fred warnings and unhandled exception.
 * 
 * 13    3/20/99 3:46p Dave
 * Added support for model-based background nebulae. Added 3 new
 * sexpressions.
 * 
 * 12    3/20/99 2:04p Dave
 * Removed unnecessary planet rendering.
 * 
 * 11    3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 12    3/15/99 6:45p Daveb
 * Put in rough nebula bitmap support.
 * 
 * 11    3/11/99 5:53p Dave
 * More network optimization. Spliced in Dell OEM planet bitmap crap.
 * 
 * 10    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 9     12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 8     12/01/98 10:32a Johnson
 * Fixed direct3d font problems. Fixed sun bitmap problem. Fixed direct3d
 * starfield problem.
 * 
 * 7     12/01/98 8:06a Dave
 * Temporary checkin to fix some texture transparency problems in d3d.
 * 
 * 6     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 5     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 4     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 3     10/07/98 11:16a Dave
 * Remove warning.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 106   5/23/98 4:14p John
 * Added code to preload textures to video card for AGP.   Added in code
 * to page in some bitmaps that weren't getting paged in at level start.
 * 
 * 105   5/13/98 2:53p John
 * Made subspace effect work under software.  Had to add new inner loop to
 * tmapper.  Added glows to end of subspace effect.  Made subspace effect
 * levels use gamepalette-subspace palette.
 * 
 * 104   5/13/98 10:28a John
 * made subpsace forward sliding 40% faster.
 * 
 * 103   5/10/98 4:18p John
 * Reversed the subspace effect direction
 * 
 * 102   5/08/98 8:38p John
 * Subspace tweaks.  Made two layers rotate independentyl.  
 * 
 * 101   5/08/98 1:32p John
 * Added code for using two layered subspace effects.
 * 
 * 100   5/06/98 5:30p John
 * Removed unused cfilearchiver.  Removed/replaced some unused/little used
 * graphics functions, namely gradient_h and _v and pixel_sp.   Put in new
 * DirectX header files and libs that fixed the Direct3D alpha blending
 * problems.
 * 
 * 99    4/22/98 4:09p John
 * String externalization
 * 
 * 98    4/22/98 3:28p John
 * Fixed XSTR bug
 * 
 * 97    4/13/98 4:54p John
 * Made uv rotate independently on subspace effect. Put in DCF function
 * for setting subspace speeds.
 * 
 * 96    4/12/98 5:55p John
 * Made models work with subspace.  Made subspace rotate also.
 * 
 * 95    4/11/98 6:53p John
 * Added first rev of subspace effect.
 * 
 * 94    4/08/98 11:31a Dave
 * AL: Fix syntax error for non-demo
 * 
 * 93    4/08/98 10:46a Lawrance
 * #ifdef out asteroid check for demo
 * 
 * 92    4/08/98 9:25a John
 * Made asteroid missions not show suns
 * 
 * 91    4/07/98 4:17p John
 * Made Fred be able to move suns.  Made suns actually affect the lighting
 * in the game.
 * 
 * 90    4/07/98 11:19a Hoffoss
 * Changed code to only use Sun01 for a bitmap by default, as John
 * requested.
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "math/floating.h"
#include "math/vecmat.h"
#include "render/3d.h"
#include "graphics/2d.h"
#include "starfield/starfield.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "freespace2/freespace.h"	
#include "io/timer.h"
#include "starfield/nebula.h"
#include "globalincs/linklist.h"
#include "lighting/lighting.h"
#include "asteroid/asteroid.h"
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "globalincs/alphacolors.h"
#include "starfield/supernova.h"

#define MAX_DEBRIS_VCLIPS	4
#define DEBRIS_ROT_MIN				10000
#define DEBRIS_ROT_RANGE			8
#define DEBRIS_ROT_RANGE_SCALER	10000
#define RND_MAX_MASK	0x3fff
#define HALF_RND_MAX 0x2000

typedef struct debris_vclip {
	int	bm;
	int	nframes;
	char  name[MAX_FILENAME_LEN+1];
} debris_vclip;

typedef struct {
	vector pos;
	vector last_pos;
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

int Num_stars = 500;
fix starfield_timestamp = 0;

// for drawing cool stuff on the background - comes from a table
starfield_bitmap Starfield_bitmaps[MAX_STARFIELD_BITMAPS];
starfield_bitmap_instance Starfield_bitmap_instance[MAX_STARFIELD_BITMAPS];
int Num_starfield_bitmaps = 0;

// sun bitmaps and sun glow bitmaps
starfield_bitmap Sun_bitmaps[MAX_STARFIELD_BITMAPS];
starfield_bitmap_instance Suns[MAX_STARFIELD_BITMAPS];
int Num_suns = 0;

int last_stars_filled = 0;
color star_colors[8];
color star_aacolors[8];

typedef struct star {
	vector pos;
	vector last_star_pos;
} star;

star Stars[MAX_STARS];

old_debris odebris[MAX_DEBRIS];

//XSTR:OFF
debris_vclip debris_vclips_normal[MAX_DEBRIS_VCLIPS] = { { -1, -1, "debris01" }, { -1, -1, "debris02" }, { -1, -1, "debris03" }, { -1, -1, "debris04" } };
debris_vclip debris_vclips_nebula[MAX_DEBRIS_VCLIPS] = { { -1, -1, "Neb01-64" }, { -1, -1, "Neb01-64" }, { -1, -1, "Neb01-64" }, { -1, -1, "Neb01-64" } };
debris_vclip *debris_vclips = debris_vclips_normal;
//XSTR:ON

int stars_debris_loaded = 0;

// background data
int Stars_background_inited = 0;			// if we're inited
int Nmodel_num = -1;							// model num
int Nmodel_bitmap = -1;						// model texture

// given a starfield_bitmap_instance, return a pointer to its parent, for suns
starfield_bitmap *stars_lookup_sun(starfield_bitmap_instance *s)
{
	int idx;

	// sanity
	if(s == NULL){
		return NULL;
	}

	// lookup
	for(idx=0; idx<MAX_STARFIELD_BITMAPS; idx++){
		if(!stricmp(Sun_bitmaps[idx].filename, s->filename)){
			return &Sun_bitmaps[idx];
		}
	}

	// no findy
	return NULL;
}

void stars_load_debris()
{
	int i;

	// if we're in nebula mode
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		debris_vclips = debris_vclips_nebula;
	} else {
		debris_vclips = debris_vclips_normal;
	}

	for (i=0; i<MAX_DEBRIS_VCLIPS; i++ )	{
		debris_vclips[i].bm = bm_load_animation( debris_vclips[i].name, &debris_vclips[i].nframes, NULL, 1 );
		if ( debris_vclips[i].bm < 0 ) {
			// try loading it as a single bitmap
			debris_vclips[i].bm = bm_load(debris_vclips[i].name);
			debris_vclips[i].nframes = 1;

			if(debris_vclips[i].bm <= 0){
				Error( LOCATION, "Couldn't load animation/bitmap '%s'\n", debris_vclips[i].name );
			}
		}
	}
	stars_debris_loaded = 1;
}

// call on game startup
void stars_init()
{
	starfield_bitmap *bm;	
	int count, idx;
	char filename[MAX_FILENAME_LEN+1] = "";
	char glow_filename[MAX_FILENAME_LEN+1] = "";
	float r, g, b, i;
	int sun_glare;

	// parse stars.tbl
	read_file_text("stars.tbl");
	reset_parse();

	// make all bitmaps invalid
	for(idx=0; idx<MAX_STARFIELD_BITMAPS; idx++){
		Starfield_bitmaps[idx].bitmap = -1;
		Starfield_bitmaps[idx].glow_bitmap = -1;		
		strcpy(Starfield_bitmaps[idx].filename, "");
		strcpy(Starfield_bitmaps[idx].glow_filename, "");

		Sun_bitmaps[idx].bitmap = -1;		
		Sun_bitmaps[idx].glow_bitmap = -1;		
		strcpy(Sun_bitmaps[idx].filename, "");
		strcpy(Sun_bitmaps[idx].glow_filename, "");
	}

	// starfield bitmaps
	count = 0;
	while(!optional_string("#end")){
		// intensity alpha bitmap
		if(optional_string("$Bitmap:")){
			stuff_string(filename, F_NAME, NULL);
			if(count < MAX_STARFIELD_BITMAPS){
				bm = &Starfield_bitmaps[count++];
				strcpy(bm->filename, filename);
				bm->xparent = 0;
				bm->bitmap = bm_load(bm->filename);				
				Assert(bm->bitmap != -1);

				// if fred is running we should lock the bitmap now
				if(Fred_running && (bm->bitmap >= 0)){
					bm_lock(bm->bitmap, 8, BMP_TEX_OTHER);
					bm_unlock(bm->bitmap);
				} 
			}
		}
		// green xparency bitmap
		else if(optional_string("$BitmapX:")){
			stuff_string(filename, F_NAME, NULL);
			if(count < MAX_STARFIELD_BITMAPS){
				bm = &Starfield_bitmaps[count++];
				strcpy(bm->filename, filename);
				bm->xparent = 1;
				bm->bitmap = bm_load(bm->filename);
				Assert(bm->bitmap != -1);

				// if fred is running we should lock as a 0, 255, 0 bitmap now
				if(Fred_running && (bm->bitmap >= 0)){
					bm_lock(bm->bitmap, 8, BMP_TEX_XPARENT);
					bm_unlock(bm->bitmap);
				} 
			}
		}
	}

	// sun bitmaps
	count = 0;
	while(!optional_string("#end")){
		if(optional_string("$Sun:")){
			stuff_string(filename, F_NAME, NULL);

			// associated glow
			required_string("$Sunglow:");
			stuff_string(glow_filename, F_NAME, NULL);

			// associated lighting values
			required_string("$SunRGBI:");
			stuff_float(&r);
			stuff_float(&g);
			stuff_float(&b);
			stuff_float(&i);

			sun_glare=!optional_string("$NoGlare:");
			if(count < MAX_STARFIELD_BITMAPS){
				bm = &Sun_bitmaps[count++];
				strcpy(bm->filename, filename);
				strcpy(bm->glow_filename, glow_filename);
				bm->xparent = 1;
				bm->bitmap = bm_load(bm->filename);
				bm->glow_bitmap = bm_load(bm->glow_filename);
				Assert(bm->bitmap != -1);
				Assert(bm->glow_bitmap != -1);
				bm->r = r;
				bm->g = g;
				bm->b = b;
				bm->i = i;
				bm->glare=sun_glare;

				// if fred is running we should lock the bitmap now
				if(Fred_running){
					if(bm->bitmap >= 0){
						bm_lock(bm->bitmap, 8, BMP_TEX_OTHER);
						bm_unlock(bm->bitmap);
					}
					if(bm->glow_bitmap >= 0){
						bm_lock(bm->glow_bitmap, 8, BMP_TEX_OTHER);
						bm_unlock(bm->glow_bitmap);
					}
				} 
			}
		}
	}	

	// normal debris pieces
	count = 0;
	while(!optional_string("#end")){
		required_string("$Debris:");
		stuff_string(filename, F_NAME, NULL);

		if(count < MAX_DEBRIS_VCLIPS){
			strcpy(debris_vclips_normal[count++].name, filename);
		}
	}
	Assert(count == 4);

	// nebula debris pieces
	count = 0;
	while(!optional_string("#end")){
		required_string("$DebrisNeb:");
		stuff_string(filename, F_NAME, NULL);

		if(count < MAX_DEBRIS_VCLIPS){
			strcpy(debris_vclips_nebula[count++].name, filename);
		}
	}
	Assert(count == 4);
}

// call this in game_post_level_init() so we know whether we're running in full nebula mode or not
void stars_level_init()
{
	int i;
	vector v;
	float dist, dist_max;

	// reset to -1 so we reload it each mission (if we need to)
	Nmodel_num = -1;		
	if(Nmodel_bitmap != -1){
		bm_unload(Nmodel_bitmap);
		Nmodel_bitmap = -1;
	}

	// if (!stars_debris_loaded){
		stars_load_debris();
	// }

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
	}

	for (i=0; i<MAX_DEBRIS; i++) {
		odebris[i].active = 0;
	}

	for (i=0; i<8; i++ )	{
		ubyte intensity = (ubyte)((i + 1) * 24);
		gr_init_alphacolor(&star_aacolors[i], 255, 255, 255, intensity, AC_TYPE_BLEND );
		gr_init_color(&star_colors[i], intensity, intensity, intensity );
	}

	last_stars_filled = 0;

	// if we have no sun instances, create one
	if(Num_suns <= 0){
		mprintf(("Adding default sun\n"));
		
		// stuff some values
		strcpy(Suns[0].filename, Sun_bitmaps[0].filename);
		Suns[0].scale_x = 1.0f;
		Suns[0].scale_y = 1.0f;
		Suns[0].div_x = 1;
		Suns[0].div_y = 1;
		memset(&Suns[0].ang, 0, sizeof(angles));
		Suns[0].ang.h = fl_radian(60.0f);

		// one sun
		Num_suns = 1;
	}		
}


#include "object/object.h"
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
#define STAR_FLAG_ANTIALIAS	(1<<2)	// Draw the star using antialiased lines
#define STAR_FLAG_DEFAULT		(STAR_FLAG_TAIL | STAR_FLAG_DIM)

uint Star_flags = STAR_FLAG_DEFAULT;

//XSTR:OFF
DCF(stars,"Set parameters for starfield")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "tail" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				Star_amount = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "len" ))	{
			dc_get_arg(ARG_FLOAT);
			Star_max_length = Dc_arg_float;
		} else if ( !strcmp( Dc_arg, "dim" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( Dc_arg_float < 0.0f )	{
				Dc_help = 1;
			} else {
				Star_dim = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "flag" ))	{
			dc_get_arg(ARG_STRING);
			if ( !strcmp( Dc_arg, "tail" ))	{
				Star_flags ^= STAR_FLAG_TAIL;
			} else if ( !strcmp( Dc_arg, "dim" ))	{
				Star_flags ^= STAR_FLAG_DIM;
			} else if ( !strcmp( Dc_arg, "aa" ))	{
				Star_flags ^= STAR_FLAG_ANTIALIAS;
			} else {
				Dc_help = 1;	
			}
		} else if ( !strcmp( Dc_arg, "cap" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 255.0f) )	{
				Dc_help = 1;
			} else {
				Star_cap = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "m0" )  )	{
			Star_amount = 0.0f;
			Star_dim = 0.0f;
			Star_cap = 0.0f;
			Star_flags = 0;
			Star_max_length = STAR_MAX_LENGTH_DEFAULT;
		} else if ( !strcmp( Dc_arg, "m1" ) || !strcmp( Dc_arg, "default" ))	{
			Star_amount = STAR_AMOUNT_DEFAULT;
			Star_dim = STAR_DIM_DEFAULT;
			Star_cap = STAR_CAP_DEFAULT;
			Star_flags = STAR_FLAG_DEFAULT;
			Star_max_length = STAR_MAX_LENGTH_DEFAULT;
		} else if ( !strcmp( Dc_arg, "m2" ))	{
			Star_amount = 0.75f;
			Star_dim = 20.0f;
			Star_cap = 75.0f;
			Star_flags = STAR_FLAG_TAIL|STAR_FLAG_DIM|STAR_FLAG_ANTIALIAS;
			Star_max_length = STAR_MAX_LENGTH_DEFAULT;
		} else if ( !strcmp( Dc_arg, "num" ))	{
			dc_get_arg(ARG_INT);
			if ( (Dc_arg_int < 0) || (Dc_arg_int > MAX_STARS) )	{
				Dc_help = 1;
			} else {
				Num_stars = Dc_arg_int;
			} 
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
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
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Num_stars: %d\n", Num_stars );
		dc_printf( "Tail: %.2f\n", Star_amount );
		dc_printf( "Dim: %.2f\n", Star_dim );
		dc_printf( "Cap: %.2f\n", Star_cap );
		dc_printf( "Max length: %.2f\n", Star_max_length );
		dc_printf( "Flags:\n" );
		dc_printf( "  Tail: %s\n", (Star_flags&STAR_FLAG_TAIL?"On":"Off") );
		dc_printf( "  Dim: %s\n", (Star_flags&STAR_FLAG_DIM?"On":"Off") );
		dc_printf( "  Antialias: %s\n", (Star_flags&STAR_FLAG_ANTIALIAS?"On":"Off") );
		dc_printf( "\nTHESE AREN'T SAVED TO DISK, SO IF YOU TWEAK\n" );
		dc_printf( "THESE AND LIKE THEM, WRITE THEM DOWN!!\n" );
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
extern float Viewer_zoom;

// get the world coords of the sun pos on the unit sphere.
void stars_get_sun_pos(int sun_n, vector *pos)
{
	vector temp;
	matrix rot;

	// sanity
	Assert(sun_n < Num_suns);
	if((sun_n >= Num_suns) || (sun_n < 0)){
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
void stars_draw_sun( int show_sun )
{	
	int idx;
	vector sun_pos;
	vector sun_dir;
	vertex sun_vex;	
	starfield_bitmap *bm;
	float local_scale = 1.0f;

	// no suns drew yet
	Sun_drew = 0;

	// draw all suns
	for(idx=0; idx<Num_suns; idx++){		
		// get the instance
		bm = stars_lookup_sun(&Suns[idx]);
		if(bm == NULL){
			continue;
		}

		// get sun pos
		sun_pos = vmd_zero_vector;
		sun_pos.xyz.y = 1.0f;
		stars_get_sun_pos(idx, &sun_pos);
		
		// get the direction		
		sun_dir = sun_pos;
		vm_vec_normalize(&sun_dir);

		// add the light source corresponding to the sun
		light_add_directional(&sun_dir, bm->i, bm->r, bm->g, bm->b);

		// if supernova
		if(supernova_active()){
			local_scale = 1.0f + (SUPERNOVA_SUN_SCALE * supernova_pct_complete());
		}

		// draw the sun itself, keep track of how many we drew
		gr_set_bitmap(bm->bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.999f);
		g3_rotate_faraway_vertex(&sun_vex, &sun_pos);
		if(!g3_draw_bitmap(&sun_vex, 0, 0.05f * Suns[idx].scale_x * local_scale, TMAP_FLAG_TEXTURED)){
			Sun_drew++;
		}
	}
}

// draw the corresponding glow for sun_n
void stars_draw_sun_glow(int sun_n)
{
	starfield_bitmap *bm;		
	vector sun_pos, sun_dir;
	vertex sun_vex;	
	float local_scale = 1.0f;

	// sanity
	Assert(sun_n < Num_suns);
	if((sun_n >= Num_suns) || (sun_n < 0)){
		return;
	}

	// get the instance
	bm = stars_lookup_sun(&Suns[sun_n]);
	if(bm == NULL){
		return;
	}

	// get sun pos
	sun_pos = vmd_zero_vector;
	sun_pos.xyz.y = 1.0f;
	stars_get_sun_pos(sun_n, &sun_pos);	

	// get the direction		
	sun_dir = sun_pos;
	vm_vec_normalize(&sun_dir);	

	// if supernova
	if(supernova_active()){
		local_scale = 1.0f + (SUPERNOVA_SUN_SCALE * supernova_pct_complete());
	}

	// draw the sun itself, keep track of how many we drew
	gr_set_bitmap(bm->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.5f);
	g3_rotate_faraway_vertex(&sun_vex, &sun_pos);
	g3_draw_bitmap(&sun_vex, 0, 0.10f * Suns[sun_n].scale_x * local_scale, TMAP_FLAG_TEXTURED);	
}

// draw bitmaps
void stars_draw_bitmaps( int show_bitmaps )
{
	int idx;
	int star_index;	

	// if we're in the nebula, don't render any backgrounds
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		return;
	}

	// detail settings
	if(!Detail.planets_suns){
		return;
	}
	
	// render all bitmaps
	for(idx=0; idx<Num_starfield_bitmaps; idx++){
		// lookup the info index
		star_index = stars_find_bitmap(Starfield_bitmap_instance[idx].filename);
		if(star_index < 0){
			continue;
		}
	
		// set the bitmap				
		if(Fred_running){
			gr_set_bitmap(Starfield_bitmaps[star_index].bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);		
			g3_draw_perspective_bitmap(&Starfield_bitmap_instance[idx].ang, Starfield_bitmap_instance[idx].scale_x, Starfield_bitmap_instance[idx].scale_y, Starfield_bitmap_instance[idx].div_x, Starfield_bitmap_instance[idx].div_y, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT);
		} else {
			if(Starfield_bitmaps[star_index].xparent){
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap);		
				g3_draw_perspective_bitmap(&Starfield_bitmap_instance[idx].ang, Starfield_bitmap_instance[idx].scale_x, Starfield_bitmap_instance[idx].scale_y, Starfield_bitmap_instance[idx].div_x, Starfield_bitmap_instance[idx].div_y, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT | TMAP_FLAG_XPARENT);
			} else {				
				gr_set_bitmap(Starfield_bitmaps[star_index].bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.9999f);		
				g3_draw_perspective_bitmap(&Starfield_bitmap_instance[idx].ang, Starfield_bitmap_instance[idx].scale_x, Starfield_bitmap_instance[idx].scale_y, Starfield_bitmap_instance[idx].div_x, Starfield_bitmap_instance[idx].div_y, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT);
			}
		}
	}
}

/*
void calculate_bitmap_matrix(starfield_bitmaps *bm, vector *v)
{
	vm_vector_2_matrix(&bm->m, v, NULL, NULL);
	vm_orthogonalize_matrix(&bm->m);
}

void calculate_bitmap_points(starfield_bitmaps *bm, float bank)
{
	int i;
	vector fvec, uvec, rvec, tmp;
	angles tangles;

	vm_orthogonalize_matrix(&bm->m);
	if (bank) {
		tangles.p = tangles.h = 0.0f;
		tangles.b = bank;
		vm_rotate_matrix_by_angles(&bm->m, &tangles);
	}

	fvec = bm->m.fvec;
	vm_vec_scale(&fvec, bm->dist );
	uvec = bm->m.uvec;
	rvec = bm->m.rvec;

	vm_vec_sub(&tmp, &fvec, &uvec);
	vm_vec_sub(&bm->points[3], &tmp, &rvec);

	vm_vec_sub(&tmp, &fvec, &uvec);
	vm_vec_add(&bm->points[2], &tmp, &rvec);

	vm_vec_add(&tmp, &fvec, &uvec);
	vm_vec_add(&bm->points[1], &tmp, &rvec);

	vm_vec_add(&tmp, &fvec, &uvec);
	vm_vec_sub(&bm->points[0], &tmp, &rvec);

	for (i=0; i<4; i++){
		vm_vec_normalize(&bm->points[i]);
	}
}
*/

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
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "u" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( Dc_arg_float < 0.0f )	{
				Dc_help = 1;
			} else {
				subspace_u_speed = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "v" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( Dc_arg_float < 0.0f )	{
				Dc_help = 1;
			} else {
				subspace_v_speed = Dc_arg_float;
			} 
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: subspace keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "subspace u X    Where X is how fast u moves.\n", MAX_STARS );
		dc_printf( "subspace v X    Where X is how fast v moves.\n" );
		dc_printf( "\nUse '? subspace' to see current values.\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "u: %.2f\n", subspace_u_speed );
		dc_printf( "v: %.2f\n", subspace_v_speed );
	}
}
//XSTR:ON

void subspace_render()
{
	if ( Subspace_model_inner == -1 )	{
		Subspace_model_inner = model_load( "subspace_small.pof", 0, NULL );
		Assert(Subspace_model_inner>-1);
	}

	if ( Subspace_model_outer == -1 )	{
		Subspace_model_outer = model_load( "subspace_big.pof", 0, NULL );
		Assert(Subspace_model_outer>-1);
	}

	if ( Subspace_glow_bitmap == -1 )	{
		Subspace_glow_bitmap = bm_load( NOX("SunGlow01"));
		Assert(Subspace_glow_bitmap>-1);
	}

	Subspace_glow_frame += flFrametime * 1.0f;

	float total_time = i2fl(NOISE_NUM_FRAMES) / 15.0f;

	// Sanity checks
	if ( Subspace_glow_frame < 0.0f )	Subspace_glow_frame = 0.0f;
	if ( Subspace_glow_frame > 100.0f ) Subspace_glow_frame = 0.0f;

	while ( Subspace_glow_frame > total_time )	{
		Subspace_glow_frame -= total_time;
	}
	int framenum = fl2i( (Subspace_glow_frame*NOISE_NUM_FRAMES) / total_time );
	if ( framenum < 0 ) framenum = 0;
	if ( framenum >= NOISE_NUM_FRAMES ) framenum = NOISE_NUM_FRAMES-1;

	subspace_offset_u += flFrametime*subspace_u_speed;
	if (subspace_offset_u > 1.0f )	{
		subspace_offset_u -= 1.0f;
	}

	subspace_offset_u_inner += flFrametime*subspace_u_speed*3.0f;
	if (subspace_offset_u > 1.0f )	{
		subspace_offset_u -= 1.0f;
	}

	subspace_offset_v += flFrametime*subspace_v_speed;
	if (subspace_offset_v > 1.0f )	{
		subspace_offset_v -= 1.0f;
	}

	

	matrix tmp;
	angles angs = { 0.0f, 0.0f, 0.0f };
	angs.b = subspace_offset_v * PI2;
	
	vm_angles_2_matrix(&tmp,&angs);
	
	int saved_gr_zbuffering = 	gr_zbuffer_get();

	gr_zbuffer_set(GR_ZBUFF_NONE);

	if ( !D3D_enabled )	{

		int render_flags = MR_NO_LIGHTING | MR_ALWAYS_REDRAW;

		Interp_subspace = 1;	
		Interp_subspace_offset_u = 1.0f - subspace_offset_u;
		Interp_subspace_offset_v = 0.0f;

		vector temp;
		temp.xyz.x = 1.0f;
		temp.xyz.y = 1.0f;
		temp.xyz.z = 1.0f;
		model_set_thrust( Subspace_model_inner, temp, -1, Subspace_glow_bitmap, Noise[framenum] );
		render_flags |= MR_SHOW_THRUSTERS;
		model_render( Subspace_model_outer, &tmp, &Eye_position, render_flags );	//MR_NO_CORRECT|MR_SHOW_OUTLINE 

	} else {

		int render_flags = MR_NO_LIGHTING | MR_ALWAYS_REDRAW;

		Interp_subspace = 1;	
		Interp_subspace_offset_u = 1.0f - subspace_offset_u;
		Interp_subspace_offset_v = 0.0f;

		vector temp;
		temp.xyz.x = 1.0f;
		temp.xyz.y = 1.0f;
		temp.xyz.z = 1.0f;

		model_set_thrust( Subspace_model_inner, temp, -1, Subspace_glow_bitmap, Noise[framenum] );
		render_flags |= MR_SHOW_THRUSTERS;
		model_render( Subspace_model_outer, &tmp, &Eye_position, render_flags );	//MR_NO_CORRECT|MR_SHOW_OUTLINE 
		
		Interp_subspace = 1;	
		Interp_subspace_offset_u = 1.0f - subspace_offset_u_inner;
		Interp_subspace_offset_v = 0.0f;	

		angs.b = -subspace_offset_v * PI2;

		vm_angles_2_matrix(&tmp,&angs);

		model_set_outline_color(255,255,255);

//		vector temp;
		temp.xyz.x = 1.0f;
		temp.xyz.y = 1.0f;
		temp.xyz.z = 1.0f;

		model_set_thrust( Subspace_model_inner, temp, -1, Subspace_glow_bitmap, Noise[framenum] );
		render_flags |= MR_SHOW_THRUSTERS;

		model_render( Subspace_model_inner, &tmp, &Eye_position, render_flags  );	//MR_NO_CORRECT|MR_SHOW_OUTLINE 
	}

	Interp_subspace = 0;
	gr_zbuffer_set(saved_gr_zbuffering);
}

void stars_draw( int show_stars, int show_suns, int show_nebulas, int show_subspace )
{
	int i;
	float vdist;


	int gr_zbuffering_save = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);

	if ( show_subspace )	{
		subspace_render();
	}

	
	if (Num_stars >= MAX_STARS){
		Num_stars = MAX_STARS;
	}

#ifdef TIME_STAR_CODE
	fix xt1, xt2;
	xt1 = timer_get_fixed_seconds();
#endif
	
	if ( show_nebulas && (Game_detail_flags & DETAIL_FLAG_NEBULAS) && (Neb2_render_mode != NEB2_RENDER_POF) && (Neb2_render_mode != NEB2_RENDER_LAME))	{
		nebula_render();
	}

	// draw background stuff	
	if((Neb2_render_mode != NEB2_RENDER_POLY) && (Neb2_render_mode != NEB2_RENDER_LAME) && show_stars){
		// semi-hack, do we don't fog the background
		int neb_save = Neb2_render_mode;
		Neb2_render_mode = NEB2_RENDER_NONE;
		extern void stars_draw_background();
		stars_draw_background();
		Neb2_render_mode = neb_save;
	}

	if (show_stars && ( Game_detail_flags & DETAIL_FLAG_STARS) && !(The_mission.flags & MISSION_FLAG_FULLNEB) && (supernova_active() < 3))	{
		//Num_stars = 1;
	
		star *sp;

		if ( !last_stars_filled )	{
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
		if (tmp_num_stars < 0 )	{
			tmp_num_stars = 0;
		} else if ( tmp_num_stars > Num_stars )	{
			tmp_num_stars = Num_stars;
		}
		
		for (sp=Stars,i=0; i<tmp_num_stars; i++, sp++ ) {
			vertex p1, p2;			
			int can_draw = 1;			

			memset(&p1, 0, sizeof(vertex));

			// This makes a star look "proper" by not translating the
			// point around the viewer's eye before rotation.  In other
			// words, when the ship translates, the stars do not change.

			g3_rotate_faraway_vertex(&p2, &sp->pos);
			if ( p2.codes )	{
				can_draw = 0;
			} else {
				g3_project_vertex(&p2);
				if ( p2.flags & PF_OVERFLOW )	{
					can_draw = 0;
				}
			}

			float dist = 0.0f;

			if ( can_draw && (Star_flags & (STAR_FLAG_TAIL|STAR_FLAG_DIM)) )	{

				dist = vm_vec_dist_quick( &sp->last_star_pos, (vector *)&p2.x );

				float ratio;
				if ( dist > Star_max_length )	{
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

				if ( p1.codes )	{
					can_draw = 0;
				} else {
					g3_project_vertex(&p1);
					if ( p1.flags & PF_OVERFLOW )	{
						can_draw = 0;
					}
				}
			}

			sp->last_star_pos.xyz.x = p2.x;
			sp->last_star_pos.xyz.y = p2.y;
			sp->last_star_pos.xyz.z = p2.z;

			if ( !can_draw )	continue;

			int color;

			if ( Star_flags & STAR_FLAG_DIM )	{

				float colorf = 255.0f - dist*Star_dim;

				if ( colorf < Star_cap )
					colorf = Star_cap;

				color = (fl2i(colorf)*(i&7))/256;

			} else {
				color = i & 7;
			}

			if ( (Star_flags & STAR_FLAG_ANTIALIAS) || (D3D_enabled) )	{
				gr_set_color_fast( &star_aacolors[color] );

				// if the two points are the same, fudge it, since some D3D cards (G200 and G400) are lame.				
				if( (fl2i(p1.sx) == fl2i(p2.sx)) && (fl2i(p1.sy) == fl2i(p2.sy)) ){					
					p1.sx += 1.0f;
				}								
				gr_aaline(&p1,&p2);
			} else {
				// use alphablended line so that dark stars don't look bad on top of nebulas
				gr_set_color_fast( &star_aacolors[color] );
				if ( Star_flags & STAR_FLAG_TAIL )	{
					gr_line(fl2i(p1.sx),fl2i(p1.sy),fl2i(p2.sx),fl2i(p2.sy));
				} else {
					gr_pixel( fl2i(p2.sx),fl2i(p2.sy) );
				}
			}
		}
	}

	last_stars_filled = 1;

#ifdef TIME_STAR_CODE
	xt2 = timer_get_fixed_seconds();
	mprintf(( "Stars: %d\n", xt2-xt1 ));
#endif
	

	if ( (Game_detail_flags & DETAIL_FLAG_MOTION) && (!Fred_running) && (supernova_active() < 3) )	{

		gr_set_color( 0, 0, 0 );

		// turn off fogging
		if(The_mission.flags & MISSION_FLAG_FULLNEB){
			gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
		}

		old_debris * d = odebris; 
		for (i=0; i<MAX_DEBRIS; i++, d++ ) {
			vertex p;

			if (!d->active)	{
				d->pos.xyz.x = f2fl(myrand() - RAND_MAX/2);
				d->pos.xyz.y = f2fl(myrand() - RAND_MAX/2);
				d->pos.xyz.z = f2fl(myrand() - RAND_MAX/2);

				vm_vec_normalize(&d->pos);

				vm_vec_scale(&d->pos, MAX_DIST);
				vm_vec_add2(&d->pos, &Eye_position );
//				vm_vec_add2(&d->pos, &Player_obj->pos );
				d->active = 1;
				d->vclip = i % MAX_DEBRIS_VCLIPS;	//rand()

				// if we're in full neb mode
				if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)){
					d->size = i2fl(myrand() % 4)*BASE_SIZE_NEB;
				} else {
					d->size = i2fl(myrand() % 4)*BASE_SIZE;
				}

				vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );
			}

			if ( reload_old_debris )	{
				vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );
			}
			
			g3_rotate_vertex(&p, &d->pos);

			if (p.codes == 0) {
				int frame = Missiontime / (DEBRIS_ROT_MIN + (i % DEBRIS_ROT_RANGE) * DEBRIS_ROT_RANGE_SCALER);
				frame %= debris_vclips[d->vclip].nframes;

				if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)){
					gr_set_bitmap( debris_vclips[d->vclip].bm + frame, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.3f);	
				} else {
					gr_set_bitmap( debris_vclips[d->vclip].bm + frame );						
				}
					
				vector tmp;
				vm_vec_add( &tmp, &d->last_pos, &Eye_position );
				g3_draw_laser( &d->pos,d->size,&tmp,d->size, TMAP_FLAG_TEXTURED|TMAP_FLAG_XPARENT, 25.0f );					
			}

			vm_vec_sub( &d->last_pos, &d->pos, &Eye_position );

			vdist = vm_vec_mag_quick(&d->last_pos);

			if (vdist > MAX_DIST_RANGE)
				d->active = 0;
			else if (vdist < MIN_DIST_RANGE)
				d->active = 0;

	//		vector tmp;
	//		vm_vec_sub( &tmp, &d->pos, &Player_obj->pos );
	//		vdist = vm_vec_dot( &tmp, &Player_obj->orient.fvec );
	//		if ( vdist < 0.0f )
	//			d->active = 0;

		}

		reload_old_debris = 0;
	}


	stars_draw_sun( show_suns );	
	stars_draw_bitmaps( show_suns );

	gr_zbuffer_set( gr_zbuffering_save );
}

void stars_page_in()
{
	int i, j;

	// Initialize the subspace stuff

	if ( Game_subspace_effect )	{

		Subspace_model_inner = model_load( "subspace_small.pof", 0, NULL );
		Assert(Subspace_model_inner>-1);
		Subspace_model_outer = model_load( "subspace_big.pof", 0, NULL );
		Assert(Subspace_model_outer>-1);

		polymodel *pm;
		
		pm = model_get(Subspace_model_inner);
		
		nprintf(( "Paging", "Paging in textures for subspace effect.\n" ));

		for (j=0; j<pm->n_textures; j++ )	{
			int bitmap_num = pm->original_textures[j];

			if ( bitmap_num > -1 )	{
				bm_page_in_texture( bitmap_num );
			}
		}

		pm = model_get(Subspace_model_outer);
		
		nprintf(( "Paging", "Paging in textures for subspace effect.\n" ));

		for (j=0; j<pm->n_textures; j++ )	{
			int bitmap_num = pm->original_textures[j];

			if ( bitmap_num > -1 )	{
				bm_page_in_texture( bitmap_num );
			}
		}
	} else {
		Subspace_model_inner = -1;
		Subspace_model_outer = -1;
	}

	Subspace_glow_bitmap = bm_load( NOX("SunGlow01"));
	bm_page_in_xparent_texture(Subspace_glow_bitmap);

	// page in starfield bitmaps
	int idx;
	idx = 0;
	while((idx < MAX_STARFIELD_BITMAPS) && (Starfield_bitmaps[idx].bitmap != -1)){	
		if(Starfield_bitmaps[idx].xparent){
			bm_page_in_xparent_texture(Starfield_bitmaps[idx].bitmap);
		} else { 
			bm_page_in_texture(Starfield_bitmaps[idx].bitmap);
		}

		// next;
		idx++;
	}

	// sun bitmaps and glows
	idx = 0;
	while((idx < MAX_STARFIELD_BITMAPS) && (Sun_bitmaps[idx].bitmap != -1) && (Sun_bitmaps[idx].glow_bitmap != -1)){
		bm_page_in_texture(Sun_bitmaps[idx].bitmap);
		bm_page_in_texture(Sun_bitmaps[idx].glow_bitmap);

		// next 
		idx++;
	}

	for (i=0; i<MAX_DEBRIS_VCLIPS; i++ )	{
		for (j=0; j<debris_vclips[i].nframes; j++ )	{
			bm_page_in_xparent_texture(debris_vclips[i].bm + j);
		}
	}	
}

// background nebula models and planets
void stars_draw_background()
{				
	if((Nmodel_num < 0) || (Nmodel_bitmap < 0)){
		return;
	}

	// draw the model at the player's eye wif no z-buffering
	model_set_alpha(1.0f);
	model_set_forced_texture(Nmodel_bitmap);	
	model_render(Nmodel_num, &vmd_identity_matrix, &Eye_position, MR_NO_ZBUFFER | MR_NO_CULL | MR_ALL_XPARENT | MR_NO_LIGHTING | MR_FORCE_TEXTURE);	
	model_set_forced_texture(-1);
}

// call this to set a specific model as the background model
void stars_set_background_model(char *model_name, char *texture_name)
{
	Nmodel_num = model_load(model_name, 0, NULL);
	Nmodel_bitmap = bm_load(texture_name);
}

// lookup a starfield bitmap, return index or -1 on fail
int stars_find_bitmap(char *name)
{
	int idx;

	// lookup
	for(idx=0; idx<MAX_STARFIELD_BITMAPS; idx++){
		if(!strcmp(name, Starfield_bitmaps[idx].filename)){
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

	// lookup
	for(idx=0; idx<MAX_STARFIELD_BITMAPS; idx++){
		if(!strcmp(name, Sun_bitmaps[idx].filename)){
			return idx;
		}
	}

	// not found 
	return -1;
}
