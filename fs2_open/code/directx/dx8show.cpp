#if 1 //change this to 1, if you are not me, change it to one,realy do it now
#include <dshow.h>
#include <stdio.h>

#include "osapi/osapi.h"
#include "dx8show.h"
#include "debugconsole/dbugfile.h"
#include "graphics/2d.h"

#include <commctrl.h>
#include <commdlg.h>
#include <tchar.h>
#include <atlbase.h>


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
IVideoFrameStep *pFS = NULL;

void PassMsgToVideoWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (pVW)
        pVW->NotifyOwnerMessage((LONG_PTR) hWnd, message, wParam, lParam);
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
    USES_CONVERSION;
    WCHAR wFile[MAX_PATH];
    HRESULT hr;

    // Clear open dialog remnants before calling RenderFile()
    UpdateWindow(ghApp);

    // Convert filename to wide character string
    wcscpy(wFile, T2W(szFile));

    // Get the interface for DirectShow's GraphBuilder
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                         IID_IGraphBuilder, (void **)&pGB));

    if(SUCCEEDED(hr))
    {
        // Have the graph builder construct its the appropriate graph automatically
        JIF(pGB->RenderFile(wFile, NULL));

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
    SAFE_RELEASE(pFS);
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
	while(SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR *) &evParam1,
	                (LONG_PTR *) &evParam2, 0))) {
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







#if 0


bool play_media(LPTSTR movie_filename)
{
    HRESULT hr;
    WCHAR file_name[MAX_PATH];

#ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, movie_filename, -1, file_name, MAX_PATH);
#else
    lstrcpy(file_name, movie_filename);
#endif

	// Allow DirectShow to create the FilterGraph for this media file
	RUN_FUNCTION_ROF(pGB->RenderFile(file_name, NULL));

    // Set the message drain of the video window to point to our hidden application window.  This allows 
	// keyboard input to be transferred to our main window for processing.
    //
    // If this is an audio-only or MIDI file, then put_MessageDrain will fail.
    //	
	RUN_FUNCTION(pVW->put_MessageDrain((OAHWND) internal_hwnd));

	if(set_fullscreen() == false)
	{
		return false;
	}
	
    // Display first frame of the movie and play
 	RUN_FUNCTION_ROF(pMC->Pause());
  	RUN_FUNCTION_ROF(pMC->Run());

    // Update state variables
    bool movie_continue = true;

    // Enter a loop of checking for events and sampling keyboard input
    while (movie_continue)
    {
        long event_code, param1, param2;

        // Has there been a media event?  Look for end of stream condition.
        if(E_ABORT != pME->GetEvent(&event_code, &param1, &param2, 0))
        {
            // Is this the end of the movie?
            if (event_code == EC_COMPLETE)
            {
				movie_continue = false;
            }

			// Free the media event resources.
            RUN_FUNCTION(pME->FreeEventParams(event_code, param1, param2));
        }

		
        // Give system threads time to run (and don't sample user input madly)
        Sleep(KEYBOARD_SAMPLE_FREQ);

		MSG msg;
		// Quits movie if escape, space or return is pressed
		while(PeekMessage(&msg, internal_hwnd, 0, 0, PM_REMOVE))
		{
 			TranslateMessage(&msg);
			
			if(msg.message != WM_KEYDOWN)
			{
				DispatchMessage(&msg);
				continue;
			}

			switch(msg.wParam )
			{
				case VK_ESCAPE:
			    case VK_SPACE:
			    case VK_RETURN:
				{
					DBUGFILE_OUTPUT_0("User skipped cut scene");
				 	movie_continue = false;
			    }
			}
 		}
		
    }

    return true;
}

#endif

#else
#include <dshow.h>
#include <stdio.h>

#include "osapi/osapi.h"
#include "dx8show.h"
#include "debugconsole/dbugfile.h"


bool OpenClip(HWND ghApp, char *g_szFileName)
{
	return false;
}
void PassMsgToVideoWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
}
bool dx8show_stream_movie(void)
{
	return false;
}
void CloseClip(HWND ghApp)
{
}

//
// Constants
//
const int KEYBOARD_SAMPLE_FREQ = 200;  // Sample user input on an interval

//
// Globals
//
IGraphBuilder  *pGB = NULL;
IMediaControl  *pMC = NULL;
IVideoWindow   *pVW = NULL;
IMediaEvent    *pME = NULL;

//
// Function prototypes
//
HRESULT get_interfaces(void);
bool play_media(LPTSTR movie_filename);
bool set_fullscreen(void);
void cleanup_interfaces(void);

HWND internal_hwnd = NULL;

// This define, 0 by default, if set to 1 will run window binding code which currently 
// doesnt work, left in for fun
#define VIS_CODE 0
//
// Helper Macros (Jump-If-Failed, Log-If-Failed)
//
#define RUN_FUNCTION_ROF(x) if(FAILED(hr=(x))) \
   	{DBUGFILE_OUTPUT_0("FAILED in " #x); return false;}

#define RUN_FUNCTION_GET_INTERFACE(x) if(FAILED(hr=(x))) \
    {DBUGFILE_OUTPUT_0("FAILED " #x); cleanup_interfaces(); return hr;}

#define RUN_FUNCTION(x) if(FAILED(hr=(x))) \
    {DBUGFILE_OUTPUT_0("FAILED " #x);}

#define RELEASE(i) {if (i) i->Release(); i = NULL;}

void process_messages(HWND hwnd)
{
	MSG msg;
   	while(PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
   	{
   		TranslateMessage(&msg);
   		DispatchMessage(&msg);
   	}
}

void dx8show_set_hwnd(void *hwnd)
{
	internal_hwnd = (HWND) hwnd;
}

bool dx8show_play_cutscene(char *movie_file)
{
    HRESULT hr;

    // Get DirectShow interfaces
    RUN_FUNCTION_ROF(get_interfaces());

	if(internal_hwnd == NULL)
	{
		internal_hwnd = (HWND) os_get_window();
	}			

#if VIS_CODE
	RUN_FUNCTION(pVW->put_Visible(OATRUE));
	RUN_FUNCTION(pVW->put_Owner((OAHWND) internal_hwnd));
#endif

    // Play the movie / cutscene
    play_media(movie_file);
    
#if VIS_CODE
    RUN_FUNCTION(pVW->put_Visible(OAFALSE));
	RUN_FUNCTION(pVW->put_Owner(NULL));
#endif

    // Release DirectShow interfaces
    cleanup_interfaces();

    return true;
}

HRESULT get_interfaces(void)
{
    HRESULT hr = S_OK;

	// Instantiate filter graph interface
	RUN_FUNCTION_GET_INTERFACE(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **) &pGB));
    // Get interfaces to control playback & screensize
    RUN_FUNCTION_GET_INTERFACE(pGB->QueryInterface(IID_IMediaControl,  (void **) &pMC));
    RUN_FUNCTION_GET_INTERFACE(pGB->QueryInterface(IID_IVideoWindow,   (void **) &pVW));

    // Get interface to allow the app to wait for completion of playback
    RUN_FUNCTION_GET_INTERFACE(pGB->QueryInterface(IID_IMediaEventEx,  (void **)&pME));

    return hr;
}

void cleanup_interfaces(void)
{
    // Release the DirectShow interfaces
    RELEASE(pMC);
    RELEASE(pVW);
    RELEASE(pME);
    RELEASE(pGB);
}

bool play_media(LPTSTR movie_filename)
{
    HRESULT hr;
    WCHAR file_name[MAX_PATH];

#ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, movie_filename, -1, file_name, MAX_PATH);
#else
    lstrcpy(file_name, movie_filename);
#endif

	// Allow DirectShow to create the FilterGraph for this media file
	RUN_FUNCTION_ROF(pGB->RenderFile(file_name, NULL));

    // Set the message drain of the video window to point to our hidden application window.  This allows 
	// keyboard input to be transferred to our main window for processing.
    //
    // If this is an audio-only or MIDI file, then put_MessageDrain will fail.
    //	
	RUN_FUNCTION(pVW->put_MessageDrain((OAHWND) internal_hwnd));

	if(set_fullscreen() == false)
	{
		return false;
	}
	
    // Display first frame of the movie and play
 	RUN_FUNCTION_ROF(pMC->Pause());
  	RUN_FUNCTION_ROF(pMC->Run());

    // Update state variables
    bool movie_continue = true;

    // Enter a loop of checking for events and sampling keyboard input
    while (movie_continue)
    {
        long event_code, param1, param2;

        // Has there been a media event?  Look for end of stream condition.
        if(E_ABORT != pME->GetEvent(&event_code, &param1, &param2, 0))
        {
            // Is this the end of the movie?
            if (event_code == EC_COMPLETE)
            {
				movie_continue = false;
            }

			// Free the media event resources.
            RUN_FUNCTION(pME->FreeEventParams(event_code, param1, param2));
        }

		
        // Give system threads time to run (and don't sample user input madly)
        Sleep(KEYBOARD_SAMPLE_FREQ);

		MSG msg;
		// Quits movie if escape, space or return is pressed
		while(PeekMessage(&msg, internal_hwnd, 0, 0, PM_REMOVE))
		{
 			TranslateMessage(&msg);
			
			if(msg.message != WM_KEYDOWN)
			{
				DispatchMessage(&msg);
				continue;
			}

			switch(msg.wParam )
			{
				case VK_ESCAPE:
			    case VK_SPACE:
			    case VK_RETURN:
				{
					DBUGFILE_OUTPUT_0("User skipped cut scene");
				 	movie_continue = false;
			    }
			}
 		}
		
    }

    return true;
}

bool set_fullscreen(void)
{
    HRESULT hr;
    LONG mode;
    HWND drain=0;

    if (!pVW)
	{
        return S_FALSE;
	}

    // Read current state
	RUN_FUNCTION_ROF(pVW->get_FullScreenMode(&mode));

    if (mode == OAFALSE)	
    {
        // Save current message drain
		RUN_FUNCTION_ROF(pVW->get_MessageDrain((OAHWND *) &drain));

		// This is very important. It ensures that the video window doesnt end
		// up playing behind the main window
		SetFocus(drain);

        // Set message drain to application main window
		RUN_FUNCTION_ROF(pVW->put_MessageDrain((OAHWND) internal_hwnd));
		  
        // Switch to full-screen mode
        mode = OATRUE;
			
		RUN_FUNCTION_ROF(pVW->put_FullScreenMode(mode));
    }

    return true;
}
#pragma message( "WARNING: if you are not Bobboau your direct show is currently broken, go into dx8show.cpp and change the indicated 0 to a 1, it'll be the first thing in the file" )
#endif