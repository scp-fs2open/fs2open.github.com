
#include "audio_stream.h"

#include "sound/audiostr.h"

namespace scripting {
namespace api {

//**********HANDLE: Asteroid
ADE_OBJ(l_AudioStream, int, "audio_stream", "An audio stream handle");

ADE_FUNC(play,
	l_AudioStream,
   "[number volume = -1.0 /* By default sets the last used volume of this stream, if applicable. Otherwise, uses preset volume of the stream type */, boolean loop = false]",
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

ADE_FUNC(stop,
	l_AudioStream,
	nullptr,
	"Stops the audio stream so that it can be started again later",
	"boolean",
	"true on success, false otherwise")
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

ADE_FUNC(close,
	l_AudioStream,
	"[boolean fade = true]",
	"Irrevocably closes the audio file and optionally fades the music before stopping playback. This invalidates the "
	"audio stream handle.",
	"boolean",
	"true on success, false otherwise")
{
	// We get ourself a pointer here so that we can invalidate the passed handle since otherwise it might be possible to
	// still call functions on that handle
	int* streamHandlePointer = nullptr;
	bool fade = true;
	if (!ade_get_args(L, "o|b", l_AudioStream.GetPtr(&streamHandlePointer), &fade)) {
		return ADE_RETURN_FALSE;
	}

	if (streamHandlePointer == nullptr) {
		return ADE_RETURN_FALSE;
	}

	if (*streamHandlePointer < 0) {
		return ADE_RETURN_FALSE;
	}

	audiostream_close_file(*streamHandlePointer, fade);

	// Invalidate the handle so that it cannot be reused
	*streamHandlePointer = -1;

	return ade_set_args(L, "b", true);
}

ADE_FUNC(isPlaying,
	l_AudioStream,
	nullptr,
	"Determines if the audio stream is still playing",
	"boolean",
	"true when still playing, false otherwise")
{
	int streamHandle = -1;
	if (!ade_get_args(L, "o", l_AudioStream.Get(&streamHandle))) {
		return ADE_RETURN_FALSE;
	}

	if (streamHandle < 0) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", audiostream_is_playing(streamHandle) != 0);
}

ADE_FUNC(setVolume,	l_AudioStream, "number volume", "Sets the volume of the audio stream, 0 - 1", "boolean", "true on success, false otherwise")
{
	int streamHandle = -1;
	float volume = -1.0f;
	if (!ade_get_args(L, "of", l_AudioStream.Get(&streamHandle), &volume)) {
		return ADE_RETURN_FALSE;
	}

	if (streamHandle < 0) {
		return ADE_RETURN_FALSE;
	}
	audiostream_set_volume(streamHandle, volume);
	return ADE_RETURN_TRUE;
}

ADE_FUNC(getDuration, l_AudioStream, nullptr, "Gets the duration of the stream", "number", "the duration in milliseconds or nil if invalid")
{
	int streamHandle = -1;
	if (!ade_get_args(L, "o", l_AudioStream.Get(&streamHandle))) {
		return ADE_RETURN_NIL;
	}

	if (streamHandle < 0) {
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "i", audiostream_get_duration(streamHandle));
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
