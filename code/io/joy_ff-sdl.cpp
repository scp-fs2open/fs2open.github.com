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
#include "math/vecmat.h"
#include "mod_table/mod_table.h"
#include "options/Option.h"
#include "osapi/osapi.h"
#include "osapi/osregistry.h"

#include "SDL_haptic.h"


#ifndef SDL_INIT_HAPTIC
#define SDL_INIT_HAPTIC		0x00001000
#endif

static int Joy_ff_enabled = 0;
static int Joy_ff_acquired = 0;
static SDL_Haptic *haptic = NULL;
static int joy_ff_handling_scaler = 0;
static bool Joy_ff_directional_hit_effect_enabled = true;
static int Joy_ff_afterburning = 0;

typedef struct haptic_effect_t {
	SDL_HapticEffect eff;
	int id;
	bool loaded;

	void init() {
		eff = {0};
		id = -1;
		loaded = false;
	}

	haptic_effect_t() { init(); }
} haptic_effect_t;

static haptic_effect_t pHitEffect1;
static haptic_effect_t pHitEffect2;
static haptic_effect_t pAfterburn1;
static haptic_effect_t pAfterburn2;
static haptic_effect_t pShootEffect;
static haptic_effect_t pSecShootEffect;
static haptic_effect_t pSpring;
static haptic_effect_t pDock;

static auto ForceFeedbackOption = options::OptionBuilder<bool>("Input.ForceFeedback",
                     std::pair<const char*, int>{"Force Feedback", 1728},
                     std::pair<const char*, int>{"Enable or disable force feedback", 1729})
                     .category(std::make_pair("Input", 1827))
                     .level(options::ExpertLevel::Beginner)
                     .default_val(false)
                     .finish();

static auto HitEffectOption = options::OptionBuilder<bool>("Input.HitEffect",
                     std::pair<const char*, int>{"Directional Hit", 1730},
                     std::pair<const char*, int>{"Enable or disable the directional hit effect", 1731})
                     .category(std::make_pair("Input", 1827))
                     .level(options::ExpertLevel::Beginner)
                     .default_val(false)
                     .finish();

static auto ForceFeedbackStrength = options::OptionBuilder<float>("Input.FFStrength",
                     std::pair<const char*, int>{"Force Feedback Strength", 1756},
                     std::pair<const char*, int>{"The realtive strength of Force Feedback effects", 1757})
                     .category(std::make_pair("Input", 1827))
                     .level(options::ExpertLevel::Beginner)
                     .range(0, 100)
                     .default_val(100)
                     .finish();

static bool joy_ff_create_effects();
static void joy_ff_start_effect(haptic_effect_t *eff, const char *name);
static void joy_ff_stop_effect(haptic_effect_t *eff, const char *name);

static void check_and_print_haptic_feature(uint32_t flags, uint32_t check_flag, const char* description) {
	auto has_flag = (flags & check_flag) == check_flag;

	mprintf(("      Supports %s: %s\n", description, has_flag ? "true" : "false"));
}

static void print_haptic_support() {
	mprintf(("    Haptic feature support:\n"));

	auto supported = SDL_HapticQuery(haptic);
	check_and_print_haptic_feature(supported, SDL_HAPTIC_CONSTANT, "Constant effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_SINE, "Sine effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_LEFTRIGHT, "Left right effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_TRIANGLE, "Triangle effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_SAWTOOTHUP, "Sawtooth up effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_SAWTOOTHDOWN, "Sawtooth down effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_RAMP, "Ramp effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_SPRING, "Spring effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_DAMPER, "Damper effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_INERTIA, "Inertia effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_FRICTION, "Friction effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_CUSTOM, "Custom effect");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_GAIN, "global gain");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_AUTOCENTER, "Autocenter");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_STATUS, "Status query");
	check_and_print_haptic_feature(supported, SDL_HAPTIC_PAUSE, "Pause effects");
}

int joy_ff_init()
{
	bool ff_enabled;

	if (Joy_ff_enabled) {
		return 0;
	}

	auto Joy = io::joystick::getPlayerJoystick(CID_JOY0);

	if ( !Joy || !Joy->isHaptic() ) {
		return 0;
	}

	if (Using_in_game_options) {
		ff_enabled = ForceFeedbackOption->getValue();
	} else {
		ff_enabled = os_config_read_uint(nullptr, "EnableJoystickFF", 1) != 0;
	}

	if ( !ff_enabled ) {
		return 0;
	}

	// ignore gamepads
	if ( Joy->isGamepad() ) {
		// TODO: add support for gamepad API and rumble effects
		return 0;
	}

	mprintf(("\n"));
	mprintf(("  Initializing Haptic...\n"));

	if (SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0) {
		mprintf(("    ERROR: Could not initialize Haptic subsystem: %s\n", SDL_GetError()));
		return -1;
	}

#ifndef NDEBUG
	auto numHaptics = SDL_NumHaptics();
	mprintf(("  Available haptic devices:\n"));

	if (numHaptics <= 0)
	{
		mprintf(("    <none>\n"));
	}
	else
	{
		for (int i = 0; i < numHaptics; ++i)
		{
			mprintf(("    %s\n", SDL_HapticName(i)));
		}
	}
#endif

	haptic = SDL_HapticOpenFromJoystick(Joy->getDevice());

	if (haptic == NULL) {
		mprintf(("    ERROR: Unable to open haptic joystick: %s\n", SDL_GetError()));
		SDL_QuitSubSystem(SDL_INIT_HAPTIC);
		return -1;
	}

	Joy_ff_enabled = 1;
	Joy_ff_acquired = 1;

	if ( !joy_ff_create_effects() ) {
		mprintf(("    ERROR: Unable to create effects\n"));
		joy_ff_shutdown();
		return -1;
	}

	if (Using_in_game_options) {
		Joy_ff_directional_hit_effect_enabled = HitEffectOption->getValue();
	} else {
		Joy_ff_directional_hit_effect_enabled = os_config_read_uint(nullptr, "EnableHitEffect", 1) != 0;
	}
	// ForceFeedback.Strength lets the user specify how strong the effects should be. This uses SDL_HapticSetGain which
	// needs to be supported by the haptic device
	int ff_strength;

	if (Using_in_game_options) {
		ff_strength = (int)ForceFeedbackStrength->getValue();
	} else {
		ff_strength = os_config_read_uint("ForceFeedback", "Strength", 100);
	}
	CLAMP(ff_strength, 0, 100);
	if (SDL_HapticQuery(haptic) & SDL_HAPTIC_GAIN) {
		SDL_HapticSetGain(haptic, ff_strength);
	} else {
		if (ff_strength != 100) {
			ReleaseWarning(LOCATION, "The configuration file is configured with a force feedback strength value of %d%% "
				"but your haptic device does not support setting the global strength of the effects. All effects will be"
				"played with full strength.", ff_strength);
		}
	}

	mprintf(("\n"));
	mprintf(("    Number of haptic axes: %d\n", SDL_HapticNumAxes(haptic)));
	mprintf(("    Number of effects supported: %d\n", SDL_HapticNumEffects(haptic)));
	mprintf(("    Number of simultaneous effects: %d\n", SDL_HapticNumEffectsPlaying(haptic)));
	print_haptic_support();

	mprintf(("  ... Haptic successfully initialized!\n"));

	return 0;
}

void joy_ff_shutdown()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	joy_ff_afterburn_off();
	SDL_HapticStopAll(haptic);

	SDL_HapticClose(haptic);
	haptic = NULL;

	SDL_QuitSubSystem(SDL_INIT_HAPTIC);

	Joy_ff_enabled = 0;
	Joy_ff_acquired = 0;
}

int joy_ff_reinit()
{
	if (Joy_ff_enabled) {
		joy_ff_shutdown();
	}

	return joy_ff_init();
}

static bool joy_ff_create_effects()
{
	int num_loaded = 0;

	// clear all SDL errors
	SDL_ClearError();

	// missing effects aren't fatal, we'll just ignore/disable them
	auto supported = SDL_HapticQuery(haptic);

	// effects are created in priority order, most important first
	// this allows a soft fail for low memory devices that can't load everything

	if (supported & SDL_HAPTIC_SPRING) {
		// pSpring
		pSpring.init();

		pSpring.eff.type = SDL_HAPTIC_SPRING;
		pSpring.eff.condition.type = SDL_HAPTIC_SPRING;
		pSpring.eff.condition.length = SDL_HAPTIC_INFINITY;

		for (size_t i = 0; i < SDL_arraysize(pSpring.eff.condition.right_sat); i++) {
			pSpring.eff.condition.right_sat[i] = 0x7FFF;
			pSpring.eff.condition.left_sat[i] = 0x7FFF;
			pSpring.eff.condition.right_coeff[i] = 0x147;
			pSpring.eff.condition.left_coeff[i] = 0x147;
		}

		pSpring.id = SDL_HapticNewEffect(haptic, &pSpring.eff);

		if (pSpring.id < 0) {
			mprintf(("    Spring effect failed to load:\n      %s\n", SDL_GetError()));
		} else {
			pSpring.loaded = true;
			++num_loaded;
		}
	}

	if (supported & SDL_HAPTIC_SAWTOOTHDOWN) {
		// pShootEffect
		pShootEffect.init();

		pShootEffect.eff.type = SDL_HAPTIC_SAWTOOTHDOWN;
		pShootEffect.eff.periodic.type = SDL_HAPTIC_SAWTOOTHDOWN;
		pShootEffect.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
		pShootEffect.eff.periodic.direction.dir[0] = 0;
		pShootEffect.eff.periodic.length = 160;
		pShootEffect.eff.periodic.period = 20;
		pShootEffect.eff.periodic.magnitude = 0x7FFF;
		pShootEffect.eff.periodic.fade_length = 120;

		pShootEffect.id = SDL_HapticNewEffect(haptic, &pShootEffect.eff);

		if (pShootEffect.id < 0) {
			mprintf(("    Fire primary effect failed to load:\n      %s\n", SDL_GetError()));
		} else {
			pShootEffect.loaded = true;
			++num_loaded;
		}
	}

	if (supported & SDL_HAPTIC_CONSTANT) {
		// pSecShootEffect
		pSecShootEffect.init();

		pSecShootEffect.eff.type = SDL_HAPTIC_CONSTANT;
		pSecShootEffect.eff.constant.type = SDL_HAPTIC_CONSTANT;
		pSecShootEffect.eff.constant.direction.type = SDL_HAPTIC_POLAR;
		pSecShootEffect.eff.constant.direction.dir[0] = 0;
		pSecShootEffect.eff.constant.length = 200;
		pSecShootEffect.eff.constant.level = 0x7FFF;
		pSecShootEffect.eff.constant.attack_length = 50;
		pSecShootEffect.eff.constant.attack_level = 0x7FFF;
		pSecShootEffect.eff.constant.fade_length = 100;
		pSecShootEffect.eff.constant.fade_level = 1;

		pSecShootEffect.id = SDL_HapticNewEffect(haptic, &pSecShootEffect.eff);

		if (pSecShootEffect.id < 0) {
			mprintf(("    Fire secondary effect failed to load:\n      %s\n", SDL_GetError()));
		} else {
			pSecShootEffect.loaded = true;
			++num_loaded;
		}
	}

	if (supported & SDL_HAPTIC_SINE) {
		// pAfterburn1
		pAfterburn1.init();

		pAfterburn1.eff.type = SDL_HAPTIC_SINE;
		pAfterburn1.eff.periodic.type = SDL_HAPTIC_SINE;
		pAfterburn1.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
		pAfterburn1.eff.periodic.direction.dir[0] = 0;
		pAfterburn1.eff.periodic.length = SDL_HAPTIC_INFINITY;
		pAfterburn1.eff.periodic.period = 20;
		pAfterburn1.eff.periodic.magnitude = 0x3332;

		pAfterburn1.id = SDL_HapticNewEffect(haptic, &pAfterburn1.eff);

		if (pAfterburn1.id < 0) {
			mprintf(("    Afterburn effect 1 failed to load:\n      %s\n", SDL_GetError()));
		} else {
			pAfterburn1.loaded = true;
			++num_loaded;
		}

		// pAfterburn2
		pAfterburn2.init();

		pAfterburn2.eff.type = SDL_HAPTIC_SINE;
		pAfterburn2.eff.periodic.type = SDL_HAPTIC_SINE;
		pAfterburn2.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
		pAfterburn2.eff.periodic.direction.dir[0] = 9000;
		pAfterburn2.eff.periodic.length = 125;
		pAfterburn2.eff.periodic.period = 100;
		pAfterburn2.eff.periodic.magnitude = 0x1999;

		pAfterburn2.id = SDL_HapticNewEffect(haptic, &pAfterburn2.eff);

		if (pAfterburn2.id < 0) {
			mprintf(("    Afterburn effect 2 failed to load:\n      %s\n", SDL_GetError()));
		} else {
			pAfterburn2.loaded = true;
			++num_loaded;
		}
	}

	if (supported & SDL_HAPTIC_CONSTANT) {
		// pHitEffect1
		pHitEffect1.init();

		pHitEffect1.eff.type = SDL_HAPTIC_CONSTANT;
		pHitEffect1.eff.constant.type = SDL_HAPTIC_CONSTANT;
		pHitEffect1.eff.constant.direction.type = SDL_HAPTIC_POLAR;
		pHitEffect1.eff.constant.direction.dir[0] = 0;
		pHitEffect1.eff.constant.length = 300;
		pHitEffect1.eff.constant.level = 0x7FFF;
		pHitEffect1.eff.constant.attack_length = 0;
		pHitEffect1.eff.constant.attack_level = 0x7FFF;
		pHitEffect1.eff.constant.fade_length = 120;
		pHitEffect1.eff.constant.fade_level = 1;

		pHitEffect1.id = SDL_HapticNewEffect(haptic, &pHitEffect1.eff);

		if (pHitEffect1.id < 0) {
			mprintf(("    Hit effect 1 failed to load:\n      %s\n", SDL_GetError()));
		} else {
			pHitEffect1.loaded = true;
			++num_loaded;
		}
	}

	if (supported & SDL_HAPTIC_SINE) {
		// pHitEffect2
		pHitEffect2.init();

		pHitEffect2.eff.type = SDL_HAPTIC_SINE;
		pHitEffect2.eff.periodic.type = SDL_HAPTIC_SINE;
		pHitEffect2.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
		pHitEffect2.eff.periodic.direction.dir[0] = 9000;
		pHitEffect2.eff.periodic.length = 300;
		pHitEffect2.eff.periodic.period = 100;
		pHitEffect2.eff.periodic.magnitude = 0x7FFF;
		pHitEffect2.eff.periodic.attack_length = 100;
		pHitEffect2.eff.periodic.fade_length = 100;

		pHitEffect2.id = SDL_HapticNewEffect(haptic, &pHitEffect2.eff);

		if (pHitEffect2.id < 0) {
			mprintf(("    Hit effect 2 failed to load:\n      %s\n", SDL_GetError()));
		} else {
			pHitEffect2.loaded = true;
			++num_loaded;
		}
	}

	if (supported & SDL_HAPTIC_SINE /* SDL_HAPTIC_SQUARE */) {
		// pDock
		pDock.init();

		pDock.eff.type = SDL_HAPTIC_SINE; //SDL_HAPTIC_SQUARE;
		pDock.eff.periodic.type = SDL_HAPTIC_SINE; //SDL_HAPTIC_SQUARE;
		pDock.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
		pDock.eff.periodic.direction.dir[0] = 9000;
		pDock.eff.periodic.length = 125;
		pDock.eff.periodic.period = 100;
		pDock.eff.periodic.magnitude = 0x3332;

		pDock.id = SDL_HapticNewEffect(haptic, &pDock.eff);

		if (pDock.id < 0) {
			mprintf(("    Dock effect failed to load:\n      %s\n", SDL_GetError()));
		} else {
			pDock.loaded = true;
			++num_loaded;
		}
	}

	return (num_loaded > 0);
}

static bool joy_ff_can_play()
{
	return (Joy_ff_enabled && Joy_ff_acquired);
}

static void joy_ff_start_effect(haptic_effect_t *eff, const char* /* name */)
{
	if ( !eff->loaded ) {
		return;
	}

//	nprintf(("Joystick", "FF: Starting effect %s\n", name));

	if (SDL_HapticRunEffect(haptic, eff->id, 1) != 0) {
//		nprintf(("Joystick", "FF: Unable to run %s:\n  %s\n", name, SDL_GetError()));
	}
}

static void joy_ff_update_effect(haptic_effect_t *eff, const char* /* name */)
{
	if ( !eff->loaded ) {
		return;
	}

//	nprintf(("Joystick", "FF: Updating effect %s\n", name));

	if (SDL_HapticUpdateEffect(haptic, eff->id, &eff->eff) != 0) {
//		nprintf(("Joystick", "FF: Unable to update %s:\n  %s\n", name, SDL_GetError()));
	}
}

static void joy_ff_stop_effect(haptic_effect_t *eff, const char* /* name */)
{
	if ( !eff->loaded ) {
		return;
	}

//	nprintf(("Joystick", "FF: Stopping effect %s\n", name));

	if (SDL_HapticStopEffect(haptic, eff->id) != 0) {
//		nprintf(("Joystick", "FF: Unable to stop %s:\n  %s\n", name, SDL_GetError()));
	}
}

void joy_ff_stop_effects()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	joy_ff_afterburn_off();

	// stop everything *except* spring
	joy_ff_stop_effect(&pHitEffect1, "HitEffect1");
	joy_ff_stop_effect(&pHitEffect2, "HitEffect2");
	joy_ff_stop_effect(&pAfterburn1, "Afterburn1");
	joy_ff_stop_effect(&pAfterburn2, "Afterburn2");
	joy_ff_stop_effect(&pShootEffect, "ShootEffect");
	joy_ff_stop_effect(&pSecShootEffect, "SecShootEffect");
	joy_ff_stop_effect(&pDock, "Dock");
}

void joy_ff_mission_init(vec3d v)
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	v.xyz.z = 0.0f;

	joy_ff_handling_scaler = (int) ((vm_vec_mag(&v) + 1.3f) * 5.0f);

	joy_ff_adjust_handling(0);
	joy_ff_start_effect(&pSpring, "Spring");

	Joy_ff_afterburning = 0;

	// reset afterburn effects to default values

	pAfterburn1.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pAfterburn1.eff.periodic.period = 20;
	pAfterburn1.eff.periodic.magnitude = 0x3332;
	pAfterburn1.eff.periodic.attack_length = 0;

	joy_ff_update_effect(&pAfterburn1, "Afterburn1 (init)");

	pAfterburn2.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pAfterburn2.eff.periodic.period = 100;
	pAfterburn2.eff.periodic.magnitude = 0x1999;
	pAfterburn2.eff.periodic.attack_length = 0;

	joy_ff_update_effect(&pAfterburn2, "Afterburn2 (init)");

	// reset primary shoot effect to default values

	pShootEffect.eff.periodic.direction.dir[0] = 0;
	pShootEffect.eff.periodic.length = 160;
	pShootEffect.eff.periodic.fade_length = 120;

	joy_ff_update_effect(&pShootEffect, "ShootEffect (init)");
}

void joy_reacquire_ff()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if (Joy_ff_acquired) {
		return;
	}

	Joy_ff_acquired = 1;

	joy_ff_start_effect(&pSpring, "Spring");
}

void joy_unacquire_ff()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if ( !Joy_ff_acquired ) {
		return;
	}

	joy_ff_afterburn_off();
	SDL_HapticStopAll(haptic);

	Joy_ff_acquired = 0;
}

void joy_ff_play_vector_effect(vec3d *v, float scaler)
{
	vec3d vf;
	float x, y;

	if ( !joy_ff_can_play() ) {
		return;
	}

//	nprintf(("Joystick", "FF: vec = { %f, %f, %f } s = %f\n", v->xyz.x, v->xyz.y, v->xyz.z, scaler));
	vm_vec_copy_scale(&vf, v, scaler);
	x = vf.xyz.x;
	vf.xyz.x = 0.0f;

	if (vf.xyz.y + vf.xyz.z < 0.0f) {
		y = -vm_vec_mag(&vf);
	} else {
		y = vm_vec_mag(&vf);
	}

	joy_ff_play_dir_effect(-x, -y);
}

void joy_ff_play_dir_effect(float x, float y)
{
	static UI_TIMESTAMP hit_timeout;
	int idegs, imag;
	float degs;

	if ( !joy_ff_can_play() ) {
		return;
	}

	// allow for at least one of the effects to work
	if ( !pHitEffect1.loaded && !pHitEffect2.loaded ) {
		return;
	}

	if ( !ui_timestamp_elapsed(hit_timeout) ) {
		nprintf(("Joystick", "FF: HitEffect already playing.  Skipping\n"));
		return;
	}

	hit_timeout = ui_timestamp(pHitEffect1.eff.condition.length);

	if (Joy_ff_directional_hit_effect_enabled) {
		if (x > 8000.0f) {
			x = 8000.0f;
		} else if (x < -8000.0f) {
			x = -8000.0f;
		}

		if (y > 8000.0f) {
			y = 8000.0f;
		} else if (y < -8000.0f) {
			y = -8000.0f;
		}

		imag = (int) fl_sqrt(x * x + y * y);
		if (imag > 10000) {
			imag = 10000;
		}

		degs = (float)atan2(x, y);
		idegs = (int) (degs * 18000.0f / PI) + 90;
		while (idegs < 0) {
			idegs += 36000;
		}

		while (idegs >= 36000) {
			idegs -= 36000;
		}

		pHitEffect1.eff.constant.direction.dir[0] = idegs;
		pHitEffect1.eff.constant.level = (Sint16)fl2i(0x7FFF * (imag / 10000.0f));

		joy_ff_update_effect(&pHitEffect1, "HitEffect1");

		idegs += 9000;
		if (idegs >= 36000)
			idegs -= 36000;

		pHitEffect2.eff.periodic.direction.dir[0] = idegs;
		pHitEffect2.eff.periodic.magnitude = (Sint16)fl2i(0x7FFF * (imag / 10000.0f));

		joy_ff_update_effect(&pHitEffect2, "HitEffect2");
	}

	joy_ff_start_effect(&pHitEffect1, "HitEffect1");
	joy_ff_start_effect(&pHitEffect2, "HitEffect2");
}

void joy_ff_play_primary_shoot(int gain)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if ( !pShootEffect.loaded ) {
		return;
	}

	CLAMP(gain, 1, 10000);

	static int primary_ff_level = 10000;

	if (gain != primary_ff_level) {
		pShootEffect.eff.periodic.magnitude = (Sint16) fl2i(0x7FFF * (gain / 10000.0f));

		joy_ff_update_effect(&pShootEffect, "ShootEffect");

		primary_ff_level = gain;
	}

	joy_ff_start_effect(&pShootEffect, "ShootEffect");
}

void joy_ff_play_secondary_shoot(int gain)
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if ( !pSecShootEffect.loaded ) {
		return;
	}

	gain = gain * 100 + 2500;

	CLAMP(gain, 1, 10000);

	static int secondary_ff_level = 10000;

	if (gain != secondary_ff_level) {
		pSecShootEffect.eff.constant.level = (Sint16) fl2i(0x7FFF * (gain / 10000.0f));
		pSecShootEffect.eff.constant.length = (150000 + gain * 25) / 1000;

		joy_ff_update_effect(&pSecShootEffect, "SecShootEffect");

		secondary_ff_level = gain;
//		nprintf(("Joystick", "FF: Secondary force = 0x%04x\n", pSecShootEffect.eff.constant.level));
	}

	joy_ff_start_effect(&pSecShootEffect, "SecShootEffect");
}

void joy_ff_adjust_handling(int speed)
{
	int v;
	short coeff = 0;

	if ( !joy_ff_can_play() ) {
		return;
	}

	if ( !pSpring.loaded ) {
		return;
	}

	static int last_speed = -1000;

	if (speed == last_speed) {
		return;
	}

	last_speed = speed;

	v = speed * joy_ff_handling_scaler * 2 / 3;
//	v += joy_ff_handling_scaler * joy_ff_handling_scaler * 6 / 7 + 250;
	v += joy_ff_handling_scaler * 45 - 500;

	CLAMP(v, 0, 10000);

	coeff = (Sint16) fl2i(0x7FFF * (v / 10000.0f));

	for (size_t i = 0; i < SDL_arraysize(pSpring.eff.condition.right_coeff); i++) {
		pSpring.eff.condition.right_coeff[i] = coeff;
		pSpring.eff.condition.left_coeff[i] = coeff;
	}

//	nprintf(("Joystick", "FF: New handling force = 0x%04x\n", coeff));

	joy_ff_update_effect(&pSpring, "Spring");
}

void joy_ff_docked()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if ( !pDock.loaded ) {
		return;
	}

	pDock.eff.periodic.magnitude = 0x3332;

	joy_ff_update_effect(&pDock, "Dock");
	joy_ff_start_effect(&pDock, "Dock");
}

void joy_ff_play_reload_effect()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if ( !pDock.loaded ) {
		return;
	}

	pDock.eff.periodic.magnitude = 0x1999;

	joy_ff_update_effect(&pDock, "Dock (Reload)");
	joy_ff_start_effect(&pDock, "Dock (Reload)");
}

void joy_ff_afterburn_on()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Joy_ff_afterburning) {
		return;
	}

	pAfterburn1.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pAfterburn1.eff.periodic.magnitude = 0x3332;

	joy_ff_update_effect(&pAfterburn1, "Afterburn1");

	pAfterburn2.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pAfterburn2.eff.periodic.magnitude = 0x1999;

	joy_ff_update_effect(&pAfterburn2, "Afterburn2");

	joy_ff_start_effect(&pAfterburn1, "Afterburn1");
	joy_ff_start_effect(&pAfterburn2, "Afterburn2");

//	nprintf(("Joystick", "FF: Afterburn started\n"));

	Joy_ff_afterburning = 1;
}

void joy_ff_afterburn_off()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if ( !Joy_ff_afterburning ) {
		return;
	}

	joy_ff_stop_effect(&pAfterburn1, "Afterburn1");
	joy_ff_stop_effect(&pAfterburn2, "Afterburn2");

	Joy_ff_afterburning = 0;

//	nprintf(("Joystick", "FF: Afterburn stopped\n"));
}

void joy_ff_explode()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	joy_ff_afterburn_off();

	// reuse shoot effect here to save device memory
	if (pShootEffect.loaded) {
		// direction changes so an effect stop is needed before udpate
		joy_ff_stop_effect(&pShootEffect, "ShootEffect (Explode)");

		pShootEffect.eff.periodic.direction.dir[0] = 9000;
		pShootEffect.eff.periodic.length = 500;
		pShootEffect.eff.periodic.magnitude = 0x7FFF;
		pShootEffect.eff.periodic.fade_length = 500;

		joy_ff_update_effect(&pShootEffect, "ShootEffect (Explode)");

		joy_ff_start_effect(&pShootEffect, "ShootEffect (Explode)");
	}
}

void joy_ff_fly_by(int mag)
{
	int gain;

	if ( !joy_ff_can_play() ) {
		return;
	}

	if (Joy_ff_afterburning) {
		return;
	}

	gain = mag * 120 + 4000;

	CLAMP(gain, 1, 10000);

	pAfterburn1.eff.periodic.length = (6000 * mag + 400000) / 1000;
	pAfterburn1.eff.periodic.magnitude = (Sint16) fl2i(0x7FFF * (gain / 10000.0f));

	joy_ff_update_effect(&pAfterburn1, "Afterburn1 (Fly by)");

	pAfterburn2.eff.periodic.length = (6000 * mag + 400000) / 1000;
	pAfterburn2.eff.periodic.magnitude = (Sint16) fl2i(0x7FFF * (gain / 10000.0f));

	joy_ff_update_effect(&pAfterburn2, "Afterburn2 (Fly by)");

	joy_ff_start_effect(&pAfterburn1, "Afterburn1 (Fly by)");
	joy_ff_start_effect(&pAfterburn2, "Afterburn2 (Fly by)");
}

void joy_ff_deathroll()
{
	if ( !joy_ff_can_play() ) {
		return;
	}

	if ( !(pAfterburn1.loaded && pAfterburn2.loaded) ) {
		return;
	}

	// reuse afterburn effects here to save device memory

	pAfterburn1.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pAfterburn1.eff.periodic.period = 200;
	pAfterburn1.eff.periodic.magnitude = 0x7FFF;
	pAfterburn1.eff.periodic.attack_length = 200;

	joy_ff_update_effect(&pAfterburn1, "Afterburn1 (Death Roll)");

	pAfterburn2.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pAfterburn2.eff.periodic.period = 200;
	pAfterburn2.eff.periodic.magnitude = 0x7FFF;
	pAfterburn2.eff.periodic.attack_length = 200;

	joy_ff_update_effect(&pAfterburn2, "Afterburn2 (Death Roll)");

	joy_ff_start_effect(&pAfterburn1, "Afterburn1 (Death Roll)");
	joy_ff_start_effect(&pAfterburn2, "Afterburn2 (Death Roll)");

	Joy_ff_afterburning = 1;	// to prevent flyby effect
}
