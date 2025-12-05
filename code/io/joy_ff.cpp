/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "globalincs/pstypes.h"
#include "options/Option.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "io/joy_haptic.h"
#include "io/joy_rumble.h"


static bool Joy_ff_enabled = false;
static bool Joy_ff_acquired = false;
static bool Joy_ff_directional_hit_effect_enabled = true;
static bool Joy_ff_afterburning = false;
static bool Using_rumble = false;


static bool joy_ff_can_play()
{
	return (Joy_ff_enabled && Joy_ff_acquired);
}


static auto ForceFeedbackOption = options::OptionBuilder<bool>("Input.ForceFeedback",
					 std::pair<const char*, int>{"Force Feedback", 1728},
					 std::pair<const char*, int>{"Enable or disable force feedback", 1729})
					 .category(std::make_pair("Input", 1827))
					 .level(options::ExpertLevel::Beginner)
					 .default_val(true)
					 .change_listener([](bool val, bool) {
						 if (val) joy_ff_init();
						 else joy_ff_shutdown();
						 return true;
					 })
					 .finish();

static auto HitEffectOption = options::OptionBuilder<bool>("Input.HitEffect",
					 std::pair<const char*, int>{"Directional Hit", 1730},
					 std::pair<const char*, int>{"Enable or disable the directional hit effect", 1731})
					 .category(std::make_pair("Input", 1827))
					 .level(options::ExpertLevel::Beginner)
					 .default_val(true)
					 .change_listener([](bool val, bool) {
						 Joy_ff_directional_hit_effect_enabled = val;
						 return true;
					 })
					 .finish();

static auto ForceFeedbackStrength = options::OptionBuilder<int>("Input.FFStrength",
					 std::pair<const char*, int>{"Force Feedback Strength", 1756},
					 std::pair<const char*, int>{"The realtive strength of Force Feedback effects", 1757})
					 .category(std::make_pair("Input", 1827))
					 .level(options::ExpertLevel::Beginner)
					 .range(0, 100)
					 .default_val(100)
					 .flags({options::OptionFlags::RangeTypeInteger})
					 .change_listener([](int val, bool) {
						 joy_haptic_set_gain(val);
						 joy_rumble_set_gain(val);
						 return true;
					 })
					 .finish();


bool joy_ff_init()
{
	bool ff_enabled;
	int ff_strength;

	if (Joy_ff_enabled) {
		return true;
	}

	auto Joy = io::joystick::getPlayerJoystick(CID_JOY0);

	if ( !Joy ) {
		return false;
	}

	if (Using_in_game_options) {
		ff_enabled = ForceFeedbackOption->getValue();
	} else {
		ff_enabled = os_config_read_uint(nullptr, "EnableJoystickFF", 1) != 0;
	}

	if ( !ff_enabled ) {
		return false;
	}

	if (Using_in_game_options) {
		Joy_ff_directional_hit_effect_enabled = HitEffectOption->getValue();
	} else {
		Joy_ff_directional_hit_effect_enabled = os_config_read_uint(nullptr, "EnableHitEffect", 1) != 0;
	}

	if (Using_in_game_options) {
		ff_strength = (int)ForceFeedbackStrength->getValue();
	} else {
		ff_strength = os_config_read_uint("ForceFeedback", "Strength", 100);
	}

	if (Joy->isGamepad()) {
		Using_rumble = joy_rumble_init();

		if ( !Using_rumble ) {
			joy_ff_shutdown();
			return false;
		}

		joy_rumble_set_gain(ff_strength);
	} else {
		if ( !joy_haptic_init() ) {
			joy_ff_shutdown();
			return false;
		}

		joy_haptic_set_gain(ff_strength);
	}

	Joy_ff_enabled = true;
	Joy_ff_acquired = true;

	return true;
}

void joy_ff_shutdown()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	joy_ff_afterburn_off();

	joy_haptic_shutdown();
	joy_rumble_shutdown();

	Joy_ff_enabled = false;
	Joy_ff_acquired = false;
	Using_rumble = false;
}

bool joy_ff_reinit()
{
	if (Joy_ff_enabled) {
		joy_ff_shutdown();
	}

	return joy_ff_init();
}

bool joy_ff_hit_effect_enabled()
{
	return Joy_ff_directional_hit_effect_enabled;
}

void joy_ff_stop_effects()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_stop_effects();
	} else {
		joy_haptic_stop_effects();
	}
}

void joy_ff_mission_init(vec3d v)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	Joy_ff_afterburning = false;

	if (Using_rumble) {
		joy_rumble_mission_init(&v);
	} else {
		joy_haptic_mission_init(&v);
	}
}

void joy_reacquire_ff()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if (Joy_ff_acquired) {
		return;
	}

	Joy_ff_acquired = true;
}

void joy_unacquire_ff()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if ( !Joy_ff_acquired ) {
		return;
	}

	joy_ff_stop_effects();

	Joy_ff_acquired = false;
}

void joy_ff_play_vector_effect(vec3d *v, float scaler)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_play_vector_effect(v, scaler);
	} else {
		joy_haptic_play_vector_effect(v, scaler);
	}
}

void joy_ff_play_dir_effect(float x, float y)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_play_dir_effect(x, y);
	} else {
		joy_haptic_play_dir_effect(x, y);
	}
}

void joy_ff_play_primary_shoot(int gain)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_play_primary_shoot(gain);
	} else {
		joy_haptic_play_primary_shoot(gain);
	}
}

void joy_ff_play_secondary_shoot(int gain)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_play_secondary_shoot(gain);
	} else {
		joy_haptic_play_secondary_shoot(gain);
	}
}

void joy_ff_adjust_handling(int speed)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_adjust_handling(speed);
	} else {
		joy_haptic_adjust_handling(speed);
	}
}

void joy_ff_docked()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_docked();
	} else {
		joy_haptic_docked();
	}
}

void joy_ff_play_reload_effect()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_play_reload_effect();
	} else {
		joy_haptic_play_reload_effect();
	}
}

void joy_ff_afterburn_on()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Joy_ff_afterburning) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_afterburn_on();
	} else {
		joy_haptic_afterburn_on();
	}

	Joy_ff_afterburning = true;
}

void joy_ff_afterburn_off()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if ( !Joy_ff_afterburning ) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_afterburn_off();
	} else {
		joy_haptic_afterburn_off();
	}

	Joy_ff_afterburning = false;
}

void joy_ff_explode()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	joy_ff_afterburn_off();

	if (Using_rumble) {
		joy_rumble_explode();
	} else {
		joy_haptic_explode();
	}
}

void joy_ff_fly_by(int mag)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Joy_ff_afterburning) {
		return;
	}

	if (Using_rumble) {
		joy_rumble_fly_by(mag);
	} else {
		joy_haptic_fly_by(mag);
	}
}

void joy_ff_deathroll()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	joy_ff_afterburn_off();

	if (Using_rumble) {
		joy_rumble_deathroll();
	} else {
		joy_haptic_deathroll();
	}

	Joy_ff_afterburning = true;
}
