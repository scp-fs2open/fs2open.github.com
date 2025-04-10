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
#include "graphics/generic.h"
#include "freespace.h"
#include "gamesequence/gamesequence.h"
#include "menuui/fishtank.h"

// fish
typedef struct fish {
	float x, y;              // x and y coords
	float x_speed, y_speed;  // x and y speed
	float scale;             // big fish or small fish?
	bool left;	             // left or right
	generic_anim anim;       // the animation
	bool onscreen;           // visible? 			
	bool swimming;           // whee
} fish;

constexpr size_t MAX_FISH = 24; // was 12.. bigger screens need more fish!
SCP_vector<fish> All_fish;

// fish anim names
SCP_string Fish_left_anim_name;
SCP_string Fish_right_anim_name;

int Fish_inited = 0;

void fish_generate()
{
	if (!Fish_inited) {
		return;
	}

	// Find a free fish
	auto it = std::find_if(All_fish.begin(), All_fish.end(), [](const fish& f) { return !f.swimming; });

	// No fish left so bail
	if (it == All_fish.end()) {
		return;
	}

	// Found a fishy!
	fish* f = &(*it);

	// Pick a direction randomly
	f->left = frand_range(0.0f, 1.0f) >= 0.5f;

	// Let it freeeeeeee!
	f->swimming = true;

	// Set the animation
	if (f->left) {
		generic_anim_init(&f->anim, Fish_left_anim_name);

		// Doh! Something went wrong. Maybe the fish escaped!
		if (generic_anim_stream(&f->anim) == -1) {
			f->swimming = false;
			generic_anim_init(&f->anim);
		}
	} else {
		generic_anim_init(&f->anim, Fish_right_anim_name);
		
		// Doh! Something went wrong. Maybe the fish escaped!
		if (generic_anim_stream(&f->anim) == -1) {
			f->swimming = false;
			generic_anim_init(&f->anim);
		}
	}

	// Pick a scale
	f->scale = frand_range(0.5f, 1.0f);

	// Pick a starting location
	if(f->left){
		f->x = gr_screen.max_w_unscaled_zoomed + frand_range(0.0f, 50.0f);
	} else {
		f->x = frand_range(0.0f, -50.0f) - f->anim.width;
	}
	f->y = frand_range(-40.0f, (float)gr_screen.max_h_unscaled_zoomed + 40.0f);

	// Maybe give it zoomies. Maybe give it a sedative.
	if(f->left){
		f->x_speed = frand_range(-1.0f, -15.0f);
	} else {
		f->x_speed = frand_range(1.0f, 15.0f);
	}
	f->y_speed = frand_range(0.0f, 1.0f) < 0.5f ? frand_range(1.0f, 4.0f) : frand_range(-1.0f, -4.0f);

	// Fish can be way too slow on big screens cause this was written for 1024 pixels wide MAX. So let's scale it for our current screen size.
	f->y_speed *= (gr_screen.max_h / 786.0f);
	f->x_speed *= (gr_screen.max_w / 1024.0f);

	// All fish start out offscreen
	f->onscreen = false;
}

void fish_flush(fish *f)
{
	// Bad fish!
	if(f == nullptr){
		return;
	}

	// Catch and release or something
	if (f->anim.first_frame != -1) {
		generic_anim_unload(&f->anim);
	}

	// No longer swimming
	f->swimming = false;
}

void fishtank_start(const SCP_string& f_left, const SCP_string& f_right)
{
	if(Fish_inited){
		return;
	}
	
	// Get our anim names
	Fish_left_anim_name = f_left;
	Fish_right_anim_name = f_right;

	generic_anim fish_left;
	generic_anim fish_right;

	// Test that we can load the anims. Bail if we can't. Unload and continue if we can.
	generic_anim_init(&fish_left, Fish_left_anim_name);
	if (generic_anim_stream(&fish_left) == -1) {
		Warning(LOCATION, "Could not load fish tank animation %s", Fish_left_anim_name.c_str());
		return;
	}
	generic_anim_unload(&fish_left);

	generic_anim_init(&fish_right, Fish_right_anim_name);
	if (generic_anim_stream(&fish_right) == -1) {
		Warning(LOCATION, "Could not load fish tank animation %s", Fish_right_anim_name.c_str());
		return;
	}
	generic_anim_unload(&fish_right);

	// We have good anims so prep the vector and make sure it's clean.
	All_fish.clear();
	All_fish.shrink_to_fit();

	Fish_inited = 1;

	// Generate a random # of fish
	int count = Random::next(1, MAX_FISH);
	for (int idx = 0; idx < count; idx++) {
		fish new_fish;
		new_fish.swimming = false;
		All_fish.push_back(new_fish);
		fish_generate();
	}		
}

void fishtank_stop()
{
	if(!Fish_inited){
		return;
	}

	// Release stuff		
	for (fish& f : All_fish) {
		if (f.anim.first_frame != -1) {
			generic_anim_unload(&f.anim);
		}
		f.swimming = false;
	}


	All_fish.clear();
	All_fish.shrink_to_fit();

	Fish_inited = 0;
}

void fishtank_change_speed(float& speed, float increase, float decrease, float multiplier)
{
	float speed_modifier = (frand_range(0.0f, 1.0f) < 0.5f) ? increase : decrease;
	float min = 5.0f * multiplier;
	float max = 20.0f * multiplier;

	// Positive speed
	if (speed > 0.0f) {
		if ((speed < min && speed_modifier == decrease) || (speed > max && speed_modifier == increase)) {
			speed_modifier = 1.0f;
		}
	// Negative speed
	} else {
		if ((speed > -min && speed_modifier == decrease) || (speed < -max && speed_modifier == increase)) {
			speed_modifier = 1.0f;
		}
	}

	speed = speed * speed_modifier;
}

void fishtank_process()
{
	if(!Fish_inited){
		return;
	}

	// Small chance to add a fish
	if (All_fish.size() < MAX_FISH) {
		if (frand_range(0.0f, 1.0f) < 0.0005f) {
			fish new_fish;
			new_fish.swimming = false;
			All_fish.push_back(new_fish);
			fish_generate();
		}
	}

	// Process all fish
	for (auto it = All_fish.begin(); it != All_fish.end();) {
		fish& f = *it;

		// Not swimming?
		if (!f.swimming) {
			++it;
			continue;
		}

		// Small chance to swap vertical direction
		if (frand_range(0.0f, 1.0f) < 0.001f) {
			f.y_speed *= -1;
		}

		// Small chance to slightly change speed
		if (frand_range(0.0f, 1.0f) < 0.001f) {
			if (frand_range(0.0f, 1.0f) < 0.5f) {
				fishtank_change_speed(f.x_speed, 1.7f, 0.5f, gr_screen.max_w / 1024.0f);
			} else {
				fishtank_change_speed(f.y_speed, 1.1f, 0.9f, gr_screen.max_h / 786.0f);
			}
		}

		// Move it along according to its speed settings
		f.x += f.x_speed * flFrametime;
		f.y += f.y_speed * flFrametime;

		// Check if it's on screen still
		bool onscreen = false;
		if ((f.x < static_cast<float>(gr_screen.max_w_unscaled_zoomed)) && ((f.x + f.anim.width) >= 0.0f) &&
			(f.y < static_cast<float>(gr_screen.max_h_unscaled_zoomed)) && ((f.y + f.anim.height) >= 0.0f)) {
			onscreen = true;
		}

		// If it was onscreen before but is no longer, yeet it or flush it
		if (f.onscreen && !onscreen) {
			// Small chance to yeet the fish instead of flushing it. I didn't come up with this language. Don't ask me.
			if (frand_range(0.0f, 1.0f) < 0.3f) {
				it = All_fish.erase(it);
			} else {
				fish_flush(&f); // Flush the fish and keep it in the pool. How does that make sense?
				fish_generate();
				++it;
			}
			continue;
		}

		// Otherwise, just mark its current status
		f.onscreen = onscreen;

		// Render the fish if it's onscreen
		if (f.onscreen) {
			generic_anim_render(&f.anim, flFrametime, (int)f.x, (int)f.y, false, nullptr, f.scale);
		}

		++it;
	}
}
