/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "globalincs/pstypes.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "io/joy_rumble.h"


static SDL_Gamepad *gamepad = nullptr;
static bool Gain_adjust = false;
static float Gain = 1.0f;

static ushort adjust_gain(ushort val)
{
	if (Gain_adjust) {
		val = static_cast<ushort>(val * Gain);
	}

	return val;
}

bool joy_rumble_init()
{
	auto Joy = io::joystick::getPlayerJoystick(CID_JOY0);

	if ( !Joy || !Joy->isGamepad() ) {
		return false;
	}

	mprintf(("\n"));
	mprintf(("  Initializing Rumble...\n"));

	gamepad = Joy->getGamepad();

	auto props = SDL_GetGamepadProperties(gamepad);

	if ( !SDL_GetBooleanProperty(props, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false) ) {
		mprintf(("    ERROR: Rumble not supported\n"));
		gamepad = nullptr;
		return false;
	}

	// TODO: SDL3 => figure out how best to use trigger rumble
	bool rumble_triggers = SDL_GetBooleanProperty(props, SDL_PROP_GAMEPAD_CAP_TRIGGER_RUMBLE_BOOLEAN, false);

	mprintf(("    Supports trigger rumble: %s\n", rumble_triggers ? "true" : "false"));

	mprintf(("  ... Rumble successfully initialized!\n"));

	return true;
}

void joy_rumble_shutdown()
{
	if (gamepad) {
		SDL_RumbleGamepad(gamepad, 0, 0, 0);
		gamepad = nullptr;
	}
}

void joy_rumble_set_gain(int gain)
{
	CAP(gain, 1, 100);

	Gain = gain / 100.0f;
	Gain_adjust = (gain < 100);
}

void joy_rumble_stop_effects()
{
	SDL_RumbleGamepad(gamepad, 0, 0, 0);
}

void joy_rumble_mission_init(vec3d *v __UNUSED)
{
}

void joy_rumble_play_vector_effect(vec3d *v __UNUSED, float scaler __UNUSED)
{
	SDL_RumbleGamepad(gamepad, adjust_gain(0x2fff), adjust_gain(0x4fff), 300);
}

void joy_rumble_play_dir_effect(float x __UNUSED, float y __UNUSED)
{
	SDL_RumbleGamepad(gamepad, adjust_gain(0x2fff), adjust_gain(0x4fff), 300);
}

void joy_rumble_play_primary_shoot(int gain)
{
	CAP(gain, 1, 10000);

	auto freq = ushort(0x7fff * (gain / 10000.0f));
	freq = adjust_gain(freq);

	SDL_RumbleGamepad(gamepad, freq, freq/2, 100);
}

void joy_rumble_play_secondary_shoot(int gain)
{
	gain = gain * 100 + 2500;

	CAP(gain, 1, 10000);

	int duration = (150000 + gain * 25) / 1000;
	auto freq = ushort(0xffff * (gain / 10000.0f));
	freq = adjust_gain(freq);

	SDL_RumbleGamepad(gamepad, freq, freq/2, duration);
}

void joy_rumble_adjust_handling(int speed __UNUSED)
{
}

void joy_rumble_docked()
{
	SDL_RumbleGamepad(gamepad, adjust_gain(0xffff), 0, 150);
}

void joy_rumble_play_reload_effect()
{
	SDL_RumbleGamepad(gamepad, adjust_gain(0x1fff), adjust_gain(0x7fff), 50);
}

void joy_rumble_afterburn_on()
{
	SDL_RumbleGamepad(gamepad, adjust_gain(0x7fff), 0, 1500);
}

void joy_rumble_afterburn_off()
{
	SDL_RumbleGamepad(gamepad, 0, 0, 0);
}

void joy_rumble_explode()
{
	SDL_RumbleGamepad(gamepad, adjust_gain(0xffff), adjust_gain(0x7fff), 500);
}

void joy_rumble_fly_by(int mag)
{
	int gain = mag * 120 + 4000;

	CAP(gain, 1, 10000);

	int duration = (6000 * mag + 400000) / 1000;

	SDL_RumbleGamepad(gamepad, adjust_gain(0x3fff), adjust_gain(0x7fff), duration);
}

void joy_rumble_deathroll()
{
	SDL_RumbleGamepad(gamepad, adjust_gain(0x4fff), adjust_gain(0x1fff), 4000);
}
