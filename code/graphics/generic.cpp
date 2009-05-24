/*
 * $Logfile: /Freespace2/code/graphics/generic.cpp $
 * $Revision: 1.1.2.3 $
 * $Date: 2007-02-12 00:22:05 $
 * $Author: taylor $
 *
 * Generic graphics functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1.2.2  2006/12/26 05:15:40  taylor
 * support both "none" and "<none>" on filenames to skip (will do this better later, ran out of time)
 *
 * Revision 1.1.2.1  2006/10/07 02:43:46  Goober5000
 * bypass annoying warnings for nonexistent bitmaps
 *
 * Revision 1.1  2005/11/21 23:55:00  taylor
 * add generic.cpp and generic.h
 *
 *
 * $NoKeywords: $
 */


#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "graphics/generic.h"


// Goober5000
void generic_anim_init(generic_anim *ga, char *filename)
{
	if (filename == NULL) {
		ga->filename[0] = '\0';
	} else {
		strncpy(ga->filename, filename, MAX_FILENAME_LEN - 1);
	}

	ga->first_frame = -1;
	ga->num_frames = 0;
	ga->total_time = 1.0f;
}

// Goober5000
void generic_bitmap_init(generic_bitmap *gb, char *filename)
{
	if (filename == NULL) {
		gb->filename[0] = '\0';
	} else {
		strncpy(gb->filename, filename, MAX_FILENAME_LEN - 1);
	}

	gb->bitmap_id = -1;
}

// Goober5000
// load a generic_anim
// return 0 is successful, otherwise return -1
int generic_anim_load(generic_anim *ga)
{
	int		fps;

	if ( !VALID_FNAME(ga->filename) )
		return -1;

	ga->first_frame = bm_load_animation(ga->filename, &ga->num_frames, &fps);

	if (ga->first_frame < 0)
		return -1;

	Assert(fps != 0);
	ga->total_time = ga->num_frames / (float)fps;

	return 0;
}

int generic_bitmap_load(generic_bitmap *gb)
{
	if ( !VALID_FNAME(gb->filename) )
		return -1;

	gb->bitmap_id = bm_load(gb->filename);

	if (gb->bitmap_id < 0)
		return -1;

	return 0;
}
