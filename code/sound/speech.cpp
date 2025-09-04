/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 





#ifndef FS2_SPEECH
#if defined(_WIN32) || defined(__APPLE__)
#if NDEBUG
	#pragma message( "WARNING: You have not compiled speech into this build (use FS2_SPEECH)" )
#endif // NDEBUG
#endif // _WIN32 or __APPLE__
#elif !defined(__APPLE__) // to end-of-file ...


#ifdef LAUNCHER
#include "stdafx.h"
#endif	//LAUNCHER

#ifdef _WIN32

// Since we define these ourself we need to undefine them for the sapi header
#pragma push_macro("strcpy_s")
#pragma push_macro("strncpy_s")
#pragma push_macro("strcat_s")
#pragma push_macro("memset")
#pragma push_macro("memcpy")
#undef strcpy_s
#undef strncpy_s
#undef strcat_s
#undef memset
#undef memcpy

	#include <windows.h>
	#include <sapi.h>

	#include <sphelper.h>

#pragma pushpop_macro("strcpy_s")
#pragma pushpop_macro("strncpy_s")
#pragma pushpop_macro("strcat_s")
#pragma pushpop_macro("memset")
#pragma pushpop_macro("memcpy")

	ISpVoice *Voice_device;
#elif defined(SCP_UNIX)
	#include <fcntl.h>
//	#include <stdio.h>

	int speech_dev = -1;
//	FILE *speech_dev = NULL;
#else 
	#pragma error( "ERROR: Unknown platform, speech (FS2_SPEECH) is not supported" )
#endif	//_WIN32

#pragma warning(push)
#pragma warning(disable: 4995)
// Visual Studio complains that some functions are deprecated so this fixes that
#include <cstring>
#include <cwchar>
#include <cstdio>
#pragma warning(pop)

#include "globalincs/pstypes.h"
#include "utils/unicode.h"
#include "speech.h"

#ifdef __ANDROID__
#include <jni.h>
#include "SDL.h"
#include "SDL_system.h"

static JNIEnv* env = nullptr;
static jclass tts_manager = nullptr;
static jmethodID tts_speak = nullptr, tts_stop = nullptr, tts_pause = nullptr, tts_resume = nullptr, tts_isSpeaking = nullptr, tts_shutdown = nullptr;

// Ask SDL for the JniEnviroment and get the name and path 
// of the external TTSManager on the android side of things
// Then assign all methods ids for later use
bool android_tts_init()
{
	mprintf(("Speech : Init JNI TTSManager...\n"));
	env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	if (env == nullptr) {
		mprintf(("Speech : Unable to get JNI environment!\n"));
		return false;
	}
	tts_manager = env->FindClass("com/shivansps/fsowrapper/tts/TTSManager");
	if (tts_manager == nullptr) {
		mprintf(("Speech : Unable to find the TTSManager class!\n"));
		return false;
	}

	tts_speak = env->GetStaticMethodID(tts_manager, "speak", "(Ljava/lang/String;)Z");
	tts_stop = env->GetStaticMethodID(tts_manager, "stop", "()Z");
	tts_pause = env->GetStaticMethodID(tts_manager, "pause", "()Z");
	tts_resume = env->GetStaticMethodID(tts_manager, "resume", "()Z");
	tts_isSpeaking = env->GetStaticMethodID(tts_manager, "isSpeaking", "()Z");
	tts_shutdown = env->GetStaticMethodID(tts_manager, "shutdown", "()V");

	if (!tts_speak || !tts_stop || !tts_pause || !tts_resume || !tts_isSpeaking || !tts_shutdown) {
		mprintf(("Speech : Unable to map at least one TTS method!\n"));
		return false;
	}
	mprintf(("Speech : Init Completed!\n"));
	return true;
}


bool android_tts_play(const char* text) 
{
	if(env == nullptr || tts_manager == nullptr || tts_speak == nullptr)
		return false;
	jstring j_txt = env->NewStringUTF(text != nullptr ? text : "");
	mprintf(("Speech : Playing TTS string: %s!\n",text ));
	jboolean ok = env->CallStaticBooleanMethod(tts_manager, tts_speak, j_txt);
	env->DeleteLocalRef(j_txt);
	if (ok != JNI_TRUE) {
		mprintf(("Speech : Error playing TTS string!\n"));
		return false;
	}
	return true;
}

bool android_tts_stop() 
{ 
	return env->CallStaticBooleanMethod(tts_manager, tts_stop) == JNI_TRUE;
}

bool android_tts_pause() 
{ 
	return env->CallStaticBooleanMethod(tts_manager, tts_pause) == JNI_TRUE;
}

bool android_tts_resume() 
{ 
	return env->CallStaticBooleanMethod(tts_manager, tts_resume) == JNI_TRUE;
}

bool android_tts_is_speaking() 
{ 
	return env->CallStaticBooleanMethod(tts_manager, tts_isSpeaking) == JNI_TRUE;
}

void android_tts_deinit() 
{ 
	env->CallStaticVoidMethod(tts_manager, tts_shutdown); 
}
#endif


bool Speech_init = false;

bool speech_init()
{
#ifdef _WIN32
    HRESULT hr = CoCreateInstance(
		CLSID_SpVoice, 
		NULL, 
		CLSCTX_ALL, 
		IID_ISpVoice, 
		(void **)&Voice_device);

	Speech_init = SUCCEEDED(hr);
#elif defined(__ANDROID__)
	Speech_init = android_tts_init();
#else
	speech_dev = open("/dev/speech", O_WRONLY | O_DIRECT);
//	speech_dev = fopen("/dev/speech", "w");

	if (speech_dev == -1) {
//	if (speech_dev == NULL) {
		mprintf(("Couldn't open '/dev/speech', turning text-to-speech off...\n"));
		return false;
	}

	Speech_init = true;
#endif

	nprintf(("Speech", "Speech init %s\n", Speech_init ? "succeeded!" : "failed!"));
	return Speech_init;
}

void speech_deinit()
{
	if(Speech_init == false) return;

#ifdef _WIN32
	Voice_device->Release();
#elif defined(__ANDROID__)
	android_tts_deinit();
#else
	close(speech_dev);
//	fclose(speech_dev);
#endif
}

bool speech_play(const char *text)
{
	nprintf(("Speech", "Attempting to play speech string %s...\n", text));

	if(Speech_init == false) return true;
	if (text == NULL) {
		nprintf(("Speech", "Not playing speech because passed text is null.\n"));
		return false;
	}

#ifdef _WIN32
	SCP_string work_buffer;

	bool saw_dollar = false;
	for (auto ch : unicode::codepoint_range(text)) {
		if (ch == UNICODE_CHAR('$')) {
			// Skip $ escape sequences which appear in briefing text
			saw_dollar = true;
			continue;
		} else if (saw_dollar) {
			saw_dollar = false;
			continue;
		}

		unicode::encode(ch, std::back_inserter(work_buffer));
	}

	// Determine the needed amount of data
	auto num_chars = MultiByteToWideChar(CP_UTF8, 0, work_buffer.c_str(), (int) work_buffer.size(), nullptr, 0);

	if (num_chars <= 0) {
		// Error
		return false;
	}

	std::wstring wide_string;
	wide_string.resize(num_chars);

	auto err = MultiByteToWideChar(CP_UTF8, 0, work_buffer.c_str(), (int)work_buffer.size(), &wide_string[0], num_chars);

	if (err <= 0) {
		return false;
	}

	speech_stop();
	return SUCCEEDED(Voice_device->Speak(wide_string.c_str(), SPF_ASYNC, NULL));
#elif defined(__ANDROID__)
	speech_stop();
	return android_tts_play(text);
#else
	int len = strlen(text);
	char Conversion_buffer[MAX_SPEECH_CHAR_LEN];

	if(len > (MAX_SPEECH_CHAR_LEN - 1)) {
		len = MAX_SPEECH_CHAR_LEN - 1;
	}

	int count = 0;
	for(int i = 0; i < len; i++) {
		if(text[i] == '$') {
			i++;
			continue;
		}

		Conversion_buffer[count] = text[i];
		count++;
	}

	Conversion_buffer[count] = '\0';

	if ( write(speech_dev, Conversion_buffer, count) == -1 )
		return false;
//	if (fwrite(Conversion_buffer, count, 1, speech_dev))
//		fflush(speech_dev);
//	else
//		return false;

	return true;
#endif	//_WIN32
}

bool speech_pause()
{
	if(Speech_init == false) return true;
#ifdef _WIN32
	return SUCCEEDED(Voice_device->Pause());
#elif defined(__ANDROID__)
	return android_tts_pause();
#else
	STUB_FUNCTION;

	return true;
#endif
}

bool speech_resume()
{
	if(Speech_init == false) return true;
#ifdef _WIN32
	return SUCCEEDED(Voice_device->Resume());
#elif defined(__ANDROID__)
	return android_tts_resume();
#else
	STUB_FUNCTION;

	return true;
#endif
}

bool speech_stop()
{
	if(Speech_init == false) return true;
#ifdef _WIN32
    return SUCCEEDED(Voice_device->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL ));
#elif defined(__ANDROID__)
	return android_tts_stop();
#else
	STUB_FUNCTION;

	return true;
#endif
}

bool speech_set_volume(unsigned short volume)
{
#ifdef _WIN32
    return SUCCEEDED(Voice_device->SetVolume(volume));
#elif defined(__ANDROID__)
	(void)volume;
	return true;
#else
	STUB_FUNCTION;

	return true;
#endif
}

bool speech_set_voice(int voice)
{
#ifdef _WIN32
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
#elif defined(__ANDROID__)
	(void)voice;
	return true;
#else
	STUB_FUNCTION;

	return true;
#endif
}

// Goober5000
bool speech_is_speaking()
{
#ifdef _WIN32
	HRESULT			hr;
	SPVOICESTATUS	pStatus;

	hr = Voice_device->GetStatus(&pStatus, NULL);
	if (FAILED(hr)) return false;

	return (pStatus.dwRunningState != SPRS_DONE);
#elif defined(__ANDROID__)
	return android_tts_is_speaking();
#else
	STUB_FUNCTION;

	return false;
#endif
}

SCP_vector<SCP_string> speech_enumerate_voices()
{
#ifdef _WIN32
	HRESULT hr = CoCreateInstance(
		CLSID_SpVoice,
		NULL,
		CLSCTX_ALL,
		IID_ISpVoice,
		(void **)&Voice_device);

	if (FAILED(hr)) {
		return SCP_vector<SCP_string>();
	}

	// This code is mostly copied from wxLauncher
	ISpObjectTokenCategory * comTokenCategory = NULL;
	IEnumSpObjectTokens * comVoices = NULL;
	ULONG comVoicesCount = 0;

	// Generate enumeration of voices
	hr = ::CoCreateInstance(CLSID_SpObjectTokenCategory, NULL,
		CLSCTX_INPROC_SERVER, IID_ISpObjectTokenCategory, (LPVOID*)&comTokenCategory);
	if (FAILED(hr)) {
		return SCP_vector<SCP_string>();
	}

	hr = comTokenCategory->SetId(SPCAT_VOICES, false);
	if (FAILED(hr)) {
		return SCP_vector<SCP_string>();
	}

	hr = comTokenCategory->EnumTokens(NULL, NULL, &comVoices);
	if (FAILED(hr)) {
		return SCP_vector<SCP_string>();
	}

	hr = comVoices->GetCount(&comVoicesCount);
	if (FAILED(hr)) {
		return SCP_vector<SCP_string>();
	}

	SCP_vector<SCP_string> voices;
	while (comVoicesCount > 0) {
		ISpObjectToken * comAVoice = NULL;

		comVoices->Next(1, &comAVoice, NULL); // retrieve just one

		LPWSTR id = NULL;
		comAVoice->GetStringValue(NULL, &id);

		auto idlength = wcslen(id);
		auto buffer_size = WideCharToMultiByte(CP_UTF8, 0, id, (int)idlength, nullptr, 0, nullptr, nullptr);

		if (buffer_size > 0) {
			SCP_string voiceName;
			voiceName.resize(buffer_size);
			buffer_size = WideCharToMultiByte(CP_UTF8, 0, id, (int)idlength, &voiceName[0], buffer_size, nullptr, nullptr);

			voices.push_back(voiceName);
		}

		CoTaskMemFree(id);
		comAVoice->Release();
		comVoicesCount--;
	}

	comTokenCategory->Release();

	Voice_device->Release();

	return voices;
#else
	STUB_FUNCTION;

	return SCP_vector<SCP_string>();
#endif
}

#endif // FS2_SPEECH
