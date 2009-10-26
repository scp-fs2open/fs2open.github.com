#include "globalincs/pstypes.h"
#include "bmpman/bmpman.h"
#include "graphics/generic.h"
#include "graphics/2d.h"

// Goober5000
void generic_anim_init(generic_anim *ga, char *filename)
{
	//memset(ga, 0, sizeof(ga));	//this makes the mission load screen crash :(
	if (filename == NULL) {
		ga->filename[0] = '\0';
	} else {
		strncpy(ga->filename, filename, MAX_FILENAME_LEN - 1);
	}

	ga->first_frame = -1;
	ga->num_frames = 0;
	ga->total_time = 1.0f;
	ga->anim_time = 0.0f;
	ga->done_playing = 0;
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

	ga->first_frame = bm_load_animation(ga->filename, &ga->num_frames, &fps, &ga->keyframe);
	//mprintf(("generic_anim_load: %s - keyframe = %d\n", ga->filename, ga->keyframe));

	if (ga->first_frame < 0)
		return -1;

	Assert(fps != 0);
	ga->total_time = ga->num_frames / (float)fps;
	ga->done_playing = 0;
	ga->anim_time = 0.0f;

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

void generic_anim_unload(generic_anim *ga)
{
	int i;

	for(i = ga->first_frame; i < (ga->first_frame + ga->num_frames); i++)
		bm_unload(i);
	generic_anim_init(ga, NULL);
}

void generic_anim_render(generic_anim *ga, float frametime, int x, int y)
{
	float keytime = 0.0;

	if(ga->keyframe)
		keytime = (ga->total_time * ((float)ga->keyframe / (float)ga->num_frames));
	//don't mess with the frame time if we're paused
	if((ga->direction & GENERIC_ANIM_DIRECTION_PAUSED) == 0) {
		if(ga->direction & GENERIC_ANIM_DIRECTION_BACKWARDS) {
			//keep going forwards if we're in a keyframe loop
			if(ga->keyframe && (ga->anim_time > keytime)) {
				ga->anim_time += frametime;
				if(ga->anim_time >= ga->total_time) {
					ga->anim_time = keytime - 0.001;
					ga->done_playing = 0;
				}
			}
			else {
				//playing backwards
				ga->anim_time -= frametime;
				if((ga->direction & GENERIC_ANIM_DIRECTION_NOLOOP) && ga->anim_time < 0.0) {
					ga->anim_time = 0;		//stop on first frame when playing in reverse
				}
				else {
					while(ga->anim_time < 0.0)
						ga->anim_time += ga->total_time;	//make sure we're always positive, so we can go back to the end
				}
			}
		}
		else {
			ga->anim_time += frametime;
			if(ga->anim_time >= ga->total_time) {
				if(ga->direction & GENERIC_ANIM_DIRECTION_NOLOOP) {
					ga->anim_time = ga->total_time - 0.001;		//stop on last frame when playing - if it's equal we jump to the first frame
					ga->done_playing = 1;
				}
				else if(!ga->done_playing){
					//we've played this at least once
					ga->done_playing = 1;
					if(ga->keyframe) {
						ga->anim_time = keytime;
					}
				}
			}
		}
	}
	if(ga->num_frames > 0)
	{
		ga->current_frame = 0;
		if(ga->done_playing && ga->keyframe) {
			ga->anim_time = fmod(ga->anim_time - keytime, ga->total_time - keytime) + keytime;
		}
		else {
			ga->anim_time = fmod(ga->anim_time, ga->total_time);
		}
		ga->current_frame += fl2i(ga->anim_time * ga->num_frames / ga->total_time);
		//sanity check
		CLAMP(ga->current_frame, 0, ga->num_frames - 1);
		gr_set_bitmap(ga->first_frame + ga->current_frame);
		gr_bitmap(x, y);
	}
}
