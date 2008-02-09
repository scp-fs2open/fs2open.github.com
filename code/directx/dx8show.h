#ifndef _DX8SHOW_HEADER_FILE
#define _DX8SHOW_HEADER_FILE

bool OpenClip(HWND ghApp, char *g_szFileName);
void CloseClip(HWND ghApp);
bool dx8show_stream_movie(void);
void PassMsgToVideoWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif

