#ifndef _FSSPEECH_H_
#define _FSSPEECH_H_

enum
{
	FSSPEECH_FROM_TECHROOM,
	FSSPEECH_FROM_BRIEFING,
	FSSPEECH_FROM_INGAME,
	FSSPEECH_FROM_MAX
};

bool fsspeech_init();
void fsspeech_deinit();
void fsspeech_play(int type, char *text);
void fsspeech_stop();
void fsspeech_pause(bool playing);

void fsspeech_start_buffer();
void fsspeech_stuff_buffer(char *text);
void fsspeech_play_buffer(int type);

#endif