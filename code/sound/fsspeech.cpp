/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 





#ifdef FS2_SPEECH


#ifdef _WIN32
#include <windows.h>
#endif

#include "sound/fsspeech.h"
#include "sound/speech.h"
#include "osapi/osregistry.h"
#include "globalincs/pstypes.h"


extern int Cmdline_freespace_no_sound;

const int MAX_SPEECH_BUFFER_LEN = 4096;

static int speech_inited = 0;

bool FSSpeech_play_from[FSSPEECH_FROM_MAX];
char *FSSpeech_play_id[FSSPEECH_FROM_MAX] =
{
	"SpeechTechroom",
	"SpeechBriefings",
	"SpeechIngame",
	"SpeechMulti"
};

char Speech_buffer[MAX_SPEECH_BUFFER_LEN] = "";
int  Speech_buffer_len;

bool fsspeech_init()
{
	if (speech_inited) {
		return true;
	}

	// if sound is disabled from the cmdline line then don't do speech either
	if (Cmdline_freespace_no_sound) {
		return false;
	}

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

	speech_inited = 1;

	return true;
}

void fsspeech_deinit()
{
	if (!speech_inited)
		return;

	speech_deinit();

	speech_inited = 0;
}

void fsspeech_play(int type, const char *text)
{
	if (!speech_inited)
		return;

	if(type >= FSSPEECH_FROM_MAX) return;

	if(type >= 0 && FSSpeech_play_from[type] == false) return;

	speech_play(text);
}

void fsspeech_stop()
{
	if (!speech_inited)
		return;

	speech_stop();
}

void fsspeech_pause(bool playing)
{
	if (!speech_inited)
		return;

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

void fsspeech_stuff_buffer(const char *text)
{
	if (!speech_inited)
		return;

	int len = strlen(text);

	if(Speech_buffer_len + len < MAX_SPEECH_BUFFER_LEN) {
		strcat_s(Speech_buffer, text);
	}

	Speech_buffer_len += len;
}

void fsspeech_play_buffer(int type)
{
	if (!speech_inited)
		return;

	fsspeech_play(type, Speech_buffer);
}

// Goober5000
bool fsspeech_play_from(int type)
{
	Assert(type >= 0 && type < FSSPEECH_FROM_MAX);

	return (speech_inited && FSSpeech_play_from[type]);
}

// Goober5000
bool fsspeech_playing()
{
	if (!speech_inited)
		return false;

	return speech_is_speaking();
}

#endif	// FS2_SPEECH defined
