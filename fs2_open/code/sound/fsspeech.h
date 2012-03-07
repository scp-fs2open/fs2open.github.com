/*
 * Code created by Thomas Whittaker (RT) for a FreeSpace 2 source code project
 *
 * You may not sell or otherwise commercially exploit the source or things you 
 * created based on the source.
 *
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

#ifdef FS2_SPEECH

bool fsspeech_init();
void fsspeech_deinit();
void fsspeech_play(int type, const char *text);
void fsspeech_stop();
void fsspeech_pause(bool playing);

void fsspeech_start_buffer();
void fsspeech_stuff_buffer(const char *text);
void fsspeech_play_buffer(int type);

bool fsspeech_play_from(int type);
bool fsspeech_playing();

inline bool fsspeech_was_compiled() { return true; }

#else

// stub functions (c.f. NO_SOUND)

inline bool fsspeech_init() { return false; }
inline void fsspeech_deinit() {}
inline void fsspeech_play(int type, const char *text) { (type); (text); }
inline void fsspeech_stop() {}
inline void fsspeech_pause(bool playing) { (playing); }

inline void fsspeech_start_buffer() {}
inline void fsspeech_stuff_buffer(const char *text) { (text); }
inline void fsspeech_play_buffer(int type) {}

inline bool fsspeech_play_from(int type) { return false; }
inline bool fsspeech_playing() { return false; }
inline bool fsspeech_was_compiled( ) { return false; }

#endif

#endif	// header define
