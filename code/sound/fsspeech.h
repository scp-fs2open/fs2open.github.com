/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include "PreProcDefines.h"
#ifndef _FSSPEECH_H_
#define _FSSPEECH_H_

enum
{
	FSSPEECH_FROM_TECHROOM,
	FSSPEECH_FROM_BRIEFING,
	FSSPEECH_FROM_INGAME,
	FSSPEECH_FROM_MAX
};

#ifdef FS2_SPEECH

bool fsspeech_init();
void fsspeech_deinit();
void fsspeech_play(int type, char *text);
void fsspeech_stop();
void fsspeech_pause(bool playing);

void fsspeech_start_buffer();
void fsspeech_stuff_buffer(char *text);
void fsspeech_play_buffer(int type);

bool fsspeech_play_from(int type);
bool fsspeech_playing();

#define fsspeech_was_compiled()	(true)

#else

// stub functions (c.f. NO_SOUND)
#define fsspeech_init()	(false)
#define fsspeech_deinit()
#define fsspeech_play(type, text) ((void) ((type), (text), false))
#define fsspeech_stop()
#define fsspeech_pause(playing) ((void) ((playing), false))
#define fsspeech_start_buffer()
#define fsspeech_stuff_buffer(text) ((void) ((text), false))
#define fsspeech_play_buffer(type) ((void) ((type), false))
#define fsspeech_play_from(type) ((type), false)
#define fsspeech_playing() (false)
#define fsspeech_was_compiled() (false)

#endif

#endif	// header define
