/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "render/3d.h"
#include "sound/sound.h"
#include "sound/audiostr.h"
#include "cmdline/cmdline.h"
#include "osapi/osapi.h"
#include "globalincs/vmallocator.h"
#include "debugconsole/console.h"

#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "cfile/cfile.h"

#include "sound/ds.h"
#include "sound/ds3d.h"
#include "sound/acm.h"
#include "sound/dscap.h"
#include "sound/ogg/ogg.h"

#include "globalincs/pstypes.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <limits.h>


#define SND_F_USED			(1<<0)		// Sounds[] element is used

typedef struct sound	{
	int				sid;			// software id
	char				filename[MAX_FILENAME_LEN];
	int				sig;
	int				flags;
	sound_info		info;
	int				uncompressed_size;		// size (in bytes) of sound (uncompressed)
	int				duration;
} sound;

SCP_vector<sound> Sounds;

int Sound_enabled = FALSE;				// global flag to turn sound on/off
int Snd_sram;								// mem (in bytes) used up by storing sounds in system memory
float Master_sound_volume = 1.0f;	// range is 0 -> 1, used for non-music sound fx
float Master_voice_volume = 0.7f;	// range is 0 -> 1, used for all voice playback

unsigned int SND_ENV_DEFAULT = 0;

struct LoopingSoundInfo {
	int m_dsHandle;
	float m_defaultVolume;	//!< The default volume of this sound (from game_snd)
	float m_dynamicVolume;	//!< The dynamic volume before scripted volume adjustment is applied (is updated via snd_set_volume)

	LoopingSoundInfo(int dsHandle, float defaultVolume, float dynamicVolume):
		m_dsHandle(dsHandle),
		m_defaultVolume(defaultVolume),
		m_dynamicVolume(dynamicVolume)
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

	if ( OGG_init() == -1 ) {
		mprintf(("Could not initialize the OGG vorbis converter.\n"));
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


void snd_spew_info()
{
	size_t idx;
	char txt[512] = "";
	CFILE *out = cfopen("sounds.txt", "wt", CFILE_NORMAL, CF_TYPE_DATA);
	if(out == NULL){
		return;
	}
	
	cfwrite_string("Sounds loaded :\n", out);

	// spew info for all sounds
	for (idx = 0; idx < Sounds.size(); idx++) {
		if(!(Sounds[idx].flags & SND_F_USED)){
			continue;
		}
		
		sprintf(txt, "%s (%ds)\n", Sounds[idx].filename, Sounds[idx].info.duration); 
		cfwrite_string(txt, out);
	}

	// close the outfile
	if(out != NULL){
		cfclose(out);
		out = NULL;
	}
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
			if(!stricmp(gs->filename, Sounds[idx].filename)){
				game_sounds++;
				done = 1;
			}
		}

		if(!done){
			for(SCP_vector<game_snd>::iterator gs = Snds.begin(); gs != Snds.end(); ++gs){
				if(!stricmp(gs->filename, Sounds[idx].filename)){
					interface_sounds++;
					done = 1;
				}
			}
		}

		if(!done){
			message_sounds++;
		}		
	}

	// spew info
	gr_set_color_fast(&Color_normal);
	gr_printf_no_resize(30, 100, "Game sounds : %d\n", game_sounds);
	gr_printf_no_resize(30, 110, "Interface sounds : %d\n", interface_sounds);
	gr_printf_no_resize(30, 120, "Message sounds : %d\n", message_sounds);
	gr_printf_no_resize(30, 130, "Total sounds : %d\n", game_sounds + interface_sounds + message_sounds);
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
int snd_load( game_snd *gs, int allow_hardware_load )
{
	int				type;
	sound_info		*si;
	sound			*snd;
	WAVEFORMATEX	*header = NULL;
	int				rc, FileSize, FileOffset;
	char			fullpath[MAX_PATH];
	char			filename[MAX_FILENAME_LEN];
	const int		NUM_EXT = 2;
	const char		*audio_ext[NUM_EXT] = { ".ogg", ".wav" };
	size_t			n;


	if ( !ds_initialized )
		return -1;

	if ( !VALID_FNAME(gs->filename) )
		return -1;

	for (n = 0; n < Sounds.size(); n++) {
		if ( !(Sounds[n].flags & SND_F_USED) ) {
			break;
		} else if ( !stricmp( Sounds[n].filename, gs->filename) ) {
			// extra check: make sure the sound is actually loaded in a compatible way (2D vs. 3D)
			//
			// NOTE: this will allow a duplicate 3D entry if 2D stereo entry exists,
			//       but will not load a duplicate 2D entry to get stereo if 3D
			//       version already loaded
			if ( (Sounds[n].info.n_channels == 1) || !(gs->flags & GAME_SND_USE_DS3D) ) {
				return (int)n;
			}
		}
	}

	if ( n == Sounds.size() ) {
		sound new_sound;
		new_sound.sid = -1;
		new_sound.flags = 0;

		Sounds.push_back( new_sound );
	}

	snd = &Sounds[n];

	si = &snd->info;

	si->data = NULL;
	si->size = 0;

	// strip the extension from the filename and try to open any extension
	strcpy_s( filename, gs->filename );
	char *p = strrchr(filename, '.');
	if ( p ) *p = 0;

	rc = cf_find_file_location_ext(filename, NUM_EXT, audio_ext, CF_TYPE_ANY, sizeof(fullpath) - 1, fullpath, &FileSize, &FileOffset);

	if (rc < 0)
		return -1;

	// open the file
	CFILE *fp = cfopen_special(fullpath, "rb", FileSize, FileOffset);

	// ok, we got it, so set the proper filename for logging purposes
	strcat_s(filename, audio_ext[rc]);

	nprintf(("Sound", "SOUND => Loading '%s'\n", filename));

	// ds_parse_sound() will do a NULL check on fp for us
	if ( ds_parse_sound(fp, &si->data, &si->size, &header, (rc == 0), &si->ogg_info) == -1 ) {
		nprintf(("Sound", "SOUND ==> Could not read sound file!\n"));
 		return -1;
	}

	// Load was a success, should be some sort of WAV or an OGG
	si->format				= header->wFormatTag;		// 16-bit flag (wFormatTag)
	si->n_channels			= header->nChannels;		// 16-bit channel count (nChannels)
	si->sample_rate			= header->nSamplesPerSec;	// 32-bit sample rate (nSamplesPerSec)
	si->avg_bytes_per_sec	= header->nAvgBytesPerSec;	// 32-bit average bytes per second (nAvgBytesPerSec)
	si->n_block_align		= header->nBlockAlign;		// 16-bit block alignment (nBlockAlign)
	si->bits				= header->wBitsPerSample;	// Read 16-bit bits per sample	

	type = 0;

	if (gs->flags & GAME_SND_USE_DS3D) {
		type |= DS_3D;
	}

	rc = ds_load_buffer(&snd->sid, &snd->uncompressed_size, header, si, type);

	// NOTE: "si" values can change once loaded in the buffer
	snd->duration = fl2i(1000.0f * ((si->size / (si->bits/8.0f)) / si->sample_rate / si->n_channels));

	// free the header if needed
	if (header != NULL)
		vm_free(header);

	// we don't need to keep si->data around anymore, this should be NULL for OGG files
	if (si->data != NULL) {
		vm_free(si->data);
		si->data = NULL;
 	}
 
	// make sure the file handle is closed
	if (fp != NULL)
		cfclose(fp);

	if ( rc == -1 ) {
		nprintf(("Sound", "SOUND ==> Failed to load '%s'\n", filename));
		return -1;
	}

	strncpy( snd->filename, gs->filename, MAX_FILENAME_LEN );
	snd->flags = SND_F_USED;

	snd->sig = snd_next_sig++;
	if (snd_next_sig < 0 ) snd_next_sig = 1;
	gs->id_sig = snd->sig;
	gs->id = (int)n;

//	nprintf(("Sound", "SOUND ==> Finished loading '%s'\n", filename));

	return (int)n;
}

// ---------------------------------------------------------------------------------------
// snd_unload() 
//
// Unload a sound from memory.  This will release the storage, and the sound must be re-loaded via
// sound_load() before it can be played again.
//
int snd_unload( int n )
{
	if (!ds_initialized)
		return 0;

	if ( (n < 0) || ((size_t)n >= Sounds.size()) ) {
		return 0;
	}

	ds_unload_buffer(Sounds[n].sid);

	if (Sounds[n].sid != -1) {
		Snd_sram -= Sounds[n].uncompressed_size;
	}

	//If this sound is at the end of the array, we might as well get rid of it
	if ( (size_t)n == Sounds.size()-1 ) {
		Sounds.pop_back();
	} else {
		Sounds[n].sid = -1;
		Sounds[n].flags &= ~SND_F_USED;
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
		snd_unload( Sounds.size()-1 );
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
int snd_play_raw( int soundnum, float pan, float vol_scale, int priority )
{
	game_snd gs;
	int		rval;

	if ( (soundnum < 0) || ((size_t)soundnum >= Sounds.size() ) ) {
		return -1;
	}

	gs.id = soundnum;
	gs.id_sig = Sounds[soundnum].sig;
	gs.filename[0] = 0;
	gs.default_volume = 1.0f;
//	gs.flags = GAME_SND_VOICE | GAME_SND_USE_DS3D;
	gs.flags = GAME_SND_VOICE;

	rval = snd_play(&gs, pan, vol_scale, priority, true);
	return rval;
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
int snd_play( game_snd *gs, float pan, float vol_scale, int priority, bool is_voice_msg )
{
	float volume;
	sound	*snd;

	int handle = -1;

	if (!Sound_enabled)
		return -1;

	if (gs == NULL) {
		Int3();
		return -1;
	}

	MONITOR_INC( NumSoundsStarted, 1 );

	if ( gs->id == -1 ) {
		gs->id = snd_load(gs);
		MONITOR_INC( NumSoundsLoaded, 1);
	} else if ( gs->id_sig != Sounds[gs->id].sig ) {
		gs->id = snd_load(gs);
	}

	if ( gs->id == -1 )
		return -1;

	volume = gs->default_volume * vol_scale;
	if ( gs->flags&GAME_SND_VOICE ) {
		volume *= (Master_voice_volume * aav_voice_volume);
	} else {
		volume *= (Master_sound_volume * aav_effect_volume);
	}
	if ( volume > 1.0f )
		volume = 1.0f;

	snd = &Sounds[gs->id];

	if ( !(snd->flags & SND_F_USED) )
		return -1;

	if (!ds_initialized)
		return -1;

	if ( volume > MIN_SOUND_VOLUME ) {
		handle = ds_play( snd->sid, gs->id_sig, ds_priority(priority), volume, pan, 0, is_voice_msg);
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
int snd_play_3d(game_snd *gs, vec3d *source_pos, vec3d *listen_pos, float radius, vec3d *source_vel, int looping, float vol_scale, int priority, vec3d *sound_fvec, float range_factor, int force )
{
	int		handle;
	vec3d	vector_to_sound;
	sound		*snd;
	float		volume, distance, max_volume;
	float		min_range, max_range;
	float		pan;

	if ( !Sound_enabled )
		return -1;

	if (gs == NULL) {
		Int3();
		return -1;
	}

	MONITOR_INC( Num3DSoundsStarted, 1 );

	if ( gs->id < 0 ) {
		gs->id = snd_load(gs);
		MONITOR_INC( Num3DSoundsLoaded, 1 );
	}else if ( gs->id_sig != Sounds[gs->id].sig ) {
		gs->id = snd_load(gs);
	}

	if ( gs->id == -1 )
		return -1;

	snd = &Sounds[gs->id];

	if ( !(snd->flags & SND_F_USED) )
		return -1;

	if (snd->sid < 0) {
		return -1;
	}

	handle = -1;

	min_range = (gs->min + radius) * range_factor;
	max_range = (gs->max + radius) * range_factor;

	if (!ds_initialized)
		return -1;
	
	// DirectSound3D will not cut off sounds, no matter how quite they become.. so manually
	// prevent sounds from playing past the max distance.
	//IMPORTANT THIS IS NOT WORKING RIGHT OMG WTF
	distance = vm_vec_normalized_dir_quick( &vector_to_sound, source_pos, listen_pos );

	if ( (distance > max_range) && !force){
		return -1;
	}

	max_volume = gs->default_volume * vol_scale;

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
		return -1;
	}

	// any stereo sounds will not play in proper 3D, but they should have
	// been converted to mono already!
	Assertion( snd->info.n_channels == 1, "Sound should be mono! Sound file: %s", snd->filename );

	if (Cmdline_no_3d_sound) {
		if (distance <= 0.0f) {
			pan = 0.0f;
		} else {
			pan = vm_vec_dot(&View_matrix.vec.rvec, &vector_to_sound);
		}

		handle = ds_play(snd->sid, gs->id_sig, ds_priority(priority), volume / gs->default_volume, pan, looping);
	} else {
		handle = ds3d_play(snd->sid, gs->id_sig, source_pos, source_vel, min_range, max_range, looping, (max_volume*Master_sound_volume*aav_effect_volume), volume, ds_priority(priority));
	}

	return handle;
}

// update the given 3d sound with a new position
void snd_update_3d_pos(int soundnum, game_snd *gs, vec3d *new_pos, float radius, float range_factor)
{
	float vol, pan;
	
	// get new volume and pan vals
	snd_get_3d_vol_and_pan(gs, new_pos, &vol, &pan, radius, range_factor);

	// set volume
	snd_set_volume(soundnum, vol);

	// set pan
	snd_set_pan(soundnum, pan);
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
	sound		*snd;

	*vol = 0.0f;
	*pan = 0.0f;

	if (!ds_initialized)
		return -1;

	if (gs == NULL) {
		Int3();
		return -1;
	}

	if ( gs->id == -1 ) {
		gs->id = snd_load(gs);
	}

	if (gs->id == -1)
		return -1;

	snd = &Sounds[gs->id];
	if ( !(snd->flags & SND_F_USED) )
		return -1;

	float min_range = (float) (fl2i( (gs->min) * range_factor));
	float max_range = (float) (fl2i( (gs->max) * range_factor + 0.5f));

	distance = vm_vec_normalized_dir_quick( &vector_to_sound, pos, &View_position );
	distance -= radius;

	max_volume = gs->default_volume;
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
int snd_play_looping( game_snd *gs, float pan, int start_loop, int stop_loop, float vol_scale, int scriptingUpdateVolume)
{	
	float volume;
	int	handle = -1;
	sound	*snd;	

	if (!Sound_enabled)
		return -1;

	if (!ds_initialized)
		return -1;

	if (gs == NULL) {
		Int3();
		return -1;
	}

	if ( gs->id == -1 ) {
		gs->id = snd_load(gs);
	}
	else if ( gs->id_sig != Sounds[gs->id].sig ) {
		gs->id = snd_load(gs);
	}

	if ( gs->id == -1 )
		return -1;

	snd = &Sounds[gs->id];

	if ( !(snd->flags & SND_F_USED) )
		return -1;

	volume = gs->default_volume * vol_scale;
	volume *= (Master_sound_volume * aav_effect_volume);
	if ( volume > 1.0f )
		volume = 1.0f;

	if (volume > MIN_SOUND_VOLUME) {
		handle = ds_play( snd->sid, gs->id_sig, DS_MUST_PLAY, volume, pan, 1);

		if(handle != -1 && scriptingUpdateVolume) {
			currentlyLoopingSoundInfos.push_back(LoopingSoundInfo(handle, gs->default_volume, vol_scale));
		}
	}

	return handle;
}

/**
 * Stop a sound from playing.
 *
 * @param sig handle to sound, what is returned from snd_play()
 */
void snd_stop( int sig )
{
	int channel;

	if (!ds_initialized) return;
	if ( sig < 0 ) return;

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
void snd_set_volume( int sig, float volume )
{
	int	channel;
	float	new_volume;

	if (!ds_initialized)
		return;

	if ( sig < 0 )
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
void snd_set_pan( int sig, float pan )
{
	int channel;

	if (!ds_initialized)
		return;

	if ( sig < 0 )
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
int snd_get_pitch(int sig)
{
	int channel, pitch=10000;

	if (!ds_initialized)
		return -1;

	if ( sig < 0 )
		return -1;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to get pitch for a non-playing sound.\n" ));
		return -1;
	}

	pitch = ds_get_pitch(channel);

	return pitch;
}

// ---------------------------------------------------------------------------------------
// snd_set_pitch()
//
// Set the pitch of a currently playing sound
//
// parameters:		sig		=> handle to sound, what is returned from snd_play()
//						pan		=> pitch of sound (range: 100 to 100000)
//
void snd_set_pitch( int sig, int pitch )
{
	int channel;

	if (!ds_initialized) return;
	if ( sig < 0 ) return;

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
int snd_is_playing( int sig )
{
	int	channel, is_playing;

	if (!ds_initialized)
		return 0;

	if ( sig < 0 )
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
int snd_get_duration(int snd_id)
{
	if ( snd_id < 0 )
		return 0;

	Assertion( !Sounds.empty(), "Sounds vector is empty. Why are we trying to look up an index?\n" );
	
	if ( Sounds.empty() )
		return 0;

	Assertion(Sounds[snd_id].duration > 0, "Sound duration for sound %s is bogus (%d)\n", Sounds[snd_id].filename, Sounds[snd_id].duration);

	if (Sounds[snd_id].duration > 0)
		return Sounds[snd_id].duration;
	else
		return 0;
}

// return the time in ms for the duration of the sound
const char *snd_get_filename(int snd_id)
{
	Assertion(snd_id >= 0 && snd_id < (int) Sounds.size(), "Invalid sound id %d!", snd_id);

	return Sounds[snd_id].filename;
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
void snd_rewind(int snd_handle, game_snd *gs, float seconds)
{			
	float current_time,desired_time;
	float bps;
	DWORD current_offset,desired_offset;
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
		return;

	if (gs->id < 0)
		return;

	snd = &Sounds[gs->id].info;
	
	current_offset = ds_get_play_position(ds_get_channel(snd_handle));	// current offset into the sound
	bps = (float)snd->sample_rate * (float)snd->bits;							// data rate
	current_time = (float)current_offset/bps;										// how many seconds we're into the sound

	// don't rewind if it'll put us before the beginning of the sound
	if(current_time - seconds < 0.0f)
		return;

	desired_time = current_time - seconds;											// where we want to be
	desired_offset = (DWORD)(desired_time * bps);								// the target
			
	ds_set_position(ds_get_channel(snd_handle),desired_offset);
}

// this could probably be optimized a bit
void snd_ffwd(int snd_handle, game_snd *gs, float seconds)
{
	float current_time,desired_time;
	float bps;
	DWORD current_offset,desired_offset;
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
		return;

	if (gs->id < 0)
		return;

	snd = &Sounds[gs->id].info;

	current_offset = ds_get_play_position(ds_get_channel(snd_handle));	// current offset into the sound
	bps = (float)snd->sample_rate * (float)snd->bits;							// data rate
	current_time = (float)current_offset/bps;										// how many seconds we're into the sound

	// don't rewind if it'll put us past the end of the sound
	if(current_time + seconds > (float)snd->duration)
		return;

	desired_time = current_time + seconds;											// where we want to be
	desired_offset = (DWORD)(desired_time * bps);								// the target
			
	ds_set_position(ds_get_channel(snd_handle),desired_offset);
}

// this could probably be optimized a bit
void snd_set_pos(int snd_handle, game_snd *gs, float val,int as_pct)
{
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
		return;

	if (gs->id < 0)
		return;

	snd = &Sounds[gs->id].info;

	// set position as an absolute from 0 to 1
	if(as_pct){
		Assert((val >= 0.0) && (val <= 1.0));
		ds_set_position(ds_get_channel(snd_handle),(DWORD)((float)snd->size * val));
	} 
	// set the position as an absolute # of seconds from the beginning of the sound
	else {
		float bps;
		Assert(val <= (float)snd->duration/1000.0f);
		bps = (float)snd->sample_rate * (float)snd->bits;							// data rate			
		ds_set_position(ds_get_channel(snd_handle),(DWORD)(bps * val));
	}
}

// Return the number of sounds currently playing
int snd_num_playing()
{
	return ds_get_number_channels();
}

// Stop the first channel found that is playing a sound
void snd_stop_any_sound()
{
	int i;

	for ( i = 0; i < 16; i++ ) {
		if ( ds_is_channel_playing(i) ) {
			ds_stop_channel(i);
			break;
		}
	}
}

// Return the raw sound data for a loaded sound
//
// input:	handle	=>	index into Sounds[] array
//				data		=>	allocated mem to hold sound
//
// exit:		0	=>	success
//				!0	=>	fail
int snd_get_data(int handle, char *data)
{
	Assert( (handle >= 0) && ((size_t)handle < Sounds.size()) );

	if ( ds_get_data(Sounds[handle].sid, data) ) {
		return -1;
	}

	return 0;
}

// return the size of the sound data associated with the sound handle
int snd_size(int handle, int *size)
{
	Assert( (handle >= 0) && ((size_t)handle < Sounds.size()) );

	if ( ds_get_size(Sounds[handle].sid, size) ) {
		return -1;
	}

	return 0;
}

// retrieve the bits per sample and frequency for a given sound
void snd_get_format(int handle, int *bits_per_sample, int *frequency)
{
	Assert( (handle >= 0) && ((size_t)handle < Sounds.size()) );

	if (bits_per_sample)
		*bits_per_sample = Sounds[handle].info.bits;

	if (frequency)
		*frequency = Sounds[handle].info.sample_rate;
}

// given a sound sig (handle) return the index in Sounds[] for that sound
int snd_get_index(int sig)
{
	int channel, channel_id;
	size_t i;

	channel = ds_get_channel(sig);

	if (channel < 0) {
		return -1;
	}

	channel_id = ds_get_sound_id(channel);

	for (i = 0; i < Sounds.size(); i++) {
		if ( (Sounds[i].flags & SND_F_USED) && (Sounds[i].sig == channel_id) ) {
			return (int)i;
		}
	}

	return -1;
}

// return the time for the sound to play in milliseconds
int snd_time_remaining(int handle)
{
	int channel, is_playing, time_remaining = 0;

	if (!ds_initialized)
		return 0;

	if ( handle < 0 )
		return 0;

	channel = ds_get_channel(handle);
	if ( channel == -1 )
		return 0;

	is_playing = ds_is_channel_playing(channel);
	if ( !is_playing ) {
		return 0;
	}

	int current_offset, max_offset, sdx;
	int bits_per_sample = 0, frequency = 0;

	sdx = snd_get_index(handle);

	if (sdx < 0) {
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
		time_remaining = fl2i(1000.0f * samples_remaining/frequency + 0.5f);
	}

//	mprintf(("time_remaining: %d\n", time_remaining));	
	return time_remaining;
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

// return the number of samples per pre-defined measure in a piece of audio
int snd_get_samples_per_measure(char *filename, float num_measures)
{
	sound_info si;
	uint total_bytes = 0;
	int bytes_per_measure = 0;

	// although this function doesn't require sound to work, if sound is disabled then this is useless
	if ( !Sound_enabled )
		return -1;

	if ( !VALID_FNAME(filename) )
		return -1;

	if (num_measures <= 0.0f)
		return -1;


	if ( ds_parse_sound_info(filename, &si) ) {
		nprintf(("Sound", "Could not read sould file '%s' for SPM check!\n", filename));
		return -1;
	}

	total_bytes = (uint)si.size;

	// if it's ADPCM then we have to account for the uncompressed size
	if (si.format == WAVE_FORMAT_ADPCM) {
		total_bytes *= 16;	// we always decode APDCM to 16-bit (for OpenAL at least)
		total_bytes /= si.bits;
		total_bytes *= 2;	// this part isn't at all accurate though
	}

	bytes_per_measure = fl2i(total_bytes / num_measures);

	// ok, now return the samples per measure (which is half of bytes_per_measure)
	return (bytes_per_measure / 2);
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

uint nextSignature = 0;

game_snd::game_snd()
	: name ( "" ), signature( nextSignature++ ), default_volume( 0 ), preload( false ), id( -1 ), id_sig( -1 ), flags( 0 )
{
	filename[0] = 0;
	min = 0;
	max = 0;
}
