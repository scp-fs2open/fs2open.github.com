//#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "globalincs/pstypes.h"

#if FS2_SPEECH
#include <sapi.h>
#endif

#include "sound/speech.h"

#if FS2_SPEECH
ISpVoice *Voice_device;
#endif

bool Speech_init = false;

unsigned short Conversion_buffer[MAX_SPEECH_CHAR_LEN];

bool speech_init()
{
#ifndef FS2_SPEECH
	return false;
#else
    Speech_init = SUCCEEDED(CoCreateInstance(
		CLSID_SpVoice, 
		NULL, 
		CLSCTX_ALL, 
		IID_ISpVoice, 
		(void **)&Voice_device));

	return Speech_init;
#endif
}

void speech_deinit()
{
	if(Speech_init == false) return;

#ifdef FS2_SPEECH
	Voice_device->Release();
#endif
}

bool speech_play(char *text)
{
#ifndef FS2_SPEECH
	return false;
#endif

	if(Speech_init == false) {
		mprintf(("trying to play speech but it is not inited"));
		return true;
	}
	else
		mprintf(("Should be speaking now"));

	if(text == NULL) return false;

	int len = strlen(text);

	if(len > (MAX_SPEECH_CHAR_LEN - 1)) {
		len = MAX_SPEECH_CHAR_LEN - 1;
	}

	for(int i = 0; i < len; i++) {
		Conversion_buffer[i] = (unsigned short) text[i];
	}

	Conversion_buffer[i] = '\0';

	return speech_play(Conversion_buffer);
}

bool speech_play(unsigned short *text)
{
#ifndef FS2_SPEECH
	return false;
#else
	if(Speech_init == false) {
		mprintf(("trying to play speech but it is not inited"));
		return true;
	}
	if(text == NULL) return false;

	speech_stop();
  	return SUCCEEDED(Voice_device->Speak(text, SPF_ASYNC, NULL));
#endif
}

bool speech_pause()
{
#ifndef FS2_SPEECH
	return false;
#else
	if(Speech_init == false) return true;
	return SUCCEEDED(Voice_device->Pause());
#endif
}

bool speech_resume()
{
#ifndef FS2_SPEECH
	return false;
#else
	if(Speech_init == false) return true;
	return SUCCEEDED(Voice_device->Resume());
#endif
}

bool speech_stop()
{
#ifndef FS2_SPEECH
	return false;
#else
	if(Speech_init == false) return true;
    return SUCCEEDED(Voice_device->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL ));
#endif
}

