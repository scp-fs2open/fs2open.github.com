/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#ifndef _SPEECH_H_
#define _SPEECH_H_

#include "globalincs/pstypes.h"

#if FS2_SPEECH

bool speech_init();
void speech_deinit();
bool speech_play(const char *text);
bool speech_pause();
bool speech_resume();
bool speech_stop();

bool speech_set_volume(unsigned short volume);
bool speech_set_voice(int voice);

bool speech_is_speaking();

SCP_vector<SCP_string> speech_enumerate_voices();

#else

// Goober5000: see, the *real* way to do stubs (avoiding the warnings)
// is to just use #defines (c.f. NO_SOUND)
#define speech_init() (false)
#define speech_deinit()
#define speech_play(text) ((text), false)
#define speech_pause() (false)
#define speech_resume() (false)
#define speech_stop() (false)
#define speech_set_volume(volume) ((volume), false)
#define speech_set_voice(voice) ((voice), false)
#define speech_is_speaking() (false)

SCP_vector<SCP_string> speech_enumerate_voices() {
	return SCP_vector<SCP_string>();
}

#endif

#endif
