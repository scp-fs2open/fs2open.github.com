/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#ifndef _SPEECH_H_
#define _SPEECH_H_

const int MAX_SPEECH_CHAR_LEN = 10000;

bool speech_init();
void speech_deinit();
bool speech_play(char *text);
bool speech_play(unsigned short *text);
bool speech_pause();
bool speech_resume();
bool speech_stop();

bool speech_set_volume(int volume);
bool speech_set_voice(void *new_voice);

#endif