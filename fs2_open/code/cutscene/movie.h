#ifndef _MOVIE_HEADER_FILE
#define _MOVIE_HEADER_FILE

void movie_set_shutdown_fgx(bool state);
bool movie_play(char *name, int unknown_value = 0);
bool movie_play_two(char *name1, char *name2, int unknown_value = 0);

#endif
