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
 * $Revision: 2.1 $
 * $Date: 2004-08-11 05:06:34 $
 * $Author: Kazan $
 *
 * Routines to stream large WAV files from disk
 *
 * $Log: not supported by cvs2svn $
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

#include "PreProcDefines.h"
#ifndef _AUDIOSTR_H
#define _AUDIOSTR_H

// type of audio stream
#define ASF_SOUNDFX			0
#define ASF_EVENTMUSIC		1
#define ASF_VOICE				2
#define ASF_NONE				3		// used to catch errors


#ifndef NO_SOUND
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

// set the number of bytes that the sound should cutoff after
void audiostream_set_byte_cutoff(int i, unsigned int cutoff);

// return the number of bytes streamed to the Direct Sound buffer so far
unsigned int audiostream_get_bytes_committed(int i);

// check if the streaming has read all the bytes from disk yet
int audiostream_done_reading(int i);

// return if audiostream has initialized ok
int audiostream_is_inited();

void audiostream_pause(int i);	// pause a particular stream
void audiostream_pause_all();	// pause all audio streams											

void audiostream_unpause(int i);	// unpause a particular stream
void audiostream_unpause_all();	// unpause all audio streams

#else

#define audiostream_init()
#define audiostream_close()
#define audiostream_open(filename, type)          ((filename), (type), 0)
#define audiostream_close_file(i, fade)           ((void)((i), (fade)))
#define audiostream_close_all(fade)               ((void)(fade))
#define audiostream_play(i, volume, looping)      ((void)((i), (volume), (looping)))
#define audiostream_is_playing(i)                 ((i), 0)
#define audiostream_stop(i, rewind, paused)       ((void)((i), (rewind), (paused)))
#define audiostream_set_volume_all(volume, type)  ((void)((volume), (type)))
#define audiostream_set_volume(i, volume)         ((void)((i), (volume)))
#define audiostream_is_paused(i)                  ((i), 0)
#define audiostream_set_byte_cutoff(i, cutoff)    ((void)((i), (cutoff)))
#define audiostream_get_bytes_committed(i)        ((i), 0)
#define audiostream_done_reading(i)               ((i), 1)
#define audiostream_is_inited()                   (1)
#define audiostream_pause(i)                      ((void)(i))
#define audiostream_pause_all()
#define audiostream_unpause(i)                    ((void)(i))
#define audiostream_unpause_all()

#endif // ifndef NO_SOUND

#endif // _AUDIOSTR_H
