/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include <windows.h>
	
/*
 * This stuff is for voice select which we are not ready for yet
 *
#if FS2_SPEECH

// Have to do this to let sphelper.h compile OK
#undef Assert

#include <atlbase.h>
#include <sapi.h>
#include <sphelper.h>

#if defined(NDEBUG)
#define Assert(x) do {} while (0)
#else
void gr_activate(int);
#define Assert(x) do { if (!(x)){ gr_activate(0); WinAssert(#x,__FILE__,__LINE__); gr_activate(1); } } while (0)
#endif

#endif
  */
  

#include "sound/speech.h"
#include "osapi/osregistry.h"

#include "debugconsole/dbugfile.h"
#include "sound/fsspeech.h"

#pragma warning(disable:4711)

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
	speech_set_volume(volume);

	return true;
}

void fsspeech_deinit()
{
	speech_deinit();
}

void fsspeech_play(int type, char *text)
{
	if(type >= FSSPEECH_FROM_MAX) return;
	if(FSSpeech_play_from[type] == false) return;

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
