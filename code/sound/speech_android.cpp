#ifdef FS2_SPEECH
#ifdef __ANDROID__
#include "globalincs/pstypes.h"
#include "utils/unicode.h"
#include "speech.h"
#include <jni.h>
#include "SDL.h"
#include "SDL_system.h"

bool Speech_init = false;
static JNIEnv* env = nullptr;
static jobject j_game_class = nullptr;
static jmethodID tts_speak      = nullptr;
static jmethodID tts_stop       = nullptr;
static jmethodID tts_pause      = nullptr;
static jmethodID tts_resume     = nullptr;
static jmethodID tts_isSpeaking = nullptr;
static jmethodID tts_shutdown   = nullptr;
static jmethodID tts_setRate    = nullptr;
static jmethodID tts_setVoice   = nullptr;
static jmethodID tts_getVoices  = nullptr;

// Ask SDL for the JNI Environment and hook to
// the external TTSManager on the android side of things.
// Then assign all method IDs for later use.
bool speech_init()
{
	Speech_init = false;
	mprintf(("Speech : Try to init TTSManager on GameActivity...\n"));

	// Get the JNI Environment pointer and current Activity instance via SDL
	env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

	if (env == nullptr) {
		mprintf(("Speech : Unable to get JNI environment!\n"));
		return false;
	}

	if (activity == nullptr) {
		mprintf(("Speech : Unable to get SDL Android activity!\n"));
		return false;
	}

	// GetObjectClass returns a local ref — promote to global so it survives
	// across JNI calls made from different scopes/threads.
	jclass local_class = env->GetObjectClass(activity);
	env->DeleteLocalRef(activity); 

	if (local_class == nullptr) {
		mprintf(("Speech : Unable to find the GameActivity class!\n"));
		return false;
	}

	j_game_class = (jclass)env->NewGlobalRef(local_class);
	env->DeleteLocalRef(local_class);

	// Map all static methods from TTSManager
	tts_speak      = env->GetStaticMethodID(j_game_class, "tts_speak",                    "(Ljava/lang/String;)Z");
	tts_stop       = env->GetStaticMethodID(j_game_class, "tts_stop",                     "()Z");
	tts_pause      = env->GetStaticMethodID(j_game_class, "tts_pause",                    "()Z");
	tts_resume     = env->GetStaticMethodID(j_game_class, "tts_resume",                   "()Z");
	tts_isSpeaking = env->GetStaticMethodID(j_game_class, "tts_isSpeaking",               "()Z");
	tts_shutdown   = env->GetStaticMethodID(j_game_class, "tts_shutdown",                 "()V");
	tts_setRate    = env->GetStaticMethodID(j_game_class, "tts_setRate",                  "(F)V");
	tts_setVoice   = env->GetStaticMethodID(j_game_class, "tts_setLanguageTag",           "(Ljava/lang/String;)V");
	tts_getVoices  = env->GetStaticMethodID(j_game_class, "tts_getAvailableLanguageTags", "()[Ljava/lang/String;");

	if (!tts_speak || !tts_stop || !tts_pause || !tts_resume || !tts_isSpeaking || !tts_shutdown || !tts_setRate || !tts_setVoice || !tts_getVoices) {
		mprintf(("Speech : Unable to map at least one core TTS method to GameActivity!\n"));
		env->DeleteGlobalRef(j_game_class);
		j_game_class = nullptr;
		return false;
	}

	mprintf(("Speech : Init Completed!\n"));
	Speech_init = true;
	return true;
}


bool speech_play(const SCP_string& text)
{
	if (!Speech_init)
		return false;

	if (text.empty()) {
		nprintf(("Speech", "Not playing speech because passed text is empty.\n"));
		return false;
	}

	jstring j_txt = env->NewStringUTF(text.c_str());
	mprintf(("Speech : Playing TTS string: %s!\n", text.c_str()));
	jboolean ok = env->CallStaticBooleanMethod(j_game_class, tts_speak, j_txt);
	env->DeleteLocalRef(j_txt);

	if (ok != JNI_TRUE) {
		mprintf(("Speech : Error playing TTS string!\n"));
		return false;
	}
	return true;
}

bool speech_stop()
{
	if (!Speech_init)
		return false;
	return env->CallStaticBooleanMethod(j_game_class, tts_stop) == JNI_TRUE;
}

bool speech_pause()
{
	if (!Speech_init)
		return false;
	return env->CallStaticBooleanMethod(j_game_class, tts_pause) == JNI_TRUE;
}

bool speech_resume()
{
	if (!Speech_init)
		return false;
	return env->CallStaticBooleanMethod(j_game_class, tts_resume) == JNI_TRUE;
}

bool speech_is_speaking()
{
	if (!Speech_init)
		return false;
	return env->CallStaticBooleanMethod(j_game_class, tts_isSpeaking) == JNI_TRUE;
}

void speech_deinit()
{
	if (!Speech_init)
		return;
	env->CallStaticVoidMethod(j_game_class, tts_shutdown);
	env->DeleteGlobalRef(j_game_class);
	j_game_class    = nullptr;
	tts_speak       = nullptr;
	tts_stop        = nullptr;
	tts_pause       = nullptr;
	tts_resume      = nullptr;
	tts_isSpeaking  = nullptr;
	tts_shutdown    = nullptr;
	tts_setRate     = nullptr;
	tts_setVoice    = nullptr;
	tts_getVoices   = nullptr;
	Speech_init     = false;
}


// Android TTS does not expose a direct volume API.
// Volume is controlled by the STREAM_MUSIC channel at the OS level.
bool speech_set_volume(unsigned short /*volume*/)
{
	if (!Speech_init)
		return false;
	return true;
}

bool speech_set_rate(float rate_percent)
{
	if (!Speech_init)
		return false;
	
	if(rate_percent > 150.0)
		rate_percent = 150.0;
	else if(rate_percent < 50.0)
		rate_percent = 50.0;
	
	float android_rate = rate_percent / 100.0f;   // 0.5 .. 1.0 .. 1.5
	env->CallStaticVoidMethod(j_game_class, tts_setRate, (jfloat)android_rate);
	return true;
}

bool speech_set_voice(int voice)
{
	if (!Speech_init)
		return false;

	jobjectArray tags = (jobjectArray)env->CallStaticObjectMethod(j_game_class, tts_getVoices);
	if (tags == nullptr)
		return false;

	jsize count = env->GetArrayLength(tags);
	if (voice < 0 || voice >= (int)count) {
		env->DeleteLocalRef(tags);
		return false;
	}

	jstring tag = (jstring)env->GetObjectArrayElement(tags, (jsize)voice);
	env->CallStaticVoidMethod(j_game_class, tts_setVoice, tag);
	env->DeleteLocalRef(tag);
	env->DeleteLocalRef(tags);
	return true;
}

SCP_vector<std::pair<int, SCP_string>> speech_enumerate_voices()
{
	SCP_vector<std::pair<int, SCP_string>> voices;

	if (!Speech_init)
		return voices;

	jobjectArray tags = (jobjectArray)env->CallStaticObjectMethod(j_game_class, tts_getVoices);
	if (tags == nullptr)
		return voices;

	jsize count = env->GetArrayLength(tags);
	voices.reserve((size_t)count);

	for (jsize i = 0; i < count; ++i) {
		jstring tag = (jstring)env->GetObjectArrayElement(tags, i);
		if (tag == nullptr)
			continue;

		const char* raw = env->GetStringUTFChars(tag, nullptr);
		if (raw) {
			voices.emplace_back((int)i, SCP_string(raw));
			env->ReleaseStringUTFChars(tag, raw);
		}
		env->DeleteLocalRef(tag);
	}

	env->DeleteLocalRef(tags);
	return voices;
}

#endif // __ANDROID__
#endif // FS2_SPEECH
