/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/sound/fsspeech.cpp $
 * $Revision: 1.16 $
 * $Date: 2005-06-30 01:35:52 $
 * $Author: Goober5000 $
 *
 * This module contains freespace specific stuff leaving the speech module to handle generic stuff.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2005/06/24 19:36:49  taylor
 * we only want to have m_data_offset be 0 for oggs since the seeking callback will account for the true offset
 * only extern the one int we need for the -nosound speech fix rather than including the entire header
 *
 * Revision 1.14  2005/06/20 15:54:58  phreak
 * added cmdline.h include so the compiler would see the no sound command line option
 * also added CVS header information.
 *
 *
 *
 * $NoKeywords: $
 */


#include "PreProcDefines.h"

#ifdef FS2_SPEECH


#ifdef _WIN32
#include <windows.h>
#endif

#include "sound/fsspeech.h"
#include "sound/speech.h"
#include "osapi/osregistry.h"
#include "debugconsole/dbugfile.h"
#include "globalincs/pstypes.h"


extern int Cmdline_freespace_no_sound;

#pragma warning(disable:4711)	// function selected for inlining

const int MAX_SPEECH_BUFFER_LEN = 4096;

static int speech_inited = 0;

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

void fsspeech_play(int type, char *text)
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

void fsspeech_stuff_buffer(char *text)
{
	if (!speech_inited)
		return;

	int len = strlen(text);

	if(Speech_buffer_len + len < MAX_SPEECH_BUFFER_LEN) {
		strcat(Speech_buffer, text);
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

#endif	// FS2_SPEECH defined
