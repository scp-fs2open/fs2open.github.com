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

#define SPCAT_VOICES_ONECORE           L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech_OneCore\\Voices"

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

inline bool speech_init() { return false; }
inline void speech_deinit() {}
inline bool speech_play(const char* /*text*/) { return false; }
inline bool speech_pause() { return false; }
inline bool speech_resume() { return false; }
inline bool speech_stop() { return false; }
inline bool speech_set_volume(unsigned short /*volume*/) { return false; }
inline bool speech_set_voice(int /*voice*/) { return false; }
inline bool speech_is_speaking() { return false; }

inline SCP_vector<SCP_string> speech_enumerate_voices() {
	return SCP_vector<SCP_string>();
}

#endif

#endif
