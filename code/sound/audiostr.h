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

extern const char *audio_ext_list[];
extern const int NUM_AUDIO_EXT;

// Initializes the audio streaming library.  Called
// automatically when the sound stuff is inited.
void audiostream_init();

// Closes down the audio streaming library
void audiostream_close();

// Opens a wave file but doesn't play it.
int audiostream_open( const char * filename, int type );

// Opens wave file contents previously loaded into memory but doesn't play them.
int audiostream_open_mem( const uint8_t* snddata, size_t snd_len, int type );

// Closes the opened wave file.  This doesn't have to be
// called between songs, because when you open the next
// song, it will call this internally.
void audiostream_close_file(int i, bool fade = true);

void audiostream_close_all(bool fade);

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

// get duration of a stream in milliseconds
double audiostream_get_duration(int i);

// set the number of samples that the sound should cutoff after
void audiostream_set_sample_cutoff(int i, unsigned int cutoff);

// return the number of samples streamed to the Direct Sound buffer so far
unsigned int audiostream_get_samples_committed(int i);

// check if the streaming has read all the bytes from disk yet
int audiostream_done_reading(int i);

// return if audiostream has initialized ok
int audiostream_is_inited();

void audiostream_pause(int i, bool via_sexp_or_script = false);	// pause a particular stream
void audiostream_unpause(int i, bool via_sexp_or_script = false);	// unpause a particular stream

void audiostream_pause_all(bool via_sexp_or_script = false);	// pause all audio streams											
void audiostream_unpause_all(bool via_sexp_or_script = false);	// unpause all audio streams

#endif // _AUDIOSTR_H
