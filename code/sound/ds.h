/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __DS_H__
#define __DS_H__

#include "globalincs/pstypes.h"
#include "sound/ogg/ogg.h"
#include "cfile/cfile.h"

#ifdef _WIN32
#define VC_EXTRALEAN
#include <windows.h>
#include <mmsystem.h>
#endif


// Constants that DirectSound should assign, but doesn't
#define MIN_PITCH		100
#define MAX_PITCH		100000


// User specified sound quality
#define DS_SQ_LOW		0
#define DS_SQ_MEDIUM	1
#define DS_SQ_HIGH		2

extern int Ds_sound_quality;
extern int Ds_float_supported;

// limits placed on how many concurrent sounds of the same type can play simultaneously
#define DS_MUST_PLAY	0
#define DS_LIMIT_ONE	1
#define DS_LIMIT_TWO	2
#define DS_LIMIT_THREE	3

#define DS_3D		(1<<0)

typedef struct sound_info {
	int format;		// WAVE_FORMAT_* defines from mmreg.h
	OggVorbis_File ogg_info;
	uint size;
	int sample_rate;
	int avg_bytes_per_sec;
	int n_block_align;
	int bits;
	int n_channels;
	int duration;	// time in ms for duration of sound
	ubyte *data;
} sound_info;

extern int ds_initialized;

int ds_init();
void ds_close();
int ds_parse_sound(CFILE *fp, ubyte **dest, uint *dest_size, WAVEFORMATEX **header, bool ogg = false, OggVorbis_File *ovf = NULL);
int ds_parse_sound_info(char *real_filename, sound_info *s_info);
int ds_load_buffer(int *sid, int *final_size, void *header, sound_info *si, int flags);
void ds_unload_buffer(int sid);
int ds_play(int sid, int snd_id, int priority, float volume, float pan, int looping, bool is_voice_msg = false);
int ds_get_channel(int sig);
int ds_is_channel_playing(int channel);
void ds_stop_channel(int channel);
void ds_stop_channel_all();
void ds_set_volume( int channel, float vol );
void ds_set_pan( int channel, float pan );
int ds_get_pitch(int channel);
void ds_set_pitch(int channel, int pitch);
void ds_chg_loop_status(int channel, int loop);
void ds_set_position(int channel, unsigned int offset);
unsigned int ds_get_play_position(int channel);
unsigned int ds_get_write_position(int channel);
int ds_get_data(int sid, char *data);
int ds_get_size(int sid, int *size);

int ds_create_buffer(int frequency, int bits_per_sample, int nchannels, int nseconds);
int ds_lock_data(int sid, unsigned char *data, int size);
int ds_play_easy(int sid, float volume);
void ds_stop_easy(int sid);
int ds_get_channel_size(int channel);

int ds_get_sound_id(int channel);

// Returns the number of channels that are actually playing
int ds_get_number_channels();

int ds3d_play( int sid, int snd_id, vec3d *pos, vec3d *vel, float min, float max, int looping, float max_volume, float estimated_vol, int priority=DS_MUST_PLAY );

void ds_do_frame();

// --------------------
//
// Creative eax.h
//
// --------------------

// use this structure for get/set all properties...

// used for const array of default values
typedef struct EFXREVERBPROPERTIES_list
{
	char *name;
	float flDensity;
	float flDiffusion;
	float flGain;
	float flGainHF;
	float flGainLF;
	float flDecayTime;
	float flDecayHFRatio;
	float flDecayLFRatio;
	float flReflectionsGain;
	float flReflectionsDelay;
	float flReflectionsPan[3];
	float flLateReverbGain;
	float flLateReverbDelay;
	float flLateReverbPan[3];
	float flEchoTime;
	float flEchoDepth;
	float flModulationTime;
	float flModulationDepth;
	float flAirAbsorptionGainHF;
	float flHFReference;
	float flLFReference;
	float flRoomRolloffFactor;
	int	iDecayHFLimit;
} EFXREVERBPROPERTIES_list;

typedef struct EFXREVERBPROPERTIES
{
	SCP_string name;
	float flDensity;
	float flDiffusion;
	float flGain;
	float flGainHF;
	float flGainLF;
	float flDecayTime;
	float flDecayHFRatio;
	float flDecayLFRatio;
	float flReflectionsGain;
	float flReflectionsDelay;
	float flReflectionsPan[3];
	float flLateReverbGain;
	float flLateReverbDelay;
	float flLateReverbPan[3];
	float flEchoTime;
	float flEchoDepth;
	float flModulationTime;
	float flModulationDepth;
	float flAirAbsorptionGainHF;
	float flHFReference;
	float flLFReference;
	float flRoomRolloffFactor;
	int	iDecayHFLimit;
	EFXREVERBPROPERTIES()
	{
	}
	EFXREVERBPROPERTIES(const EFXREVERBPROPERTIES_list &list);
} EFXREVERBPROPERTIES;

extern SCP_vector<EFXREVERBPROPERTIES> EFX_presets;

typedef struct
{
	unsigned int environment;	// 0 to EAX_ENVIRONMENT_COUNT-1
	float fVolume;				// 0 to 1
	float fDecayTime_sec;		// seconds, 0.1 to 100
	float fDamping;				// 0 to 1
} EAX_REVERBPROPERTIES;

enum
{
	EAX_ENVIRONMENT_GENERIC,
	EAX_ENVIRONMENT_PADDEDCELL,
	EAX_ENVIRONMENT_ROOM,
	EAX_ENVIRONMENT_BATHROOM,
	EAX_ENVIRONMENT_LIVINGROOM,
	EAX_ENVIRONMENT_STONEROOM,
	EAX_ENVIRONMENT_AUDITORIUM,
	EAX_ENVIRONMENT_CONCERTHALL,
	EAX_ENVIRONMENT_CAVE,
	EAX_ENVIRONMENT_ARENA,
	EAX_ENVIRONMENT_HANGAR,
	EAX_ENVIRONMENT_CARPETEDHALLWAY,
	EAX_ENVIRONMENT_HALLWAY,
	EAX_ENVIRONMENT_STONECORRIDOR,
	EAX_ENVIRONMENT_ALLEY,
	EAX_ENVIRONMENT_FOREST,
	EAX_ENVIRONMENT_CITY,
	EAX_ENVIRONMENT_MOUNTAINS,
	EAX_ENVIRONMENT_QUARRY,
	EAX_ENVIRONMENT_PLAIN,
	EAX_ENVIRONMENT_PARKINGLOT,
	EAX_ENVIRONMENT_SEWERPIPE,
	EAX_ENVIRONMENT_UNDERWATER,
	EAX_ENVIRONMENT_DRUGGED,
	EAX_ENVIRONMENT_DIZZY,
	EAX_ENVIRONMENT_PSYCHOTIC,
	EAX_ENVIRONMENT_COUNT		// total number of environments
};

#define EAX_MAX_ENVIRONMENT (EAX_ENVIRONMENT_COUNT - 1)


// prototypes

int ds_eax_init();
void ds_eax_close();

int ds_eax_get_preset_id(const char *name);
int ds_eax_get_prop(EFXREVERBPROPERTIES **props, const char *name, const char *template_name = NULL);

int ds_eax_set_volume(float volume);
int ds_eax_set_decay_time(float seconds);
int ds_eax_set_damping(float damp);
int ds_eax_set_environment(unsigned long envid);
int ds_eax_set_all(unsigned long id, float volume, float damping, float decay);
int ds_eax_get_all(EAX_REVERBPROPERTIES *er, int id = -1);
int ds_eax_is_inited();

#endif /* __DS_H__ */
