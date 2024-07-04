
#include "globalincs/pstypes.h"
#include "sound/openal.h"
#include "osapi/osregistry.h"
#include "options/Option.h"
#include "parse/parselo.h"
#include "libs/jansson.h"

#include <string>
#include <algorithm>

#ifdef _WIN32
#define VC_EXTRALEAN
#include <windows.h>
#endif

// Stupid windows workaround...
#ifdef MessageBox
#undef MessageBox
#endif


static SCP_string Playback_device;
static SCP_string Capture_device;


enum {
	OAL_DEVICE_GENERIC,
	OAL_DEVICE_DEFAULT,
	OAL_DEVICE_USER
};

typedef struct OALdevice {
	SCP_string device_name;
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

// List of audio device names for the in-game option
SCP_vector<SCP_string> PlaybackDeviceList;

static int playbackdevice_deserializer(const json_t* value)
{
	const char* device;

	json_error_t err;
	if (json_unpack_ex((json_t*)value, &err, 0, "s", &device) != 0) {
		throw json_exception(err);
	}

	int id = openal_find_playback_device_by_name(device);

	if (SCP_vector_inbounds(PlaybackDeviceList, id)) {
		return id;
	}

	return -1;
}
static json_t* playbackdevice_serializer(int value)
{
	if (!SCP_vector_inbounds(PlaybackDeviceList, value)) {
		return json_pack("s", "");
	}
	
	return json_pack("s", PlaybackDeviceList[value].c_str());
}
static SCP_vector<int> playbackdevice_enumerator()
{
	SCP_vector<int> vals;
	for (int i = 0; i < static_cast<int>(PlaybackDeviceList.size()); ++i) {
		vals.push_back(i);
	}
	return vals;
}
static SCP_string playbackdevice_display(int id)
{
	if (!SCP_vector_inbounds(PlaybackDeviceList, id)) {
		return "";
	}
	
	SCP_string out;
	sprintf(out, "(%d) %s", id + 1, PlaybackDeviceList[id].c_str());
	return out;
}
static bool playbackdevice_change(int /*device*/, bool initial)
{
	if (initial) {
		return false; // On game boot always return false
	}

	// Looking at ds.cpp's ds_init(), it's probably possible to not require a restart for this
	// but for now let's keep this simple and just restart. If/when this is implemented the device
	// parameter will be required. Currently playing audio will need to be copied from the old device
	// to the new as part of the initialization. This will require deeper changes to the audio code
	// which can be handled later in a follow-up PR.

	return false;
}
static auto PlaybackDeviceOption = options::OptionBuilder<int>("Audio.PlaybackDevice",
                     std::pair<const char*, int>{"Playback Device", 1834},
                     std::pair<const char*, int>{"The device used for audio playback", 1835})
                     .category(std::make_pair("Audio", 1826))
                     .level(options::ExpertLevel::Beginner)
                     .deserializer(playbackdevice_deserializer)
                     .serializer(playbackdevice_serializer)
                     .enumerator(playbackdevice_enumerator)
                     .display(playbackdevice_display)
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .default_val(0)
                     .change_listener(playbackdevice_change)
                     .importance(99)
                     .finish();

// List of audio device names for the in-game option
SCP_vector<SCP_string> CaptureDeviceList;

static int capturedevice_deserializer(const json_t* value)
{
	const char* device;

	json_error_t err;
	if (json_unpack_ex((json_t*)value, &err, 0, "s", &device) != 0) {
		throw json_exception(err);
	}

	int id = openal_find_capture_device_by_name(device);

	if (SCP_vector_inbounds(CaptureDeviceList, id)) {
		return id;
	}

	return -1;
}
static json_t* capturedevice_serializer(int value)
{
	if (!SCP_vector_inbounds(CaptureDeviceList, value)) {
		return json_pack("s", "");
	}
	
	return json_pack("s", CaptureDeviceList[value].c_str());
}
static SCP_vector<int> capturedevice_enumerator()
{
	SCP_vector<int> vals;
	for (int i = 0; i < static_cast<int>(CaptureDeviceList.size()); ++i) {
		vals.push_back(i);
	}
	return vals;
}
static SCP_string capturedevice_display(int id)
{
	if (!SCP_vector_inbounds(CaptureDeviceList, id)) {
		return "";
	}
	
	SCP_string out;
	sprintf(out, "(%d) %s", id + 1, CaptureDeviceList[id].c_str());
	return out;
}
static bool capturedevice_change(int /*device*/, bool initial)
{
	if (initial) {
		return false; // On game boot always return false
	}

	// Looking at ds.cpp's ds_init(), it's probably possible to not require a restart for this
	// but for now let's keep this simple and just restart. If/when this is implemented the device
	// parameter will be required. Currently playing audio will need to be copied from the old device
	// to the new as part of the initialization. This will require deeper changes to the audio code
	// which can be handled later in a follow-up PR.

	return false;
}
static auto CaptureDeviceOption = options::OptionBuilder<int>("Audio.CaptureDevice",
                     std::pair<const char*, int>{"Capture Device", 1836},
                     std::pair<const char*, int>{"The device used for audio capture", 1837})
                     .category(std::make_pair("Audio", 1826))
                     .level(options::ExpertLevel::Beginner)
                     .deserializer(capturedevice_deserializer)
                     .serializer(capturedevice_serializer)
                     .enumerator(capturedevice_enumerator)
                     .display(capturedevice_display)
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .default_val(0)
                     .change_listener(capturedevice_change)
                     .importance(99)
                     .finish();

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

static void find_playback_device(OpenALInformation* info)
{
	// First, build a list of device names for the in-game option to use and pull from
	for (auto& device : info->playback_devices) {
		OALdevice new_device(device.c_str());
		PlaybackDeviceList.push_back(new_device.device_name);
	}
	
	const char* user_device = os_config_read_string("Sound", "PlaybackDevice", nullptr);

	if (Using_in_game_options) {
		if (SCP_vector_inbounds(PlaybackDeviceList, PlaybackDeviceOption->getValue())) {
			user_device = PlaybackDeviceList[PlaybackDeviceOption->getValue()].c_str();
		}
	}

	const char *default_device = info->default_playback_device.c_str();

	// in case they are the same, we only want to test it once
	if ( (user_device && default_device) && !strcmp(user_device, default_device) ) {
		user_device = NULL;
	}

	for (auto& device : info->playback_devices) {
		OALdevice new_device(device.c_str());

		if (user_device && !strcmp(device.c_str(), user_device)) {
			new_device.type = OAL_DEVICE_USER;
		} else if (default_device && !strcmp(device.c_str(), default_device)) {
			new_device.type = OAL_DEVICE_DEFAULT;
		}

		PlaybackDevices.push_back( new_device );
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

static void find_capture_device(OpenALInformation* info)
{
	// First, build a list of device names for the in-game option to use and pull from
	for (auto& device : info->capture_devices) {
		OALdevice new_device(device.c_str());
		CaptureDeviceList.push_back(new_device.device_name);
	}
	
	const char* user_device = os_config_read_string("Sound", "CaptureDevice", nullptr);

	if (Using_in_game_options) {
		if (SCP_vector_inbounds(CaptureDevices, CaptureDeviceOption->getValue())) {
			user_device = CaptureDevices[CaptureDeviceOption->getValue()].device_name.c_str();
		}
	}

	const char *default_device = info->default_capture_device.c_str();

	// in case they are the same, we only want to test it once
	if ( (user_device && default_device) && !strcmp(user_device, default_device) ) {
		user_device = NULL;
	}

	for (auto& device : info->capture_devices) {
		OALdevice new_device(device.c_str());

		if (user_device && !strcmp(device.c_str(), user_device)) {
			new_device.type = OAL_DEVICE_USER;
		} else if (default_device && !strcmp(device.c_str(), default_device)) {
			new_device.type = OAL_DEVICE_DEFAULT;
		}

		CaptureDevices.push_back( new_device );
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

// find a playback device's vector index
int openal_find_playback_device_by_name(const SCP_string& device)
{
	for (int i = 0; i < static_cast<int>(PlaybackDeviceList.size()); i++) {
		if (!stricmp(PlaybackDeviceList[i].c_str(), device.c_str())) {
			return i;
		}
	}

	return -1;
}

// find a capture device's vector index
int openal_find_capture_device_by_name(const SCP_string& device)
{
	for (int i = 0; i < static_cast<int>(CaptureDeviceList.size()); i++) {
		if (!stricmp(CaptureDeviceList[i].c_str(), device.c_str())) {
			return i;
		}
	}

	return -1;
}

// initializes hardware device from perferred/default/enumerated list
bool openal_init_device(SCP_string *playback, SCP_string *capture)
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

	// This reuses the code for the launcher to make sure everything is consistent
	auto platform_info = openal_get_platform_information();

	if (platform_info.version_major <= 1 && platform_info.version_minor < 1) {
		os::dialogs::Message(os::dialogs::MESSAGEBOX_ERROR,
			"OpenAL 1.1 or newer is required for proper operation. On Linux and Windows OpenAL Soft is recommended. If you are on Mac OS X you need to upgrade your OS.");
		return false;
	}

	// go through and find out what devices we actually want to use ...
	find_playback_device(&platform_info);
	find_capture_device(&platform_info);

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

static void get_version_info(OpenALInformation* info) {
	// initialize default setup first, for version check...
	ALCdevice *device = alcOpenDevice(NULL);

	if (device == NULL) {
		return;
	}

	ALCcontext *context = alcCreateContext(device, NULL);

	if (context == NULL) {
		alcCloseDevice(device);
		return;
	}

	alcMakeContextCurrent(context);

	// version check (for 1.0 or 1.1)
	ALCint AL_minor_version = 0;
	ALCint AL_major_version = 0;
	alcGetIntegerv(NULL, ALC_MAJOR_VERSION, sizeof(ALCint), &AL_major_version);
	alcGetIntegerv(NULL, ALC_MINOR_VERSION, sizeof(ALCint), &AL_minor_version);

	info->version_major = static_cast<uint32_t>(AL_major_version);
	info->version_minor = static_cast<uint32_t>(AL_minor_version);

	alcGetError(device);

	// close default device
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

static bool device_supports_efx(const char* device_name) {
	ALCdevice *device = alcOpenDevice(device_name);

	if (device == NULL) {
		return false;
	}

	bool result = false;
	if ( alcIsExtensionPresent(device, (const ALchar*)"ALC_EXT_EFX") == AL_TRUE ) {
		result = true;
	}

	alcCloseDevice(device);
	return result;
}

static void enumerate_playback_devices(OpenALInformation* info) {
	info->playback_devices.clear();
	info->default_playback_device.clear();

	auto default_device = alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );
	if (default_device != nullptr) {
		info->default_playback_device = default_device;
		info->efx_support.push_back(std::make_pair(SCP_string(default_device), device_supports_efx(default_device)));
	}

	if ( alcIsExtensionPresent(NULL, (const ALCchar*)"ALC_ENUMERATION_EXT") == AL_TRUE ) {
		const char *all_devices = NULL;

		if ( alcIsExtensionPresent(NULL, (const ALCchar*)"ALC_ENUMERATE_ALL_EXT") == AL_TRUE ) {
			all_devices = (const char*) alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
		} else {
			all_devices = (const char*) alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		}

		const char *str_list = all_devices;
		size_t ext_length = 0;

		if ( (str_list != NULL) && ((ext_length = strlen(str_list)) > 0) ) {
			while (ext_length) {
				info->playback_devices.push_back( SCP_string(str_list) );
				info->efx_support.push_back(std::make_pair(SCP_string(str_list), device_supports_efx(str_list)));

				str_list += (ext_length + 1);
				ext_length = strlen(str_list);
			}
		}
	} else {
		if (default_device) {
			info->playback_devices.push_back(SCP_string(default_device));
		}
	}
}

static void enumerate_capture_devices(OpenALInformation* info) {
	info->capture_devices.clear();
	info->default_capture_device.clear();

	auto default_device = alcGetString( NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER );
	if (default_device != nullptr) {
		info->default_capture_device = default_device;
	}

	if ( alcIsExtensionPresent(NULL, (const ALCchar*)"ALC_ENUMERATION_EXT") == AL_TRUE ) {
		const char *all_devices = (const char*) alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

		const char *str_list = all_devices;
		size_t ext_length = 0;

		if ( (str_list != NULL) && ((ext_length = strlen(str_list)) > 0) ) {
			while (ext_length) {
				info->capture_devices.push_back( SCP_string(str_list));

				str_list += (ext_length + 1);
				ext_length = strlen(str_list);
			}
		}
	} else {
		if (default_device) {
			info->capture_devices.push_back(SCP_string(default_device));
		}
	}
}

OpenALInformation openal_get_platform_information() {
	OpenALInformation info;

	get_version_info(&info);

	enumerate_playback_devices(&info);
	enumerate_capture_devices(&info);

	return info;
}
