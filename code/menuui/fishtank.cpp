/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "anim/animplay.h"
#include "anim/packunpack.h"
#include "menuui/fishtank.h"
#include "freespace2/freespace.h"
#include "gamesequence/gamesequence.h"



// fish
typedef struct fish {
	float x, y;						// x and y coords
	float	x_speed, y_speed;		// x and y speed
	int	left;						// left or right
	anim_instance *a;				// tha animation
	int	onscreen;				
	int	swimming;				// whee
} fish;
#define MAX_FISH					12
fish Fish[MAX_FISH];

// fish anim name
#define FISH_LEFT_ANIM_NAME			"f_left.ani"
#define FISH_RIGHT_ANIM_NAME			"f_right.ani"

#define FISH_ANIM_WIDTH			100
#define FISH_ANIM_HEIGHT		30

anim *Fish_left_anim = NULL;
anim *Fish_right_anim = NULL;

int Fish_inited = 0;

void fish_generate()
{
	fish *f;
	int idx;

	if(!Fish_inited){
		return;
	}

	// bogus anims
	if((Fish_left_anim == NULL) || (Fish_right_anim == NULL)){
		return;
	}

	// find a free fish
	f = NULL;
	for(idx=0; idx<MAX_FISH; idx++){
		if(!Fish[idx].swimming){
			f = &Fish[idx];
		}
	}

	// no fish left
	if(f == NULL){
		return;
	}	

	// left or right
	f->left = frand_range(0.0f, 1.0f) < 0.5f ? 0 : 1;

	// start location
	if(f->left){
		f->x = gr_screen.max_w_unscaled_zoomed + frand_range(0.0f, 50.0f);
	} else {
		f->x = frand_range(0.0f, -50.0f) - FISH_ANIM_WIDTH;
	}
	f->y = frand_range(-40.0f, (float)gr_screen.max_h_unscaled_zoomed + 40.0f);

	// speed
	if(f->left){
		f->x_speed = frand_range(-1.0f, -15.0f);
	} else {
		f->x_speed = frand_range(1.0f, 15.0f);
	}
	f->y_speed = frand_range(0.0f, 1.0f) < 0.5f ? frand_range(1.0f, 4.0f) : frand_range(-1.0f, -4.0f);

	// all fish start out offscreen
	f->onscreen = 0;

	// he's swimming
	f->swimming = 1;

	// anim instance
	anim_play_struct aps;

	if(f->left){
		anim_play_init(&aps, Fish_left_anim, (int)f->x, (int)f->y);		
		f->a = anim_play(&aps);

		// doh. cancel him
		if(f->a == NULL){
			f->swimming = 0;
		} else {
			f->a->screen_id = GS_STATE_MAIN_MENU;
			f->a->looped = 1;
			f->a->framerate_independent = 1;
		}
	} else {
		anim_play_init(&aps, Fish_right_anim, (int)f->x, (int)f->y);		
		f->a = anim_play(&aps);

		// doh. cancel him
		if(f->a == NULL){
			f->swimming = 0;
		} else {
			f->a->screen_id = GS_STATE_MAIN_MENU;
			f->a->looped = 1;
			f->a->framerate_independent = 1;
		}
	}
}

void fish_flush(fish *f)
{
	// bogus
	if(f == NULL){
		return;
	}

	// release his render instance
	if(f->a != NULL){
		anim_release_render_instance(f->a);
		f->a = NULL;
	}

	// no longer swimming
	f->swimming = 0;
}

void fishtank_start()
{
	int idx;

	if(Fish_inited){
		return;
	}

	// try and load the fish anim
	Fish_left_anim = anim_load(FISH_LEFT_ANIM_NAME);
	if(Fish_left_anim == NULL){
		return;
	}
	Fish_right_anim = anim_load(FISH_RIGHT_ANIM_NAME);
	if(Fish_right_anim == NULL){
		return;
	}

	// no anim instances
	for(idx=0; idx<MAX_FISH; idx++){
		Fish[idx].a = NULL;
		Fish[idx].swimming = 0;
	}	

	Fish_inited = 1;

	// generate a random # of fish
	int count = (int)frand_range(1.0f, (float)(MAX_FISH - 1));
	for(idx=0; idx<count; idx++){
		fish_generate();
	}		
}

void fishtank_stop()
{
	int idx;

	if(!Fish_inited){
		return;
	}

	// release stuff		
	for(idx=0; idx<MAX_FISH; idx++){
		if(Fish[idx].a != NULL){
			anim_release_render_instance(Fish[idx].a);
			Fish[idx].a = NULL;
		}
		Fish[idx].swimming = 0;
	}
	if(Fish_left_anim != NULL){
		anim_free(Fish_left_anim);
		Fish_left_anim = NULL;
	}
	if(Fish_right_anim != NULL){
		anim_free(Fish_right_anim);
		Fish_right_anim = NULL;
	}	

	Fish_inited = 0;
}

void fishtank_process()
{
	int idx, onscreen;
	fish *f;

	if(!Fish_inited){
		return;
	}

	// process all fish
	for(idx=0; idx<MAX_FISH; idx++){		
		f = &Fish[idx];

		// not swimming?
		if(!f->swimming){
			continue;
		}

		// move him along
		f->x += f->x_speed * flFrametime;
		f->y += f->y_speed * flFrametime;

		// is he currently onscreen ?
		onscreen = 0;
		if( (f->x < (float)gr_screen.max_w_unscaled_zoomed) && ((f->x + FISH_ANIM_WIDTH) >= 0.0f) &&
			 (f->y < (float)gr_screen.max_h_unscaled_zoomed) && ((f->y + FISH_ANIM_HEIGHT) >= 0.0f) ){
			onscreen = 1;
		}

		// if he was onscreen before, but is no longer, flush him and make a new fish
		if(f->onscreen && !onscreen){
			fish_flush(f);

			fish_generate();
			continue;
		}

		// otherwise just mark his current status
		f->onscreen = onscreen;

		// render
		if(f->onscreen){
			// set coords
			f->a->x = (int)f->x;
			f->a->y = (int)f->y;

			anim_render_one(GS_STATE_MAIN_MENU, f->a, flFrametime);
		}
	}
}

