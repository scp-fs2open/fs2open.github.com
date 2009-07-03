

#ifndef _GENERIC_H_
#define _GENERIC_H_

#include "globalincs/pstypes.h"


// Goober5000
typedef struct generic_anim {
	char filename[MAX_FILENAME_LEN];
	int	first_frame;
	int	num_frames;
	float total_time;		// in seconds
	
	generic_anim( )
		: first_frame( 0 ), num_frames( 0 ), total_time( 0 )
	{
		filename[ 0 ] = NULL;
	}
} generic_anim;

// Goober5000
typedef struct generic_bitmap {
	char filename[MAX_FILENAME_LEN];
	int bitmap_id;

	generic_bitmap( )
		: bitmap_id( 0 )
	{
		filename[ 0 ] = NULL;
	}
} generic_bitmap;


void generic_anim_init(generic_anim *ga, char *filename = NULL);
void generic_bitmap_init(generic_bitmap *gb, char *filename = NULL);
int generic_anim_load(generic_anim *ga);
int generic_bitmap_load(generic_bitmap *gb);

#endif
