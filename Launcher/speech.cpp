/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include "stdafx.h"

#include <windows.h>
#include <atlbase.h>

#ifdef FS2_SPEECH
#include <sapi.h>
#else
#pragma message("NOTE: Speech is not compiled in")
#endif

#include "speech.h"

#ifdef FS2_SPEECH
ISpVoice *Voice_device;
#endif

bool Speech_init = false;
										   
unsigned short Conversion_buffer[MAX_SPEECH_CHAR_LEN];

bool speech_init()
{
#ifndef FS2_SPEECH
	return false;
#else
    HRESULT hr = CoCreateInstance(
		CLSID_SpVoice, 
		NULL, 
		CLSCTX_ALL, 
		IID_ISpVoice, 
		(void **)&Voice_device);

	Speech_init = SUCCEEDED(hr);

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
#else
	if(Speech_init == false) return true;
	if(text == NULL) return false;

	int len = strlen(text);

	if(len > (MAX_SPEECH_CHAR_LEN - 1)) {
		len = MAX_SPEECH_CHAR_LEN - 1;
	}

	int i;
	for(i = 0; i < len; i++) {
		Conversion_buffer[i] = (unsigned short) text[i];
	}

	Conversion_buffer[i] = '\0';

	return speech_play(Conversion_buffer);
#endif
}

bool speech_play(unsigned short *text)
{
#ifndef FS2_SPEECH
	return false;
#else
	if(Speech_init == false) return true;
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

bool speech_set_volume(int volume)
{
#ifndef FS2_SPEECH
	return false;
#else
    return SUCCEEDED(Voice_device->SetVolume(volume));
#endif
}

bool speech_set_voice(void *new_voice)
{
#ifdef FS2_SPEECH
    return SUCCEEDED(Voice_device->SetVoice( (ISpObjectToken *) new_voice ));
#endif
	return false;
}
