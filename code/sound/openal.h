
#ifndef _AL_H
#define _AL_H


#if defined(__APPLE__)
#include "al.h"
#include "alc.h"
#include "alext.h"
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#endif // defined(__APPLE__)

#include <string>


const char* openal_error_string(int get_alc = 0);
bool openal_init_device(SCP_string *playback, SCP_string *capture);

ALenum openal_get_format(ALint bits, ALint n_channels);

struct OpenALInformation {
	uint32_t version_major = 0;
	uint32_t version_minor = 0;

	SCP_string default_playback_device{};
	SCP_string default_capture_device{};

	SCP_vector<SCP_string> playback_devices;
	SCP_vector<SCP_string> capture_devices;

	SCP_vector<std::pair<SCP_string, bool>> efx_support;
};

OpenALInformation openal_get_platform_information();

int openal_find_playback_device_by_name(const SCP_string& device);
int openal_find_capture_device_by_name(const SCP_string& device);

// if an error occurs after executing 'x' then do 'y'
#define OpenAL_ErrorCheck( x, y )	do {	\
	x;	\
	const char *error_text = openal_error_string(0);	\
	if ( error_text != NULL ) {	\
		nprintf(("Warning", "SOUND: %s:%d - OpenAL error = '%s'\n", __FILE__, __LINE__, error_text));	\
		y;	\
	}	\
} while (false);

// like OpenAL_ErrorCheck() except that it gives the error message from x but does nothing about it
#define OpenAL_ErrorPrint( x )	do {	\
	x;	\
	const char *error_text = openal_error_string(0);	\
	if ( error_text != NULL ) {	\
		nprintf(("Sound", "OpenAL ERROR: \"%s\" in %s, line %i\n", error_text, __FILE__, __LINE__));	\
	}	\
} while (0);

// same as the above two, but looks for ALC errors instead of standard AL errors
#define OpenAL_C_ErrorCheck( x, y )	do {	\
	x;	\
	const char *error_text = openal_error_string(1);	\
	if ( error_text != NULL ) {	\
		nprintf(("Warning", "SOUND: %s:%d - OpenAL error = '%s'\n", __FILE__, __LINE__, error_text));	\
		y;	\
	}	\
} while (0);

// like OpenAL_ErrorCheck() except that it gives the error message from x but does nothing about it
#define OpenAL_C_ErrorPrint( x )	do {	\
	x;	\
	const char *error_text = openal_error_string(1);	\
	if ( error_text != NULL ) {	\
		nprintf(("Sound", "OpenAL ERROR: \"%s\" in %s, line %i\n", error_text, __FILE__, __LINE__));	\
	}	\
} while (0);


// not define by older OpenAL versions
#ifndef AL_BYTE_OFFSET
#define AL_BYTE_OFFSET	0x1026
#endif

#ifndef ALC_EXT_EFX
#define ALC_EXT_EFX	 1
#define AL_FILTER_TYPE					0x8001
#define AL_EFFECT_TYPE					0x8001
#define AL_FILTER_NULL					0x0000
#define AL_FILTER_LOWPASS				0x0001
#define AL_EFFECT_NULL					0x0000
#define AL_EFFECT_EAXREVERB			0x8000
#define AL_EFFECT_REVERB				0x0001
#define AL_EFFECT_ECHO					0x0004
#define ALC_EFX_MAJOR_VERSION			0x20001
#define ALC_EFX_MINOR_VERSION			0x20002
#define ALC_MAX_AUXILIARY_SENDS		0x20003

#define AL_AUXILIARY_SEND_FILTER		0x20006

#define AL_EAXREVERB_DENSITY					0x0001
#define AL_EAXREVERB_DIFFUSION					0x0002
#define AL_EAXREVERB_GAIN						0x0003
#define AL_EAXREVERB_GAINHF					0x0004
#define AL_EAXREVERB_GAINLF					0x0005
#define AL_EAXREVERB_DECAY_TIME				0x0006
#define AL_EAXREVERB_DECAY_HFRATIO				0x0007
#define AL_EAXREVERB_DECAY_LFRATIO				0x0008
#define AL_EAXREVERB_REFLECTIONS_GAIN			0x0009
#define AL_EAXREVERB_REFLECTIONS_DELAY			0x000A
#define AL_EAXREVERB_REFLECTIONS_PAN			0x000B
#define AL_EAXREVERB_LATE_REVERB_GAIN			0x000C
#define AL_EAXREVERB_LATE_REVERB_DELAY			0x000D
#define AL_EAXREVERB_LATE_REVERB_PAN			0x000E
#define AL_EAXREVERB_ECHO_TIME					0x000F
#define AL_EAXREVERB_ECHO_DEPTH				0x0010
#define AL_EAXREVERB_MODULATION_TIME			0x0011
#define AL_EAXREVERB_MODULATION_DEPTH			0x0012
#define AL_EAXREVERB_AIR_ABSORPTION_GAINHF	0x0013
#define AL_EAXREVERB_HFREFERENCE				0x0014
#define AL_EAXREVERB_LFREFERENCE				0x0015
#define AL_EAXREVERB_ROOM_ROLLOFF_FACTOR		0x0016
#define AL_EAXREVERB_DECAY_HFLIMIT				0x0017

#define AL_EFFECTSLOT_NULL						0x0000
#define AL_EFFECTSLOT_EFFECT					0x0001
#endif

#ifndef AL_EXT_float32
#define AL_EXT_float32 1
#define AL_FORMAT_MONO_FLOAT32			0x10010
#define AL_FORMAT_STEREO_FLOAT32			0x10011
#endif

#ifndef ALC_SOFT_loopback
#define ALC_FORMAT_CHANNELS_SOFT				0x1990
#define ALC_FORMAT_TYPE_SOFT					0x1991

/* Sample types */
#define ALC_BYTE_SOFT							0x1400
#define ALC_UNSIGNED_BYTE_SOFT					0x1401
#define ALC_SHORT_SOFT							0x1402
#define ALC_UNSIGNED_SHORT_SOFT					0x1403
#define ALC_INT_SOFT							0x1404
#define ALC_UNSIGNED_INT_SOFT					0x1405
#define ALC_FLOAT_SOFT							0x1406

/* Channel configurations */
#define ALC_MONO_SOFT							0x1500
#define ALC_STEREO_SOFT							0x1501
#define ALC_QUAD_SOFT							0x1503
#define ALC_5POINT1_SOFT						0x1504
#define ALC_6POINT1_SOFT						0x1505
#define ALC_7POINT1_SOFT						0x1506
#endif

#endif	// _AL_H
