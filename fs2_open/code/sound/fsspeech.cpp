/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

// This module contains freespace specific stuff leaving the speech module to handle generic stuff.

#include "sound/fsspeech.h"

#ifdef FS2_SPEECH

#include <windows.h>  
#include "sound/speech.h"
#include "osapi/osregistry.h"
#include "debugconsole/dbugfile.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

#pragma warning(disable:4711)	// function selected for inlining

const int MAX_SPEECH_BUFFER_LEN = 4096;

bool FSSpeech_play_from[FSSPEECH_FROM_MAX];
char *FSSpeech_play_id[FSSPEECH_FROM_MAX] =
{
	"SpeechTechroom",
	"SpeechBriefings",
	"SpeechIngame"
};

char Speech_buffer[MAX_SPEECH_BUFFER_LEN] = "";
int  Speech_buffer_len;

bool fsspeech_init()
{
	if(speech_init() == false) {
		return false;
	}

	// Get the settings from the registry
	for(int i = 0; i < FSSPEECH_FROM_MAX; i++) {
		FSSpeech_play_from[i] =
			os_config_read_uint(NULL, FSSpeech_play_id[i], 0) ? true : false;
	}

	int volume = os_config_read_uint(NULL, "SpeechVolume", 100);
	speech_set_volume((unsigned short) volume);

	int voice = os_config_read_uint(NULL, "SpeechVoice", 0);
	speech_set_voice(voice);

	return true;
}

void fsspeech_deinit()
{
	speech_deinit();
}

void fsspeech_play(int type, char *text)
{
	if(type >= FSSPEECH_FROM_MAX) return;

	if(type >= 0 && FSSpeech_play_from[type] == false) return;

	speech_play(text);
}

void fsspeech_stop()
{
	speech_stop();
}

void fsspeech_pause(bool playing)
{
	if(playing) {
		speech_pause();
	} else {
		speech_resume();
	}
}

void fsspeech_start_buffer()
{
	Speech_buffer_len = 0;
	Speech_buffer[0] = '\0';
}

void fsspeech_stuff_buffer(char *text)
{
	int len = strlen(text);

	if(Speech_buffer_len + len < MAX_SPEECH_BUFFER_LEN) {
		strcat(Speech_buffer, text);
	}

	Speech_buffer_len += len;
}

void fsspeech_play_buffer(int type)
{
	fsspeech_play(type, Speech_buffer);
}

#endif	// FS2_SPEECH defined
