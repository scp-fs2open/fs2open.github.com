/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Sound/AudioStr.h $
 * $Revision: 2.5.2.2 $
 * $Date: 2007-03-22 20:35:45 $
 * $Author: taylor $
 *
 * Routines to stream large WAV files from disk
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.5.2.1  2007/02/10 00:17:40  taylor
 * remove NO_SOUND
 *
 * Revision 2.5  2005/07/13 03:35:29  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.4  2005/06/19 04:59:04  taylor
 * woorps!
 *
 * Revision 2.3  2005/06/19 02:45:55  taylor
 * OGG streaming fixes to get data reading right and avoid skipping
 * properly handle seeking in OGG streams
 * compiler warning fix in OpenAL builds
 *
 * Revision 2.2  2005/01/18 01:14:17  wmcoolmon
 * OGG fixes, ship selection fixes
 *
 * Revision 2.1  2004/08/11 05:06:34  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/24 15:38:55  mharris
 * Fixed boneheaded mistake in #defines for NO_JOYSTICK and NO_SOUND
 *
 * Revision 1.2  2002/05/09 22:56:41  mharris
 * audiostream functions are do-nothing macros if NO_SOUND is defined
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 9     5/24/98 4:42p Dan
 * AL: Fix several bugs related to pausing and enabling/disabling event
 * music
 * 
 * 8     3/31/98 4:50p Dan
 * AL: Clean up all audio streams if necessary in
 * event_music_level_close()
 * 
 * 7     12/10/97 10:04p Lawrance
 * modify what happens in Audio_stream constructor
 * 
 * 6     12/09/97 6:14p Lawrance
 * add -nomusic flag
 * 
 * 5     11/20/97 1:06a Lawrance
 * Add Master_voice_volume, make voices play back at correctly scaled
 * volumes
 * 
 * 4     10/03/97 8:24a Lawrance
 * When unpausing, be sure to retain looping status
 * 
 * 3     9/18/97 10:31p Lawrance
 * add functions to pause and unpause all audio streams
 * 
 * 2     6/04/97 1:19p Lawrance
 * added function to check if system is initialized
 * 
 * 1     4/28/97 4:45p John
 * Initial version of ripping sound & movie out of OsAPI.
 * 
 * 8     4/14/97 1:52p Lawrance
 * making transitions happen on measure boundries
 * 
 * 7     4/09/97 11:14a Lawrance
 * working on event music transitions
 * 
 * 6     4/07/97 3:15p Lawrance
 * allowing event music to pause
 * 
 * 5     4/03/97 4:27p Lawrance
 * expanding functionality to support event driven music
 * 
 * 4     4/01/97 1:31p Lawrance
 * make music fade quickly out when stopping.  Delay onset of new music to
 * allow old music to fade out.
 * 
 * 3     3/31/97 5:45p Lawrance
 * supporting changes to allow multiple streamed audio files
 * 
 * 2     3/31/97 3:56p Lawrance
 * decompress ADPCM->PCM for streaming sounds working
 * 
 * 1     1/22/97 10:43a John
 *
 * $NoKeywords: $
 */

#ifndef _AUDIOSTR_H
#define _AUDIOSTR_H



// type of audio stream
#define ASF_SOUNDFX			0
#define ASF_EVENTMUSIC		1
#define ASF_MENUMUSIC		2
#define ASF_VOICE			3
#define ASF_NONE			4		// used to catch errors
// NOTE: the only difference between EVENTMUSIC and everything else is that EVENTMUSIC
//       will always respect the file type, everything else will load first available type


#ifdef NEED_STRHDL
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#endif

#include "sound/ogg/ogg.h"

// audio stream file handle information
typedef struct {
	HMMIO cfp;		// handle for mmio

	long true_offset;	// true offset of file into VP
	uint size;			// total size of file being read

	// for OGGs
	OggVorbis_File vorbis_file;	// vorbis file info
} STRHDL;
#endif	// NEED_STRHDL

// Initializes the audio streaming library.  Called
// automatically when the sound stuff is inited.
void audiostream_init();

// Closes down the audio streaming library
void audiostream_close();

// Opens a wave file but doesn't play it.
int audiostream_open( char * filename, int type );

// Closes the opened wave file.  This doesn't have to be
// called between songs, because when you open the next
// song, it will call this internally.
void audiostream_close_file(int i, int fade = 1);

void audiostream_close_all(int fade);

// Plays the currently opened wave file
void audiostream_play(int i, float volume = -1.0f, int looping = 1);

// See if a particular stream is playing
int audiostream_is_playing(int i);

// Stops the currently opened wave file
void audiostream_stop(int i, int rewind = 1, int paused = 0);

// set the volume for every audio stream of a particular type
void audiostream_set_volume_all(float volume, int type);

// set the volume for a particular audio stream
void audiostream_set_volume(int i, float volume);

// see if a particular stream is paused
int audiostream_is_paused(int i);

// set the number of samples that the sound should cutoff after
void audiostream_set_sample_cutoff(int i, unsigned int cutoff);

// return the number of samples streamed to the Direct Sound buffer so far
unsigned int audiostream_get_samples_committed(int i);

// check if the streaming has read all the bytes from disk yet
int audiostream_done_reading(int i);

// return if audiostream has initialized ok
int audiostream_is_inited();

void audiostream_pause(int i);	// pause a particular stream
void audiostream_pause_all();	// pause all audio streams											

void audiostream_unpause(int i);	// unpause a particular stream
void audiostream_unpause_all();	// unpause all audio streams

#endif // _AUDIOSTR_H
