/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
*/ 

/* SAPI TOKENS
* 
$SPELL$
$ESPELL$

$VOL0$
$VOL1$
$EVOL$

$SIL$
*/

#ifndef _FSSPEECH_H_
#define _FSSPEECH_H_

enum
{
	FSSPEECH_FROM_TECHROOM,
	FSSPEECH_FROM_BRIEFING,
	FSSPEECH_FROM_INGAME,
	FSSPEECH_FROM_MULTI,
	FSSPEECH_FROM_MAX
};

enum
{
	FSSPEECH_TAG_SET_GENDER,
	FSSPEECH_TAG_END_GENDER,
	FSSPEECH_TAG_SET_LANGID,
	FSSPEECH_TAG_END_LANGID,
	FSSPEECH_TAG_SET_RATE,
	FSSPEECH_TAG_END_RATE,
	FSSPEECH_TAG_SET_PITCH,
	FSSPEECH_TAG_END_PITCH,
	FSSPEECH_TAG_SET_VOLUME,
	FSSPEECH_TAG_END_VOLUME,
	FSSPEECH_TAG_SET_SPELL,
	FSSPEECH_TAG_END_SPELL,
	FSSPEECH_TAG_SET_SILENCE
};

bool fsspeech_init();
void fsspeech_deinit();
void fsspeech_play(int type, const char* text);
void fsspeech_stop();
void fsspeech_pause(bool playing);

void fsspeech_start_buffer();
void fsspeech_stuff_buffer(const char *text);
void fsspeech_play_buffer(int type);

bool fsspeech_play_from(int type);
bool fsspeech_playing();

SCP_string fsspeech_write_tag(int type, const char* data = NULL);
void fsspeech_translate_tokens(SCP_string& text);
void fsspeech_remove_tokens(SCP_string& text);
SCP_string fsspeech_create_speech_text(const char* msg_text, const char* speech_tags);

#endif	// header define
