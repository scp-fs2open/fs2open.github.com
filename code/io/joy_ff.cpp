/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "math/vecmat.h"
#include "io/sw_force.h"
#include "osapi/osregistry.h"
#include "io/joy_ff.h"
#include "osapi/osapi.h"



typedef struct {
	DIEFFECT effect;
	DIPERIODIC periodic_struct;
	DIENVELOPE envelope_struct;
	DWORD axes[2];
	LONG direction[2];
	GUID guid;
} di_periodic_effect_struct;

int joy_ff_handling_scaler;
int Joy_ff_enabled = 0;
int Joy_ff_directional_hit_effect_enabled = 1;

LPDIRECTINPUT pDi;
LPDIRECTINPUTDEVICE2 pDiDevice;

int joy_ff_create_effects();
void joy_ff_stop_effects();

LPDIRECTINPUTEFFECT pHitEffect1;
LPDIRECTINPUTEFFECT pHitEffect2;
LPDIRECTINPUTEFFECT pAfterburn1;
LPDIRECTINPUTEFFECT pAfterburn2;
LPDIRECTINPUTEFFECT pShootEffect;
LPDIRECTINPUTEFFECT pSecShootEffect;
LPDIRECTINPUTEFFECT pSpring;
LPDIRECTINPUTEFFECT pDock;
LPDIRECTINPUTEFFECT pDeathroll1;
LPDIRECTINPUTEFFECT pDeathroll2;
LPDIRECTINPUTEFFECT pExplode;

di_periodic_effect_struct Struct_deathroll1;
di_periodic_effect_struct Struct_deathroll2;
di_periodic_effect_struct Struct_explode;
di_periodic_effect_struct Struct_afterburn1;
di_periodic_effect_struct Struct_afterburn2;
di_periodic_effect_struct Struct_dock;

di_condition_effect_struct Spring_cond_effect;

void joy_ff_afterburn_off();
void init_periodic_effect_struct(di_periodic_effect_struct *effect, int type, int dur, int per, int ang = 0, int mag = 10000, int att = 0, int fade = 0);

int joy_ff_init()
{
	int ff_enabled;

	Joy_ff_enabled = 0;		// Assume no force feedback
	ff_enabled = os_config_read_uint(NULL, "EnableJoystickFF", 0);

	if (ff_enabled) {
		HRESULT hr;
		TCHAR g_szOutput[256];
		TCHAR szCodeString[256];

		Joy_ff_directional_hit_effect_enabled = os_config_read_uint(NULL, "EnableHitEffect", 1);

		hr = SWFF_OpenDefaultFFJoystick((HWND) os_get_window(), &pDi, &pDiDevice);
		if (FAILED(hr)) {
			nprintf(("Sandeep", "No FF On Joystick, not using FF\n"));
			SWFF_ErrorCodeToString(hr, &szCodeString[0]);
			wsprintf(g_szOutput, "Make sure JOYSTICKID1 has Force Feedback\n"
				"Code = %lx: %s\n", hr, szCodeString);

			nprintf(("Sandeep", g_szOutput));
			return -1;
			
		}

		nprintf(("Sandeep", "There is FF on this joystick! (The peasants cheer)\n"));
		SWFF_DestroyAllEffects(pDiDevice);
		if (joy_ff_create_effects())
			return -1;
		Joy_ff_enabled = 1;
	} 

	return 0;
}

void joy_ff_shutdown()
{
	if (Joy_ff_enabled) {
		pSpring->Stop();
		joy_ff_stop_effects();
		SWFF_DestroyAllEffects(pDiDevice);
		pDiDevice->Unacquire();
		pDiDevice->Release();
		pDi->Release();
	}
}

HRESULT joy_ff_handle_error(HRESULT hr, char *eff_name = NULL)
{
	if (FAILED(hr)) {
		TCHAR szCodeString[256];

		SWFF_ErrorCodeToString(hr, szCodeString);
		if (eff_name)
			nprintf(("Joystick", "FF: Error for %s: %s\n", eff_name, szCodeString));
		else
			nprintf(("Joystick", "FF: Error: %s\n", szCodeString));
	}

	return hr;
}

int joy_ff_create_std_periodic(LPDIRECTINPUTEFFECT *eff, int type, int dur, int per, int ang = 0, int mag = 10000, int att = 0, int fade = 0)
{
	joy_ff_handle_error(SWFF_CreatePeriodicEffect(pDiDevice, eff, type, dur, per, ang, mag, 0, att, 0, fade, 0, -1));
	if (!*eff)
		return -1;

	return 0;
}

void joy_ff_start_effect(LPDIRECTINPUTEFFECT eff, char *name)
{
	HRESULT hr;

	nprintf(("Joystick", "FF: Starting effect %s\n", name));
	hr = joy_ff_handle_error(eff->Start(1, 0));
	if (hr == DIERR_INPUTLOST) {
		joy_reacquire_ff();
		joy_ff_handle_error(eff->Start(1, 0));
	}
}

int joy_ff_create_effects()
{	
	joy_ff_handle_error(SWFF_CreateConstantForceEffect(
		pDiDevice,
		&pHitEffect1,
		300000,						// Duration
		0,								// Angle
		10000,						// Magnitude
		0,								// Attack time
		10000,						// Attack level
		120000,						// Fade time
		1,								// Fade level
		-1), "HitEffect1");

	if (pHitEffect1)
		nprintf(("Joystick", "FF: Hit effect 1 loaded\n"));
	else {  // bail out early if we can't even load this (because rest probably won't either and failing is slow.
		nprintf(("Joystick", "FF: Hit effect 1 failed to load\n"));
		return -1;
	}

	joy_ff_create_std_periodic(
		&pHitEffect2,
		SINE,
		300000,						// Duration
		100000,						// Period
		9000,							// Angle
		10000,						// Magnitude
		100000,						// Attack time
		100000);						// Fade time

	if (pHitEffect2)
		nprintf(("Joystick", "FF: Hit effect 2 loaded\n"));
	else
		nprintf(("Joystick", "FF: Hit effect 2 failed to load\n"));

	joy_ff_handle_error(SWFF_CreatePeriodicEffect(
		pDiDevice,
		&pShootEffect,
		SAWTOOTH_DOWN,
		160000,						// Duration
		20000,						// Period
		0,								// Angle
		10000,						// Magnitude
		0,
		0,
		0,
		120000,
		0, -1), "ShootEffect");

	if (pShootEffect)
		nprintf(("Joystick", "FF: Fire primary effect loaded\n"));
	else
		nprintf(("Joystick", "FF: Fire primary effect failed to load\n"));

	joy_ff_handle_error(SWFF_CreateConstantForceEffect(
		pDiDevice,
		&pSecShootEffect,
		200000,						// Duration
		0,								// Angle
		10000,						// Magnitude
		50000,						// Attack time
		10000,						// Attack level
		100000,						// Fade time
		1,								// Fade level
		-1), "SecShootEffect");

	if (pSecShootEffect)
		nprintf(("Joystick", "FF: Fire Secondary effect loaded\n"));
	else
		nprintf(("Joystick", "FF: Fire Secondary effect failed to load\n"));

	joy_ff_handle_error(SWFF_CreateConditionEffectStruct(&Spring_cond_effect,
		pDiDevice,
		&pSpring,
		SPRING,
		INFINITE,			// uS
		100,					// X Coefficient
		0,						// X Offset
		100,					// Y Coefficient
		0,						// Y Offset
		-1),					// button play mask
		"Spring");

	if (pSpring) {
		nprintf(("Joystick", "FF: Spring effect loaded\n"));
		joy_ff_start_effect(pSpring, "Spring");

	} else
		nprintf(("Joystick", "FF: Spring effect failed to load\n"));

	init_periodic_effect_struct(
		&Struct_afterburn1,
		SINE,
		INFINITE,					// Duration
		20000,						// Period
		0,								// Angle
		8000);						// Magnitude

	pDiDevice->CreateEffect(Struct_afterburn1.guid, &Struct_afterburn1.effect, &pAfterburn1, NULL);
	if (pAfterburn1)
		nprintf(("Joystick", "FF: Afterburner effect 1 loaded\n"));
	else
		nprintf(("Joystick", "FF: Afterburner effect 1 failed to load\n"));

	init_periodic_effect_struct(
		&Struct_afterburn2,
		SINE,
		INFINITE,					// Duration
		120000,						// Period
		9000,							// Angle
		4400);						// Magnitude

	pDiDevice->CreateEffect(Struct_afterburn2.guid, &Struct_afterburn2.effect, &pAfterburn2, NULL);
	if (pAfterburn2)
		nprintf(("Joystick", "FF: Afterburner effect 2 loaded\n"));
	else
		nprintf(("Joystick", "FF: Afterburner effect 2 failed to load\n"));

	init_periodic_effect_struct(
		&Struct_dock,
		SQUARE_HIGH,
		125000,						// Duration
		100000,						// Period
		9000,							// Angle
		4000);						// Magnitude

	pDiDevice->CreateEffect(Struct_dock.guid, &Struct_dock.effect, &pDock, NULL);
	if (pDock)
		nprintf(("Joystick", "FF: Dock effect loaded\n"));
	else
		nprintf(("Joystick", "FF: Dock effect failed to load\n"));

	init_periodic_effect_struct(
		&Struct_explode,
		SAWTOOTH_DOWN,
		500000,						// Duration
		20000,						// Period
		9000,							// Angle
		10000,						// Magnitude
		0,								// Attack time
		500000);						// Fade time

	pDiDevice->CreateEffect(Struct_explode.guid, &Struct_explode.effect, &pExplode, NULL);
	if (pExplode)
		nprintf(("Joystick", "FF: Explosion effect loaded\n"));
	else
		nprintf(("Joystick", "FF: Explosion effect failed to load\n"));

	init_periodic_effect_struct(
		&Struct_deathroll1,
		SINE,
		INFINITE,					// Duration
		200000,						// Period
		0,								// Angle
		10000,						// Magnitude
		2000000,						// Attack time
		0);							// Fade time

	pDiDevice->CreateEffect(Struct_deathroll1.guid, &Struct_deathroll1.effect, &pDeathroll1, NULL);
	if (pDeathroll1)
		nprintf(("Joystick", "FF: Deathroll effect 1 loaded\n"));
	else
		nprintf(("Joystick", "FF: Deathroll effect 1 failed to load\n"));

	init_periodic_effect_struct(
		&Struct_deathroll2,
		SINE,
		INFINITE,					// Duration
		200000,						// Period
		9000,							// Angle
		10000,						// Magnitude
		2000000,						// Attack time
		0);							// Fade time

	pDiDevice->CreateEffect(Struct_deathroll2.guid, &Struct_deathroll2.effect, &pDeathroll2, NULL);
	if (pDeathroll2)
		nprintf(("Joystick", "FF: Deathroll effect 2 loaded\n"));
	else
		nprintf(("Joystick", "FF: Deathroll effect 2 failed to load\n"));

	return 0;
}

void joy_ff_stop_effects()
{
	joy_ff_afterburn_off();

	if (pDeathroll1)
		pDeathroll1->Stop();

	if (pDeathroll2)
		pDeathroll2->Stop();
}

void joy_ff_mission_init(vec3d v)
{
	v.xyz.z = 0.0f;
//	joy_ff_handling_scaler = (int) ((vm_vec_mag(&v) - 1.3f) * 10.5f);
	joy_ff_handling_scaler = (int) ((vm_vec_mag(&v) + 1.3f) * 5.0f);
//	joy_ff_handling_scaler = (int) (vm_vec_mag(&v) * 7.5f);
}

void joy_ff_adjust_handling(int speed)
{
	int v;

	v = speed * joy_ff_handling_scaler * 2 / 3;
//	v += joy_ff_handling_scaler * joy_ff_handling_scaler * 6 / 7 + 250;
	v += joy_ff_handling_scaler * 45 - 500;
	if (v > 10000)
		v = 10000;

	if (pSpring) {
		if (Spring_cond_effect.DIConditionStruct[0].lPositiveCoefficient != v) {
			HRESULT hr;

			Spring_cond_effect.DIConditionStruct[0].lPositiveCoefficient = v;
			Spring_cond_effect.DIConditionStruct[0].lNegativeCoefficient = v;
			Spring_cond_effect.DIConditionStruct[1].lPositiveCoefficient = v;
			Spring_cond_effect.DIConditionStruct[1].lNegativeCoefficient = v;
			nprintf(("Joystick", "FF: New handling force = %d\n", v));

			hr = joy_ff_handle_error(pSpring->SetParameters(&Spring_cond_effect.DIEffectStruct, DIEP_TYPESPECIFICPARAMS), "Spring");
			if (hr == DIERR_INPUTLOST) {
				joy_reacquire_ff();
				joy_ff_handle_error(pSpring->SetParameters(&Spring_cond_effect.DIEffectStruct, DIEP_TYPESPECIFICPARAMS), "Spring");
			}
		}
	}
}

void joy_ff_change_effect(di_periodic_effect_struct *s, LPDIRECTINPUTEFFECT eff, int gain = -1, int dur = 0, int flags = -1)
{
	int reload = 0;

	if ((gain >= 0) && ((int) s->effect.dwGain != gain)) {
		s->effect.dwGain = gain;
		nprintf(("Joystick", "FF: Gain reset to %d\n", gain));
		reload = 1;
	}

	if (dur && ((int) s->effect.dwDuration != dur)) {
		s->effect.dwDuration = dur;
		nprintf(("Joystick", "FF: Duration reset to %d\n", dur));
		reload = 1;
	}

	if (flags == -1) {
		flags = DIEP_DURATION | DIEP_SAMPLEPERIOD | DIEP_GAIN | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS;
		nprintf(("Joystick", "FF: Doing full reload of effect\n"));
	}

	if (flags != (DIEP_DURATION | DIEP_GAIN))
		reload = 1;

	if (reload) {
		HRESULT hr;

		nprintf(("Joystick", "FF: Swapping in a new effect\n"));
		hr = joy_ff_handle_error(eff->SetParameters(&s->effect, flags));
		if (hr == DIERR_INPUTLOST) {
			joy_reacquire_ff();
			joy_ff_handle_error(eff->SetParameters(&s->effect, flags));
		}

	} else
		nprintf(("Joystick", "FF: Swap effect requested, but nothing changed\n"));
}

int joy_ff_effect_playing(LPDIRECTINPUTEFFECT eff)
{
	DWORD flags;

	eff->GetEffectStatus(&flags);
	return (flags & DIEGES_PLAYING);
}

void joy_ff_docked()
{
	if (pDock) {
		pDock->Stop();
		if (joy_ff_handle_error(SWFF_SetGain(pDock, 10000), "Dock") == DIERR_INPUTLOST) {
			joy_reacquire_ff();
			joy_ff_handle_error(SWFF_SetGain(pDock, 10000), "Dock");
		}

		joy_ff_start_effect(pDock, "Dock");
	}
}

void joy_ff_play_reload_effect()
{
	if (pDock) {
		pDock->Stop();
		if (joy_ff_handle_error(SWFF_SetGain(pDock, 5000), "Dock (Reload)") == DIERR_INPUTLOST) {
			joy_reacquire_ff();
			joy_ff_handle_error(SWFF_SetGain(pDock, 5000), "Dock (Reload)");
		}

		joy_ff_start_effect(pDock, "Dock (Reload)");
	}
}

int Joy_ff_afterburning = 0;

void joy_ff_afterburn_on()
{
	if (pAfterburn1) {
		pAfterburn1->Stop();
		joy_ff_change_effect(&Struct_afterburn1, pAfterburn1, 5000, INFINITE, DIEP_DURATION | DIEP_GAIN);
		joy_ff_start_effect(pAfterburn1, "Afterburn1");
	}

	if (pAfterburn2) {
		pAfterburn2->Stop();
		joy_ff_change_effect(&Struct_afterburn2, pAfterburn2, 5000, INFINITE, DIEP_DURATION | DIEP_GAIN);
		joy_ff_start_effect(pAfterburn2, "Afterburn2");
	}

	nprintf(("Joystick", "FF: Afterburn started\n"));
	Joy_ff_afterburning = 1;
}

void joy_ff_afterburn_off()
{
	if (!Joy_ff_afterburning)
		return;

	if (pAfterburn1) {
		pAfterburn1->Stop();
	}

	if (pAfterburn2) {
		pAfterburn2->Stop();
	}

	Joy_ff_afterburning = 0;
	nprintf(("Joystick", "FF: Afterburn stopped\n"));
}

void joy_ff_deathroll()
{
	if (pDeathroll1) {
		pDeathroll1->Stop();
		joy_ff_start_effect(pDeathroll1, "Deathroll1");
	}

	if (pDeathroll2) {
		pDeathroll2->Stop();
		joy_ff_start_effect(pDeathroll2, "Deathroll2");
	}
}

void joy_ff_explode()
{
	if (pDeathroll1)
		pDeathroll1->Stop();

	if (pDeathroll2)
		pDeathroll2->Stop();

	if (pExplode) {
		pExplode->Stop();
		joy_ff_start_effect(pExplode, "Explode");
	}
}

void joy_ff_fly_by(int mag)
{
	int gain;

	if (Joy_ff_afterburning)
		return;

	gain = mag * 120 + 4000;
	if (gain > 10000)
		gain = 10000;

	if (pAfterburn1) {
		pAfterburn1->Stop();
		joy_ff_change_effect(&Struct_afterburn1, pAfterburn1, gain, 6000 * mag + 400000, DIEP_DURATION | DIEP_GAIN);
		joy_ff_start_effect(pAfterburn1, "Afterburn1 (Fly by)");
	}

	if (pAfterburn2) {
		pAfterburn2->Stop();
		joy_ff_change_effect(&Struct_afterburn2, pAfterburn2, gain, 6000 * mag + 400000, DIEP_DURATION | DIEP_GAIN);
		joy_ff_start_effect(pAfterburn2, "Afterburn2 (Fly by)");
	}
}

void joy_reacquire_ff()
{
	if (!Joy_ff_enabled)
		return;

	nprintf(("Joystick", "FF: Reacquiring\n"));
	pDiDevice->Acquire();
	joy_ff_start_effect(pSpring, "Spring");
}

void joy_unacquire_ff()
{
}

void joy_ff_play_dir_effect(float x, float y)
{
	int idegs, imag;
	float degs;

	if (!Joy_ff_enabled)
		return;

	if (!pHitEffect1 || !pHitEffect2)
		return;

	if (joy_ff_effect_playing(pHitEffect1) || joy_ff_effect_playing(pHitEffect2)) {
		nprintf(("Joystick", "FF: HitEffect already playing.  Skipping\n"));
		return;
	}

	if (Joy_ff_directional_hit_effect_enabled) {
		if (x > 8000.0f)
			x = 8000.0f;
		else if (x < -8000.0f)
			x = -8000.0f;

		if (y > 8000.0f)
			y = 8000.0f;
		else if (y < -8000.0f)
			y = -8000.0f;

		imag = (int) fl_sqrt(x * x + y * y);
		if (imag > 10000)
			imag = 10000;

		degs = (float)atan2(x, y);
		idegs = (int) (degs * 18000.0f / PI) + 90;
		while (idegs < 0)
			idegs += 36000;

		while (idegs >= 36000)
			idegs -= 36000;

		if (joy_ff_handle_error(SWFF_SetDirectionGain(pHitEffect1, idegs, imag), "HitEffect1") == DIERR_INPUTLOST) {
			joy_reacquire_ff();
			joy_ff_handle_error(SWFF_SetDirectionGain(pHitEffect1, idegs, imag), "HitEffect1");
		}

		idegs += 9000;
		if (idegs >= 36000)
			idegs -= 36000;

		if (joy_ff_handle_error(SWFF_SetDirectionGain(pHitEffect2, idegs, imag), "HitEffect2") == DIERR_INPUTLOST) {
			joy_reacquire_ff();
			joy_ff_handle_error(SWFF_SetDirectionGain(pHitEffect2, idegs, imag), "HitEffect2");
		}
	}

	joy_ff_start_effect(pHitEffect1, "HitEffect1");
	joy_ff_start_effect(pHitEffect2, "HitEffect2");
	//nprintf(("Joystick", "FF: Dir: %d, Mag = %d\n", idegs, imag));
}

void joy_ff_play_vector_effect(vec3d *v, float scaler)
{
	vec3d vf;
	float x, y;

	nprintf(("Joystick", "FF: vec = { %f, %f, %f } s = %f\n", v->xyz.x, v->xyz.y, v->xyz.z, scaler));
	vm_vec_copy_scale(&vf, v, scaler);
	x = vf.xyz.x;
	vf.xyz.x = 0.0f;

	if (vf.xyz.y + vf.xyz.z < 0)
		y = -vm_vec_mag(&vf);
	else
		y = vm_vec_mag(&vf);

	joy_ff_play_dir_effect(-x, -y);
}

static int secondary_ff_level = 0;

void joy_ff_play_secondary_shoot(int gain)
{
	if (!Joy_ff_enabled)
		return;

	if (!pSecShootEffect)
		return;

	gain = gain * 100 + 2500;
	if (gain > 10000)
		gain = 10000;

	if (gain != secondary_ff_level) {
		if (joy_ff_handle_error(SWFF_SetGain(pSecShootEffect, gain), "SecShootEffect") == DIERR_INPUTLOST) {
			joy_reacquire_ff();
			joy_ff_handle_error(SWFF_SetGain(pSecShootEffect, gain), "SecShootEffect");
		}

		if (joy_ff_handle_error(SWFF_SetDuration(pSecShootEffect, 150000 + gain * 25), "SecShootEffect") == DIERR_INPUTLOST) {
			joy_reacquire_ff();
			joy_ff_handle_error(SWFF_SetDuration(pSecShootEffect, 150000 + gain * 25), "SecShootEffect");
		}

		secondary_ff_level = gain;
		nprintf(("Joystick", "FF: Secondary force = %d\n", gain));
	}

	pSecShootEffect->Stop();
	joy_ff_start_effect(pSecShootEffect, "SecShootEffect");
}

static int primary_ff_level = 0;

void joy_ff_play_primary_shoot(int gain)
{
	if (!Joy_ff_enabled)
		return;

	if (!pShootEffect)
		return;

	if (gain > 10000)
		gain = 10000;

	if (gain != primary_ff_level) {
		if (joy_ff_handle_error(SWFF_SetGain(pShootEffect, gain), "ShootEffect") == DIERR_INPUTLOST) {
			joy_reacquire_ff();
			joy_ff_handle_error(SWFF_SetGain(pShootEffect, gain), "ShootEffect");
		}

		primary_ff_level = gain;
	}

	pShootEffect->Stop();
	joy_ff_start_effect(pShootEffect, "ShootEffect");
}

void init_periodic_effect_struct(di_periodic_effect_struct *effect, int type, int dur, int per, int ang, int mag, int att, int fade)
{
	// type-specific stuff
	DWORD dwPhase = 0;
	GUID guid = GUID_Square;

	switch (type) {
		case SINE:
			guid = GUID_Sine;
			break;
		case COSINE:
			guid = GUID_Sine;
			dwPhase = 9000;
			break;
		case SQUARE_HIGH:
			guid = GUID_Square;
			break;
		case SQUARE_LOW:
			guid = GUID_Square;
			dwPhase = 18000;
			break;
		case TRIANGLE_UP:
			guid = GUID_Triangle;
			break;
		case TRIANGLE_DOWN:
			guid = GUID_Triangle;
			dwPhase = 18000;
			break;
		case SAWTOOTH_UP:
			guid = GUID_SawtoothUp;
			break;
		case SAWTOOTH_DOWN:
			guid = GUID_SawtoothDown;
			break;
		default:
			Int3();  // illegal
			break;
	}

	effect->guid = guid;
	effect->periodic_struct.dwMagnitude = mag;
	effect->periodic_struct.lOffset = 0;
	effect->periodic_struct.dwPhase = dwPhase;
	effect->periodic_struct.dwPeriod = per;

	effect->envelope_struct.dwSize = sizeof(DIENVELOPE);
	effect->envelope_struct.dwAttackTime = att;
	effect->envelope_struct.dwAttackLevel = 0;
	effect->envelope_struct.dwFadeTime = fade;
	effect->envelope_struct.dwFadeLevel = 0;

	effect->axes[0] = DIJOFS_X;
	effect->axes[1] = DIJOFS_Y;

	effect->direction[0] = ang;
	effect->direction[1] = 0;

	effect->effect.dwSize						= sizeof(DIEFFECT);
	effect->effect.dwFlags						= DIEFF_OBJECTOFFSETS | DIEFF_POLAR;
	effect->effect.dwDuration					= dur;
	effect->effect.dwSamplePeriod				= HZ_TO_uS(100);
	effect->effect.dwGain						= 10000;
	effect->effect.dwTriggerButton			= DIEB_NOTRIGGER;
	effect->effect.dwTriggerRepeatInterval	= 0;
	effect->effect.cAxes							= 2;
	effect->effect.rgdwAxes						= effect->axes;
	effect->effect.rglDirection				= effect->direction;
	effect->effect.lpEnvelope					= &effect->envelope_struct;
	effect->effect.cbTypeSpecificParams		= sizeof(effect->periodic_struct);
	effect->effect.lpvTypeSpecificParams	= &effect->periodic_struct;
}
