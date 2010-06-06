
#include "globalincs/pstypes.h"
#include "sound/openal.h"
#include "osapi/osregistry.h"

#include <string>
#include <algorithm>

#ifdef _WIN32
#define VC_EXTRALEAN
#include <windows.h>
#endif


static std::string Playback_device;
static std::string Capture_device;


enum {
	OAL_DEVICE_GENERIC,
	OAL_DEVICE_DEFAULT,
	OAL_DEVICE_USER
};

typedef struct OALdevice {
	std::string device_name;
	int type;
	bool usable;

	OALdevice() :
		type(OAL_DEVICE_GENERIC), usable(false)
	{
	}

	OALdevice(const char *name) :
		device_name(name), type(OAL_DEVICE_GENERIC), usable(false)
	{
	}
} OALdevice;

static SCP_vector<OALdevice> PlaybackDevices;
static SCP_vector<OALdevice> CaptureDevices;


// enumeration extension
#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER                0x1013
#endif


//--------------------------------------------------------------------------
// openal_error_string()
//
// Returns the human readable error string if there is an error or NULL if not
//
const char *openal_error_string(int get_alc)
{
	int i;

	if (get_alc) {
		ALCdevice *device = alcGetContextsDevice( alcGetCurrentContext() );

		i = alcGetError(device);

		if ( i != ALC_NO_ERROR )
			return (const char*) alcGetString(device, i);
	}
	else {
		i = alGetError();

		if ( i != AL_NO_ERROR )
			return (const char*)alGetString(i);
	}

	return NULL;
}

ALenum openal_get_format(ALint bits, ALint n_channels)
{
	ALenum format = AL_INVALID_VALUE;

	if ( (n_channels < 1) || (n_channels > 2) ) {
		return format;
	}

	switch (bits) {
		case 32:
			format = (n_channels == 1) ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
			break;

		case 16:
			format = (n_channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
			break;

		case 8:
			format = (n_channels == 1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
			break;
	}

	return format;
}

static bool openal_device_sort_func(const OALdevice &d1, const OALdevice &d2)
{
	if (d1.type > d2.type) {
		return true;
	}

	return false;
}

static void find_playback_device()
{
	const char *user_device = os_config_read_string( "Sound", "PlaybackDevice", NULL );
	const char *default_device = (const char*) alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );

	// in case they are the same, we only want to test it once
	if ( (user_device && default_device) && !strcmp(user_device, default_device) ) {
		user_device = NULL;
	}

    if ( alcIsExtensionPresent(NULL, (const ALCchar*)"ALC_ENUMERATION_EXT") == AL_TRUE ) {
		const char *all_devices = NULL;

        if ( alcIsExtensionPresent(NULL, (const ALCchar*)"ALC_ENUMERATE_ALL_EXT") == AL_TRUE ) {
			all_devices = (const char*) alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
		} else {
			all_devices = (const char*) alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		}

		const char *str_list = all_devices;
		int ext_length = 0;

		if ( (str_list != NULL) && ((ext_length = strlen(str_list)) > 0) ) {
			while (ext_length) {
				OALdevice new_device(str_list);

				if (user_device && !strcmp(str_list, user_device)) {
					new_device.type = OAL_DEVICE_USER;
				} else if (default_device && !strcmp(str_list, default_device)) {
					new_device.type = OAL_DEVICE_DEFAULT;
				}

				PlaybackDevices.push_back( new_device );

				str_list += (ext_length + 1);
				ext_length = strlen(str_list);
			}
		}
	} else {
		if (default_device) {
			OALdevice new_device(default_device);
			new_device.type = OAL_DEVICE_DEFAULT;

			PlaybackDevices.push_back( new_device );
		}

		if (user_device) {
			OALdevice new_device(user_device);
			new_device.type = OAL_DEVICE_USER;

			PlaybackDevices.push_back( new_device );
		}
	}

	if ( PlaybackDevices.empty() ) {
		return;
	}

	std::sort( PlaybackDevices.begin(), PlaybackDevices.end(), openal_device_sort_func );


	ALCdevice *device = NULL;
	ALCcontext *context = NULL;

	// for each device that we have available, try and figure out which to use
	for (size_t idx = 0; idx < PlaybackDevices.size(); idx++) {
		OALdevice *pdev = &PlaybackDevices[idx];

		// open our specfic device
		device = alcOpenDevice( (const ALCchar*) pdev->device_name.c_str() );

		if (device == NULL) {
			continue;
		}

		context = alcCreateContext(device, NULL);

		if (context == NULL) {
			alcCloseDevice(device);
			continue;
		}

		alcMakeContextCurrent(context);
		alcGetError(device);

		// check how many sources we can create
		static const int MIN_SOURCES = 48;	// MAX_CHANNELS + 16 spare
		int si = 0;

		for (si = 0; si < MIN_SOURCES; si++) {
			ALuint source_id = 0;
			alGenSources(1, &source_id);

			if (alGetError() != AL_NO_ERROR) {
				break;
			}

			alDeleteSources(1, &source_id);
		}

		if (si == MIN_SOURCES) {
			// ok, it supports our minimum requirements
			pdev->usable = true;

			// need this for the future
			Playback_device = pdev->device_name;

			// done
			break;
		} else {
			// clean up for next pass
			alcMakeContextCurrent(NULL);
			alcDestroyContext(context);
			alcCloseDevice(device);

			context = NULL;
			device = NULL;
		}
	}

	alcMakeContextCurrent(NULL);

	if (context) {
		alcDestroyContext(context);
	}

	if (device) {
		alcCloseDevice(device);
	}
}

static void find_capture_device()
{
	const char *user_device = os_config_read_string( "Sound", "CaptureDevice", NULL );
	const char *default_device = (const char*) alcGetString( NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER );

	// in case they are the same, we only want to test it once
	if ( (user_device && default_device) && !strcmp(user_device, default_device) ) {
		user_device = NULL;
	}

    if ( alcIsExtensionPresent(NULL, (const ALCchar*)"ALC_ENUMERATION_EXT") == AL_TRUE ) {
		const char *all_devices = (const char*) alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

		const char *str_list = all_devices;
		int ext_length = 0;

		if ( (str_list != NULL) && ((ext_length = strlen(str_list)) > 0) ) {
			while (ext_length) {
				OALdevice new_device(str_list);

				if (user_device && !strcmp(str_list, user_device)) {
					new_device.type = OAL_DEVICE_USER;
				} else if (default_device && !strcmp(str_list, default_device)) {
					new_device.type = OAL_DEVICE_DEFAULT;
				}

				CaptureDevices.push_back( new_device );

				str_list += (ext_length + 1);
				ext_length = strlen(str_list);
			}
		}
	} else {
		if (default_device) {
			OALdevice new_device(default_device);
			new_device.type = OAL_DEVICE_DEFAULT;

			CaptureDevices.push_back( new_device );
		}

		if (user_device) {
			OALdevice new_device(user_device);
			new_device.type = OAL_DEVICE_USER;

			CaptureDevices.push_back( new_device );
		}
	}

	if ( CaptureDevices.empty() ) {
		return;
	}

	std::sort( CaptureDevices.begin(), CaptureDevices.end(), openal_device_sort_func );


	// for each device that we have available, try and figure out which to use
	for (size_t idx = 0; idx < CaptureDevices.size(); idx++) {
		const ALCchar *device_name = CaptureDevices[idx].device_name.c_str();

		ALCdevice *device = alcCaptureOpenDevice(device_name, 22050, AL_FORMAT_MONO8, 22050 * 2);

		if (device == NULL) {
			continue;
		}

		if (alcGetError(device) != ALC_NO_ERROR) {
			alcCaptureCloseDevice(device);
			continue;
		}

		// ok, we should be good with this one
		Capture_device = CaptureDevices[idx].device_name;

		alcCaptureCloseDevice(device);

		break;
	}
}

// initializes hardware device from perferred/default/enumerated list
bool openal_init_device(std::string *playback, std::string *capture)
{
	if ( !Playback_device.empty() ) {
		if (playback) {
			*playback = Playback_device;
		}

		if (capture) {
			*capture = Capture_device;
		}

		return true;
	}

	if (playback) {
		playback->erase();
	}

	if (capture) {
		capture->erase();
	}

	// initialize default setup first, for version check...

	ALCdevice *device = alcOpenDevice(NULL);

	if (device == NULL) {
		return false;
	}

	ALCcontext *context = alcCreateContext(device, NULL);

	if (context == NULL) {
		alcCloseDevice(device);
		return false;
	}

	alcMakeContextCurrent(context);

	// version check (for 1.0 or 1.1)
	ALCint AL_minor_version = 0;
	alcGetIntegerv(NULL, ALC_MINOR_VERSION, sizeof(ALCint), &AL_minor_version);

	if (AL_minor_version < 1) {
#ifdef _WIN32
		MessageBox(NULL, "OpenAL 1.1 or newer is required for proper operation.  Please upgrade your OpenAL drivers, which\nare available at http://www.openal.org/downloads.html, and try running the game again.", NULL, MB_OK);
#else
		printf("OpenAL 1.1 or newer is required for proper operation.\n");
		printf("Please upgrade to a newer version if on OS X or switch\n");
		printf("to OpenAL-Soft on Linux.\n");
#endif

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);

		return false;
	}

	alcGetError(device);

	mprintf(( "  OpenAL Vendor     : %s\n", alGetString( AL_VENDOR ) ));
	mprintf(( "  OpenAL Renderer   : %s\n", alGetString( AL_RENDERER ) ));
	mprintf(( "  OpenAL Version    : %s\n", alGetString( AL_VERSION ) ));
	mprintf(( "\n" ));

	// close default device
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);


	// go through and find out what devices we actually want to use ...
	find_playback_device();
	find_capture_device();

	if ( Playback_device.empty() ) {
		return false;
	}


#ifndef NDEBUG
	if ( !PlaybackDevices.empty() ) {
		nprintf(("OpenAL", "  Available Playback Devices:\n"));

		for (size_t idx = 0; idx < PlaybackDevices.size(); idx++) {
			nprintf(("OpenAL", "    %s", PlaybackDevices[idx].device_name.c_str()));

			if (PlaybackDevices[idx].type == OAL_DEVICE_USER) {
				nprintf(("OpenAL", "  *preferred*\n"));
			} else if (PlaybackDevices[idx].type == OAL_DEVICE_DEFAULT) {
				nprintf(("OpenAL", "  *default*\n"));
			} else {
				nprintf(("OpenAL", "\n"));
			}
		}
	}

	if ( !CaptureDevices.empty() ) {
		if ( !PlaybackDevices.empty() ) {
			nprintf(("OpenAL", "\n"));
		}

		nprintf(("OpenAL", "  Available Capture Devices:\n"));

		for (size_t idx = 0; idx < CaptureDevices.size(); idx++) {
			nprintf(("OpenAL", "    %s", CaptureDevices[idx].device_name.c_str()));

			if (CaptureDevices[idx].type == OAL_DEVICE_USER) {
				nprintf(("OpenAL", "  *preferred*\n"));
			} else if (CaptureDevices[idx].type == OAL_DEVICE_DEFAULT) {
				nprintf(("OpenAL", "  *default*\n"));
			} else {
				nprintf(("OpenAL", "\n"));
			}
		}

		nprintf(("OpenAL", "\n"));
	}
#endif


	// cleanup
	PlaybackDevices.clear();
	CaptureDevices.clear();


	if (playback) {
		*playback = Playback_device;
	}

	if (capture) {
		*capture = Capture_device;
	}

	return true;
}

