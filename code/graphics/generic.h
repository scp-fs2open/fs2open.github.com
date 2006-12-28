/*
 * $Logfile: /Freespace2/code/graphics/generic.h $
 * $Revision: 1.2 $
 * $Date: 2006-12-28 00:59:26 $
 * $Author: wmcoolmon $
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

#ifndef _GENERIC_H_
#define _GENERIC_H_

#include "globalincs/pstypes.h"


// Goober5000
typedef struct generic_anim {
	char filename[MAX_FILENAME_LEN];
	int	first_frame;
	int	num_frames;
	int total_time;		// in seconds

	generic_anim(){memset(this, 0, sizeof(generic_anim)); first_frame=-1;}
} generic_anim;

// Goober5000
typedef struct generic_bitmap {
	char filename[MAX_FILENAME_LEN];
	int bitmap;
} generic_bitmap;


void generic_anim_init(generic_anim *ga, char *filename = NULL);
void generic_bitmap_init(generic_bitmap *gb, char *filename = NULL);
int generic_anim_load(generic_anim *ga);
int generic_bitmap_load(generic_bitmap *gb);

#endif
