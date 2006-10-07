/*
 * $Logfile: /Freespace2/code/graphics/generic.cpp $
 * $Revision: 1.2 $
 * $Date: 2006-10-07 02:43:44 $
 * $Author: Goober5000 $
 *
 * Generic graphics functions
 *
 * $Log: not supported by cvs2svn $
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
	if (filename == NULL)
		ga->filename[0] = '\0';
	else
		strcpy(ga->filename, filename);
	
	ga->first_frame = -1;
	ga->num_frames = 0;
}

// Goober5000
void generic_bitmap_init(generic_bitmap *gb, char *filename)
{
	if (filename == NULL)
		gb->filename[0] = '\0';
	else
		strcpy(gb->filename, filename);
	
	gb->bitmap = -1;
}

// Goober5000
// load a generic_anim
// return 0 is successful, otherwise return -1
int generic_anim_load(generic_anim *ga)
{
	int		fps;
	
	if ( !strlen(ga->filename) )
		return -1;
	
	ga->first_frame = bm_load_animation(ga->filename, &ga->num_frames, &fps);
	if ( ga->first_frame < 0)
	{
		Warning(LOCATION, "Couldn't load animation %s", ga->filename);
		return -1;
	}
	
	Assert(fps != 0);
	ga->total_time = (int) i2fl(ga->num_frames)/fps;
	
	return 0;
}

int generic_bitmap_load(generic_bitmap *gb)
{
	if ( !strlen(gb->filename) || !stricmp(gb->filename, "none") )
		return -1;
	
	gb->bitmap = bm_load(gb->filename);
	
	if ( gb->bitmap < 0 ) {
		Warning(LOCATION, "Couldn't load bitmap %s", gb->filename);
		return -1;
	}
	
	return 0;
}
