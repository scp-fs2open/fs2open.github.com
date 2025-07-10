
#include "globalincs/pstypes.h"
#include "sound/openal.h"
#include "osapi/osregistry.h"
#include "options/Option.h"
#include "parse/parselo.h"
#include "libs/jansson.h"

#include <algorithm>

// lookback device functionality
typedef ALCdevice* (ALC_APIENTRY *ALCLOOPBACKOPENDEVICESOFT)(const ALCchar*);
typedef ALCboolean (ALC_APIENTRY *ALCISRENDERFORMATSUPPORTEDSOFT)(ALCdevice*,ALCsizei,ALCenum,ALCenum);
typedef void (ALC_APIENTRY *ALCRENDERSAMPLESSOFT)(ALCdevice*,ALCvoid*,ALCsizei);

static ALCLOOPBACKOPENDEVICESOFT alcLoopbackOpenDeviceSOFT = nullptr;
static ALCISRENDERFORMATSUPPORTEDSOFT alcIsRenderFormatSupportedSOFT = nullptr;
static ALCRENDERSAMPLESSOFT alcRenderSamplesSOFT = nullptr;


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

typedef struct audio_context {
	SDL_AudioStream *stream;

	ALCdevice *device;
	ALCcontext *context;

	SCP_vector<uint8_t *> render_buffer;

	int frame_size;
} audio_context;

static audio_context Audio{};

static SCP_vector<OALdevice> PlaybackDevices;
static SCP_vector<OALdevice> CaptureDevices;


// enumeration extension
#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER                0x1013
#endif

// List of audio device names for the in-game option
static SCP_string PlaybackDeviceCompat;

static SDL_AudioDeviceID playbackdevice_deserializer(const json_t* value)
{
	const char* device;

	json_error_t err;
	if (json_unpack_ex((json_t*)value, &err, 0, "s", &device) != 0) {
		throw json_exception(err);
	}

	// store old config value to stay compatible with SDL2 builds
	PlaybackDeviceCompat = device;

	// just use system default
	return SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
}
static json_t* playbackdevice_serializer(SDL_AudioDeviceID /* value */)
{
	// save old config value to stay compatible with SDL2 builds
	return json_pack("s", PlaybackDeviceCompat.c_str());
}
static SCP_vector<SDL_AudioDeviceID> playbackdevice_enumerator()
{
	SCP_vector<SDL_AudioDeviceID> vals;
	// just use system default
	vals.push_back(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
	return vals;
}
static SCP_string playbackdevice_display(SDL_AudioDeviceID id)
{
	SCP_string out;

	// TODO: SDL3 => when using defaults this only works properly with SDL 3.4
	//               so we need a fallback/hack for 3.2

	auto device_name = SDL_GetAudioDeviceName(id);

	if (id == SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK) {
		// hack for SDL 3.2 and default device
		if ( !device_name && Audio.stream ) {
			device_name = SDL_GetAudioDeviceName(SDL_GetAudioStreamDevice(Audio.stream));
		}

		if (device_name) {
			sprintf(out, "System default (%s)", device_name);
		} else {
			out = "System default";
		}
	} else if (device_name) {
		out = device_name;
	} else {
		out = "<unknown>";
	}

	return out;
}
static bool playbackdevice_change(SDL_AudioDeviceID /*device*/, bool initial)
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

static auto PlaybackDeviceOption = options::OptionBuilder<SDL_AudioDeviceID>("Audio.PlaybackDevice",
                     std::pair<const char*, int>{"Playback Device", 1834},
                     std::pair<const char*, int>{"The device used for audio playback", 1835})
                     .category(std::make_pair("Audio", 1826))
                     .level(options::ExpertLevel::Beginner)
                     .deserializer(playbackdevice_deserializer)
                     .serializer(playbackdevice_serializer)
                     .enumerator(playbackdevice_enumerator)
                     .display(playbackdevice_display)
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .default_val(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK)
                     .change_listener(playbackdevice_change)
                     .importance(99)
                     .finish();

// List of audio device names for the in-game option
static SCP_string CaptureDeviceCompat;

static SDL_AudioDeviceID capturedevice_deserializer(const json_t* value)
{
	const char* device;

	json_error_t err;
	if (json_unpack_ex((json_t*)value, &err, 0, "s", &device) != 0) {
		throw json_exception(err);
	}

	// store old config value to stay compatible with SDL2 builds
	CaptureDeviceCompat = device;

	// just use system default
	return SDL_AUDIO_DEVICE_DEFAULT_RECORDING;
}
static json_t* capturedevice_serializer(SDL_AudioDeviceID /* value */)
{
	// save old config value to stay compatible with SDL2 builds
	return json_pack("s", CaptureDeviceCompat.c_str());
}
static SCP_vector<SDL_AudioDeviceID> capturedevice_enumerator()
{
	SCP_vector<SDL_AudioDeviceID> vals;
	// just use system default
	vals.push_back(SDL_AUDIO_DEVICE_DEFAULT_RECORDING);
	return vals;
}
static SCP_string capturedevice_display(SDL_AudioDeviceID id)
{
	SCP_string out;

	// TODO: SDL3 => when using defaults this only works properly with SDL 3.4
	//               so we need a fallback/hack for 3.2

	auto device_name = SDL_GetAudioDeviceName(id);

	if (id == SDL_AUDIO_DEVICE_DEFAULT_RECORDING) {
		// TODO: SDL3 => audio capture stuff

		if (device_name) {
			sprintf(out, "System default (%s)", device_name);
		} else {
			out = "System default";
		}
	} else if (device_name) {
		out = device_name;
	} else {
		out = "<unknown>";
	}

	return out;
}
static bool capturedevice_change(SDL_AudioDeviceID /*device*/, bool initial)
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
static auto CaptureDeviceOption = options::OptionBuilder<SDL_AudioDeviceID>("Audio.CaptureDevice",
                     std::pair<const char*, int>{"Capture Device", 1836},
                     std::pair<const char*, int>{"The device used for audio capture", 1837})
                     .category(std::make_pair("Audio", 1826))
                     .level(options::ExpertLevel::Beginner)
                     .deserializer(capturedevice_deserializer)
                     .serializer(capturedevice_serializer)
                     .enumerator(capturedevice_enumerator)
                     .display(capturedevice_display)
                     .flags({options::OptionFlags::ForceMultiValueSelection})
                     .default_val(SDL_AUDIO_DEVICE_DEFAULT_RECORDING)
                     .change_listener(capturedevice_change)
                     .importance(98)
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

static void SDLCALL openal_render_samples(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
	auto ctx = reinterpret_cast<audio_context *>(userdata);

	if (additional_amount < 0) {
		additional_amount = total_amount;
	}

	if (additional_amount <= 0) {
		return;
	}

	if (static_cast<size_t>(additional_amount) > ctx->render_buffer.size()) {
		ctx->render_buffer.resize(additional_amount);
	}

	alcRenderSamplesSOFT(ctx->device, ctx->render_buffer.data(), additional_amount / ctx->frame_size);

	SDL_PutAudioStreamData(stream, ctx->render_buffer.data(), additional_amount);
}

static const char *sdl_channels_to_str(int ch)
{
	switch (ch) {
		case 1: return "mono";
		case 2: return "stereo";
		case 3: return "2.1";
		case 4: return "quad";
		case 5: return "4.1";
		case 6: return "5.1";
		case 7: return "6.1";
		case 8: return "7.1";
		default: break;
	}

	return "?";
}

static const char *sdl_format_to_str(SDL_AudioFormat fmt)
{
	switch (fmt) {
		case SDL_AUDIO_U8: return "U8";
		case SDL_AUDIO_S8: return "S8";
		case SDL_AUDIO_S16: return "S16";
		case SDL_AUDIO_S32: return "S32";
		case SDL_AUDIO_F32: return "F32";
		default: break;
	}

	return "?";
}

static void *alc_load_function(const char *func_name)
{
	void *func = alcGetProcAddress(nullptr, func_name);
	if ( !func ) {
		throw std::runtime_error(func_name);
	}
	return func;
}

static bool openal_init_loopback()
{
	SDL_AudioSpec spec;
	ALCint attrs[10] = {};

	if ( !alcIsExtensionPresent(nullptr, "ALC_SOFT_loopback") ) {
		mprintf(("  ERROR: Loopback extension not present!\n"));
		return false;
	}

	try {
		alcLoopbackOpenDeviceSOFT = reinterpret_cast<ALCLOOPBACKOPENDEVICESOFT>(alc_load_function("alcLoopbackOpenDeviceSOFT"));
		alcIsRenderFormatSupportedSOFT = reinterpret_cast<ALCISRENDERFORMATSUPPORTEDSOFT>(alc_load_function("alcIsRenderFormatSupportedSOFT"));
		alcRenderSamplesSOFT = reinterpret_cast<ALCRENDERSAMPLESSOFT>(alc_load_function("alcRenderSamplesSOFT"));
	} catch (const std::exception& err) {
		mprintf(("  ERROR:  Unable to load function: %s()\n", err.what()));
		return false;
	}

	Audio.stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
											 nullptr, openal_render_samples,
											 &Audio);

	if ( !Audio.stream ) {
		mprintf(("  ERROR: Unable to create audio stream!\n"));
		return false;
	}

	SDL_GetAudioStreamFormat(Audio.stream, &spec, nullptr);

	attrs[0] = ALC_FORMAT_CHANNELS_SOFT;

	if (spec.channels == 1) {
		attrs[1] = ALC_MONO_SOFT;
	} else if (spec.channels == 2) {
		attrs[1] = ALC_STEREO_SOFT;
	} else if (spec.channels == 4) {
		attrs[1] = ALC_QUAD_SOFT;
	} else if (spec.channels == 6) {
		attrs[1] = ALC_5POINT1_SOFT;
	} else if (spec.channels == 7) {
		attrs[1] = ALC_6POINT1_SOFT;
	} else if (spec.channels == 8) {
		attrs[1] = ALC_7POINT1_SOFT;
	} else {
		mprintf(("  ERROR: Unsupported channel setup!\n"));
		return false;
	}

	attrs[2] = ALC_FORMAT_TYPE_SOFT;

	if (spec.format == SDL_AUDIO_U8) {
		attrs[3] = ALC_UNSIGNED_BYTE_SOFT;
	} else if (spec.format == SDL_AUDIO_S8) {
		attrs[3] = ALC_BYTE_SOFT;
	} else if (spec.format == SDL_AUDIO_S16) {
		attrs[3] = ALC_SHORT_SOFT;
	} else if (spec.format == SDL_AUDIO_S32) {
		attrs[3] = ALC_INT_SOFT;
	} else if (spec.format == SDL_AUDIO_F32) {
		attrs[3] = ALC_FLOAT_SOFT;
	} else {
		mprintf(("  ERROR: Unsupported format type!\n"));
		return false;
	}

	attrs[4] = ALC_FREQUENCY;
	attrs[5] = spec.freq;

	attrs[6] = 0;	// end

	Audio.frame_size = spec.channels * SDL_AUDIO_BYTESIZE(spec.format);

	// init loopback device
	Audio.device = alcLoopbackOpenDeviceSOFT(nullptr);

	if ( !Audio.device ) {
		mprintf(("  ERROR: Unable to open loopback device!\n"));
		return false;
	}

	// confirm that format is actually supported
	if (alcIsRenderFormatSupportedSOFT(Audio.device, attrs[5], attrs[1], attrs[3]) == AL_FALSE) {
		mprintf(("  ERROR: Audio render format not supported!\n"));
		return false;
	}

	// TODO: SDL3 => update openal context with new attributes when SDL device
	//               format changes (via alcResetDeviceSOFT())

	Audio.context = alcCreateContext(Audio.device, attrs);

	if ( !Audio.context ) {
		mprintf(("  ERROR: Unable to create OpenAL context!\n"));
		return false;
	}

	alcMakeContextCurrent(Audio.context);

	const auto device_id = SDL_GetAudioStreamDevice(Audio.stream);

	mprintf(("  Audio Device      : Default (%s)\n", SDL_GetAudioDeviceName(device_id)));
	mprintf(("  Audio Format      : %s %s %dHz\n", sdl_format_to_str(spec.format),
			 sdl_channels_to_str(spec.channels), spec.freq));
	mprintf(("  Audio Driver      : %s\n", SDL_GetCurrentAudioDriver()));

	mprintf(("  OpenAL Version    : %s\n", alGetString(AL_VERSION)));

	// start stream
	SDL_ResumeAudioStreamDevice(Audio.stream);

	return true;
}

// initializes hardware device from perferred/default/enumerated list
bool openal_init_device()
{
	SDL_zero(Audio);

	return openal_init_loopback();
}

void openal_close_device()
{
	SDL_PauseAudioStreamDevice(Audio.stream);
	alcMakeContextCurrent(nullptr);

	if (Audio.stream) {
		SDL_DestroyAudioStream(Audio.stream);
		Audio.stream = nullptr;
	}

	if (Audio.context) {
		alcDestroyContext(Audio.context);
		Audio.context = nullptr;
	}

	if (Audio.device) {
		alcCloseDevice(Audio.device);
		Audio.device = nullptr;
	}

	Audio.render_buffer.clear();
	Audio.render_buffer.shrink_to_fit();

	Audio.frame_size = 0;
}

static void get_version_info(OpenALInformation* info) {
	// version check (for 1.0 or 1.1)
	ALCint AL_minor_version = 0;
	ALCint AL_major_version = 0;
	alcGetIntegerv(NULL, ALC_MAJOR_VERSION, sizeof(ALCint), &AL_major_version);
	alcGetIntegerv(NULL, ALC_MINOR_VERSION, sizeof(ALCint), &AL_minor_version);

	info->version_major = static_cast<uint32_t>(AL_major_version);
	info->version_minor = static_cast<uint32_t>(AL_minor_version);
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

	const char *default_device = nullptr;

	if (alcIsExtensionPresent(nullptr, reinterpret_cast<const ALCchar*>("ALC_ENUMERATE_ALL_EXT")) == AL_TRUE) {
		default_device = alcGetString(nullptr, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
	} else {
		default_device = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	}

	if (default_device) {
		info->default_playback_device = default_device;
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
				info->playback_devices.emplace_back(SCP_string(str_list));
				info->efx_support.emplace_back(SCP_string(str_list), device_supports_efx(str_list));

				str_list += (ext_length + 1);
				ext_length = strlen(str_list);
			}
		}
	} else {
		if (default_device) {
			info->playback_devices.emplace_back(SCP_string(default_device));
			info->efx_support.emplace_back(SCP_string(default_device), device_supports_efx(default_device));
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
