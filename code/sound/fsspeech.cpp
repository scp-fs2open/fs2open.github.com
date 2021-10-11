/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

#include "globalincs/pstypes.h"
#include "osapi/osregistry.h"
#include "sound/fsspeech.h"
#include "sound/speech.h"
#include <regex>


extern int Cmdline_freespace_no_sound;

const size_t MAX_SPEECH_BUFFER_LEN = 4096;

static int speech_inited = 0;

bool FSSpeech_play_from[FSSPEECH_FROM_MAX];
const char *FSSpeech_play_id[FSSPEECH_FROM_MAX] =
{
	"SpeechTechroom",
	"SpeechBriefings",
	"SpeechIngame",
	"SpeechMulti"
};

char Speech_buffer[MAX_SPEECH_BUFFER_LEN] = "";
size_t  Speech_buffer_len;

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
		nprintf(("Speech", "Play %s: %s\n", FSSpeech_play_id[i], FSSpeech_play_from[i] ? "true" : "false"));
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
	if (!speech_inited) {
		nprintf(("Speech", "Aborting fsspech_play because speech_inited is false.\n"));
		return;
	}

	if (type >= FSSPEECH_FROM_MAX) {
		nprintf(("Speech", "Aborting fsspeech_play because speech type is out of range.\n"));
		return;
	}

	if (type >= 0 && FSSpeech_play_from[type] == false) {
		nprintf(("Speech", "Aborting fsspeech_play because we aren't supposed to play from type %s.\n", FSSpeech_play_id[type]));
		return;
	}

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

	size_t len = strlen(text);

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

SCP_string fsspeech_write_tag(int type, const char* data)
{
	SCP_string tag;
	#ifdef _WIN32
	switch (type) {
		case FSSPEECH_TAG_SET_GENDER: tag.append("<voice required='Gender="); tag.append(data); tag.append("'>"); break;
		case FSSPEECH_TAG_END_GENDER: tag.append("</voice>"); break;
		case FSSPEECH_TAG_SET_LANGID: tag.append("<lang langid='"); tag.append(data); tag.append("'>"); break;
		case FSSPEECH_TAG_END_LANGID: tag.append("</lang>"); break;
		case FSSPEECH_TAG_SET_RATE: tag.append("<rate speed='"); tag.append(data); tag.append("'>"); break;
		case FSSPEECH_TAG_END_RATE: tag.append("</rate>"); break;
		case FSSPEECH_TAG_SET_PITCH: tag.append("<pitch middle='"); tag.append(data); tag.append("'>"); break;
		case FSSPEECH_TAG_END_PITCH: tag.append("</pitch>"); break;
		case FSSPEECH_TAG_SET_VOLUME: tag.append("<volume level='"); tag.append(data); tag.append("'>"); break;
		case FSSPEECH_TAG_END_VOLUME: tag.append("</volume>"); break;
		case FSSPEECH_TAG_SET_SPELL: tag.append("<spell>"); break;
		case FSSPEECH_TAG_END_SPELL: tag.append("</spell>"); break;
		case FSSPEECH_TAG_SET_SILENCE: tag.append("<silence msec='"); tag.append(data); tag.append("'/>"); break;
	}
	#else
	#endif
	return tag;
}

void fsspeech_translate_tokens(SCP_string& text)
{
	text = std::regex_replace(text, std::regex(R"(\$SIL\$)"), fsspeech_write_tag(FSSPEECH_TAG_SET_SILENCE, "500"));
	text = std::regex_replace(text, std::regex(R"(\$VOL0\$)"), fsspeech_write_tag(FSSPEECH_TAG_SET_VOLUME, "0"));
	text = std::regex_replace(text, std::regex(R"(\$VOL1\$)"), fsspeech_write_tag(FSSPEECH_TAG_SET_VOLUME, "25"));
	text = std::regex_replace(text, std::regex(R"(\$SPELL\$)"), fsspeech_write_tag(FSSPEECH_TAG_SET_SPELL));
	text = std::regex_replace(text, std::regex(R"(\$EVOL\$)"), fsspeech_write_tag(FSSPEECH_TAG_END_VOLUME));
	text = std::regex_replace(text, std::regex(R"(\$ESPELL\$)"), fsspeech_write_tag(FSSPEECH_TAG_END_SPELL));
}

void fsspeech_remove_tokens(SCP_string& text)
{
	text = std::regex_replace(text, std::regex(R"(\$SIL\$)"), "");
	text = std::regex_replace(text, std::regex(R"(\$VOL0\$)"), "");
	text = std::regex_replace(text, std::regex(R"(\$VOL1\$)"), "");
	text = std::regex_replace(text, std::regex(R"(\$SPELL\$)"), "");
	text = std::regex_replace(text, std::regex(R"(\$EVOL\$)"), "");
	text = std::regex_replace(text, std::regex(R"(\$ESPELL\$)"), "");
}

SCP_string fsspeech_create_speech_text(const char* msg_text, const char* speech_tags)
{
	SCP_string speech_text = msg_text;
#ifdef _WIN32
	fsspeech_translate_tokens(speech_text);
	if (speech_tags != nullptr) {
		speech_text.insert(0, speech_tags);
	}
#else
	fsspeech_remove_tokens(speech_text);
#endif
	return speech_text;
}