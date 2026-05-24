#ifdef FS2_SPEECH
#include <dlfcn.h> 
#include "globalincs/pstypes.h"
#include "utils/unicode.h"
#include "speech.h"

// Adapted from libspeechd.h / speechd_types.h
// https://github.com/brailcom/speechd/tree/master/src/api/c

typedef struct SPDConnection SPDConnection;

typedef struct {
    char *name;
    char *language;
    char *variant;
} SPDVoice;

typedef enum {
    SPD_MODE_SINGLE = 0,
    SPD_MODE_THREADED = 1
} SPDConnectionMode;

typedef enum {
    SPD_IMPORTANT   = 1,
    SPD_MESSAGE     = 2,
    SPD_TEXT        = 3,
    SPD_NOTIFICATION = 4,
    SPD_PROGRESS    = 5
} SPDPriority;

static void* lib_handle = nullptr;

typedef SPDConnection* (*pfn_spd_open)(const char*, const char*, const char*, SPDConnectionMode);
typedef void (*pfn_spd_close)(SPDConnection*);
typedef int (*pfn_spd_say)(SPDConnection*, SPDPriority, const char*);
typedef int (*pfn_spd_pause)(SPDConnection*);
typedef int (*pfn_spd_resume)(SPDConnection*);
typedef int (*pfn_spd_stop)(SPDConnection*);
typedef int (*pfn_spd_set_volume)(SPDConnection*, signed int);
typedef int (*pfn_spd_set_synthesis_voice)(SPDConnection*, const char*);
typedef int (*pfn_spd_set_voice_rate)(SPDConnection*, signed int);
typedef SPDVoice** (*pfn_spd_list_synthesis_voices)(SPDConnection*);
typedef void (*pfn_free_spd_voices)(SPDVoice**);

static pfn_spd_open                		p_spd_open = nullptr;
static pfn_spd_close                	p_spd_close = nullptr;
static pfn_spd_say                  	p_spd_say = nullptr;
static pfn_spd_pause                	p_spd_pause = nullptr;
static pfn_spd_resume               	p_spd_resume = nullptr;
static pfn_spd_stop                 	p_spd_stop = nullptr;
static pfn_spd_set_volume           	p_spd_set_volume = nullptr;
static pfn_spd_set_synthesis_voice  	p_spd_set_synthesis_voice = nullptr;
static pfn_spd_list_synthesis_voices	p_spd_list_synthesis_voices = nullptr;
static pfn_spd_set_voice_rate			p_spd_set_voice_rate = nullptr;
static pfn_free_spd_voices 				p_free_spd_voices = nullptr;

// Load speech-dispatcher with dlopen and load symbols
static bool ensure_speechd_lib()
{
    if (lib_handle) return true;
    lib_handle = dlopen("libspeechd.so.2", RTLD_LAZY | RTLD_LOCAL);
    if (!lib_handle) {
		lib_handle = dlopen("libspeechd.so", RTLD_LAZY | RTLD_LOCAL);
    }

    if (!lib_handle) {
        mprintf(("Speech: Unable to load libspeechd.so: %s\n", dlerror()));
        return false;
    }
    
    // used symbols
    p_spd_open                	= (pfn_spd_open)               		dlsym(lib_handle, "spd_open");
    p_spd_close              	= (pfn_spd_close)              		dlsym(lib_handle, "spd_close");
    p_spd_say                 	= (pfn_spd_say)                		dlsym(lib_handle, "spd_say");
    p_spd_pause               	= (pfn_spd_pause)              		dlsym(lib_handle, "spd_pause");
    p_spd_resume              	= (pfn_spd_resume)             		dlsym(lib_handle, "spd_resume");
    p_spd_stop                	= (pfn_spd_stop)               		dlsym(lib_handle, "spd_stop");
    p_spd_set_volume          	= (pfn_spd_set_volume)         		dlsym(lib_handle, "spd_set_volume");
    p_spd_set_synthesis_voice 	= (pfn_spd_set_synthesis_voice)		dlsym(lib_handle, "spd_set_synthesis_voice");
    p_spd_list_synthesis_voices = (pfn_spd_list_synthesis_voices)	dlsym(lib_handle, "spd_list_synthesis_voices");
	p_spd_set_voice_rate		= (pfn_spd_set_voice_rate)			dlsym(lib_handle, "spd_set_voice_rate");
    p_free_spd_voices 			= (pfn_free_spd_voices)				dlsym(lib_handle, "free_spd_voices");

    if (!p_spd_open || !p_spd_close || !p_spd_say || !p_spd_pause || !p_spd_resume || !p_spd_stop || !p_spd_set_volume 
		|| !p_spd_set_voice_rate || !p_spd_set_synthesis_voice || !p_spd_list_synthesis_voices || !p_free_spd_voices) {
        mprintf(("Speech: Unable to load one or more symbols from libspeechd.so: %s\n", dlerror()));
        dlclose(lib_handle);
        lib_handle = nullptr;
        return false;
    }

    return true;
}

// Speech handling starts here
static bool Speech_init = false;
static SPDConnection* spd = nullptr;

bool speech_init()
{
	if (Speech_init) {
		return true;
	}
	    
	if (!ensure_speechd_lib()) {
        return false;
    }
    
    spd = p_spd_open("freespace_open", "main", nullptr, SPD_MODE_SINGLE);
    if (!spd) {
        mprintf(("Speech: Unable to connect to speech-dispatcher\n"));
		if (lib_handle) {
			dlclose(lib_handle);
			lib_handle = nullptr;
		}
        return false;
    }

	Speech_init = true;
	return true;
}

void speech_deinit()
{
	if ( !Speech_init ) {
		return;
	}
	p_spd_close(spd);
	Speech_init = false;
	spd = nullptr;
    if (lib_handle) { 
		dlclose(lib_handle); 
		lib_handle = nullptr; 
	}
}

bool speech_play(const SCP_string& text)
{
	if ( !Speech_init ) {
		return false;
	}

	if (text.empty()) {
		nprintf(("Speech", "Not playing speech because passed text is empty.\n"));
		return false;
	}

	return (p_spd_say(spd, SPD_TEXT, text.c_str()) >= 0);
}

bool speech_pause()
{
	if ( !Speech_init ) {
		return false;
	}

	p_spd_pause(spd);
	
	return true;
}

bool speech_resume()
{
	if ( !Speech_init ) {
		return false;
	}

	p_spd_resume(spd);
	
	return true;
}

bool speech_stop()
{
	if ( !Speech_init ) {
		return false;
	}

	p_spd_stop(spd);
	
	return true;
}

bool speech_set_volume(unsigned short volume)
{
	if ( !Speech_init ) {
		return false;
	}

	p_spd_set_volume(spd, volume); 
	
	return true;
}

bool speech_set_voice(int voice)
{
	if ( !Speech_init ) {
		return false;
	}
	
	auto voices = speech_enumerate_voices();

	if (voice < 0 || static_cast<size_t>(voice) >= voices.size()) {
        return false;
    }
    
	p_spd_set_synthesis_voice(spd, voices[voice].second.c_str());
	
	return true;
}

bool speech_set_rate(float rate_percent)
{
	if (!Speech_init) {
		return false;
	}

	// 50 / +150 -> 100 = normal -> range -100 / +100
	auto rate = static_cast<signed int>(rate_percent - 100.0f);
	CAP(rate, -100, 100);

	p_spd_set_voice_rate(spd, rate);
	return true;
}

/* 
	TODO: Try to implement this in some way,
		  there is no simple way to do it.
*/
bool speech_is_speaking()
{
	if ( !Speech_init ) {
		return false;
	}

	return false;
}

SCP_vector<std::pair<int, SCP_string>> speech_enumerate_voices()
{
	SCP_vector<std::pair<int, SCP_string>> fsoVoices;

	if (!Speech_init) {
		if (!ensure_speechd_lib()) {
			return fsoVoices;
		}
		spd = p_spd_open("freespace_open", "main", nullptr, SPD_MODE_SINGLE);
		if (!spd) {
			mprintf(("Speech: Unable to connect to speech-dispatcher\n"));
			if (lib_handle) {
				dlclose(lib_handle);
				lib_handle = nullptr;
			}
			return fsoVoices;
		}
	}

	SPDVoice** voices = p_spd_list_synthesis_voices(spd);

	if (voices) {
		for (int i = 0; voices[i] != nullptr; i++) {
			fsoVoices.emplace_back(std::make_pair(i, voices[i]->name));
		}
		p_free_spd_voices(voices);
	}
	else {
		mprintf(("Speech: Unable to get voice list from speech-dispatcher.\n"));
	}

	if (!Speech_init) {
		p_spd_close(spd);
		spd = nullptr;
		if (lib_handle) {
			dlclose(lib_handle);
			lib_handle = nullptr;
		}
	}

	return fsoVoices;
}

#endif
