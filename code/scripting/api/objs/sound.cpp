//
//

#include "sound.h"
#include "vecmath.h"

#include "gamesnd/gamesnd.h"

namespace scripting {
namespace api {

sound_entry_h::sound_entry_h() {
}
sound_entry_h::sound_entry_h(gamesnd_id n_idx) {
	idx = n_idx;
}
game_snd* sound_entry_h::Get() {
	if (!this->IsValid())
		return NULL;

	return gamesnd_get_game_sound(idx);
}
bool sound_entry_h::IsValid() {
	return gamesnd_game_sound_valid(idx);
}

//**********HANDLE: SoundEntry
ADE_OBJ(l_SoundEntry, sound_entry_h, "soundentry", "sounds.tbl table entry handle");

ADE_VIRTVAR(DefaultVolume, l_SoundEntry, "number", "The default volume of this game sound. If the sound entry has a volume range then the maximum volume will be returned by this.", "number", "Volume in the range from 1 to 0 or -1 on error")
{
	sound_entry_h *seh = NULL;
	float newVal = -1.0f;

	if (!ade_get_args(L, "o|f", l_SoundEntry.GetPtr(&seh), &newVal))
		return ade_set_error(L, "f", -1.0f);

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	if (ADE_SETTING_VAR)
	{
		if (seh->Get() != NULL)
		{
			CAP(newVal, 0.0f, 1.0f);

			seh->Get()->volume_range = ::util::UniformFloatRange(newVal);
		}
	}

	return ade_set_args(L, "f", seh->Get()->volume_range.max());
}

ADE_FUNC(getFilename, l_SoundEntry, NULL, "Returns the filename of this sound. If the sound has multiple entries then the filename of the first entry will be returned.", "string", "filename or empty string on error")
{
	sound_entry_h *seh = NULL;

	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ade_set_error(L, "s", "");

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "s", "");

	Assertion(!seh->Get()->sound_entries.empty(),
			  "Sound entry vector of sound %s is empty! This should not happen. Get a coder!",
			  seh->Get()->name.c_str());

	return ade_set_args(L, "s", seh->Get()->sound_entries[0].filename);
}

ADE_FUNC(getDuration, l_SoundEntry, NULL, "Gives the length of the sound in seconds. If the sound has multiple entries or a pitch range then the maximum duration of the sound will be returned.", "number", "the length, or -1 on error")
{
	sound_entry_h *seh = NULL;

	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ade_set_error(L, "f", -1.0f);

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "f", -1.0f);

	return ade_set_args(L, "f", (i2fl(gamesnd_get_max_duration(seh->Get())) / 1000.0f));
}

ADE_FUNC(get3DValues,
	l_SoundEntry,
	"vector Postion, [number radius=0.0]",
	"Computes the volume and the panning of the sound when it would be played from the specified position.<br>"
	"If range is given then the volume will diminish when the listener is withing that distance to the source.<br>"
	"The position of the listener is always the the current viewing position.",
	"number, number",
	"The volume and the panning, in that sequence, or both -1 on error")
{
	sound_entry_h *seh = NULL;
	vec3d *sourcePos = NULL;
	float radius = 0.0f;

	float vol = 0.0f;
	float pan = 0.0f;

	if (!ade_get_args(L, "oo|f", l_SoundEntry.GetPtr(&seh), l_Vector.GetPtr(&sourcePos), &radius))
		return ade_set_error(L, "ff", -1.0f, -1.0f);

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "ff", -1.0f, -1.0f);

	int result = snd_get_3d_vol_and_pan(seh->Get(), sourcePos, &vol, &pan, radius);

	if (result < 0)
	{
		return ade_set_args(L, "ff", -1.0f, -1.0f);
	}
	else
	{
		return ade_set_args(L, "ff", vol, pan);
	}
}

ADE_FUNC(isValid, l_SoundEntry, nullptr, "Detects whether handle is valid", "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	sound_entry_h *seh;
	if(!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", seh->IsValid());
}

ADE_FUNC(exists, l_SoundEntry, nullptr, "Detects whether handle references a sound that can be loaded", "boolean", "true if exists, false if not, nil if a syntax/type error occurs")
{
	sound_entry_h *seh;
	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", gamesnd_game_sound_exists(seh->idx));
}

sound_h::sound_h() : sound_entry_h() { sig = sound_handle::invalid(); }
sound_h::sound_h(gamesnd_id n_gs_idx, sound_handle n_sig) : sound_entry_h(n_gs_idx) { sig = n_sig; }
sound_handle sound_h::getSignature()
{
	if (!IsValid())
		return sound_handle::invalid();

	return sig;
}
bool sound_h::IsSoundValid() {
	if (!sig.isValid() || ds_get_channel(sig) < 0)
		return false;

	return true;
}
bool sound_h::IsValid() {
	if(!sound_entry_h::IsValid())
		return false;

	if (!sig.isValid() || ds_get_channel(sig) < 0)
		return false;

	return true;
}

//**********HANDLE: Sound
ADE_OBJ(l_Sound, sound_h, "sound", "sound instance handle");

ADE_VIRTVAR(Pitch, l_Sound, "number", "Pitch of sound, from 100 to 100000", "number", "Pitch, or 0 if handle is invalid")
{
	sound_h *sh;
	int newpitch = 100;
	if(!ade_get_args(L, "o|i", l_Sound.GetPtr(&sh), &newpitch))
		return ade_set_error(L, "f", 0.0f);

	if (!sh->IsSoundValid())
		return ade_set_error(L, "f", 0.0f);

	if(ADE_SETTING_VAR)
	{
		if(newpitch < 100)
			newpitch = 100;
		if(newpitch > 100000)
			newpitch = 100000;

		snd_set_pitch(sh->sig, log10f(i2fl(newpitch)) - 2.0f);
	}

	return ade_set_args(L, "f", (float) pow(10.0, snd_get_pitch(sh->sig) + 2.0));
}

ADE_FUNC(getRemainingTime, l_Sound, NULL, "The remaining time of this sound handle", "number", "Remaining time, or -1 on error")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ade_set_error(L, "f", -1.0f);

	if (!sh->IsSoundValid())
		return ade_set_error(L, "f", -1.0f);

	int remaining = snd_time_remaining(sh->getSignature());

	return ade_set_args(L, "f", i2fl(remaining) / 1000.0f);
}

ADE_FUNC(setVolume, l_Sound, "number", "Sets the volume of this sound instance", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	float newVol = -1.0f;
	if(!ade_get_args(L, "of", l_Sound.GetPtr(&sh), &newVol))
		return ADE_RETURN_FALSE;

	if (!sh->IsSoundValid())
		return ADE_RETURN_FALSE;

	CAP(newVol, 0.0f, 1.0f);

	snd_set_volume(sh->getSignature(), newVol);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(setPanning, l_Sound, "number", "Sets the panning of this sound. Argument ranges from -1 for left to 1 for right", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	float newPan = -1.0f;
	if(!ade_get_args(L, "of", l_Sound.GetPtr(&sh), &newPan))
		return ADE_RETURN_FALSE;

	if (!sh->IsSoundValid())
		return ADE_RETURN_FALSE;

	CAP(newPan, -1.0f, 1.0f);

	snd_set_pan(sh->getSignature(), newPan);

	return ADE_RETURN_TRUE;
}


ADE_FUNC(setPosition, l_Sound, "number value, boolean percent = true",
		 "Sets the absolute position of the sound. If boolean argument is true then the value is given as a percentage<br>"
			 "This operation fails if there is no backing soundentry!",
		 "boolean", "true if successfull, false otherwise")
{
	sound_h *sh;
	float val = -1.0f;
	bool percent = true;
	if(!ade_get_args(L, "of|b", l_Sound.GetPtr(&sh), &val, &percent))
		return ADE_RETURN_FALSE;

	if (!sh->IsValid())
		return ADE_RETURN_FALSE;

	if (val <= 0.0f)
		return ADE_RETURN_FALSE;

	snd_set_pos(sh->getSignature(), val, percent);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(rewind, l_Sound, "number", "Rewinds the sound by the given number of seconds<br>"
	"This operation fails if there is no backing soundentry!", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	float val = -1.0f;
	if(!ade_get_args(L, "of", l_Sound.GetPtr(&sh), &val))
		return ADE_RETURN_FALSE;

	if (!sh->IsValid())
		return ADE_RETURN_FALSE;

	if (val <= 0.0f)
		return ADE_RETURN_FALSE;

	snd_rewind(sh->getSignature(), val);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(skip, l_Sound, "number", "Skips the given number of seconds of the sound<br>"
	"This operation fails if there is no backing soundentry!", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	float val = -1.0f;
	if(!ade_get_args(L, "of", l_Sound.GetPtr(&sh), &val))
		return ADE_RETURN_FALSE;

	if (!sh->IsValid())
		return ADE_RETURN_FALSE;

	if (val <= 0.0f)
		return ADE_RETURN_FALSE;

	snd_ffwd(sh->getSignature(), val);

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isPlaying, l_Sound, NULL, "Specifies if this handle is currently playing", "boolean", "true if playing, false if otherwise")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ade_set_error(L, "b", false);

	if (!sh->IsSoundValid())
		return ade_set_error(L, "b", false);

	return ade_set_args(L, "b", snd_is_playing(sh->getSignature()) == 1);
}

ADE_FUNC(stop, l_Sound, NULL, "Stops the sound of this handle", "boolean", "true if succeeded, false otherwise")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ade_set_error(L, "b", false);

	if (!sh->IsSoundValid())
		return ade_set_error(L, "b", false);

	snd_stop(sh->getSignature());

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isValid, l_Sound, nullptr,
         "Detects whether the whole handle is valid.<br>"
         "<b>Warning:</b> This does not work for sounds started by a "
         "directly loaded sound file! Use isSoundValid() in that case instead.",
         "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sh->IsValid());
}

ADE_FUNC(isSoundValid, l_Sound, NULL, "Checks if only the sound is valid, should be used for non soundentry sounds",
		 "boolean", "true if valid, false if handle is invalid, nil if a syntax/type error occurs")
{
	sound_h *sh;
	if(!ade_get_args(L, "o", l_Sound.GetPtr(&sh)))
		return ADE_RETURN_NIL;

	return ade_set_args(L, "b", sh->IsSoundValid());
}

ADE_OBJ_DERIV(l_Sound3D, sound_h, "sound3D", "3D sound instance handle", l_Sound);

ADE_FUNC(updatePosition, l_Sound3D, "vector Position, [number radius = 0.0]", "Updates the given 3D sound with the specified position and an optional range value", "boolean", "true if succeesed, false otherwise")
{
	sound_h *sh;
	vec3d *newPos = NULL;
	float radius = 0.0f;

	if(!ade_get_args(L, "oo|f", l_Sound.GetPtr(&sh), l_Vector.GetPtr(&newPos), &radius))
		return ade_set_error(L, "b", false);

	if (!sh->IsValid() || newPos == NULL)
		return ade_set_error(L, "b", false);

	snd_update_3d_pos(sh->getSignature(), sh->Get(), newPos, radius);

	return ADE_RETURN_TRUE;
}


//**********HANDLE: Soundfile
soundfile_h::soundfile_h(sound_load_id id) : idx(id) {}

ADE_OBJ(l_Soundfile, soundfile_h, "soundfile", "Handle to a sound file");

ADE_VIRTVAR(Duration, l_Soundfile, "number", "The duration of the sound file, in seconds", "number", "The duration or -1 on error")
{
	soundfile_h* handle = nullptr;

	if (!ade_get_args(L, "o", l_Soundfile.GetPtr(&handle)))
		return ade_set_error(L, "f", -1.0f);

	if (!handle->idx.isValid())
		return ade_set_error(L, "f", -1.0f);

	int duration = snd_get_duration(handle->idx);

	return ade_set_args(L, "f", i2fl(duration) / 1000.0f);
}

ADE_VIRTVAR(Filename, l_Soundfile, "string", "The filename of the file", "string", "The file name or empty string on error")
{
	soundfile_h* handle = nullptr;

	if (!ade_get_args(L, "o", l_Soundfile.GetPtr(&handle)))
		return ade_set_error(L, "s", "");

	if (!handle->idx.isValid())
		return ade_set_error(L, "s", "");

	const char* filename = snd_get_filename(handle->idx);

	return ade_set_args(L, "s", filename);
}


ADE_FUNC(play, l_Soundfile, "[number volume = 1.0, number panning = 0.0]", "Plays the sound", "sound", "A sound handle or invalid handle on error")
{
	soundfile_h* handle = nullptr;
	float volume = 1.0f;
	float panning = 0.0f;

	if (!ade_get_args(L, "o|ff", l_Soundfile.GetPtr(&handle), &volume, &panning))
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	if (!handle->idx.isValid())
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	if (volume < 0.0f)
	{
		LuaError(L, "Invalid volume value of %f specified!", volume);
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));
	}

	auto snd_handle = snd_play_raw(handle->idx, panning, volume);

	return ade_set_args(L, "o", l_Sound.Set(sound_h(gamesnd_id(), snd_handle)));
}

ADE_FUNC(unload, l_Soundfile, nullptr,
         "Unloads the audio data loaded for this sound file. This invalidates the handle on which this is called!",
         "boolean", "true if successful, false otherwise")
{
	soundfile_h* handle = nullptr;

	if (!ade_get_args(L, "o", l_Soundfile.GetPtr(&handle)))
		return ade_set_error(L, "b", false);

	if (!handle->idx.isValid())
		return ade_set_error(L, "b", false);

	auto result = snd_unload(handle->idx);

	if (result != 0) {
		// Invalidate this handle so that the script cannot do something bad with it
		handle->idx = sound_load_id::invalid();
	}

	return ade_set_args(L, "b", result != 0);
}

ADE_FUNC(isValid, l_Soundfile, NULL, "Checks if the soundfile handle is valid", "boolean", "true if valid, false otherwise")
{
	soundfile_h* handle = nullptr;

	if (!ade_get_args(L, "o", l_Soundfile.GetPtr(&handle)))
		return ADE_RETURN_FALSE;

	if (handle == nullptr) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", handle->idx.isValid());
}


}
}
