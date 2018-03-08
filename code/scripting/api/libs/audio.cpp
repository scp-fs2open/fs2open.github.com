//
//

#include "audio.h"

#include "scripting/api/objs/sound.h"
#include "scripting/api/objs/vecmath.h"

#include "gamesnd/gamesnd.h"
#include "missionui/missionbrief.h"
#include "menuui/mainhallmenu.h"
#include "menuui/credits.h"
#include "render/3d.h"

extern float Master_event_music_volume;

namespace scripting {
namespace api {

//**********LIBRARY: Audio
ADE_LIB(l_Audio, "Audio", "ad", "Sound/Music Library");

ADE_FUNC(getSoundentry, l_Audio, "string/number", "Return a sound entry matching the specified index or name. If you are using a number then the first valid index is 1", "soundentry", "soundentry or invalid handle on error")
{
	int index = -1;

	if (lua_isnumber(L, 1))
	{
		int idx = -1;
		if(!ade_get_args(L, "i", &idx))
			return ade_set_error(L, "o", l_SoundEntry.Set(sound_entry_h()));

		index = gamesnd_get_by_tbl_index(idx);
	}
	else
	{
		char *s = NULL;
		if(!ade_get_args(L, "s", &s))
			return ade_set_error(L, "o", l_SoundEntry.Set(sound_entry_h()));

		if (s == NULL)
			return ade_set_error(L, "o", l_SoundEntry.Set(sound_entry_h()));

		index = gamesnd_get_by_name(s);
	}

	if (index < 0)
	{
		return ade_set_args(L, "o", l_SoundEntry.Set(sound_entry_h()));
	}
	else
	{
		return ade_set_args(L, "o", l_SoundEntry.Set(sound_entry_h(index)));
	}
}

ADE_FUNC(loadSoundfile, l_Audio, "string filename", "Loads the specified sound file", "soundfile", "A soundfile handle")
{
	char* fileName = NULL;

	if (!ade_get_args(L, "s", &fileName))
		return ade_set_error(L, "o", l_Soundfile.Set(-1));

	game_snd_entry tmp_gs;
	strcpy_s( tmp_gs.filename, fileName );
	int n = snd_load( &tmp_gs, 0, 0 );

	return ade_set_error(L, "o", l_Soundfile.Set(n));
}

ADE_FUNC(playSound, l_Audio, "soundentry", "Plays the specified sound entry handle", "sound", "A handle to the playing sound")
{
	sound_entry_h *seh = NULL;

	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	int handle = snd_play(seh->Get());

	if (handle < 0)
	{
		return ade_set_args(L, "o", l_Sound.Set(sound_h()));
	}
	else
	{
		return ade_set_args(L, "o", l_Sound.Set(sound_h(seh->idx, handle)));
	}
}

ADE_FUNC(playLoopingSound, l_Audio, "soundentry", "Plays the specified sound as a looping sound", "sound", "A handle to the playing sound or invalid handle if playback failed")
{
	sound_entry_h *seh = NULL;

	if (!ade_get_args(L, "o", l_SoundEntry.GetPtr(&seh)))
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "o", l_Sound.Set(sound_h()));

	int handle = snd_play_looping(seh->Get());

	if (handle < 0)
	{
		return ade_set_args(L, "o", l_Sound.Set(sound_h()));
	}
	else
	{
		return ade_set_args(L, "o", l_Sound.Set(sound_h(seh->idx, handle)));
	}
}

ADE_FUNC(play3DSound, l_Audio, "soundentry[, vector source[, vector listener]]", "Plays the specified sound entry handle. Source if by default 0, 0, 0 and listener is by default the current viewposition", "3Dsound", "A handle to the playing sound")
{
	sound_entry_h *seh = NULL;
	vec3d *source = &vmd_zero_vector;
	vec3d *listener = &View_position;

	if (!ade_get_args(L, "o|oo", l_SoundEntry.GetPtr(&seh), l_Vector.GetPtr(&source), l_Vector.GetPtr(&listener)))
		return ade_set_error(L, "o", l_Sound3D.Set(sound_h()));

	if (seh == NULL || !seh->IsValid())
		return ade_set_error(L, "o", l_Sound3D.Set(sound_h()));

	int handle = snd_play_3d(seh->Get(), source, listener);

	if (handle < 0)
	{
		return ade_set_args(L, "o", l_Sound3D.Set(sound_h()));
	}
	else
	{
		return ade_set_args(L, "o", l_Sound3D.Set(sound_h(seh->idx, handle)));
	}
}

ADE_FUNC(playGameSound, l_Audio, "Sound index, [Panning (-1.0 left to 1.0 right), Volume %, Priority 0-3, Voice Message?]", "Plays a sound from #Game Sounds in sounds.tbl. A priority of 0 indicates that the song must play; 1-3 will specify the maximum number of that sound that can be played", "boolean", "True if sound was played, false if not (Replaced with a sound instance object in the future)")
{
	int idx, gamesnd_idx;
	float pan=0.0f;
	float vol=100.0f;
	int pri=0;
	bool voice_msg = false;
	if(!ade_get_args(L, "i|ffib", &idx, &pan, &vol, &pri, &voice_msg))
		return ADE_RETURN_NIL;

	if(idx < 0)
		return ADE_RETURN_FALSE;

	if(pri < 0 || pri > 3)
		pri = 0;

	CLAMP(pan, -1.0f, 1.0f);
	CLAMP(vol, 0.0f, 100.0f);

	gamesnd_idx = gamesnd_get_by_tbl_index(idx);

	if (gamesnd_idx >= 0) {
		int sound_handle = snd_play(gamesnd_get_game_sound(gamesnd_idx), pan, vol*0.01f, pri, voice_msg);
		return ade_set_args(L, "b", sound_handle >= 0);
	} else {
		LuaError(L, "Invalid sound index %i (Snds[%i]) in playGameSound()", idx, gamesnd_idx);
		return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(playInterfaceSound, l_Audio, "Sound index", "Plays a sound from #Interface Sounds in sounds.tbl", "boolean", "True if sound was played, false if not")
{
	int idx, gamesnd_idx;
	if(!ade_get_args(L, "i", &idx))
		return ade_set_error(L, "b", false);

	gamesnd_idx = gamesnd_get_by_iface_tbl_index(idx);

	if (gamesnd_idx >= 0) {
		gamesnd_play_iface(gamesnd_idx);
		return ade_set_args(L, "b", true);
	} else {
		LuaError(L, "Invalid sound index %i (Snds[%i]) in playInterfaceSound()", idx, gamesnd_idx);
		return ADE_RETURN_FALSE;
	}
}

ADE_FUNC(playMusic, l_Audio, "string Filename, [float volume = 1.0, bool looping = true]", "Plays a music file using FS2Open's builtin music system. Volume is currently ignored, uses players music volume setting. Files passed to this function are looped by default.", "number", "Audiohandle of the created audiostream, or -1 on failure")
{
	char *s;
	float volume = 1.0f;
	bool loop = true;
	if (!ade_get_args(L, "s|fb", &s, &volume, &loop))
		return ade_set_error(L, "i", -1);

	int ah = audiostream_open(s, ASF_MENUMUSIC);
	if(ah < 0)
		return ade_set_error(L, "i", -1);

	// didn't remove the volume parameter because it'll break the API
	volume = Master_event_music_volume;

	audiostream_play(ah, volume, loop ? 1 : 0);
	return ade_set_args(L, "i", ah);
}

ADE_FUNC(stopMusic, l_Audio, "int audiohandle, [bool fade = false], [string 'briefing|credits|mainhall']", "Stops a playing music file, provided audiohandle is valid. If the 3rd arg is set to one of briefing,credits,mainhall then that music will be stopped despite the audiohandle given.", NULL, NULL)
{
	int ah;
	bool fade = false;
	char *music_type = NULL;

	if(!ade_get_args(L, "i|bs", &ah, &fade, &music_type))
		return ADE_RETURN_NIL;

	if (ah >= MAX_AUDIO_STREAMS || ah < 0 )
		return ADE_RETURN_NIL;

	if (music_type == NULL) {
		audiostream_close_file(ah, fade);
	} else {
		if (!stricmp(music_type, "briefing"))	{
			briefing_stop_music(fade);
		} else if (!stricmp(music_type, "credits")) {
			credits_stop_music(fade);
		} else if (!stricmp(music_type, "mainhall")) {
			main_hall_stop_music(fade);
		} else {
			LuaError(L, "Invalid music type (%s) passed to stopMusic", music_type);
		}
	}

	return ADE_RETURN_NIL;
}

ADE_FUNC(pauseMusic, l_Audio, "int audiohandle, bool pause", "Pauses or unpauses a playing music file, provided audiohandle is valid. The boolean argument should be true to pause and false to unpause. If the audiohandle is -1, *all* audio streams are paused or unpaused.", NULL, NULL)
{
	int ah;
	bool pause;

	if(!ade_get_args(L, "ib", &ah, &pause))
		return ADE_RETURN_NIL;

	if (ah >= 0 && ah < MAX_AUDIO_STREAMS)
	{
		if (pause)
			audiostream_pause(ah, true);
		else
			audiostream_unpause(ah, true);
	}
	else if (ah == -1)
	{
		if (pause)
			audiostream_pause_all(true);
		else
			audiostream_unpause_all(true);
	}

	return ADE_RETURN_NIL;
}


}
}
