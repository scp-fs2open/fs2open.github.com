#ifndef _DX8SHOW_HEADER_FILE
#define _DX8SHOW_HEADER_FILE

bool dx8show_play_cutscene(char *filename);
bool dx8show_stop_cutscene();
//void dx8show_set_window_hwnd(HWND hwnd);
void dx8show_set_hwnd(void *hwnd);

#endif

