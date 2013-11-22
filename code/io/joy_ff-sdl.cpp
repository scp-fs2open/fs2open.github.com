/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#include "globalincs/pstypes.h"
#include "math/vecmat.h"
#include "osapi/osregistry.h"
#include "io/joy_ff.h"
#include "osapi/osapi.h"

#include "SDL_haptic.h"


#ifndef SDL_INIT_HAPTIC
#define SDL_INIT_HAPTIC		0x00001000
#endif

extern SDL_Joystick *sdljoy;

static int Joy_ff_enabled = 0;
static SDL_Haptic *haptic = NULL;
static int joy_ff_handling_scaler = 0;
static int Joy_ff_directional_hit_effect_enabled = 1;

typedef struct {
	SDL_HapticEffect eff;
	int id;
	int loaded;
} haptic_effect_t;

static haptic_effect_t pHitEffect1;
static haptic_effect_t pHitEffect2;
static haptic_effect_t pAfterburn1;
static haptic_effect_t pAfterburn2;
static haptic_effect_t pShootEffect;
static haptic_effect_t pSecShootEffect;
static haptic_effect_t pSpring;
static haptic_effect_t pDock;
static haptic_effect_t pDeathroll1;
static haptic_effect_t pDeathroll2;
static haptic_effect_t pExplode;

static int joy_ff_create_effects();
static int joy_ff_has_valid_effects();
static int joy_ff_effect_playing(haptic_effect_t *eff);
static void joy_ff_start_effect(haptic_effect_t *eff, char *name);


static void *sdl_lib = NULL;

typedef int (SDLCALL * SDLJOYSTICISHAPTICPTR) (SDL_Joystick *);
typedef SDL_Haptic * (SDLCALL * SDLHAPTICOPENFROMJOYSTICKPTR) (SDL_Joystick *);
typedef void (SDLCALL * SDLHAPTICCLOSEPTR) (SDL_Haptic *);
typedef int (SDLCALL * SDLHAPTICNUMAXESPTR) (SDL_Haptic *);
typedef int (SDLCALL * SDLHAPTICNUMEFFECTSPTR) (SDL_Haptic *);
typedef int (SDLCALL * SDLHAPTICNUMEFFECTSPLAYINGPTR) (SDL_Haptic *);
typedef int (SDLCALL * SDLHAPTICNEWEFFECTPTR) (SDL_Haptic *, SDL_HapticEffect *);
typedef int (SDLCALL * SDLHAPTICUPDATEEFFECTPTR) (SDL_Haptic *, int, SDL_HapticEffect *);
typedef int (SDLCALL * SDLHAPTICRUNEFFECTPTR) (SDL_Haptic *, int, Uint32);
typedef int (SDLCALL * SDLHAPTICSTOPEFFECTPTR) (SDL_Haptic *, int);
typedef void (SDLCALL * SDLHAPTICDESTROYEFFECTPTR) (SDL_Haptic *, int);
typedef int (SDLCALL * SDLHAPTICGETEFFECTSTATUSPTR) (SDL_Haptic *, int);
typedef int (SDLCALL * SDLHAPTICSTOPALLPTR) (SDL_Haptic *);
typedef unsigned int (SDLCALL * SDLHAPTICQUERYPTR) (SDL_Haptic *);

static SDLJOYSTICISHAPTICPTR v_SDL_JoystickIsHaptic = NULL;
static SDLHAPTICOPENFROMJOYSTICKPTR v_SDL_HapticOpenFromJoystick = NULL;
static SDLHAPTICCLOSEPTR v_SDL_HapticClose = NULL;
static SDLHAPTICNUMAXESPTR v_SDL_HapticNumAxes = NULL;
static SDLHAPTICNUMEFFECTSPTR v_SDL_HapticNumEffects = NULL;
static SDLHAPTICNUMEFFECTSPLAYINGPTR v_SDL_HapticNumEffectsPlaying = NULL;
static SDLHAPTICNEWEFFECTPTR v_SDL_HapticNewEffect = NULL;
static SDLHAPTICUPDATEEFFECTPTR v_SDL_HapticUpdateEffect = NULL;
static SDLHAPTICRUNEFFECTPTR v_SDL_HapticRunEffect = NULL;
static SDLHAPTICSTOPEFFECTPTR v_SDL_HapticStopEffect = NULL;
static SDLHAPTICDESTROYEFFECTPTR v_SDL_HapticDestroyEffect = NULL;
static SDLHAPTICGETEFFECTSTATUSPTR v_SDL_HapticGetEffectStatus = NULL;
static SDLHAPTICSTOPALLPTR v_SDL_HapticStopAll = NULL;
static SDLHAPTICQUERYPTR v_SDL_HapticQuery = NULL;


static void *load_haptic_function(const char *fname)
{
	void *func = SDL_LoadFunction(sdl_lib, fname);

	if (func == NULL) {
		throw fname;
	}

	return func;
}

static int load_haptic()
{
	sdl_lib = SDL_LoadObject(NULL);

	if (sdl_lib == NULL) {
	//	nprintf(("HapticDEBUG", "Unable to load process!\n"));
		return -1;
	}

	try {
		v_SDL_JoystickIsHaptic = (SDLJOYSTICISHAPTICPTR) load_haptic_function("SDL_JoystickIsHaptic");
		v_SDL_HapticOpenFromJoystick = (SDLHAPTICOPENFROMJOYSTICKPTR) load_haptic_function("SDL_HapticOpenFromJoystick");
		v_SDL_HapticClose = (SDLHAPTICCLOSEPTR) load_haptic_function("SDL_HapticClose");
		v_SDL_HapticNumAxes = (SDLHAPTICNUMAXESPTR) load_haptic_function("SDL_HapticNumAxes");
		v_SDL_HapticNumEffects = (SDLHAPTICNUMEFFECTSPTR) load_haptic_function("SDL_HapticNumEffects");
		v_SDL_HapticNumEffectsPlaying = (SDLHAPTICNUMEFFECTSPLAYINGPTR) load_haptic_function("SDL_HapticNumEffectsPlaying");
		v_SDL_HapticNewEffect = (SDLHAPTICNEWEFFECTPTR) load_haptic_function("SDL_HapticNewEffect");
		v_SDL_HapticUpdateEffect = (SDLHAPTICUPDATEEFFECTPTR) load_haptic_function("SDL_HapticUpdateEffect");
		v_SDL_HapticRunEffect = (SDLHAPTICRUNEFFECTPTR) load_haptic_function("SDL_HapticRunEffect");
		v_SDL_HapticStopEffect = (SDLHAPTICSTOPEFFECTPTR) load_haptic_function("SDL_HapticStopEffect");
		v_SDL_HapticDestroyEffect = (SDLHAPTICDESTROYEFFECTPTR) load_haptic_function("SDL_HapticDestroyEffect");
		v_SDL_HapticGetEffectStatus = (SDLHAPTICGETEFFECTSTATUSPTR) load_haptic_function("SDL_HapticGetEffectStatus");
		v_SDL_HapticStopAll = (SDLHAPTICSTOPALLPTR) load_haptic_function("SDL_HapticStopAll");
		v_SDL_HapticQuery = (SDLHAPTICQUERYPTR) load_haptic_function("SDL_HapticQuery");
	} catch (const char *msg) {
		nprintf(("HapticDEBUG", "Missing haptic function: %s\n", msg));
		return -1;
	}

	return 0;
}

static void close_haptic()
{
	if (sdl_lib) {
		SDL_UnloadObject(sdl_lib);
		sdl_lib = NULL;
	}
}

int joy_ff_init()
{
	int ff_enabled = 0;

	ff_enabled = os_config_read_uint(NULL, "EnableJoystickFF", 1);

	if ( !ff_enabled ) {
		return 0;
	}

	mprintf(("\n"));
	mprintf(("  Initializing Haptic...\n"));

	if (load_haptic() < 0) {
		mprintf(("    <not available>\n"));
		return -1;
	}

	if (SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0) {
		mprintf(("    ERROR: Could not initialize Haptic subsystem\n"));
		return -1;
	}

	if ( !v_SDL_JoystickIsHaptic(sdljoy) ) {
		mprintf(("    ERROR: Joystick does not have haptic capabilities\n"));
		SDL_QuitSubSystem(SDL_INIT_HAPTIC);
		return -1;
	}

	haptic = v_SDL_HapticOpenFromJoystick(sdljoy);

	if (haptic == NULL) {
		mprintf(("    ERROR: Unable to open haptic joystick\n"));
		SDL_QuitSubSystem(SDL_INIT_HAPTIC);
		return -1;
	}

	Joy_ff_enabled = 1;

	if ( !joy_ff_has_valid_effects() ) {
		mprintf(("    ERROR: Haptic joystick does not support the necessary effects\n"));
		joy_ff_shutdown();
		return -1;
	}

	if ( joy_ff_create_effects() ) {
		mprintf(("    ERROR: Unable to create effects\n"));
		joy_ff_shutdown();
		return -1;
	}

	Joy_ff_directional_hit_effect_enabled = os_config_read_uint(NULL, "EnableHitEffect", 1);

	mprintf(("\n"));
	mprintf(("    Number of haptic axes: %d\n", v_SDL_HapticNumAxes(haptic)));
	mprintf(("    Number of effects supported: %d\n", v_SDL_HapticNumEffects(haptic)));
	mprintf(("    Number of simultaneous effects: %d\n", v_SDL_HapticNumEffectsPlaying(haptic)));

	mprintf(("  ... Haptic successfully initialized!\n"));

	return 0;
}

void joy_ff_shutdown()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	v_SDL_HapticClose(haptic);
	haptic = NULL;

	SDL_QuitSubSystem(SDL_INIT_HAPTIC);

	close_haptic();

	Joy_ff_enabled = 0;
}

static int joy_ff_has_valid_effects()
{
	unsigned int supported = 0;
	int rval = 1;

	supported = v_SDL_HapticQuery(haptic);

	if ( !(supported & SDL_HAPTIC_CONSTANT) ) {
		mprintf((" missing constant effect\n"));
		rval = 0;
	}

	if ( !(supported & SDL_HAPTIC_SINE) ) {
		mprintf((" missing sine effect\n"));
		rval = 0;
	}

	if ( !(supported & SDL_HAPTIC_SAWTOOTHDOWN) ) {
		mprintf((" missing sawtoothdown effect\n"));
		rval = 0;
	}

	if ( !(supported & SDL_HAPTIC_SPRING) ) {
		mprintf((" missing spring effect\n"));
		rval = 0;
	}

	//SDL_HAPTIC_SQUARE has been removed from SDL2, pending reintroduction in SDL 2.1
	/*if ( !(supported & SDL_HAPTIC_SQUARE) ) {
		mprintf((" missing square effect\n"));
		rval = 0;
	}*/

	return rval;
}

static int joy_ff_create_effects()
{
	// clear all SDL errors
	SDL_ClearError();


	// pHitEffect1
	memset(&pHitEffect1, 0, sizeof(haptic_effect_t));

	pHitEffect1.eff.type = SDL_HAPTIC_CONSTANT;
	pHitEffect1.eff.constant.direction.type = SDL_HAPTIC_POLAR;
	pHitEffect1.eff.constant.direction.dir[0] = 0;
	pHitEffect1.eff.constant.length = 300;
	pHitEffect1.eff.constant.level = 0x7FFF;
	pHitEffect1.eff.constant.attack_length = 0;
	pHitEffect1.eff.constant.attack_level = 0x7FFF;
	pHitEffect1.eff.constant.fade_length = 120;
	pHitEffect1.eff.constant.fade_level = 1;

	pHitEffect1.id = v_SDL_HapticNewEffect(haptic, &pHitEffect1.eff);

	if (pHitEffect1.id < 0) {
		mprintf(("    Hit effect 1 failed to load:\n      %s\n", SDL_GetError()));
		return -1;
	} else {
		pHitEffect1.loaded = 1;
	}

	// pHitEffect2
	memset(&pHitEffect2, 0, sizeof(haptic_effect_t));

	pHitEffect2.eff.type = SDL_HAPTIC_SINE;
	pHitEffect2.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
	pHitEffect2.eff.periodic.direction.dir[0] = 9000;
	pHitEffect2.eff.periodic.length = 300;
	pHitEffect2.eff.periodic.period = 100;
	pHitEffect2.eff.periodic.magnitude = 0x7FFF;
	pHitEffect2.eff.periodic.attack_length = 100;
	pHitEffect2.eff.periodic.fade_length = 100;

	pHitEffect2.id = v_SDL_HapticNewEffect(haptic, &pHitEffect2.eff);

	if (pHitEffect2.id < 0) {
		mprintf(("    Hit effect 2 failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pHitEffect2.loaded = 1;
	}

	// pShootEffect
	memset(&pShootEffect, 0, sizeof(haptic_effect_t));

	pShootEffect.eff.type = SDL_HAPTIC_SAWTOOTHDOWN;
	pShootEffect.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
	pShootEffect.eff.periodic.direction.dir[0] = 0;
	pShootEffect.eff.periodic.length = 160;
	pShootEffect.eff.periodic.period = 20;
	pShootEffect.eff.periodic.magnitude = 0x7FFF;
	pShootEffect.eff.periodic.fade_length = 120;

	pShootEffect.id = v_SDL_HapticNewEffect(haptic, &pShootEffect.eff);

	if (pShootEffect.id < 0) {
		mprintf(("    Fire primary effect failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pShootEffect.loaded = 1;
	}

	// pSecShootEffect
	memset(&pSecShootEffect, 0, sizeof(haptic_effect_t));

	pSecShootEffect.eff.type = SDL_HAPTIC_CONSTANT;
	pSecShootEffect.eff.constant.direction.type = SDL_HAPTIC_POLAR;
	pSecShootEffect.eff.constant.direction.dir[0] = 0;
	pSecShootEffect.eff.constant.length = 200;
	pSecShootEffect.eff.constant.level = 0x7FFF;
	pSecShootEffect.eff.constant.attack_length = 50;
	pSecShootEffect.eff.constant.attack_level = 0x7FFF;
	pSecShootEffect.eff.constant.fade_length = 100;
	pSecShootEffect.eff.constant.fade_level = 1;

	pSecShootEffect.id = v_SDL_HapticNewEffect(haptic, &pSecShootEffect.eff);

	if (pSecShootEffect.id < 0) {
		mprintf(("    Fire secondary effect failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pSecShootEffect.loaded = 1;
	}

	// pSpring
	memset(&pSpring, 0, sizeof(haptic_effect_t));

	pSpring.eff.type = SDL_HAPTIC_SPRING;
	pSpring.eff.condition.length = SDL_HAPTIC_INFINITY;

	for (int i = 0; i < v_SDL_HapticNumAxes(haptic); i++) {
		pSpring.eff.condition.right_sat[i] = 0x7FFF;
		pSpring.eff.condition.left_sat[i] = 0x7FFF;
		pSpring.eff.condition.right_coeff[i] = 0x147;
		pSpring.eff.condition.left_coeff[i] = 0x147;
	}

	pSpring.id = v_SDL_HapticNewEffect(haptic, &pSpring.eff);

	if (pSpring.id < 0) {
		mprintf(("    Spring effect failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pSpring.loaded = 1;
		joy_ff_start_effect(&pSpring, "Spring");
	}

	// pAfterburn1
	memset(&pAfterburn1, 0, sizeof(haptic_effect_t));

	pAfterburn1.eff.type = SDL_HAPTIC_SINE;
	pAfterburn1.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
	pAfterburn1.eff.periodic.direction.dir[0] = 0;
	pAfterburn1.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pAfterburn1.eff.periodic.period = 20;
	pAfterburn1.eff.periodic.magnitude = 0x6665;

	pAfterburn1.id = v_SDL_HapticNewEffect(haptic, &pAfterburn1.eff);

	if (pAfterburn1.id < 0) {
		mprintf(("    Afterburn effect 1 failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pAfterburn1.loaded = 1;
	}

	// pAfterburn2
	memset(&pAfterburn2, 0, sizeof(haptic_effect_t));

	pAfterburn2.eff.type = SDL_HAPTIC_SINE;
	pAfterburn2.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
	pAfterburn2.eff.periodic.direction.dir[0] = 9000;
	pAfterburn2.eff.periodic.length = 125;
	pAfterburn2.eff.periodic.period = 100;
	pAfterburn2.eff.periodic.magnitude = 0x3332;

	pAfterburn2.id = v_SDL_HapticNewEffect(haptic, &pAfterburn2.eff);

	if (pAfterburn2.id < 0) {
		mprintf(("    Afterburn effect 2 failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pAfterburn2.loaded = 1;
	}

	// pDock
	memset(&pDock, 0, sizeof(haptic_effect_t));

	pDock.eff.type = SDL_HAPTIC_LEFTRIGHT; //SDL_HAPTIC_SQUARE;
	pDock.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
	pDock.eff.periodic.direction.dir[0] = 9000;
	pDock.eff.periodic.length = 125;
	pDock.eff.periodic.period = 100;
	pDock.eff.periodic.magnitude = 0x3332;

	pDock.id = v_SDL_HapticNewEffect(haptic, &pDock.eff);

	if (pDock.id < 0) {
		mprintf(("    Dock effect failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pDock.loaded = 1;
	}

	// pExplode
	memset(&pExplode, 0, sizeof(haptic_effect_t));

	pExplode.eff.type = SDL_HAPTIC_SAWTOOTHDOWN;
	pExplode.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
	pExplode.eff.periodic.direction.dir[0] = 9000;
	pExplode.eff.periodic.length = 500;
	pExplode.eff.periodic.period = 20;
	pExplode.eff.periodic.magnitude = 0x7FFF;
	pExplode.eff.periodic.attack_length = 0;
	pExplode.eff.periodic.fade_length = 500;

	pExplode.id = v_SDL_HapticNewEffect(haptic, &pExplode.eff);

	if (pExplode.id < 0) {
		mprintf(("    Explosion effect failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pExplode.loaded = 1;
	}

	// pDeathroll1
	memset(&pDeathroll1, 0, sizeof(haptic_effect_t));

	pDeathroll1.eff.type = SDL_HAPTIC_SINE;
	pDeathroll1.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
	pDeathroll1.eff.periodic.direction.dir[0] = 0;
	pDeathroll1.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pDeathroll1.eff.periodic.period = 200;
	pDeathroll1.eff.periodic.magnitude = 0x7FFF;
	pDeathroll1.eff.periodic.attack_length = 200;

	pDeathroll1.id = v_SDL_HapticNewEffect(haptic, &pDeathroll1.eff);

	if (pDeathroll1.id < 0) {
		mprintf(("    Deathroll effect 1 failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pDeathroll1.loaded = 1;
	}

	// pDeathroll2
	memset(&pDeathroll2, 0, sizeof(haptic_effect_t));

	pDeathroll2.eff.type = SDL_HAPTIC_SINE;
	pDeathroll2.eff.periodic.direction.type = SDL_HAPTIC_POLAR;
	pDeathroll2.eff.periodic.direction.dir[0] = 9000;
	pDeathroll2.eff.periodic.length = SDL_HAPTIC_INFINITY;
	pDeathroll2.eff.periodic.period = 200;
	pDeathroll2.eff.periodic.magnitude = 0x7FFF;
	pDeathroll2.eff.periodic.attack_length = 200;

	pDeathroll2.id = v_SDL_HapticNewEffect(haptic, &pDeathroll2.eff);

	if (pDeathroll2.id < 0) {
		mprintf(("    Deathroll effect 2 failed to load:\n      %s\n", SDL_GetError()));
	} else {
		pDeathroll2.loaded = 1;
	}

	return 0;
}

static void joy_ff_start_effect(haptic_effect_t *eff, char *name)
{
	if ( !eff->loaded ) {
		return;
	}

//	nprintf(("Joystick", "FF: Starting effect %s\n", name));

	v_SDL_HapticRunEffect(haptic, eff->id, 1);
}

void joy_ff_stop_effects()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	v_SDL_HapticStopAll(haptic);
}

void joy_ff_mission_init(vec3d v)
{
	v.xyz.z = 0.0f;

	joy_ff_handling_scaler = (int) ((vm_vec_mag(&v) + 1.3f) * 5.0f);
}

void joy_reacquire_ff()
{
}

void joy_unacquire_ff()
{
}

void joy_ff_play_vector_effect(vec3d *v, float scaler)
{
	vec3d vf;
	float x, y;

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
	int idegs, imag;
	float degs;

	if ( !Joy_ff_enabled ) {
		return;
	}

	if (joy_ff_effect_playing(&pHitEffect1) || joy_ff_effect_playing(&pHitEffect2)) {
		nprintf(("Joystick", "FF: HitEffect already playing.  Skipping\n"));
		return;
	}

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

		if (pHitEffect1.loaded) {
			pHitEffect1.eff.constant.direction.dir[0] = idegs;
			pHitEffect1.eff.constant.level = (Sint16)fl2i(0x7FFF * (imag / 10000.0f));

			if ( v_SDL_HapticUpdateEffect(haptic, pHitEffect1.id, &pHitEffect1.eff) < 0 ) {
				mprintf(("HapticERROR:  Unable to update pHitEffect1:\n  %s\n", SDL_GetError()));
			}
		}

		idegs += 9000;
		if (idegs >= 36000)
			idegs -= 36000;

		if (pHitEffect2.loaded) {
			pHitEffect2.eff.periodic.direction.dir[0] = idegs;
			pHitEffect2.eff.periodic.magnitude = (Sint16)fl2i(0x7FFF * (imag / 10000.0f));

			if ( v_SDL_HapticUpdateEffect(haptic, pHitEffect2.id, &pHitEffect2.eff) < 0 ) {
				mprintf(("HapticERROR:  Unable to update pHitEffect2:\n  %s\n", SDL_GetError()));
			}
		}
	}

	joy_ff_start_effect(&pHitEffect1, "HitEffect1");
	joy_ff_start_effect(&pHitEffect2, "HitEffect2");
}

static int primary_ff_level = 10000;

void joy_ff_play_primary_shoot(int gain)
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if ( !pShootEffect.loaded ) {
		return;
	}

	CLAMP(gain, 1, 10000);

	v_SDL_HapticStopEffect(haptic, pShootEffect.id);

	if (gain != primary_ff_level) {
		pShootEffect.eff.periodic.magnitude = (Sint16) fl2i(0x7FFF * (gain / 10000.0f));

		if ( v_SDL_HapticUpdateEffect(haptic, pShootEffect.id, &pShootEffect.eff) < 0 ) {
			mprintf(("HapticERROR:  Unable to update pShootEffect:\n  %s\n", SDL_GetError()));
		}

		primary_ff_level = gain;
	}

	joy_ff_start_effect(&pShootEffect, "ShootEffect");
}

static int secondary_ff_level = 10000;

void joy_ff_play_secondary_shoot(int gain)
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if ( !pSecShootEffect.loaded ) {
		return;
	}

	gain = gain * 100 + 2500;

	CLAMP(gain, 1, 10000);

	v_SDL_HapticStopEffect(haptic, pSecShootEffect.id);

	if (gain != secondary_ff_level) {
		pSecShootEffect.eff.constant.level = (Sint16) fl2i(0x7FFF * (gain / 10000.0f));
		pSecShootEffect.eff.constant.length = (150000 + gain * 25) / 1000;

		if ( v_SDL_HapticUpdateEffect(haptic, pSecShootEffect.id, &pSecShootEffect.eff) < 0 ) {
			mprintf(("HapticERROR:  Unable to update pSecShootEffect:\n  %s\n", SDL_GetError()));
		}

		secondary_ff_level = gain;
		nprintf(("Joystick", "FF: Secondary force = 0x%04x\n", pSecShootEffect.eff.constant.level));
	}

	joy_ff_start_effect(&pSecShootEffect, "SecShootEffect");
}

void joy_ff_adjust_handling(int speed)
{
	int v;
	short coeff = 0;

	if ( !Joy_ff_enabled ) {
		return;
	}

	if ( !pSpring.loaded ) {
		return;
	}

	v = speed * joy_ff_handling_scaler * 2 / 3;
//	v += joy_ff_handling_scaler * joy_ff_handling_scaler * 6 / 7 + 250;
	v += joy_ff_handling_scaler * 45 - 500;

	CLAMP(v, 0, 10000);

	coeff = (Sint16) fl2i(0x7FFF * (v / 10000.0f));

	for (int i = 0; i < v_SDL_HapticNumAxes(haptic); i++) {
		pSpring.eff.condition.right_coeff[i] = coeff;
		pSpring.eff.condition.left_coeff[i] = coeff;
	}

//	nprintf(("Joystick", "FF: New handling force = 0x%04x\n", coeff));

	v_SDL_HapticUpdateEffect(haptic, pSpring.id, &pSpring.eff);
}

static int joy_ff_effect_playing(haptic_effect_t *eff)
{
	return (v_SDL_HapticGetEffectStatus(haptic, eff->id) > 0);
}

void joy_ff_docked()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if ( !pDock.loaded ) {
		return;
	}

	v_SDL_HapticStopEffect(haptic, pDock.id);

	pDock.eff.periodic.magnitude = 0x7fff;

	if ( v_SDL_HapticUpdateEffect(haptic, pDock.id, &pDock.eff) < 0 ) {
		mprintf(("HapticERROR:  Unable to update pDock:\n  %s\n", SDL_GetError()));
	}

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

	v_SDL_HapticStopEffect(haptic, pDock.id);

	pDock.eff.periodic.magnitude = 0x3fff;

	if ( v_SDL_HapticUpdateEffect(haptic, pDock.id, &pDock.eff) < 0 ) {
		mprintf(("HapticERROR:  Unable to update pDock:\n  %s\n", SDL_GetError()));
	}

	joy_ff_start_effect(&pDock, "Dock (Reload)");
}

static int Joy_ff_afterburning = 0;

void joy_ff_afterburn_on()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if (pAfterburn1.loaded) {
		v_SDL_HapticStopEffect(haptic, pAfterburn1.id);

		pAfterburn1.eff.periodic.length = SDL_HAPTIC_INFINITY;
		pAfterburn1.eff.periodic.magnitude = 0x3fff;

		if ( v_SDL_HapticUpdateEffect(haptic, pAfterburn1.id, &pAfterburn1.eff) < 0 ) {
			mprintf(("HapticERROR:  Unable to update pAfterburn1:\n  %s\n", SDL_GetError()));
		}
	}

	if (pAfterburn2.loaded) {
		v_SDL_HapticStopEffect(haptic, pAfterburn2.id);

		pAfterburn2.eff.periodic.length = SDL_HAPTIC_INFINITY;
		pAfterburn2.eff.periodic.magnitude = 0x3fff;

		if ( v_SDL_HapticUpdateEffect(haptic, pAfterburn2.id, &pAfterburn2.eff) < 0 ) {
			mprintf(("HapticERROR:  Unable to update pAfterburn2:\n  %s\n", SDL_GetError()));
		}
	}

	joy_ff_start_effect(&pAfterburn1, "Afterburn1");
	joy_ff_start_effect(&pAfterburn2, "Afterburn2");

//	nprintf(("Joystick", "FF: Afterburn started\n"));

	Joy_ff_afterburning = 1;
}

void joy_ff_afterburn_off()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if ( !Joy_ff_afterburning ) {
		return;
	}

	if (pAfterburn1.loaded) {
		v_SDL_HapticStopEffect(haptic, pAfterburn1.id);
	}

	if (pAfterburn2.loaded) {
		v_SDL_HapticStopEffect(haptic, pAfterburn2.id);
	}

	Joy_ff_afterburning = 0;

//	nprintf(("Joystick", "FF: Afterburn stopped\n"));
}

void joy_ff_explode()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if (pDeathroll1.loaded) {
		v_SDL_HapticStopEffect(haptic, pDeathroll1.id);
	}

	if (pDeathroll2.loaded) {
		v_SDL_HapticStopEffect(haptic, pDeathroll2.id);
	}

	if (pExplode.loaded) {
		v_SDL_HapticStopEffect(haptic, pExplode.id);
	}

	joy_ff_start_effect(&pExplode, "Explode");
}

void joy_ff_fly_by(int mag)
{
	int gain;

	if ( !Joy_ff_enabled ) {
		return;
	}

	if (Joy_ff_afterburning) {
		return;
	}

	gain = mag * 120 + 4000;

	CLAMP(gain, 1, 10000);

	if (pAfterburn1.loaded) {
		v_SDL_HapticStopEffect(haptic, pAfterburn1.id);

		pAfterburn1.eff.periodic.length = (6000 * mag + 400000) / 1000;
		pAfterburn1.eff.periodic.magnitude = (Sint16) fl2i(0x7FFF * (gain / 10000.0f));

		if ( v_SDL_HapticUpdateEffect(haptic, pAfterburn1.id, &pAfterburn1.eff) < 0 ) {
			mprintf(("HapticERROR:  Unable to update pAfterburn1:\n  %s\n", SDL_GetError()));
		}
	}

	if (pAfterburn2.loaded) {
		v_SDL_HapticStopEffect(haptic, pAfterburn2.id);

		pAfterburn2.eff.periodic.length = (6000 * mag + 400000) / 1000;
		pAfterburn2.eff.periodic.magnitude = (Sint16) fl2i(0x7FFF * (gain / 10000.0f));

		if ( v_SDL_HapticUpdateEffect(haptic, pAfterburn2.id, &pAfterburn2.eff) < 0 ) {
			mprintf(("HapticERROR:  Unable to update pAfterburn2:\n  %s\n", SDL_GetError()));
		}
	}

	joy_ff_start_effect(&pAfterburn1, "Afterburn1 (Fly by)");
	joy_ff_start_effect(&pAfterburn2, "Afterburn2 (Fly by)");
}

void joy_ff_deathroll()
{
	if ( !Joy_ff_enabled ) {
		return;
	}

	if (pDeathroll1.loaded) {
		v_SDL_HapticStopEffect(haptic, pDeathroll1.id);
	}

	if (pDeathroll2.loaded) {
		v_SDL_HapticStopEffect(haptic, pDeathroll2.id);
	}

	joy_ff_start_effect(&pDeathroll1, "Deathroll1");
	joy_ff_start_effect(&pDeathroll2, "Deathroll2");
}
