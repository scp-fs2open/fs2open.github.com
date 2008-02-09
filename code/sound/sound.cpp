/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/Sound.cpp $
 * $Revision: 2.10 $
 * $Date: 2004-12-25 00:23:46 $
 * $Author: wmcoolmon $
 *
 * Low-level sound code
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.9  2004/07/26 20:47:52  Kazan
 * remove MCD complete
 *
 * Revision 2.8  2004/07/12 16:33:06  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.7  2004/06/28 02:13:08  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 2.6  2004/06/22 23:14:10  wmcoolmon
 * Nonworking OGG support for sound (not music) added, disabled load-only-used-weapons code, modification to sound system registry code.
 * OGG code has been commented out, so you don't need the SDK yet.
 *
 * Revision 2.5  2004/06/18 04:59:55  wmcoolmon
 * Only used weapons paged in instead of all, fixed music box in FRED, sound quality settable with SoundSampleRate and SoundSampleBits registry values
 *
 * Revision 2.4  2004/03/05 09:01:59  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.3  2003/03/02 06:37:24  penguin
 * Use multimedia headers in local dir, not system's (headers are not present in MinGW distribution)
 *  - penguin
 *
 * Revision 2.2  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/07 19:56:00  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 9     9/12/99 8:09p Dave
 * Fixed problem where skip-training button would cause mission messages
 * not to get paged out for the current mission.
 * 
 * 8     8/27/99 6:38p Alanl
 * crush the blasted repeating messages bug
 * 
 * 7     8/22/99 11:06p Alanl
 * don't convert priority in snd_play_3d
 * 
 * 6     8/04/99 9:48p Alanl
 * fix bug with setting 3D properties on a 2D sound buffer
 * 
 * 5     6/18/99 5:16p Dave
 * Added real beam weapon lighting. Fixed beam weapon sounds. Added MOTD
 * dialog to PXO screen.
 * 
 * 4     5/23/99 8:11p Alanl
 * Added support for EAX
 * 
 * 3     1/29/99 12:47a Dave
 * Put in sounds for beam weapon. A bunch of interface screens (tech
 * database stuff).
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 77    6/13/98 6:02p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 76    5/23/98 9:27p Lawrance
 * Change ACM failure message to use a message box, not a Warning()
 * 
 * 75    5/15/98 3:36p John
 * Fixed bug with new graphics window code and standalone server.  Made
 * hwndApp not be a global anymore.
 * 
 * 74    5/12/98 5:39p Lawrance
 * Increase MAX_SOUNDS to 175.. to account for missions with lots of voice
 * 
 * 73    5/12/98 2:43p Lawrance
 * Make snd_time_remaining() work for arbitrary format
 * 
 * 72    5/11/98 2:02p Andsager
 * doh!
 * 
 * 71    5/11/98 1:56p Andsager
 * increase min_range in snd_play_3d when range_factor is not 1.0
 * 
 * 70    5/10/98 3:49p Sandeep
 * Fix problem with having the audio streaming while trying to close down
 * sound
 * 
 * 69    5/09/98 10:48p Lawrance
 * Default voice volume to 0.7
 * 
 * 68    5/05/98 4:49p Lawrance
 * Put in code to authenticate A3D, improve A3D support
 * 
 * 67    4/25/98 12:01p Lawrance
 * try to init sound for 5 seconds... to overcome conflicts with launch
 * app sounds
 * 
 * 66    4/20/98 12:03a Lawrance
 * Allow prioritizing of CTRL3D buffers
 * 
 * 65    4/19/98 9:30p Lawrance
 * Use Aureal_enabled flag
 * 
 * 64    4/18/98 9:12p Lawrance
 * Added Aureal support.
 * 
 * 63    4/14/98 3:29p John
 * took out mprintf
 * 
 * 62    4/13/98 5:04p Lawrance
 * Write functions to determine how many milliseconds are left in a sound
 * 
 * 61    4/09/98 5:53p Lawrance
 * Make DirectSound init more robust
 * 
 * 60    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 59    3/29/98 12:56a Lawrance
 * preload the warp in and explosions sounds before a mission.
 * 
 * 58    3/25/98 6:10p Lawrance
 * Work on DirectSound3D
 * 
 * 57    3/24/98 4:28p Lawrance
 * Make DirectSound3D support more robust
 * 
 * 56    3/23/98 10:32a Lawrance
 * Add functions for extracting raw sound data
 * 
 * 55    3/21/98 3:34p Lawrance
 * Allow 3d sounds to have their ranges modified dynamically
 * 
 * 54    3/19/98 5:36p Lawrance
 * Add some sound debug functions to see how many sounds are playing, and
 * to start/stop random looping sounds.
 * 
 * 53    3/17/98 5:55p Lawrance
 * Support object-linked sounds for asteroids.
 * 
 * 52    2/20/98 8:32p Lawrance
 * Add radius parm to sound_play_3d()
 * 
 * 51    2/18/98 5:49p Lawrance
 * Even if the ADPCM codec is unavailable, allow game to continue.
 * 
 * 50    2/15/98 4:43p Lawrance
 * work on real-time voice
 * 
 * 49    2/06/98 7:30p John
 * Added code to monitor the number of channels of sound actually playing.
 * 
 * 48    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 47    1/31/98 5:57p Lawrance
 * remove debug code that was stopping any sounds from playing
 * 
 * 46    1/31/98 5:48p Lawrance
 * Start on real-time voice recording
 * 
 * 45    1/19/98 11:37p Lawrance
 * Fixing Optimization build warnings
 * 
 * 44    1/13/98 5:36p Lawrance
 * Reduce default sound volume to 0.9.
 * 
 * 43    1/11/98 11:14p Lawrance
 * Preload sounds that we expect will get played.
 * 
 * 42    1/10/98 1:14p John
 * Added explanation to debug console commands
 * 
 * 41    1/07/98 11:08a Lawrance
 * pass priority to snd_play_raw()
 * 
 * 40    12/21/97 4:33p John
 * Made debug console functions a class that registers itself
 * automatically, so you don't need to add the function to
 * debugfunctions.cpp.  
 * 
 * 39    12/05/97 5:19p Lawrance
 * re-do sound priorities to make more general and extensible
 * 
 * 38    12/04/97 10:21a Lawrance
 * if a 3D sound has priority, only play if above a minimum volume level
 * 
 * 37    11/20/97 5:36p Dave
 * Hooked in a bunch of main hall changes (including sound). Made it
 * possible to reposition (rewind/ffwd) 
 * sound buffer pointers. Fixed animation direction change framerate
 * problem.
 * 
 * 36    11/20/97 1:06a Lawrance
 * Add Master_voice_volume, make voices play back at correctly scaled
 * volumes
 * 
 * 35    11/12/97 4:40p Dave
 * Put in multiplayer campaign support parsing, loading and saving. Made
 * command-line variables better named. Changed some things on the initial
 * pilot select screen.
 * 
 * 34    10/14/97 11:33p Lawrance
 * get RSX implemented
 * 
 * 33    10/13/97 7:41p Lawrance
 * store duration of sound
 * 
 * 32    10/06/97 4:12p Lawrance
 * fix volume bug with 3d sounds
 * 
 * 31    10/01/97 5:55p Lawrance
 * change call to snd_play_3d() to allow for arbitrary listening position
 * 
 * 30    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 29    9/03/97 5:03p Lawrance
 * add support for -nosound command line parm
 * 
 * 28    7/31/97 11:16a Dan
 * Add some Asserts() to check validity of vectors send to DirectSound3D
 * 
 * 27    7/28/97 11:39a Lawrance
 * allow individual volume scaling on 3D buffers
 * 
 * 26    7/17/97 9:53a Lawrance
 * if a sound is supposed to use DirectSound3D, ensure it does when
 * reloading it
 * 
 * 25    7/17/97 9:32a John
 * made all directX header files name start with a v
 * 
 * 24    7/15/97 11:15a Lawrance
 * limit the max instances of simultaneous sound effects, implement
 * priorities to force critical sounds
 * 
 * 23    7/13/97 5:52p Lawrance
 * allow snd_set_volume() to set volume at any level
 * 
 * 22    6/13/97 4:44p Lawrance
 * added another sound hook in ship selection
 * 
 * 21    6/09/97 11:50p Lawrance
 * integrating DirectSound3D
 * 
 * 20    6/05/97 11:25a Lawrance
 * use sound signatures to ensure correct sound is loaded
 * 
 * 19    6/05/97 1:36a Lawrance
 * using a new interface to play sounds
 * 
 * 18    6/05/97 1:08a Lawrance
 * new sound play interface
 * 
 * 17    6/03/97 9:23a Lawrance
 * fix bug that caused when sound not found
 * 
 * 16    6/02/97 1:45p Lawrance
 * implementing hardware mixing
 * 
 * 15    5/29/97 4:01p Lawrance
 * don't fail sound init if dsound3d fails init
 * 
 * 14    5/29/97 3:32p Lawrance
 * added call to snd_do_frame()
 * 
 * 13    5/29/97 12:04p Lawrance
 * split off acm, direct sound, and direct sound 3d portions to separate
 * files
 * 
 * 12    5/21/97 3:55p Lawrance
 * when reclaiming sound channels, never stop a looping sound
 * 
 * 11    5/19/97 4:32p Lawrance
 * remove misleading nprintf
 * 
 * 10    5/15/97 9:05a Lawrance
 * If no sound channels, take away the lowest volume sound (if that volume
 * is less than requested sound fx volume)
 * 
 * 9     5/12/97 10:32a Lawrance
 * take out nprintf
 * 
 * 8     5/09/97 4:51p Lawrance
 * improve pan sound
 * 
 * 7     5/09/97 4:34p Lawrance
 * improve comments
 * 
 * 6     5/07/97 3:29p Lawrance
 * make volume conversion use a lookup table, fix errors in volume
 * conversion
 * 
 * 5     5/07/97 11:33a Lawrance
 * improve snd_chg_loop_status()
 * 
 * 4     5/06/97 9:36a Lawrance
 * added support for min and max distances for 3d sounds
 * 
 * 3     5/02/97 4:36p Lawrance
 * send correct volume scaling when playing a 3D sound
 * 
 * 2     4/28/97 5:13p John
 * more moving sound/movie code out of osapi.
 * 
 * $NoKeywords: $
 */

#include <windows.h>
#include <limits.h>

#include "render/3d.h"
#include "sound/sound.h"
#include "sound/audiostr.h"
#include "cmdline/cmdline.h"
#include "osapi/osapi.h"

#include "gamesnd/gamesnd.h"
#include "globalincs/alphacolors.h"
#include "cfile/cfile.h"

#include "sound/ds.h"
#include "sound/ds3d.h"
#include "sound/acm.h"
#include "sound/dscap.h"
#include "sound/ogg/ogg.h"
		




#define SND_F_USED			(1<<0)		// Sounds[] element is used

typedef struct sound	{
	int				sid;			// software id
	int				hid;			// hardware id, -1 if sound is not in hardware
	char				filename[MAX_FILENAME_LEN];
	int				sig;
	int				flags;
	sound_info		info;
	int				uncompressed_size;		// size (in bytes) of sound (uncompressed)
	int				duration;
} sound;

sound	Sounds[MAX_SOUNDS];

int Sound_enabled = TRUE;				// global flag to turn sound on/off
int Snd_sram;								// mem (in bytes) used up by storing sounds in system memory
int Snd_hram;								// mem (in bytes) used up by storing sounds in soundcard memory
float Master_sound_volume = 1.0f;	// range is 0 -> 1, used for non-music sound fx
float Master_voice_volume = 0.7f;	// range is 0 -> 1, used for all voice playback

// min volume to play a sound after all volume processing (range is 0.0 -> 1.0)
#define	MIN_SOUND_VOLUME				0.10f

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
	int i;

	// flag all Sounds[] as free
	for (i=0; i<MAX_SOUNDS; i++ )	{
		Sounds[i].flags &=  ~SND_F_USED;
		Sounds[i].sid = -1;
		Sounds[i].hid = -1;
	}

	// reset how much storage sounds are taking up in memory
	Snd_sram = 0;
	Snd_hram = 0;
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
int snd_init(int use_a3d, int use_eax, unsigned int sample_rate, unsigned short sample_bits)
{
	int rval;

	if ( Cmdline_freespace_no_sound )
		return 0;

	if (ds_initialized)	{
		nprintf(( "Sound", "SOUND => Direct Sound is already initialized!\n" ));
		return 1;
	}

	snd_clear();

	// Init DirectSound 

	// Connect to DirectSound
	int num_tries=0;
	int gave_warning = 0;
	while(1) {
		rval = ds_init(use_a3d, use_eax, sample_rate, sample_bits);

		if( rval != 0 ) {
			nprintf(( "Sound", "SOUND ==> Error initializing DirectSound, trying again in 1 second.\n"));
			Sleep(1000);
		} else {
			break;
		}

		if ( num_tries++ > 5 ) {
			if ( !gave_warning ) {
				MessageBox(NULL, XSTR("DirectSound could not be initialized.  If you are running any applications playing sound in the background, you should stop them before continuing.",971), NULL, MB_OK);
				gave_warning = 1;
			} else {
				goto Failure;
			}
		}
	}

	// Init the Audio Compression Manager
	if ( ACM_init() == -1 ) {
		HWND hwnd = (HWND)os_get_window();
		MessageBox(hwnd, XSTR("Could not properly initialize the Microsoft ADPCM codec.\n\nPlease see the readme.txt file for detailed instructions on installing the Microsoft ADPCM codec.",972), NULL, MB_OK);
//		Warning(LOCATION, "Could not properly initialize the Microsoft ADPCM codec.\nPlease see the readme.txt file for detailed instructions on installing the Microsoft ADPCM codec.");
	}

	if ( OGG_init() == -1)
	{
		mprintf(("Could not initialize the OGG vorbis converter."));
	}

	// Init the audio streaming stuff
	audiostream_init();
			
	ds_initialized = 1;
	return 1;

Failure:
//	Warning(LOCATION, "Sound system was unable to be initialized.  If you continue, sound will be disabled.\n");
	nprintf(( "Sound", "SOUND => Direct Sound init unsuccessful, continuing without sound.\n" ));
	return 0;
}


void snd_spew_info()
{
	int idx;
	char txt[512] = "";
	CFILE *out = cfopen("sounds.txt", "wt", CFILE_NORMAL, CF_TYPE_DATA);
	if(out == NULL){
		return;
	}
	
	cfwrite_string("Sounds loaded :\n", out);

	// spew info for all sounds
	for(idx=0; idx<MAX_SOUNDS; idx++){
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
DCF(show_sounds, "")
{
	Sound_spew = !Sound_spew;
	if(Sound_spew){
		dc_printf("Sound debug info ON");
	} else {
		dc_printf("Sound debug info OFF");
	}
}
void snd_spew_debug_info()
{
	int game_sounds = 0;
	int message_sounds = 0;
	int interface_sounds = 0;
	int done = 0;
	int s_idx;

	if(!Sound_spew){
		return;
	}

	// count up game, interface and message sounds
	for(int idx=0; idx<MAX_SOUNDS; idx++){
		if(!Sounds[idx].flags & SND_F_USED){
			continue;
		}

		done = 0;

		// what kind of sound is this
		for(s_idx=0; s_idx<MAX_GAME_SOUNDS; s_idx++){
			if(!stricmp(Snds[s_idx].filename, Sounds[idx].filename)){
				game_sounds++;
				done = 1;
			}
		}

		if(!done){
			for(s_idx=0; s_idx<MAX_GAME_SOUNDS; s_idx++){
				if(!stricmp(Snds_iface[s_idx].filename, Sounds[idx].filename)){
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
	gr_printf(30, 100, "Game sounds : %d\n", game_sounds);
	gr_printf(30, 110, "Interface sounds : %d\n", interface_sounds);
	gr_printf(30, 120, "Message sounds : %d\n", message_sounds);
	gr_printf(30, 130, "Total sounds : %d\n", game_sounds + interface_sounds + message_sounds);
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
	int				n, rc, type;
	sound_info		*si;
	sound				*snd;
	WAVEFORMATEX	*header = NULL;

	if ( gs->filename == NULL || gs->filename[0] == 0 )
		return -1;

	for (n=0; n<MAX_SOUNDS; n++ )	{
		if (!(Sounds[n].flags & SND_F_USED))
			break;
		else if ( !stricmp( Sounds[n].filename, gs->filename )) {
			gs->sig = Sounds[n].sig;
			return n;
		}
	}

	if ( n == MAX_SOUNDS ) {
#ifndef NDEBUG
		// spew sound info
		snd_spew_info();
#endif

		Int3();
		return -1;
	}

	snd = &Sounds[n];

	if ( !ds_initialized )
		return -1;

	si = &snd->info;

	CFILE * fp = cfopen(gs->filename, "rb");
	if(fp == NULL)
	{
		mprintf(("Couldn't open sound file %s\n", gs->filename));
	}
	//Because ds_parse_wave uses seek_set, we can get away with not resetting the stream
	if(!ov_open_callbacks(fp, &si->ogg_info, NULL, 0, cfile_callbacks))
	{
		//It's an OGG
		ov_info(&si->ogg_info, -1);
		si->format = OGG_FORMAT_VORBIS;
		si->n_channels = si->ogg_info.vi->channels;
		si->sample_rate = si->ogg_info.vi->rate;
		si->bits = 16;								//OGGs always decoded at 16 bits here
		si->n_block_align = si->n_channels * 2;
		si->avg_bytes_per_sec = si->sample_rate * si->n_block_align;
		if(ov_pcm_total(&si->ogg_info, -1) < UINT_MAX)
		{
			si->size = (uint) si->sample_rate * si->n_channels * si->bits * 2;
		}
		else
		{
			mprintf(("%s is too big to play!", gs->filename));
			return -1;
		}
		snd->duration = fl2i(1000.0f * (si->size / (si->bits/8.0f)) / si->sample_rate);
	}
	else if ( ds_parse_wave(fp, &si->data, &si->size, &header) != -1 )
	{
		//It's some sort of WAV
		si->format					= header->wFormatTag;		// 16-bit flag (wFormatTag)
		si->n_channels				= header->nChannels;			// 16-bit channel count (nChannels)
		si->sample_rate			= header->nSamplesPerSec;	// 32-bit sample rate (nSamplesPerSec)
		si->avg_bytes_per_sec	= header->nAvgBytesPerSec;	// 32-bit average bytes per second (nAvgBytesPerSec)
		si->n_block_align			= header->nBlockAlign;		// 16-bit block alignment (nBlockAlign)
		si->bits						= header->wBitsPerSample;	// Read 16-bit bits per sample	
		snd->duration = fl2i(1000.0f * (si->size / (si->bits/8.0f)) / si->sample_rate);

		//Only close the FP if it's not Vorbis...vorbis neeeds it.
		cfclose(fp);
	}
	else
	{
		mprintf(("Could not read sound file %s", gs->filename));
		return -1;
	}		

	type = 0;

	if ( allow_hardware_load ) {
		if ( gs->preload ) {
			type |= DS_HARDWARE;
		}
	}

	if ( (gs->flags&GAME_SND_USE_DS3D)  ) {
		type |= DS_USE_DS3D;
	}
	
	rc = ds_load_buffer(&snd->sid, &snd->hid, &snd->uncompressed_size, header, si, type);

	free(header);
	if(si->format != OGG_FORMAT_VORBIS)
	{
		free(si->data);	// don't want to keep this around
	}

	if ( rc == -1 )
		return -1;

	strncpy( snd->filename, gs->filename, MAX_FILENAME_LEN );
	snd->flags = SND_F_USED;

	snd->sig = snd_next_sig++;
	if (snd_next_sig < 0 ) snd_next_sig = 1;
	gs->id_sig = snd->sig;
	gs->id = n;

	nprintf(("Sound", "Loaded %s\n", gs->filename));
	return n;
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

	if ( (n < 0) || ( n >= MAX_SOUNDS) )
		return 0;

	if ( !(Sounds[n].flags & SND_F_USED) )
		return 0;
	
	ds_unload_buffer(Sounds[n].sid, Sounds[n].hid);
	if ( Sounds[n].sid != -1 ) {
		Snd_sram -= Sounds[n].uncompressed_size;
	}
	if ( Sounds[n].hid != -1 ) {
		Snd_hram -= Sounds[n].uncompressed_size;
	}

	Sounds[n].flags &= ~SND_F_USED;

	return 1;
}

// ---------------------------------------------------------------------------------------
// snd_unload_all() 
//
// Unload all sounds from memory.  This will release the storage, and the sound must be re-loaded via
// sound_load() before it can be played again.
//
void snd_unload_all()
{
	int i;
	for (i=0; i<MAX_SOUNDS; i++ )	{
		if ( Sounds[i].flags & SND_F_USED )
			snd_unload(i);
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
	ACM_close();	// Close the Audio Compression Manager (ACM)
	ds3d_close();	// Close DirectSound3D
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

	gs.id = soundnum;
	gs.id_sig = Sounds[soundnum].sig;
	gs.filename[0] = 0;
	gs.default_volume = 1.0f;
//	gs.flags = GAME_SND_VOICE | GAME_SND_USE_DS3D;
	gs.flags = GAME_SND_VOICE;

	rval = snd_play(&gs, 0.0f, vol_scale, priority, true);
	return rval;
}

MONITOR( NumSoundsStarted );
MONITOR( NumSoundsLoaded );

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

	Assert( gs != NULL );

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
		volume *= Master_voice_volume;
	} else {
		volume *= Master_sound_volume;
	}
	if ( volume > 1.0f )
		volume = 1.0f;

	snd = &Sounds[gs->id];

	if ( !(snd->flags & SND_F_USED) )
		return -1;

	if (!ds_initialized)
		return -1;

	if ( volume > MIN_SOUND_VOLUME ) {
		handle = ds_play( snd->sid, snd->hid, gs->id_sig, ds_priority(priority), ds_convert_volume(volume), fl2i(pan*MAX_PAN), 0, is_voice_msg);
	}

	return handle;
}

MONITOR( Num3DSoundsStarted );
MONITOR( Num3DSoundsLoaded );

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
int snd_play_3d(game_snd *gs, vector *source_pos, vector *listen_pos, float radius, vector *source_vel, int looping, float vol_scale, int priority, vector *sound_fvec, float range_factor, int force )
{
	int		handle, min_range, max_range;
	vector	vector_to_sound;
	sound		*snd;
	float		volume, distance, pan, max_volume;

	if ( !Sound_enabled )
		return -1;

	Assert(gs != NULL);

	MONITOR_INC( Num3DSoundsStarted, 1 );

	if ( gs->id == -1 ) {
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

	handle = -1;

	min_range = fl2i( (gs->min + radius) * range_factor);
	max_range = fl2i( (gs->max + radius) * range_factor + 0.5f);

	if (!ds_initialized)
		return -1;
	
	// DirectSound3D will not cut off sounds, no matter how quite they become.. so manually
	// prevent sounds from playing past the max distance.
	distance = vm_vec_normalized_dir_quick( &vector_to_sound, source_pos, listen_pos );
	max_volume = gs->default_volume * vol_scale;
	if ( (distance > max_range) && !force){
		return -1;
	}

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

	volume *= Master_sound_volume;
	if ( (volume < MIN_SOUND_VOLUME) && !force) {
		return -1;
	}

	int play_using_ds3d = 0;

	if (ds_using_ds3d()) {
		if ( ds_is_3d_buffer(snd->sid) ) {
			play_using_ds3d = 1;
		}
	}

	if ( play_using_ds3d ) {
		// play through DirectSound3D
		handle = ds3d_play( snd->sid, snd->hid, gs->id_sig, source_pos, source_vel, min_range, max_range, looping, ds_convert_volume(max_volume*Master_sound_volume), ds_convert_volume(volume), ds_priority(priority));
	}
	else {
		// play sound as a fake 3D sound
		if ( distance <= 0 ) {
			pan = 0.0f;
		}
		else {
			pan = vm_vec_dot(&View_matrix.vec.rvec,&vector_to_sound);
		}
		if(looping){
			handle = snd_play_looping( gs, pan, -1, -1, volume/gs->default_volume, priority, force );
		} else {
			handle = snd_play( gs, pan, volume/gs->default_volume, priority);
		}
	}

	return handle;
}

// update the given 3d sound with a new position
void snd_update_3d_pos(int soundnum, game_snd *gs, vector *new_pos)
{
	float vol, pan;
	
	// get new volume and pan vals
	snd_get_3d_vol_and_pan(gs, new_pos, &vol, &pan);

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
int snd_get_3d_vol_and_pan(game_snd *gs, vector *pos, float* vol, float *pan, float radius)
{
	vector	vector_to_sound;
	float		distance, max_volume;
	sound		*snd;

	*vol = 0.0f;
	*pan = 0.0f;

	if (!ds_initialized)
		return -1;

	Assert(gs != NULL);

	if ( gs->id == -1 ) {
		gs->id = snd_load(gs);
	}
	
	snd = &Sounds[gs->id];
	if ( !(snd->flags & SND_F_USED) )
		return -1;

	distance = vm_vec_normalized_dir_quick( &vector_to_sound, pos, &View_position );
	distance -= radius;

	max_volume = gs->default_volume;
	if ( distance <= gs->min ) {
		*vol = max_volume;
	}
	else {
		*vol = max_volume - (distance - gs->min) * max_volume / (gs->max - gs->min);
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

// ---------------------------------------------------------------------------------------
// volume 0 to 1.0.  Returns the handle of the sound. -1 if failed.
// If startloop or stoploop are not -1, then then are used.
//
//	NOTE: vol_scale parameter is the multiplicative scaling applied to the default volume
//       (vol_scale is a default parameter with a default value of 1.0f)
//
// input:	gs				=>	game-level sound description
//				source_pos	=>	global pos of where the sound is
//				listen_pos	=>	global pos of where listener is
//				source_vel	=>	velocity of the source playing the sound (used for DirectSound3D only)
//				looping		=>	flag to indicate the sound should loop (default value 0)
//				vol_scale	=>	factor to scale the static volume by (applied before attenuation)
//				priority		=> SND_PRIORITY_MUST_PLAY			(default value)
//									SND_PRIORITY_SINGLE_INSTANCE
//									SND_PRIORITY_DOUBLE_INSTANCE
//									SND_PRIORITY_TRIPLE_INSTANCE
//
// returns:		-1		=>		sound could not be played
//					n		=>		handle for instance of sound
//
int snd_play_looping( game_snd *gs, float pan, int start_loop, int stop_loop, float vol_scale, int priority, int force )
{	
	float volume;
	int	handle = -1;
	sound	*snd;	

	if (!Sound_enabled)
		return -1;

	Assert( gs != NULL );

	if (!ds_initialized)
		return -1;

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
	volume *= Master_sound_volume;
	if ( volume > 1.0f )
		volume = 1.0f;

	if ( (volume > MIN_SOUND_VOLUME) || force) {
		handle = ds_play( snd->sid, snd->hid, gs->id_sig, ds_priority(priority), ds_convert_volume(volume), fl2i(pan*MAX_PAN), 1);
	}

	return handle;
}

// ---------------------------------------------------------------------------------------
// snd_stop()
//
// Stop a sound from playing.
//
// parameters:		sig => handle to sound, what is returned from snd_play()
//
void snd_stop( int sig )
{
	int channel;

	if (!ds_initialized) return;
	if ( sig < 0 ) return;

	channel = ds_get_channel(sig);
	if ( channel == -1 )
		return;
	
	ds_stop_channel(channel);
}

// ---------------------------------------------------------------------------------------
// snd_set_volume()
//
// Set the volume of a currently playing sound
//
// parameters:		sig		=> handle to sound, what is returned from snd_play()
//						volume	=> volume of sound (range: 0.0 -> 1.0)
//
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

	new_volume = volume * Master_sound_volume;
	ds_set_volume( channel, ds_convert_volume(new_volume) );
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

	ds_set_pan( channel, fl2i(pan*MAX_PAN) );
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
// snd_chg_loop_status()
//
// Change whether a currently playing song is looping or not
//
// parameters:		sig			=> handle to sound, what is returned from snd_play()
//						loop			=> whether to start (1) or stop (0) looping
//
void snd_chg_loop_status(int sig, int loop)
{
	int channel;

	if (!ds_initialized)
		return;

	if ( sig < 0 )
		return;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to change loop status of a non-playing sound!\n" ));
		return;
	}

	ds_chg_loop_status(channel, loop);
}

// ---------------------------------------------------------------------------------------
// snd_stop_all()
//
// Stop all playing sound channels (including looping sounds)
//
// NOTE: This stops all sounds that are playing from Channels[] sound buffers.  It doesn't
//			stop every secondary sound buffer in existance
//
void snd_stop_all()
{
	if (!ds_initialized)
		return;

	ds_stop_channel_all();
}

// ---------------------------------------------------------------------------------------
// sound_get_ds()
//
// Return the pointer to the DirectSound interface
//
//
uint sound_get_ds()
{
	return (uint)pDirectSound;
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

	return Sounds[snd_id].duration;
}


MONITOR( SoundChannels );

// update the position of the listener for the specific 3D sound API we're 
// using
void snd_update_listener(vector *pos, vector *vel, matrix *orient)
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
	if(!snd_is_playing(snd_handle))
		return;

	float current_time,desired_time;
	float bps;
	DWORD current_offset,desired_offset;
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
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
	if(!snd_is_playing(snd_handle))
		return;

	sound_info *snd;

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
	Assert(handle >= 0 && handle < MAX_SOUNDS);
	if ( ds_get_data(Sounds[handle].sid, data) ) {
		return -1;
	}

	return 0;
}

// return the size of the sound data associated with the sound handle
int snd_size(int handle, int *size)
{
	Assert(handle >= 0 && handle < MAX_SOUNDS);
	if ( ds_get_size(Sounds[handle].sid, size) ) {
		return -1;
	}

	return 0;
}

// retrieve the bits per sample and frequency for a given sound
void snd_get_format(int handle, int *bits_per_sample, int *frequency)
{
	Assert(handle >= 0 && handle < MAX_SOUNDS);
	*bits_per_sample = Sounds[handle].info.bits;
	*frequency = Sounds[handle].info.sample_rate;
}

// return the time for the sound to play in milliseconds
int snd_time_remaining(int handle, int bits_per_sample, int frequency)
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

	int current_offset, max_offset;

	current_offset = ds_get_play_position(channel);
	max_offset = ds_get_channel_size(channel);

	if ( current_offset < max_offset ) {
		int bytes_remaining = max_offset - current_offset;
		int samples_remaining = bytes_remaining / fl2i(bits_per_sample/8.0f);
		time_remaining = fl2i(1000 * samples_remaining/frequency + 0.5f);
	}	

//	mprintf(("time_remaining: %d\n", time_remaining));	
	return time_remaining;
}


// snd_env_ interface

static unsigned long Sound_env_id;
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
int sound_env_get(sound_env *se)
{
	EAX_REVERBPROPERTIES er;

	if (ds_eax_get_all(&er) == 0) {
		se->id = er.environment;
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
	se.id = SND_ENV_GENERIC;
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
void snd_do_frame()
{
	ds_do_frame();
}
