/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/ds.h $
 * $Revision: 2.2 $
 * $Date: 2003-03-02 06:37:24 $
 * $Author: penguin $
 *
 * Header file for interface to DirectSound
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     8/27/99 6:38p Alanl
 * crush the blasted repeating messages bug
 * 
 * 4     8/01/99 2:06p Alanl
 * increase the rolloff for A3D
 * 
 * 3     5/23/99 8:11p Alanl
 * Added support for EAX
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 21    5/05/98 4:49p Lawrance
 * Put in code to authenticate A3D, improve A3D support
 * 
 * 20    4/20/98 12:03a Lawrance
 * Allow prioritizing of CTRL3D buffers
 * 
 * 19    4/18/98 9:12p Lawrance
 * Added Aureal support.
 * 
 * 18    4/13/98 5:04p Lawrance
 * Write functions to determine how many milliseconds are left in a sound
 * 
 * 17    3/23/98 10:32a Lawrance
 * Add functions for extracting raw sound data
 * 
 * 16    2/15/98 11:10p Lawrance
 * more work on real-time voice system
 * 
 * 15    2/15/98 4:43p Lawrance
 * work on real-time voice
 * 
 * 14    2/06/98 7:30p John
 * Added code to monitor the number of channels of sound actually playing.
 * 
 * 13    2/06/98 8:56a Allender
 * fixed calling convention problem with DLL handles
 * 
 * 12    2/04/98 6:08p Lawrance
 * Read function pointers from dsound.dll, further work on
 * DirectSoundCapture.
 * 
 * 11    1/31/98 5:48p Lawrance
 * Start on real-time voice recording
 * 
 * 10    12/05/97 5:19p Lawrance
 * re-do sound priorities to make more general and extensible
 * 
 * 9     11/20/97 5:36p Dave
 * Hooked in a bunch of main hall changes (including sound). Made it
 * possible to reposition (rewind/ffwd) 
 * sound buffer pointers. Fixed animation direction change framerate
 * problem.
 * 
 * 8     10/13/97 7:41p Lawrance
 * store duration of sound
 * 
 * 7     7/28/97 11:39a Lawrance
 * allow individual volume scaling on 3D buffers
 * 
 * 6     7/17/97 9:32a John
 * made all directX header files name start with a v
 * 
 * 5     7/15/97 11:15a Lawrance
 * limit the max instances of simultaneous sound effects, implement
 * priorities to force critical sounds
 * 
 * 4     6/09/97 11:50p Lawrance
 * integrating DirectSound3D
 * 
 * 3     6/02/97 1:45p Lawrance
 * implementing hardware mixing
 * 
 * 2     5/29/97 12:04p Lawrance
 * creation of file to hold DirectSound specific portions
 *
 * $NoKeywords: $
 */

#ifndef __DS_H__
#define __DS_H__

#include <windows.h>
#include "directx/vdsound.h"
#include "globalincs/pstypes.h"

#define DS_HARDWARE	(1<<0)
#define DS_3D			(1<<1)
#define DS_USE_DS3D	(1<<2)

// Constants that DirectSound should assign, but doesn't
#define MAX_PAN		1500.0f
#define MIN_PITCH		100
#define MAX_PITCH		100000


// limits placed on how many concurrent sounds of the same type can play simultaneously
#define DS_MUST_PLAY				0
#define DS_LIMIT_ONE				1
#define DS_LIMIT_TWO				2
#define DS_LIMIT_THREE			3

#define DS_DUP_FAILED			-99

typedef struct sound_info {
	int	format;		// WAVE_FORMAT_* defines from mmreg.h
	uint	size;
	int	sample_rate;
	int	avg_bytes_per_sec;
	int	n_block_align;
	int	bits;
	int	n_channels;
	int	duration;	// time in ms for duration of sound
	ubyte	*data;
} sound_info;

extern int							ds_initialized;
extern LPDIRECTSOUNDBUFFER		pPrimaryBuffer;
extern LPDIRECTSOUND				pDirectSound;

extern HRESULT (__stdcall *pfn_DirectSoundCaptureCreate)(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC, LPUNKNOWN pUnkOuter);

int	ds_init(int use_a3d, int use_eax);
void	ds_close();
void	ds_get_primary_format(WAVEFORMATEX *wfx);
int	ds_parse_wave(char *filename, ubyte **dest, uint *dest_size, WAVEFORMATEX **header);
int	ds_load_buffer(int *sid, int *hid, int *final_size, void *header, sound_info *si, int flags);
void	ds_unload_buffer(int sid, int hid);
int	ds_play(int sid, int hid, int snd_id, int priority, int volume, int pan, int looping, bool is_voice_msg = false);
int	ds_convert_volume(float volume);		// Convert a volume from 0.0f->1.0f to -10000 -> 0
float ds_get_percentage_vol(int ds_vol);	// Convert a volume from -10000 -> 0 to 0.0f->1.0f
int	ds_get_channel(int sig);
int	ds_is_channel_playing(int channel);
void	ds_stop_channel(int channel);
void	ds_stop_channel_all();
void	ds_set_volume( int channel, int vol );
void	ds_set_pan( int channel, int pan );
int	ds_get_pitch(int channel);
void	ds_set_pitch(int channel, int pitch);
void	ds_chg_loop_status(int channel, int loop);
void  ds_set_position(int channel, DWORD offset);
DWORD ds_get_play_position(int channel);
DWORD ds_get_write_position(int channel);
int	ds_get_data(int sid, char *data);
int	ds_get_size(int sid, int *size);

int	ds_create_buffer(int frequency, int bits_per_sample, int nchannels, int nseconds);
int	ds_lock_data(int sid, unsigned char *data, int size);
int	ds_play_easy(int sid, int volume);
void	ds_stop_easy(int sid);
int	ds_get_channel_size(int channel);
int	ds_is_3d_buffer(int sid);
int	ds_using_ds3d();
bool	ds_using_a3d();

unsigned int ds_get_primary_buffer_interface();
unsigned int ds_get_dsound_interface();
unsigned int ds_get_property_set_interface();

// Returns the number of channels that are actually playing
int ds_get_number_channels();

int ds3d_play( int sid, int hid, int snd_id, vector *pos, vector *vel, int min, int max, int looping, int max_volume, int estimated_vol, int priority=DS_MUST_PLAY );

// Get a character string for the error code
char	*get_DSERR_text(int DSResult);

void ds_do_frame();

// --------------------
//
// Creative eax.h
//
// --------------------

// EAX (listener) reverb property set {4a4e6fc1-c341-11d1-b73a-444553540000}
DEFINE_GUID(DSPROPSETID_EAX_ReverbProperties, 
    0x4a4e6fc1,
    0xc341,
    0x11d1,
    0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);

typedef enum 
{
    DSPROPERTY_EAX_ALL,                // all reverb properties
    DSPROPERTY_EAX_ENVIRONMENT,        // standard environment no.
    DSPROPERTY_EAX_VOLUME,             // loudness of the reverb
    DSPROPERTY_EAX_DECAYTIME,          // how long the reverb lasts
    DSPROPERTY_EAX_DAMPING             // the high frequencies decay faster
} DSPROPERTY_EAX_REVERBPROPERTY;

#define EAX_NUM_STANDARD_PROPERTIES (DSPROPERTY_EAX_DAMPING + 1)

// use this structure for get/set all properties...
typedef struct 
{
    unsigned long environment;          // 0 to EAX_ENVIRONMENT_COUNT-1
    float fVolume;                      // 0 to 1
    float fDecayTime_sec;               // seconds, 0.1 to 100
    float fDamping;                     // 0 to 1
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

    EAX_ENVIRONMENT_COUNT           // total number of environments
};

#define EAX_MAX_ENVIRONMENT (EAX_ENVIRONMENT_COUNT - 1)

// EAX buffer reverb property set {4a4e6fc0-c341-11d1-b73a-444553540000}
DEFINE_GUID(DSPROPSETID_EAXBUFFER_ReverbProperties, 
    0x4a4e6fc0,
    0xc341,
    0x11d1,
    0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);

typedef enum 
{
    DSPROPERTY_EAXBUFFER_ALL,           // all reverb buffer properties
    DSPROPERTY_EAXBUFFER_REVERBMIX      // the wet source amount
} DSPROPERTY_EAXBUFFER_REVERBPROPERTY;

// use this structure for get/set all properties...
typedef struct 
{
    float fMix;                          // linear factor, 0.0F to 1.0F
} EAXBUFFER_REVERBPROPERTIES;

#define EAX_REVERBMIX_USEDISTANCE -1.0F // out of normal range
                                         // signifies the reverb engine should
                                         // calculate it's own reverb mix value
                                         // based on distance

// prototypes

int ds_eax_init();
void ds_eax_close();

int ds_eax_set_preset(unsigned long envid);

int ds_eax_set_volume(float volume);
int ds_eax_set_decay_time(float seconds);
int ds_eax_set_damping(float damp);
int ds_eax_set_environment(unsigned long envid);
int ds_eax_set_all(unsigned long id, float volume, float damping, float decay);
int ds_eax_get_all(EAX_REVERBPROPERTIES *er);
int ds_eax_is_inited();

#endif /* __DS_H__ */
