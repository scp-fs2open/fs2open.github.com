/*
 * Code created by Thomas Whittaker (RT) for a Freespace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#ifdef LAUNCHER
#include "stdafx.h"
#endif

#include <windows.h>
#include <atlbase.h>

#if FS2_SPEECH
#include <sapi.h>
#include <sphelper.h>
#else
#if NDEBUG
#pragma message( "WARNING: You have not compiled speech into this build (use FS2_SPEECH)" )
#endif
#endif

#include "speech.h"

#if FS2_SPEECH
ISpVoice *Voice_device;
bool Speech_init = false;
unsigned short Conversion_buffer[MAX_SPEECH_CHAR_LEN];

bool speech_init()
{
    HRESULT hr = CoCreateInstance(
		CLSID_SpVoice, 
		NULL, 
		CLSCTX_ALL, 
		IID_ISpVoice, 
		(void **)&Voice_device);

	Speech_init = SUCCEEDED(hr);

	return Speech_init;
}

void speech_deinit()
{
	if(Speech_init == false) return;

	Voice_device->Release();
}

bool speech_play(char *text)
{
	if(Speech_init == false) return true;
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
	if(Speech_init == false) return true;
	if(text == NULL) return false;

	speech_stop();
  	return SUCCEEDED(Voice_device->Speak(text, SPF_ASYNC, NULL));
}

bool speech_pause()
{
	if(Speech_init == false) return true;
	return SUCCEEDED(Voice_device->Pause());
}

bool speech_resume()
{
	if(Speech_init == false) return true;
	return SUCCEEDED(Voice_device->Resume());
}

bool speech_stop()
{
	if(Speech_init == false) return true;
    return SUCCEEDED(Voice_device->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL ));
}

bool speech_set_volume(int volume)
{
    return SUCCEEDED(Voice_device->SetVolume(volume));
}

bool speech_set_voice(int voice)
{
	HRESULT                             hr;
	CComPtr<ISpObjectToken>             cpVoiceToken;
	CComPtr<IEnumSpObjectTokens>        cpEnum;
	ULONG                               num_voices = 0;

	//Enumerate the available voices 
	hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);

	if(FAILED(hr)) return false;

    hr = cpEnum->GetCount(&num_voices);

	if(FAILED(hr)) return false;

	int count = 0;
	// Obtain a list of available voice tokens, set the voice to the token, and call Speak
	while (num_voices -- )
	{
		cpVoiceToken.Release();
		
		hr = cpEnum->Next( 1, &cpVoiceToken, NULL );

		if(FAILED(hr)) {
			return false;
		}

		if(count == voice) {
			return SUCCEEDED(Voice_device->SetVoice(cpVoiceToken));
		}

		count++;
	}
	return false;
}

#else

// Stubs

// Goober5000: see, the *real* way to do stubs (avoiding the warnings)
// is to just use #defines (c.f. NO_SOUND)
#define speech_init() (false)
#define speech_deinit()
#define speech_play(text) ((void) text)
#define speech_play(text) ((void) text)
#define speech_pause() (false)
#define speech_resume() (false)
#define speech_stop() (false)
#define speech_set_volume(volume) ((void) volume)
#define speech_set_voice(voice) ((void voice)

#endif