/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "globalincs/pstypes.h"
#include "cfile/cfile.h"
#include "sound/openal.h"
#include "sound/ds.h"
#include "sound/ds3d.h"
#include "sound/channel.h"
#include "sound/acm.h"
#include "osapi/osapi.h"
#include "sound/dscap.h"


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


static int MAX_CHANNELS = 32;		// initialized properly in ds_init_channels()
channel *Channels = NULL;
static int channel_next_sig = 1;

const int BUFFER_BUMP = 50;
SCP_vector<sound_buffer> sound_buffers;

extern int Snd_sram;				// mem (in bytes) used up by storing sounds in system memory

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
static int Ds_active_env = -1;


static void *al_load_function(const char *func_name)
{
	void *func = alGetProcAddress(func_name);
	if ( !func ) {
		throw func_name;
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

	Ds_active_env = presetid;
}


int ds_initialized = FALSE;

/**
 * @brief Parse a wave file.
 *
 * @param fp File of sound to parse
 * @param dest Address of pointer of where to store raw sound data (output parm)
 * @param dest_size Number of bytes of sound data stored (output parm)
 * @param header Address of pointer to a WAVEFORMATEX struct (output parm)
 * @param ogg Boolean to indicate OGG vorbis file, if false assume Wave file
 * @param ovf Pointer to a OggVorbis_File struct, OGG vorbis only (output parm)
 *
 * @return 0 if wave file successfully parsed, -1 if an error occurred
 *
 *	NOTE: memory is malloced for the header and dest (if not OGG) in this function.  It is the responsibility
 *	of the caller to free this memory later.
 */
int ds_parse_sound(CFILE* fp, ubyte **dest, uint *dest_size, WAVEFORMATEX **header, bool ogg, OggVorbis_File *ovf)
{
	PCMWAVEFORMAT	PCM_header;
	ushort			cbExtra = 0;
	unsigned int	tag, size, next_chunk;
	bool			got_fmt = false, got_data = false;

	// some preinit stuff, could be done from calling function but this should guarantee it's right
	*dest = NULL;
	*dest_size = 0;

	if (fp == NULL) {
		return -1;
	}

	// if we should have a Vorbis file then try for it
	if (ogg) {
		if (ovf == NULL) {
			Int3();
			return -1;
		}

		// Check for OGG Vorbis first
		if ( !ov_open_callbacks(fp, ovf, NULL, 0, cfile_callbacks) ) {
			// got one, now read all of the needed header info
			ov_info(ovf, -1);

			// we only support one logical bitstream
			if ( ov_streams(ovf) != 1 ) {
				nprintf(( "Sound", "SOUND ==> OGG reading error: We don't support bitstream changes!\n" ));
				return -1;
			}

			if ( (*header = (WAVEFORMATEX *) vm_malloc ( sizeof(WAVEFORMATEX) )) != NULL ) {
				(*header)->wFormatTag = OGG_FORMAT_VORBIS;
				(*header)->nChannels = (ushort)ovf->vi->channels;
				(*header)->nSamplesPerSec = ovf->vi->rate;

				switch (Ds_sound_quality) {
					case DS_SQ_HIGH:
						(*header)->wBitsPerSample = Ds_float_supported ? 32 : 16;
						break;

					case DS_SQ_MEDIUM:
						(*header)->wBitsPerSample = 16;
						break;

					default:
						(*header)->wBitsPerSample = 8;
						break;
				}

				(*header)->nBlockAlign = (ushort)(((*header)->wBitsPerSample / 8) * ovf->vi->channels);
				(*header)->nAvgBytesPerSec = ovf->vi->rate * (*header)->nBlockAlign;

				// WMC - Total samples * channels * bits/sample

				ogg_int64_t pcm_total_size = ov_pcm_total(ovf, -1);
				if (pcm_total_size > 0) {
					*dest_size = (uint)(pcm_total_size * (*header)->nBlockAlign);
				} else {
					nprintf(("Sound", "SOUND ==> Size returned for this file is invalid. Please reencode the file, as it will not work correctly.\n"));
					return -1;
				}
			} else {
				Assert( 0 );
				return -1;
			}

			// we're all good, can leave now
			return 0;
		}
	} else { 
		// Otherwise we assime Wave format
		// Skip the "RIFF" tag and file size (8 bytes)
		// Skip the "WAVE" tag (4 bytes)
		// IMPORTANT!! Look at snd_load before even THINKING about changing this.
		cfseek( fp, 12, CF_SEEK_SET );

		// Now read RIFF tags until the end of file
		while (1) {
			if ( cfread( &tag, sizeof(uint), 1, fp ) != 1 ) {
				break;
			}

			tag = INTEL_INT( tag );

			if ( cfread( &size, sizeof(uint), 1, fp ) != 1 ) {
				break;
			}

			size = INTEL_INT( size );

			next_chunk = cftell(fp) + size;

			switch (tag) {
				case 0x20746d66: { // The 'fmt ' tag
					PCM_header.wf.wFormatTag		= cfread_ushort(fp);
					PCM_header.wf.nChannels			= cfread_ushort(fp);
					PCM_header.wf.nSamplesPerSec	= cfread_uint(fp);
					PCM_header.wf.nAvgBytesPerSec	= cfread_uint(fp);
					PCM_header.wf.nBlockAlign		= cfread_ushort(fp);
					PCM_header.wBitsPerSample		= cfread_ushort(fp);

					// should be either: WAVE_FORMAT_PCM, WAVE_FORMAT_ADPCM, WAVE_FORMAT_IEEE_FLOAT
					switch (PCM_header.wf.wFormatTag) {
						case WAVE_FORMAT_PCM: {
							if ( (PCM_header.wBitsPerSample != 8) && (PCM_header.wBitsPerSample != 16) ) {
								nprintf(("Sound", "SOUND ==> %d-bit PCM is not supported!\n", PCM_header.wBitsPerSample));
								return -1;
							}
							// fix block align
							PCM_header.wf.nBlockAlign = (PCM_header.wBitsPerSample / 8) * PCM_header.wf.nChannels;
							break;
						}

						case WAVE_FORMAT_IEEE_FLOAT: {
							if (PCM_header.wBitsPerSample != 32) {
								nprintf(("Sound", "SOUND ==> %d-bit FLOAT PCM is not supported!\n", PCM_header.wBitsPerSample));
								return -1;
							}

							switch (Ds_sound_quality) {
								case DS_SQ_HIGH:
									PCM_header.wBitsPerSample = Ds_float_supported ? 32 : 16;
									break;

								case DS_SQ_MEDIUM:
									PCM_header.wBitsPerSample = 16;
									break;

								default:
									PCM_header.wBitsPerSample = 8;
									break;
							}
							// fix block align
							PCM_header.wf.nBlockAlign = (PCM_header.wBitsPerSample / 8) * PCM_header.wf.nChannels;
							break;
						}

						case WAVE_FORMAT_ADPCM:
							// block align doesn't get fixed here
							break;

						default: {
							nprintf(("Sound", "SOUND ==> Format '%d' is not supported!\n", PCM_header.wf.wFormatTag));
							return -1;
						}
					}

					if (PCM_header.wf.wFormatTag == WAVE_FORMAT_ADPCM) {
						cbExtra = cfread_ushort(fp);
					}

					// Allocate memory for WAVEFORMATEX structure + extra bytes
					if ( (*header = (WAVEFORMATEX *) vm_malloc ( sizeof(WAVEFORMATEX)+cbExtra )) != NULL ) {
						// Copy bytes from temporary format structure
						memcpy (*header, &PCM_header, sizeof(PCM_header));
						(*header)->cbSize = cbExtra;

						// Read those extra bytes, append to WAVEFORMATEX structure
						if (cbExtra != 0) {
							cfread( ((ubyte *)(*header) + sizeof(WAVEFORMATEX)), cbExtra, 1, fp);
						}
					} else {
						// malloc failed
						return -1;
					}
					got_fmt = true;
					break;
				}

				case 0x61746164: { // the 'data' tag
					*dest_size = size;

					(*dest) = (ubyte *)vm_malloc(size);
					Assert( *dest != NULL );

					cfread( *dest, size, 1, fp );

					got_data = true;

					break;
				}

				default: // unknown, skip it
					break;
			}

			// This is here so that we can avoid reading data that we don't understand or properly handle.
			// We could do this just as well by checking the RIFF size, but this is easier - taylor
			if (got_fmt && got_data) {
				break;
			}

			cfseek( fp, next_chunk, CF_SEEK_SET );
		}

		// we're all good, can leave now
		return 0;
	}

	return -1;
}

/**
 * Parse a sound file, any format, and store the info in s_info.
 *
 * @param real_filename Filename to parse
 * @param s_info Storage for the sound file info
 */
int ds_parse_sound_info(char *real_filename, sound_info *s_info)
{
	PCMWAVEFORMAT	PCM_header;
	uint			tag, size, next_chunk;
	bool			got_fmt = false, got_data = false;
	OggVorbis_File	ovf;
	int				rc, FileSize, FileOffset;
	char			fullpath[MAX_PATH];
	char			filename[MAX_FILENAME_LEN];
	const int		NUM_EXT = 2;
	const char		*audio_ext[NUM_EXT] = { ".ogg", ".wav" };
	int				rval = -1;

	if ( (real_filename == NULL) || (s_info == NULL) ) {
		return -1;
	}

	// remove extension
	strcpy_s( filename, real_filename );
	char *p = strrchr(filename, '.');
	if ( p ) *p = 0;

	rc = cf_find_file_location_ext(filename, NUM_EXT, audio_ext, CF_TYPE_ANY, sizeof(fullpath) - 1, fullpath, &FileSize, &FileOffset);

	if (rc < 0) {
		return -1;
	}

	// open the file
	CFILE *fp = cfopen_special(fullpath, "rb", FileSize, FileOffset);

	if (fp == NULL) {
		return -1;
	}

	// Ogg Vorbis
	if (rc == 0) {
		if ( !ov_open_callbacks(fp, &ovf, NULL, 0, cfile_callbacks) ) {
			// got one, now read all of the needed header info
			ov_info(&ovf, -1);

			// we only support one logical bitstream
			if ( ov_streams(&ovf) != 1 ) {
				nprintf(( "Sound", "SOUND ==> OGG reading error: We don't support bitstream changes!\n" ));
				ov_clear(&ovf);
				goto Done;
			}

			s_info->format = OGG_FORMAT_VORBIS;
			s_info->n_channels = (ushort)ovf.vi->channels;
			s_info->sample_rate = ovf.vi->rate;

			switch (Ds_sound_quality) {
				case DS_SQ_HIGH:
					s_info->bits = Ds_float_supported ? 32 : 16;
				break;

				case DS_SQ_MEDIUM:
					s_info->bits = 16;
				break;

				default:
					s_info->bits = 8;
				break;
			}

			s_info->n_block_align = (ushort)((s_info->bits / 8) * s_info->n_channels);
			s_info->avg_bytes_per_sec = s_info->sample_rate * s_info->n_block_align;

			s_info->size = (uint)(ov_pcm_total(&ovf, -1) * s_info->n_block_align);

			ov_clear(&ovf);

			// we're all good, can leave now
			rval = 0;
			goto Done;
		}
	}
	// PCM Wave
	else if (rc == 1) {
		// Skip the "RIFF" tag and file size (8 bytes)
		// Skip the "WAVE" tag (4 bytes)
		// IMPORTANT!! Look at snd_load before even THINKING about changing this.
		cfseek( fp, 12, CF_SEEK_SET );

		// Now read RIFF tags until the end of file
		while (1) {
			if ( cfread( &tag, sizeof(uint), 1, fp ) != 1 ) {
				break;
			}

			tag = INTEL_INT( tag );

			if ( cfread( &size, sizeof(uint), 1, fp ) != 1 ) {
				break;
			}

			size = INTEL_INT( size );
			next_chunk = cftell(fp) + size;

			switch (tag) {
				case 0x20746d66: // The 'fmt ' tag
					PCM_header.wf.wFormatTag		= cfread_ushort(fp);
					PCM_header.wf.nChannels			= cfread_ushort(fp);
					PCM_header.wf.nSamplesPerSec	= cfread_uint(fp);
					PCM_header.wf.nAvgBytesPerSec	= cfread_uint(fp);
					PCM_header.wf.nBlockAlign		= cfread_ushort(fp);
					PCM_header.wBitsPerSample		= cfread_ushort(fp);

					// should be either: WAVE_FORMAT_PCM, WAVE_FORMAT_ADPCM, WAVE_FORMAT_IEEE_FLOAT
					switch (PCM_header.wf.wFormatTag) {
						case WAVE_FORMAT_PCM: {
							if ( (PCM_header.wBitsPerSample != 8) && (PCM_header.wBitsPerSample != 16) ) {
								nprintf(("Sound", "SOUND ==> %d-bit PCM is not supported!\n", PCM_header.wBitsPerSample));
								goto Done;
							}
							// fix block align
							PCM_header.wf.nBlockAlign = (PCM_header.wBitsPerSample / 8) * PCM_header.wf.nChannels;
						break;
						}

						case WAVE_FORMAT_IEEE_FLOAT: {
							if (PCM_header.wBitsPerSample != 32) {
								nprintf(("Sound", "SOUND ==> %d-bit FLOAT PCM is not supported!\n", PCM_header.wBitsPerSample));
								goto Done;
							}

							switch (Ds_sound_quality) {
								case DS_SQ_HIGH:
									PCM_header.wBitsPerSample = Ds_float_supported ? 32 : 16;
								break;

								case DS_SQ_MEDIUM:
									PCM_header.wBitsPerSample = 16;
								break;

								default:
									PCM_header.wBitsPerSample = 8;
								break;
							}
							// fix block align
							PCM_header.wf.nBlockAlign = (PCM_header.wBitsPerSample / 8) * PCM_header.wf.nChannels;
						break;
						}

						case WAVE_FORMAT_ADPCM:
							// block align doesn't get fixed here
						break;

						default: {
							nprintf(("Sound", "SOUND ==> Format '%d' is not supported!\n", PCM_header.wf.wFormatTag));
							goto Done;
						}
					}

					s_info->format = PCM_header.wf.wFormatTag;
					s_info->n_channels = PCM_header.wf.nChannels;
					s_info->sample_rate = PCM_header.wf.nSamplesPerSec;
					s_info->bits = PCM_header.wBitsPerSample;
					s_info->avg_bytes_per_sec =  PCM_header.wf.nAvgBytesPerSec;
					s_info->n_block_align = PCM_header.wf.nBlockAlign;

					got_fmt = true;
				break;

				case 0x61746164: // the 'data' tag
					s_info->size = size;
					got_data = true;
				break;

				default:
			break;
			}

			if (got_fmt && got_data) {
				rval = 0;
				goto Done;
			}
			cfseek( fp, next_chunk, CF_SEEK_SET );
		}
	}
Done:
	cfclose(fp);
	return rval;
}

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

/**
 * @brief Load a secondary buffer with sound data.
 * @details The sounds data for game sounds are stored in the DirectSound secondary buffers, 
 * and are duplicated as needed and placed in the Channels[] array to be played.
 * 
 * @param sid Pointer to software id for sound ( output parm)
 * @param final_size Pointer to storage to receive uncompressed sound size (output parm)
 * @param header Pointer to a WAVEFORMATEX structure
 * @param si ::sound_info structure, contains details on the sound format
 * @param flags	Buffer properties ( DS_HARDWARE , DS_3D )
 *
 * @return 1 if sound effect could not loaded into a secondary buffer, 0 if sound effect successfully loaded into a secondary buffer
 *
 * NOTE: this function is slow, especially when sounds are loaded into hardware.  Don't call this
 * function from within gameplay.
 */
int ds_load_buffer(int *sid, int *final_size, void *header, sound_info *si, int flags)
{
	Assert( final_size != NULL );
	Assert( header != NULL );

	if (si == NULL) {
		Int3();
		return -1;
	}

	// All sounds are required to have a software buffer
	*sid = ds_get_sid();
	if ( *sid == -1 ) {
		nprintf(("Sound","SOUND ==> No more sound buffers available\n"));
		return -1;
	}

	ALuint pi;
	OpenAL_ErrorCheck( alGenBuffers (1, &pi), return -1 );

	ALenum format;
	ALsizei size;
	ALint bits, bps, n_channels = si->n_channels;
	ALuint frequency;
	ALvoid *data = NULL;
	int sign, byte_order = 0, section, last_section = -1;

	// the below two covnert_ variables are only used when the wav format is not
	// PCM.  DirectSound only takes PCM sound data, so we must convert to PCM if required
	ubyte *convert_buffer = NULL;		// storage for converted wav file
	int convert_len;					// num bytes of converted wav file
	uint src_bytes_used;				// number of source bytes actually converted (should always be equal to original size)
	int rc;
	WAVEFORMATEX *pwfx = (WAVEFORMATEX *)header;


	switch (si->format) {
		case WAVE_FORMAT_PCM: {
			Assert( si->data != NULL );

			bits = si->bits;
			bps  = si->avg_bytes_per_sec;
			size = si->size;

#if BYTE_ORDER == BIG_ENDIAN
			// swap 16-bit sound data
			if (bits == 16) {
				ushort *swap_tmp;

				for (uint i=0; i<size; i=i+2) {
					swap_tmp = (ushort*)(si->data + i);
					*swap_tmp = INTEL_SHORT(*swap_tmp);
				}
			}
#endif

			data = si->data;
			break;
		}

		case WAVE_FORMAT_ADPCM: {
			Assert( si->data != NULL );

			bits = (Ds_sound_quality == DS_SQ_LOW) ? 8 : 16;

			nprintf(( "Sound", "SOUND ==> Converting sound from ADPCM to PCM\n" ));

			rc = ACM_convert_ADPCM_to_PCM(pwfx, si->data, si->size, &convert_buffer, 0, &convert_len, &src_bytes_used, bits);

			if ( rc == -1 ) {
				return -1;
			}

			if (src_bytes_used != si->size) {
				return -1;	// ACM conversion failed?
			}

			bps  = (((si->n_channels * bits) / 8) * si->sample_rate);
			size = convert_len;
			data = convert_buffer;

			nprintf(( "Sound", "SOUND ==> Coverted sound from ADPCM to PCM successfully\n" ));
		break;
		}

		case WAVE_FORMAT_IEEE_FLOAT: {
			Assert( si->data != NULL );

			bits = si->bits;

#if BYTE_ORDER == BIG_ENDIAN
			// need to byte-swap before any later conversions
			float *swap_tmp;

			for (uint i = 0; i < si->size; i += sizeof(float)) {
				swap_tmp = (float *)(si->data + i);
				*swap_tmp = INTEL_FLOAT(swap_tmp);
			}
#endif

			if (bits == 32) {
				bps = si->avg_bytes_per_sec;
				size = si->size;

				data = si->data;
			} else if (bits == 16) {
				bps = si->avg_bytes_per_sec >> 1;
				size = si->size >> 1;

				convert_buffer = (ubyte*)vm_malloc_q(size);

				if (convert_buffer == NULL) {
					return -1;
				}

				float *in_p = (float*)si->data;
				short *out_p = (short*)convert_buffer;

				int end = si->size / sizeof(float);

				for (int i = 0; i < end; i++) {
					int i_val = (int)(in_p[i] * 32767.0f + 0.5f);
					CLAMP(i_val, -32768, 32767);

					*out_p++ = (short)i_val;
				}

				data = convert_buffer;
			} else {
				bps = si->avg_bytes_per_sec >> 2;
				size = si->size >> 2;

				convert_buffer = (ubyte*)vm_malloc_q(size);

				if (convert_buffer == NULL) {
					return -1;
				}

				float *in_p = (float*)si->data;
				ubyte *out_p = (ubyte*)convert_buffer;

				int end = si->size / sizeof(float);

				for (int i = 0; i < end; i++) {
					int i_val = (int)(in_p[i] * 127.0f + 0.5f) + 128;
					CLAMP(i_val, 0, 255);
					*out_p++ = (ubyte)i_val;
				}

				data = convert_buffer;
			}
		break;
		}

		case OGG_FORMAT_VORBIS: {
			nprintf(( "Sound", "SOUND ==> converting sound from OGG to PCM\n" ));

			sign = (si->bits == 8) ? 0 : 1;
#if BYTE_ORDER == BIG_ENDIAN
			byte_order = 1;
#endif
			src_bytes_used = 0;

			convert_buffer = (ubyte*)vm_malloc_q(si->size);

			if (convert_buffer == NULL) {
				return -1;
			}

			while (src_bytes_used < si->size) {
				float **pcm = NULL;

				if (si->bits == 32) {
					rc = ov_read_float(&si->ogg_info, &pcm, 1024, &section);
				} else {
					rc = ov_read(&si->ogg_info, (char *) convert_buffer + src_bytes_used, si->size - src_bytes_used, byte_order, si->bits / 8, sign, &section);
				}

				// fail if the bitstream changes, shouldn't get this far if that's the case though
				if ((last_section != -1) && (last_section != section)) {
					nprintf(( "Sound", "SOUND ==> OGG reading error: We don't support bitstream changes!\n" ));
					ov_clear(&si->ogg_info);
					vm_free(convert_buffer);
					convert_buffer = NULL;
					return -1;
				}

				if (rc == OV_EBADLINK) {
					ov_clear(&si->ogg_info);
					vm_free(convert_buffer);
					convert_buffer = NULL;

					return -1;
				} else if (rc == 0) {
					break;
				} else if (rc > 0) {
					if (si->bits == 32) {
						float *out_p = (float*)(convert_buffer + src_bytes_used);

						for (int i = 0; i < rc; i++) {
							for (int j = 0; j < si->n_channels; j++) {
								*out_p++ = pcm[j][i];
							}
						}

						src_bytes_used += (rc * si->n_block_align);
					} else {
						src_bytes_used += rc;
					}
					last_section = section;
				}
			}

			bits = si->bits;
			bps = si->avg_bytes_per_sec;
			size = (int)src_bytes_used;

			data = convert_buffer;

			// we're done with ogg stuff so clean it up
			ov_clear(&si->ogg_info);

			nprintf(( "Sound", "SOUND ==> Coverted sound from OGG successfully\n" ));
		break;
		}

		default:
			return -1;
	}

	// if this is supposed to play in 3D then make sure it's mono
	if ( (flags & DS_3D) && (n_channels > 1) ) {
		ubyte *mono_buffer = NULL;

		mono_buffer = (ubyte*)vm_malloc_q(size >> 1);

		if (mono_buffer == NULL) {
			if (convert_buffer) {
				vm_free(convert_buffer);
			}

			return -1;
		}

		if (bits == 32) {
			float *in_p = (float*)data;
			float *out_p = (float*)mono_buffer;

			int end = size / sizeof(float);

			for (int i = 0; i < end; i += 2) {
				float i_val = (in_p[i] + in_p[i+1]) * 0.5f;
				CLAMP(i_val, -1.0f, 1.0f);
				*out_p++ = i_val;
			}
		} else if (bits == 16) {
			short *in_p = (short*)data;
			short *out_p = (short*)mono_buffer;
			int end = size / sizeof(short);

			for (int i = 0; i < end; i += 2) {
				int i_val = (in_p[i] + in_p[i+1]) >> 1;
				CLAMP(i_val, -32768, 32767);
				*out_p++ = (short)i_val;
			}
		} else {
			Assert( bits == 8 );
			ubyte *in_p = (ubyte*)data;
			ubyte *out_p = (ubyte*)mono_buffer;

			for (int i = 0; i < size; i += 2) {
				int i_val = (in_p[i] + in_p[i+1]) >> 1;
				CLAMP(i_val, 0, 255);
				*out_p++ = (ubyte)i_val;
			}
		}

		n_channels = 1;
		size >>= 1;
		bps >>= 1;
		data = mono_buffer;

		if (convert_buffer) {
			vm_free(convert_buffer);
			convert_buffer = NULL;
		}
		nprintf(("Sound", "SOUND ==> Converted 3D sound from stereo to mono\n"));
	}

	// format is now in pcm
	frequency = si->sample_rate;
	format = openal_get_format(bits, n_channels);

	if (format == AL_INVALID_VALUE) {
		if (convert_buffer) {
			vm_free(convert_buffer);
		}
		return -1;
	}

	Snd_sram += size;

	if (final_size) {
		*final_size = size;
	}

	OpenAL_ErrorCheck( alBufferData(pi, format, data, size, frequency), { if (convert_buffer) vm_free(convert_buffer); return -1; } );

	sound_buffers[*sid].buf_id = pi;
	sound_buffers[*sid].channel_id = -1;
	sound_buffers[*sid].frequency = frequency;
	sound_buffers[*sid].bits_per_sample = bits;
	sound_buffers[*sid].nchannels = n_channels;
	sound_buffers[*sid].nseconds = size / bps;
	sound_buffers[*sid].nbytes = size;

	// update sound_info struct with any changed data
	si->bits = bits;
	si->n_channels = n_channels;
	si->size = size;
	si->n_block_align = (bits / 8) * n_channels;
	si->avg_bytes_per_sec = bps;

	if ( convert_buffer ) {
		vm_free( convert_buffer );
	}
	return 0;
}

/**
 * Initialise the ::Channels[] array
 */
void ds_init_channels()
{
	try {
		Channels = new channel[MAX_CHANNELS];
	} catch (std::bad_alloc) {
		Error(LOCATION, "Unable to allocate %d bytes for %d audio channels.", sizeof(channel) * MAX_CHANNELS, MAX_CHANNELS);
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
		Channels[i].sig = -1;
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
		Channels[i].sig = -1;
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
		ds_unload_buffer(i);
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
 *
 * @param new_volume Volume for sound to play at
 * @param snd_id Which kind of sound to play
 * @param priority ::DS_MUST_PLAY, ::DS_LIMIT_ONE, ::DS_LIMIT_TWO, ::DS_LIMIT_THREE
 *
 * @returns	Channel number to play sound on, or -1 if no channel could be found
 *
 * NOTE: snd_id is needed since we limit the number of concurrent samples
 */
int ds_get_free_channel(float new_volume, int snd_id, int priority)
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
			} else if ( (chp->vol < lowest_vol) && (chp->looping == FALSE) ) {
				lowest_vol_index = i;
				lowest_vol = chp->vol;
			}
		}
	}

	if (first_free_channel < 0) {
		// If we've exceeded the limit, then maybe stop the duplicate if it is lower volume
		if ( (instance_count >= limit) && (lowest_instance_vol_index >= 0) ) {
			// If there is a lower volume duplicate, stop it.... otherwise, don't play the sound
			if (lowest_instance_vol <= new_volume) {
				ds_close_channel_fast(lowest_instance_vol_index);
				first_free_channel = lowest_instance_vol_index;
			}
		} else {
			// there is no limit barrier to play the sound, so see if we've ran out of channels
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
	}

	if ( (first_free_channel >= 0) && (Channels[first_free_channel].source_id == 0) ) {
		OpenAL_ErrorCheck( alGenSources(1, &Channels[first_free_channel].source_id), return -1 );
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
 * Play a sound without the usual baggage (used for playing back real-time voice)
 *  
 * @param sid Software id of sound
 * @param volume Volume of sound effect in linear scale
 */
int ds_play_easy(int sid, float volume)
{
	if (!ds_initialized) {
		return -1;
	}

	int ch_idx = ds_get_free_channel(volume, -1, DS_MUST_PLAY);

	if (ch_idx < 0) {
		return -1;
	}

	ALuint source_id = Channels[ch_idx].source_id;

	OpenAL_ErrorPrint( alSourceStop(source_id) );

	if (Channels[ch_idx].sid != sid) {
		ALuint buffer_id = sound_buffers[sid].buf_id;
		OpenAL_ErrorCheck( alSourcei(source_id, AL_BUFFER, buffer_id), return -1 );
	}

	Channels[ch_idx].sid = sid;

	OpenAL_ErrorPrint( alSourcef(source_id, AL_GAIN, volume) );

	OpenAL_ErrorPrint( alSourcei(source_id, AL_LOOPING, AL_FALSE) );
	OpenAL_ErrorPrint( alSourcei(source_id, AL_SOURCE_RELATIVE, AL_TRUE) );

	OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_POSITION, 0.0f, 0.0f, 0.0f) );
	OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_VELOCITY, 0.0f, 0.0f, 0.0f) );

	OpenAL_ErrorPrint( alDopplerFactor(0.0f) );

	OpenAL_ErrorPrint( alSourcePlay(source_id) );

	return 0;
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
int ds_play(int sid, int snd_id, int priority, float volume, float pan, int looping, bool is_voice_msg)
{
	int ch_idx;

	if (!ds_initialized) {
		return -1;
	}

	ch_idx = ds_get_free_channel(volume, snd_id, priority);

	if (ch_idx < 0) {
		return -1;
	}

	if (Channels[ch_idx].source_id == 0) {
		return -1;
	}

	OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_POSITION, pan, 0.0f, 0.0f) );
	OpenAL_ErrorPrint( alSource3f(Channels[ch_idx].source_id, AL_VELOCITY, 0.0f, 0.0f, 0.0f) );

	OpenAL_ErrorPrint( alDopplerFactor(0.0f) );
	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_PITCH, 1.0f) );
	OpenAL_ErrorPrint( alSourcef(Channels[ch_idx].source_id, AL_GAIN, volume) );


	ALint status;
	OpenAL_ErrorCheck( alGetSourcei(Channels[ch_idx].source_id, AL_SOURCE_STATE, &status), return -1 );

	if (status == AL_PLAYING) {
		OpenAL_ErrorPrint( alSourceStop(Channels[ch_idx].source_id) );
	}


	OpenAL_ErrorCheck( alSourcei(Channels[ch_idx].source_id, AL_BUFFER, sound_buffers[sid].buf_id), return -1 );

	OpenAL_ErrorPrint( alSourcei(Channels[ch_idx].source_id, AL_SOURCE_RELATIVE, AL_TRUE) );

	OpenAL_ErrorPrint( alSourcei(Channels[ch_idx].source_id, AL_LOOPING, (looping) ? AL_TRUE : AL_FALSE) );

	if (Ds_eax_inited) {
		OpenAL_ErrorPrint( alSource3i(Channels[ch_idx].source_id, AL_AUXILIARY_SEND_FILTER, AL_EFX_aux_id, 0, AL_FILTER_NULL) );
	}

	OpenAL_ErrorPrint( alSourcePlay(Channels[ch_idx].source_id) );

	sound_buffers[sid].channel_id = ch_idx;

	Channels[ch_idx].sid = sid;
	Channels[ch_idx].snd_id = snd_id;
	Channels[ch_idx].sig = channel_next_sig++;
	Channels[ch_idx].last_position = 0;
	Channels[ch_idx].is_voice_msg = is_voice_msg;
	Channels[ch_idx].vol = volume;
	Channels[ch_idx].looping = looping;
	Channels[ch_idx].priority = priority;

	if (channel_next_sig < 0) {
		channel_next_sig = 1;
	}

	return Channels[ch_idx].sig;
}


/**
 * Return the channel number that is playing the sound identified by sig.
 * @return Channel number, if not playing, return -1.
 */
int ds_get_channel(int sig)
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
int ds_is_channel_playing(int channel)
{
	if ( Channels[channel].source_id != 0 ) {
		ALint status;

		OpenAL_ErrorPrint( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &status) );

		return (status == AL_PLAYING);
	}

	return 0;
}

/**
 * @todo Documentation
 */
void ds_stop_channel(int channel)
{
	if ( Channels[channel].source_id != 0 ) {
		OpenAL_ErrorPrint( alSourceStop(Channels[channel].source_id) );
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
void ds_set_volume( int channel, float vol )
{
	if ( (channel < 0) || (channel >= MAX_CHANNELS) ) {
		return;
	}

	ALuint source_id = Channels[channel].source_id;

	if (source_id != 0) {
		CAP(vol, 0.0f, 1.0f);
		OpenAL_ErrorPrint( alSourcef(source_id, AL_GAIN, vol) );
	}
}

/**
 * Set the pan for a channel.  The pan is expected to be in DirectSound units
 */
void ds_set_pan( int channel, float pan )
{
	if ( (channel < 0) || (channel >= MAX_CHANNELS) ) {
		return;
	}

	ALint state;

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &state), return );

	if (state == AL_PLAYING) {
		OpenAL_ErrorPrint( alSourcei(Channels[channel].source_id, AL_SOURCE_RELATIVE, AL_TRUE) );
		OpenAL_ErrorPrint( alSource3f(Channels[channel].source_id, AL_POSITION, pan, 0.0f, 0.0f) );
	}
}

/**
 * Get the pitch of a channel
 */
int ds_get_pitch(int channel)
{
	ALint status;
	ALfloat alpitch = 0;
	int pitch;

	if ( (channel < 0) || (channel >= MAX_CHANNELS) ) {
		return -1;
	}

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &status), return -1 );

	if (status == AL_PLAYING) {
		OpenAL_ErrorPrint( alGetSourcef(Channels[channel].source_id, AL_PITCH, &alpitch) );
	}

	// convert OpenAL values to DirectSound values and return
	pitch = fl2i( pow(10.0, (alpitch + 2.0)) );

	return pitch;
}

/**
 * Set the pitch of a channel
 */
void ds_set_pitch(int channel, int pitch)
{
	ALint status;

	if ( (channel < 0) || (channel >= MAX_CHANNELS) ) {
		return;
	}

	if ( pitch < MIN_PITCH ) {
		pitch = MIN_PITCH;
	}

	if ( pitch > MAX_PITCH ) {
		pitch = MAX_PITCH;
	}

	OpenAL_ErrorCheck( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &status), return );

	if (status == AL_PLAYING) {
		ALfloat alpitch = log10f((float)pitch) - 2.0f;
		OpenAL_ErrorPrint( alSourcef(Channels[channel].source_id, AL_PITCH, alpitch) );
	}
}

/**
 * @todo Documentation
 */
void ds_chg_loop_status(int channel, int loop)
{
	if ( (channel < 0) || (channel >= MAX_CHANNELS) ) {
		return;
	}

	ALuint source_id = Channels[channel].source_id;

	OpenAL_ErrorPrint( alSourcei(source_id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE) );
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
int ds3d_play(int sid, int snd_id, vec3d *pos, vec3d *vel, float min, float max, int looping, float max_volume, float estimated_vol, int priority )
{
	int channel;

	if (!ds_initialized) {
		return -1;
	}

	channel = ds_get_free_channel(estimated_vol, snd_id, priority);

	if (channel < 0) {
		return -1;
	}

	if ( Channels[channel].source_id == 0 ) {
		return -1;
	}

	// set up 3D sound data here
	ds3d_update_buffer(channel, min, max, pos, vel);


	OpenAL_ErrorPrint( alSourcef(Channels[channel].source_id, AL_PITCH, 1.0f) );

	OpenAL_ErrorPrint( alSourcef(Channels[channel].source_id, AL_GAIN, max_volume) );

	ALint status;
	OpenAL_ErrorCheck( alGetSourcei(Channels[channel].source_id, AL_SOURCE_STATE, &status), return -1 );
	
	if (status == AL_PLAYING) {
		OpenAL_ErrorPrint( alSourceStop(Channels[channel].source_id) );
	}


	OpenAL_ErrorCheck( alSourcei(Channels[channel].source_id, AL_BUFFER, sound_buffers[sid].buf_id), return -1 );

	if (Ds_eax_inited) {
		OpenAL_ErrorPrint( alSource3i(Channels[channel].source_id, AL_AUXILIARY_SEND_FILTER, AL_EFX_aux_id, 0, AL_FILTER_NULL) );
	}

	OpenAL_ErrorPrint( alSourcei(Channels[channel].source_id, AL_SOURCE_RELATIVE, AL_FALSE) );

	OpenAL_ErrorPrint( alSourcei(Channels[channel].source_id, AL_LOOPING, (looping) ? AL_TRUE : AL_FALSE) );

	OpenAL_ErrorPrint( alSourcePlay(Channels[channel].source_id) );


	sound_buffers[sid].channel_id = channel;

	Channels[channel].sid = sid;
	Channels[channel].snd_id = snd_id;
	Channels[channel].sig = channel_next_sig++;
	Channels[channel].last_position = 0;
	Channels[channel].is_voice_msg = false;
	Channels[channel].vol = max_volume;
	Channels[channel].looping = looping;
	Channels[channel].priority = priority;

	if (channel_next_sig < 0 ) {
		channel_next_sig = 1;
	}

	return Channels[channel].sig;
}

/**
 * @todo Documentation
 */
void ds_set_position(int channel, unsigned int offset)
{
	if ( (channel < 0) || (channel >= MAX_CHANNELS) ) {
		return;
	}

	OpenAL_ErrorPrint( alSourcei(Channels[channel].source_id, AL_BYTE_OFFSET, offset) );
}

/**
 * @todo Documentation
 */
unsigned int ds_get_play_position(int channel)
{
	if ( (channel < 0) || (channel >= MAX_CHANNELS) ) {
		return 0;
	}

	ALint pos = -1;
	int	sid = Channels[channel].sid;

	if ( (sid < 0) || ((size_t)sid >= sound_buffers.size()) ) {
		return 0;
	}

	if (AL_play_position) {
		OpenAL_ErrorPrint( alGetSourcei(Channels[channel].source_id, AL_BYTE_LOKI, &pos) );

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
		OpenAL_ErrorPrint( alGetSourcei(Channels[channel].source_id, AL_BYTE_OFFSET, &pos) );

		if (pos < 0) {
			pos = 0;
		}
	}

	return (unsigned int) pos;
}

/**
 * @todo Documentation
 */
unsigned int ds_get_write_position(int channel)
{
	return 0;
}

/**
 * @todo Documentation
 */
int ds_get_channel_size(int channel)
{
	if ( (channel < 0) || (channel >= MAX_CHANNELS) ) {
		return 0;
	}

	int sid = Channels[channel].sid;

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
int ds_get_data(int sid, char *data)
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
 * Set up the environment type for all sound sources.
 *
 * @param envid Value from the EAX_ENVIRONMENT_* enumeration in ds_eax.h
 * @return Always returns 0.
 * @todo Proper error reporting, otherwise make a void return type.
 */
int ds_eax_set_environment(unsigned long envid)
{
	al_efx_load_preset(envid);

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
		Ds_active_env = -1;
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
			return i;
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
		id = EFX_presets.size();

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
		if (Ds_active_env < 0) {
			return -1;
		}

		er->environment = Ds_active_env;

		OpenAL_ErrorPrint( v_alGetEffectf(AL_EFX_effect_id, AL_EAXREVERB_GAIN, &er->fVolume) );
		OpenAL_ErrorPrint( v_alGetEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_TIME, &er->fDecayTime_sec) );
		OpenAL_ErrorPrint( v_alGetEffectf(AL_EFX_effect_id, AL_EAXREVERB_DECAY_HFRATIO, &er->fDamping) );
	} else if (id < (int)EFX_presets.size()) {
		er->environment = (unsigned int)id;

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
	} catch (const char *err) {
		mprintf(("\n  EFX:  Unable to load function: %s()\n", err));

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
int ds_get_sound_id(int channel)
{
	Assert( channel >= 0 );

	return Channels[channel].snd_id;
}

