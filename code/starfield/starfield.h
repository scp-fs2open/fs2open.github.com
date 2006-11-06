/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Starfield/StarField.h $
 * $Revision: 2.18 $
 * $Date: 2006-11-06 06:46:08 $
 * $Author: taylor $
 *
 * Code to handle and draw starfields, background space image bitmaps, floating
 * debris, etc.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.17  2006/08/06 18:47:29  Goober5000
 * add the multiple background feature
 * --Goober5000
 *
 * Revision 2.16  2006/04/20 06:32:30  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 2.15  2006/02/20 07:30:15  taylor
 * updated to newest dynamic starfield code
 *  - this mainly is to just better support SEXP based starfield bitmap changes (preloading, better in-mission stuff loading)
 *  - also fixes index_buffer related double-free()
 *  - done waste memory for env index buffer if env is not enabled
 *  - address a couple of bm load/release issues and comment a little to tell why
 *
 * Revision 2.14  2006/01/30 06:31:30  taylor
 * dynamic starfield bitmaps (if the thought it was freaky before, just take a look at the new and "improved" version ;))
 *
 * Revision 2.13  2005/10/09 08:03:21  wmcoolmon
 * New SEXP stuff
 *
 * Revision 2.12  2005/07/13 03:35:34  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.11  2005/04/05 05:53:25  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.10  2005/03/20 20:02:29  phreak
 * export the functions that deal with the creation and destruction of the starfield buffer.
 * FRED needs them
 *
 * Revision 2.9  2004/08/11 05:06:35  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.8  2004/07/01 01:12:33  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.7  2004/03/17 04:07:32  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.6  2004/03/05 09:02:07  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.5  2003/09/10 11:38:31  fryday
 * Added Lens Flares, enabled by optional table entry in stars.tbl on a per-sun basis
 *
 * Revision 2.4  2003/08/31 06:00:41  bobboau
 * an asortment of bugfixes, mostly with the specular code,
 * HUD flickering should be completly gone now
 *
 * Revision 2.3  2003/08/16 03:52:24  bobboau
 * update for the specmapping code includeing
 * suport for seperate specular levels on lights and
 * optional strings for the stars table
 * code has been made more organised,
 * though there seems to be a bug in the state selecting code
 * resulting in the HUD being rendered incorectly
 * and specmapping failing ocasionaly
 *
 * Revision 2.2  2002/09/20 20:04:54  phreak
 * added glare variable for ambient suns
 * if glare is 0 then the sun glare whiteout is not shown when looking at the sun
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 11    7/21/99 8:10p Dave
 * First run of supernova effect.
 * 
 * 10    6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 9     5/09/99 6:00p Dave
 * Lots of cool new effects. E3 build tweaks.
 * 
 * 8     5/07/99 1:59p Johnson
 * Tweaked background bitmaps a bit.
 * 
 * 7     4/23/99 5:53p Dave
 * Started putting in new pof nebula support into Fred.
 * 
 * 6     4/07/99 6:22p Dave
 * Fred and FreeSpace support for multiple background bitmaps and suns.
 * Fixed link errors on all subprojects. Moved encrypt_init() to
 * cfile_init() and lcl_init(), since its safe to call twice.
 * 
 * 5     3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 4     11/14/98 5:33p Dave
 * Lots of nebula work. Put in ship contrails.
 * 
 * 3     11/05/98 5:55p Dave
 * Big pass at reducing #includes
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 21    4/11/98 6:53p John
 * Added first rev of subspace effect.
 * 
 * 20    4/07/98 4:17p John
 * Made Fred be able to move suns.  Made suns actually affect the lighting
 * in the game.
 * 
 * 19    3/21/98 7:36p Lawrance
 * Move jump nodes to own lib.
 * 
 * 18    3/19/98 12:29p Lawrance
 * Fix jumpnode targeting bug on the target monitor
 * 
 * 17    3/15/98 3:41p Allender
 * new sexpression to gauge whether a ship warped out in proximity of jump
 * node
 * 
 * 16    3/11/98 5:33p Lawrance
 * Support rendering and targeting of jump nodes
 * 
 * 15    3/10/98 4:26p Hoffoss
 * Changed jump node structure to include a name.  Position is now taken
 * from the object (each jump node has an associated object now).
 * 
 * 14    3/06/98 5:30p Hoffoss
 * Added jump node rendering code to FreeSpace.
 * 
 * 13    2/26/98 10:08p Hoffoss
 * Rewrote state saving and restoring to fix bugs and simplify the code.
 * 
 * 12    2/06/98 3:08p Mike
 * More asteroid stuff, including resolving conflicts between the two
 * asteroid_field structs!
 * 
 * 11    1/18/98 4:24p John
 * Detect when view camera "cuts" and tell the star code about it so it
 * won't blur stars and debris between the last frame and this frame.
 * 
 * 10    8/25/97 5:56p Hoffoss
 * Added multiple asteroid field support, loading and saving of asteroid
 * fields, and ship score field to Fred.
 * 
 * 9     8/05/97 10:18a Lawrance
 * my_rand() being used temporarily instead of rand()
 * 
 * 8     2/17/97 3:06p Hoffoss
 * Changed header description.
 * 
 * 7     2/14/97 3:29p Hoffoss
 * Added header for MSDEV to fill in.
 *
 * $NoKeywords: $
 */

#ifndef _STARFIELD_H
#define _STARFIELD_H

#include <vector>

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"

#define MAX_STARFIELD_BITMAP_LISTS	1
#define MAX_ASTEROID_FIELDS			4

// nice low polygon background
#define BACKGROUND_MODEL_FILENAME					"spherec.pof"


// starfield list
typedef struct starfield_list_entry {
	char filename[MAX_FILENAME_LEN];				// bitmap filename
	float scale_x, scale_y;							// x and y scale
	int div_x, div_y;								// # of x and y divisions
	angles ang;										// angles from FRED
} starfield_list_entry;

// backgrounds
typedef struct background_t {
	std::vector<starfield_list_entry> bitmaps;
	std::vector<starfield_list_entry> suns;
} background_t;

#define MAX_BACKGROUNDS	2

extern int Num_backgrounds;
extern int Cur_background;
extern background_t Backgrounds[MAX_BACKGROUNDS];

void stars_swap_backgrounds(int idx1, int idx2);
void stars_pack_backgrounds();
bool stars_background_empty();


// add a new sun or bitmap instance
int stars_add_sun_entry(starfield_list_entry *sun);
int stars_add_bitmap_entry(starfield_list_entry *bitmap);

// get the number of entries that each vector contains
// "sun" will get sun instance counts, otherwise it gets normal starfield bitmap instance counts
// "bitmap_count" will get number of starfield_bitmap entries rather than starfield_bitmap_instance entries
int stars_get_num_entries(bool sun, bool bitmap_count);

// macros to get the number of sun or starfield bitmap *instances* available
#define stars_get_num_bitmaps()	stars_get_num_entries(false, false)
#define stars_get_num_suns()	stars_get_num_entries(true, false)

// make a bitmap or sun instance as unusable (doesn't free anything but does prevent rendering)
void stars_mark_instance_unused(int index, bool sun);
// macros to easily mark a sun or bitmap as unused
#define stars_mark_sun_unused(x)	stars_mark_instance_unused((x),true)
#define stars_mark_bitmap_unused(x)	stars_mark_instance_unused((x),false)

// get a name from an instance index
const char *stars_get_name_from_instance(int index, bool sun);
// macros to easily get a sun or a bitmap name
#define stars_get_sun_name(x)		stars_get_name_from_instance((x),true)
#define stars_get_bitmap_name(x)	stars_get_name_from_instance((x),false)

extern const int MAX_STARS;
extern int Num_stars;

// call on game startup
void stars_init();
// call on game shutdown
void stars_close();

// call this before mission parse to reset all data to a sane state
void stars_pre_level_init(bool clear_backgrounds = true);

// call this in game_post_level_init() so we know whether we're running in full nebula mode or not
void stars_post_level_init();

// draw background bitmaps
void stars_draw_background();

// This *must* be called to initialize the lighting.
// You can turn off all the stars and suns and nebulas, though.
void stars_draw(int show_stars, int show_suns, int show_nebulas, int show_subspace, int env);
// void calculate_bitmap_matrix(starfield_bitmaps *bm, vec3d *v);
// void calculate_bitmap_points(starfield_bitmaps *bm, float bank = 0.0f);

// draw the corresponding glow for sun_n
void stars_draw_sun_glow(int sun_n);

// Call when the viewer camera "cuts" so stars and debris
// don't draw incorrect blurs between last frame and this frame.
void stars_camera_cut();

// call this to set a specific model as the background model
void stars_set_background_model(char *model_name, char *texture_name);

// lookup a starfield bitmap, return index or -1 on fail
int stars_find_bitmap(char *name);

// lookup a sun by bitmap filename, return index or -1 on fail
int stars_find_sun(char *name);

// get the world coords of the sun pos on the unit sphere.
void stars_get_sun_pos(int sun_n, vec3d *pos);

// for SEXP stuff so that we can mark a bitmap as being used regardless of whether 
// or not there is an instance for it yet
void stars_preload_sun_bitmap(char *fname);
void stars_preload_background_bitmap(char *fname);

void stars_set_nebula(bool activate);

// Starfield functions that should be used only by FRED ...

// get a name based on the index into starfield_bitmap, only FRED should ever need this
const char *stars_get_name_FRED(int index, bool sun);
// erase an instance, note that this is very slow so it should only be done in FRED
void stars_delete_entry_FRED(int index, bool sun);
// modify an existing starfield bitmap instance, or add a new one if needed
void stars_modify_entry_FRED(int index, const char *name, starfield_list_entry *sbi_new, bool sun);


// Goober5000
void stars_load_first_valid_background();

#endif
