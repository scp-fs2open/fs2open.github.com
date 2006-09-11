/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Nebula/Neb.cpp $
 * $Revision: 2.52 $
 * $Date: 2006-09-11 06:50:42 $
 * $Author: taylor $
 *
 * Nebula effect
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.51  2006/06/07 05:19:49  wmcoolmon
 * Move fog disappearance factor to objecttypes.tbl
 *
 * Revision 2.50  2006/04/12 01:03:00  taylor
 * more s/colour/color/ changes
 * throw in a couple of safety checks for some neb2 functions
 *
 * Revision 2.49  2006/03/18 10:25:45  taylor
 * some cleanup to the nebula debug console help messages
 *
 * Revision 2.48  2006/01/30 19:37:33  taylor
 * and after all of my recent work fixing div-by-0 zero bugs, ya had to know this was coming :)
 *
 * Revision 2.47  2006/01/30 06:36:01  taylor
 * minor warning fixage
 *
 * Revision 2.46  2006/01/26 03:59:59  taylor
 * first version of this sucked, this should be a bit better while working with both new and retail data
 *   - get an average color from the palette rather than picking first (unoptimized palettes could have unused pixels in first entry)
 *   - slight cleanup, and s/colour/color/ (yeah, stupid American, deal with it! :))
 *
 * Revision 2.45  2006/01/18 16:01:14  taylor
 * support only PCX for the nebula background graphics (given how it works that's the fastest and easiest way to get it done)
 *
 * Revision 2.44  2005/12/29 08:08:39  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.43  2005/12/08 15:17:34  taylor
 * fix several bad crash related problems from WMC's commits on the 4th
 *
 * Revision 2.42  2005/12/04 19:07:48  wmcoolmon
 * Final commit of codebase
 *
 * Revision 2.41  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 2.40  2005/10/30 06:44:57  wmcoolmon
 * Codebase commit - nebula.tbl, scripting, new dinky explosion/shockwave stuff, moving muzzle flashes
 *
 * Revision 2.39  2005/10/09 08:03:20  wmcoolmon
 * New SEXP stuff
 *
 * Revision 2.38  2005/06/21 00:20:24  taylor
 * in the model _render functions change "light_ignore_id" to "objnum" since that's what it really is
 *   and this makes it so much easier to realize that
 * properly deal with the fact that objnum can be -1 in  model_really_render()
 * add NULL check to neb2_get_fog_values() so that it can just send back defaults if objp is NULL
 * small compiler warning fix for neb code
 *
 * Revision 2.37  2005/05/12 17:49:15  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.36  2005/04/25 00:27:32  wmcoolmon
 * Commented out unneeded array (Glide)
 *
 * Revision 2.35  2005/04/05 05:53:20  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.34  2005/03/03 06:05:30  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.33  2005/02/23 04:55:08  taylor
 * more bm_unload() -> bm_release() changes
 *
 * Revision 2.32  2005/02/04 20:06:04  taylor
 * merge with Linux/OSX tree - p0204-2
 *
 * Revision 2.31  2005/01/22 22:53:07  wmcoolmon
 * Instead of trying to allocate a ridiculously large buffer if the file is defined but doesn't exist, give a warning. Someone with nebula-code experience should probably double-check this; also added jpg and TGA support.
 *
 * Revision 2.30  2005/01/10 06:58:12  wmcoolmon
 * Bug-guard
 *
 * Revision 2.29  2004/10/31 21:56:09  taylor
 * new bmpman support, new OGL fixes don't need the freaky neb lighting fix anymore
 *
 * Revision 2.28  2004/07/26 20:47:41  Kazan
 * remove MCD complete
 *
 * Revision 2.27  2004/07/12 16:32:56  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.26  2004/07/01 01:12:33  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.25  2004/04/18 19:39:13  randomtiger
 * Added -2d_poof command which allows access to 2D poof rendering
 * Added -radar_reduce to launcher flag description structure
 *
 * Revision 2.24  2004/04/14 10:25:50  taylor
 * don't page in neb poofs unless in a neb2 mission, fix OGL whiteout issue
 *
 * Revision 2.23  2004/04/11 13:56:33  randomtiger
 * Adding batching functions here and there and into gr_screen for use with OGL when its ready.
 *
 * Revision 2.22  2004/04/03 02:55:49  bobboau
 * commiting recent minor bug fixes
 *
 * Revision 2.21  2004/03/17 04:07:30  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.20  2004/03/05 09:02:07  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.19  2004/02/28 14:14:57  randomtiger
 * Removed a few uneeded if DIRECT3D's.
 * Set laser function to only render the effect one sided.
 * Added some stuff to the credits.
 * Set D3D fogging to fall back to vertex fog if table fog not supported.
 *
 * Revision 2.18  2004/02/14 00:18:34  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.17  2004/02/13 04:17:14  randomtiger
 * Turned off fog in OGL for Fred.
 * Simulated speech doesnt say tags marked by $ now.
 * The following are fixes to issues that came up testing TBP in fs2_open and fred2_open:
 * Changed vm_vec_mag and parse_tmap to fail gracefully on bad data.
 * Error now given on missing briefing icon and bad ship normal data.
 * Solved more species divide by zero error.
 * Fixed neb cube crash.
 *
 * Revision 2.16  2004/02/04 04:28:15  Goober5000
 * fixed Asserts in two places and commented out an unneeded variable
 * --Goober5000
 *
 * Revision 2.15  2004/01/12 21:12:42  randomtiger
 * Added fix for fogging debris in D3D htl.
 *
 * Revision 2.14  2003/11/29 10:52:10  randomtiger
 * Turned off D3D file mapping, its using too much memory which may be hurting older systems and doesnt seem to be providing much of a speed benifit.
 * Added stats command for ingame stats on memory usage.
 * Trys to play intro.mve and intro.avi, just to be safe since its not set by table.
 * Added fix for fonts wrapping round in non standard hi res modes.
 * Changed D3D mipmapping to a good value to suit htl mode.
 * Added new fog colour method which makes use of the bitmap, making this htl feature backcompatible again.
 *
 * Revision 2.13  2003/11/19 20:37:24  randomtiger
 * Almost fully working 32 bit pcx, use -pcx32 flag to activate.
 * Made some commandline variables fit the naming standard.
 * Changed timerbar system not to run pushes and pops if its not in use.
 * Put in a note about not uncommenting asserts.
 * Fixed up a lot of missing POP's on early returns?
 * Perhaps the motivation for Assert functionality getting commented out?
 * Fixed up some bad asserts.
 * Changed nebula poofs to render in 2D in htl, it makes it look how it used to in non htl. (neb.cpp,1248)
 * Before the poofs were creating a nasty stripe effect where they intersected with ships hulls.
 * Put in a special check for the signs of that D3D init bug I need to lock down.
 *
 * Revision 2.12  2003/11/17 04:25:57  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.11  2003/11/16 04:09:21  Goober5000
 * language
 *
 * Revision 2.10  2003/11/11 03:56:12  bobboau
 * lots of bug fixing, much of it in nebula and bitmap drawing
 *
 * Revision 2.9  2003/11/11 02:15:46  Goober5000
 * ubercommit - basically spelling and language fixes with some additional
 * warnings disabled
 * --Goober5000
 *
 * Revision 2.8  2003/10/14 17:39:16  randomtiger
 * Implemented hardware fog for the HT&L code path.
 * It doesnt use the backgrounds anymore but its still an improvement.
 * Currently it fogs to a brighter colour than it should because of Bob specular code.
 * I will fix this after discussing it with Bob.
 *
 * Also tided up some D3D stuff, a cmdline variable name and changed a small bit of
 * the htl code to use the existing D3D engine instead of work around it.
 * And added extra information in version number on bottom left of frontend screen.
 *
 * Revision 2.7  2003/09/26 14:37:15  bobboau
 * commiting Hardware T&L code, everything is ifdefed out with the compile flag HTL
 * still needs a lot of work, ubt the frame rates were getting with it are incredable
 * the biggest problem it still has is a bad lightmanegment system, and the zbuffer
 * doesn't work well with things still getting rendered useing the sofware pipeline, like thrusters,
 * and weapons, I think these should be modifyed to be sent through hardware,
 * it would be slightly faster and it would likely fix the problem
 *
 * also the thruster glow/particle stuff I did is now in.
 *
 * Revision 2.6  2003/03/19 23:06:40  Goober5000
 * bit o' housecleaning
 * --Goober5000
 *
 * Revision 2.5  2003/03/19 09:05:26  Goober5000
 * more housecleaning, this time for debug warnings
 * --Goober5000
 *
 * Revision 2.4  2003/03/19 06:22:58  Goober5000
 * added typecasting to get rid of some build warnings
 * --Goober5000
 *
 * Revision 2.3  2003/03/18 10:07:04  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.2  2002/11/18 21:32:48  phreak
 * added some lines that makes ogl work in windowed mode for fullneb - phreak
 *
 * Revision 2.1.2.3  2002/11/11 21:26:04  randomtiger
 *
 * Tided up D3DX8 calls, did some documentation and add new file: grd3dcalls.cpp. - RT
 *
 * Revision 2.1.2.2  2002/11/10 02:44:43  randomtiger
 *
 * Have put in a hack to get fog working in D3D8 but the method V has used is frowned
 * on in D3D8 and will likely be a lot slower than the same thing in D3D5. - RT
 *
 * Revision 2.1.2.1  2002/10/26 09:55:42  unknownplayer
 *
 * Fixed the nebula flicker bug. Check the NO_DIRECT3D conditional compile
 * sections in the future for bits of code which may cause problems due to hacks
 * correcting for DirectX5 that no longer apply.
 *
 * Revision 2.1  2002/08/01 01:41:07  penguin
 * The big include file move
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
 * 50    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 49    8/10/99 6:54p Dave
 * Mad optimizations. Added paging to the nebula effect.
 * 
 * 48    8/05/99 2:05a Dave
 * Whee.
 * 
 * 47    7/30/99 10:55a Anoop
 * Hmm. Fixed release build problem again, with area-rotated bitmaps.
 * 
 * 46    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 44    7/29/99 12:05a Dave
 * Nebula speed optimizations.
 * 
 * 43    7/19/99 7:20p Dave
 * Beam tooling. Specialized player-killed-self messages. Fixed d3d nebula
 * pre-rendering.
 * 
 * 42    7/18/99 5:20p Dave
 * Jump node icon. Fixed debris fogging. Framerate warning stuff.
 * 
 * 41    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 40    7/09/99 5:54p Dave
 * Seperated cruiser types into individual types. Added tons of new
 * briefing icons. Campaign screen.
 * 
 * 39    7/07/99 10:44a Jamesa
 * Make sure the nebula regens properly after loading a mission.
 * 
 * 38    6/11/99 2:32p Dave
 * Toned down nebula brightness a bit.
 * 
 * 37    5/26/99 3:39p Dave
 * Fixed nebula regeneration problem. Removed optimizations from
 * neblightning.cpp
 * 
 * 36    5/26/99 11:46a Dave
 * Added ship-blasting lighting and made the randomization of lighting
 * much more customizable.
 * 
 * 35    5/24/99 5:45p Dave
 * Added detail levels to the nebula, with a decent speedup. Split nebula
 * lightning into its own section.
 * 
 * $NoKeywords: $
 */

#include "nebula/neb.h"
#include "render/3d.h"
#include "bmpman/bmpman.h"
#include "object/object.h"
#include "freespace2/freespace.h"
#include "starfield/starfield.h"
#include "parse/parselo.h"
#include "pcxutils/pcxutils.h"
#include "tgautils/tgautils.h"
#include "jpgutils/jpgutils.h"
#include "ddsutils/ddsutils.h"
#include "mission/missionparse.h"
#include "ship/ship.h"
#include "cmdline/cmdline.h"



// --------------------------------------------------------------------------------------------------------
// NEBULA DEFINES/VARS
//

bool Nebula_sexp_used = false;

static ubyte Neb2_fog_color_r = 0;
static ubyte Neb2_fog_color_g = 0;
static ubyte Neb2_fog_color_b = 0;

static ubyte *Neb2_htl_fog_data = NULL;

// #define NEB2_THUMBNAIL

/*
3D CARDS THAT FOG PROPERLY
Voodoo1
Voodoo2
G200
TNT

3D CARDS THAT DON'T FOG PROPERLY
Permedia2
AccelStar II
*/

// if nebula rendering is active (DCF stuff - not mission specific)
int Neb2_render_mode = NEB2_RENDER_NONE;

// array of neb2 poofs
char Neb2_poof_filenames[MAX_NEB2_POOFS][MAX_FILENAME_LEN] = {
	"", "", "", "", "", ""
};
int Neb2_poofs[MAX_NEB2_POOFS] = { -1, -1, -1, -1, -1, -1 };
int Neb2_poof_flags = ( (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5) );
int Neb2_poof_count = 0;

// array of neb2 bitmaps
char Neb2_bitmap_filenames[MAX_NEB2_BITMAPS][MAX_FILENAME_LEN] = {
	"", "", "", "", "", ""
};
int Neb2_bitmap[MAX_NEB2_BITMAPS] = { -1, -1, -1, -1, -1, -1 };
int Neb2_bitmap_count = 0;

// texture to use for this level
char Neb2_texture_name[MAX_FILENAME_LEN] = "";

// nebula flags
#define NF_USED						(1<<0)		// if this nebula slot is used

float max_rotation = 3.75f;
float neb2_flash_fade = 0.3f;

// fog values for different ship types
//Why the heck is this still here? Commenting out. -WMC
/*
float Neb_ship_fog_vals_glide[MAX_SHIP_TYPE_COUNTS][2] = {
	{0.0f, 0.0f},				// SHIP_TYPE_NONE
	{10.0f, 500.0f},			// SHIP_TYPE_CARGO
	{10.0f, 500.0f},			// SHIP_TYPE_FIGHTER_BOMBER
	{10.0f, 600.0f},			// SHIP_TYPE_CRUISER
	{10.0f, 600.0f},			// SHIP_TYPE_FREIGHTER
	{10.0f, 750.0f},			// SHIP_TYPE_CAPITAL
	{10.0f, 500.0f},			// SHIP_TYPE_TRANSPORT
	{10.0f, 500.0f},			// SHIP_TYPE_REPAIR_REARM
	{10.0f, 500.0f},			// SHIP_TYPE_NAVBUOY
	{10.0f, 500.0f},			// SHIP_TYPE_SENTRYGUN
	{10.0f, 600.0f},			// SHIP_TYPE_ESCAPEPOD
	{10.0f, 1000.0f},			// SHIP_TYPE_SUPERCAP
	{10.0f, 500.0f},			// SHIP_TYPE_STEALTH
	{10.0f, 500.0f},			// SHIP_TYPE_FIGHTER
	{10.0f, 500.0f},			// SHIP_TYPE_BOMBER
	{10.0f, 750.0f},			// SHIP_TYPE_DRYDOCK
	{10.0f, 600.0f},			// SHIP_TYPE_AWACS
	{10.0f, 600.0f},			// SHIP_TYPE_GAS_MINER
	{10.0f, 600.0f},			// SHIP_TYPE_CORVETTE
	{10.0f, 1000.0f},			// SHIP_TYPE_KNOSSOS_DEVICE
};
*/
/*
float Neb_ship_fog_vals_d3d[MAX_SHIP_TYPE_COUNTS][2] = {
	{0.0f, 0.0f},				// SHIP_TYPE_NONE
	{10.0f, 500.0f},			// SHIP_TYPE_CARGO
	{10.0f, 500.0f},			// SHIP_TYPE_FIGHTER_BOMBER
	{10.0f, 600.0f},			// SHIP_TYPE_CRUISER
	{10.0f, 600.0f},			// SHIP_TYPE_FREIGHTER
	{10.0f, 750.0f},			// SHIP_TYPE_CAPITAL
	{10.0f, 500.0f},			// SHIP_TYPE_TRANSPORT
	{10.0f, 500.0f},			// SHIP_TYPE_REPAIR_REARM
	{10.0f, 500.0f},			// SHIP_TYPE_NAVBUOY
	{10.0f, 500.0f},			// SHIP_TYPE_SENTRYGUN
	{10.0f, 600.0f},			// SHIP_TYPE_ESCAPEPOD
	{10.0f, 1000.0f},			// SHIP_TYPE_SUPERCAP
	{10.0f, 500.0f},			// SHIP_TYPE_STEALTH
	{10.0f, 500.0f},			// SHIP_TYPE_FIGHTER
	{10.0f, 500.0f},			// SHIP_TYPE_BOMBER
	{10.0f, 750.0f},			// SHIP_TYPE_DRYDOCK
	{10.0f, 600.0f},			// SHIP_TYPE_AWACS
	{10.0f, 600.0f},			// SHIP_TYPE_GAS_MINER
	{10.0f, 600.0f},			// SHIP_TYPE_CORVETTE
	{10.0f, 1000.0f},			// SHIP_TYPE_KNOSSOS_DEVICE
};
*/
//WMC - these were originally indexed to SHIP_TYPE_FIGHTER_BOMBER
const static float Default_fog_near = 10.0f;
const static float Default_fog_far = 500.0f;

// fog near and far values for rendering the background nebula
#define NEB_BACKG_FOG_NEAR_GLIDE				2.5f
#define NEB_BACKG_FOG_NEAR_D3D				4.5f
#define NEB_BACKG_FOG_FAR_GLIDE				10.0f
#define NEB_BACKG_FOG_FAR_D3D					10.0f
float Neb_backg_fog_near = NEB_BACKG_FOG_NEAR_GLIDE;
float Neb_backg_fog_far = NEB_BACKG_FOG_FAR_GLIDE;

// stats
int pneb_tried = 0;				// total pnebs tried to render
int pneb_tossed_alpha = 0;		// pnebs tossed because of alpha optimization
int pneb_tossed_dot = 0;		// pnebs tossed because of dot product
int pneb_tossed_off = 0;		// pnebs tossed because of being offscree
int neb_tried = 0;				// total nebs tried
int neb_tossed_alpha = 0;		// nebs tossed because of alpha
int neb_tossed_dot = 0;			// nebs tossed because of dot product
int neb_tossed_count = 0;		// nebs tossed because of max render count 

// the AWACS suppresion level for the nebula
float Neb2_awacs = -1.0f;

// how many "slices" are in the current player nebuls
int Neb2_slices = 5;

cube_poof Neb2_cubes[MAX_CPTS][MAX_CPTS][MAX_CPTS];

// nebula detail level
typedef struct neb2_detail {
	float max_alpha_glide;					// max alpha for this detail level in Glide
	float max_alpha_d3d;						// max alpha for this detail level in D3d
	float break_alpha;						// break alpha (below which, poofs don't draw). this affects the speed and visual quality a lot
	float break_x, break_y;					// x and y alpha fade/break values. adjust alpha on the polys as they move offscreen 
	float cube_dim;							// total dimension of player poof cube
	float cube_inner;							// inner radius of the player poof cube
	float cube_outer;							// outer radius of the player pood cube
	float prad;									// radius of the poofs
	float wj, hj, dj;							// width, height, depth jittering. best left at 1.0	
} neb2_detail;
neb2_detail	Neb2_detail[MAX_DETAIL_LEVEL] = {
	{ // lowest detail level
		0.575f,										// max alpha for this detail level in Glide
		0.71f,									// max alpha for this detail level in D3d
		0.13f,									// break alpha (below which, poofs don't draw). this affects the speed and visual quality a lot
		150.0f, 150.0f / 1.3333f,			// x and y alpha fade/break values. adjust alpha on the polys as they move offscreen 
		510.0f,									// total dimension of player poof cube
		50.0f,									// inner radius of the player poof cube
		250.0f,									// outer radius of the player pood cube
		120.0f,									// radius of the poofs
		1.0f, 1.0f, 1.0f						// width, height, depth jittering. best left at 1.0	
	},	
	{ // 2nd lowest detail level
		0.575f,										// max alpha for this detail level in Glide
		0.71f,									// max alpha for this detail level in D3d
		0.125f,									// break alpha (below which, poofs don't draw). this affects the speed and visual quality a lot
		300.0f, 300.0f / 1.3333f,			// x and y alpha fade/break values. adjust alpha on the polys as they move offscreen 
		550.0f,									// total dimension of player poof cube
		100.0f,									// inner radius of the player poof cube
		250.0f,									// outer radius of the player pood cube
		125.0f,									// radius of the poofs
		1.0f, 1.0f, 1.0f						// width, height, depth jittering. best left at 1.0	
	},
	{ // 2nd highest detail level
		0.575f,										// max alpha for this detail level in Glide
		0.71f,									// max alpha for this detail level in D3d
		0.1f,										// break alpha (below which, poofs don't draw). this affects the speed and visual quality a lot
		300.0f, 300.0f / 1.3333f,			// x and y alpha fade/break values. adjust alpha on the polys as they move offscreen 
		550.0f,									// total dimension of player poof cube
		150.0f,									// inner radius of the player poof cube
		250.0f,									// outer radius of the player pood cube
		125.0f,									// radius of the poofs
		1.0f, 1.0f, 1.0f						// width, height, depth jittering. best left at 1.0	
	},
	{ // higest detail level
		0.475f,									// max alpha for this detail level in Glide
		0.575f,									// max alpha for this detail level in D3d
		0.05f,									// break alpha (below which, poofs don't draw). this affects the speed and visual quality a lot
		200.0f, 200.0f / 1.3333f,			// x and y alpha fade/break values. adjust alpha on the polys as they move offscreen 
		750.0f,									// total dimension of player poof cube
		200.0f,									// inner radius of the player poof cube
		360.0f,									// outer radius of the player pood cube
		150.0f,									// radius of the poofs
		1.0f, 1.0f, 1.0f						// width, height, depth jittering. best left at 1.0	
	},		
};
neb2_detail *Nd = &Neb2_detail[MAX_DETAIL_LEVEL - 2];

int Neb2_background_color[3] = {0, 0, 255};			// rgb background color (used for lame rendering)

int Neb2_regen = 0;

// --------------------------------------------------------------------------------------------------------
// NEBULA FORWARD DECLARATIONS
//

// return the alpha the passed poof should be rendered with, for a 2 shell nebula
float neb2_get_alpha_2shell(float inner_radius, float outer_radius, float magic_num, vec3d *v);

// return an alpha value for a bitmap offscreen based upon "break" value
float neb2_get_alpha_offscreen(float sx, float sy, float incoming_alpha);

// do a pre-render of the background nebula
void neb2_pre_render(vec3d *eye_pos, matrix *eye_orient);

// fill in the position of the eye for this frame
void neb2_get_eye_pos(vec3d *eye);

// fill in the eye orient for this frame
void neb2_get_eye_orient(matrix *eye);

// get a (semi) random bitmap to use for a poof
int neb2_get_bitmap();

// regenerate the player nebula
void neb2_regen();


// --------------------------------------------------------------------------------------------------------
// NEBULA FUNCTIONS
//

// initialize neb2 stuff at game startup
void neb2_init()
{	
	char name[MAX_FILENAME_LEN];

	// read in the nebula.tbl
	read_file_text("nebula.tbl");
	reset_parse();

	// background bitmaps
	Neb2_bitmap_count = 0;
	while(!optional_string("#end")){
		// nebula
		required_string("+Nebula:");
		stuff_string(name, F_NAME, MAX_FILENAME_LEN);
		
		if(Neb2_bitmap_count < MAX_NEB2_BITMAPS){
			strcpy(Neb2_bitmap_filenames[Neb2_bitmap_count++], name);
		}
	}

	// poofs
	Neb2_poof_count = 0;
	while(!optional_string("#end")){
		// nebula
		required_string("+Poof:");
		stuff_string(name, F_NAME, MAX_FILENAME_LEN);

		if(Neb2_poof_count < MAX_NEB2_POOFS){
			strcpy(Neb2_poof_filenames[Neb2_poof_count++], name);
		}
	}

	//Distance
	//WMC - Obsolete! Obsolete! Obsolete!
	/*
	if(optional_string("#Fog Distance"))
	{
		char buf[32];
		int len = sizeof(Neb_ship_fog_vals_d3d)/sizeof(Neb_ship_fog_vals_d3d[0]);

		for(int i = 0; i < len; i++)
		{
			Assert( i < (int)(sizeof(Ship_type_names) / sizeof(Ship_type_names[0])) );

			//Create the var string and parse it if necessary
			sprintf(buf, "$%s:", Ship_type_names[i]);
			if(optional_string(buf)) {
				stuff_float_list((float *)&(Neb_ship_fog_vals_d3d[i]), 2);
			}
		}

		required_string("#End");
	}*/

	// should always have 6 neb poofs
	Assert(Neb2_poof_count == 6);
}

// set detail level
void neb2_set_detail_level(int level)
{
	// sanity
	if(level < 0){
		Nd = &Neb2_detail[0];
		return;
	}
	if(level >= MAX_DETAIL_LEVEL){
		Nd = &Neb2_detail[MAX_DETAIL_LEVEL-1];
		return;
	}

	Nd = &Neb2_detail[level];

	// regen the player neb
	Neb2_regen = 1;
}

void neb2_get_fog_color(ubyte *r, ubyte *g, ubyte *b)
{
	if (r) *r = Neb2_fog_color_r;
	if (g) *g = Neb2_fog_color_g;
	if (b) *b = Neb2_fog_color_b;
}

void neb2_level_init()
{
	Nebula_sexp_used = false;
}

// initialize nebula stuff - call from game_post_level_init(), so the mission has been loaded
void neb2_post_level_init()
{
	int idx;		

	// standalone servers can bail here
	if(Game_mode & GM_STANDALONE_SERVER){
		return;
	}

	// if the mission is not a fullneb mission, skip
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB) && !Nebula_sexp_used){
		Neb2_render_mode = NEB2_RENDER_NONE;
		Neb2_awacs = -1.0f;
		return;
	}

	if(Cmdline_nohtl || Fred_running && (The_mission.flags & MISSION_FLAG_FULLNEB)) {
		// by default we'll use pof rendering
		Neb2_render_mode = NEB2_RENDER_POF;
		stars_set_background_model(BACKGROUND_MODEL_FILENAME, Neb2_texture_name);
	} else {
		// Set a default colour just in case something goes wrong
		Neb2_fog_color_r =  30;
		Neb2_fog_color_g =  52;
		Neb2_fog_color_b = 157;

		// OK, lets try something a bit more interesting
		if (strlen(Neb2_texture_name)) {
			Neb2_htl_fog_data = new ubyte[768];

			if ( pcx_read_header(Neb2_texture_name, NULL, NULL, NULL, NULL, Neb2_htl_fog_data) == PCX_ERROR_NONE ) {
				// based on the palette, get an average color value (this doesn't really account for actual pixel usage though)
				ushort r = 0, g = 0, b = 0, pcount = 0;
				for (idx = 0; idx < 768; idx += 3) {
					if (Neb2_htl_fog_data[idx] || Neb2_htl_fog_data[idx+1] || Neb2_htl_fog_data[idx+2]) {
						r += Neb2_htl_fog_data[idx];
						g += Neb2_htl_fog_data[idx+1];
						b += Neb2_htl_fog_data[idx+2];
						pcount++;
					}
				}

				if (pcount > 0) {
					Neb2_fog_color_r = (ubyte)(r / pcount);
					Neb2_fog_color_g = (ubyte)(g / pcount);
					Neb2_fog_color_b = (ubyte)(b / pcount);
				} else {
					// it's just black
					Neb2_fog_color_r = Neb2_fog_color_g = Neb2_fog_color_b = 0;
				}

				// done, now free up the palette data
				if ( Neb2_htl_fog_data != NULL ) {
					delete[] Neb2_htl_fog_data;
					Neb2_htl_fog_data = NULL;
				}
			}
		}

		Neb2_render_mode = NEB2_RENDER_HTL;
	}

	// load in all nebula bitmaps
	for(idx=0; idx<Neb2_poof_count; idx++){
		if(Neb2_poofs[idx] < 0){
			Neb2_poofs[idx] = bm_load(Neb2_poof_filenames[idx]);
		}
	}

	pneb_tried = 0;		
	pneb_tossed_alpha = 0;		
	pneb_tossed_dot = 0;
	neb_tried = 0;		
	neb_tossed_alpha = 0;		
	neb_tossed_dot = 0;
	neb_tossed_count = 0;

	// setup proper fogging values
	Neb_backg_fog_near = NEB_BACKG_FOG_NEAR_D3D;
	Neb_backg_fog_far = NEB_BACKG_FOG_FAR_D3D;					

	// regen the nebula
	neb2_eye_changed();
}

// shutdown nebula stuff
void neb2_level_close()
{
	int idx;
	
	// standalone servers can bail here
	if(Game_mode & GM_STANDALONE_SERVER){
		return;
	}

	// if the mission is not a fullneb mission, skip
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB) && !Nebula_sexp_used){
		return;
	}

	// unload all nebula bitmaps
	for(idx=0; idx<Neb2_poof_count; idx++){
		if(Neb2_poofs[idx] >= 0){
			bm_release(Neb2_poofs[idx]);
			Neb2_poofs[idx] = -1;
		}
	}	

	// unflag the mission as being fullneb so stuff doesn't fog in the techdata room :D
	The_mission.flags &= ~MISSION_FLAG_FULLNEB;

	if (Neb2_htl_fog_data) {
		delete[] Neb2_htl_fog_data;
		Neb2_htl_fog_data = NULL;
	}
}

// call before beginning all rendering
void neb2_render_setup(vec3d *eye_pos, matrix *eye_orient)
{
	// standalone servers can bail here
	if(Game_mode & GM_STANDALONE_SERVER){
		return;
	}

	// if the mission is not a fullneb mission, skip
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB) && !Nebula_sexp_used){		
		return;
	}

	if (Neb2_render_mode == NEB2_RENDER_HTL) {
		// RT The background needs to be the same colour as the fog and this seems
		// to be the ideal place to do it
		ubyte tr = gr_screen.current_clear_color.red;  
		ubyte tg = gr_screen.current_clear_color.green;
		ubyte tb = gr_screen.current_clear_color.blue; 

		neb2_get_fog_color(
			&gr_screen.current_clear_color.red,
			&gr_screen.current_clear_color.green,
			&gr_screen.current_clear_color.blue);
		
		gr_clear();
		
		gr_screen.current_clear_color.red   = tr;
		gr_screen.current_clear_color.green = tg;
		gr_screen.current_clear_color.blue  = tb;

		return;	 
	}


	// pre-render the real background nebula
	neb2_pre_render(eye_pos, eye_orient);		
}

// level paging code
void neb2_page_in()
{
	int idx;

	// load in all nebula bitmaps
	if (The_mission.flags & MISSION_FLAG_FULLNEB || Nebula_sexp_used) {
		for(idx=0; idx<Neb2_poof_count; idx++){
			if((Neb2_poofs[idx] >= 0) && (Neb2_poof_flags & (1<<idx))){
				bm_page_in_texture(Neb2_poofs[idx]);
			}
		}
	}
}

// should we not render this object because its obscured by the nebula?
int neb_skip_opt = 1;
DCF(neb_skip, "")
{
	neb_skip_opt = !neb_skip_opt;
	if(neb_skip_opt){
		dc_printf("Using neb object skipping!\n");
	} else {
		dc_printf("Not using neb object skipping!\n");
	}
}
int neb2_skip_render(object *objp, float z_depth)
{
	float fog_near, fog_far;		

	// if we're never skipping
	if(!neb_skip_opt){
		return 0;
	}

	// lame rendering
	if(Neb2_render_mode == NEB2_RENDER_LAME){
		return 0;
	}

	// get near and far fog values based upon object type and rendering mode
	neb2_get_fog_values(&fog_near, &fog_far, objp);

	// by object type
	switch( objp->type )	{
	// some objects we always render
	case OBJ_SHOCKWAVE:
	case OBJ_JUMP_NODE:
	case OBJ_NONE:
	case OBJ_GHOST:
	case OBJ_BEAM:
	case OBJ_WAYPOINT:
		return 0;			
		
	// any weapon over 500 meters away 
	case OBJ_WEAPON:		
		if(z_depth >= 500.0f){
			return 1;
		}
		break;

	// any small ship over the fog limit, or any cruiser 50% further than the fog limit
	case OBJ_SHIP:	
		ship_info *sip;
		if((objp->instance >= 0) && (Ships[objp->instance].ship_info_index >= 0)){
			sip = &Ship_info[Ships[objp->instance].ship_info_index];
		} else {
			return 0;
		}

		// small ships over the fog limit by a small factor
		if(sip->class_type > -1)
		{
			if(z_depth >= (fog_far * Ship_types[sip->class_type].fog_disappear_factor)) {
				return 1;
			}
		}
		break;

	// any fireball over the fog limit for small ships
	case OBJ_FIREBALL:		
		/*
		if(z_depth >= fog_far){
			return 1;
		}
		*/
		return 0;
		break;	

	// any debris over the fog limit for small ships
	case OBJ_DEBRIS:		
		/*
		if(z_depth >= fog_far){
			return 1;
		}
		*/
		return 0;
		break;

	// any asteroids 50% farther than the fog limit for small ships
	case OBJ_ASTEROID:		
		if(z_depth >= (fog_far * 1.5f)){
			return 1;
		}
		break;
/*
	// any countermeasures over 100 meters away
	case OBJ_CMEASURE:		
		if(z_depth >= 100.0f){
			return 1;
		}
		break;	
*/
	// hmmm. unknown object type - should probably let it through
	default:
		Int3();
		return 0;
	}

	return 0;
}

// extend LOD 
float neb2_get_lod_scale(int objnum)
{	
	ship *shipp;
	ship_info *sip;

	// bogus
	if((objnum < 0) || (objnum >= MAX_OBJECTS) || (Objects[objnum].type != OBJ_SHIP) || (Objects[objnum].instance < 0) || (Objects[objnum].instance >= MAX_SHIPS)){
		return 1.0f;
	}
	shipp = &Ships[Objects[objnum].instance];
	sip = &Ship_info[shipp->ship_info_index];

	// small ship?
	if(sip->flags & SIF_SMALL_SHIP){
		return 1.8f;
	} else if(sip->flags & SIF_BIG_SHIP){
		return 1.4f;
	}	

	// hmm
	return 1.0f;
}


// --------------------------------------------------------------------------------------------------------
// NEBULA FORWARD DEFINITIONS
//

// return the alpha the passed poof should be rendered with, for a 2 shell nebula
float neb2_get_alpha_2shell(float inner_radius, float outer_radius, float magic_num, vec3d *v)
{			
	float dist;
	float alpha;
	vec3d eye_pos;

	// get the eye position
	neb2_get_eye_pos(&eye_pos);
	
	// determine what alpha to draw this bitmap with
	// higher alpha the closer the bitmap gets to the eye
	dist = vm_vec_dist_quick(&eye_pos, v);	

	// if the point is inside the inner radius, alpha is based on distance to the player's eye, 
	// becoming more transparent as it gets close
	if(dist <= inner_radius){
		// alpha per meter between the magic # and the inner radius
		alpha = Nd->max_alpha_glide / (inner_radius - magic_num);

		// above value times the # of meters away we are
		alpha *= (dist - magic_num);
		return alpha < 0.0f ? 0.0f : alpha;
	}
	// if the point is outside the inner radius, it starts out as completely transparent at max 	
	// outer radius, and becomes more opaque as it moves towards inner radius	
	else if(dist <= outer_radius){				
		// alpha per meter between the outer radius and the inner radius
		alpha = Nd->max_alpha_glide / (outer_radius - inner_radius);

		// above value times the range between the outer radius and the poof
		return alpha < 0.0f ? 0.0f : alpha * (outer_radius - dist);
	}

	// otherwise transparent
	return 0.0f;	
}

// return an alpha value for a bitmap offscreen based upon "break" value
float neb2_get_alpha_offscreen(float sx, float sy, float incoming_alpha)
{	
	float alpha = 0.0f;
	float per_pixel_x = incoming_alpha / (float)Nd->break_x;
	float per_pixel_y = incoming_alpha / (float)Nd->break_y;
	int off_x = ((sx < 0.0f) || (sx > (float)gr_screen.max_w));	
	int off_y = ((sy < 0.0f) || (sy > (float)gr_screen.max_h));
	float off_x_amount = 0.0f;
	float off_y_amount = 0.0f;

	// determine how many pixels outside we are
	if(off_x){
		if(sx < 0.0f){
			off_x_amount = fl_abs(sx);
		} else {
			off_x_amount = sx - (float)gr_screen.max_w;
		}
	}
	if(off_y){
		if(sy < 0.0f){
			off_y_amount = fl_abs(sy);
		} else {
			off_y_amount = sy - (float)gr_screen.max_h;
		}
	}

	// if offscreen X
	if(off_x){
		// offscreen X and Y - and Y is greater
		if(off_y && (off_y_amount > off_x_amount)){			
			alpha = incoming_alpha - (off_y_amount * per_pixel_y);
		} else {
			alpha = incoming_alpha - (off_x_amount * per_pixel_x);
		}
	}
	// offscreen y
	else if(off_y){
		alpha = incoming_alpha - (off_y_amount * per_pixel_y);
	}
	// should never get here
	else { 		
		Int3();
	}

	return alpha < 0.0f ? 0.0f : alpha;			
}

// -------------------------------------------------------------------------------------------------
// WACKY LOCAL PLAYER NEBULA STUFF
//

vec3d cube_cen;

int crossed_border()
{
	vec3d eye_pos;
	float ws = Nd->cube_dim / (float)Neb2_slices;
	float hs = Nd->cube_dim / (float)Neb2_slices;
	float ds = Nd->cube_dim / (float)Neb2_slices;

	// get the eye position
	neb2_get_eye_pos(&eye_pos);

	// check left, right (0, and 1, x and -x)
	if(cube_cen.xyz.x - eye_pos.xyz.x > ws){
		// -x
		return 0;
	} else if(eye_pos.xyz.x - cube_cen.xyz.x > ws){
		// +x
		return 1;
	}

	// check up, down (2, and 3, y and -y)
	if(cube_cen.xyz.y - eye_pos.xyz.y > hs){
		// -y
		return 2;
	} else if(eye_pos.xyz.y - cube_cen.xyz.y > hs){
		// +y
		return 3;
	}

	// check front, back (4, and 5, z and -z)
	if(cube_cen.xyz.z - eye_pos.xyz.z > ds){
		// -z
		return 4;
	} else if(eye_pos.xyz.z - cube_cen.xyz.z > ds){
		// +z
		return 5;
	}

	// nothing
	return -1;
}

void neb2_copy(int xyz, int src, int dest)
{
	int idx1, idx2;

	switch(xyz){
	case 0:
		for(idx1=0; idx1<Neb2_slices; idx1++){
			for(idx2=0; idx2<Neb2_slices; idx2++){
				Neb2_cubes[dest][idx1][idx2] = Neb2_cubes[src][idx1][idx2];				
			}
		}
		break;
	case 1:
		for(idx1=0; idx1<Neb2_slices; idx1++){
			for(idx2=0; idx2<Neb2_slices; idx2++){				
				Neb2_cubes[idx1][dest][idx2] = Neb2_cubes[idx1][src][idx2];				
			}
		}
		break;
	case 2:
		for(idx1=0; idx1<Neb2_slices; idx1++){
			for(idx2=0; idx2<Neb2_slices; idx2++){
				Neb2_cubes[idx1][idx2][dest] = Neb2_cubes[idx1][idx2][src];				
			}
		}
		break;
	default:
		Int3();
		break;
	}
}

void neb2_gen_slice(int xyz, int src, vec3d *cube_center)
{
	int idx1, idx2;	
	float h_incw, h_inch, h_incd;
	float ws, hs, ds;
	vec3d cube_corner;	
	vec3d *v;

	ws = Nd->cube_dim / (float)Neb2_slices;
	h_incw = ws / 2.0f;
	hs = Nd->cube_dim / (float)Neb2_slices;
	h_inch = hs / 2.0f;
	ds = Nd->cube_dim / (float)Neb2_slices;	
	h_incd = ds / 2.0f;
	cube_corner = *cube_center;		
	cube_corner.xyz.x -= (Nd->cube_dim / 2.0f);			
	cube_corner.xyz.y -= (Nd->cube_dim / 2.0f);	
	cube_corner.xyz.z -= (Nd->cube_dim / 2.0f);	
	switch(xyz){
	case 0:
		for(idx1=0; idx1<Neb2_slices; idx1++){
			for(idx2=0; idx2<Neb2_slices; idx2++){
				v = &Neb2_cubes[src][idx1][idx2].pt;

				v->xyz.x = h_incw + (ws * (float)src) + frand_range(-Nd->wj, Nd->wj);
				v->xyz.y = h_inch + (hs * (float)idx1) + frand_range(-Nd->hj, Nd->hj);
				v->xyz.z = h_incd + (ds * (float)idx2) + frand_range(-Nd->dj, Nd->dj);
				vm_vec_add2(v, &cube_corner);

				// set the bitmap
				Neb2_cubes[src][idx1][idx2].bmap = neb2_get_bitmap();

				// set the rotation speed
				Neb2_cubes[src][idx1][idx2].rot = 0.0f;
				Neb2_cubes[src][idx1][idx2].rot_speed = frand_range(-max_rotation, max_rotation);
				Neb2_cubes[src][idx1][idx2].flash = 0.0f;
			}
		}
		break;
	case 1:
		for(idx1=0; idx1<Neb2_slices; idx1++){
			for(idx2=0; idx2<Neb2_slices; idx2++){
				v = &Neb2_cubes[idx1][src][idx2].pt;
				
				v->xyz.x = h_incw + (ws * (float)idx1) + frand_range(-Nd->wj, Nd->wj);
				v->xyz.y = h_inch + (hs * (float)src) + frand_range(-Nd->hj, Nd->hj);
				v->xyz.z = h_incd + (ds * (float)idx2) + frand_range(-Nd->dj, Nd->dj);
				vm_vec_add2(v, &cube_corner);

				// set the bitmap
				Neb2_cubes[idx1][src][idx2].bmap = neb2_get_bitmap();

				// set the rotation speed
				Neb2_cubes[idx1][src][idx2].rot = 0.0f;
				Neb2_cubes[idx1][src][idx2].rot_speed = frand_range(-max_rotation, max_rotation);
				Neb2_cubes[src][idx1][idx2].flash = 0.0f;
			}
		}
		break;
	case 2:
		for(idx1=0; idx1<Neb2_slices; idx1++){
			for(idx2=0; idx2<Neb2_slices; idx2++){
				v = &Neb2_cubes[idx1][idx2][src].pt;

				v->xyz.x = h_incw + (ws * (float)idx1) + frand_range(-Nd->wj, Nd->wj);
				v->xyz.y = h_inch + (hs * (float)idx2) + frand_range(-Nd->hj, Nd->hj);
				v->xyz.z = h_incd + (ds * (float)src) + frand_range(-Nd->dj, Nd->dj);
				vm_vec_add2(v, &cube_corner);
				
				// set the bitmap
				Neb2_cubes[idx1][idx2][src].bmap = neb2_get_bitmap();

				// set the rotation speed
				Neb2_cubes[idx1][idx2][src].rot = 0.0f;
				Neb2_cubes[idx1][idx2][src].rot_speed = frand_range(-max_rotation, max_rotation);
				Neb2_cubes[src][idx1][idx2].flash = 0.0f;
			}
		}
		break;
	default:
		Int3();
		break;
	}
}

// regenerate the player nebula
void neb2_regen()
{
	int idx;
	vec3d eye_pos;	
	matrix eye_orient;

	mprintf(("Regenerating local nebula!\n"));

	// get eye position and orientation
	neb2_get_eye_pos(&eye_pos);
	neb2_get_eye_orient(&eye_orient);	

	// determine the corner of the cube
	cube_cen = eye_pos;
		
	// generate slices of the cube
	for(idx=0; idx<Neb2_slices; idx++){
		neb2_gen_slice(0, idx, &cube_cen);
	}
}

float max_area = 100000000.0f;
DCF(max_area, "")
{
	dc_get_arg(ARG_FLOAT);
	max_area = Dc_arg_float;
}

float g3_draw_rotated_bitmap_area(vertex *pnt, float angle, float rad, uint tmap_flags, float area);
int neb_mode = 1;
int frames_total = 0;
int frame_count = 0;
float frame_avg;
void neb2_render_player()
{
	vertex p, ptemp;
	int idx1, idx2, idx3;
	float alpha;
	int frame_rendered;	
	vec3d eye_pos;
	matrix eye_orient;


	// standalone servers can bail here
	if(Game_mode & GM_STANDALONE_SERVER){
		return;
	}

	// if the mission is not a fullneb mission, skip
	if(!(The_mission.flags & MISSION_FLAG_FULLNEB)){
		return;
	}		

	if(Neb2_regen){
		neb2_regen();
		Neb2_regen = 0;
	}

	// don't render in lame mode
	if((Neb2_render_mode == NEB2_RENDER_LAME) || (Neb2_render_mode == NEB2_RENDER_NONE)){
		return;
	}

	// get eye position and orientation
	neb2_get_eye_pos(&eye_pos);
	neb2_get_eye_orient(&eye_orient);
	
	// maybe swap stuff around if the player crossed a "border"	
	for(idx2=0; idx2<3; idx2++){
		switch(crossed_border()){
		case -1:
			break;
		// -x
		case 0 :
			cube_cen.xyz.x -= Nd->cube_dim / (float)Neb2_slices;
			for(idx1=Neb2_slices-1; idx1>0; idx1--){
				neb2_copy(0, idx1-1, idx1);
			}
			neb2_gen_slice(0, 0, &cube_cen);				
			break;
		// x
		case 1 :
			cube_cen.xyz.x += Nd->cube_dim / (float)Neb2_slices;
			for(idx1=0; idx1<Neb2_slices-1; idx1++){
				neb2_copy(0, idx1+1, idx1);
			}				
			neb2_gen_slice(0, Neb2_slices - 1, &cube_cen);				
			break;
		// -y
		case 2 :			
			cube_cen.xyz.y -= Nd->cube_dim / (float)Neb2_slices;
			for(idx1=Neb2_slices-1; idx1>0; idx1--){
				neb2_copy(1, idx1-1, idx1);
			}				
			neb2_gen_slice(1, 0, &cube_cen);				
			break;
		// y
		case 3 :						
			cube_cen.xyz.y += Nd->cube_dim / (float)Neb2_slices;
			for(idx1=0; idx1<Neb2_slices-1; idx1++){
				neb2_copy(1, idx1+1, idx1);
			}				
			neb2_gen_slice(1, Neb2_slices - 1, &cube_cen);				
			break;
		// -z
		case 4 :			
			cube_cen.xyz.z -= Nd->cube_dim / (float)Neb2_slices;
			for(idx1=Neb2_slices-1; idx1>0; idx1--){
				neb2_copy(2, idx1-1, idx1);
			}								
			neb2_gen_slice(2, 0, &cube_cen);				
			break;
		// z
		case 5 :						
			cube_cen.xyz.z += Nd->cube_dim / (float)Neb2_slices;
			for(idx1=0; idx1<Neb2_slices-1; idx1++){
				neb2_copy(2, idx1+1, idx1);
			}												
			neb2_gen_slice(2, Neb2_slices - 1, &cube_cen);				
			break;
		}	
	}	

	// if we've switched nebula rendering off
	if(Neb2_render_mode == NEB2_RENDER_NONE){
		return;
	}	
	
	frame_rendered = 0;
	// render the nebula
	for(idx1=0; idx1<Neb2_slices; idx1++){
		for(idx2=0; idx2<Neb2_slices; idx2++){
			for(idx3=0; idx3<Neb2_slices; idx3++){

				// Miss this one out if the id is -1
				if(Neb2_cubes[idx1][idx2][idx3].bmap == -1)
					continue;

				pneb_tried++;				

				// rotate the poof
				Neb2_cubes[idx1][idx2][idx3].rot += (Neb2_cubes[idx1][idx2][idx3].rot_speed * flFrametime);
				if(Neb2_cubes[idx1][idx2][idx3].rot >= 360.0f){
					Neb2_cubes[idx1][idx2][idx3].rot = 0.0f;
				}				
				
				// optimization 1 - don't draw backfacing poly's
				// useless
				if(vm_vec_dot_to_point(&eye_orient.vec.fvec, &eye_pos, &Neb2_cubes[idx1][idx2][idx3].pt) <= 0.0f){
					pneb_tossed_dot++;
					continue;
				}

				vertex p_;
				// rotate and project the vertex into viewspace
				g3_rotate_vertex(&p, &Neb2_cubes[idx1][idx2][idx3].pt);
				if(!Cmdline_nohtl) g3_transfer_vertex(&p_, &Neb2_cubes[idx1][idx2][idx3].pt);

				ptemp = p;
				g3_project_vertex(&ptemp);

				// get the proper alpha value				
				alpha = neb2_get_alpha_2shell(Nd->cube_inner, Nd->cube_outer, Nd->prad/4.0f, &Neb2_cubes[idx1][idx2][idx3].pt);

				// optimization 2 - don't draw 0.0f or less poly's
				// this amounts to big savings
				if(alpha <= Nd->break_alpha){
					pneb_tossed_alpha++;
					continue;
				}

				// drop poly's which are offscreen at all				
				// if the poly's are offscreen						
				if((ptemp.sx < 0.0f) || (ptemp.sx > (float)gr_screen.max_w) || (ptemp.sy < 0.0f) || (ptemp.sy > (float)gr_screen.max_h) ){
					alpha = neb2_get_alpha_offscreen(ptemp.sx, ptemp.sy, alpha);
				}				

				// optimization 2 - don't draw 0.0f or less poly's
				// this amounts to big savings
				if(alpha <= Nd->break_alpha){
					pneb_tossed_alpha++;
					continue;
				}
	
				// set the bitmap and render				
				gr_set_bitmap(Neb2_cubes[idx1][idx2][idx3].bmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, (alpha + Neb2_cubes[idx1][idx2][idx3].flash));

/*#ifndef NDEBUG
				//	float this_area;
				float frame_area = max_area;
				float total_area = 0.0f;

				this_area = g3_draw_rotated_bitmap_area(&p, fl_radian(Neb2_cubes[idx1][idx2][idx3].rot), Nd->prad, TMAP_FLAG_TEXTURED, max_area);				
				total_area += this_area;
				frame_area -= this_area;
				frame_rendered++;			
#else */
				if (!Cmdline_nohtl) gr_set_lighting(false, false);
				gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
		 	  	if(Cmdline_nohtl || Cmdline_2d_poof)
	 				g3_draw_rotated_bitmap(&p, fl_radian(Neb2_cubes[idx1][idx2][idx3].rot), Nd->prad, TMAP_FLAG_TEXTURED);
		 	  	else g3_draw_rotated_bitmap(&p_, fl_radian(Neb2_cubes[idx1][idx2][idx3].rot), Nd->prad, TMAP_FLAG_TEXTURED | TMAP_HTL_3D_UNLIT);
//#endif
			}
		}
	}	

	frames_total += frame_rendered;
	frame_count++;
	frame_avg = (float)frames_total / (float)frame_count;	

	// gr_set_color_fast(&Color_bright_red);
	// gr_printf(30, 100, "Area %.3f", total_area);
#ifdef NEB2_THUMBNAIL
	extern int tbmap;
	if(tbmap != -1){
		gr_set_bitmap(tbmap);
		gr_bitmap(0, 0);
	}
#endif
}	

// call this when the player's viewpoint has changed, this will cause the code to properly reset
// the eye's local poofs
void neb2_eye_changed()
{
	Neb2_regen = 1;
}

/*
//Object types
#define OBJ_NONE				0		//unused object
#define OBJ_SHIP				1		//a ship
#define OBJ_WEAPON			2		//a laser, missile, etc
#define OBJ_FIREBALL			3		//an explosion
#define OBJ_START				4		//a starting point marker (player start, etc)
#define OBJ_WAYPOINT			5		//a waypoint object, maybe only ever used by Fred
#define OBJ_DEBRIS			6		//a flying piece of ship debris
#define OBJ_CMEASURE			7		//a countermeasure, such as chaff
#define OBJ_GHOST				8		//so far, just a placeholder for when a player dies.
#define OBJ_POINT				9		//generic object type to display a point in Fred.
#define OBJ_SHOCKWAVE		10		// a shockwave
#define OBJ_WING				11		// not really a type used anywhere, but I need it for Fred.
#define OBJ_OBSERVER       12    // used for multiplayer observers (possibly single player later)
#define OBJ_ASTEROID			13		//	An asteroid, you know, a big rock, like debris, sort of.
#define OBJ_JUMP_NODE		14		// A jump node object, used only in Fred.
#define OBJ_BEAM				15		// beam weapons. we have to roll them into the object system to get the benefits of the collision pairs
*/
// get near and far fog values based upon object type and rendering mode
void neb2_get_fog_values(float *fnear, float *ffar, object *objp)
{
	int type_index = -1;

	// default values in case something truly nasty happens
	*fnear = 10.0f;
	*ffar = 1000.0f;

	if (objp == NULL)
		return;

	//Otherwise, use defaults
	*fnear = Default_fog_near;
	*ffar = Default_fog_far;

	// determine what fog index to use
	if(objp->type == OBJ_SHIP)
	{
		Assert((objp->instance >= 0) && (objp->instance < MAX_SHIPS));
		if((objp->instance >= 0) && (objp->instance < MAX_SHIPS))
		{
			type_index = ship_query_general_type(objp->instance);
			if(type_index > 0)
			{
				*fnear = Ship_types[type_index].fog_start_dist;
				*ffar = Ship_types[type_index].fog_complete_dist;
			}
		}
	}
	else if(objp->type == OBJ_FIREBALL)
	{//mostly here for the warp effect
		*fnear = objp->radius*2;
		*ffar = (objp->radius*objp->radius*200)+objp->radius*200;
		return;
		//we want these to show up realy far away, this seems like a good value-Bobboau
/*	}else if(objp->type == OBJ_BEAM){
		*fnear = 10.0f;
		*ffar = 10000.0f;
		return;
	// fog everything else like a fighter
*/	}
}

float nNf_near, nNf_far;
// given a position in space, return a value from 0.0 to 1.0 representing the fog level 
float neb2_get_fog_intensity(object *obj)
{
	float pct;

	// get near and far fog values based upon object type and rendering mode
	neb2_get_fog_values(&nNf_near, &nNf_far, obj);

	// get the fog pct
	pct = vm_vec_dist_quick(&Eye_position, &obj->pos) / (nNf_far - nNf_near);
	if(pct < 0.0f){
		return 0.0f;
	} else if(pct > 1.0f){
		return 1.0f;
	}

	return pct;
}

//this only gets called after the one above has been called as it assumes you have set the near and far planes properly
//doun't use this outside of ship rendering
float neb2_get_fog_intensity(vec3d *pos)
{
	float pct;

	// get near and far fog values based upon object type and rendering mode
//	neb2_get_fog_values(&f_near, &f_far, pos);

	// get the fog pct
	pct = vm_vec_dist_quick(&Eye_position, pos) / ((nNf_far*2) - nNf_near);
	if(pct < 0.0f){
		return 0.0f;
	} else if(pct > 1.0f){
		return 1.0f;
	}

	return pct;
}

// fogging stuff --------------------------------------------------------------------

// do a pre-render of the background nebula
#define ESIZE					32
ubyte tpixels[ESIZE * ESIZE * 4];		// for 32 bits
int last_esize = -1;
int this_esize = ESIZE;
extern float Viewer_zoom;
float ex_scale, ey_scale;
int tbmap = -1;
// UnknownPlayer : Contained herein, the origins of the nebula rendering bug!
// I am really not entirely sure what this code achieves, but the old
// D3D calls were the cause of the nebula bug - they have been commented out.
// If you want to save some rendering time, I would suggest maybe kill this off.
// It doesn't use much, but it APPEARS to be fairly useless unless someone wants
// to enlighten me.
//
void neb2_pre_render(vec3d *eye_pos, matrix *eye_orient)
{
	// if the mission is not a fullneb mission, skip
	if (!(The_mission.flags & MISSION_FLAG_FULLNEB))
		return;

	// bail early in lame and poly modes
	if (Neb2_render_mode != NEB2_RENDER_POF)
		return;


	// set the view clip
	gr_screen.clip_width = this_esize;
	gr_screen.clip_height = this_esize;
	g3_start_frame(1);							// Turn on zbuffering
	g3_set_view_matrix(eye_pos, eye_orient, Viewer_zoom);
	gr_set_clip(0, 0, this_esize, this_esize);		

	// render the background properly
	// hack - turn off nebula stuff
	int neb_save = Neb2_render_mode;
	Neb2_render_mode = NEB2_RENDER_NONE;

	// draw background stuff nebula			
	stars_draw_background(0);		

	Neb2_render_mode = neb_save;
	
	// grab the region
	gr_get_region(0, this_esize, this_esize, (ubyte*)tpixels);	

#ifdef NEB2_THUMBNAIL
	if(tbmap == -1){
		tbmap = bm_create(16, this_esize, this_esize, tpixels, 0);
		bm_lock(tbmap, 16, 0);
		bm_unlock(tbmap);
	}
#endif

	// maybe do some swizzling
	
	// end the frame
	g3_end_frame();
	
	gr_clear();	

	// if the size has changed between frames, make a new bitmap
	if(this_esize != last_esize){
		last_esize = this_esize;						
		
		// recalculate ex_scale and ey_scale values for looking up color values
		ex_scale = (float)this_esize / (float)gr_screen.max_w;
		ey_scale = (float)this_esize / (float)gr_screen.max_h;
	}	
}

// wacky scheme for smoothing colors
int wacky_scheme = 3;

// get the color of the pixel in the small pre-rendered background nebula
#define PIXEL_INDEX_SMALL(xv, yv)	( (this_esize * (yv) * gr_screen.bytes_per_pixel) + ((xv) * gr_screen.bytes_per_pixel) )
void neb2_get_pixel(int x, int y, int *r, int *g, int *b)
{
	// we shouldn't ever be here if in htl rendering mode
	Assert( Neb2_render_mode != NEB2_RENDER_HTL );

	int ra, ga, ba;
	ubyte rv, gv, bv;	
	ubyte avg_count;
	int xs, ys;

	// if we're in lame rendering mode, return a constant value
	if (Neb2_render_mode == NEB2_RENDER_LAME) {
		*r = Neb2_background_color[0];
		*g = Neb2_background_color[1];
		*b = Neb2_background_color[2];

		return;
	}		
	
	// get the proper pixel index to be looking up
	rv = gv = bv = 0;	
	
	// select screen format
	BM_SELECT_SCREEN_FORMAT();

	// pixel plus all immediate neighbors (on the small screen - should be more effective than 2 or 1)	
	xs = (int)(ex_scale * x);
	ys = (int)(ey_scale * y);		

	// sometimes goes over by 1 in direct3d
	if(ys >= (this_esize - 1)){
		ys--;
	}

	avg_count = 0;
	bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs, ys)], &rv, &gv, &bv, NULL);
	ra = rv;
	ga = gv;
	ba = bv;
	avg_count++;
	if(xs > 0){			
		bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs - 1, ys)], &rv, &gv, &bv, NULL);	// left
		ra += rv;
		ba += bv;
		ga += gv;
		avg_count++;
	}
	if(xs < this_esize - 1){			
		bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs + 1, ys)], &rv, &gv, &bv, NULL);	// right
		ra += rv;
		ba += bv;
		ga += gv;
		avg_count++;
	}
	if(ys > 0){			
		bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs, ys - 1)], &rv, &gv, &bv, NULL);	// top
		ra += rv;
		ba += bv;
		ga += gv;
		avg_count++;
	}				
	if(ys < this_esize - 2){			
		bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs, ys + 1)], &rv, &gv, &bv, NULL);	// bottom
		ra += rv;
		ba += bv;
		ga += gv;
		avg_count++;
	}
		
	if((xs > 0) && (ys > 0)){			
		bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs - 1, ys - 1)], &rv, &gv, &bv, NULL);	// upper left
		ra += rv;
		ba += bv;
		ga += gv;
		avg_count++;
	}
	if((xs < this_esize - 1) && (ys < this_esize - 1)){			
		bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs + 1, ys + 1)], &rv, &gv, &bv, NULL);	// lower right
		ra += rv;
		ba += bv;
		ga += gv;
		avg_count++;
	}
	if((ys > 0) && (xs < this_esize - 1)){			
		bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs + 1, ys - 1)], &rv, &gv, &bv, NULL);	// lower left
		ra += rv;
		ba += bv;
		ga += gv;
		avg_count++;
	}
	if((ys < this_esize - 1) && (xs > 0)){			
		bm_get_components(&tpixels[PIXEL_INDEX_SMALL(xs - 1, ys + 1)], &rv, &gv, &bv, NULL);	// upper right
		ra += rv;
		ba += bv;
		ga += gv;
		avg_count++;
	}		

	rv = (ubyte)(ra / (int)avg_count);
	gv = (ubyte)(ga / (int)avg_count);
	bv = (ubyte)(ba / (int)avg_count);

	// return values
	*r = (int)rv;
	*g = (int)gv;
	*b = (int)bv;
}

// get the color to fog the background color to
void neb2_get_backg_color(int *r, int *g, int *b)
{
	*r = Neb2_background_color[0];
	*g = Neb2_background_color[1];
	*b = Neb2_background_color[2];
}

// set the background color
void neb2_set_backg_color(int r, int g, int b)
{
	Neb2_background_color[0] = r;
	Neb2_background_color[1] = g;
	Neb2_background_color[2] = b;
}

// fill in the position of the eye for this frame
void neb2_get_eye_pos(vec3d *eye)
{
	*eye = Eye_position;
}

// fill in the eye orient for this frame
void neb2_get_eye_orient(matrix *eye)
{
	*eye = Eye_matrix;
}

// get a (semi) random bitmap to use for a poof
int neb2_get_bitmap()
{
	int count = 0;
	int huh;
	static int neb2_choose = 0;

	// get a random count
	count = (int)frand_range(1.0f, 5.0f);

	// very ad-hoc
	while(count > 0){
		// don't cycle too many times
		huh = 0;		
		do {
			if(neb2_choose == MAX_NEB2_POOFS - 1){
				neb2_choose = 0;
			} else {
				neb2_choose++;
			}

			huh++;
		} while(!(Neb2_poof_flags & (1<<neb2_choose)) && (huh < 10));

		count--;
	}

	// bitmap 0
	if(Neb2_poofs[neb2_choose] < 0){	
		return Neb2_poofs[0];
	} 
	return Neb2_poofs[neb2_choose];
}

// nebula DCF functions ------------------------------------------------------

DCF(neb2, "list nebula console commands")
{		
//	dc_printf("neb2_fog <X> <float> <float>  : set near and far fog planes for ship type X\n");
//	dc_printf("where X is an integer from 1 - 11\n");
//	dc_printf("1 = cargo containers, 2 = fighters/bombers, 3 = cruisers\n");
//	dc_printf("4 = freighters, 5 = capital ships, 6 = transports, 7 = support ships\n");
//	dc_printf("8 = navbuoys, 9 = sentryguns, 10 = escape pods, 11 = background nebula polygons\n\n");
	
	dc_printf("neb2_max_alpha   : max alpha value (0.0 to 1.0) for cloud poofs. 0.0 is completely transparent\n");
	dc_printf("neb2_break_alpha : alpha value (0.0 to 1.0) at which faded polygons are not drawn. higher values generally equals higher framerate, with more visual cloud popping\n");
	dc_printf("neb2_break_off   : how many pixels offscreen (left, right, top, bottom) when a cloud poof becomes fully transparent. Lower values cause quicker fading\n");
	dc_printf("neb2_smooth      : magic fog smoothing modes (0 - 3)\n");
	dc_printf("neb2_select      : <int> <int>  where the first # is the bitmap to be adjusting (0 through 5), and the second int is a 0 or 1, to turn off and on\n");
	dc_printf("neb2_rot         : set max rotation speed for poofs\n");
	dc_printf("neb2_prad        : set cloud poof radius\n");
	dc_printf("neb2_cdim        : poof cube dimension\n");
	dc_printf("neb2_cinner      : poof cube inner dimension\n");
	dc_printf("neb2_couter      : poof cube outer dimension\n");
	dc_printf("neb2_jitter      : poof jitter\n");
	dc_printf("neb2_mode        : switch between no nebula, polygon background, pof background, lame or HTL rendering (0, 1, 2, 3 and 4 respectively)\n\n");	
	dc_printf("neb2_ff          : flash fade/sec\n");
	dc_printf("neb2_background	: rgb background color\n");
	dc_printf("neb2_fog_color   : rgb fog color\n");

//	dc_printf("neb2_fog_vals    : display all the current settings for all above values\n");	
}

DCF(neb2_prad, "")
{
	dc_get_arg(ARG_FLOAT);
	Nd->prad = Dc_arg_float;
}
DCF(neb2_cdim, "")
{
	dc_get_arg(ARG_FLOAT);
	Nd->cube_dim = Dc_arg_float;
}

DCF(neb2_cinner, "")
{
	dc_get_arg(ARG_FLOAT);
	Nd->cube_inner = Dc_arg_float;
}

DCF(neb2_couter, "")
{
	dc_get_arg(ARG_FLOAT);
	Nd->cube_outer = Dc_arg_float;
}

DCF(neb2_jitter, "")
{
	dc_get_arg(ARG_FLOAT);
	Nd->hj = Nd->dj = Nd->wj = Dc_arg_float;
}
//WMC - unfortunately, this has to go bye-bye
//I don't think anyone used it anyway so I'm not going to try to resurrect it
/*
DCF(neb2_fog, "")
{
	int index;
	float fnear, ffar;
	dc_get_arg(ARG_INT);
	index = Dc_arg_int;
	dc_get_arg(ARG_FLOAT);
	fnear = Dc_arg_float;
	dc_get_arg(ARG_FLOAT);
	ffar = Dc_arg_float;

	if((index >= 1) && (index <= 11) && (fnear >= 0.0f) && (ffar >= 0.0f) && (ffar > fnear)){
		if(index == 11){
			Neb_backg_fog_near = fnear;
			Neb_backg_fog_far = ffar;
		} else {
			Neb_ship_fog_vals_d3d[index][0] = fnear;
			Neb_ship_fog_vals_d3d[index][1] = ffar;
		}
	}
}*/

DCF(neb2_max_alpha, "")
{
	dc_get_arg(ARG_FLOAT);
	Nd->max_alpha_glide = Dc_arg_float;
}

DCF(neb2_break_alpha, "")
{
	dc_get_arg(ARG_FLOAT);
	Nd->break_alpha = Dc_arg_float;
}

DCF(neb2_break_off, "")
{
	dc_get_arg(ARG_INT);
	Nd->break_y = (float)Dc_arg_int;
	Nd->break_x = Nd->break_y * 1.3333f;
}

DCF(neb2_smooth, "")
{
	int index;
	dc_get_arg(ARG_INT);
	index = Dc_arg_int;
	if((index >= 0) && (index <= 3)){
		wacky_scheme = index;
	}
}

DCF(neb2_select, "")
{
	dc_get_arg(ARG_INT);	
	int bmap = Dc_arg_int;
	if((bmap >= 0) && (bmap < Neb2_poof_count)){
		dc_get_arg(ARG_INT);

		if(Dc_arg_int){
			Neb2_poof_flags |= (1<<bmap);
		} else {
			Neb2_poof_flags &= ~(1<<bmap);
		}
	}
}

DCF(neb2_rot, "")
{
	dc_get_arg(ARG_FLOAT);
	max_rotation = Dc_arg_float;
}

DCF(neb2_ff, "")
{
	dc_get_arg(ARG_FLOAT);
	neb2_flash_fade = Dc_arg_float;
}

DCF(neb2_mode, "")
{
	dc_get_arg(ARG_INT);

	switch (Dc_arg_int)
	{
		case NEB2_RENDER_NONE:
			Neb2_render_mode = NEB2_RENDER_NONE;
			break;

		case NEB2_RENDER_POLY:
			Neb2_render_mode = NEB2_RENDER_POLY;
			break;

		case NEB2_RENDER_POF:
			Neb2_render_mode = NEB2_RENDER_POF;
			stars_set_background_model(BACKGROUND_MODEL_FILENAME, "Eraseme3");
			break;

		case NEB2_RENDER_LAME:
			Neb2_render_mode = NEB2_RENDER_LAME;
			break;

		case NEB2_RENDER_HTL:
			if (!Cmdline_nohtl) {
				Neb2_render_mode = NEB2_RENDER_HTL;
			}
			break;
	}
}

DCF(neb2_slices, "")
{
	dc_get_arg(ARG_INT);
	Neb2_slices = Dc_arg_int;
	Neb2_regen = 1;
}

DCF(neb2_background, "")
{
	int r, g, b;

	dc_get_arg(ARG_INT);
	r = Dc_arg_int;
	dc_get_arg(ARG_INT);
	g = Dc_arg_int;
	dc_get_arg(ARG_INT);
	b = Dc_arg_int;

	Neb2_background_color[0] = r;
	Neb2_background_color[1] = g;
	Neb2_background_color[2] = b;
}

DCF(neb2_fog_color, "")
{
	int r, g, b;

	dc_get_arg(ARG_INT);
	r = Dc_arg_int;
	dc_get_arg(ARG_INT);
	g = Dc_arg_int;
	dc_get_arg(ARG_INT);
	b = Dc_arg_int;

	Neb2_fog_color_r = r;
	Neb2_fog_color_g = g;
	Neb2_fog_color_b = b;
}

//WMC - Going bye-bye for ship types too
/*
DCF(neb2_fog_vals, "")
{
	dc_printf("neb2_fog : \n");
	{
		dc_printf("(1)cargo containers : %f, %f\n", Neb_ship_fog_vals_d3d[1][0], Neb_ship_fog_vals_d3d[1][1]);
		dc_printf("(2)fighters/bombers : %f, %f\n", Neb_ship_fog_vals_d3d[2][0], Neb_ship_fog_vals_d3d[2][1]);
		dc_printf("(3)cruisers : %f, %f\n", Neb_ship_fog_vals_d3d[3][0], Neb_ship_fog_vals_d3d[3][1]);
		dc_printf("(4)freighters : %f, %f\n", Neb_ship_fog_vals_d3d[4][0], Neb_ship_fog_vals_d3d[4][1]);
		dc_printf("(5)cap ships : %f, %f\n", Neb_ship_fog_vals_d3d[5][0], Neb_ship_fog_vals_d3d[5][1]);
		dc_printf("(6)transports : %f, %f\n", Neb_ship_fog_vals_d3d[6][0], Neb_ship_fog_vals_d3d[6][1]);
		dc_printf("(7)support ships : %f, %f\n", Neb_ship_fog_vals_d3d[7][0], Neb_ship_fog_vals_d3d[7][1]);
		dc_printf("(8)navbuoys : %f, %f\n", Neb_ship_fog_vals_d3d[8][0], Neb_ship_fog_vals_d3d[8][1]);
		dc_printf("(9)sentry guns : %f, %f\n", Neb_ship_fog_vals_d3d[9][0], Neb_ship_fog_vals_d3d[9][1]);
		dc_printf("(10)escape pods : %f, %f\n", Neb_ship_fog_vals_d3d[10][0], Neb_ship_fog_vals_d3d[10][1]);
		dc_printf("(11)background polys : %f, %f\n\n", Neb_backg_fog_near, Neb_backg_fog_far);		
	}
	dc_printf("neb2_max_alpha   : %f\n", Nd->max_alpha_glide);
	dc_printf("neb2_break_alpha : %f\n", Nd->break_alpha);
	dc_printf("neb2_break_off   : %d\n", (int)Nd->break_y);
	dc_printf("neb2_smooth      : %d\n", wacky_scheme);
	dc_printf("neb2_toggle      : %s\n", Neb2_render_mode ? "on" : "off");
	dc_printf("neb2_rot			 : %f\n", max_rotation);
	dc_printf("neb2_prad			 : %f\n", Nd->prad);
	dc_printf("neb2_cdim			 : %f\n", Nd->cube_dim);
	dc_printf("neb2_couter      : %f\n", Nd->cube_outer);
	dc_printf("neb2_cinner		 : %f\n", Nd->cube_inner);
	dc_printf("neb2_jitter		 : %f\n", Nd->wj);
	dc_printf("neb2_ff			 : %f\n", neb2_flash_fade);
	dc_printf("neb2_background	 : %d %d %d\n", Neb2_background_color[0], Neb2_background_color[1], Neb2_background_color[2]);
}*/

/* Obsolete !?
DCF(neb2_create, "create a basic nebula")
{
	int points = 0;
	float rad1 = 0.0f;
	float rad2 = 0.0f;
	
	dc_get_arg(ARG_INT);
	if(Dc_arg_type & ARG_INT){
		points = Dc_arg_int;
	}
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		rad1 = Dc_arg_float;
	}
	dc_get_arg(ARG_FLOAT);
	if(Dc_arg_type & ARG_FLOAT){
		rad2 = Dc_arg_float;
	}
	neb2_create(&vmd_zero_vector, points, rad1, -1.0f, rad2);
}

DCF(neb2_del, "delete existing nebulae")
{
	for(int idx=0; idx<MAX_OBJECTS; idx++){
		if(Objects[idx].type == OBJ_NEB2){
			obj_delete(idx);			
		}
	}
}

int magic0 = 15;
float magic1 = 1000.0f;
float magic2 = -1.0f;
float magic3 = 700.0f;
DCF(neb2_def, "create a default nebula")
{
	vec3d a,b,c,d,e,f;
	vm_vec_make(&a, 0.0f, 0.0f, 0.0f);
	vm_vec_make(&b, 3600.0f, 700.0f, 0.0f);
	vm_vec_make(&c, -3000.0f, 20.0f, 480.0f);
	vm_vec_make(&d, -4000.0f, 100.0f, 100.0f);
	vm_vec_make(&e, 0.0f, 3000.0f, -400.0f);
	vm_vec_make(&f, 670.0f, -2500.0f, -1600.0f);

	neb2_create(&a, magic0, magic1, magic2, magic3);
	neb2_create(&b, magic0, magic1, magic2, magic3);
	neb2_create(&c, magic0, magic1, magic2, magic3);
	neb2_create(&d, magic0, magic1, magic2, magic3);
	neb2_create(&e, magic0, magic1, magic2, magic3);
	neb2_create(&f, magic0, magic1, magic2, magic3);
}

DCF(neb2_plr, "regenerate the player's nebula")
{
	Neb2_regen = 0;
}

DCF(neb2_stats, "display info about the nebula rendering")
{
	dc_printf("Player poofs tried : %d\n", pneb_tried);
	dc_printf("Player poofs tossed (alpha): %d\n", pneb_tossed_alpha);
	dc_printf("Player poofs tossed (dot): %d\n", pneb_tossed_dot);
	dc_printf("Player poofs tossed (off): %d\n", pneb_tossed_off);

	dc_printf("Poofs tried : %d\n", neb_tried);
	dc_printf("Poofs tossed (alpha): %d\n", neb_tossed_alpha);
	dc_printf("Poofs tossed (dot): %d\n", neb_tossed_dot);
	dc_printf("Poofs tossed (count): %d\n", neb_tossed_count);

	dc_printf("Avg poofs/frame: %f\n", frame_avg);
}

// create a nebula object, return objnum of the nebula or -1 on fail
// NOTE : in most cases you will want to pass -1.0f for outer_radius. Trust me on this
int neb2_create(vec3d *offset, int num_poofs, float inner_radius, float outer_radius, float max_poof_radius)
{	
	Int3();
	return -1;
}

// delete a nebula object
void neb2_delete(object *objp)
{	
	Int3();
}

// renders a nebula object
void neb2_render(object *objp)
{	
	Int3();
}

// preprocess the nebula object before simulation
void neb2_process_pre(object *objp)
{
	Int3();
}

// process the nebula object after simulating, but before rendering
void neb2_process_post(object *objp)
{	
	Int3();
}
*/

/*
// add N poofs to the inner shell of the nebula
// if orient and ang are specified, generate the poofs so that they are "visible" around
// the orient fvec in a cone of ang degrees
void neb2_add_inner(neb2 *neb, int num_poofs, matrix *orient, float ang)
{	
	int idx;
	vec3d pt, pt2, pt3;
	int final_index = (neb->num_poofs + num_poofs) > neb->max_poofs ? neb->max_poofs : (neb->num_poofs + num_poofs);

	// add the points a pick a random bitmap
	for(idx=neb->num_poofs; idx<final_index; idx++){
		if(orient != NULL){
			// put a point directly in front of the player, between 0 and inner_radius distance away
			vm_vec_copy_scale(&pt, &orient->fvec, frand_range(neb->magic_num, neb->inner_radius));

			// rotate the point by -ang <-> ang around the up vector
			vm_rot_point_around_line(&pt2, &pt, fl_radian(frand_range(-ang, ang)), &vmd_zero_vector, &orient->uvec);

			// rotate the point by -ang <-> ang around the right vector
			vm_rot_point_around_line(&pt3, &pt2, fl_radian(frand_range(-ang, ang)), &vmd_zero_vector, &orient->rvec);

			// now add in the center of the nebula so its placed properly (ie, not around the origin)
			vm_vec_add(&neb->pts[idx], &pt3, &Objects[neb->objnum].pos);
		} else {
			neb->pts[idx].xyz.x = frand_range(-1.0f * neb->inner_radius, neb->inner_radius) + Objects[neb->objnum].pos.xyz.x;
			neb->pts[idx].xyz.y = frand_range(-1.0f * neb->inner_radius, neb->inner_radius) + Objects[neb->objnum].pos.xyz.y;
			neb->pts[idx].xyz.z = frand_range(-1.0f * neb->inner_radius, neb->inner_radius) + Objects[neb->objnum].pos.xyz.z;
		}

		neb->bmaps[idx] = (int)frand_range(0.0f, (float)2);
		neb->num_poofs++;
	}
}

// add N poofs to the outer shell of the nebula
// if orient and ang are specified, generate the poofs so that they are "visible" around
// the orient fvec in a cone of ang degrees
void neb2_add_outer(neb2 *neb, int num_poofs, matrix *orient, float ang)
{
	int idx;
	float phi, theta;
	vec3d pt, pt2, pt3;
	int final_index = (neb->num_poofs + num_poofs) > neb->max_poofs ? neb->max_poofs : (neb->num_poofs + num_poofs);

	// add the points a pick a random bitmap
	for(idx=neb->num_poofs; idx<final_index; idx++){
		if(orient != NULL){
			// put a point directly in front of the player, at outer_radius distance away
			vm_vec_copy_scale(&pt, &orient->fvec, neb->outer_radius);

			// rotate the point by -ang <-> ang around the up vector
			vm_rot_point_around_line(&pt2, &pt, fl_radian(frand_range(-ang, ang)), &vmd_zero_vector, &orient->uvec);

			// rotate the point by -ang <-> ang around the right vector
			vm_rot_point_around_line(&pt3, &pt2, fl_radian(frand_range(-ang, ang)), &vmd_zero_vector, &orient->rvec);

			// now add in the center of the nebula so its placed properly (ie, not around the origin)
			vm_vec_add(&neb->pts[idx], &pt, &Objects[neb->objnum].pos);
		} else {
			// get a random point on the very outer radius, using spherical coords
			phi = fl_radian(frand_range(0.0f, 360.0f));
			theta = fl_radian(frand_range(0.0f, 360.f));
	
			neb->pts[idx].xyz.x = neb->outer_radius * (float)sin(phi) * (float)cos(theta);
			neb->pts[idx].xyz.y = neb->outer_radius * (float)sin(phi) * (float)sin(theta);
			neb->pts[idx].xyz.z = neb->outer_radius * (float)cos(phi);			
		}

		// pick a random bitmap and increment the # of poofs
		neb->bmaps[idx] = (int)frand_range(0.0f, (float)2);
		neb->num_poofs++;
	}
}

// return the alpha the passed poof should be rendered with, for a 1 shell nebula
float neb2_get_alpha_1shell(neb2 *neb, int poof_index)
{
	float dist;
	float alpha;
	vec3d eye_pos;

	// get the eye position
	neb2_get_eye_pos(&eye_pos);
	
	// determine what alpha to draw this bitmap with
	// higher alpha the closer the bitmap gets to the eye
	dist = vm_vec_dist_quick(&eye_pos, &neb->pts[poof_index]);
	
	// at poof radius or greater, alpha should be 1.0
	// scale from 0.0 to 1.0 between radius and magic
	if(dist >= neb->max_poof_radius){
		return max_alpha - 0.0001f;
	} else if(dist > neb->magic_num){
		// alpha per meter between the magic # and the max radius
		alpha = max_alpha / (neb->max_poof_radius - neb->magic_num);

		// above value times the # of meters away we are
		return alpha * (dist - neb->magic_num);
	} 	
	
	// otherwise transparent
	return 0.0f;
}
*/
