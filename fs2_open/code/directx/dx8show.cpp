#include <dshow.h>
#include <stdio.h>

#include "osapi/osapi.h"
#include "directx/dx8show.h"
#include "debugconsole/dbugfile.h"
#include "graphics/2d.h"

#include <commctrl.h>
#include <commdlg.h>
#include <tchar.h>
//#include <atlbase.h> // No dependecy on ATL!!!



enum PLAYSTATE {Stopped, Paused, Running, Init};
void Msg(TCHAR *szFormat, ...);

// Local
HRESULT InitPlayerWindow(HWND ghApp);
HRESULT InitVideoWindow(HWND ghApp, long w, long h);
bool HandleGraphEvent(void);
void CheckVisibility(void);
void CloseInterfaces(void);

//
// Constants
//
#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L

#define WM_GRAPHNOTIFY  WM_USER

char error_buffer[512];
char *ParseDShowError(HRESULT hr)
{
#ifndef NDEBUG
 	AMGetErrorText(hr, error_buffer, 512);
	return error_buffer;
#else
	return "DShow error";
#endif
}

//
// Macros
//
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define JIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) " (%s) " TEXT("\n"), hr, ParseDShowError(hr)); return hr;}

#define LIF(x) if (FAILED(hr=(x))) \
    {Msg(TEXT("FAILED(hr=0x%x) in ") TEXT(#x) TEXT("\n"), hr);}

//
// Global data
//
BOOL      g_bAudioOnly=FALSE;
LONG      g_lVolume=VOLUME_FULL;
DWORD     g_dwGraphRegister=0;
PLAYSTATE g_psCurrent=Stopped;

// DirectShow interfaces
IGraphBuilder *pGB = NULL;
IMediaControl *pMC = NULL;
IMediaEventEx *pME = NULL;
IVideoWindow  *pVW = NULL;
IBasicAudio   *pBA = NULL;
IBasicVideo   *pBV = NULL;
IMediaSeeking *pMS = NULL;
IMediaPosition *pMP = NULL;

void PassMsgToVideoWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (pVW)
        pVW->NotifyOwnerMessage((long) hWnd, message, wParam, lParam);
}

void SetCurrentState(PLAYSTATE state)
{
	g_psCurrent = state;
}

PLAYSTATE GetCurrentState()
{
	return g_psCurrent;
}

BOOL IsAudioOnly()
{
	return g_bAudioOnly; 
}

HRESULT PlayMovieInWindow(HWND ghApp, LPTSTR szFile)
{
//    USES_CONVERSION;
    WCHAR wFile[MAX_PATH];
    HRESULT hr;

    // Clear open dialog remnants before calling RenderFile()
    UpdateWindow(ghApp);

    // Convert filename to wide character string
//    wcscpy(wFile, T2W(szFile)); // requires ATL
	swprintf(wFile, L"%S", szFile); // Uses libc to convert instead

    // Get the interface for DirectShow's GraphBuilder
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                         IID_IGraphBuilder, (void **)&pGB));

    if(SUCCEEDED(hr))
    {
        HRESULT load_attempt = pGB->RenderFile(wFile, NULL);

        // Have the graph builder construct its the appropriate graph automatically
        if(FAILED(load_attempt))
		{
			return load_attempt;
		}

        // QueryInterface for DirectShow interfaces
        JIF(pGB->QueryInterface(IID_IMediaControl, (void **)&pMC));
        JIF(pGB->QueryInterface(IID_IMediaEventEx, (void **)&pME));
        JIF(pGB->QueryInterface(IID_IMediaSeeking, (void **)&pMS));
        JIF(pGB->QueryInterface(IID_IMediaPosition, (void **)&pMP));

        // Query for video interfaces, which may not be relevant for audio files
        JIF(pGB->QueryInterface(IID_IVideoWindow, (void **)&pVW));
        JIF(pGB->QueryInterface(IID_IBasicVideo, (void **)&pBV));

        // Query for audio interfaces, which may not be relevant for video-only files
        JIF(pGB->QueryInterface(IID_IBasicAudio, (void **)&pBA));

        // Is this an audio-only file (no video component)?
        CheckVisibility();

        // Have the graph signal event via window callbacks for performance
        JIF(pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0));

		long width, height;
		pBV->get_VideoWidth(&width);
		pBV->get_VideoHeight(&height);

        JIF(InitVideoWindow(ghApp,width, height));

		if (!g_bAudioOnly)
        {
            JIF(pVW->put_Owner((OAHWND)ghApp));
            JIF(pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE));
			JIF(pVW->SetWindowForeground(OATRUE));
 		}

        // Run the graph to play the media file
        JIF(pMC->Run());
        g_psCurrent=Running;
    }

    return hr;
}

extern int Cmdline_window;
HRESULT InitVideoWindow(HWND ghApp, long video_w, long video_h)
{
    HRESULT hr = S_OK;
    RECT rect;

    GetClientRect(ghApp, &rect);

	int screen_w = rect.right;
	int screen_h = rect.bottom;

	int video_rescale_w = (video_w * screen_w) / video_w;
	int video_rescale_h = (video_rescale_w * video_h) / video_w;

	int extra_w = (screen_w - video_rescale_w) / 2;
	int extra_h = (screen_h - video_rescale_h) / 2;
	
	if (Cmdline_window)
	{
		rect.right += GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
		rect.bottom += 2 * GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
	}

    if(SetWindowPos(ghApp, 0, 0, 0, rect.right, rect.bottom, SWP_NOMOVE | SWP_NOOWNERZORDER) == FALSE) {

   		MessageBox(ghApp,"Failed SetWindowPos","Error", MB_OK);
	}

    JIF(pVW->SetWindowPosition(extra_w, extra_h, video_rescale_w, video_rescale_h));
    return hr;
}

void CheckVisibility(void)
{
    long lVisible;
    HRESULT hr;

    if ((!pVW) || (!pBV))
    {
        // Audio-only files have no video interfaces.  This might also
        // be a file whose video component uses an unknown video codec.
        g_bAudioOnly = TRUE;
        return;
    }
    else
    {
        // Clear the global flag
        g_bAudioOnly = FALSE;
    }

    hr = pVW->get_Visible(&lVisible);
    if (FAILED(hr))
    {

        // If this is an audio-only clip, get_Visible() won't work.
        //
        // Also, if this video is encoded with an unsupported codec,
        // we won't see any video, although the audio will work if it is
        // of a supported format.
        //
        if (hr == E_NOINTERFACE)
        {
            g_bAudioOnly = TRUE;
        }
        else
        {
            Msg(TEXT("Failed(%08lx) in pVW->get_Visible()!\r\n"), hr);
        }
    }
}

bool OpenClip(HWND ghApp, char *g_szFileName)
{
    // Reset status variables
    g_psCurrent = Stopped;
    g_lVolume = VOLUME_FULL;

    // Start playing the media file
    HRESULT hr = PlayMovieInWindow(ghApp, g_szFileName);

    // If we couldn't play the clip, clean up
    if (FAILED(hr)) {
        CloseClip(ghApp);
		return false;
	}

	return true;
}

void CloseClip(HWND ghApp)
{
    HRESULT hr;

    // Stop media playback
    if(pMC)
        hr = pMC->Stop();

    // Clear global flags
    g_psCurrent = Stopped;
    g_bAudioOnly = TRUE;

    // Free DirectShow interfaces
    CloseInterfaces();

    // No current media state
    g_psCurrent = Init;
}


void CloseInterfaces(void)
{
    // Relinquish ownership (IMPORTANT!) after hiding video window
    if(pVW)
        pVW->put_Visible(OAFALSE);

    // Disable event callbacks
    if (pME)
        pME->SetNotifyWindow((OAHWND)NULL, 0, 0);

    // Release and zero DirectShow interfaces
    SAFE_RELEASE(pME);
    SAFE_RELEASE(pMS);
    SAFE_RELEASE(pMP);
    SAFE_RELEASE(pMC);
    SAFE_RELEASE(pBA);
    SAFE_RELEASE(pBV);
    SAFE_RELEASE(pVW);
    SAFE_RELEASE(pGB);
}

void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[512];  // Large buffer for very long filenames (like HTTP)

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);
    _vstprintf(szBuffer, szFormat, pArgs);
    va_end(pArgs);

    // Display a message box with the formatted string
    MessageBox(NULL, szBuffer, TEXT("PlayWnd Sample"), MB_OK);
}

// returns true when finished
bool dx8show_stream_movie(void)
{
    LONG evCode, evParam1, evParam2;
    HRESULT hr=S_OK;

    // Make sure that we don't access the media event interface
    // after it has already been released.
    if (!pME) {
	    return S_OK;
	}

	// Process all queued events
	while(SUCCEEDED(pME->GetEvent(&evCode, &evParam1,&evParam2, 0))) {
	    // Free memory associated with callback, since we're not using it
	    hr = pME->FreeEventParams(evCode, evParam1, evParam2);
	
	    // If this is the end of the clip, reset to beginning
	    if(EC_COMPLETE == evCode)
	    {
			return true;
	    }
	}

    return false;
}

