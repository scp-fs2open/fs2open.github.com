

#ifndef _GENERIC_H_
#define _GENERIC_H_

#include "globalincs/pstypes.h"

#define GENERIC_ANIM_DIRECTION_FORWARDS		0
#define GENERIC_ANIM_DIRECTION_BACKWARDS	1
#define GENERIC_ANIM_DIRECTION_NOLOOP		2
#define GENERIC_ANIM_DIRECTION_PAUSED		4

// Goober5000
typedef struct generic_anim {
	char filename[MAX_FILENAME_LEN];
	int	first_frame;
	int	num_frames;
	int	keyframe;
	int current_frame;
	unsigned char direction;
	unsigned char done_playing;
	float total_time;		// in seconds
	float anim_time;	// current animation time
	
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
void generic_anim_unload(generic_anim *ga);
void generic_anim_render(generic_anim *ga, float frametime, int x, int y);

#endif
