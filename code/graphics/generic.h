

#ifndef _GENERIC_H_
#define _GENERIC_H_

#include "anim/animplay.h"
#include "bmpman/bmpman.h"
#include "cfile/cfile.h"
#include "globalincs/pstypes.h"
#include "pngutils/pngutils.h"

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
			BM_TYPE bg_type;	//to store background type to avoid messed up colours
		} ani;
		struct {
			int next_frame;
		} eff;
		struct {
			apng::apng_ani* anim;
			float previous_frame_time;
		} png;
	};
	BM_TYPE type;
	unsigned char streaming;
	ubyte *buffer;
	int height;
	int width;
	int bitmap_id;
	bool use_hud_color;
} generic_anim;

// Goober5000
typedef struct generic_bitmap {
	char filename[MAX_FILENAME_LEN];
	int bitmap_id;
} generic_bitmap;

/*
 * @brief helper class to reduce params passed to generic_anim_render
 */
class generic_extras {
public:
	int width, height;
	float u0, v0, u1, v1;
	float alpha;
	bool draw;
	int resize_mode;

	generic_extras()
		: width(0), height(0)
		, u0(0.0f), v0(0.0f)
		, u1(1.0f), v1(1.0f)
		, alpha(1.0f)
		, draw(true)
		, resize_mode(0)	// GR_RESIZE_NONE
	{}
};

bool generic_bitmap_exists(const char *filename);
bool generic_anim_exists(const char *filename);
int generic_anim_init_and_stream(generic_anim *ga, const char *anim_filename, BM_TYPE bg_type, bool attempt_hi_res);
void generic_anim_init(generic_anim *ga);
void generic_anim_init(generic_anim *ga, const char *filename);
void generic_anim_init(generic_anim *ga, const SCP_string& filename);
void generic_bitmap_init(generic_bitmap *gb, const char *filename = NULL);
int generic_anim_load(generic_anim *ga);
int generic_anim_stream(generic_anim *ga, const bool cache = true);
int generic_bitmap_load(generic_bitmap *gb);
void generic_anim_unload(generic_anim *ga);
void generic_anim_render(generic_anim *ga, float frametime, int x, int y, bool menu = false, const generic_extras *ge = nullptr, float scale_factor = 1.0f);
void generic_anim_bitmap_set(generic_anim* ga, float frametime, const generic_extras* ge = nullptr);
void generic_anim_reset(generic_anim *ga);
#endif
