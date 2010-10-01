/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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

#define MAX_AUDIO_STREAMS	30

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
