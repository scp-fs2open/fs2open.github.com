

#ifndef _GENERIC_H_
#define _GENERIC_H_

#include "globalincs/pstypes.h"
#include "cfile/cfile.h"
#include "anim/animplay.h"

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
	int keyoffset;
	int current_frame;
	int previous_frame;
	unsigned char direction;
	unsigned char done_playing;
	float total_time;		// in seconds
	float anim_time;	// current animation time

	//we only care about the stuff below if we're streaming
	union {
		struct {
			anim *animation;
			anim_instance *instance;
			unsigned char bg_type;	//to store background type to avoid messed up colours
		} ani;
		struct {
			int next_frame;
		} eff;
	};
	ubyte type;
	unsigned char streaming;
	ubyte *buffer;
	int height;
	int width;
	int bitmap_id;
	bool colored;
} generic_anim;

// Goober5000
typedef struct generic_bitmap {
	char filename[MAX_FILENAME_LEN];
	int bitmap_id;
} generic_bitmap;

int generic_anim_init_and_stream(generic_anim *anim, const char *anim_filename, ubyte bg_type, bool attempt_hi_res);
void generic_anim_init(generic_anim *ga);
void generic_anim_init(generic_anim *ga, const char *filename);
void generic_anim_init(generic_anim *ga, const SCP_string& filename);
void generic_bitmap_init(generic_bitmap *gb, const char *filename = NULL);
int generic_anim_load(generic_anim *ga);
int generic_anim_stream(generic_anim *ga);
int generic_bitmap_load(generic_bitmap *gb);
void generic_anim_unload(generic_anim *ga);
void generic_anim_render(generic_anim *ga, float frametime, int x, int y);

#endif
