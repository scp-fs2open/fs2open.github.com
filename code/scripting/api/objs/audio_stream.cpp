
#include "audio_stream.h"

#include "sound/audiostr.h"

namespace scripting {
namespace api {

//**********HANDLE: Asteroid
ADE_OBJ(l_AudioStream, int, "audio_stream", "An audio stream handle");

ADE_FUNC(play,
	l_AudioStream,
	"[number volume = -1.0 /* By default uses preset volume of the stream type */, boolean loop = false]",
	"Starts playing the audio stream",
	"boolean",
	"true on success, false otherwise")
{
	int streamHandle = -1;
	float volume = -1.0f;
	bool loop = false;
	if (!ade_get_args(L, "o|fb", l_AudioStream.Get(&streamHandle), &volume, &loop)) {
		return ADE_RETURN_FALSE;
	}

	if (streamHandle < 0) {
		return ade_set_args(L, "b", false);
	}

	audiostream_play(streamHandle, volume, loop ? 1 : 0);
	return ade_set_args(L, "b", true);
}

ADE_FUNC(pause, l_AudioStream, nullptr, "Pauses the audio stream", "boolean", "true on success, false otherwise")
{
	int streamHandle = -1;
	if (!ade_get_args(L, "o", l_AudioStream.Get(&streamHandle))) {
		return ADE_RETURN_FALSE;
	}

	if (streamHandle < 0) {
		return ADE_RETURN_FALSE;
	}

	audiostream_pause(streamHandle, true);
	return ade_set_args(L, "b", true);
}

ADE_FUNC(unpause, l_AudioStream, nullptr, "Unpauses the audio stream", "boolean", "true on success, false otherwise")
{
	int streamHandle = -1;
	if (!ade_get_args(L, "o", l_AudioStream.Get(&streamHandle))) {
		return ADE_RETURN_FALSE;
	}

	if (streamHandle < 0) {
		return ADE_RETURN_FALSE;
	}

	audiostream_unpause(streamHandle, true);
	return ade_set_args(L, "b", true);
}

ADE_FUNC(stop, l_AudioStream, nullptr, "Stops the audio stream", "boolean", "true on success, false otherwise")
{
	int streamHandle = -1;
	if (!ade_get_args(L, "o", l_AudioStream.Get(&streamHandle))) {
		return ADE_RETURN_FALSE;
	}

	if (streamHandle < 0) {
		return ADE_RETURN_FALSE;
	}

	audiostream_stop(streamHandle);
	return ade_set_args(L, "b", true);
}

ADE_FUNC(isValid,
	l_AudioStream,
	nullptr,
	"Determines if the handle is valid",
	"boolean",
	"true if valid, false otherwise")
{
	int streamHandle = -1;
	if (!ade_get_args(L, "o", l_AudioStream.Get(&streamHandle))) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", streamHandle >= 0);
}

} // namespace api
} // namespace scripting
