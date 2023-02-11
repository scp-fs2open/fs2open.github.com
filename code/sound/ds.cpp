/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "sound/audiostr.h"
#include "sound/channel.h"
#include "sound/ds.h"
#include "sound/ds3d.h"
#include "sound/dscap.h"
#include "sound/openal.h"
#include "sound/sound.h" // jg18 - for enhanced sound


typedef struct sound_buffer
{
	ALuint buf_id;		// OpenAL buffer id
	int channel_id;		// Channel[] index this buffer is currently bound to

	int frequency;
	int bits_per_sample;
	int nchannels;
	int nseconds;
	int nbytes;

	sound_buffer():
		buf_id(0), channel_id(-1), frequency(0), bits_per_sample(0),
		nchannels(0), nseconds(0), nbytes(0)
	{
	}
} sound_buffer;


static int MAX_CHANNELS;		// initialized properly in ds_init_channels()
channel *Channels = NULL;
static int channel_next_sig = 1;

const int BUFFER_BUMP = 50;
SCP_vector<sound_buffer> sound_buffers;

static int Ds_use_eax = 0;

static int Ds_eax_inited = 0;

static int AL_play_position = 0;

// NOTE: these can't be static
int Ds_sound_quality = DS_SQ_MEDIUM;
int Ds_float_supported = 0;


// this is so stupid - required to get VC6 to use the following array initializer
EFXREVERBPROPERTIES::EFXREVERBPROPERTIES(const EFXREVERBPROPERTIES_list &list)
{
	name = list.name;
	flDensity = list.flDensity;
	flDiffusion = list.flDiffusion;
	flGain = list.flGain;
	flGainHF = list.flGainHF;
	flGainLF = list.flGainLF;
	flDecayTime = list.flDecayTime;
	flDecayHFRatio = list.flDecayHFRatio;
	flDecayLFRatio = list.flDecayLFRatio;
	flReflectionsGain = list.flReflectionsGain;
	flReflectionsDelay = list.flReflectionsDelay;
	flReflectionsPan[0] = list.flReflectionsPan[0];
	flReflectionsPan[1] = list.flReflectionsPan[1];
	flReflectionsPan[2] = list.flReflectionsPan[2];
	flLateReverbGain = list.flLateReverbGain;
	flLateReverbDelay = list.flLateReverbDelay;
	flLateReverbPan[0] = list.flLateReverbPan[0];
	flLateReverbPan[1] = list.flLateReverbPan[1];
	flLateReverbPan[2] = list.flLateReverbPan[2];
	flEchoTime = list.flEchoTime;
	flEchoDepth = list.flEchoDepth;
	flModulationTime = list.flModulationTime;
	flModulationDepth = list.flModulationDepth;
	flAirAbsorptionGainHF = list.flAirAbsorptionGainHF;
	flHFReference = list.flHFReference;
	flLFReference = list.flLFReference;
	flRoomRolloffFactor = list.flRoomRolloffFactor;
	iDecayHFLimit = list.iDecayHFLimit;
}

static const EFXREVERBPROPERTIES_list EFX_Reverb_Defaults[EAX_ENVIRONMENT_COUNT] =
{
	{ "Generic", 1.0f, 1.0f, 0.316228f, 0.891251f, 1.0f, 1.49f, 0.83f, 1.0f, 0.050003f, 0.007f, {0.0f, 0.0f, 0.0f}, 1.258925f, 0.011f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Padded cell", 0.171500f, 1.0f, 0.316228f, 0.001f, 1.0f, 0.17f, 0.10f, 1.0f, 0.250035f, 0.001f, {0.0f, 0.0f, 0.0f}, 1.269112f, 0.002f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Room", 0.428687f, 1.0f, 0.316228f, 0.592925f, 1.0f, 0.40f, 0.83f, 1.0f, 0.150314f, 0.002f, {0.0f, 0.0f, 0.0f}, 1.062919f, 0.003f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Bathroom", 0.171500f, 1.0f, 0.316228f, 0.251189f, 1.0f, 1.49f, 0.54f, 1.0f, 0.653131f, 0.007f, {0.0f, 0.0f, 0.0f}, 3.273407f, 0.011f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Living room", 0.976563f, 1.0f, 0.316228f, 0.001f, 1.0f, 0.50f, 0.10f, 1.0f, 0.205116f, 0.003f, {0.0f, 0.0f, 0.0f}, 0.280543f, 0.004f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Stone room", 1.0f, 1.0f, 0.316228f, 0.707946f, 1.0f, 2.31f, 0.64f, 1.0f, 0.441062f, 0.012f, {0.0f, 0.0f, 0.0f}, 1.100272f, 0.017f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Auditorium", 1.0f, 1.0f, 0.316228f, 0.578096f, 1.0f, 4.32f, 0.59f, 1.0f, 0.403181f, 0.02f, {0.0f, 0.0f, 0.0f}, 0.716968f, 0.03f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Concert hall", 1.0f, 1.0f, 0.316228f, 0.562341f, 1.0f, 3.92f, 0.70f, 1.0f, 0.242661f, 0.02f, {0.0f, 0.0f, 0.0f}, 0.997700f, 0.029f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Cave", 1.0f, 1.0f, 0.316228f, 1.0f, 1.0f, 2.91f, 1.30f, 1.0f, 0.500035f, 0.015f, {0.0f, 0.0f, 0.0f}, 0.706318f, 0.022f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 0 },
	{ "Arena", 1.0f, 1.0f, 0.316228f, 0.447713f, 1.0f, 7.24f, 0.33f, 1.0f, 0.261216f, 0.02f, {0.0f, 0.0f, 0.0f}, 1.018591f, 0.03f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Hangar", 1.0f, 1.0f, 0.316228f, 0.316228f, 1.0f, 10.05f, 0.23f, 1.0f, 0.500035f, 0.02f, {0.0f, 0.0f, 0.0f}, 1.256030f, 0.03f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Carpeted hallway", 0.428687f, 1.0f, 0.316228f, 0.01f, 1.0f, 0.30f, 0.10f, 1.0f, 0.121479f, 0.002f, {0.0f, 0.0f, 0.0f}, 0.153109f, 0.03f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Hallway", 0.364500f, 1.0f, 0.316228f, 0.707946f, 1.0f, 1.49f, 0.59f, 1.0f, 0.245754f, 0.007f, {0.0f, 0.0f, 0.0f}, 1.661499f, 0.011f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Stone corridor", 1.0f, 1.0f, 0.316228f, 0.761202f, 1.0f, 2.70f, 0.79f, 1.0f, 0.247172f, 0.013f, {0.0f, 0.0f, 0.0f}, 1.575796f, 0.02f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Alley", 1.0f, 0.30f, 0.316228f, 0.732825f, 1.0f, 1.49f, 0.86f, 1.0f, 0.250035f, 0.007f, {0.0f, 0.0f, 0.0f}, 0.995405f, 0.011f, {0.0f, 0.0f, 0.0f}, 0.125f, 0.95f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Forest", 1.0f, 0.30f, 0.316228f, 0.022387f, 1.0f, 1.49f, 0.54f, 1.0f, 0.052481f, 0.162f, {0.0f, 0.0f, 0.0f}, 0.768245f, 0.088f, {0.0f, 0.0f, 0.0f}, 0.125f, 1.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "City", 1.0f, 0.50f, 0.316228f, 0.398107f, 1.0f, 1.49f, 0.67f, 1.0f, 0.073030f, 0.007f, {0.0f, 0.0f, 0.0f}, 0.142725f, 0.011f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Mountains", 1.0f, 0.27f, 0.316228f, 0.056234f, 1.0f, 1.49f, 0.21f, 1.0f, 0.040738f, 0.30f, {0.0f, 0.0f, 0.0f}, 0.191867f, 0.10f, {0.0f, 0.0f, 0.0f}, 0.25f, 1.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 0 },
	{ "Quarry", 1.0f, 1.0f, 0.316228f, 0.316228f, 1.0f, 1.49f, 0.83f, 1.0f, 0.0f, 0.061f, {0.0f, 0.0f, 0.0f}, 1.778279f, 0.025f, {0.0f, 0.0f, 0.0f}, 0.125f, 0.70f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Plain", 1.0f, 0.21f, 0.316228f, 0.10f, 1.0f, 1.49f, 0.50f, 1.0f, 0.058479f, 0.179f, {0.0f, 0.0f, 0.0f}, 0.108893f, 0.10f, {0.0f, 0.0f, 0.0f}, 0.25f, 1.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Parking lot", 1.0f, 1.0f, 0.316228f, 1.0f, 1.0f, 1.65f, 1.50f, 1.0f, 0.208209f, 0.008f, {0.0f, 0.0f, 0.0f}, 0.265155f, 0.012f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 0 },
	{ "Sewer pipe", 0.307063f, 0.80f, 0.316228f, 0.316228f, 1.0f, 2.81f, 0.14f, 1.0f, 1.638702f, 0.014f, {0.0f, 0.0f, 0.0f}, 3.247133f, 0.021f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 0.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Underwater", 0.364500f, 1.0f, 0.316228f, 0.01f, 1.0f, 1.49f, 0.10f, 1.0f, 0.596348f, 0.007f, {0.0f, 0.0f, 0.0f}, 7.079458f, 0.011f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 1.18f, 0.348f, 0.994260f, 5000.0f, 250.0f, 0.0f, 1 },
	{ "Drugged", 0.428687f, 0.50f, 0.316228f, 1.0f, 1.0f, 8.39f, 1.39f, 1.0f, 0.875992f, 0.002f, {0.0f, 0.0f, 0.0f}, 3.108136f, 0.03f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 0.25f, 1.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 0 },
	{ "Dizzy", 0.364500f, 0.60f, 0.316228f, 0.630957f, 1.0f, 17.23f, 0.56f, 1.0f, 0.139155f, 0.02f, {0.0f, 0.0f, 0.0f}, 0.493742f, 0.03f, {0.0f, 0.0f, 0.0f}, 0.25f, 1.0f, 0.81f, 0.31f, 0.994260f, 5000.0f, 250.0f, 0.0f, 0 },
	{ "Psychotic", 0.062500f, 0.50f, 0.316228f, 0.840427f, 1.0f, 7.56f, 0.91f, 1.0f, 0.486407f, 0.02f, {0.0f, 0.0f, 0.0f}, 2.437811f, 0.03f, {0.0f, 0.0f, 0.0f}, 0.25f, 0.0f, 4.00f, 1.0f, 0.994260f, 5000.0f, 250.0f, 0.0f, 0 }
};

SCP_vector<EFXREVERBPROPERTIES> EFX_presets;


typedef ALvoid (AL_APIENTRY * ALGENFILTERS) (ALsizei, ALuint*);
typedef ALvoid (AL_APIENTRY * ALDELETEFILTERS) (ALsizei, ALuint*);
typedef ALvoid (AL_APIENTRY * ALFILTERI) (ALuint, ALenum, ALint);
typedef ALvoid (AL_APIENTRY * ALGENEFFECTS) (ALsizei, ALuint*);
typedef ALvoid (AL_APIENTRY * ALDELETEEFFECTS) (ALsizei, ALuint*);
typedef ALvoid (AL_APIENTRY * ALEFFECTI) (ALuint, ALenum, ALint);
typedef ALvoid (AL_APIENTRY * ALEFFECTF) (ALuint, ALenum, ALfloat);
typedef ALvoid (AL_APIENTRY * ALEFFECTFV) (ALuint, ALenum, ALfloat*);
typedef ALvoid (AL_APIENTRY * ALGETEFFECTF) (ALuint, ALenum, ALfloat*);
typedef ALvoid (AL_APIENTRY * ALGENAUXILIARYEFFECTSLOTS) (ALsizei, ALuint*);
typedef ALvoid (AL_APIENTRY * ALDELETEAUXILIARYEFFECTSLOTS) (ALsizei, ALuint*);
typedef ALboolean (AL_APIENTRY * ALISAUXILIARYEFFECTSLOT) (ALuint);
typedef ALvoid (AL_APIENTRY * ALAUXILIARYEFFECTSLOTI) (ALuint, ALenum, ALint);
typedef ALvoid (AL_APIENTRY * ALAUXILIARYEFFECTSLOTIV) (ALuint, ALenum, ALint*);
typedef ALvoid (AL_APIENTRY * ALAUXILIARYEFFECTSLOTF) (ALuint, ALenum, ALfloat);
typedef ALvoid (AL_APIENTRY * ALAUXILIARYEFFECTSLOTFV) (ALuint, ALenum, ALfloat*);


ALGENFILTERS v_alGenFilters = NULL;
ALDELETEFILTERS v_alDeleteFilters = NULL;

ALFILTERI v_alFilteri = NULL;

ALGENEFFECTS v_alGenEffecs = NULL;
ALDELETEEFFECTS v_alDeleteEffects = NULL;

ALEFFECTI v_alEffecti = NULL;
ALEFFECTF v_alEffectf = NULL;
ALEFFECTFV v_alEffectfv = NULL;
ALGETEFFECTF v_alGetEffectf = NULL;

ALGENAUXILIARYEFFECTSLOTS v_alGenAuxiliaryEffectSlots = NULL;
ALDELETEAUXILIARYEFFECTSLOTS v_alDeleteAuxiliaryEffectSlots = NULL;

ALISAUXILIARYEFFECTSLOT v_alIsAuxiliaryEffectSlot = NULL;
ALAUXILIARYEFFECTSLOTI v_alAuxiliaryEffectSloti = NULL;
ALAUXILIARYEFFECTSLOTIV v_alAuxiliaryEffectSlotiv = NULL;
ALAUXILIARYEFFECTSLOTF v_alAuxiliaryEffectSlotf = NULL;
ALAUXILIARYEFFECTSLOTFV v_alAuxiliaryEffectSlotfv = NULL;

ALCdevice *ds_sound_device = NULL;
ALCcontext *ds_sound_context = NULL;

ALuint AL_EFX_aux_id = 0;

static ALuint AL_EFX_effect_id = 0;
static bool Ds_active_env = false;
static size_t Ds_active_env_idx = 0;


static void *al_load_function(const char *func_name)
{
	void *func = alGetProcAddress(func_name);
	if ( !func ) {
		throw std::runtime_error(func_name);
	}
	return func;
}

static void al_efx_load_preset(size_t presetid)
{
	if ( !Ds_eax_inited ) {
		return;
	}

	if (presetid >= EFX_presets.size()) {
		return;
	}

	EFXREVERBPROPERTIES *prop = &EFX_presets[presetid];

	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DENSITY, prop->flDensity) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DIFFUSION, prop->flDiffusion) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_GAIN, prop->flGain) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_GAINHF, prop->flGainHF) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_GAINLF, prop->flGainLF) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_TIME, prop->flDecayTime) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_HFRATIO, prop->flDecayHFRatio) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_LFRATIO, prop->flDecayLFRatio) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_REFLECTIONS_GAIN, prop->flReflectionsGain) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_REFLECTIONS_DELAY, prop->flReflectionsDelay) );
	OpenAL_ErrorPrint( v_alEffectfv(AL_EFX_effect_id, AL_EAXREVERB_REFLECTIONS_PAN, prop->flReflectionsPan) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_LATE_REVERB_GAIN, prop->flLateReverbGain) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_LATE_REVERB_DELAY, prop->flLateReverbDelay) );
	OpenAL_ErrorPrint( v_alEffectfv(AL_EFX_effect_id, AL_EAXREVERB_LATE_REVERB_PAN, prop->flLateReverbPan) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_ECHO_TIME, prop->flEchoTime) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_ECHO_DEPTH, prop->flEchoDepth) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_MODULATION_TIME, prop->flModulationTime) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_MODULATION_DEPTH, prop->flModulationDepth) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, prop->flAirAbsorptionGainHF) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_HFREFERENCE, prop->flHFReference) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_LFREFERENCE, prop->flLFReference) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, prop->flRoomRolloffFactor) );
	OpenAL_ErrorPrint( v_alEffecti(AL_EFX_effect_id, AL_EAXREVERB_DECAY_HFLIMIT, prop->iDecayHFLimit) );

	Ds_active_env_idx = presetid;
	Ds_active_env = true;
}


int ds_initialized = FALSE;

/**
 * 
 */
int ds_get_sid()
{
	sound_buffer new_buffer;
	uint i;

	for (i = 0; i < sound_buffers.size(); i++) {
		if (sound_buffers[i].buf_id == 0) {
			return (int)i;
		}
	}

	sound_buffers.push_back( new_buffer );

	return (int)(sound_buffers.size() - 1);
}

int ds_load_buffer(int *sid, int  /*flags*/, sound::IAudioFile* file)
{
	Assert(sid != NULL);
	Assert(file != NULL);

	// All sounds are required to have a software buffer
	*sid = ds_get_sid();
	if (*sid == -1) {
		nprintf(("Sound", "SOUND ==> No more sound buffers available\n"));
		return -1;
	}

	ALuint pi;
	OpenAL_ErrorCheck(alGenBuffers(1, &pi), return -1);

	const auto fileProps = file->getFileProperties();

	ALenum format;
	ALsizei size = fileProps.total_samples * fileProps.bytes_per_sample * fileProps.num_channels;
	ALint n_channels = fileProps.num_channels;
	ALsizei frequency;
		
	// format is now in pcm
	frequency = fileProps.sample_rate;
	format = openal_get_format(fileProps.bytes_per_sample * 8, fileProps.num_channels);

	if (format == AL_INVALID_VALUE) {
		return -1;
	}

	SCP_vector<uint8_t> audio_buffer;
	audio_buffer.reserve(size);

	SCP_vector<uint8_t> buffer(fileProps.sample_rate * fileProps.bytes_per_sample * fileProps.num_channels);
	int read;
	while((read = file->Read(&buffer[0], buffer.size())) >= 0) {
		if (read == 0) {
			// buffer not large enough
			buffer.resize(buffer.size() * 2);
		} else {
			audio_buffer.insert(audio_buffer.end(), buffer.begin(), std::next(buffer.begin(), read));
		}
	}

	Snd_sram += audio_buffer.size();

	OpenAL_ErrorCheck(alBufferData(pi, format, audio_buffer.data(), (ALsizei)audio_buffer.size(), frequency), return -1; );

	sound_buffers[*sid].buf_id = pi;
	sound_buffers[*sid].channel_id = -1;
	sound_buffers[*sid].frequency = frequency;
	sound_buffers[*sid].bits_per_sample = fileProps.bytes_per_sample * 8;
	sound_buffers[*sid].nchannels = n_channels;
	sound_buffers[*sid].nseconds = fl2i(fileProps.duration);
	sound_buffers[*sid].nbytes = (int)audio_buffer.size();

	return 0;
}

/**
 * Initialise the ::Channels[] array dynamically based on system resources.
 */
void ds_init_channels()
{
	// Legacy assumed safe value if for some reason dynamicly setting it does not work
	MAX_CHANNELS = 128;

	if (Cmdline_no_enhanced_sound)
		MAX_CHANNELS = 32; // Old sound code limts
	else {
		ALCint size;
		alcGetIntegerv(ds_sound_device, ALC_ATTRIBUTES_SIZE, 1, &size);
		std::vector<ALCint> attrs(size);
		alcGetIntegerv(ds_sound_device, ALC_ALL_ATTRIBUTES, size, &attrs[0]);
		for (size_t i = 0; i < attrs.size(); ++i) {
			//We reserve a portion of the reported channels to avoid colliding with other types of sound
			//Using the 32 channels of the no_enhanced_sound as a floor.
			if (attrs[i] == ALC_MONO_SOURCES && attrs[i+1] - DS_RESERVED_CHANNELS > 32) {
				MAX_CHANNELS = attrs[i + 1] - DS_RESERVED_CHANNELS;
				mprintf(("  ALC reported %i available sources, setting max to use at %i ", attrs[i + 1],MAX_CHANNELS));
			}
		}
	}
	try {
		Channels = new channel[MAX_CHANNELS];
	} catch (const std::bad_alloc&) {
		Error(LOCATION, "Unable to allocate " SIZE_T_ARG " bytes for %d audio channels.", sizeof(channel) * MAX_CHANNELS, MAX_CHANNELS);
	}
}

/**
 * Initialise the both the software and hardware buffers
 */
void ds_init_buffers()
{
	sound_buffers.clear();

	// pre-allocate for at least BUFFER_BUMP buffers
	sound_buffers.reserve( BUFFER_BUMP );
}

/**
* Check if the player is using OpenAL Soft,
* which is required to use enhanced sound.
* Returns true on success, false otherwise.
*/
bool ds_check_for_openal_soft()
{	
	const ALchar * renderer = alGetString(AL_RENDERER);
	if (renderer == NULL)
	{
		mprintf(("ds_check_for_openal_soft: renderer is null!\n"));
		return false;
	}
	else if (!stricmp((const char *)renderer, "OpenAL Soft"))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Sound initialisation
 * @return -1 if init failed, 0 if init success
 */
int ds_init()
{
	ALfloat list_orien[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	ALCint attrList[] = { ALC_FREQUENCY, 22050, 0 };
	unsigned int sample_rate = 22050;

	mprintf(("Initializing OpenAL...\n"));

	Ds_sound_quality = os_config_read_uint("Sound", "Quality", DS_SQ_MEDIUM);
	CLAMP(Ds_sound_quality, DS_SQ_LOW, DS_SQ_HIGH);

	switch (Ds_sound_quality) {
		case DS_SQ_HIGH:
			sample_rate = 48000;
		break;

		case DS_SQ_MEDIUM:
			sample_rate = 44100;
		break;

		default:
			sample_rate = 22050;
		break;
	}

	sample_rate = os_config_read_uint("Sound", "SampleRate", sample_rate);
	attrList[1] = sample_rate;
	SCP_string playback_device;
	SCP_string capture_device;

	if ( openal_init_device(&playback_device, &capture_device) == false ) {
		mprintf(("\n  ERROR: Unable to find suitable playback device!\n\n"));
		goto AL_InitError;
	}

	ds_sound_device = alcOpenDevice( (const ALCchar*) playback_device.c_str() );

	if (ds_sound_device == NULL) {
		mprintf(("  Failed to open playback_device (%s) returning error (%s)\n", playback_device.c_str(), openal_error_string(1)));
		goto AL_InitError;
	}

	ds_sound_context = alcCreateContext(ds_sound_device, attrList);

	if (ds_sound_context == NULL) {
		mprintf(("  Failed to create context for playback_device (%s) with attrList = { 0x%x, %d, %d } returning error (%s)\n",
			playback_device.c_str(), attrList[0], attrList[1], attrList[2], openal_error_string(1)));
		goto AL_InitError;
	}

	alcMakeContextCurrent(ds_sound_context);

	alcGetError(ds_sound_device);

	mprintf(("  OpenAL Vendor     : %s\n", alGetString(AL_VENDOR)));
	mprintf(("  OpenAL Renderer   : %s\n", alGetString(AL_RENDERER)));
	mprintf(("  OpenAL Version    : %s\n", alGetString(AL_VERSION)));
	mprintf(("\n"));

	// we need to clear out all errors before moving on
	alcGetError(NULL);
	alGetError();

	// set distance model to basically match what the D3D code does
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

	// make sure we can actually use AL_BYTE_LOKI (Mac/Win OpenAL doesn't have it)
	if ( alIsExtensionPresent( (const ALchar*)"AL_LOKI_play_position" ) == AL_TRUE ) {
		mprintf(("  Found extension \"AL_LOKI_play_position\".\n"));
		AL_play_position = 1;
	}

	if ( alIsExtensionPresent( (const ALchar*)"AL_EXT_float32" ) == AL_TRUE ) {
		mprintf(("  Found extension \"AL_EXT_float32\".\n"));
		Ds_float_supported = 1;
	}

	Ds_use_eax = 0;

	if ( alcIsExtensionPresent(ds_sound_device, (const ALchar*)"ALC_EXT_EFX") == AL_TRUE ) {
		mprintf(("  Found extension \"ALC_EXT_EFX\".\n"));
		Ds_use_eax = os_config_read_uint("Sound", "EnableEFX", Fred_running);
	}

	if (Ds_use_eax == 1) {
		if (ds_eax_init() != 0) {
			Ds_use_eax = 0;
		}
	}

	// the presets always need to be available to FRED
	if ( !Ds_use_eax && Fred_running ) {
		EFX_presets.reserve(EAX_ENVIRONMENT_COUNT);

		for (size_t i = 0; i < EAX_ENVIRONMENT_COUNT; i++) {
			EFX_presets.push_back( EFX_Reverb_Defaults[i] );
		}
	}

	if (!Cmdline_no_enhanced_sound)
	{
		if (!ds_check_for_openal_soft())
		{
			mprintf(("You are not using OpenAL Soft. Disabling enhanced sound.\n"));
			Cmdline_no_enhanced_sound = 1;
		}
		else
		{
			mprintf(("Enhanced sound is enabled.\n"));
		}
	}
	else
	{
		mprintf(("Enhanced sound is manually disabled.\n"));
	}

	// setup default listener position/orientation
	// this is needed for 2D pan
	OpenAL_ErrorPrint( alListener3f(AL_POSITION, 0.0, 0.0, 0.0) );
	OpenAL_ErrorPrint( alListenerfv(AL_ORIENTATION, list_orien) );

	// disable doppler (FIXME)
	OpenAL_ErrorPrint( alDopplerFactor(0.0f) );

	ds_init_channels();
	ds_init_buffers();

	mprintf(("\n"));

	{
	ALCint freq = 0;
	OpenAL_ErrorPrint( alcGetIntegerv(ds_sound_device, ALC_FREQUENCY, sizeof(ALCint), &freq) );

	mprintf(("  Sample rate: %d (%d)\n", freq, sample_rate));
	}

	if (Ds_use_eax) {
		ALCint major = 0, minor = 0, max_sends = 0;

		alcGetIntegerv(ds_sound_device, ALC_EFX_MAJOR_VERSION, 1, &major);
		alcGetIntegerv(ds_sound_device, ALC_EFX_MINOR_VERSION, 1, &minor);
		alcGetIntegerv(ds_sound_device, ALC_MAX_AUXILIARY_SENDS, 1, &max_sends);

		mprintf(("  EFX version: %d.%d\n", (int)major, (int)minor));
		mprintf(("  Max auxiliary sends: %d\n", max_sends));
	} else {
		mprintf(("  EFX enabled: NO\n"));
	}

	mprintf(("  Playback device: %s\n", playback_device.c_str()));
	mprintf(("  Capture device: %s\n", (capture_device.empty()) ? "<not available>" : capture_device.c_str()));

	mprintf(("... OpenAL successfully initialized!\n"));

	// we need to clear out any errors before moving on
	alcGetError(NULL);
	alGetError();

	return 0;

AL_InitError:
	alcMakeContextCurrent(NULL);

	if (ds_sound_context != NULL) {
		alcDestroyContext(ds_sound_context);
		ds_sound_context = NULL;
	}

	if (ds_sound_device != NULL) {
		alcCloseDevice(ds_sound_device);
		ds_sound_device = NULL;
	}

	mprintf(("... OpenAL failed to initialize!\n"));

	return -1;
}

/**
 * Free a single channel
 */
void ds_close_channel(int i)
{
	if ( (i < 0) || (i >= MAX_CHANNELS) ) {
		return;
	}

	if ( (Channels[i].source_id != 0) && alIsSource(Channels[i].source_id) ) {
		OpenAL_ErrorPrint( alSourceStop(Channels[i].source_id) );
		OpenAL_ErrorPrint( alSourcei(Channels[i].source_id, AL_BUFFER, 0) );

		if (Ds_eax_inited) {
			OpenAL_ErrorPrint( alSource3i(Channels[i].source_id, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL) );
		}

		OpenAL_ErrorPrint( alDeleteSources(1, &Channels[i].source_id) );

		if (Channels[i].sid >= 0) {
			sound_buffers[Channels[i].sid].channel_id = -1;
		}

		Channels[i].source_id = 0;
		Channels[i].sid = -1;
		Channels[i].sig       = ds_sound_handle::invalid();
		Channels[i].snd_id = -1;
	}
}

void ds_close_channel_fast(int i)
{
	if ( (i < 0) || (i >= MAX_CHANNELS) ) {
		return;
	}

	if ( (Channels[i].source_id != 0) && alIsSource(Channels[i].source_id) ) {
		OpenAL_ErrorPrint( alSourceStop(Channels[i].source_id) );
		OpenAL_ErrorPrint( alSourcei(Channels[i].source_id, AL_BUFFER, 0) );

		if (Ds_eax_inited) {
			OpenAL_ErrorPrint( alSource3i(Channels[i].source_id, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL) );
		}

		if (Channels[i].sid >= 0) {
			sound_buffers[Channels[i].sid].channel_id = -1;
		}

		Channels[i].sid = -1;
		Channels[i].sig    = ds_sound_handle::invalid();
		Channels[i].snd_id = -1;
	}
}

/**
 * Free all the channel buffers
 */
void ds_close_all_channels()
{
	int i;

	for (i = 0; i < MAX_CHANNELS; i++) {
		ds_close_channel(i);
	}
}

/**
 * Unload a buffer
 */
void ds_unload_buffer(int sid)
{
	if ( (sid < 0) || ((size_t)sid >= sound_buffers.size()) ) {
		return;
	}

	if (sound_buffers[sid].channel_id >= 0) {
		ds_close_channel_fast(sound_buffers[sid].channel_id);
		sound_buffers[sid].channel_id = -1;
	}

	ALuint buf_id = sound_buffers[sid].buf_id;

	if ( (buf_id != 0) && alIsBuffer(buf_id) ) {
		OpenAL_ErrorCheck( alDeleteBuffers(1, &buf_id), return );
	}

	sound_buffers[sid].buf_id = 0;
}

/**
 * Unload all the channel buffers
 */
void ds_close_buffers()
{
	size_t i;

	for (i = 0; i < sound_buffers.size(); i++) {
		ds_unload_buffer((int)i);
	}

	sound_buffers.clear();
}

/**
 * Close the sound system
 */
void ds_close()
{
	ds_close_all_channels();
	ds_close_buffers();
	ds_eax_close();

	// free the Channels[] array, since it was dynamically allocated
	delete [] Channels;
	Channels = NULL;

	alcMakeContextCurrent(NULL);	// hangs on me for some reason

	if (ds_sound_context != NULL) {
		alcDestroyContext(ds_sound_context);
		ds_sound_context = NULL;
	}

	if (ds_sound_device != NULL) {
		alcCloseDevice(ds_sound_device);
		ds_sound_device = NULL;
	}
}


/**
 * Find a free channel to play a sound on.  If no free channels exists, free up one based on volume levels.
 * This is the original retail version of ds_get_free_channel().
 *
 * @param new_volume Volume for sound to play at
 * @param snd_id Which kind of sound to play
 * @param priority ::DS_MUST_PLAY, ::DS_LIMIT_ONE, ::DS_LIMIT_TWO, ::DS_LIMIT_THREE
 *
 * @returns	Channel number to play sound on, or -1 if no channel could be found
 *
 * NOTE: snd_id is needed since we limit the number of concurrent samples
 */
int ds_get_free_channel_retail(float new_volume, int snd_id, int priority)
{
	int			i, first_free_channel, limit = 100;
	int			instance_count;	// number of instances of sound already playing
	int			lowest_vol_index = -1, lowest_instance_vol_index = -1;
	float		lowest_vol = 1.0f, lowest_instance_vol = 1.0f;
	channel		*chp;
	int status;

	instance_count = 0;
	first_free_channel = -1;

	// determine the limit of concurrent instances of this sound
	switch (priority) {
		case DS_MUST_PLAY:
			limit = 100;
		break;

		case DS_LIMIT_ONE:
			limit = 1;
		break;

		case DS_LIMIT_TWO:
			limit = 2;
		break;

		case DS_LIMIT_THREE:
			limit = 3;
		break;

		default:
			Int3();			// get Alan
			limit = 100;
		break;
	}

	// Look for a channel to use to play this sample
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		chp = &Channels[i];

		// source not created yet
		if (chp->source_id == 0) {
			if (first_free_channel == -1) {
				first_free_channel = i;
			}
			continue;
		}

		// source not bound to a buffer
		if (chp->sid == -1) {
			if (first_free_channel == -1) {
				first_free_channel = i;
			}
			continue;
		}

		OpenAL_ErrorCheck( alGetSourcei(chp->source_id, AL_SOURCE_STATE, &status), continue );

		if ( (status == AL_INITIAL) || (status == AL_STOPPED) ) {
			ds_close_channel_fast(i);

			if (first_free_channel == -1) {
				first_free_channel = i;
			}
			continue;
		} else {
			if ( chp->snd_id == snd_id ) {
				instance_count++;
				if ( (chp->vol < lowest_instance_vol) && (chp->looping == FALSE) ) {
					lowest_instance_vol = chp->vol;
					lowest_instance_vol_index = i;
				}
			} else if ( chp->is_voice_msg ) {
				// a playing voice message is not allowed to be preempted.
			} else if ( (chp->vol < lowest_vol) && (chp->looping == FALSE) ) {
				lowest_vol_index = i;
				lowest_vol = chp->vol;
			}
		}
	}

	// If we've exceeded the limit, then maybe stop the duplicate if it is lower volume
	if ( (instance_count >= limit) && (lowest_instance_vol_index >= 0) ) {
		// If there is a lower volume duplicate, stop it.... otherwise, don't play the sound
		if (lowest_instance_vol <= new_volume) {
			ds_close_channel_fast(lowest_instance_vol_index);
			first_free_channel = lowest_instance_vol_index;
		} else {
			// NOTE: yes we are preventing the sound from playing even if
			// there is an available channel because we are over the limit
			// requested by the rest of the engine, which means if we do
			// not honour its request to limit the count then the engine
			// will trip itself up by using all channels without having
			// the intention of actually doing so.  This means we get
			// very loud sounds, missing more important sounds, etc.
			// Effectivly the problem is the rest of the engine assumes
			// it is still stuck in the 90s with a sound card that only has
			// <=16 channels so we need to give it a sound card that
			// has 16 channels (though we are actually allowing 32 channels
			// just because we can).
			first_free_channel = -1;
		}
	} else if (first_free_channel == -1) {
		// there is no limit barrier to play the sound, but we have run out of channels
		// stop the lowest volume instance to play our sound if priority demands it
		if ( (lowest_vol_index != -1) && (priority == DS_MUST_PLAY) ) {
			// Check if the lowest volume playing is less than the volume of the requested sound.
			// If so, then we are going to trash the lowest volume sound.
			if ( Channels[lowest_vol_index].vol <= new_volume ) {
				ds_close_channel_fast(lowest_vol_index);
				first_free_channel = lowest_vol_index;
			}
		}
	}

	if ( (first_free_channel >= 0) && (Channels[first_free_channel].source_id == 0) ) {
		OpenAL_ErrorCheck( alGenSources(1, &Channels[first_free_channel].source_id), return -1 );
	}
	return first_free_channel;
}

/**
 * Find a free channel to play a sound on.  If no free channels exists, free up one based on priority and volume levels.
 * Special version for enhanced mode.
 *
 * @param new_volume Volume for sound to play at
 * @param snd_id Which kind of sound to play
 * @param enhanced_priority Priority level, see EnhancedSoundPriority enum in gamesnd.h
 * @param enhanced_limit Per-sound concurrency limit
 *
 * @returns	Channel number to play sound on, or -1 if no channel could be found
 *
 * NOTE: snd_id is needed since we limit the number of concurrent samples
 */
int ds_get_free_channel_enhanced(float new_volume, int snd_id, int enhanced_priority, unsigned int enhanced_limit)
{
	int			i, first_free_channel;
	int			instance_count;	// number of instances of sound already playing
	// least important means lowest volume among lowest priority
	// note that higher priority value means lower priority
	int			least_important_index = -1, least_important_instance_index = -1;
	float		least_important_vol = 1.1f, least_important_instance_vol = 1.1f;
	int			least_important_priority = -1, least_important_instance_priority = -1;
	channel		*chp;
	int status;

	instance_count = 0;
	first_free_channel = -1;

	// Look for a channel to use to play this sample
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		chp = &Channels[i];

		// source not created yet
		if (chp->source_id == 0) {
			if (first_free_channel == -1) {
				first_free_channel = i;
			}
			continue;
		}

		// source not bound to a buffer
		if (chp->sid == -1) {
			if (first_free_channel == -1) {
				first_free_channel = i;
			}
			continue;
		}

		OpenAL_ErrorCheck( alGetSourcei(chp->source_id, AL_SOURCE_STATE, &status), continue );

		if ( (status == AL_INITIAL) || (status == AL_STOPPED) ) {
			ds_close_channel_fast(i);

			if (first_free_channel == -1) {
				first_free_channel = i;
			}
			continue;
		} else {
			if ( chp->snd_id == snd_id ) {
				instance_count++;

				// looping or ambient soudns can't be preempted
				if ((chp->looping == FALSE) && !chp->is_ambient)
				{
					if (chp->priority > least_important_instance_priority) {
						least_important_instance_vol = chp->vol;
						least_important_instance_priority = chp->priority;
						least_important_instance_index = i;
					} else if ((chp->priority == least_important_instance_priority)
						 && (chp->vol < least_important_instance_vol)) {
						least_important_instance_vol = chp->vol;
						least_important_instance_priority = chp->priority;
						least_important_instance_index = i;
					}
				}
			} else if ( chp->is_voice_msg || (chp->looping == TRUE) || chp->is_ambient ) {
				// a playing voice message, looping sound, or ambient sound is not allowed to be preempted.
			} else if ( (chp->priority > least_important_priority) ) {
				least_important_index = i;
				least_important_vol = chp->vol;
				least_important_priority = chp->priority;
			} else if ( (chp->priority == least_important_priority) && (chp->vol < least_important_vol) ) {
				least_important_index = i;
				least_important_vol = chp->vol;
				least_important_priority = chp->priority;
			}
		}
	}

	// If we've exceeded the limit, then stop the least important duplicate if it is lower or equal volume
	// otherwise, don't play the sound
	if ( ((unsigned int)instance_count >= enhanced_limit) && (least_important_instance_index >= 0) ) {
		if (least_important_instance_vol <= new_volume) {
			ds_close_channel_fast(least_important_instance_index);
			first_free_channel =least_important_instance_index;
		} else {
			// don't exceed per-sound concurrency limits, even if there are spare channels
			first_free_channel = -1;
		}
	} else if (first_free_channel == -1) {
		// we haven't reached the limit, but we have run out of channels
		// attempt to stop the least important sound to play our sound if priority demands it
		if ( (least_important_index != -1)) {
			// Check if the priority difference is great enough (must play or at least 2 levels away)
			// and if the least important sound's volume playing is less than or equal to the volume of the requested sound.
			// If so, then we are going to trash the least important sound.
			if ( (enhanced_priority == SND_ENHANCED_PRIORITY_MUST_PLAY) || (Channels[least_important_index].priority - enhanced_priority >= 2)) {
				if ( Channels[least_important_index].vol <= new_volume ) {
					ds_close_channel_fast(least_important_index);
					first_free_channel = least_important_index;
				}
			}
		}
	}

	if ( (first_free_channel >= 0) && (Channels[first_free_channel].source_id == 0) ) {
		OpenAL_ErrorCheck( alGenSources(1, &Channels[first_free_channel].source_id), return -1 );
	}
	return first_free_channel;
}

/**
 * Generic function for getting a free channel
 *  If enhanced soudn is used, set priority to the computed priority.
 *  Returns -1 if no free channel could be found.
 */

/**
 * Find a free channel to play a sound on.  If no free channels exists, free up one based on volume levels.
 * This is the new generic version of ds_get_free_channel().
 *
 * @param new_volume Volume for sound to play at
 * @param snd_id Which kind of sound to play
 * @param priority From retail :DS_MUST_PLAY, ::DS_LIMIT_ONE, ::DS_LIMIT_TWO, ::DS_LIMIT_THREE
 * @param enhanced_priority Output param that's updated with correct priority if enhanced sound is enabled
 *
 * @returns	Channel number to play sound on, or -1 if no channel could be found
 *
 * NOTE: snd_id is needed since we limit the number of concurrent samples
 */
int ds_get_free_channel(float volume, int snd_id, int priority, int & enhanced_priority, const EnhancedSoundData & enhanced_sound_data)
{
	int first_free_channel = -1;
	unsigned int enhanced_limit = 0;

	if (!Cmdline_no_enhanced_sound) {
		enhanced_priority = enhanced_sound_data.priority;
		enhanced_limit = enhanced_sound_data.limit;

		// exception: if retail priority is must play, we assume it's for a good reason
		// and thus follow suit with enhanced sound
		if (priority == DS_MUST_PLAY) {
			enhanced_priority = SND_ENHANCED_PRIORITY_MUST_PLAY;
		}

		first_free_channel = ds_get_free_channel_enhanced(volume, snd_id, enhanced_priority, enhanced_limit);
	} else { // enhanced sound is off
		first_free_channel = ds_get_free_channel_retail(volume, snd_id, priority);
	}

	return first_free_channel;
}

/**
 * Create a sound buffer in software, without locking any data in
 */
int ds_create_buffer(int frequency, int bits_per_sample, int nchannels, int nseconds)
{
	ALuint i;
	int sid;

	if (!ds_initialized) {
		return -1;
	}

	sid = ds_get_sid();
	if ( sid == -1 ) {
		nprintf(("Sound","SOUND ==> No more OpenAL buffers available\n"));
		return -1;
	}

	OpenAL_ErrorCheck( alGenBuffers(1, &i), return -1 );

	sound_buffers[sid].buf_id = i;
	sound_buffers[sid].channel_id = -1;
	sound_buffers[sid].frequency = frequency;
	sound_buffers[sid].bits_per_sample = bits_per_sample;
	sound_buffers[sid].nchannels = nchannels;
	sound_buffers[sid].nseconds = nseconds;
	sound_buffers[sid].nbytes = nseconds * (bits_per_sample / 8) * nchannels * frequency;

	return sid;
}

/**
 * Lock data into an existing buffer
 */
int ds_lock_data(int sid, unsigned char *data, int size)
{
	if ( (sid < 0) || ((size_t)sid >= sound_buffers.size()) ) {
		return -1;
	}

	ALenum format = openal_get_format(sound_buffers[sid].bits_per_sample, sound_buffers[sid].nchannels);

	if (format == AL_INVALID_VALUE) {
		return -1;
	}

	sound_buffers[sid].nbytes = size;

	OpenAL_ErrorCheck( alBufferData(sound_buffers[sid].buf_id, format, data, size, sound_buffers[sid].frequency), return -1 );

	return 0;
}

/**
 * Stop a buffer from playing directly
 */
void ds_stop_easy(int sid)
{
	Assert(sid >= 0);

	int cid = sound_buffers[sid].channel_id;

	if (cid != -1) {
		ALuint source_id = Channels[cid].source_id;
		OpenAL_ErrorPrint( alSourceStop(source_id) );
	}
}

/**
 * Play a sound secondary buffer.  
 *
 * @param sid Software id of sound
 * @param snd_id What kind of sound this is
 * @param priority ::DS_MUST_PLAY, ::DS_LIMIT_ONE, ::DS_LIMIT_TWO, ::DS_LIMIT_THREE
 * @param volume Volume of sound effect in DirectSound units
 * @param pan Pan of sound in sound units
 * @param looping Whether the sound effect is looping or not
 * @param is_voice_msg If a voice message
 * 
 * @return 1 if sound effect could not be started, >=0 sig for sound effect successfully started
 */
ds_sound_handle ds_play(int sid, int snd_id, int priority, const EnhancedSoundData* enhanced_sound_data, float volume,
                        float pan, int looping, bool is_voice_msg)
{
	int ch_idx;
	int enhanced_priority = SND_ENHANCED_PRIORITY_INVALID;

	if (!ds_initialized) {
		return ds_sound_handle::invalid();
	}

	ch_idx = ds_get_free_channel(volume, snd_id, priority, enhanced_priority, *enhanced_sound_data);

	if (ch_idx < 0) {
		return ds_sound_handle::invalid();
	}

	if (Channels[ch_idx].source_id == 0) {
		return ds_sound_handle::invalid();
	}

	if (pan) {
		OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_POSITION, pan, 0.0f, -1.0f) );
	} else {
		OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_POSITION, 0.0f, 0.0f, 0.0f) );
	}

	OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_VELOCITY, 0.0f, 0.0f, 0.0f) );

	OpenAL_ErrorPrint( alDopplerFactor(0.0f) );
	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_PITCH, 1.0f) );
	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_GAIN, volume) );


	ALint status;
	OpenAL_ErrorCheck(alGetSourcei(Channels[ch_idx].source_id, AL_SOURCE_STATE, &status),
	                  return ds_sound_handle::invalid());

	if (status == AL_PLAYING) {
		OpenAL_ErrorPrint( alSourceStop(Channels[ch_idx].source_id) );
	}

	OpenAL_ErrorCheck(alSourcei(Channels[ch_idx].source_id, AL_BUFFER, sound_buffers[sid].buf_id),
	                  return ds_sound_handle::invalid());

	OpenAL_ErrorPrint( alSourcei(Channels[ch_idx].source_id, AL_SOURCE_RELATIVE, AL_TRUE) );

	OpenAL_ErrorPrint( alSourcei(Channels[ch_idx].source_id, AL_LOOPING, (looping) ? AL_TRUE : AL_FALSE) );

	if (Ds_eax_inited) {
		OpenAL_ErrorPrint( alSource3i(Channels[ch_idx].source_id, AL_AUXILIARY_SEND_FILTER,
								is_voice_msg ? AL_EFFECTSLOT_NULL : AL_EFX_aux_id, 0, AL_FILTER_NULL) );
	}

	OpenAL_ErrorPrint( alSourcePlay(Channels[ch_idx].source_id) );

	sound_buffers[sid].channel_id = ch_idx;

	Channels[ch_idx].sid = sid;
	Channels[ch_idx].snd_id = snd_id;
	Channels[ch_idx].sig           = ds_sound_handle(channel_next_sig++);
	Channels[ch_idx].last_position = 0;
	Channels[ch_idx].is_voice_msg = is_voice_msg;
	Channels[ch_idx].vol = volume;
	Channels[ch_idx].looping = looping;
	Channels[ch_idx].priority = enhanced_priority;
	Channels[ch_idx].is_ambient = false; // no support for 2D ambient sounds

	if (channel_next_sig < 0) {
		channel_next_sig = 1;
	}

	return Channels[ch_idx].sig;
}


/**
 * Return the channel number that is playing the sound identified by sig.
 * @return Channel number, if not playing, return -1.
 */
int ds_get_channel(ds_sound_handle sig)
{
	int i;

	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( Channels[i].source_id && (Channels[i].sig == sig) ) {
			if ( ds_is_channel_playing(i) == TRUE ) {
				return i;
			}
		}
	}

	return -1;
}

/**
 * @todo Documentation
 */
int ds_is_channel_playing(int channel_id)
{
	if ( Channels[channel_id].source_id != 0 ) {
		ALint status;

		OpenAL_ErrorPrint( alGetSourcei(Channels[channel_id].source_id, AL_SOURCE_STATE, &status) );

		return (status == AL_PLAYING);
	}

	return 0;
}

/**
 * @todo Documentation
 */
void ds_stop_channel(int channel_id)
{
	if ( Channels[channel_id].source_id != 0 ) {
		OpenAL_ErrorPrint( alSourceStop(Channels[channel_id].source_id) );
	}
}

/**
 * @todo Documentation
 */
void ds_stop_channel_all()
{
	int i;

	for ( i=0; i<MAX_CHANNELS; i++ ) {
		if ( Channels[i].source_id != 0 ) {
			OpenAL_ErrorPrint( alSourceStop(Channels[i].source_id) );
		}
	}
}

/**
 * @brief Set the volume for a channel.  The volume is expected to be in linear scale
 * @details If the sound is a 3D sound buffer, this is like re-establishing the maximum volume.
 */
void ds_set_volume( int channel_id, float vol )
{
	if ( (channel_id < 0) || (channel_id >= MAX_CHANNELS) ) {
		return;
	}

	ALuint source_id = Channels[channel_id].source_id;

	if (source_id != 0) {
		CAP(vol, 0.0f, 1.0f);
		OpenAL_ErrorPrint( alSourcef(source_id, AL_GAIN, vol) );
	}
}

/**
 * Set the pan for a channel.  The pan is expected to be in DirectSound units
 */
void ds_set_pan( int channel_id, float pan )
{
	if ( (channel_id < 0) || (channel_id >= MAX_CHANNELS) ) {
		return;
	}

	ALint state;

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel_id].source_id, AL_SOURCE_STATE, &state), return );

	if (state == AL_PLAYING) {
		//OpenAL_ErrorPrint( alSourcei(Channels[channel_id].source_id, AL_SOURCE_RELATIVE, AL_TRUE) );
		OpenAL_ErrorPrint( alSource3f(Channels[channel_id].source_id, AL_POSITION, pan, 0.0f, -1.0f) );
	}
}

float ds_get_pitch(int channel_id)
{
	ALint status;
	ALfloat alpitch = 1.0;

	if ( (channel_id < 0) || (channel_id >= MAX_CHANNELS) ) {
		return -1;
	}

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel_id].source_id, AL_SOURCE_STATE, &status), return -1 );

	if (status == AL_PLAYING) {
		OpenAL_ErrorPrint( alGetSourcef(Channels[channel_id].source_id, AL_PITCH, &alpitch) );
	}

	return alpitch;
}

void ds_set_pitch(int channel_id, float pitch)
{
	Assertion(pitch > 0.0f, "Pitch may not be less than zero!");

	ALint status;

	if ( (channel_id < 0) || (channel_id >= MAX_CHANNELS) ) {
		return;
	}

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel_id].source_id, AL_SOURCE_STATE, &status), return );

	if (status == AL_PLAYING) {
		OpenAL_ErrorPrint( alSourcef(Channels[channel_id].source_id, AL_PITCH, pitch) );
	}
}

/**
 * Starts a ds3d sound playing
 *
 * @param sid Software id for sound to play
 * @param snd_id Identifies what type of sound is playing
 * @param pos World pos of sound
 * @param vel Velocity of object emitting sound
 * @param min Distance at which sound doesn't get any louder
 * @param max Distance at which sound becomes inaudible
 * @param looping Whether to loop the sound or not
 * @param max_volume Volume (0 to 1) for 3d sound at maximum
 * @param estimated_vol	Manual estimated volume
 * @param priority ::DS_MUST_PLAY, ::DS_LIMIT_ONE, ::DS_LIMIT_TWO, ::DS_LIMIT_THREE
 *
 * @return 0 if sound started successfully, -1 if sound could not be played
 */
ds_sound_handle ds3d_play(int sid, int snd_id, vec3d* pos, vec3d* vel, float min, float max, int looping,
                          float max_volume, float estimated_vol, const EnhancedSoundData* enhanced_sound_data,
                          int priority, bool is_ambient)
{
	int channel_id;
	int enhanced_priority = SND_ENHANCED_PRIORITY_INVALID;


	if (!ds_initialized) {
		return ds_sound_handle::invalid();
	}

	channel_id = ds_get_free_channel(estimated_vol, snd_id, priority, enhanced_priority, *enhanced_sound_data);

	if (channel_id < 0) {
		return ds_sound_handle::invalid();
	}

	if ( Channels[channel_id].source_id == 0 ) {
		return ds_sound_handle::invalid();
	}

	// set up 3D sound data here
	ds3d_update_buffer(channel_id, min, max, pos, vel);


	OpenAL_ErrorPrint( alSourcef(Channels[channel_id].source_id, AL_PITCH, 1.0f) );

	OpenAL_ErrorPrint( alSourcef(Channels[channel_id].source_id, AL_GAIN, max_volume) );

	ALint status;
	OpenAL_ErrorCheck(alGetSourcei(Channels[channel_id].source_id, AL_SOURCE_STATE, &status),
	                  return ds_sound_handle::invalid());

	if (status == AL_PLAYING) {
		OpenAL_ErrorPrint( alSourceStop(Channels[channel_id].source_id) );
	}

	OpenAL_ErrorCheck(alSourcei(Channels[channel_id].source_id, AL_BUFFER, sound_buffers[sid].buf_id),
	                  return ds_sound_handle::invalid());

	if (Ds_eax_inited) {
		OpenAL_ErrorPrint( alSource3i(Channels[channel_id].source_id, AL_AUXILIARY_SEND_FILTER, AL_EFX_aux_id, 0, AL_FILTER_NULL) );
	}

	OpenAL_ErrorPrint( alSourcei(Channels[channel_id].source_id, AL_SOURCE_RELATIVE, AL_FALSE) );

	OpenAL_ErrorPrint( alSourcei(Channels[channel_id].source_id, AL_LOOPING, (looping) ? AL_TRUE : AL_FALSE) );

	OpenAL_ErrorPrint( alSourcePlay(Channels[channel_id].source_id) );


	sound_buffers[sid].channel_id = channel_id;

	Channels[channel_id].sid = sid;
	Channels[channel_id].snd_id = snd_id;
	Channels[channel_id].sig           = ds_sound_handle(channel_next_sig++);
	Channels[channel_id].last_position = 0;
	Channels[channel_id].is_voice_msg = false;
	Channels[channel_id].vol = max_volume;
	Channels[channel_id].looping = looping;
	Channels[channel_id].priority = enhanced_priority;
	Channels[channel_id].is_ambient = is_ambient;

	if (channel_next_sig < 0 ) {
		channel_next_sig = 1;
	}

	return Channels[channel_id].sig;
}

/**
 * @todo Documentation
 */
void ds_set_position(int channel_id, unsigned int offset)
{
	if ( (channel_id < 0) || (channel_id >= MAX_CHANNELS) ) {
		return;
	}

	OpenAL_ErrorPrint( alSourcei(Channels[channel_id].source_id, AL_BYTE_OFFSET, offset) );
}

/**
 * @todo Documentation
 */
unsigned int ds_get_play_position(int channel_id)
{
	if ( (channel_id < 0) || (channel_id >= MAX_CHANNELS) ) {
		return 0;
	}

	ALint pos = -1;
	int	sid = Channels[channel_id].sid;

	if ( (sid < 0) || ((size_t)sid >= sound_buffers.size()) ) {
		return 0;
	}

	if (AL_play_position) {
		OpenAL_ErrorPrint( alGetSourcei(Channels[channel_id].source_id, AL_BYTE_LOKI, &pos) );

		if ( pos < 0 ) {
			pos = 0;
		} else if ( pos > 0 ) {
			// AL_BYTE_LOKI returns position in canon format which may differ
			// from our sample, so we may have to scale it
			ALuint buf_id = sound_buffers[sid].buf_id;
			ALint size;

			OpenAL_ErrorCheck( alGetBufferi(buf_id, AL_SIZE, &size), return 0 );

			pos = (ALint)(pos * ((float)sound_buffers[sid].nbytes / size));
		}
	} else {
		OpenAL_ErrorPrint( alGetSourcei(Channels[channel_id].source_id, AL_BYTE_OFFSET, &pos) );

		if (pos < 0) {
			pos = 0;
		}
	}

	return (unsigned int) pos;
}

/**
 * @todo Documentation
 */
int ds_get_channel_size(int channel_id)
{
	if ( (channel_id < 0) || (channel_id >= MAX_CHANNELS) ) {
		return 0;
	}

	int sid = Channels[channel_id].sid;

	if ( (sid < 0) || ((size_t)sid >= sound_buffers.size()) ) {
		return 0;
	}

	ALuint buf_id = sound_buffers[sid].buf_id;
	ALint data_size = 0;

	if ( (buf_id != 0) && alIsBuffer(buf_id)) {
		OpenAL_ErrorPrint( alGetBufferi(buf_id, AL_SIZE, &data_size) );
	}

	return (int) data_size;
}

/** 
 * Returns the number of channels that are actually playing
 */
int ds_get_number_channels()
{
	int i,n;

	if (!ds_initialized) {
		return 0;
	}

	n = 0;
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( Channels[i].source_id ) {
			if ( ds_is_channel_playing(i) == TRUE ) {
				n++;
			}
		}
	}

	return n;
}

/**
 * Retreive raw data from a sound buffer
 */
int ds_get_data(int  /*sid*/, char * /*data*/)
{
	return -1;
}

/**
 * Return the size of the raw sound data
 */
int ds_get_size(int sid, int *size)
{
	Assert(sid >= 0);

	if ( (sid < 0) || ((size_t)sid >= sound_buffers.size()) ) {
		return 0;
	}

	ALuint buf_id = sound_buffers[sid].buf_id;
	ALint data_size = 0;

	if ( (buf_id != 0) && alIsBuffer(buf_id)) {
		OpenAL_ErrorPrint( alGetBufferi(buf_id, AL_SIZE, &data_size) );

		if (size) {
			*size = (int) data_size;
		}
		return 0;

	}
	return -1;
}

// --------------------
//
// EAX Functions below
//
// --------------------

/**
 * Set the master volume for the reverb added to all sound sources.
 *
 * @param volume Volume, range from 0 to 1.0
 * @returns 0 if the volume is set successfully, otherwise return -1
 */
int ds_eax_set_volume(float volume)
{
	if ( !Ds_eax_inited ) {
		return -1;
	}

	CAP(volume, 0.0f, 1.0f);

	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_GAIN, volume) );

	OpenAL_ErrorCheck( v_alAuxiliaryEffectSloti(AL_EFX_aux_id, AL_EFFECTSLOT_EFFECT, AL_EFX_effect_id), return -1 );

	return 0;
}

/**
 * Set the decay time for the EAX environment (ie all sound sources)
 *
 * @param seconds Decay time in seconds
 * @return 0 if decay time is successfully set, otherwise return -1
 */
int ds_eax_set_decay_time(float seconds)
{
	if ( !Ds_eax_inited ) {
		return -1;
	}

	CAP(seconds, 0.1f, 20.0f);

	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_TIME, seconds) );

	OpenAL_ErrorCheck( v_alAuxiliaryEffectSloti(AL_EFX_aux_id, AL_EFFECTSLOT_EFFECT, AL_EFX_effect_id), return -1 );

	return 0;
}

/**
 * Set the damping value for the EAX environment (ie all sound sources)
 *
 * @param damp Damp value from 0 to 2.0
 * @return 0 if the damp value is successfully set, otherwise return -1
 */
int ds_eax_set_damping(float damp)
{
	if ( !Ds_eax_inited ) {
		return -1;
	}

	CAP(damp, 0.1f, 2.0f);

	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_HFRATIO, damp) );

	OpenAL_ErrorCheck( v_alAuxiliaryEffectSloti(AL_EFX_aux_id, AL_EFFECTSLOT_EFFECT, AL_EFX_effect_id), return -1 );

	return 0;
}

/**
 * Set up all the parameters for an environment
 *
 * @param id Value from the EAX_ENVIRONMENT_* enumeration
 * @param vol Volume for the environment (0 to 1.0)
 * @param damping Damp value for the environment (0 to 2.0)
 * @param decay Decay time in seconds (0.1 to 20.0)
 * @return 0 if successful, otherwise return -1
 */
int ds_eax_set_all(unsigned long id, float vol, float damping, float decay)
{
	if ( !Ds_eax_inited ) {
		return -1;
	}

	// special disabled case
	if ( (id == EAX_ENVIRONMENT_GENERIC) && (vol == 0.0f) && (damping == 0.0f) && (decay == 0.0f) ) {
		v_alAuxiliaryEffectSloti(AL_EFX_aux_id, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
		Ds_active_env_idx = 0;
		Ds_active_env = false;
		return 0;
	}

	al_efx_load_preset(id);

	CAP(vol, 0.0f, 1.0f);
	CAP(decay, 0.1f, 20.0f);
	CAP(damping, 0.1f, 2.0f);

	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_GAIN, vol) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_TIME, decay) );
	OpenAL_ErrorPrint( v_alEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_HFRATIO, damping) );

	OpenAL_ErrorCheck( v_alAuxiliaryEffectSloti(AL_EFX_aux_id, AL_EFFECTSLOT_EFFECT, AL_EFX_effect_id), return -1 );

	return 0;
}

int ds_eax_get_preset_id(const char *name)
{
	if ( !name || !strlen(name) ) {
		return -1;
	}

	size_t count = EFX_presets.size();

	for (size_t i = 0; i < count; i++) {
		if ( !stricmp(name, EFX_presets[i].name.c_str()) ) {
			return (int)i;
		}
	}

	return -1;
}

int ds_eax_get_prop(EFXREVERBPROPERTIES **props, const char *name, const char *template_name)
{
	Assert( props != NULL );
	Assert( name != NULL );
	Assert( strlen(name) > 0 ); //-V805

	int template_id = -1;

	int id = ds_eax_get_preset_id(name);

	if (id >= 0) {
		*props = &EFX_presets[id];
	} else {
		id = (int)EFX_presets.size();

		EFXREVERBPROPERTIES n_prop;

		if ( (template_name != NULL) && (template_name[0] != '\0') ) {
			template_id = ds_eax_get_preset_id(template_name);
		}

		if (template_id >= 0) {
			n_prop = EFX_presets[template_id];
			n_prop.name = name;
		} else {
			n_prop.name = name;
			n_prop.flDensity = 1.0f;
			n_prop.flDiffusion = 1.0f;
			n_prop.flGain = 0.32f;
			n_prop.flGainHF = 0.89f;
			n_prop.flGainLF = 0.0f;
			n_prop.flDecayTime = 1.49f;
			n_prop.flDecayHFRatio = 0.83f;
			n_prop.flDecayLFRatio = 1.0f;
			n_prop.flReflectionsGain = 0.05f;
			n_prop.flReflectionsDelay = 0.007f;
			n_prop.flReflectionsPan[0] = 0.0f;
			n_prop.flReflectionsPan[1] = 0.0f;
			n_prop.flReflectionsPan[2] = 0.0f;
			n_prop.flLateReverbGain = 1.26f;
			n_prop.flLateReverbDelay = 0.011f;
			n_prop.flLateReverbPan[0] = 0.0f;
			n_prop.flLateReverbPan[1] = 0.0f;
			n_prop.flLateReverbPan[2] = 0.0f;
			n_prop.flEchoTime = 0.25f;
			n_prop.flEchoDepth = 0.0f;
			n_prop.flModulationTime = 0.25f;
			n_prop.flModulationDepth = 0.0f;
			n_prop.flAirAbsorptionGainHF = 0.994f;
			n_prop.flHFReference = 5000.0f;
			n_prop.flLFReference = 250.0f;
			n_prop.flRoomRolloffFactor = 0.0f;
			n_prop.iDecayHFLimit = AL_TRUE;
		}

		EFX_presets.push_back( n_prop );

		*props = &EFX_presets[id];
	}

	if ( !stricmp(name, "default") ) {
		extern unsigned int SND_ENV_DEFAULT;
		SND_ENV_DEFAULT = id;
	}

	return 0;
}

/**
 * Get up the parameters for the current environment
 *
 * @param er (output) Hold environment parameters
 * @param id If set will get specified preset env, otherwise current env
 * @return 0 if successful, otherwise return -1
 */
int ds_eax_get_all(EAX_REVERBPROPERTIES *er, int id)
{
	if ( !er ) {
		return -1;
	}

	if (id < 0) {
		if (!Ds_active_env) {
			return -1;
		}

		er->environment = Ds_active_env_idx;

		OpenAL_ErrorPrint( v_alGetEffectf(AL_EFX_effect_id, AL_EAXREVERB_GAIN, &er->fVolume) );
		OpenAL_ErrorPrint( v_alGetEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_TIME, &er->fDecayTime_sec) );
		OpenAL_ErrorPrint( v_alGetEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_HFRATIO, &er->fDamping) );
	} else if (id < (int)EFX_presets.size()) {
		er->environment = (size_t)id;

		er->fVolume = EFX_presets[id].flGain;
		er->fDecayTime_sec = EFX_presets[id].flDecayTime;
		er->fDamping = EFX_presets[id].flDecayHFRatio;
	} else {
		return -1;
	}

	return 0;
}

/**
 * Close down EAX, freeing any allocated resources
 */
void ds_eax_close()
{
	if (Ds_eax_inited == 0) {
		return;
	}

	v_alAuxiliaryEffectSloti(AL_EFX_aux_id, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);

	v_alDeleteEffects(1, &AL_EFX_effect_id);
	AL_EFX_effect_id = 0;

	v_alDeleteAuxiliaryEffectSlots(1, &AL_EFX_aux_id);
	AL_EFX_aux_id = 0;

	Ds_eax_inited = 0;
}

/**
 * Initialize EAX
 * @return 0 if initialization is successful, otherwise return -1
 */
int ds_eax_init()
{
	if (Ds_eax_inited) {
		return 0;
	}

	try {
		v_alGenFilters = (ALGENFILTERS) al_load_function("alGenFilters");
		v_alDeleteFilters = (ALDELETEFILTERS) al_load_function("alDeleteFilters");
		v_alFilteri = (ALFILTERI) al_load_function("alFilteri");
		v_alGenEffecs = (ALGENEFFECTS) al_load_function("alGenEffects");
		v_alDeleteEffects = (ALDELETEEFFECTS) al_load_function("alDeleteEffects");
		v_alEffecti = (ALEFFECTI) al_load_function("alEffecti");
		v_alEffectf = (ALEFFECTF) al_load_function("alEffectf");
		v_alEffectfv = (ALEFFECTFV) al_load_function("alEffectfv");
		v_alGetEffectf = (ALGETEFFECTF) al_load_function("alGetEffectf");

		v_alGenAuxiliaryEffectSlots = (ALGENAUXILIARYEFFECTSLOTS) al_load_function("alGenAuxiliaryEffectSlots");
		v_alDeleteAuxiliaryEffectSlots = (ALDELETEAUXILIARYEFFECTSLOTS) al_load_function("alDeleteAuxiliaryEffectSlots");
		v_alIsAuxiliaryEffectSlot = (ALISAUXILIARYEFFECTSLOT) al_load_function("alIsAuxiliaryEffectSlot");
		v_alAuxiliaryEffectSloti = (ALAUXILIARYEFFECTSLOTI) al_load_function("alAuxiliaryEffectSloti");
		v_alAuxiliaryEffectSlotiv = (ALAUXILIARYEFFECTSLOTIV) al_load_function("alAuxiliaryEffectSlotiv");
		v_alAuxiliaryEffectSlotf = (ALAUXILIARYEFFECTSLOTF) al_load_function("alAuxiliaryEffectSlotf");
		v_alAuxiliaryEffectSlotfv = (ALAUXILIARYEFFECTSLOTFV) al_load_function("alAuxiliaryEffectSlotfv");
	} catch (const std::exception& err) {
		mprintf(("\n  EFX:  Unable to load function: %s()\n", err.what()));

		Ds_eax_inited = 0;
		return -1;
	}

	v_alGenAuxiliaryEffectSlots(1, &AL_EFX_aux_id);

	if (alGetError() != AL_NO_ERROR) {
		mprintf(("\n  EFX:  Unable to create Aux effect!\n"));
		return -1;
	}

	v_alGenEffecs(1, &AL_EFX_effect_id);

	if (alGetError() != AL_NO_ERROR) {
		mprintf(("\n  EFX:  Unable to create effect!\n"));
		return -1;
	}

	v_alEffecti(AL_EFX_effect_id, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);

	if (alGetError() != AL_NO_ERROR) {
		mprintf(("\n  EFX:  EAXReverb not supported!\n"));
		return -1;
	}

	v_alAuxiliaryEffectSloti(AL_EFX_aux_id, AL_EFFECTSLOT_EFFECT, AL_EFX_effect_id);

	if (alGetError() != AL_NO_ERROR) {
		mprintf(("\n  EFX:  Couldn't load effect!\n"));
		return -1;
	}

	// add default presets
	EFX_presets.reserve(EAX_ENVIRONMENT_COUNT);

	for (int i = 0; i < EAX_ENVIRONMENT_COUNT; i++) {
		EFX_presets.push_back( EFX_Reverb_Defaults[i] );
	}

	Ds_eax_inited = 1;

	// disabled by default
	ds_eax_set_all(EAX_ENVIRONMENT_GENERIC, 0.0f, 0.0f, 0.0f);

	return 0;
}

/**
 * @todo Documentation
 */
int ds_eax_is_inited()
{
	return Ds_eax_inited;
}

/**
 * Called once per game frame to make sure voice messages aren't looping
 */
void ds_do_frame()
{
	if (!ds_initialized) {
		return;
	}

	int i;
	channel *cp = NULL;

	for (i = 0; i < MAX_CHANNELS; i++) {
		cp = &Channels[i];
		Assert( cp != NULL );

		if (cp->is_voice_msg) {
			if( cp->source_id == 0 ) {
				continue;
			}

			unsigned int current_position = ds_get_play_position(i);
			if (current_position != 0) {
				if (current_position < cp->last_position) {
					ds_close_channel(i);
				} else {
					cp->last_position = current_position;
				}
			}
		}
	}
}

/**
 * Given a valid channel return the sound id
 */
int ds_get_sound_id(int channel_id)
{
	Assert( channel_id >= 0 );

	return Channels[channel_id].snd_id;
}

/**
 * @brief Given a valid channel, returns the sound signature (typically the sound index)
 * @param channel_id
 * @return
 */
int ds_get_sound_index(int channel_id) {
	Assert( channel_id >= 0 );

	return Channels[channel_id].sid;
}
