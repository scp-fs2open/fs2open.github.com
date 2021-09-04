/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "gamesnd/eventmusic.h"
#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "menuui/mainhallmenu.h"
#include "mod_table/mod_table.h"
#include "options/Option.h"
#include "osapi/osapi.h"
#include "render/3d.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "sound/ds3d.h"
#include "sound/dscap.h"
#include "tracing/Monitor.h"
#include "tracing/tracing.h"

#ifdef WITH_FFMPEG
#include "sound/ffmpeg/FFmpegWaveFile.h"
#endif

#include <climits>

const unsigned int SND_ENHANCED_MAX_LIMIT = 15; // seems like a good max limit

#define SND_F_USED			(1<<0)		// Sounds[] element is used

struct loaded_sound {
	int sid; // software id
	char filename[MAX_FILENAME_LEN];
	int sig;
	int flags;
	sound_info info;
	int uncompressed_size; // size (in bytes) of sound (uncompressed)
	int duration;
};

SCP_vector<loaded_sound> Sounds;

int Sound_enabled = FALSE;				// global flag to turn sound on/off
size_t Snd_sram;								// mem (in bytes) used up by storing sounds in system memory

float Default_sound_volume = 1.0f;		// range is 0 -> 1, used for non-music sound fx
float Master_sound_volume = Default_sound_volume;

static bool effects_volume_change_listener(float new_val, bool /*initial*/)
{
	Assertion(new_val >= 0.0f && new_val <= 1.0f, "Invalid value %f supplied by options system!", new_val);

	snd_set_effects_volume(new_val);

	return true;
}

static auto EffectVolumeOption =
    options::OptionBuilder<float>("Audio.Effects", "Effects", "Volume used for playing in-game effects")
        .category("Audio")
        .default_val(Default_sound_volume)
        .range(0.0f, 1.0f)
        .change_listener(effects_volume_change_listener)
        .importance(2)
        .finish();

float Default_voice_volume = 0.7f; // range is 0 -> 1, used for all voice playback
float Master_voice_volume = Default_voice_volume;

static bool voice_volume_change_listener(float new_val, bool /*initial*/)
{
	Assertion(new_val >= 0.0f && new_val <= 1.0f, "Invalid value %f supplied by options system!", new_val);

	snd_set_voice_volume(new_val);

	return true;
}

static auto VoiceVolumeOption =
    options::OptionBuilder<float>("Audio.Voice", "Voice", "Volume used for playing voice audio")
        .category("Audio")
        .default_val(Default_voice_volume)
        .range(0.0f, 1.0f)
        .change_listener(voice_volume_change_listener)
        .importance(0)
        .finish();

unsigned int SND_ENV_DEFAULT = 0;

struct LoopingSoundInfo {
	sound_handle m_dsHandle;
	float m_defaultVolume;	//!< The default volume of this sound (from game_snd)
	float m_dynamicVolume;	//!< The dynamic volume before scripted volume adjustment is applied (is updated via snd_set_volume)

	LoopingSoundInfo(sound_handle dsHandle, float defaultVolume, float dynamicVolume)
	    : m_dsHandle(dsHandle), m_defaultVolume(defaultVolume), m_dynamicVolume(dynamicVolume)
	{
	}
    
    LoopingSoundInfo() : m_dsHandle(-1), m_defaultVolume(0.0f), m_dynamicVolume(0.0f)
    {
    }
};

SCP_list<LoopingSoundInfo> currentlyLoopingSoundInfos;

//For the adjust-audio-volume sexp
float aav_voice_volume = 1.0f;
float aav_music_volume = 1.0f;
float aav_effect_volume = 1.0f;

struct aav {
	float start_volume;
	float delta;
	float start_time;
	int delta_time;
};

aav aav_data[3];

// min volume to play a sound after all volume processing (range is 0.0 -> 1.0)
#define	MIN_SOUND_VOLUME				0.05f

static int snd_next_sig	= 1;

// convert the game level sound priorities to the DirectSound priority descriptions
int ds_priority(int priority)
{
	switch(priority){
		case SND_PRIORITY_MUST_PLAY:
			return DS_MUST_PLAY;
		case SND_PRIORITY_SINGLE_INSTANCE:
			return DS_LIMIT_ONE;
		case SND_PRIORITY_DOUBLE_INSTANCE:
			return DS_LIMIT_TWO;
		case SND_PRIORITY_TRIPLE_INSTANCE:
			return DS_LIMIT_THREE;
		default:
			Int3();
			return DS_MUST_PLAY;
	};
}

void snd_clear()
{
	Sounds.clear();

	// reset how much storage sounds are taking up in memory
	Snd_sram = 0;
}

// ---------------------------------------------------------------------------------------
// Initialize the game sound system
// 
// Initialize the game sound system.  Depending on what sound library is being used,
// call the appropriate low-level initiailizations
//
// returns:     1		=> init success
//              0		=> init failed
//
int snd_init()
{
	int rval;

	if ( Cmdline_freespace_no_sound )
		return 0;

	if (ds_initialized)	{
		nprintf(( "Sound", "SOUND => Audio is already initialized!\n" ));
		return 1;
	}

	snd_clear();

	rval = ds_init();

	if ( rval != 0 ) {
		nprintf(( "Sound", "SOUND ==> Fatal error initializing audio device, turn sound off.\n" ));
		Cmdline_freespace_no_sound = Cmdline_freespace_no_music = 1;
		goto Failure;
	}
	
	snd_aav_init();

	// Init the audio streaming stuff
	audiostream_init();
			
	ds_initialized = 1;
	Sound_enabled = TRUE;
	return 1;

Failure:
//	Warning(LOCATION, "Sound system was unable to be initialized.  If you continue, sound will be disabled.\n");
	nprintf(( "Sound", "SOUND => Audio init unsuccessful, continuing without sound.\n" ));
	return 0;
}

int Sound_spew = 0;
DCF(show_sounds, "Toggles display of sound debug info")
{
	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Sound debug info is %s", (Sound_spew ? "ON" : "OFF"));
		return;
	}

	Sound_spew = !Sound_spew;
	dc_printf("Sound debug info is %s", (Sound_spew ? "ON" : "OFF"));
}

extern SCP_vector<game_snd>	Snds;
extern SCP_vector<game_snd>	Snds_iface;

void snd_spew_debug_info()
{
	int game_sounds = 0;
	int message_sounds = 0;
	int interface_sounds = 0;
	int done = 0;

	if(!Sound_spew){
		return;
	}

	// count up game, interface and message sounds
	for (size_t idx = 0; idx < Sounds.size(); idx++) {
		if(!(Sounds[idx].flags & SND_F_USED)){
			continue;
		}

		done = 0;

		// what kind of sound is this
		for(SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); ++gs){
			for (auto& entry : gs->sound_entries) {
				if(!stricmp(entry.filename, Sounds[idx].filename)){
					game_sounds++;
					done = 1;
				}
			}
		}

		if(!done){
			for(SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); ++gs){
				for (auto& entry : gs->sound_entries) {
					if(!stricmp(entry.filename, Sounds[idx].filename)) {
						interface_sounds++;
						done = 1;
					}
				}
			}
		}

		if(!done){
			message_sounds++;
		}		
	}

	// spew info
	int line_height = gr_get_font_height() + 1;
	int sx = gr_screen.center_offset_x + 30;
	int sy = gr_screen.center_offset_y + 100;
	gr_set_color_fast(&Color_normal);
	gr_printf_no_resize(sx, sy, "Game sounds : %d\n", game_sounds);
	sy += line_height;
	gr_printf_no_resize(sx, sy, "Interface sounds : %d\n", interface_sounds);
	sy += line_height;
	gr_printf_no_resize(sx, sy, "Message sounds : %d\n", message_sounds);
	sy += line_height;
	gr_printf_no_resize(sx, sy, "Total sounds : %d\n", game_sounds + interface_sounds + message_sounds);
}

static std::unique_ptr<sound::IAudioFile> openAudioFile(const char* fileName)
{
#ifdef WITH_FFMPEG
	{
		std::unique_ptr<sound::IAudioFile> audio_file(new sound::ffmpeg::FFmpegWaveFile());

		if (audio_file->Open(fileName, false)) {
			return audio_file;
		}
	}
#endif

	return nullptr;
}

// ---------------------------------------------------------------------------------------
// snd_load() 
//
// Load a sound into memory and prepare it for playback.  The sound will reside in memory as
// a single instance, and can be played multiple times simultaneously.  Through the magic of
// DirectSound, only 1 copy of the sound is used.
//
// parameters:		gs							=> file of sound to load
//						allow_hardware_load	=> whether to try to allocate in hardware
//
// returns:			success => index of sound in Sounds[] array
//						failure => -1
//
//int snd_load( char *filename, int hardware, int use_ds3d, int *sig)
sound_load_id snd_load(game_snd_entry* entry, int flags, int /*allow_hardware_load*/)
{
	int type;
	sound_info* si;
	loaded_sound* snd;
	size_t n;

	if (!ds_initialized)
		return sound_load_id::invalid();

	if (!VALID_FNAME(entry->filename))
		return sound_load_id::invalid();

	for (n = 0; n < Sounds.size(); n++) {
		if ( !(Sounds[n].flags & SND_F_USED) ) {
			break;
		} else if ( !stricmp( Sounds[n].filename, entry->filename) ) {
			// extra check: make sure the sound is actually loaded in a compatible way (2D vs. 3D)
			//
			// NOTE: this will allow a duplicate 3D entry if 2D stereo entry exists,
			//       but will not load a duplicate 2D entry to get stereo if 3D
			//       version already loaded
			if ( (Sounds[n].info.n_channels == 1) || !(flags & GAME_SND_USE_DS3D) ) {
				return sound_load_id(static_cast<int>(n));
			}
		}
	}

	if ( n == Sounds.size() ) {
		loaded_sound new_sound;
		new_sound.sid   = -1;
		new_sound.flags = 0;

		Sounds.push_back(new_sound);
	}

	snd = &Sounds[n];

	si = &snd->info;

	TRACE_SCOPE(tracing::LoadSound);

	nprintf(("Sound", "SOUND ==> Loading '%s'\n", entry->filename));

	std::unique_ptr<sound::IAudioFile> audio_file = openAudioFile(entry->filename);

	if (audio_file == nullptr) {
		return sound_load_id::invalid();
	}

	auto fileProps = audio_file->getFileProperties();

	type = 0;
	if (flags & GAME_SND_USE_DS3D) {
		type |= DS_3D;

		if (fileProps.num_channels > 1) {
			// We need to resample the audio down to one channel
			sound::ResampleProperties resample;
			resample.num_channels = 1;

			audio_file->setResamplingProperties(resample);
			fileProps = audio_file->getFileProperties(); // Refresh properties so that we have accurate information

#ifndef NDEBUG
			// Retail has a few sounds that triggers this warning so we need to ignore those
			const char* warning_ignore_list[] = {
				"l_hit.wav",
				"m_hit.wav",
				"s_hit_2.wav",
				"Pirate.wav",
			};

			bool show_warning = true;
			for (auto& name : warning_ignore_list) {
				if (!stricmp(name, entry->filename)) {
					show_warning = false;
					break;
				}
			}

			if (show_warning) {
				if (mod_supports_version(3, 8, 0)) {
					// This warning was introduced in 3.8.0 and caused a few issues since a lot of mods use 3D sounds
					// with more than one channel. This will silence the warnings for any mod that does not support
					// 3.8.0.
					Warning(LOCATION,
							"Sound '%s' has more than one channel but is used as a 3D sound! 3D sounds may only have "
							"one channel.",
							entry->filename);
				} else {
					mprintf(("Warning: Sound '%s' has more than one channel but is used as a 3D sound! 3D sounds may "
							 "only have one channel.\n",
							 entry->filename));
				}
			}
#endif
		}
	}

	// Load was a success
	si->n_channels        = fileProps.num_channels; // 16-bit channel count (nChannels)
	si->sample_rate       = fileProps.sample_rate;  // 32-bit sample rate (nSamplesPerSec)
	si->avg_bytes_per_sec = fileProps.sample_rate * fileProps.bytes_per_sample *
							fileProps.num_channels; // 32-bit average bytes per second (nAvgBytesPerSec)
	si->bits = fileProps.bytes_per_sample * 8;      // Read 16-bit bits per sample
	si->size = fileProps.total_samples * fileProps.bytes_per_sample * fileProps.num_channels;

	snd->uncompressed_size = si->size;

	auto rc = ds_load_buffer(&snd->sid, type, audio_file.get());
	if (rc == -1) {
		nprintf(("Sound", "SOUND ==> Failed to load '%s'\n", entry->filename));
		return sound_load_id::invalid();
	}

	// NOTE: "si" values can change once loaded in the buffer
	snd->duration = fl2i(1000.0f * fileProps.duration);

	strcpy_s( snd->filename, entry->filename );
	snd->flags = SND_F_USED;

	snd->sig = snd_next_sig++;
	if (snd_next_sig < 0 ) snd_next_sig = 1;
	entry->id_sig = snd->sig;
	entry->id     = sound_load_id(static_cast<int>(n));

	nprintf(("Sound", "SOUND ==> Finished loading '%s'\n", entry->filename));

	return sound_load_id(static_cast<int>(n));
}

// ---------------------------------------------------------------------------------------
// snd_unload() 
//
// Unload a sound from memory.  This will release the storage, and the sound must be re-loaded via
// sound_load() before it can be played again.
//
int snd_unload(sound_load_id n)
{
	if (!ds_initialized)
		return 0;

	if (!n.isValid()) {
		return 0;
	}

	if ((size_t)n.value() >= Sounds.size()) {
		return 0;
	}

	auto& snd = Sounds[n.value()];

	ds_unload_buffer(snd.sid);

	if (snd.sid != -1) {
		Snd_sram -= snd.uncompressed_size;
	}

	//If this sound is at the end of the array, we might as well get rid of it
	if ((size_t)n.value() == Sounds.size() - 1) {
		Sounds.pop_back();
	} else {
		snd.sid = -1;
		snd.flags &= ~SND_F_USED;
	}

	return 1;
}

// ---------------------------------------------------------------------------------------
// snd_unload_all() 
//
// Unload all sounds from memory. If there's a problem unloading a file the array may not be fully cleared
// but future files will still use unused spots, so the array size shouldn't grow out of control.
//
void snd_unload_all()
{
	while ( !Sounds.empty() ) {
		snd_unload(sound_load_id((int)(Sounds.size() - 1)));
	}
}

// ---------------------------------------------------------------------------------------
// snd_close()
//
// This is the companion function to snd_init()... it closes down the game sound system.
//
void snd_close(void)
{
	snd_stop_all();
	if (!ds_initialized) return;
	snd_unload_all();		// free the sound data stored in DirectSound secondary buffers
	dscap_close();	// Close DirectSoundCapture
	ds_close();		// Close DirectSound off
}

// ---------------------------------------------------------------------------------------
//	snd_play_raw()
//
// Allow a sound to be played directly from the index in Sounds[].  This bypasses the 
// normal game sound management system.
//
// returns:		-1		=>		sound could not be played
//					n		=>		handle for instance of sound
//
sound_handle snd_play_raw(sound_load_id soundnum, float pan, float vol_scale, int priority)
{
	game_snd gs;

	if (!soundnum.isValid()) {
		return sound_handle::invalid();
	}

	gs.sound_entries.resize(1);
	auto& entry = gs.sound_entries.back();

	entry.id = soundnum;
	entry.id_sig      = Sounds[soundnum.value()].sig;
	entry.filename[0] = 0;
//	entry.flags = GAME_SND_VOICE | GAME_SND_USE_DS3D;
	gs.flags = GAME_SND_VOICE;

	gs.volume_range = util::UniformFloatRange(1.0f);
	gs.pitch_range = util::UniformFloatRange(1.0f);

	return snd_play(&gs, pan, vol_scale, priority, true);
}

MONITOR( NumSoundsStarted )
MONITOR( NumSoundsLoaded )

// ---------------------------------------------------------------------------------------
//	snd_play()
//
//	NOTE: vol_scale parameter is the multiplicative scaling applied to the default volume
//       (vol_scale is a default parameter with a default value of 1.0f)
//
// input:	gs				=>	game-level sound description
//				pan			=>	-1 (full left) to 1.0 (full right), this is a default parm
//				vol_scale	=>	factor to scale default volume by (applied before global sound volume applied)
//				priority		=> SND_PRIORITY_MUST_PLAY
//									SND_PRIORITY_SINGLE_INSTANCE		(default value)
//									SND_PRIORITY_DOUBLE_INSTANCE
//									SND_PRIORITY_TRIPLE_INSTANCE
//
// returns:		-1		=>		sound could not be played
//					n		=>		handle for instance of sound
//
sound_handle snd_play(game_snd* gs, float pan, float vol_scale, int priority, bool is_voice_msg)
{
	float volume;
	loaded_sound* snd;

	if (!Sound_enabled)
		return sound_handle::invalid();

	if (!ds_initialized)
		return sound_handle::invalid();

	if (gs == nullptr) {
		UNREACHABLE("gamesnd parameter must not be null!");
		return sound_handle::invalid();
	}
	if (gs->flags & GAME_SND_NOT_VALID) {
		return sound_handle::invalid();
	}

	MONITOR_INC( NumSoundsStarted, 1 );

	auto entry = gamesnd_choose_entry(gs);

	if (!entry->id.isValid()) {
		entry->id = snd_load(entry, gs->flags);
		MONITOR_INC( NumSoundsLoaded, 1 );
	} else if (entry->id_sig != Sounds[entry->id.value()].sig) {
		entry->id = snd_load(entry, gs->flags);
	}

	if (!entry->id.isValid()) {
		Warning(LOCATION, "Failed to load one or more sounds for gamesnd %s!", gs->name.c_str());
		gs->flags |= GAME_SND_NOT_VALID;
		return sound_handle::invalid();
	}

	volume = gs->volume_range.next() * vol_scale;
	if ( gs->flags&GAME_SND_VOICE ) {
		volume *= (Master_voice_volume * aav_voice_volume);
	} else {
		volume *= (Master_sound_volume * aav_effect_volume);
	}
	if ( volume > 1.0f )
		volume = 1.0f;

	snd = &Sounds[entry->id.value()];

	if ( !(snd->flags & SND_F_USED) )
		return sound_handle::invalid();

	sound_handle handle;
	if ( volume > MIN_SOUND_VOLUME ) {
		handle = ds_play( snd->sid, entry->id_sig, ds_priority(priority), &gs->enhanced_sound_data, volume, pan, 0, is_voice_msg);

		if (handle.isValid()) {
			snd_set_pitch(handle, gs->pitch_range.next());
		}
	}

	return handle;
}

MONITOR( Num3DSoundsStarted )
MONITOR( Num3DSoundsLoaded )

// ---------------------------------------------------------------------------------------
// snd_play_3d()
//
//	NOTE: vol_scale parameter is the multiplicative scaling applied to the default volume
//       (vol_scale is a default parameter with a default value of 1.0f)
//
// input:	gs				=>	game-level sound description
//				source_pos	=>	global pos of where the sound is
//				listen_pos	=>	global pos of where listener is
//				radius		=>	optional parameter, this specifes distance at which to apply min/max distances
//				source_vel	=>	velocity of the source playing the sound (used for DirectSound3D only)
//				looping		=>	flag to indicate the sound should loop (default value 0)
//				vol_scale	=>	factor to scale the static volume by (applied before attenuation)
//				priority		=> SND_PRIORITY_MUST_PLAY
//									SND_PRIORITY_SINGLE_INSTANCE	(default value)
//									SND_PRIORITY_DOUBLE_INSTANCE
//									SND_PRIORITY_TRIPLE_INSTANCE
//				sound_fvec		=> forward vector of where sound is emitting from (RSX use only)
//				range_factor	=>	factor N, which increases distance sound is heard by N times (default value 1)
//
// returns:		-1		=>		sound could not be played
//					n		=>		handle for instance of sound
//
sound_handle snd_play_3d(game_snd* gs, vec3d* source_pos, vec3d* listen_pos, float radius, vec3d* source_vel,
                         int looping, float vol_scale, int priority, vec3d* /*sound_fvec*/, float range_factor,
                         int force, bool /*is_ambient*/)
{
	vec3d vector_to_sound;
	loaded_sound* snd;
	float volume, distance, max_volume;
	float min_range, max_range;
	float pan;

	if (!Sound_enabled)
		return sound_handle::invalid();

	if (!ds_initialized)
		return sound_handle::invalid();

	if (gs == nullptr) {
		UNREACHABLE("gamesnd parameter must not be null!");
		return sound_handle::invalid();
	}
	if (gs->flags & GAME_SND_NOT_VALID) {
		return sound_handle::invalid();
	}

	MONITOR_INC( Num3DSoundsStarted, 1 );
	
	auto entry = gamesnd_choose_entry(gs);

	if (!entry->id.isValid()) {
		entry->id = snd_load(entry, gs->flags);
		MONITOR_INC( Num3DSoundsLoaded, 1 );
	} else if (entry->id_sig != Sounds[entry->id.value()].sig) {
		entry->id = snd_load(entry, gs->flags);
	}

	if (!entry->id.isValid()) {
		Warning(LOCATION, "Failed to load one or more sounds for gamesnd %s!", gs->name.c_str());
		gs->flags |= GAME_SND_NOT_VALID;
		return sound_handle::invalid();
	}

	snd = &Sounds[entry->id.value()];

	if ( !(snd->flags & SND_F_USED) )
		return sound_handle::invalid();

	if (snd->sid < 0) {
		return sound_handle::invalid();
	}

	min_range = (gs->min + radius) * range_factor;
	max_range = (gs->max + radius) * range_factor;

	// DirectSound3D will not cut off sounds, no matter how quite they become.. so manually
	// prevent sounds from playing past the max distance.
	//IMPORTANT THIS IS NOT WORKING RIGHT OMG WTF
	distance = vm_vec_normalized_dir_quick( &vector_to_sound, source_pos, listen_pos );

	if ( (distance > max_range) && !force){
		return sound_handle::invalid();
	}

	float default_volume = gs->volume_range.next();
	max_volume = default_volume * vol_scale;

	if ( distance <= min_range ) {
		volume = max_volume;
	}
	else {
		volume = max_volume - max_volume*(distance/max_range);
	}

	if ( volume > 1.0f ){
		volume = 1.0f;
	}

	if ( priority == SND_PRIORITY_MUST_PLAY ) {
		if ( volume < 0.3 ) {
			priority = SND_PRIORITY_DOUBLE_INSTANCE;
		} 
	}

	volume *= (Master_sound_volume * aav_effect_volume);
	if ( (volume < MIN_SOUND_VOLUME) && !force) {
		return sound_handle::invalid();
	}

	// any stereo sounds will not play in proper 3D, but they should have
	// been converted to mono already!
	Assertion( snd->info.n_channels == 1, "Sound should be mono! Sound file: %s", snd->filename );

	sound_handle handle;
	if (Cmdline_no_3d_sound) {
		if (distance <= 0.0f) {
			pan = 0.0f;
		} else {
			pan = vm_vec_dot(&View_matrix.vec.rvec, &vector_to_sound);
		}

		handle = ds_play(snd->sid, entry->id_sig, ds_priority(priority), &gs->enhanced_sound_data, volume / default_volume, pan, looping);
	} else {
		handle = ds3d_play(snd->sid, entry->id_sig, source_pos, source_vel, min_range, max_range, looping, (max_volume*Master_sound_volume*aav_effect_volume), volume, &gs->enhanced_sound_data, ds_priority(priority));
	}

	if (handle.isValid()) {
		snd_set_pitch(handle, gs->pitch_range.next());
	}

	return handle;
}

// update the given 3d sound with a new position
void snd_update_3d_pos(sound_handle soundnum, game_snd* gs, vec3d* new_pos, float radius, float range_factor)
{
	if (Cmdline_no_3d_sound) {
		float vol, pan;
		
		// get new volume and pan vals
		snd_get_3d_vol_and_pan(gs, new_pos, &vol, &pan, radius, range_factor);

		// set volume
		snd_set_volume(soundnum, vol);

		// set pan
		snd_set_pan(soundnum, pan);
	} else {
		// MageKing17 - It's a 3D sound effect, we should use the function for setting the position of a 3D sound effect.
		int channel;

		if (!ds_initialized)
			return;

		Assertion( gs != NULL, "*gs was NULL in snd_update_3d_pos(); get a coder!\n" );

		channel = ds_get_channel(soundnum);
		if (channel == -1) {
			nprintf(( "Sound", "WARNING: Trying to set position for a non-playing sound.\n" ));
			return;
		}

		float min_range = (float) (fl2i( (gs->min) * range_factor));
		float max_range = (float) ((int)std::lround((gs->max) * range_factor));

		ds3d_update_buffer(channel, min_range, max_range, new_pos, NULL);
	}
}

// ---------------------------------------------------------------------------------------
// snd_get_3d_vol_and_pan()
//
// Based on the 3D position the player and the object, calculate
// the correct volume and pan.
//
// parameters:		gs			=> pointer to sound description
//						pos		=> 3D position used to calc volume and pan
//						vol		=> output parameter for the volume
//						pan		=> output parameter for the pan
//						radius	=>	optional parameter (default value 0) which indicates sound attenuation
//										should occur from this radius
//
// returns:			-1			=> could not determine vol or pan
//						0			=> success 
//
//	NOTE: the volume is not scaled by the Master_sound_volume, since this always occurs
//			when snd_play() or snd_play_looping() is called
//
int snd_get_3d_vol_and_pan(game_snd *gs, vec3d *pos, float* vol, float *pan, float radius, float range_factor)
{
	vec3d	vector_to_sound;
	float		distance, max_volume;

	*vol = 0.0f;
	*pan = 0.0f;

	if (!ds_initialized)
		return -1;

	Assertion( gs != NULL, "*gs was NULL in snd_get_3d_vol_and_pan(); get a coder!\n" );

	float min_range = (float) (fl2i( (gs->min) * range_factor));
	float max_range = (float) ((int)std::lround((gs->max) * range_factor));

	distance = vm_vec_normalized_dir_quick( &vector_to_sound, pos, &View_position );
	distance -= radius;

	max_volume = gs->volume_range.max();
	if ( distance <= min_range ) {
		*vol = max_volume;
	}
	else {
		*vol = max_volume - (distance - min_range) * max_volume / (max_range - min_range);
	}

	if ( *vol > 1.0f )
		*vol = 1.0f;

	if ( *vol > MIN_SOUND_VOLUME ) {
		if ( distance <= 0 )
			*pan = 0.0f;
		else
			*pan = vm_vec_dot(&View_matrix.vec.rvec,&vector_to_sound);
	}

	return 0;
}

/**
 * Starts looping a game sound
 *
 * @param gs game-level sound description
 * @param pan -1.0 (full left) to 1.0 (full right)
 * @param start_loop TODO remove this parameter
 * @param stop_loop TODO remove this parameter
 * @param vol_scale factor to scale the static volume by (applied before attenuation)
 * @param scriptingUpdateVolume if true the looping sound value is updated default is TRUE
 * @return -1 on error, else the handle for this playing sound
 */
sound_handle snd_play_looping(game_snd* gs, float pan, int /*start_loop*/, int /*stop_loop*/, float vol_scale,
                              int scriptingUpdateVolume)
{
	float volume;
	loaded_sound* snd;

	if (!Sound_enabled)
		return sound_handle::invalid();

	if (!ds_initialized)
		return sound_handle::invalid();

	if (gs == nullptr) {
		UNREACHABLE("gamesnd parameter must not be null!");
		return sound_handle::invalid();
	}
	if (gs->flags & GAME_SND_NOT_VALID) {
		return sound_handle::invalid();
	}

	auto entry = gamesnd_choose_entry(gs);

	if (!entry->id.isValid()) {
		entry->id = snd_load(entry, gs->flags);
	} else if (entry->id_sig != Sounds[entry->id.value()].sig) {
		entry->id = snd_load(entry, gs->flags);
	}

	if (!entry->id.isValid()) {
		Warning(LOCATION, "Failed to load one or more sounds for gamesnd %s!", gs->name.c_str());
		gs->flags |= GAME_SND_NOT_VALID;
		return sound_handle::invalid();
	}

	snd = &Sounds[entry->id.value()];

	if ( !(snd->flags & SND_F_USED) )
		return sound_handle::invalid();

	auto default_volume = gs->volume_range.next();
	volume = default_volume * vol_scale;
	volume *= (Master_sound_volume * aav_effect_volume);
	if ( volume > 1.0f )
		volume = 1.0f;

	sound_handle handle;
	if (volume > MIN_SOUND_VOLUME) {
		handle = ds_play( snd->sid, entry->id_sig, DS_MUST_PLAY, &gs->enhanced_sound_data, volume, pan, 1);

		if (handle.isValid()) {
			if (scriptingUpdateVolume) {
				currentlyLoopingSoundInfos.push_back(LoopingSoundInfo(handle, default_volume, vol_scale));
			}

			snd_set_pitch(handle, gs->pitch_range.next());
		}
	}

	return handle;
}

/**
 * Stop a sound from playing.
 *
 * @param sig handle to sound, what is returned from snd_play()
 */
void snd_stop(sound_handle sig)
{
	int channel;

	if (!ds_initialized) return;
	if (!sig.isValid())
		return;

	channel = ds_get_channel(sig);
	if ( channel == -1 )
		return;
	
	SCP_list<LoopingSoundInfo>::iterator iter = currentlyLoopingSoundInfos.begin();
	while (iter != currentlyLoopingSoundInfos.end())
	{
		if(iter->m_dsHandle == sig) {
			iter = currentlyLoopingSoundInfos.erase(iter);
		} else {
			++iter;
		}
	}

	ds_stop_channel(channel);
}

/**
 * Stop all playing sound channels (including looping sounds)
 *
 * NOTE: This stops all sounds that are playing from Channels[] sound buffers.
 * It doesn't stop every secondary sound buffer in existance.
 */
void snd_stop_all()
{
	if (!ds_initialized)
		return;

	currentlyLoopingSoundInfos.clear();
	ds_stop_channel_all();
}

/**
 * Set the volume of a currently playing sound
 *
 * @param sig		handle to sound, what is returned from snd_play()
 * @param volume	volume of sound (range: 0.0 -> 1.0)
 */
void snd_set_volume(sound_handle sig, float volume)
{
	int	channel;
	float	new_volume;

	if (!ds_initialized)
		return;

	if (!sig.isValid())
		return;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to set volume for a non-playing sound.\n" ));
		return;
	}

	bool isLoopingSound = false;

	SCP_list<LoopingSoundInfo>::iterator iter;
	for (iter = currentlyLoopingSoundInfos.begin(); iter != currentlyLoopingSoundInfos.end(); ++iter) {
		if(iter->m_dsHandle == sig) {
			iter->m_dynamicVolume = volume;

			isLoopingSound = true;
			break;
		}
	}

	//looping sound volumes are updated in snd_do_frame
	if(!isLoopingSound) {
		new_volume = volume * (Master_sound_volume * aav_effect_volume);
		ds_set_volume( channel, new_volume );
	}
}

// ---------------------------------------------------------------------------------------
// snd_set_pan()
//
// Set the pan of a currently playing sound
//
// parameters:		sig	=> handle to sound, what is returned from snd_play()
//						pan	=> pan of sound (range: -1.0 -> 1.0)
//
void snd_set_pan(sound_handle sig, float pan)
{
	int channel;

	if (!ds_initialized)
		return;

	if (!sig.isValid())
		return;
	
	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to set pan for a non-playing sound.\n" ));
		return;
	}

	ds_set_pan( channel, pan );
}

// ---------------------------------------------------------------------------------------
// snd_get_pitch()
//
// Return the pitch of a currently playing sound
//
// returns:			pitch of sound ( range: 100 to 100000)
//
// parameters:		sig	=> handle to sound, what is returned from snd_play()
//
float snd_get_pitch(sound_handle sig)
{
	int channel;

	if (!ds_initialized)
		return -1;

	if (!sig.isValid())
		return -1;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to get pitch for a non-playing sound.\n" ));
		return -1;
	}

	return ds_get_pitch(channel);
}

// ---------------------------------------------------------------------------------------
// snd_set_pitch()
//
// Set the pitch of a currently playing sound
//
// parameters:		sig		=> handle to sound, what is returned from snd_play()
//						pan		=> pitch of sound (must be greater than zero)
//
void snd_set_pitch(sound_handle sig, float pitch)
{
	int channel;

	if (!ds_initialized) return;
	if (!sig.isValid())
		return;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to set pitch for a non-playing sound.\n" ));
		return;
	}

	ds_set_pitch(channel, pitch);
}

// ---------------------------------------------------------------------------------------
// snd_is_playing()
//
// Determine if a sound is playing
//
// returns:			1				=> sound is currently playing
//						0				=> sound is not playing
//
// parameters:		sig	=> signature of sound, what is returned from snd_play()
//
int snd_is_playing(sound_handle sig)
{
	int	channel, is_playing;

	if (!ds_initialized)
		return 0;

	if (!sig.isValid())
		return 0;

	channel = ds_get_channel(sig);
	if ( channel == -1 )
		return 0;

	is_playing = ds_is_channel_playing(channel);
	if ( is_playing == TRUE ) {
		return 1;
	}

	return 0;
}

// ---------------------------------------------------------------------------------------
// snd_is_inited()
//
// 
int snd_is_inited()
{
	if ( !ds_initialized )
		return FALSE;

	return TRUE;
}

// return the time in ms for the duration of the sound
int snd_get_duration(sound_load_id snd_id)
{
	if (!snd_id.isValid())
		return 0;

	Assertion( !Sounds.empty(), "Sounds vector is empty. Why are we trying to look up an index?\n" );
	
	if ( Sounds.empty() )
		return 0;

	Assertion(Sounds[snd_id.value()].duration > 0, "Sound duration for sound %s is bogus (%d)\n",
	          Sounds[snd_id.value()].filename, Sounds[snd_id.value()].duration);

	if (Sounds[snd_id.value()].duration > 0)
		return Sounds[snd_id.value()].duration;
	else
		return 0;
}

// return the time in ms for the duration of the sound
const char* snd_get_filename(sound_load_id snd_id)
{
	Assertion(snd_id.isValid(), "Invalid sound id %d!", snd_id.value());

	return Sounds[snd_id.value()].filename;
}


MONITOR( SoundChannels )

// update the position of the listener for the specific 3D sound API we're 
// using
void snd_update_listener(vec3d *pos, vec3d *vel, matrix *orient)
{
	MONITOR_INC( SoundChannels, ds_get_number_channels() );
	ds3d_update_listener(pos, vel, orient);
}

// this could probably be optimized a bit
void snd_rewind(sound_handle snd_handle, float seconds)
{			
	float current_time,desired_time;
	float bps;
	uint32_t current_offset,desired_offset;
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
		return;

	auto channel = ds_get_channel(snd_handle);

	snd = &Sounds[ds_get_sound_id(channel)].info;

	current_offset = ds_get_play_position(channel);	// current offset into the sound
	bps = (float)snd->sample_rate * (float)snd->bits;							// data rate
	current_time = (float)current_offset/bps;										// how many seconds we're into the sound

	// don't rewind if it'll put us before the beginning of the sound
	if(current_time - seconds < 0.0f)
		return;

	desired_time = current_time - seconds;											// where we want to be
	desired_offset = (uint32_t)(desired_time * bps);								// the target
			
	ds_set_position(channel, desired_offset);
}

// this could probably be optimized a bit
void snd_ffwd(sound_handle snd_handle, float seconds)
{
	float current_time,desired_time;
	float bps;
	uint32_t current_offset,desired_offset;
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
		return;

	auto channel = ds_get_channel(snd_handle);

	snd = &Sounds[ds_get_sound_id(channel)].info;

	current_offset = ds_get_play_position(ds_get_channel(snd_handle));	// current offset into the sound
	bps = (float)snd->sample_rate * (float)snd->bits;							// data rate
	current_time = (float)current_offset/bps;										// how many seconds we're into the sound

	// don't rewind if it'll put us past the end of the sound
	if(current_time + seconds > (float)snd->duration)
		return;

	desired_time = current_time + seconds;											// where we want to be
	desired_offset = (uint32_t)(desired_time * bps);								// the target
			
	ds_set_position(ds_get_channel(snd_handle),desired_offset);
}

// this could probably be optimized a bit
void snd_set_pos(sound_handle snd_handle, float val, int as_pct)
{
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
		return;

	auto channel = ds_get_channel(snd_handle);

	snd = &Sounds[ds_get_sound_index(channel)].info;

	// set position as an absolute from 0 to 1
	if(as_pct){
		Assert((val >= 0.0) && (val <= 1.0));
		ds_set_position(ds_get_channel(snd_handle),(uint32_t)((float)snd->size * val));
	} 
	// set the position as an absolute # of seconds from the beginning of the sound
	else {
		float bps;
		Assert(val <= (float)snd->duration/1000.0f);
		bps = (float)snd->sample_rate * (float)snd->bits;							// data rate			
		ds_set_position(ds_get_channel(snd_handle),(uint32_t)(bps * val));
	}
}

// Return the number of sounds currently playing
int snd_num_playing()
{
	return ds_get_number_channels();
}

// Return the raw sound data for a loaded sound
//
// input:	handle	=>	index into Sounds[] array
//				data		=>	allocated mem to hold sound
//
// exit:		0	=>	success
//				!0	=>	fail
int snd_get_data(sound_load_id handle, char* data)
{
	Assert(handle.isValid());

	if (ds_get_data(Sounds[handle.value()].sid, data)) {
		return -1;
	}

	return 0;
}

// return the size of the sound data associated with the sound handle
int snd_size(sound_load_id handle, int* size)
{
	Assert(handle.isValid());

	if (ds_get_size(Sounds[handle.value()].sid, size)) {
		return -1;
	}

	return 0;
}

// retrieve the bits per sample and frequency for a given sound
void snd_get_format(sound_load_id handle, int* bits_per_sample, int* frequency)
{
	Assert((handle.isValid()) && ((size_t)handle.value() < Sounds.size()));

	if (bits_per_sample)
		*bits_per_sample = Sounds[handle.value()].info.bits;

	if (frequency)
		*frequency = Sounds[handle.value()].info.sample_rate;
}

// return the time for the sound to play in milliseconds
int snd_time_remaining(sound_handle handle)
{
	int channel, is_playing, time_remaining = 0;

	if (!ds_initialized)
		return 0;

	if (!handle.isValid())
		return 0;

	channel = ds_get_channel(handle);
	if ( channel == -1 )
		return 0;

	is_playing = ds_is_channel_playing(channel);
	if ( !is_playing ) {
		return 0;
	}

	int current_offset, max_offset;
	int bits_per_sample = 0, frequency = 0;

	auto sdx = snd_get_sound_id(handle);

	if (!sdx.isValid()) {
		Int3();
		return 0;
	}

	snd_get_format(sdx, &bits_per_sample, &frequency);

	if ( (bits_per_sample <= 0) || (frequency <= 0) )
		return 0;

	// handle ADPCM properly.  It's always 16bit for OpenAL but should be 8 or 16
	// for Windows.  We can't leave it as 4 here (the ADPCM rate) because that is the
	// compressed bps and the math is against the uncompressed bps.
	if ( bits_per_sample == 4 ) {
		bits_per_sample = 16;
	}

	Assert( bits_per_sample >= 8 );

	current_offset = ds_get_play_position(channel);
	max_offset = ds_get_channel_size(channel);

	if ( current_offset < max_offset ) {
		int bytes_remaining = max_offset - current_offset;
		int samples_remaining = bytes_remaining / (bits_per_sample/8);
		time_remaining = (int)std::lround(1000.0f * samples_remaining/frequency);
	}

//	mprintf(("time_remaining: %d\n", time_remaining));	
	return time_remaining;
}
sound_load_id snd_get_sound_id(sound_handle snd_handle)
{
	if (!ds_initialized) {
		return sound_load_id::invalid();
	}

	auto channel = ds_get_channel(snd_handle);

	if (channel < 0) {
		return sound_load_id::invalid();
	}

	auto channel_id = ds_get_sound_id(channel);

	for (size_t i = 0; i < Sounds.size(); i++) {
		if ((Sounds[i].flags & SND_F_USED) && (Sounds[i].sig == channel_id)) {
			return sound_load_id((int)i);
		}
	}

	return sound_load_id::invalid();
}


// snd_env_ interface

static int Sound_env_id;
static float Sound_env_volume;
static float Sound_env_damping;
static float Sound_env_decay;

// Set the sound environment
//
int sound_env_set(sound_env *se)
{
	if (ds_eax_set_all(se->id, se->volume, se->damping, se->decay) == 0) {
		Sound_env_id = se->id;
		Sound_env_volume = se->volume;
		Sound_env_damping = se->damping;
		Sound_env_decay = se->decay;
		return 0;
	} else {
		return -1;
	}
}

// Get the sound environment
//
int sound_env_get(sound_env *se, int preset)
{
	EAX_REVERBPROPERTIES er;

	if (ds_eax_get_all(&er, preset) == 0) {
		se->id = (int)er.environment;
		se->volume = er.fVolume;
		se->decay = er.fDecayTime_sec;
		se->damping = er.fDamping;
		return 0;
	} else {
		return -1;
	}
}

// Turn off the sound environment
//
int sound_env_disable()
{
	sound_env se;
	se.id = EAX_ENVIRONMENT_GENERIC;
	se.volume = 0.0f;
	se.damping = 0.0f;
	se.decay = 0.0f;
	sound_env_set(&se);
	return 0;
}

// Return 1 if EAX can used to set the sound environment, otherwise return 0
//
int sound_env_supported()
{
	return ds_eax_is_inited();
}

// Called once per game frame
//

void adjust_volume_on_frame(float* volume_now, aav* data);
void snd_do_frame()
{
	adjust_volume_on_frame(&aav_music_volume, &aav_data[AAV_MUSIC]);
	adjust_volume_on_frame(&aav_voice_volume, &aav_data[AAV_VOICE]);
	adjust_volume_on_frame(&aav_effect_volume, &aav_data[AAV_EFFECTS]);

	SCP_list<LoopingSoundInfo>::iterator iter;
	for (iter = currentlyLoopingSoundInfos.begin(); iter != currentlyLoopingSoundInfos.end(); ++iter) {

		float new_volume = iter->m_defaultVolume * iter->m_dynamicVolume * (Master_sound_volume * aav_effect_volume);
		ds_set_volume(ds_get_channel(iter->m_dsHandle), new_volume);
	}

	ds_do_frame();
}

void snd_adjust_audio_volume(int type, float percent, int time)
{
	Assert( type >= 0 && type < 3 );
	
	if ( type >= 0 && type < 3 ) {
		switch (type) {
		case AAV_MUSIC:
			aav_data[type].start_volume = aav_music_volume;
			if (percent < aav_music_volume)
				aav_data[type].delta = (aav_music_volume - percent) * -1.0f;
			else
				aav_data[type].delta = percent - aav_music_volume;
			break;
		case AAV_VOICE:
			aav_data[type].start_volume = aav_voice_volume;
			if (percent < aav_voice_volume)
				aav_data[type].delta = (aav_voice_volume - percent) * -1.0f;
			else
				aav_data[type].delta = percent - aav_voice_volume;
			break;
		case AAV_EFFECTS:
			aav_data[type].start_volume = aav_effect_volume;
			if (percent < aav_effect_volume)
				aav_data[type].delta = (aav_effect_volume - percent) * -1.0f;
			else
				aav_data[type].delta = percent - aav_effect_volume;
			break;
		default:
			Int3();
		}

		aav_data[type].delta_time = time;
		aav_data[type].start_time = (f2fl(Missiontime) * 1000);	
	}
}

void adjust_volume_on_frame(float* volume_now, aav* data)
{
	if (Missiontime == 0){
		return;
	}

	if (*volume_now == (data->start_volume + data->delta))
		return;
	
	float msMissiontime = (f2fl(Missiontime) * 1000);
	
	if ( msMissiontime > ( data->start_time + data->delta_time) ) {
		*volume_now = data->start_volume + data->delta;
		return;
	}
	
	float done = 0.0f;
	//How much change do we need?
	if (data->delta_time == 0)
		done = 1.0f;
	else
		done =(float) (msMissiontime - data->start_time)/data->delta_time;

	//apply change
	*volume_now = data->start_volume + (data->delta * done);
	CLAMP(*volume_now, 0.0f, 1.0f);

	// if setting music volume, trigger volume change in playing tracks
	// done here in order to avoid setting music volume in every frame regardless if it changed or not
	if (&aav_music_volume == volume_now) {
		audiostream_set_volume_all(Master_event_music_volume * aav_music_volume, ASF_EVENTMUSIC);
	}
}

void snd_aav_init()
{
	aav_music_volume = 1.0f;
	aav_voice_volume = 1.0f;
	aav_effect_volume = 1.0f;

	for (int i = 0; i < 3; i++) {
		aav_data[i].delta = 0.0f;
		aav_data[i].start_volume = 1.0f;
		aav_data[i].delta_time = 0;
		aav_data[i].start_time = 0.0f;	
	}
}
void snd_set_effects_volume(float volume)
{
	Assertion(volume >= 0.0f && volume <= 1.0f, "Invalid effects volume %f!", volume);

	Master_sound_volume = volume;
	main_hall_reset_ambient_vol();
}
void snd_set_voice_volume(float volume)
{
	Assertion(volume >= 0.0f && volume <= 1.0f, "Invalid voice volume %f!", volume);

	Master_voice_volume = volume;
	audiostream_set_volume_all(Master_voice_volume, ASF_VOICE);
}

game_snd::game_snd() : last_entry_index(std::numeric_limits<size_t>::max())
{
}

EnhancedSoundData::EnhancedSoundData() {
}
EnhancedSoundData::EnhancedSoundData(const int new_priority, const unsigned int new_limit) :
	priority(new_priority), limit(new_limit)
{
	Assertion(priority >= SND_ENHANCED_PRIORITY_MUST_PLAY && priority <= SND_ENHANCED_PRIORITY_LOW,
			  "EnhancedSoundData ctor given invalid priority %d", priority);
	Assertion(limit > 0, "EnhancedSoundData ctor given invalid limit %d", limit);
}
game_snd_entry::game_snd_entry() {
	filename[0] = 0;
}
