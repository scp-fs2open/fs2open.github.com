/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Model/ModelInterp.cpp $
 * $Revision: 2.26 $
 * $Date: 2003-08-06 17:24:57 $
 * $Author: phreak $
 *
 *	Rendering models, I think.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.25  2003/07/15 16:09:46  phreak
 * lighting in the render functions now work for opengl as they do in d3d
 *
 * Revision 2.24  2003/07/15 02:37:59  phreak
 * models that are fully cloaked draw front-to-back
 *
 * Revision 2.23  2003/07/04 02:28:37  phreak
 * changes for cloaking implemented
 *
 * Revision 2.22  2003/06/08 17:31:37  phreak
 * debris is now properly fogged
 *
 * Revision 2.21  2003/06/04 15:28:24  phreak
 * added some code that took advantage of opengl fogging stuff
 *
 * Revision 2.20  2003/03/18 10:07:04  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.19  2003/03/18 01:44:31  Goober5000
 * fixed some misspellings
 * --Goober5000
 *
 * Revision 2.18  2003/03/02 05:55:51  penguin
 * Moved Gr_scaler_zbuffering definition to top of file
 *  - penguin
 *
 * Revision 2.17  2003/01/30 23:19:14  phreak
 * enabled RGB lighting for OpenGL
 *
 * Revision 2.16  2003/01/21 17:24:16  Goober5000
 * fixed a few bugs in Bobboau's implementation of the glow sexps; also added
 * help for the sexps in sexp_tree
 * --Goober5000
 *
 * Revision 2.15  2003/01/20 05:40:49  bobboau
 * added several sExps for turning glow points and glow maps on and off
 *
 * Revision 2.14  2003/01/19 01:07:41  bobboau
 * redid the way glowmaps are handeled, you now must set the global int GLOWMAP (no longer an array) before you render a poly that uses a glow map then set  GLOWMAP to -1 when you're done with, fixed a few other misc bugs it
 *
 * Revision 2.13  2003/01/17 01:48:49  Goober5000
 * added capability to the $Texture replace code to substitute the textures
 * without needing and extra model, however, this way you can't substitute
 * transparent or animated textures
 * --Goober5000
 *
 * Revision 2.12  2003/01/16 06:49:11  Goober5000
 * yay! got texture replacement to work!!!
 * --Goober5000
 *
 * Revision 2.11  2003/01/09 05:55:55  Goober5000
 * fixed the giant thruster glow bug
 * --Goober5000
 *
 * Revision 2.10  2003/01/06 19:33:22  Goober5000
 * cleaned up some stuff with model_set_thrust and a commented Assert that
 * shouldn't have been
 * --Goober5000
 *
 * Revision 2.9  2002/12/07 01:37:42  bobboau
 * inital decals code, if you are worried a bug is being caused by the decals code it's only references are in,
 * collideshipweapon.cpp line 262, beam.cpp line 2771, and modelinterp.cpp line 2949.
 * it needs a better renderer, but is in prety good shape for now,
 * I also (think) I squashed a bug in the warpmodel code
 *
 * Revision 2.8  2002/12/02 23:16:45  Goober5000
 * commented out an unneeded variable (ship *shipp) in model_really_render
 *
 * Revision 2.7  2002/11/14 06:15:03  bobboau
 * added nameplate code
 *
 * Revision 2.6  2002/11/14 04:18:16  bobboau
 * added warp model and type 1 glow points
 * and well as made the new glow file type,
 * some general improvement to fighter beams,
 *
 * Revision 2.5  2002/10/22 23:02:40  randomtiger
 * Made Phreaks alternative scanning style optional under the command line tag "-phreak"
 * Fixed bug that changes HUD colour when targetting debris in a full nebula. - RT
 *
 * Revision 2.4  2002/10/19 19:29:27  bobboau
 * inital commit, trying to get most of my stuff into FSO, there should be most of my fighter beam, beam rendering, beam sheild hit, ABtrails, and ssm stuff. one thing you should be happy to know is the beam texture tileing is now set in the beam section section of the weapon table entry
 *
 * Revision 2.3.2.1  2002/10/30 22:57:21  randomtiger
 *
 * Changed DX8 code to not use set render and texture states if they are already set to that value.
 * Disabled buffer saving and restoring code when windowed to stop DX8 debug runs from crashing. - RT
 *
 * Revision 2.3  2002/08/01 01:41:07  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/10 18:42:14  wmcoolmon
 * Added  Bobboau's glow code; all comments include "-Bobboau"
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:25  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/09 13:49:30  mharris
 * Added ifndef NO_DIRECT3D
 *
 * Revision 1.2  2002/05/04 04:52:22  mharris
 * 1st draft at porting
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 37    9/13/99 11:25p Dave
 * Fixed problem with mode-switching and D3D movies.
 * 
 * 36    9/13/99 10:09a Andsager
 * Add debug console commands to lower model render detail and fireball
 * LOD for big ship explosiosns.
 * 
 * 35    9/08/99 12:03a Dave
 * Make squad logos render properly in D3D all the time. Added intel anim
 * directory.
 * 
 * 34    9/01/99 10:09a Dave
 * Pirate bob.
 * 
 * 33    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 32    8/24/99 8:55p Dave
 * Make sure nondimming pixels work properly in tech menu.
 * 
 * 31    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 30    7/29/99 12:05a Dave
 * Nebula speed optimizations.
 * 
 * 29    7/27/99 3:09p Dave
 * Made g400 work. Whee.
 * 
 * 28    7/24/99 4:19p Dave
 * Fixed dumb code with briefing bitmaps. Made d3d zbuffer work much
 * better. Made model code use zbuffer more intelligently.
 * 
 * 27    7/19/99 7:20p Dave
 * Beam tooling. Specialized player-killed-self messages. Fixed d3d nebula
 * pre-rendering.
 * 
 * 26    7/18/99 5:20p Dave
 * Jump node icon. Fixed debris fogging. Framerate warning stuff.
 * 
 * 25    7/15/99 2:13p Dave
 * Added 32 bit detection.
 * 
 * 24    6/22/99 7:03p Dave
 * New detail options screen.
 * 
 * 23    6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 22    5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 21    5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * 20    5/12/99 10:05a Johnson
 * DKA:  Fix fred bug from engine wash
 * 
 * 19    5/08/99 8:25p Dave
 * Upped object pairs. First run of nebula lightning.
 * 
 * 18    4/26/99 8:49p Dave
 * Made all pof based nebula stuff full customizable through fred.
 * 
 * 17    4/23/99 5:53p Dave
 * Started putting in new pof nebula support into Fred.
 * 
 * 16    4/20/99 3:30p Andsager
 * Let get_model_closest_box_point_with_delta() take NULL as pointer to
 * is_inside
 * 
 * 15    4/19/99 12:51p Andsager
 * Add function to find the nearest point on extneded bounding box and
 * check if inside bounding box.
 * 
 * 14    3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 13    3/24/99 6:14p Dave
 * Added position and orientation checksumming for multiplayer. Fixed LOD
 * rendering bugs for squad insignias
 * 
 * 12    3/23/99 5:17p Dave
 * Changed model file format somewhat to account for LOD's on insignias
 * 
 * 11    3/19/99 9:51a Dave
 * Checkin to repair massive source safe crash. Also added support for
 * pof-style nebulae, and some new weapons code.
 * 
 * 10    3/08/99 7:03p Dave
 * First run of new object update system. Looks very promising.
 * 
 * 9     3/02/99 9:25p Dave
 * Added a bunch of model rendering debug code. Started work on fixing
 * beam weapon wacky firing.
 * 
 * 8     2/19/99 11:42a Dave
 * Put in model rendering autocentering.
 * 
 * 7     1/14/99 6:06p Dave
 * 100% full squad logo support for single player and multiplayer.
 * 
 * 6     1/08/99 2:08p Dave
 * Fixed software rendering for pofview. Super early support for AWACS and
 * beam weapons.
 * 
 * 5     1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 4     12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 3     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 193   8/28/98 3:29p Dave
 * EMP effect done. AI effects may need some tweaking as required.
 * 
 * 192   6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 191   5/24/98 4:53p John
 * changed rounding for model caching to get rid of some empty lines in
 * cached bitmap.
 * 
 * 190   5/16/98 4:47p John
 * Put back in my version 188 changes that someone so rudely (but
 * hopefully unintentionally) deleted.
 * 
 * 189   5/13/98 11:34p Mike
 * Model caching system.
 * 
 * 187   5/12/98 11:32a Mike
 * Support for weapon pof detail levels.
 * 
 * 186   5/08/98 1:32p John
 * Added code for using two layered subspace effects.
 * 
 * 185   4/29/98 11:03a John
 * Added code to show octants.
 * 
 * 184   4/22/98 9:58p John
 * Added code to view invisible faces.
 * 
 * 183   4/22/98 9:43p John
 * Added code to allow checking of invisible faces, flagged by any texture
 * name with invisible in it.
 * 
 * 182   4/20/98 4:44p John
 * Fixed problems with black being xparent on model cache rneders.  Made
 * model cache key off of detail level setting and framerate.
 * 
 * 181   4/18/98 4:00p John
 * Made highest model caching detail level disable model caching.
 * 
 * 180   4/14/98 11:11p John
 * Made ships with < 50% hull left show electrical damage arcs.
 * 
 * 179   4/13/98 4:54p John
 * Made uv rotate independently on subspace effect. Put in DCF function
 * for setting subspace speeds.
 * 
 * 178   4/12/98 5:54p John
 * Made models work with subspace.  Made subspace rotate also.
 * 
 * 177   4/12/98 9:56a John
 * Made lighting detail flags work.   Made explosions cast light on
 * highest.
 * 
 * 176   4/11/98 6:53p John
 * Added first rev of subspace effect.
 * 
 * 175   4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 174   4/09/98 11:40p John
 * Fixed some bugs with hardware lighting.
 * 
 * 173   4/09/98 4:38p John
 * Made non-darkening and transparent textures work under Glide.  Fixed
 * bug with Jim's computer not drawing any bitmaps.
 * 
 * $NoKeywords: $
 */

#include <math.h>

#define MODEL_LIB

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "model/model.h"
#include "graphics/tmapper.h"
#include "math/floating.h"
#include "math/fvi.h"
#include "lighting/lighting.h"
#include "model/modelsinc.h"
#include "fireball/fireballs.h"
#include "math/fix.h"
#include "bmpman/bmpman.h"
#include "globalincs/systemvars.h"
#include "io/key.h"
#include "render/3dinternal.h"
#include "io/timer.h"
#include "graphics/grinternal.h"
#include "palman/palman.h"
#include "object/object.h"			// For MAX_OBJECTS
#include "mission/missionparse.h"
#include "nebula/neb.h"
#include "math/staticrand.h"
#include "globalincs/alphacolors.h"
//#include "weapon/beam.h" //I don't need this after all:\ -Bobboau
#include "debugconsole/dbugfile.h"

// Some debug variables used externally for displaying stats
#ifndef NDEBUG
int modelstats_num_polys = 0;
int modelstats_num_polys_drawn = 0;
int modelstats_num_verts = 0;
int modelstats_num_sortnorms = 0;
int modelstats_num_boxes = 0;
#endif

int glow_maps_active = 1;

extern int OGL_inited;
extern int Gr_scaler_zbuffering;

// a lighting object
typedef struct model_light_object {
	ubyte		r[MAX_POLYGON_NORMS];
	ubyte		g[MAX_POLYGON_NORMS];
	ubyte		b[MAX_POLYGON_NORMS];
	int		objnum;
	int		skip;
	int		skip_max;
} model_light_object;

// -----------------------
// Local variables
//

// Vertices used internally to rotate model points
static vertex Interp_points[MAX_POLYGON_VECS];
vector *Interp_verts[MAX_POLYGON_VECS];
static int Interp_num_verts;


// -------------------------------------------------------------------
// lighting save stuff 
//
#define MAX_MODEL_LIGHTING_SAVE			30
int hack_skip_max = 1;
DCF(skip, "")
{
	dc_get_arg(ARG_INT);
	hack_skip_max = Dc_arg_int;
}
// model_light_object Interp_lighting_save[MAX_MODEL_LIGHTING_SAVE];
model_light_object Interp_lighting_temp;
model_light_object *Interp_lighting = &Interp_lighting_temp;
int Interp_use_saved_lighting = 0;
int Interp_saved_lighting_full = 0;
//
// lighting save stuff 
// -------------------------------------------------------------------


static ubyte Interp_light_applied[MAX_POLYGON_NORMS];
static vector *Interp_norms[MAX_POLYGON_NORMS];
static int Interp_num_norms = 0;
static ubyte *Interp_lights;

static float Interp_fog_level = 0.0f;

// Stuff to control rendering parameters
static color Interp_outline_color;
static int Interp_detail_level = 0;
static uint Interp_flags = 0;
static uint Interp_tmap_flags = 0;

// If non-zero, then the subobject gets scaled by Interp_thrust_scale.
static int Interp_thrust_scale_subobj=0;
static float Interp_thrust_scale = 0.1f;
static int Interp_thrust_bitmap = -1;
static int Interp_thrust_glow_bitmap = -1;
static float Interp_thrust_glow_noise = 1.0f;
static float Interp_thrust_scale_x = 0.0f;//added -bobboau
static float Interp_thrust_scale_y = 0.0f;//added -bobboau
float Model_Interp_scale_x = 1.0f;	//added these three for warpin stuff-Bobbau
float Model_Interp_scale_y = 1.0f;
float Model_Interp_scale_z = 1.0f;
int Warp_Model = -1; //global warp model number
int Warp_Map = -1;	//global map to be used while rendering the warp model
float Warp_Alpha = -1.0f;
static int Interp_objnum = -1;

// if != -1, use this bitmap when rendering ship insignias
static int Interp_insignia_bitmap = -1;

// replacement - Goober5000
static int Interp_replacement_bitmap = -1;
static int *Interp_replacement_textures = NULL;

// if != -1, use this bitmap when rendering with a forced texture
static int Interp_forced_bitmap = -1;

// for rendering with the MR_ALL_XPARENT FLAG SET
static float Interp_xparent_alpha = 1.0f;

float Interp_light = 0.0f;

int Interp_multitex_cloakmap=-1;
int Interp_cloakmap_alpha=255;

void set_warp_globals(float a, float b, float c, int d, float e){
	Model_Interp_scale_x =a;
	Model_Interp_scale_y=b;
	Model_Interp_scale_z=c;
	Warp_Map=d;
	Warp_Alpha=e;
//	mprintf(("warpmap being set to %d\n",Warp_Map));
}

static int FULLCLOAK=-1;

void model_interp_sortnorm_b2f(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check);
void model_interp_sortnorm_f2b(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check);


void (*model_interp_sortnorm)(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check) = model_interp_sortnorm_b2f;

void model_setup_cloak(vector *shift, int full_cloak, int alpha)
{
	FULLCLOAK=full_cloak;
	int unit;
	if (full_cloak)
	{
		unit=0;
		Interp_multitex_cloakmap=0;
		model_set_insignia_bitmap(-1);
		model_set_forced_texture(CLOAKMAP);
		gr_set_cull(1);
		model_interp_sortnorm=model_interp_sortnorm_f2b;
	}
	else
	{
		unit=2;
		Interp_multitex_cloakmap=1;
	}

	Interp_cloakmap_alpha=alpha;
	gr_push_texture_matrix(unit);
	gr_translate_texture_matrix(unit,shift);
}

void model_finish_cloak(int full_cloak)
{
	int unit;
	if (full_cloak){		unit=0; gr_set_cull(0);}
	else				unit=2;

	gr_pop_texture_matrix(unit);
	Interp_multitex_cloakmap=-1;
	model_set_forced_texture(-1);
	FULLCLOAK=-1;
}


// call at the beginning of a level. after the level has been loaded
void model_level_post_init()
{
	/*
	int idx;

	// reset lighting stuff	
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		Interp_lighting_save[idx].objnum = -1;
		Interp_lighting_save[idx].skip = 0;
	}

	// saved lighting is not full
	Interp_saved_lighting_full = 0;
	*/
}

// call to select an object for using "saved" lighting
void model_set_saved_lighting(int objnum, int skip_max)
{
	/*
	int idx;

	// always set to not using saved light to start with
	Interp_use_saved_lighting = 0;
	Interp_lighting = &Interp_lighting_temp;

	// if he passed a -1 for either value, no saved lighting
	if((objnum == -1) || (skip_max == -1)){
		return;
	}

	// see if the object is already on the list
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// ahha, he is on the list
		if(Interp_lighting_save[idx].objnum == objnum){
			// if he's entered a new skip max
			if(Interp_lighting_save[idx].skip_max != skip_max){
				Interp_lighting_save[idx].skip = 0;
				Interp_lighting_save[idx].skip_max = skip_max;
				Interp_use_saved_lighting = 0;
				Interp_lighting = &Interp_lighting_save[idx];
			} 
			// if we're reached the "skip" frame, re-render lighting for this guy
			else if(Interp_lighting_save[idx].skip == Interp_lighting_save[idx].skip_max){
				Interp_lighting_save[idx].skip = 0;
				Interp_use_saved_lighting = 0;
				Interp_lighting = &Interp_lighting_save[idx];
			}
			// otherwise, use his saved lighting values
			else {
				Interp_lighting_save[idx].skip++;
				Interp_use_saved_lighting = 1;
				Interp_lighting = &Interp_lighting_save[idx];
			}

			// duh
			return;
		}
	}

	// no free saved lighting slots
	if(Interp_saved_lighting_full){
		return;
	}
	
	// find a free slot
	int found = 0;
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// got one
		if(Interp_lighting_save[idx].objnum == -1){
			Interp_lighting_save[idx].objnum = objnum;
			Interp_lighting_save[idx].skip = 0;
			Interp_lighting_save[idx].skip_max = skip_max;

			Interp_use_saved_lighting = 0;
			Interp_lighting = &Interp_lighting_save[idx];

			found = 1;
			break;
		}
	}

	// oops. out of free slots
	if(!found){
		Interp_saved_lighting_full = 1;
	}
	*/
}

// notify the model system that a ship has died
void model_notify_dead_ship(int objnum)
{
	/*
	int idx;

	// see if this guy was on the lighting list
	for(idx=0; idx<MAX_MODEL_LIGHTING_SAVE; idx++){
		// free him up
		if(objnum == Interp_lighting_save[idx].objnum){
			Interp_lighting_save[idx].objnum = -1;
			Interp_saved_lighting_full = 0;
			return;
		}
	}
	*/
}

void interp_clear_instance()
{
	Interp_thrust_scale = 0.1f;
	Interp_thrust_scale_x = 0.0f;//added-Bobboau
	Interp_thrust_scale_y = 0.0f;//added-Bobboau
	Interp_thrust_bitmap = -1;
	Interp_thrust_glow_bitmap = -1;
	Interp_thrust_glow_noise = 1.0f;
	Interp_insignia_bitmap = -1;
}

// Scales the engines thrusters by this much
void model_set_thrust( int model_num, vector *length /*<-I did that-Bobboau*/, int bitmap, int glow_bitmap, float glow_noise )
{
	Interp_thrust_scale = length->xyz.z;
	Interp_thrust_scale_x = length->xyz.x;
	Interp_thrust_scale_y = length->xyz.y;
	Interp_thrust_bitmap = bitmap;
	Interp_thrust_glow_bitmap = glow_bitmap;
	Interp_thrust_glow_noise = glow_noise;

	if ( Interp_thrust_scale < 0.1f ) {
		Interp_thrust_scale = 0.1f;
	} else if ( Interp_thrust_scale > 1.0f ) {
		Interp_thrust_scale = 1.0f;
	}

	polymodel * pm = model_get( model_num );
	int i;

	// If thrust is set up, use it.
	for (i=0; i<pm->num_lights; i++ )	{
		if ( pm->lights[i].type == BSP_LIGHT_TYPE_THRUSTER )	{
			float scale = (Interp_thrust_scale-0.1f)*0.5f;

			pm->lights[i].value += (scale+Interp_thrust_glow_noise*0.2f) / 255.0f;
		}
	}
}


// Point list
// +0      int         id
// +4      int         size
// +8      int         n_verts
// +12     int         n_norms
// +16     int         offset from start of chunk to vertex data
// +20     n_verts*char    norm_counts
// +offset             vertex data. Each vertex n is a point followed by norm_counts[n] normals.
void model_interp_defpoints(ubyte * p, polymodel *pm, bsp_info *sm)
{
	int i, n;
	int nverts = w(p+8);	
	int offset = w(p+16);
	int next_norm = 0;

	ubyte * normcount = p+20;
	vertex *dest = Interp_points;
	vector *src = vp(p+offset);

	// Get pointer to lights
	Interp_lights = p+20+nverts;

	Assert( nverts < MAX_POLYGON_VECS );
	// Assert( nnorms < MAX_POLYGON_NORMS );

	Interp_num_verts = nverts;
	#ifndef NDEBUG
	modelstats_num_verts += nverts;
	#endif

/*
	static int Max_vecs = 0;
	static int Max_norms = 0;

	if ( Max_vecs < nverts )	{
		Max_vecs = nverts;
		mprintf(( "MAX NORMS = %d\n", Max_norms ));
		mprintf(( "MAX VECS = %d\n", Max_vecs ));
	}

	if ( Max_norms < nnorms )	{
		Max_norms = nnorms;
		mprintf(( "MAX NORMS = %d\n", Max_norms ));
		mprintf(( "MAX VECS = %d\n", Max_vecs ));
	}
*/

	if (Interp_thrust_scale_subobj)	{

		// Only scale vertices that aren't on the "base" of 
		// the effect.  Base is something Adam decided to be
		// anything under 1.5 meters, hence the 1.5f.
		float min_thruster_dist = -1.5f;

		if ( Interp_flags & MR_IS_MISSILE )	{
			min_thruster_dist = 0.5f;
		}

		for (n=0; n<nverts; n++ )	{
			vector tmp;

			Interp_verts[n] = src;

			// Only scale vertices that aren't on the "base" of 
			// the effect.  Base is something Adam decided to be
			// anything under 1.5 meters, hence the 1.5f.
			if ( src->xyz.z < min_thruster_dist )	{
				tmp.xyz.x = src->xyz.x * 1.0f;
				tmp.xyz.y = src->xyz.y * 1.0f;
				tmp.xyz.z = src->xyz.z * Interp_thrust_scale;
			} else {
				tmp = *src;
			}
			
			g3_rotate_vertex(dest,&tmp);
		
			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		} 

	} else if((Model_Interp_scale_x != 1) || (Model_Interp_scale_y != 1) || (Model_Interp_scale_z != 1)) {

		mprintf(("warp model being scaled by %f %f %f\n",Model_Interp_scale_x ,Model_Interp_scale_y, Model_Interp_scale_z));
		for (n=0; n<nverts; n++ )	{
			vector tmp;

			Interp_verts[n] = src;

				tmp.xyz.x = (src->xyz.x) * Model_Interp_scale_x;
				tmp.xyz.y = (src->xyz.y) * Model_Interp_scale_y;
				tmp.xyz.z = (src->xyz.z) * Model_Interp_scale_z;
			
			g3_rotate_vertex(dest,&tmp);
		
			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		} 
	}else{

		for (n=0; n<nverts; n++ )	{	
			Interp_verts[n] = src;	
			
			/*
			vector tmp = *src;
			// TEST
			if(Interp_thrust_twist > 0.0f){
				float theta;
				float st, ct;

				// determine theta for this vertex				
				theta = fl_radian(20.0f + Interp_thrust_twist2);				
				st = sin(theta);
				ct = cos(theta);

				// twist
				tmp.xyz.z = (src->xyz.z * ct) - (src->xyz.y * st);
				tmp.xyz.y = (src->xyz.z * st) + (src->xyz.y * ct);

				// scale the z a bit
				tmp.xyz.z += Interp_thrust_twist;
			}			
	
			g3_rotate_vertex(dest, &tmp);
			*/
			
			g3_rotate_vertex(dest, src);
		
			src++;		// move to normal

			for (i=0; i<normcount[n]; i++ )	{
				Interp_light_applied[next_norm] = 0;
				Interp_norms[next_norm] = src;

				next_norm++;
				src++;
			}
			dest++;
		}
	}

	Interp_num_norms = next_norm;

}

matrix *Interp_orient;
vector *Interp_pos;
vector Interp_offset;

/*
void interp_compute_environment_mapping( vector *nrm, vertex * pnt, vector *vert)
{
	vector R;
	float a;
	matrix * m = &View_matrix;

	vm_vec_rotate( &R, nrm, &View_matrix );	
	vm_vec_normalize(&R);

	a = 2.0f * R.xyz.z;
	R.xyz.x = a * R.xyz.x;	// reflected R = 2N(N.E) -E;  E = eye
	R.xyz.y = a * R.xyz.y;
	R.xyz.z = a * R.xyz.z;
	vm_vec_normalize(&R);
	a = (float)fl_sqrt( 1.0f - R.xyz.y * R.xyz.y);
	pnt->u = (float)atan2( R.xyz.x, -R.xyz.z) / (2.0f * 3.14159f);
	if (pnt->u < 0.0) pnt->u += 1.0f;
	pnt->v = 1.0f - (float)atan2( a, R.xyz.y) / 3.14159f;
}
*/


// Flat Poly
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      center
// +32     float       radius
// +36     int         nverts
// +40     byte        red
// +41     byte        green
// +42     byte        blue
// +43     byte        pad
// +44     nverts*short*short  vertlist, smoothlist
void model_interp_flatpoly(ubyte * p,polymodel * pm)
{
	vertex *Interp_list[TMAP_MAX_VERTS];
	int nv = w(p+36);

	if ( nv < 0 )	return;

	#ifndef NDEBUG
	modelstats_num_polys++;
	#endif

	if (!g3_check_normal_facing(vp(p+20),vp(p+8)) ) return;
	

	int i;
	short * verts = (short *)(p+44);
	
	for (i=0;i<nv;i++)	{
		Interp_list[i] = &Interp_points[verts[i*2]];

		if ( Interp_flags & MR_NO_LIGHTING )	{
			if ( D3D_enabled || OGL_inited )	{
				Interp_list[i]->r = 191;
				Interp_list[i]->g = 191;
				Interp_list[i]->b = 191;
			} else {
				Interp_list[i]->b = 191;
			}
		} else {
			int vertnum = verts[i*2+0];
			int norm = verts[i*2+1];
	
			if ( Interp_flags & MR_NO_SMOOTHING )	{
				if ( D3D_enabled || OGL_inited )	{
					light_apply_rgb( &Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[vertnum], vp(p+8), Interp_light );
				} else {
					Interp_list[i]->b = light_apply( Interp_verts[vertnum], vp(p+8), Interp_light );
				}
			} else {
				// if we're not using saved lighting
				if ( !Interp_use_saved_lighting && !Interp_light_applied[norm] )	{
					if ( D3D_enabled || OGL_inited )	{
						light_apply_rgb( &Interp_lighting->r[norm], &Interp_lighting->g[norm], &Interp_lighting->b[norm], Interp_verts[vertnum], vp(p+8), Interp_light );
					} else {
						Interp_lighting->b[norm] = light_apply( Interp_verts[vertnum], Interp_norms[norm], Interp_light );
					}
					Interp_light_applied[norm] = 1;
				}

				if ( D3D_enabled || OGL_inited )	{
					Interp_list[i]->r = Interp_lighting->r[norm];
					Interp_list[i]->g = Interp_lighting->g[norm];
					Interp_list[i]->b = Interp_lighting->b[norm];
				} else {
					Interp_list[i]->b = Interp_lighting->b[norm];
				}
			}
		}
	}

	// HACK!!! FIX ME!!! I'M SLOW!!!!
	if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
		gr_set_color( *(p+40), *(p+41), *(p+42) );
	}

	if ( !(Interp_flags & MR_NO_POLYS))	{
		if ( D3D_enabled || OGL_inited )	{
			g3_draw_poly( nv, Interp_list, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB );	
		} else {
			g3_draw_poly( nv, Interp_list, TMAP_FLAG_GOURAUD | TMAP_FLAG_RAMP );	
		}
	}

	if (Interp_flags & (MR_SHOW_OUTLINE|MR_SHOW_OUTLINE_PRESET))	{
		int i, j;

		if ( Interp_flags & MR_SHOW_OUTLINE )	{
			gr_set_color_fast( &Interp_outline_color );
		}

		for (i=0; i<nv; i++ )	{
			j = (i + 1) % nv;
			g3_draw_line(Interp_list[i], Interp_list[j] );
		}
	}
}

// Textured Poly
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      center
// +32     float       radius
// +36     int         nverts
// +40     int         tmap_num
// +44     nverts*(model_tmap_vert) vertlist (n,u,v)
extern int Tmap_show_layers;

int Interp_subspace = 0;
float Interp_subspace_offset_u = 0.0f;
float Interp_subspace_offset_v = 0.0f;
ubyte Interp_subspace_r = 255;
ubyte Interp_subspace_g = 255;
ubyte Interp_subspace_b = 255;

void model_interp_tmappoly(ubyte * p,polymodel * pm)
{
	vertex *Interp_list[TMAP_MAX_VERTS];
	int i;
	int nv;
	model_tmap_vert *verts;

	int is_invisible = 0;

	if(Warp_Map < 0){
		if ((!Interp_thrust_scale_subobj) && (pm->textures[w(p+40)]<0))	{
			// Don't draw invisible polygons.
			if ( !(Interp_flags & MR_SHOW_INVISIBLE_FACES))	{
				return;
			} else {
				is_invisible = 1;
			}
		}
	}


	nv = w(p+36);

//	Tmap_show_layers = 1;

	#ifndef NDEBUG
	modelstats_num_polys++;
	#endif

	if(Warp_Map < 0){
		if (!g3_check_normal_facing(vp(p+20),vp(p+8)) && !(Interp_flags & MR_NO_CULL)) return;
	}

	if ( nv < 0 ) return;

	verts = (model_tmap_vert *)(p+44);


	for (i=0;i<nv;i++)	{
		Interp_list[i] = &Interp_points[verts[i].vertnum];

		Interp_list[i]->u = verts[i].u;
		Interp_list[i]->v = verts[i].v;
		
		if ( Interp_subspace )	{
			Interp_list[i]->v += Interp_subspace_offset_u;
			Interp_list[i]->u += Interp_subspace_offset_v;
			Interp_list[i]->r = Interp_subspace_r;
			Interp_list[i]->g = Interp_subspace_g;
			Interp_list[i]->b = Interp_subspace_b;
		} else {

	//		if ( !(pm->flags & PM_FLAG_ALLOW_TILING) )	{
	//			Assert(verts[i].u <= 1.0f );
	//			Assert(verts[i].v <= 1.0f );
	//		}

	//		Assert( verts[i].normnum == verts[i].vertnum );

			if ( (Interp_flags & MR_NO_LIGHTING) || (pm->ambient[w(p+40)]))	{	//gets the ambient glow to work
				if ( D3D_enabled || OGL_inited )	{
					Interp_list[i]->r = 191;
					Interp_list[i]->g = 191;
					Interp_list[i]->b = 191;
				} else {
					Interp_list[i]->b = 191;
				}
			} else {
				int vertnum = verts[i].vertnum;
				int norm = verts[i].normnum;
		
				if ( Interp_flags & MR_NO_SMOOTHING )	{
					if ( D3D_enabled || OGL_inited )	{
						light_apply_rgb( &Interp_list[i]->r, &Interp_list[i]->g, &Interp_list[i]->b, Interp_verts[vertnum], vp(p+8), Interp_light );
					} else {
						Interp_list[i]->b = light_apply( Interp_verts[vertnum], vp(p+8), Interp_light );
					}
				} else {					
					// if we're applying lighting as normal, and not using saved lighting
					if ( !Interp_use_saved_lighting && !Interp_light_applied[norm] )	{

						if ( D3D_enabled || OGL_inited )	{
							light_apply_rgb( &Interp_lighting->r[norm], &Interp_lighting->g[norm], &Interp_lighting->b[norm], Interp_verts[vertnum], Interp_norms[norm], Interp_light );

						} else {
							int li;
							ubyte l;
							l = light_apply( Interp_verts[vertnum], Interp_norms[norm], Interp_light );


							if ( Detail.lighting > 1 )	{
								// Add in precalculated muzzle flashes
								float fl = i2fl(l)/255.0f;
								ubyte *tmp = &Interp_lights[norm*pm->num_lights];

								for ( li=0; li<pm->num_lights; li++ )	{
									fl += i2fl(tmp[li])*pm->lights[li].value;
								}

								if ( fl < 0.0f )	{
									fl = 0.0f;
								} else if ( fl > 1.0f )	{
									fl = 1.0f;
								}

								l = (ubyte)fl2i(fl*255.0f);

							}

							Interp_lighting->b[norm] = l;
						}


						Interp_light_applied[norm] = 1;
					}

					if ( D3D_enabled || OGL_inited )	{
						Interp_list[i]->r = Interp_lighting->r[norm];
						Interp_list[i]->g = Interp_lighting->g[norm];
						Interp_list[i]->b = Interp_lighting->b[norm];
					} else {
						Interp_list[i]->b = Interp_lighting->b[norm];
					}
				}
			}
		}

//		Assert(verts[i].u >= 0.0f );
//		Assert(verts[i].v >= 0.0f );
	}

	#ifndef NDEBUG
	modelstats_num_polys_drawn++;
	#endif


	if (!(Interp_flags & MR_NO_POLYS) )		{
		if ( is_invisible )	{
			gr_set_color( 0, 255, 0 );
			g3_draw_poly( nv, Interp_list, 0 );		
		} else if (Interp_thrust_scale_subobj)	{
			if ((Interp_thrust_bitmap>-1)	&& (Interp_thrust_scale > 0.0f) && !Pofview_running) {
				gr_set_bitmap( Interp_thrust_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.2f );
				g3_draw_poly( nv, Interp_list, TMAP_FLAG_TEXTURED );		
			} else if(!Pofview_running){
				if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
					gr_set_color( 128, 128, 255 );
				}
				uint tflags = Interp_tmap_flags;
				tflags &=  (~(TMAP_FLAG_TEXTURED|TMAP_FLAG_TILED|TMAP_FLAG_CORRECT));
				g3_draw_poly( nv, Interp_list, tflags );		
			}
		} else if(Warp_Map > -1){	//warpin effect-Bobboau
			gr_set_bitmap( Warp_Map, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Warp_Alpha );
			g3_draw_poly( nv, Interp_list, TMAP_FLAG_TEXTURED );
		}else{
			// all textured polys go through here
			if ( Interp_tmap_flags & TMAP_FLAG_TEXTURED )	{
				// subspace special case
				if ( Interp_subspace && D3D_enabled )	{										
					gr_set_bitmap( pm->textures[w(p+40)], GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.2f );					
				}
				// all other textures
				else {					
					int texture;

					// if we're rendering a nebula background pof, maybe select a custom texture
					if((Interp_flags & MR_FORCE_TEXTURE) && (Interp_forced_bitmap >= 0)){
						texture = Interp_forced_bitmap;
					}else if(Interp_replacement_bitmap >= 0){
						texture = Interp_replacement_bitmap;
					} else {
						if (pm->is_ani[w(p+40)]){
							texture = pm->textures[w(p+40)] + ((timestamp() / (int)(pm->fps[w(p+40)])) % pm->num_frames[w(p+40)]);//here is were it picks the texture to render for ani-Bobboau
						}else{
							texture = pm->textures[w(p+40)];//here is were it picks the texture to render for normal-Bobboau
						}

						if(glow_maps_active)
						{
							if (pm->glow_is_ani[w(p+40)])
							{
								GLOWMAP = pm->glow_textures[w(p+40)] + ((timestamp() / (int)(pm->glow_fps[w(p+40)])) % pm->glow_numframes[w(p+40)]);
							}
							else
							{
								GLOWMAP = pm->glow_textures[w(p+40)];
							}
						}
					}

					// muzzle flashes draw xparent
					if(Interp_flags & MR_ALL_XPARENT){
						gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, Interp_xparent_alpha );
					} else {
						if(pm->transparent[w(p+40)]){	//trying to get transperent textures-Bobboau
							gr_set_bitmap( texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 0.8f );
						}else{
							gr_set_bitmap( texture );
						}
					}
				}
			} else {
				if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
					gr_set_color( 128, 128, 128 );
				}
			}

			if ( Interp_subspace )	{
				g3_draw_poly( nv, Interp_list,  TMAP_FLAG_TEXTURED|TMAP_FLAG_TILED|TMAP_FLAG_CORRECT );		
			} else {				
				if(Interp_flags & MR_ALL_XPARENT){
					g3_draw_poly( nv, Interp_list, Interp_tmap_flags );
				} else {
					g3_draw_poly( nv, Interp_list, Interp_tmap_flags|TMAP_FLAG_NONDARKENING );		
				}
			}

			GLOWMAP = -1;
		}
	}

	if (Interp_flags & (MR_SHOW_OUTLINE|MR_SHOW_OUTLINE_PRESET) )	{
	
		if ( Interp_flags & MR_SHOW_OUTLINE )	{
			gr_set_color_fast( &Interp_outline_color );
		}

		for (i=0; i<nv; i++ )	{
			int j = (i + 1) % nv;
			g3_draw_line(Interp_list[i], Interp_list[j] );
		}
	}

}

void interp_draw_box( vector *min, vector *max )
{
/*
	int i;
	vector bounding_box[8];
	vertex pts[8];
	vertex *l[4];

	model_calc_bound_box(bounding_box,min,max);

	for (i=0; i<8; i++ )	{
		g3_rotate_vertex( &pts[i], &bounding_box[i] );
	}

	gr_set_color(128,0,0);

	Tmap_show_layers = 1;

	l[3] = &pts[0];  l[2] = &pts[1]; l[1] = &pts[2];  l[0] = &pts[3];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[3];  l[2] = &pts[2]; l[1] = &pts[6];  l[0] = &pts[7];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[2];  l[2] = &pts[1]; l[1] = &pts[5];  l[0] = &pts[6];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[1];  l[2] = &pts[0]; l[1] = &pts[4];  l[0] = &pts[5];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[0];  l[2] = &pts[3]; l[1] = &pts[7];  l[0] = &pts[4];
	g3_draw_poly( 4, l, 0 );

	l[3] = &pts[4];  l[2] = &pts[7]; l[1] = &pts[6];  l[0] = &pts[5];
	g3_draw_poly( 4, l, 0 );
*/
}


// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      normal_point
// +32     int         tmp=0
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset
void model_interp_sortnorm_b2f(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

//	Assert( w(p+4) == 56 );

	int frontlist = w(p+36);
	int backlist = w(p+40);
	int prelist = w(p+44);
	int postlist = w(p+48);
	int onlist = w(p+52);

	if (prelist) model_interp_sub(p+prelist,pm,sm,do_box_check);		// prelist

	if (g3_check_normal_facing(vp(p+20),vp(p+8))) {		//facing

		//draw back then front

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

	}	else {			//not facing.  draw front then back

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);
	}

	if (postlist) model_interp_sub(p+postlist,pm,sm,do_box_check);		// postlist

}


// Sortnorms
// +0      int         id
// +4      int         size 
// +8      vector      normal
// +20     vector      normal_point
// +32     int         tmp=0
// 36     int     front offset
// 40     int     back offset
// 44     int     prelist offset
// 48     int     postlist offset
// 52     int     online offset
void model_interp_sortnorm_f2b(ubyte * p,polymodel * pm, bsp_info *sm, int do_box_check)
{
	#ifndef NDEBUG
	modelstats_num_sortnorms++;
	#endif

//	Assert( w(p+4) == 56 );

	int frontlist = w(p+36);
	int backlist = w(p+40);
	int prelist = w(p+44);
	int postlist = w(p+48);
	int onlist = w(p+52);

	if (postlist) model_interp_sub(p+postlist,pm,sm,do_box_check);		// postlist

	if (g3_check_normal_facing(vp(p+20),vp(p+8))) {		//facing

		// 

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

	}	else {			//not facing.  draw front then back

		//draw back then front

		if (backlist) model_interp_sub(p+backlist,pm,sm,do_box_check);

		if (onlist) model_interp_sub(p+onlist,pm,sm,do_box_check);			//onlist

		if (frontlist) model_interp_sub(p+frontlist,pm,sm,do_box_check);

	}

	if (prelist) model_interp_sub(p+prelist,pm,sm,do_box_check);		// prelist
}




void model_draw_debug_points( polymodel *pm, bsp_info * submodel )
{
	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	// Draw the brown "core" mass
//	if ( submodel && (submodel->parent==-1) )	{
//		gr_set_color(128,128,0);
//		g3_draw_sphere_ez( &vmd_zero_vector, pm->core_radius );
//	}

	// Draw a red pivot point
	gr_set_color(128,0,0);
	g3_draw_sphere_ez(&vmd_zero_vector, 2.0f );

	// Draw a green center of mass when drawing the hull
	if ( submodel && (submodel->parent==-1) )	{
		gr_set_color(0,128,0);
		g3_draw_sphere_ez( &pm->center_of_mass, 1.0f );
	}

	if ( submodel )	{
		// Draw a blue center point
		gr_set_color(0,0,128);
		g3_draw_sphere_ez( &submodel->geometric_center, 0.9f );
	}
	
	// Draw the bounding box
	int i;
	vertex pts[8];

	if ( submodel )	{
		for (i=0; i<8; i++ )	{
			g3_rotate_vertex( &pts[i], &submodel->bounding_box[i] );
		}
		gr_set_color(128,128,128);
		g3_draw_line( &pts[0], &pts[1] );
		g3_draw_line( &pts[1], &pts[2] );
		g3_draw_line( &pts[2], &pts[3] );
		g3_draw_line( &pts[3], &pts[0] );

		g3_draw_line( &pts[4], &pts[5] );
		g3_draw_line( &pts[5], &pts[6] );
		g3_draw_line( &pts[6], &pts[7] );
		g3_draw_line( &pts[7], &pts[4] );

		g3_draw_line( &pts[0], &pts[4] );
		g3_draw_line( &pts[1], &pts[5] );
		g3_draw_line( &pts[2], &pts[6] );
		g3_draw_line( &pts[3], &pts[7] );
	} else {
		//for (i=0; i<8; i++ )	{
		//	g3_rotate_vertex( &pts[i], &pm->bounding_box[i] );
		//}
		gr_set_color(0,255,0);

		int j;
		for (j=0; j<8; j++ )	{

			vector	bounding_box[8];		// caclulated fron min/max
			model_calc_bound_box(bounding_box,&pm->octants[j].min,&pm->octants[j].max);

			for (i=0; i<8; i++ )	{
				g3_rotate_vertex( &pts[i], &bounding_box[i] );
			}
			gr_set_color(128,0,0);
			g3_draw_line( &pts[0], &pts[1] );
			g3_draw_line( &pts[1], &pts[2] );
			g3_draw_line( &pts[2], &pts[3] );
			g3_draw_line( &pts[3], &pts[0] );

			g3_draw_line( &pts[4], &pts[5] );
			g3_draw_line( &pts[5], &pts[6] );
			g3_draw_line( &pts[6], &pts[7] );
			g3_draw_line( &pts[7], &pts[4] );

			g3_draw_line( &pts[0], &pts[4] );
			g3_draw_line( &pts[1], &pts[5] );
			g3_draw_line( &pts[2], &pts[6] );
			g3_draw_line( &pts[3], &pts[7] );			
		}		
	}
}

// Debug code to show all the paths of a model
void model_draw_paths( int model_num )
{
	int i,j;
	vector pnt;
	polymodel * pm;	

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	pm = model_get(model_num);

	if (pm->n_paths<1){
		return;
	}	

	for (i=0; i<pm->n_paths; i++ )	{
		vertex prev_pnt;

		for (j=0; j<pm->paths[i].nverts; j++ )	{
			// Rotate point into world coordinates			
			pnt = pm->paths[i].verts[j].pos;

			// Pnt is now the x,y,z world coordinates of this vert.
			// For this example, I am just drawing a sphere at that
			// point.
			{
				vertex tmp;
				g3_rotate_vertex(&tmp,&pnt);

				if ( pm->paths[i].verts[j].nturrets > 0 ){
					gr_set_color( 0, 0, 255 );						// draw points covered by turrets in blue
				} else {
					gr_set_color( 255, 0, 0 );
				}

//				g3_draw_sphere( &tmp, pm->paths[i].verts[j].radius );
				g3_draw_sphere( &tmp, 0.5f );

				gr_set_color( 255, 0, 0 );
				if (j){
					g3_draw_line(&prev_pnt, &tmp);
				}

				prev_pnt = tmp;
			}
		}
	}
}


// docking bay and fighter bay paths
void model_draw_bay_paths(int model_num)
{
	int idx, s_idx;
	vector v1, v2;
	vertex l1, l2;	

	polymodel *pm = model_get(model_num);
	if(pm == NULL){
		return;
	}

	// render docking bay normals
	gr_set_color(0, 255, 0);
	for(idx=0; idx<pm->n_docks; idx++){
		for(s_idx=0; s_idx<pm->docking_bays[idx].num_slots; s_idx++){
			v1 = pm->docking_bays[idx].pnt[s_idx];
			vm_vec_scale_add(&v2, &v1, &pm->docking_bays[idx].norm[s_idx], 10.0f);

			// rotate the points
			g3_rotate_vertex(&l1, &v1);
			g3_rotate_vertex(&l2, &v2);

			// draw the point and normal
			g3_draw_sphere(&l1, 2.0);
			g3_draw_line(&l1, &l2);
		}
	}

	// render figher bay paths
	gr_set_color(0, 255, 255);
		
	// iterate through the paths that exist in the polymodel, searching for $bayN pathnames
	for (idx = 0; idx<pm->n_paths; idx++) {
		if ( !strnicmp(pm->paths[idx].name, NOX("$bay"), 4) ) {						
			for(s_idx=0; s_idx<pm->paths[idx].nverts-1; s_idx++){
				v1 = pm->paths[idx].verts[s_idx].pos;
				v2 = pm->paths[idx].verts[s_idx+1].pos;

				// rotate and draw
				g3_rotate_vertex(&l1, &v1);
				g3_rotate_vertex(&l2, &v2);
				g3_draw_line(&l1, &l2);
			}
		}
	}	
}	
/*
// struct that holds the indicies into path information associated with a fighter bay on a capital ship
// NOTE: Fighter bay paths are identified by the path_name $bayN (where N is numbered from 1).
//			Capital ships only have ONE fighter bay on the entire ship
#define MAX_SHIP_BAY_PATHS		10
typedef struct ship_bay {
	int	num_paths;							// how many paths are associated with the model's fighter bay
	int	paths[MAX_SHIP_BAY_PATHS];		// index into polymodel->paths[] array
	int	arrive_flags;	// bitfield, set to 1 when that path number is reserved for an arrival
	int	depart_flags;	// bitfield, set to 1 when that path number is reserved for a departure
} ship_bay;

  typedef struct mp_vert {
	vector		pos;				// xyz coordinates of vertex in object's frame of reference
	int			nturrets;		// number of turrets guarding this vertex
	int			*turret_ids;	// array of indices into ship_subsys linked list (can't index using [] though)
	float			radius;			// How far the closest obstruction is from this vertex
} mp_vert;

typedef struct model_path {
	char			name[MAX_NAME_LEN];					// name of the subsystem.  Probably displayed on HUD
	char			parent_name[MAX_NAME_LEN];			// parent name of submodel that path is linked to in POF
	int			parent_submodel;
	int			nverts;
	mp_vert		*verts;
	int			goal;			// Which of the verts is the one closest to the goal of this path
	int			type;			// What this path takes you to... See MP_TYPE_??? defines above for details
	int			value;		// This depends on the type.
									// For MP_TYPE_UNUSED, this means nothing.
									// For MP_TYPE_SUBSYS, this is the subsystem number this path takes you to.
} model_path;
*/


void interp_render_arc_segment( vector *v1, vector *v2, int depth )
{
	float d = vm_vec_dist_quick( v1, v2 );

	if ( d < 0.30f || (depth>4) )	{
		vertex p1, p2;
		g3_rotate_vertex( &p1, v1 );
		g3_rotate_vertex( &p2, v2 );

		//g3_draw_rod( v1, 0.2f, v2, 0.2f, NULL, 0);
		g3_draw_line( &p1, &p2 );
	} else {
		// divide in half
		vector tmp;
		vm_vec_avg( &tmp, v1, v2 );
	
		float scaler = 0.30f;
		tmp.xyz.x += (frand()-0.5f)*d*scaler;
		tmp.xyz.y += (frand()-0.5f)*d*scaler;
		tmp.xyz.z += (frand()-0.5f)*d*scaler;
		
		interp_render_arc_segment( v1, &tmp, depth+1 );
		interp_render_arc_segment( &tmp, v2, depth+1 );
	}
}

int Interp_lightning = 1;
DCF_BOOL( Arcs, Interp_lightning )

int AR = 64;
int AG = 64;
int AB = 5;
int AR2 = 128;
int AG2 = 128;
int AB2 = 10;
void interp_render_lightning( polymodel *pm, bsp_info * sm )
{
	Assert( sm->num_arcs > 0 );

	int i;

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	if (!Interp_lightning) return;

//	if ( keyd_pressed[KEY_LSHIFT] ) return;
//	if ( rad < 3.0f ) return;	
	
	for (i=0; i<sm->num_arcs; i++ )	{
		// pick a color based upon arc type
		switch(sm->arc_type[i]){
		// "normal", Freespace 1 style arcs
		case MARC_TYPE_NORMAL:
			if ( (rand()>>4) & 1 )	{
				gr_set_color( 64, 64, 255 );
			} else {
				gr_set_color( 128, 128, 255 );
			}
			break;

		// "EMP" style arcs
		case MARC_TYPE_EMP:
			if ( (rand()>>4) & 1 )	{
				gr_set_color( AR, AG, AB );
			} else {
				gr_set_color( AR2, AG2, AB2 );
			}
			break;

		default:
			Int3();
		}

		// render the actual arc segment
		interp_render_arc_segment( &sm->arc_pts[i][0], &sm->arc_pts[i][1], 0 );
	}
}

void model_interp_subcall(polymodel * pm, int mn, int detail_level)
{
	int i;
	int zbuf_mode = gr_zbuffering_mode;

	if ( (mn < 0) || (mn>=pm->n_models) )
		return;

	Assert( mn >= 0 );
	Assert( mn < pm->n_models );

//	mprintf(( "Name = '%s'\n", pm->submodel[mn].name ));
//	char * p = pm->submodel[mn].name;



//	mprintf(("model =%s, submodel=%s, interp offset=%f %f %f\n",pm->filename, pm->submodel[mn].name,Interp_offset.xyz.x,
//															Interp_offset.xyz.y,Interp_offset.xyz.z));
	
	if (pm->submodel[mn].blown_off){
		return;
	}

	if (pm->submodel[mn].is_thruster )	{
		if ( !(Interp_flags & MR_SHOW_THRUSTERS) ){
			return;
		}
		Interp_thrust_scale_subobj=1;
	} else {
		Interp_thrust_scale_subobj=0;
	}

	vm_vec_add2(&Interp_offset,&pm->submodel[mn].offset);

	g3_start_instance_angles(&pm->submodel[mn].offset, &pm->submodel[mn].angs);
	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}

	model_interp_sub( pm->submodel[mn].bsp_data, pm, &pm->submodel[mn], 0 );

	if (Interp_flags & MR_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[mn] );

	if ( pm->submodel[mn].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[mn]);
	}

	i = pm->submodel[mn].first_child;
	while( i>-1 )	{
		if (!pm->submodel[i].is_thruster )	{
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			} else {
				zbuf_mode = GR_ZBUFF_FULL;		// read only
			}

			gr_zbuffer_set(zbuf_mode);

			model_interp_subcall( pm, i, detail_level );
		}
		i = pm->submodel[i].next_sibling;
	}

	vm_vec_sub2(&Interp_offset,&pm->submodel[mn].offset);


	g3_done_instance();
}

// Returns one of the following
#define IBOX_ALL_OFF 0
#define IBOX_ALL_ON 1
#define IBOX_SOME_ON_SOME_OFF 2

int interp_box_offscreen( vector *min, vector *max )
{
	if ( keyd_pressed[KEY_LSHIFT] )	{
		return IBOX_ALL_ON;
	}

	vector v[8];
	v[0].xyz.x = min->xyz.x; v[0].xyz.y = min->xyz.y; v[0].xyz.z = min->xyz.z;
	v[1].xyz.x = max->xyz.x; v[1].xyz.y = min->xyz.y; v[1].xyz.z = min->xyz.z;
	v[2].xyz.x = max->xyz.x; v[2].xyz.y = max->xyz.y; v[2].xyz.z = min->xyz.z;
	v[3].xyz.x = min->xyz.x; v[3].xyz.y = max->xyz.y; v[3].xyz.z = min->xyz.z;

	v[4].xyz.x = min->xyz.x; v[4].xyz.y = min->xyz.y; v[4].xyz.z = max->xyz.z;
	v[5].xyz.x = max->xyz.x; v[5].xyz.y = min->xyz.y; v[5].xyz.z = max->xyz.z;
	v[6].xyz.x = max->xyz.x; v[6].xyz.y = max->xyz.y; v[6].xyz.z = max->xyz.z;
	v[7].xyz.x = min->xyz.x; v[7].xyz.y = max->xyz.y; v[7].xyz.z = max->xyz.z;

	ubyte and_codes = 0xff;
	ubyte or_codes = 0xff;
	int i;

	for (i=0; i<8; i++ )	{
		vertex tmp;
		ubyte codes=g3_rotate_vertex( &tmp, &v[i] );
// Early out which we cannot do because we want to differentiate btwn
// IBOX_SOME_ON_SOME_OFF and IBOX_ALL_ON
//		if ( !codes )	{
//			//mprintf(( "A point is inside, so render it.\n" ));
//			return 0;		// this point is in, so return 0
//		}
		or_codes |= codes;
		and_codes &= codes;
	}

	// If and_codes is set this means that all points lie off to the
	// same side of the screen.
	if (and_codes)	{
		//mprintf(( "All points offscreen, so don't render it.\n" ));
		return IBOX_ALL_OFF;	//all points off screen
	}

	// If this is set it means at least one of the points is offscreen,
	// but they aren't all off to the same side.
	if (or_codes)	{
		return IBOX_SOME_ON_SOME_OFF;
	}

	// They are all onscreen.
	return IBOX_ALL_ON;	
}


//calls the object interpreter to render an object.  
//returns true if drew
int model_interp_sub(void *model_ptr, polymodel * pm, bsp_info *sm, int do_box_check )
{
	ubyte *p = (ubyte *)model_ptr;
	int chunk_type, chunk_size;
	int pushed = 0;

	chunk_type = w(p);
	chunk_size = w(p+4);


	
	while ( chunk_type != OP_EOF )	{

//		mprintf(( "Processing chunk type %d, len=%d\n", chunk_type, chunk_size ));

		switch (chunk_type) {
		case OP_EOF: return 1;
		case OP_DEFPOINTS:		model_interp_defpoints(p,pm,sm); break;
		case OP_FLATPOLY:		model_interp_flatpoly(p,pm); break;
		case OP_TMAPPOLY:
			// possible texture replacements - Goober5000
			if (Interp_replacement_textures)
			{
				model_set_replacement_bitmap(Interp_replacement_textures[w(p+40)]);
			}
			else
			{
				model_set_replacement_bitmap(-1);
			}
			
			model_interp_tmappoly(p,pm);
			break;
		case OP_SORTNORM:		model_interp_sortnorm(p,pm,sm,do_box_check); break;
	
		case OP_BOUNDBOX:		

			if ( do_box_check )	{
				int retval = interp_box_offscreen( vp(p+8), vp(p+20) );
				switch( retval )	{
				case IBOX_ALL_OFF:
					goto DoneWithThis;	// Don't need to draw any more polys from this box
					break;

				case IBOX_ALL_ON:
					do_box_check = 0;		// Don't need to check boxes any more
					break;

				case IBOX_SOME_ON_SOME_OFF:
					// continue like we were
					break;
				default:
					Int3();
				}
			}


			if (Interp_flags & MR_SHOW_PIVOTS )	{
				#ifndef NDEBUG
				modelstats_num_boxes++;
				#endif
				interp_draw_box( vp(p+8), vp(p+20) );
			}

			if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
				if ( pushed )	{
					light_filter_pop();
					pushed = 0;

				}
				light_filter_push_box( vp(p+8), vp(p+20) );
				pushed = 1;
			}
			break;

		default:
			mprintf(( "Bad chunk type %d, len=%d in model_interp_sub\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}

DoneWithThis:

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		if ( pushed )	{
			light_filter_pop();
			pushed = 0;
		}
	}

	return 1;
}


void model_render_shields( polymodel * pm )
{
	model_set_outline_color(255,255,255);
	int i, j;
	shield_tri *tri;
	vertex pnt0, tmp, prev_pnt;

	if ( Interp_flags & MR_SHOW_OUTLINE_PRESET )	{
		return;
	}

	gr_set_color(0, 0, 200 );

	//	Scan all the triangles in the mesh.
	for (i=0; i<pm->shield.ntris; i++ )	{

		tri = &pm->shield.tris[i];

		if (g3_check_normal_facing(&pm->shield.verts[tri->verts[0]].pos,&tri->norm ) ) {

			//	Process the vertices.
			//	Note this rotates each vertex each time it's needed, very dumb.
			for (j=0; j<3; j++ )	{

				g3_rotate_vertex(&tmp, &pm->shield.verts[tri->verts[j]].pos );

				if (j)
					g3_draw_line(&prev_pnt, &tmp);
				else
					pnt0 = tmp;
				prev_pnt = tmp;
			}

			g3_draw_line(&pnt0, &prev_pnt);
		}
	}
}

void model_render_insignias(polymodel *pm, int detail_level)
{
	int idx, s_idx;
	vertex vecs[3];
	vertex *vlist[3] = { &vecs[0], &vecs[1], &vecs[2] };
	vector t1, t2, t3, x;
	int i1, i2, i3;

	x.xyz.x=0;
	x.xyz.y=0;
	x.xyz.z=0;
	// if the model has no insignias we're done
	if(pm->num_ins <= 0){
		return;
	}

	// set the proper texture
	if(Interp_insignia_bitmap >= 0){		
		gr_set_bitmap(Interp_insignia_bitmap);
	}
	// otherwise don't even bother rendering
	else {
		return;
	}

	// otherwise render them	
	for(idx=0; idx<pm->num_ins; idx++){	
		// skip insignias not on our detail level
		if(pm->ins[idx].detail_level != detail_level){
			continue;
		}

		for(s_idx=0; s_idx<pm->ins[idx].num_faces; s_idx++){
			// get vertex indices
			i1 = pm->ins[idx].faces[s_idx][0];
			i2 = pm->ins[idx].faces[s_idx][1];
			i3 = pm->ins[idx].faces[s_idx][2];

			// transform vecs and setup vertices
			vm_vec_add(&t1, &pm->ins[idx].vecs[i1], &pm->ins[idx].offset);
			vm_vec_add(&t2, &pm->ins[idx].vecs[i2], &pm->ins[idx].offset);
			vm_vec_add(&t3, &pm->ins[idx].vecs[i3], &pm->ins[idx].offset);
			g3_rotate_vertex(&vecs[0], &t1);
			g3_rotate_vertex(&vecs[1], &t2);
			g3_rotate_vertex(&vecs[2], &t3);
			
			// setup texture coords
			vecs[0].u = pm->ins[idx].u[s_idx][0];  vecs[0].v = pm->ins[idx].v[s_idx][0];
			vecs[1].u = pm->ins[idx].u[s_idx][1];  vecs[1].v = pm->ins[idx].v[s_idx][1];
			vecs[2].u = pm->ins[idx].u[s_idx][2];  vecs[2].v = pm->ins[idx].v[s_idx][2];

			// vm_vec_avg3(&x, 
/*
			for(int k =0; k < 3; k++){
				if ( D3D_enabled )	{
					light_apply_rgb( &vecs[k].r, &vecs[k].g, &vecs[k].b, &pm->ins[idx].vecs[k], &pm->ins[idx].norm[k], 0.8f );
				} else {
					vecs[k].b = light_apply( &x, &pm->ins[idx].norm[k], 0.8f );
				}
			}
			vecs[k].r = (ubyte)255;*/
//			gr_printf((0), (0), "r %d, g %d, b %d", (int)vecs[k].r, (int)vecs[k].g, (int)vecs[k].b);
			// draw the polygon
			g3_draw_poly(3, vlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT );
		//	g3_draw_poly(3, vlist, 0);
		}
	}
}

int Model_texturing = 1;
int Model_polys = 1;

DCF_BOOL( model_texturing, Model_texturing )
DCF_BOOL( model_polys, Model_polys )

MONITOR( NumModelsRend );	
MONITOR( NumHiModelsRend );	
MONITOR( NumMedModelsRend );	
MONITOR( NumLowModelsRend );	


typedef struct model_cache {
	int		model_num;
	//matrix	orient;
	vector	pos;
	int		num_lights;

	float		last_dot;

	float		cr;

	int		w, h;
	ubyte		*data;
	int		cached_valid;
	int		bitmap_id;

	angles	angs;

	// thrust stuff
	float		thrust_scale;
	int		thrust_bitmap;
	int		thrust_glow_bitmap;
	float		thrust_glow_noise;

	int		last_frame_rendered;		//	last frame in which this model was rendered not from the cache
} model_cache;

#define MAX_MODEL_CACHE MAX_OBJECTS
model_cache Model_cache[MAX_MODEL_CACHE];		// Indexed by objnum
int Model_cache_inited = 0;



// Returns 0 if not valid points
int model_cache_calc_coords(vector *pnt,float rad, float *cx, float *cy, float *cr)
{
	vertex pt;
	ubyte flags;

	flags = g3_rotate_vertex(&pt,pnt);

	if (flags == 0) {

		g3_project_vertex(&pt);

		if (!(pt.flags & (PF_OVERFLOW|CC_BEHIND)))	{

			*cx = pt.sx;
			*cy = pt.sy;
			*cr = rad*Matrix_scale.xyz.x*Canv_w2/pt.z;

			if ( *cr < 1.0f )	{
				*cr = 1.0f;
			}

			int x1, x2, y1, y2;

			x1 = fl2i(*cx-*cr); 
			if ( x1 < gr_screen.clip_left ) return 0;
			x2 = fl2i(*cx+*cr);
			if ( x2 > gr_screen.clip_right ) return 0;
			y1 = fl2i(*cy-*cr);
			if ( y1 < gr_screen.clip_top ) return 0;
			y2 = fl2i(*cy+*cr);
			if ( y2 > gr_screen.clip_bottom ) return 0;

			return 1;
		}
	}
	return 0;
}


//draws a bitmap with the specified 3d width & height 
//returns 1 if off screen, 0 if not
int model_get_rotated_bitmap_points(vertex *pnt,float angle, float rad, vertex *v)
{
	float sa, ca;
	int i;

	Assert( G3_count == 1 );



//	angle = 0.0f;
		
	sa = (float)sin(angle);
	ca = (float)cos(angle);

	float width, height;

	width = height = rad;

	v[0].x = (-width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[0].y = (-width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[0].z = pnt->z;
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;

	v[1].x = (width*ca - height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[1].y = (width*sa + height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[1].z = pnt->z;
	v[1].sw = 0.0f;
	v[1].u = 1.0f;
	v[1].v = 0.0f;

	v[2].x = (width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[2].y = (width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[2].z = pnt->z;
	v[2].sw = 0.0f;
	v[2].u = 1.0f;
	v[2].v = 1.0f;

	v[3].x = (-width*ca + height*sa)*Matrix_scale.xyz.x + pnt->x;
	v[3].y = (-width*sa - height*ca)*Matrix_scale.xyz.y + pnt->y;
	v[3].z = pnt->z;
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 1.0f;

	ubyte codes_and=0xff;

	float sw,z;
	z = pnt->z - rad / 4.0f;
	if ( z < 0.0f ) z = 0.0f;
	sw = 1.0f / z;

	for (i=0; i<4; i++ )	{
		//now code the four points
		codes_and &= g3_code_vertex(&v[i]);
		v[i].flags = 0;		// mark as not yet projected
		g3_project_vertex(&v[i]);
		v[i].sw = sw;
	}

	if (codes_and)
		return 1;		//1 means off screen

	return 0;
}


int Model_caching = 1;
DCF_BOOL( model_caching, Model_caching );

extern int Tmap_scan_read;		// 0 = normal mapper, 1=read, 2=write

#define MODEL_MAX_BITMAP_SIZE 128
ubyte tmp_bitmap[MODEL_MAX_BITMAP_SIZE*MODEL_MAX_BITMAP_SIZE];

void mc_get_bmp( ubyte *data, int x, int y, int w, int h )
{
	gr_lock();

	int i,j;

	for (i = 0; i < h; i++)	{
		ubyte *dptr = GR_SCREEN_PTR(ubyte,x,i+y);
		ubyte *sptr = data+(i*w);
		for (j=0; j<w; j++ )	{
			*sptr++ = *dptr;
			*dptr++ = 255;				// XPARENT!
		}
	}

	gr_unlock();
}

void mc_put_bmp( ubyte *data, int x, int y, int w, int h )
{
	gr_lock();

	int i,j;

	for (i = 0; i < h; i++)	{
		ubyte *dptr = GR_SCREEN_PTR(ubyte,x,i+y);
		ubyte *sptr = data+(i*w);
		for (j=0; j<w; j++ )	{
			*dptr++ = *sptr++;
		}
	}

	gr_unlock();
}

float Interp_depth_scale = 1500.0f;

DCF(model_darkening,"Makes models darker with distance")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT);
		Interp_depth_scale = Dc_arg_float;
	}

	if ( Dc_help )	{
		dc_printf( "Usage: model_darkening float\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "model_darkening = %.1f\n", Interp_depth_scale );
	}
}


		// Compare it to 999.75f at R = 64.0f
		//               0.0000f at R = 4.0f
		
		//float cmp_val = 999.75f;		// old

// Return a number from 'min' to 'max' where it is
// 'v' is between v1 and v2.
float scale_it( float min, float max, float v, float v1, float v2 )
{
	if ( v < v1 ) return min;
	if ( v > v2 ) return max;

	v = (v - v1)/(v2-v1);
	v = v*(max-min)+min;

	return v;
}

void model_render(int model_num, matrix *orient, vector * pos, uint flags, int objnum, int lighting_skip, int *replacement_textures)
{
	// replacement textures - Goober5000
	model_set_replacement_textures(replacement_textures);

	polymodel *pm = model_get(model_num);

	int time = timestamp();
	for (int i = 0; i < pm->n_glows; i++ ) { //glow point blink code -Bobboau
		glow_bank *bank = &pm->glows[i];
		if (bank->glow_timestamp == 0)
			bank->glow_timestamp=time;
		if(bank->off_time){
		//	time += bank->disp_time;
			if(bank->is_on){
				if( (bank->on_time) > ((time - bank->disp_time) % (bank->on_time + bank->off_time)) ){
					bank->glow_timestamp=time;
					bank->is_on=0;
				}
			}else{
				if( (bank->off_time) < ((time - bank->disp_time) % (bank->on_time + bank->off_time)) ){
					bank->glow_timestamp=time;
					bank->is_on=1;
				}
			}
		}
	}


	// maybe turn off (hardware) culling
	if(flags & MR_NO_CULL){
		gr_set_cull(0);
	}

	Interp_objnum = objnum;


/*
	vector thruster_pnt=vmd_zero_vector;
	vector rotated_thruster;
	//maybe add lights by engine glows
	if (Interp_flags & MR_SHOW_THRUSTERS) 
	{
		for (int thruster_num=0; thruster_num < pm->n_thrusters; thruster_num++)
		{
			for (int slot_num=0; slot_num < pm->thrusters[thruster_num].num_slots; slot_num++)
			{
				thruster_pnt=pm->thrusters[thruster_num].pnt[slot_num];
				//vm_vec_rotate(&rotated_thruster,&thruster_pnt,orient);
				
				light_add_point(&rotated_thruster,pm->thrusters[thruster_num].radius[slot_num]*5,pm->thrusters[thruster_num].radius[slot_num]*10,1.0f,0.0f,0.0f,1.0f,-1);

		//		gr_set_color(255,255,255);
			//	g3_draw_sphere_ez(&rotated_thruster,pm->thrusters[thruster_num].radius[slot_num]*15);
			}
		}
	}*/

	if ( flags & MR_NO_LIGHTING )	{
		Interp_light = 1.0f;

		// never use saved lighitng for this object
		model_set_saved_lighting(-1, -1);
	} else if ( flags & MR_IS_ASTEROID ) {
		// Dim it based on distance
		float depth = vm_vec_dist_quick( pos, &Eye_position );
		if ( depth > Interp_depth_scale )	{
			Interp_light = Interp_depth_scale/depth;
			// If it is too far, exit
			if ( Interp_light < (1.0f/32.0f) ) {
				Interp_light = 0.0f;
				return;
			} else if ( Interp_light > 1.0f )	{
				Interp_light = 1.0f;
			}
		} else {
			Interp_light = 1.0f;
		}

		// never use saved lighitng for this object
		model_set_saved_lighting(-1, -1);
	} else {
		Interp_light = 1.0f;

		// maybe use saved lighting
		model_set_saved_lighting(objnum, hack_skip_max);
	}

	int num_lights = 0;

	if ( !(flags & MR_NO_LIGHTING ) )	{
		if ( D3D_enabled || OGL_inited  )	{
			num_lights = light_filter_push( objnum, pos, pm->rad );
		} else {
			num_lights = light_filter_push( objnum, pos, pm->rad );
		}
	}

	model_try_cache_render(model_num, orient, pos, flags, objnum, num_lights );
	
	if ( !(flags & MR_NO_LIGHTING ) )	{
		light_filter_pop();
	}

	// maybe turn culling back on
	if(flags & MR_NO_CULL){
		gr_set_cull(1);
	}

	// turn off fog after each model renders
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}
}


void model_cache_init()
{
	if ( !Model_cache_inited )	{
		int i;

		Model_cache_inited = 1;

		for (i=0; i<MAX_MODEL_CACHE; i++ )	{
			Model_cache[i].cached_valid = 0;
			Model_cache[i].data = NULL;
			Model_cache[i].bitmap_id = -1;
			Model_cache[i].last_frame_rendered = -1;
		}
	}
}

void model_cache_reset()
{
	if ( Model_cache_inited )	{
		int i;

		for (i=0; i<MAX_MODEL_CACHE; i++ )	{
			Model_cache[i].cached_valid = 0;
			if ( Model_cache[i].data )	{
				free(Model_cache[i].data);
				Model_cache[i].data = NULL;
			}
			if ( Model_cache[i].bitmap_id != -1 )	{
				bm_release(Model_cache[i].bitmap_id);
				Model_cache[i].bitmap_id = -1;
			}
		}
	}
}

// tmp_detail_level
// 0 - Max
// 1
// 2
// 3
// 4 - None

#if MAX_DETAIL_LEVEL != 4
#error MAX_DETAIL_LEVEL is assumed to be 4 in ModelInterp.cpp
#endif

// Given detail level, what is the threshold for how far viewer
// can move in the object's frame of reference before a redraw.
float Mc_viewer_pos_factor[MAX_DETAIL_LEVEL+1] = {  0.080f, 0.040f, 0.020f, 0.010f, 0.0f };
float Mc_size_factor[MAX_DETAIL_LEVEL+1] = {  1.40f, 1.30f, 1.20f, 1.10f, 0.0f };

int Model_object_caching_tmp = MAX_DETAIL_LEVEL;

// When framerate goes below this, knock it down a notch.
float Mc_framerate_lo[MAX_DETAIL_LEVEL+1] = { 0.0f, 10.0f, 15.0f, 20.0f, 25.0f };
// When it goes above this, knock it up a notch.
float Mc_framerate_hi[MAX_DETAIL_LEVEL+1] = { 15.0f, 20.0f, 25.0f, 30.0f, 100000.0f };

int Mc_detail_add[MAX_DETAIL_LEVEL+1] = { -2, -1, +1, +2, +4 };

extern float flFrametime;

void model_try_cache_render(int model_num, matrix *orient, vector * pos, uint flags, int objnum, int num_lights)
{
	model_really_render(model_num, orient, pos, flags, objnum);
	/*
	int i;

	model_cache *mc = NULL;
	
	if ( (objnum>-1) && (objnum<MAX_MODEL_CACHE) )	{
		mc = &Model_cache[objnum];
	}
	
	if ( (!mc) || (!Model_caching) || (D3D_enabled) || (!Model_cache_inited) || (flags & MR_ALWAYS_REDRAW) || (Detail.object_caching > 3) )	{
		if ( mc )	{
			mc->cached_valid = 0;
		}
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	Assert( mc != NULL );

	// Fake the detail level based on framerate.
	if ( 1.0f / flFrametime < Mc_framerate_lo[Model_object_caching_tmp] )	{
		Model_object_caching_tmp--;
		//	mprintf(( "Model cache level bumped down to %d\n", Model_object_caching ));
	} else if ( 1.0f / flFrametime > Mc_framerate_hi[Model_object_caching_tmp] )	{
		Model_object_caching_tmp++;
		//	mprintf(( "Model cache level bumped up to %d\n", Model_object_caching ));
	}

	int tmp_detail_level = Model_object_caching_tmp + Mc_detail_add[Detail.object_caching];

	if ( tmp_detail_level < 0 )	{
		tmp_detail_level = 0;
	} else if (tmp_detail_level > MAX_DETAIL_LEVEL )  {
		tmp_detail_level = MAX_DETAIL_LEVEL;
	}

	if ( tmp_detail_level > 3 )	{
		if ( mc )	{
			mc->cached_valid = 0;
		}
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	
//	static int last_one = -1;
//	if ( last_one != tmp_detail_level )	{
//		last_one = tmp_detail_level;
//		mprintf(( "Detail level %d\n", tmp_detail_level ));
//	}

//	if ( keyd_pressed[KEY_LSHIFT] )	{
//		mc->cached_valid = 0;
//		model_really_render(model_num, orient, pos, flags, objnum );
//		return;
//	}


//	mprintf(( "Rendering cache model\n" ));

	polymodel *pm = model_get(model_num);
	vertex v[4];
	vertex *vertlist[4] = { &v[0], &v[1], &v[2], &v[3] };
	float cx, cy, cr;
	vertex pt;
	ubyte ccflags;

	matrix tempm, tempm2;
	angles new_angles;

	vm_copy_transpose_matrix(&tempm2,orient);
	vm_matrix_x_matrix(&tempm,&tempm2,&Eye_matrix);
	vm_extract_angles_matrix(&new_angles, &tempm );
	
	if ( !model_cache_calc_coords(pos,pm->rad, &cx, &cy, &cr) )	{
		// Not onscreen, do a real render and exit
		mc->cached_valid = 0;
		model_really_render(model_num, orient, pos, flags, objnum );
		return;
	}

	//================================================================
	// A bunch of checks to see if we need to redraw the model or not

	
	vector ship_to_eye;

	vm_vec_sub( &ship_to_eye, &Eye_position, pos );
	vm_vec_normalize_safe(&ship_to_eye);
	float this_dot = vm_vec_dot( &ship_to_eye, &orient->fvec );
	this_dot += vm_vec_dot( &ship_to_eye, &orient->rvec );

	float diff = 0.0f;
	
	if ( !mc->cached_valid )	{
		// Nothing cached
		goto RedrawIt;
	}

	Assert( mc->data != NULL );

	if (Framecount - mc->last_frame_rendered > 1 + 2*(MAX_DETAIL_LEVEL - Detail.object_caching - 1)) {
		goto RedrawIt;
	}

	diff = fl_abs( this_dot - mc->last_dot );

	if ( diff > Mc_viewer_pos_factor[tmp_detail_level] )	{
//		mprintf(( "Redraw!!! %.4f\n", diff ));
		goto RedrawIt;
	}

//	if ( keyd_pressed[KEY_LSHIFT] )	{
//		goto RedrawIt;
//	}

	if (tmp_detail_level > 2)	{
		if ( mc->thrust_glow_bitmap != Interp_thrust_glow_bitmap )	{
			// Engline glow bitmap changed
			//	mprintf(( "MC: Glow bitmap changed! %d -> %d\n", mc->thrust_glow_bitmap, Interp_thrust_glow_bitmap ));
			goto RedrawIt;
		}
	}

	if (tmp_detail_level > 2)	{
		if ( cr > 4.0f ) {
			float diff = fl_abs( mc->thrust_scale - Interp_thrust_scale );

			if ( diff > 0.1f )	{
				// Thruster size has changed
				//mprintf(( "MC: Thruster size changed! %.2f -> %.2f\n", mc->thrust_scale, Interp_thrust_scale ));
				goto RedrawIt;
			}
		}
	}

//		if (0) {
//			float diff = fl_abs( mc->thrust_glow_noise - Interp_thrust_glow_noise );

//			if ( diff > 0.1f )	{
				// Glow noise has changed
				//mprintf(( "MC: Thruster glow changed! %.2f -> %.2f\n", mc->thrust_glow_noise, Interp_thrust_glow_noise ));
//				goto RedrawIt;
//			}
//		}


	if ( mc->model_num != model_num )	{
		// Model changed
		goto RedrawIt;
	}

	if ( cr>mc->cr*Mc_size_factor[tmp_detail_level] )	{
		// Scaling up too far
		goto RedrawIt;
	}
		
	if (tmp_detail_level > 2)	{
		if ( cr > 4.0f )	{
			if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
				if (mc->num_lights != num_lights)	{
					// Lighting changed
					goto RedrawIt;
				}
			}
		}
	}

		// This method is correct, but rotating ship makes things redraw which is too slow.
	#if 0
		if ( cr > 4.0f )	{
			// Check orientation
			float angle_error = max( fl_abs( mc->angs.p-new_angles.p ),fl_abs( mc->angs.h-new_angles.h ));

			// Exact
			//if ( angle_error > 0.075f  )	{	

			// Rough
			if ( angle_error > 0.40f  )	{	
				// Ship/view turned too much
				//mprintf(( "Ship/view turned too much %.4f\n", angle_error ));

				goto RedrawIt;
			}
		}
	#endif


//		mprintf(( "Dot = %.5f\n", dot ));

#if 0
	if (0) {
		float dx, dy, dz;

		dx = vm_vec_dot( &orient->rvec, &mc->orient.rvec )+1.0f;
		dy = vm_vec_dot( &orient->uvec, &mc->orient.uvec )+1.0f;
		dz = vm_vec_dot( &orient->fvec, &mc->orient.fvec )+1.0f;
			
		float angle_error = (dx+dy+dz)*1000.0f/6.0f;		

		//mprintf(( "Angle_error = %.4f\n", angle_error ));

		// Compare it to 999.75f at R = 64.0f
		//               0.0000f at R = 0.0f
		
		float cmp_val = 999.75f;		// old
//			if ( is_asteroid )	{
//				cmp_val = scale_it( 0.0f, 999.75f, cr, 0.0f, 64.0f );
//			}
											
		if ( angle_error < cmp_val ) {
			// Ship turned too much
			goto RedrawIt;
		}
	}	
#endif


	// Have a valid cache entry, mc
	ccflags = g3_rotate_vertex(&pt,pos);

	if ( ccflags )	{
		// offscreen		
		goto RedrawIt;
	}

	if ( model_get_rotated_bitmap_points(&pt,mc->angs.b - new_angles.b, pm->rad, v ))	{
		// offscreen		
		goto RedrawIt;
	}


	gr_set_bitmap( mc->bitmap_id );

	Tmap_scan_read = 2;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	//	if ( keyd_pressed[KEY_LSHIFT] )	{
	//	gr_set_color( 255, 0, 0 );
	//	gr_pixel( fl2i(v[0].sx), fl2i(v[0].sy) );
	//	}

	//if ( keyd_pressed[KEY_RSHIFT] )	{
	//	gr_line( fl2i(v[0].sx), fl2i(v[0].sy), fl2i(v[1].sx), fl2i(v[1].sy) );
	//	gr_line( fl2i(v[1].sx), fl2i(v[1].sy), fl2i(v[2].sx), fl2i(v[2].sy) );
	//	gr_line( fl2i(v[2].sx), fl2i(v[2].sy), fl2i(v[3].sx), fl2i(v[3].sy) );
	//	gr_line( fl2i(v[3].sx), fl2i(v[3].sy), fl2i(v[0].sx), fl2i(v[0].sy) );
	//}


	return;


	//==========================================================
	// Cache is bad for model, so draw it and save it
RedrawIt:


//	if ( mc->data != NULL )	{
//		free(mc->data);
//		mc->data = NULL;
//	}

	if ( mc->bitmap_id != -1 )	{
		bm_release(mc->bitmap_id);
		mc->bitmap_id = -1;
	}

	mc->cached_valid = 0;
	mc->model_num = model_num;
	mc->pos = *pos;
	//mc->orient = *orient;
	mc->cr = cr;
	mc->angs = new_angles;	//-Physics_viewer_bank;

	mc->thrust_scale = Interp_thrust_scale;
	mc->thrust_bitmap = Interp_thrust_bitmap;
	mc->thrust_glow_bitmap = Interp_thrust_glow_bitmap;
	mc->thrust_glow_noise = Interp_thrust_glow_noise;

	mc->last_dot = this_dot;

	if ( cr > MODEL_MAX_BITMAP_SIZE/2-1 )	
		goto JustDrawIt;

	//Physics_viewer_bank

	ccflags = g3_rotate_vertex(&pt,pos);

	if ( ccflags ) {
		goto JustDrawIt;
	}

	model_get_rotated_bitmap_points(&pt,0.0f, pm->rad, v );
				
	int x1, y1, x2, y2, w, h;

	x1 = fl_round_2048( v[0].sx );
	y1 = fl_round_2048( v[0].sy );

	x2 = fl_round_2048( v[2].sx );	//+0.5f );
	y2 = fl_round_2048( v[2].sy );	//+0.5f );

	if ( x1 < gr_screen.clip_left)	
		goto JustDrawIt;
	
	if ( y1 < gr_screen.clip_top )
		goto JustDrawIt;
	
	if ( x2 > gr_screen.clip_right)
		goto JustDrawIt;

	if ( y2 > gr_screen.clip_bottom) 
		goto JustDrawIt;

	w = x2 - x1 + 1;	
	if ( w < 0 )
		Int3();

	if ( w < 2 ) 
		w = 2;

	h = y2 - y1 + 1;	

	if ( h < 0 )
		Int3();

	if ( h < 2 ) 
		h = 2;

	if ( w > MODEL_MAX_BITMAP_SIZE )
		goto JustDrawIt;
		
	if ( h > MODEL_MAX_BITMAP_SIZE )
		goto JustDrawIt;

	mc->w = w;
	mc->h = h;

//	mprintf(( "Mallocing a %dx%d bitmap\n", w, h ));

	if ( mc->data == NULL )	{
		mc->data = (ubyte *)malloc( MODEL_MAX_BITMAP_SIZE * MODEL_MAX_BITMAP_SIZE );
	}

//	mprintf(( "Done mallocing a %dx%d bitmap\n", w, h ));

	if ( mc->data == NULL )	{
		goto JustDrawIt;
	}
	for (i = 0; i < w*h; i++)	{
		mc->data[i] = 255;
	}


	mc->bitmap_id = bm_create( 8, mc->w, mc->h, mc->data, 0 );

	if ( mc->bitmap_id < 0 )	{
		goto JustDrawIt;
	}

	// Save stars and stuff on screen
	mc_get_bmp( tmp_bitmap, x1, y1, w, h );

	mc->num_lights = num_lights;

	// Didn't render a cached one... so render it and then save it in the cache

	// Turn on stippling
	model_really_render(model_num, orient, pos, flags, objnum );

	// Save screen to bitmap 
	gr_set_bitmap( mc->bitmap_id );
	Tmap_scan_read = 1;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	// Restore stars and stuff to screen
	mc_put_bmp( tmp_bitmap, x1, y1, w, h );

	// Draw the model
	gr_set_bitmap( mc->bitmap_id );
	Tmap_scan_read = 2;
	g3_draw_poly(4, vertlist, TMAP_FLAG_TEXTURED );	
	Tmap_scan_read = 0;

	mc->cached_valid = 1;
	mc->last_frame_rendered = Framecount;
	return;
	
JustDrawIt:

	// Too big to save
	model_really_render(model_num, orient, pos, flags, objnum );
	*/
}

// Find the distance from p0 to the closest point on a box.
// The box's dimensions from 'min' to 'max'.
float interp_closest_dist_to_box( vector *hitpt, vector *p0, vector *min, vector *max )
{
	float *origin = (float *)&p0->xyz.x;
	float *minB = (float *)min;
	float *maxB = (float *)max;
	float *coord = (float *)&hitpt->xyz.x;
	int inside = 1;
	int i;

	for (i=0; i<3; i++ )	{
		if ( origin[i] < minB[i] )	{
			coord[i] = minB[i];
			inside = 0;
		} else if (origin[i] > maxB[i] )	{
			coord[i] = maxB[i];
			inside = 0;
		} else {
			coord[i] = origin[i];
		}
	}
	
	if ( inside )	{
		return 0.0f;
	}

	return vm_vec_dist(hitpt,p0);
}


// Finds the closest point on a model to a point in space.  Actually only finds a point
// on the bounding box of the model.    
// Given:
//   model_num      Which model
//   orient         Orientation of the model
//   pos            Position of the model
//   eye_pos        Point that you want to find the closest point to
// Returns:
//   distance from eye_pos to closest_point.  0 means eye_pos is 
//   on or inside the bounding box.
//   Also fills in outpnt with the actual closest point.
float model_find_closest_point( vector *outpnt, int model_num, int submodel_num, matrix *orient, vector * pos, vector *eye_pos )
{
	vector closest_pos, tempv, eye_rel_pos;
	
	polymodel *pm = model_get(model_num);

	if ( submodel_num < 0 )	{
		 submodel_num = pm->detail[0];
	}

	// Rotate eye pos into object coordinates
	vm_vec_sub(&tempv,pos,eye_pos );
	vm_vec_rotate(&eye_rel_pos,&tempv,orient );

	return interp_closest_dist_to_box( &closest_pos, &eye_rel_pos, &pm->submodel[submodel_num].min, &pm->submodel[submodel_num].max );
}

int tiling = 1;
DCF(tiling, "")
{
	tiling = !tiling;
	if(tiling){
		dc_printf("Tiled textures\n");
	} else {
		dc_printf("Non-tiled textures\n");
	}
}

#ifndef NO_DIRECT3D
extern void d3d_zbias(int bias);
#endif

void moldel_calc_facing_pts( vector *top, vector *bot, vector *fvec, vector *pos, float w, float z_add, vector *Eyeposition )
{
	vector uvec, rvec;
	vector temp;

	temp = *pos;

	vm_vec_sub( &rvec, Eyeposition, &temp );
	vm_vec_normalize( &rvec );	

	vm_vec_crossprod(&uvec,fvec,&rvec);
	vm_vec_normalize(&uvec);

	vm_vec_scale_add( top, &temp, &uvec, w/2.0f );
	vm_vec_scale_add( bot, &temp, &uvec, -w/2.0f );	

//	Int3();
}


#define mSTUFF_VERTICES()	do { verts[0]->u = 0.0f; verts[0]->v = 0.0f;	verts[1]->u = 1.0f; verts[1]->v = 0.0f; verts[2]->u = 1.0f;	verts[2]->v = 1.0f; verts[3]->u = 0.0f; verts[3]->v = 1.0f; } while(0);
#define mR_VERTICES()		do { g3_rotate_vertex(verts[0], &bottom1); g3_rotate_vertex(verts[1], &bottom2);	g3_rotate_vertex(verts[2], &top2); g3_rotate_vertex(verts[3], &top1); } while(0);
#define mP_VERTICES()		do { for(idx=0; idx<4; idx++){ g3_project_vertex(verts[idx]); } } while(0);

void model_really_render(int model_num, matrix *orient, vector * pos, uint flags, int light_ignore_id )
{
	int i, detail_level;
	polymodel * pm;
	glow_maps_active = 1;

	uint save_gr_zbuffering_mode;
	int zbuf_mode;
	int objnum = light_ignore_id;
	ship *shipp = NULL;
	int is_ship = 0;
	object *obj = &Objects[objnum];
	if(obj->type == OBJ_SHIP){
		shipp = &Ships[obj->instance];
		is_ship = 1;
		glow_maps_active = shipp->glowmaps_active;
	}
	
	if (FULLCLOAK != 1)
		model_interp_sortnorm = model_interp_sortnorm_b2f;


	MONITOR_INC( NumModelsRend, 1 );	

	Interp_orient = orient;
	Interp_pos = pos;
	memset(&Interp_offset,0,sizeof(vector));

	int tmp_detail_level = Game_detail_level;
	
//	if ( D3D_enabled )	{
//		tmp_detail_level = -1;		// Force no hires models for Direct3D
//	}

	//	Tmap_show_layers = 1;
//	model_set_detail_level(0);
//	flags |= MR_LOCK_DETAIL|MR_NO_TEXTURING|MR_NO_LIGHTING;		//MR_LOCK_DETAIL |	|MR_NO_LIGHTING|MR_NO_SMOOTHINGMR_NO_TEXTURING | 

	// Turn off engine effect
	Interp_thrust_scale_subobj=0;

	if (!Model_texturing)
		flags |= MR_NO_TEXTURING;

	if ( !Model_polys )	{
		flags |= MR_NO_POLYS;
	}

	Interp_flags = flags;			 

	pm = model_get(model_num);	

	// Set the flags we will pass to the tmapper
	if ( D3D_enabled || OGL_inited )	{
		Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;
	} else {
		Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RAMP;
	}

	// if we're in nebula mode
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)){
		Interp_tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}


	if ( !(Interp_flags & MR_NO_TEXTURING) )	{
		Interp_tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling)
			Interp_tmap_flags |= TMAP_FLAG_TILED;

		if ( !(Interp_flags & MR_NO_CORRECT) )	{
			Interp_tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

	save_gr_zbuffering_mode = gr_zbuffering_mode;
	zbuf_mode = gr_zbuffering_mode;

	if (!(Game_detail_flags & DETAIL_FLAG_MODELS) )	{
		gr_set_color(0,128,0);
		g3_draw_sphere_ez( pos, pm->rad );
		return;
	}

	g3_start_instance_matrix(pos,orient);	

	if ( Interp_flags & MR_SHOW_RADIUS )	{
		if ( !(Interp_flags & MR_SHOW_OUTLINE_PRESET) )	{
			gr_set_color(0,64,0);
			g3_draw_sphere_ez(&vmd_zero_vector,pm->rad);
		}
	}

	Assert( pm->n_detail_levels < MAX_MODEL_DETAIL_LEVELS );

	vector closest_pos;
	float depth = model_find_closest_point( &closest_pos, model_num, -1, orient, pos, &Eye_position );
	if ( pm->n_detail_levels > 1 )	{

		if ( Interp_flags & MR_LOCK_DETAIL )	{
			i = Interp_detail_level+1;
		} else {

			//gr_set_color(0,128,0);
			//g3_draw_sphere_ez( &closest_pos, 2.0f );

			#if MAX_DETAIL_LEVEL != 4
			#error Code in modelInterp.cpp assumes MAX_DETAIL_LEVEL == 4
			#endif

			switch( Detail.detail_distance )	{
			case 0:		// lowest
				depth *= 8.0f;
				break;
			case 1:		// lower than normal
				depth *= 4.0f; 
				break;
			case 2:		// default  (leave the same)
				break;
			case 3:		// above normal
				depth /= 4.0f;
				break;
			case 4:		// even more normal
				depth /= 8.0f;
				break;
			}

			// nebula ?
			if(The_mission.flags & MISSION_FLAG_FULLNEB){
				depth *= neb2_get_lod_scale(Interp_objnum);
			}

			for ( i=0; i<pm->n_detail_levels; i++ )	{
				if ( depth<=pm->detail_depth[i] ){
					break;
				}
			}

			// If no valid detail depths specified, use highest.
			if ( (i > 1) && (pm->detail_depth[i-1] < 1.0f))	{
				i = 1;
			}
		}


		// maybe force lower detail
		if (Interp_flags & MR_FORCE_LOWER_DETAIL) {
			i++;
		}

		//detail_level = fl2i(depth/10.0f);		
		//detail_level = 0;
		detail_level = i-1-tmp_detail_level;

		if ( detail_level < 0 ) 
			detail_level = 0;
		else if (detail_level >= pm->n_detail_levels ) 
			detail_level = pm->n_detail_levels-1;

		//mprintf(( "Depth = %.2f, detail = %d\n", depth, detail_level ));

	} else {
		detail_level = 0;
	}

#ifndef NDEBUG
	if ( detail_level==0 )	{
		MONITOR_INC( NumHiModelsRend, 1 );	
	} else if ( detail_level ==pm->n_detail_levels-1 )	{
		MONITOR_INC( NumLowModelsRend, 1 );	
	}  else {
		MONITOR_INC( NumMedModelsRend, 1 );	
	}
#endif	

	if((Interp_flags & MR_AUTOCENTER) && (pm->flags & PM_FLAG_AUTOCEN)){
		vector auto_back = pm->autocenter;				
		vm_vec_scale(&auto_back, -1.0f);		
		g3_start_instance_matrix(&auto_back, NULL);		
	}	

#ifndef NO_DIRECT3D
	if(gr_screen.mode == GR_DIRECT3D){
		d3d_zbias(1);
	}
#endif

	// Draw the subobjects	
	i = pm->submodel[pm->detail[detail_level]].first_child;

	while( i>-1 )	{
		if (!pm->submodel[i].is_thruster )	{
			zbuf_mode = GR_ZBUFF_WRITE;

			// no zbuffering
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			}

			gr_zbuffer_set(zbuf_mode);

			model_interp_subcall( pm, i, detail_level );
		} 
		i = pm->submodel[i].next_sibling;
	}	

	// rotate lights for the hull
	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}	

	if ( pm->submodel[pm->detail[detail_level]].num_children > 0 ){
		// zbuf_mode |= GR_ZBUFF_WRITE;		// write only
		zbuf_mode = GR_ZBUFF_FULL;
	}

	// no zbuffering
	if(Interp_flags & MR_NO_ZBUFFER){
		zbuf_mode = GR_ZBUFF_NONE;
	}
	
	gr_zbuffer_set(zbuf_mode);

#ifndef NO_DIRECT3D
	if(gr_screen.mode == GR_DIRECT3D){
		d3d_zbias(0);	
	}
#endif

		
	// draw the hull of the ship
	model_interp_sub( (ubyte *)pm->submodel[pm->detail[detail_level]].bsp_data, pm, &pm->submodel[pm->detail[detail_level]], 0 );

	if (Interp_flags & MR_SHOW_PIVOTS )	{
		model_draw_debug_points( pm, NULL );
		model_draw_debug_points( pm, &pm->submodel[pm->detail[detail_level]] );

		if(pm->flags & PM_FLAG_AUTOCEN){
			gr_set_color(255, 255, 255);
			g3_draw_sphere_ez(&pm->autocenter, pm->rad / 4.5f);
		}
	}

	if (FULLCLOAK != -1)	model_finish_cloak(FULLCLOAK);

	if ( pm->submodel[pm->detail[0]].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[pm->detail[0]]);
	}	

	if ( Interp_flags & MR_SHOW_SHIELDS )	{
		model_render_shields(pm);
	}	
			
#ifndef NO_DIRECT3D
	// render model insignias
	if(gr_screen.mode == GR_DIRECT3D){
		d3d_zbias(1);
	}
#endif
	gr_zbuffer_set(GR_ZBUFF_READ);
	model_render_insignias(pm, detail_level);	

#ifndef NO_DIRECT3D
	// zbias back to 0	
	if(gr_screen.mode == GR_DIRECT3D){
		d3d_zbias(0);	
	}	
#endif
//	object *obj = &Objects[objnum];
	decal_render_all(&Objects[objnum]);
//		decal_render_all(&Objects[objnum]);//render decals-Bobboau
//		mprintf(("out of the decal stuff\n"));
/*	if(obj->type == OBJ_SHIP){

		vertex vecs[3];
		vertex *vlist[3] = { &vecs[0], &vecs[1], &vecs[2] };
	
			mprintf(("about to render all decals\n"));

	//	ship *shipp = &Ships[obj->instance];

		for(int h = 0; h < MAX_SHIP_DECALS; h++){
			decal *dec = &shipp->decals[h];
			if(dec->is_valid){
				mprintf(("decal %d is valid, and has %d polys\n",h,dec->n_poly));
				for(int i = 0; i<dec->n_poly; i++){

	mprintf(("drawing decal poly %d, with bitmap %d\n", i, dec->texture));
						gr_set_bitmap(dec->texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );
	
						vecs[0] = dec->vert[i][0];
						vecs[1] = dec->vert[i][1];
						vecs[2] = dec->vert[i][2];
						g3_rotate_vertex(&vecs[0], &obj->pos);
						g3_rotate_vertex(&vecs[1], &obj->pos);
						g3_rotate_vertex(&vecs[2], &obj->pos);
						g3_project_vertex(&vecs[0]);
						g3_project_vertex(&vecs[1]);
						g3_project_vertex(&vecs[2]);
	mprintf(("vert 1 at x %0.2f y %0.2f z %0.2f u %0.2f v %0.2f\n", vecs[0].x, vecs[0].y, vecs[0].z, vecs[0].u, vecs[0].v));
	mprintf(("vert 2 at x %0.2f y %0.2f z %0.2f u %0.2f v %0.2f\n", vecs[1].x, vecs[1].y, vecs[1].z, vecs[1].u, vecs[1].v));
	mprintf(("vert 3 at x %0.2f y %0.2f z %0.2f u %0.2f v %0.2f\n", vecs[2].x, vecs[2].y, vecs[2].z, vecs[2].u, vecs[2].v));
						gr_set_cull(0);
						g3_draw_poly(3, vlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT );
						gr_set_cull(1);
				}
			}
		}
		mprintf(("decals rendered\n"));
	}
*/

//start rendering glow points -Bobboau

		if ( (pm->n_glows) /*&& (Interp_flags & MR_SHOW_THRUSTERS) /*&& (Detail.engine_glows)*/ )	{

		for (i = 0; i < pm->n_glows; i++ ) {
			glow_bank *bank = &pm->glows[i];
			int j;
			if(bank->is_on)
			{
				if(is_ship && i<32)
					if(!(shipp->glows_active & (1 << i)))
						continue;

				for ( j=0; j<bank->num_slots; j++ )	{
					
					int flick;
						if(pm->submodel[pm->detail[0]].num_arcs){
						flick = static_rand(timestamp()%20)%(pm->submodel[pm->detail[0]].num_arcs + j); //the more damage, the more arcs, the more likely the lights will fail
						}else{
						flick = 1;
						}
 
					if(flick == 1){
						vector pnt = bank->pnt[j];
						vector norm = bank->norm[j];
					
						if(bank->submodel_parent > 0){//this is were it rotates for the submodel parent-Bobboau
							matrix m;
							angles angs = pm->submodel[bank->submodel_parent].angs;
							angs.b = PI2 - angs.b;
							angs.p = PI2 - angs.p;
							angs.h = PI2 - angs.h;

//								gr_set_color_fast(&Color_bright_blue);
//								gr_printf(0, 10, "b %0.2f, p %0.2f, h %0.2f", angs.b, angs.p, angs.h);

							vm_angles_2_matrix(&m, &angs);

							vector offset = pm->submodel[bank->submodel_parent].offset;
							vm_vec_sub(&pnt, &pnt, &offset);
							vector p = pnt;
							vector n = norm;
							vm_vec_rotate(&pnt, &p, &m);
							vm_vec_rotate(&norm, &n, &m);
							vm_vec_add2(&pnt, &offset);
						}
							vector tempv;
						switch(bank->type){
						case 0:
							float d;
							if((bank->norm[j].xyz.x == 0.0f) && (bank->norm[j].xyz.z == 0.0f) && (bank->norm[j].xyz.z == 0.0f)){
								d=1.0f;	//if given a nul vector then always show it
							}else{
								vm_vec_sub(&tempv,&View_position,&pnt);
								vm_vec_normalize(&tempv);

								d = vm_vec_dot(&tempv,&norm);
								d -= 0.25;	
							}
						
							if ( d > 0.0f)	{
								vertex p;					
		
								d *= 3.0f;
								if ( d > 1.0f ) d = 1.0f;
	
								float w = bank->radius[j];
		
								// fade them in the nebula as well
								if(The_mission.flags & MISSION_FLAG_FULLNEB){
									d *= (1.0f - Interp_fog_level);
									w *= 1.5;	//make it bigger in a nebula
								}
					
								// disable fogging
								gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
		
								g3_rotate_vertex( &p, &pnt );
								
	
							//	if(bank->submodel_parent)
//								{
//								}

								gr_set_bitmap( bank->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, d );
							//	mprintf(( "rendering glow with texture %d\n", bank->glow_bitmap ));
								Gr_scaler_zbuffering = 1;
								g3_draw_bitmap(&p,0,w*0.5f, TMAP_FLAG_TEXTURED );
								//g3_draw_rotated_bitmap(&p,0.0f,w,w, TMAP_FLAG_TEXTURED );
								Gr_scaler_zbuffering = 0;

							}//d>0
							break;
						case 1:
								vertex h1[4];				// halves of a beam section	
								vertex *verts[4] = { &h1[0], &h1[1], &h1[2], &h1[3] };	
								vector fvec, top1, bottom1, top2, bottom2, start, end;

								vm_vec_add2(&norm, &pnt);

								vm_vec_rotate(&start, &pnt, orient);
								vm_vec_rotate(&end, &norm, orient);
								vm_vec_sub(&fvec, &end, &start);

								vm_vec_normalize(&fvec);

								moldel_calc_facing_pts(&top1, &bottom1, &fvec, &pnt, 1.0f, 1.0f, &Eye_position);	
								moldel_calc_facing_pts(&top2, &bottom2, &fvec, &norm, 1.0f, 1.0f, &Eye_position);	

								int idx = 0;

/*R_VERTICES()*/		do { 
							g3_rotate_vertex(verts[0], &bottom1); 
							g3_rotate_vertex(verts[1], &bottom2);	
							g3_rotate_vertex(verts[2], &top2); 
							g3_rotate_vertex(verts[3], &top1); } while(0);
/*P_VERTICES()*/		do { for(idx=0; idx<4; idx++){ g3_project_vertex(verts[idx]); } } while(0);
/*STUFF_VERTICES()*/	do { 
								verts[0]->u = 0.0f; verts[0]->v = 0.0f;	
								verts[1]->u = 1.0f; verts[1]->v = 0.0f; 
								verts[2]->u = 1.0f;	verts[2]->v = 1.0f; 
								verts[3]->u = 0.0f; verts[3]->v = 1.0f; 
						} while(0);
//								mR_VERTICES();																// rotate and project the vertices
//								mP_VERTICES();						
//								mSTUFF_VERTICES();		// stuff the beam with creamy goodness (texture coords)

								vm_vec_sub(&tempv,&View_position,&pnt);
								vm_vec_normalize(&tempv);
								

							gr_set_cull(0);
							if(The_mission.flags & MISSION_FLAG_FULLNEB){
								gr_set_bitmap(bank->glow_neb_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
							}else{
								gr_set_bitmap(bank->glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f);		
							}
								g3_draw_poly( 4, verts, TMAP_FLAG_TILED | TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT); // added TMAP_FLAG_TILED flag for beam texture tileing -Bobboau
							gr_set_cull(1);
						}
					}//flick
				}//for slot
			}//bank is on
		}//for bank
	}//any glow banks
				
//	gr_printf((200), (20), "x %0.2f, y %0.2f, z %0.2f", Eye_position.xyz.x, Eye_position.xyz.y, Eye_position.xyz.z);

//end rendering glow points

	// Draw the thruster glow
	if ( (Interp_thrust_glow_bitmap != -1) && (Interp_flags & MR_SHOW_THRUSTERS) /*&& (Detail.engine_glows)*/ )	{

		for (i = 0; i < pm->n_thrusters; i++ ) {
			thruster_bank *bank = &pm->thrusters[i];
			int j;

			for ( j=0; j<bank->num_slots; j++ )	{
				float d;
				vector tempv;
				vm_vec_sub(&tempv,&View_position,&bank->pnt[j]);
				vm_vec_normalize(&tempv);

				d = vm_vec_dot(&tempv,&bank->norm[j]);

				if ( d > 0.0f)	{
					vertex p;					

					// Make glow bitmap fade in/out quicker from sides.
					d *= 3.0f;
					if ( d > 1.0f ) d = 1.0f;

					// fade them in the nebula as well
					if(The_mission.flags & MISSION_FLAG_FULLNEB){
						d *= (1.0f - Interp_fog_level);
					}

					//ADAM: Min throttle draws rad*MIN_SCALE, max uses max.
					#define NOISE_SCALE 0.5f
					#define MIN_SCALE 3.4f
					#define MAX_SCALE 4.7f
					float scale = MIN_SCALE;
						
// the following replaces Bobboau's code, commented out below - Goober5000
					float magnitude;
					vector scale_vec;

					// normalize banks, in case of incredibly big normals
					vm_vec_copy_normalize(&scale_vec, &bank->norm[j]);

					// adjust for thrust
					(scale_vec.xyz.x *= Interp_thrust_scale_x) -= 0.1f;
					(scale_vec.xyz.y *= Interp_thrust_scale_y) -= 0.1f;
					(scale_vec.xyz.z *= Interp_thrust_scale) -= 0.1f;

					// get magnitude, which we will use as the scaling reference
					magnitude = vm_vec_normalize(&scale_vec);

					// get absolute value
					if (magnitude < 0.0f)
						magnitude *= -1.0f;

					scale = magnitude*(MAX_SCALE-MIN_SCALE)+MIN_SCALE;

/* this is Bobboau's code
				//	if(abs(bank->norm->y + Interp_thrust_scale_y) > 0){
				//	if(abs(bank->norm->x + Interp_thrust_scale_x) > 0)
				//	if(abs(bank->norm->z + Interp_thrust_scale) > 0)

					x = ((-bank->norm[j].xyz.x + Interp_thrust_scale_x) * Interp_thrust_scale_x);
//					x = (x * Interp_thrust_scale_x)/2;
					if((bank->norm[j].xyz.x < 0.0f) && (Interp_thrust_scale_x < 0.0f)){
						x = -x;
					}
					y = ((-bank->norm[j].xyz.y + Interp_thrust_scale_y) * Interp_thrust_scale_y);
//					y = (y * Interp_thrust_scale_y)/2;
					if((bank->norm[j].xyz.y < 0.0f) && (Interp_thrust_scale_y < 0.0f)){
						y = -y;
					}

					z = ((-bank->norm[j].xyz.z + Interp_thrust_scale) * Interp_thrust_scale);
//					z = (z * Interp_thrust_scale)/2;
					if((bank->norm[j].xyz.z < 0.0f) && (Interp_thrust_scale < 0.0f)){
						z = -z;
					}


#ifndef NDEBUG
//	if ( &Objects[objnum] == Player_obj ){
		gr_set_color_fast(&Color_bright_blue);
		gr_printf((i * 200), (20 +(j * 10)), "x %0.2f, y %0.2f, z %0.2f", x, y, z);
//	}
#endif

					scale = ((bank->norm[j].xyz.x * x)+(bank->norm[j].xyz.y * y)+(bank->norm[j].xyz.z * z))*(MAX_SCALE-MIN_SCALE)+MIN_SCALE;
					//getting thruster glows to grow baised on witch direction they are pointing -Bobboau
*/

// this is the original scaling code - Goober5000
//					scale = (Interp_thrust_scale-0.1f)*(MAX_SCALE-MIN_SCALE)+MIN_SCALE;

					float w = bank->radius[j]*(scale+Interp_thrust_glow_noise*NOISE_SCALE );

					// disable fogging
					gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);

					g3_rotate_vertex( &p, &bank->pnt[j] );
					gr_set_bitmap( Interp_thrust_glow_bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, d );
					{
						Gr_scaler_zbuffering = 1;
						g3_draw_bitmap(&p,0,w*0.5f, TMAP_FLAG_TEXTURED );
						//g3_draw_rotated_bitmap(&p,0.0f,w,w, TMAP_FLAG_TEXTURED );
						Gr_scaler_zbuffering = 0;
					}
				}

			}
		}
	}

	Interp_thrust_glow_bitmap = -1;	

	gr_set_cull(0);	

	// Draw the thruster subobjects	
	i = pm->submodel[pm->detail[detail_level]].first_child;
	while( i>-1 )	{
		if (pm->submodel[i].is_thruster )	{
			zbuf_mode = GR_ZBUFF_READ;

			// no zbuffering
			if(Interp_flags & MR_NO_ZBUFFER){
				zbuf_mode = GR_ZBUFF_NONE;
			}

			gr_zbuffer_set(zbuf_mode);

			model_interp_subcall( pm, i, detail_level );
		}
		i = pm->submodel[i].next_sibling;
	}	

	gr_set_cull(1);	

	if ( Interp_flags & MR_SHOW_PATHS ){
		model_draw_paths(model_num);
	}

	if (Interp_flags & MR_BAY_PATHS ){
		model_draw_bay_paths(model_num);
	}

	if((Interp_flags & MR_AUTOCENTER) && (pm->flags & PM_FLAG_AUTOCEN)){
		g3_done_instance();
	}

	g3_done_instance();
	gr_zbuffer_set(save_gr_zbuffering_mode);
	
	glow_maps_active = 1;
	memset(&Interp_offset,0,sizeof(vector));

}


void submodel_render(int model_num, int submodel_num, matrix *orient, vector * pos, uint flags, int light_ignore_id, int *replacement_textures)
{
	// replacement textures - Goober5000
	model_set_replacement_textures(replacement_textures);

	polymodel * pm;

	MONITOR_INC( NumModelsRend, 1 );	

	if (!(Game_detail_flags & DETAIL_FLAG_MODELS) )	return;

	// Turn off engine effect
	Interp_thrust_scale_subobj=0;

	if (!Model_texturing)
		flags |= MR_NO_TEXTURING;

	Interp_flags = flags;
	Interp_pos=pos;
	
	pm = model_get(model_num);

	// Set the flags we will pass to the tmapper
	if ( D3D_enabled || OGL_inited )	{
		Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB;
	} else {
		Interp_tmap_flags = TMAP_FLAG_GOURAUD | TMAP_FLAG_RAMP;
	}

	// if we're in nebula mode
	if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE)){
		Interp_tmap_flags |= TMAP_FLAG_PIXEL_FOG;
	}

	if ( !(Interp_flags & MR_NO_TEXTURING) )	{
		Interp_tmap_flags |= TMAP_FLAG_TEXTURED;

		if ( (pm->flags & PM_FLAG_ALLOW_TILING) && tiling )
			Interp_tmap_flags |= TMAP_FLAG_TILED;

		if ( !(Interp_flags & MR_NO_CORRECT) )	{
			Interp_tmap_flags |= TMAP_FLAG_CORRECT;
		}
	}

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		if ( D3D_enabled || OGL_inited )	{
			light_filter_push( -1, pos, pm->submodel[submodel_num].rad );
		} else {
			light_filter_push( light_ignore_id, pos, pm->submodel[submodel_num].rad );
		}
	}

	g3_start_instance_matrix(pos,orient);

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_rotate_all();
	}

	model_interp_sub( pm->submodel[submodel_num].bsp_data, pm, &pm->submodel[submodel_num], 0 );
	if ( pm->submodel[submodel_num].num_arcs )	{
		interp_render_lightning( pm, &pm->submodel[submodel_num]);
	}

	if (Interp_flags & MR_SHOW_PIVOTS )
		model_draw_debug_points( pm, &pm->submodel[submodel_num] );

	if ( !(Interp_flags & MR_NO_LIGHTING ) )	{
		light_filter_pop();	
	}
	g3_done_instance();


	// turn off fog after each model renders, RT This fixes HUD being fogged when debris is in target box
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

}



// Fills in an array with points from a model.
// Only gets up to max_num verts;
// Returns number of verts found;
static int submodel_get_points_internal(int model_num, int submodel_num, int max_num, vector **pnts, vector **norms )
{
	polymodel * pm;

	pm = model_get(model_num);

	if ( submodel_num < 0 )	{
		submodel_num = pm->detail[0];
	}

	ubyte *p = pm->submodel[submodel_num].bsp_data;
	int chunk_type, chunk_size;

	chunk_type = w(p);
	chunk_size = w(p+4);

	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_EOF: return 1;
		case OP_DEFPOINTS:	{
				int n;
				int nverts = w(p+8);				
				int offset = w(p+16);				

				ubyte * normcount = p+20;
				vector *src = vp(p+offset);

				if ( nverts > max_num )
					nverts = max_num; 

				for (n=0; n<nverts; n++ )	{
					*pnts++ = src;
					*norms++ = src + 1;		// first normal associated with the point

					src += normcount[n]+1;
				} 
				return nverts;		// Read in 'n' points
			}
			break;
		case OP_FLATPOLY:		break;
		case OP_TMAPPOLY:		break;
		case OP_SORTNORM:		break;
		case OP_BOUNDBOX:		break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_points\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}
	return 0;		// Couldn't find 'em
}

// Gets two random points on a model
void submodel_get_two_random_points(int model_num, int submodel_num, vector *v1, vector *v2, vector *n1, vector *n2 )
{
	int nv = submodel_get_points_internal(model_num, submodel_num, MAX_POLYGON_VECS, Interp_verts, Interp_norms );
	
	int vn1 = (myrand()>>5) % nv;
	int vn2 = (myrand()>>5) % nv;

	*v1 = *Interp_verts[vn1];
	*v2 = *Interp_verts[vn2];

	if(n1 != NULL){
		*n1 = *Interp_norms[vn1];
	}
	if(n2 != NULL){
		*n2 = *Interp_norms[vn2];
	}
}

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color(int r, int g, int b )
{
	gr_init_color( &Interp_outline_color, r, g, b );

}

// If MR_FLAG_OUTLINE bit set this color will be used for outlines.
// This defaults to black.
void model_set_outline_color_fast(void *outline_color)
{
	Interp_outline_color = *((color*)(outline_color));
}

// IF MR_LOCK_DETAIL is set, then it will always draw detail level 'n'
// This defaults to 0. (0=highest, larger=lower)
void model_set_detail_level(int n)
{
	Interp_detail_level = n;
}


// Returns number of verts in a submodel;
int submodel_get_num_verts(int model_num, int submodel_num )
{
	polymodel * pm;

	pm = model_get(model_num);

	ubyte *p = pm->submodel[submodel_num].bsp_data;
	int chunk_type, chunk_size;

	chunk_type = w(p);
	chunk_size = w(p+4);

	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_EOF: return 0;
		case OP_DEFPOINTS:	{
				int n=w(p+8);
				return n;		// Read in 'n' points
			}
			break;
		case OP_FLATPOLY:		break;
		case OP_TMAPPOLY:		break;
		case OP_SORTNORM:		break;
		case OP_BOUNDBOX:	break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_num_verts\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}
	return 0;		// Couldn't find 'em
}

// Returns number of tmaps & flat polys in a submodel;
int submodel_get_num_polys_sub( ubyte *p )
{
	int chunk_type = w(p);
	int chunk_size = w(p+4);
	int n = 0;
	
	while (chunk_type != OP_EOF)	{
		switch (chunk_type) {
		case OP_EOF:			return n;
		case OP_DEFPOINTS:	break;
		case OP_FLATPOLY:		n++; break;
		case OP_TMAPPOLY:		n++; break;
		case OP_SORTNORM:		{
			int frontlist = w(p+36);
			int backlist = w(p+40);
			int prelist = w(p+44);
			int postlist = w(p+48);
			int onlist = w(p+52);
			n += submodel_get_num_polys_sub(p+frontlist);
			n += submodel_get_num_polys_sub(p+backlist);
			n += submodel_get_num_polys_sub(p+prelist);
			n += submodel_get_num_polys_sub(p+postlist );
			n += submodel_get_num_polys_sub(p+onlist );
			}
			break;
		case OP_BOUNDBOX:	break;
		default:
			mprintf(( "Bad chunk type %d, len=%d in submodel_get_num_polys\n", chunk_type, chunk_size ));
			Int3();		// Bad chunk type!
			return 0;
		}
		p += chunk_size;
		chunk_type = w(p);
		chunk_size = w(p+4);
	}
	return n;		
}

// Returns number of tmaps & flat polys in a submodel;
int submodel_get_num_polys(int model_num, int submodel_num )
{
	polymodel * pm;

	pm = model_get(model_num);

	return submodel_get_num_polys_sub( pm->submodel[submodel_num].bsp_data );

}

// Sets the submodel instance data in a submodel
// If show_damaged is true it shows only damaged submodels.
// If it is false it shows only undamaged submodels.
void model_show_damaged(int model_num, int show_damaged )
{
	polymodel * pm;
	int i;

	pm = model_get(model_num);

	for (i=0; i<pm->n_models; i++ )	{
		bsp_info *sm = &pm->submodel[i];

		// Set the "blown out" flags	
		sm->blown_off = 0;
	}

	for (i=0; i<pm->n_models; i++ )	{
		bsp_info *sm = &pm->submodel[i];

		// Set the "blown out" flags	
		if ( show_damaged )	{
			if ( sm->my_replacement > -1 )	{
				pm->submodel[sm->my_replacement].blown_off = 0;
				sm->blown_off = 1;
			}
		} else {
			if ( sm->my_replacement > -1 )	{
				pm->submodel[sm->my_replacement].blown_off = 1;
				sm->blown_off = 0;
			}
		}
	}
}

// set the insignia bitmap to be used when rendering a ship with an insignia (-1 switches it off altogether)
void model_set_insignia_bitmap(int bmap)
{
	Interp_insignia_bitmap = bmap;
}

void model_set_replacement_bitmap(int bmap)
{
	Interp_replacement_bitmap = bmap;
}

void model_set_replacement_textures(int *replacement_textures)
{
	Interp_replacement_textures = replacement_textures;	//replacement_textures;
}

// set the forces bitmap
void model_set_forced_texture(int bmap)
{
	Interp_forced_bitmap = bmap;
}

// set model transparency for use with MR_ALL_XPARENT
void model_set_alpha(float alpha)
{
	Interp_xparent_alpha = alpha;
}

// see if the given texture is used by the passed model. 0 if not used, 1 if used, -1 on error
int model_find_texture(int model_num, int bitmap)
{
	polymodel * pm;	
	int idx;

	// get a handle to the model
	pm = model_get(model_num);
	if(pm == NULL){
		return -1;
	}

	// find the texture
	for(idx=0; idx<pm->n_textures; idx++){
		if(pm->textures[idx] == bitmap){
			return 1;
		}
	}

	// no texture
	return 0;
}

// find closest point on extended bounding box (the bounding box plus all the planes that make it up)
// returns closest distance to extended box
// positive return value means start_point is outside extended box
// displaces closest point an optional amount delta to the outside of the box
// closest_box_point can be NULL.
float get_model_closest_box_point_with_delta(vector *closest_box_point, vector *start_point, int modelnum, int *is_inside, float delta)
{
	int i, idx;
	vector box_point, ray_direction, *extremes;
	float dist, best_dist;
	polymodel *pm;
	int inside = 0;
	int masks[6] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
	int mask_inside = 0x3f;

	best_dist = FLT_MAX;
	pm = model_get(modelnum);

	for (i=0; i<6; i++) {
		idx = i / 2;	// which row vector of Identity matrix

		memcpy(&ray_direction, vmd_identity_matrix.a2d[idx], sizeof(vector));

		// do negative, then positive plane for each axis
		if (2 * idx == i) {
			extremes = &pm->mins;
			vm_vec_negate(&ray_direction);
		} else {
			extremes = &pm->maxs;
		}

		// a negative distance means started outside the box
		dist = fvi_ray_plane(&box_point, extremes, &ray_direction, start_point, &ray_direction, 0.0f);
		if (dist > 0) {
			inside |= masks[i];
		}
		if (fabs(dist) < fabs(best_dist)) {
			best_dist = dist;
			if (closest_box_point) {
				vm_vec_scale_add(closest_box_point, &box_point, &ray_direction, delta);
			}
		}
	}

	// is start_point inside the box
	if (is_inside) {
		*is_inside = (inside == mask_inside);
	}

	return -best_dist;
}

// find closest point on extended bounding box (the bounding box plus all the planes that make it up)
// returns closest distance to extended box
// positive return value means start_point is outside extended box
// displaces closest point an optional amount delta to the outside of the box
// closest_box_point can be NULL.
float get_world_closest_box_point_with_delta(vector *closest_box_point, object *box_obj, vector *start_point, int *is_inside, float delta)
{
	vector temp, box_start;
	float dist;
	int modelnum;

	// get modelnum
	modelnum = Ships[box_obj->instance].modelnum;

	// rotate start_point to box_obj RF
	vm_vec_sub(&temp, start_point, &box_obj->pos);
	vm_vec_rotate(&box_start, &temp, &box_obj->orient);

	dist = get_model_closest_box_point_with_delta(closest_box_point, &box_start, modelnum, is_inside, delta);

	// rotate closest_box_point to world RF
	if (closest_box_point) {
		vm_vec_unrotate(&temp, closest_box_point, &box_obj->orient);
		vm_vec_add(closest_box_point, &temp, &box_obj->pos);
	}

	return dist;
}

void model_set_fog_level(float l)
{
	Interp_fog_level = l;
}

// given a newly loaded model, page in all textures
void model_page_in_textures(int modelnum, int ship_info_index)
{
	int idx;
	ship_info *sip;

	// valid ship type?
	if((ship_info_index < 0) || (ship_info_index >= Num_ship_types)){
		return;
	}
	sip = &Ship_info[ship_info_index];

	polymodel *pm = model_get(modelnum);

	// bogus
	if(pm == NULL){
		return;
	}

	// set nondarkening pixels	
	if(sip->num_nondark_colors){		
		palman_set_nondarkening(sip->nondark_colors, sip->num_nondark_colors);
	}
	// use the colors from the default table
	else {		
		palman_set_nondarkening(Palman_non_darkening_default, Palman_num_nondarkening_default);
	}
								
	for (idx=0; idx<pm->n_textures; idx++ ){
		int bitmap_num = pm->original_textures[idx];

		if ( bitmap_num > -1 )	{
			// if we're in Glide (and maybe later with D3D), use nondarkening textures
			if(gr_screen.mode == GR_GLIDE){
				bm_lock(bitmap_num, 16, BMP_TEX_NONDARK);
				bm_unlock(bitmap_num);
			} else {
				bm_lock(bitmap_num, 16, BMP_TEX_OTHER);
				bm_unlock(bitmap_num);
			}
		}
	}
}

// is the given model a pirate ship?
int model_is_pirate_ship(int modelnum)
{
	return 0;
}
/*
void render_all_decal(object * obj){
	vertex vecs[3];
	vertex *vlist[3] = { &vecs[0], &vecs[1], &vecs[2] };
	
		mprintf(("about to render all decals\n"));

	ship *shipp = &Ships[obj->instance];

	for(int h = 0; h < MAX_SHIP_DECALS; h++){
		decal *dec = &shipp->decals[h];
		if(dec->is_valid){
			mprintf(("decal %d is valid, and has %d polys\n",h,dec->n_poly));
			for(int i = 0; i<dec->n_poly; i++){

mprintf(("drawing decal poly %d, with bitmap %d\n", i, dec->texture));
					gr_set_bitmap(dec->texture, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f );

					for( int j = 0; j < 3; j++){
						vecs[j] = dec->vert[i][j];
						vector pos;
						vm_vert2vec(&vecs[j], &pos);
						vm_vec_add2(&pos, &obj->pos);
						g3_rotate_vertex(&vecs[j], &pos);
						g3_project_vertex(&vecs[j]);
mprintf(("vert %d at x %0.2f y %0.2f z %0.2f u %0.2f v %0.2f\n", j, vecs[j].x, vecs[j].y, vecs[j].z, vecs[j].u, vecs[j].v));

					}

					gr_set_cull(0);
					g3_draw_poly(3, vlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT );
					gr_set_cull(1);
			}
		}
	}
	mprintf(("decals rendered\n"));

}
*/
