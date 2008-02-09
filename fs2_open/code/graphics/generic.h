/*
 * $Logfile: /Freespace2/code/graphics/generic.h $
 * $Revision: 1.3 $
 * $Date: 2007-03-22 21:55:01 $
 * $Author: taylor $
 *
 * Generic graphics functions
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/12/28 00:59:26  wmcoolmon
 * WMC codebase commit. See pre-commit build thread for details on changes.
 *
 * Revision 1.1  2005/11/21 23:55:00  taylor
 * add generic.cpp and generic.h
 *
 *
 * $NoKeywords: $
 */

#ifndef _GENERIC_H_
#define _GENERIC_H_

#include "globalincs/pstypes.h"


typedef struct generic_anim {
	char filename[MAX_FILENAME_LEN];
	int	first_frame;
	int	num_frames;
	float total_time;		// in seconds
} generic_anim;

typedef struct generic_bitmap {
	char filename[MAX_FILENAME_LEN];
	int bitmap_id;
} generic_bitmap;


void generic_anim_init(generic_anim *ga, char *filename = NULL);
void generic_bitmap_init(generic_bitmap *gb, char *filename = NULL);
int generic_anim_load(generic_anim *ga);
int generic_bitmap_load(generic_bitmap *gb);

#endif
