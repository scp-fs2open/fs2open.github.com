#include <dshow.h>
#include <stdio.h>

#include "osapi/osapi.h"
#include "dx8show.h"

//
// Constants
//
#define KEYBOARD_SAMPLE_FREQ  1000  // Sample user input on an interval

//
// Globals
//
static IGraphBuilder  *pGB = NULL;
static IMediaControl  *pMC = NULL;
static IVideoWindow   *pVW = NULL;
static IMediaEvent    *pME = NULL;

static BOOL g_bContinue = TRUE;
static BOOL g_bUserInterruptedPlayback = FALSE;

//
// Function prototypes
//
HRESULT play_media(LPTSTR movie_filename);
HRESULT get_interfaces(void);
HRESULT set_fullscreen(void);
void cleanup_interfaces(void);

HWND internal_hwnd = NULL;

//
// Helper Macros (Jump-If-Failed, Log-If-Failed)
//
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

bool dx8show_stop_cutscene()
{
	pMC->Stop();
	return true;
}

bool dx8show_play_cutscene(char *movie_file)
{
    HRESULT hr;

    // Initialize COM
    if (FAILED(hr = CoInitialize(NULL)))
	{
        return false;
	}

    // Get DirectShow interfaces
    if (FAILED(hr = get_interfaces()))
    {
        CoUninitialize();
        return false;
    }

	if(internal_hwnd == NULL)
	{
		internal_hwnd = (HWND) os_get_window();
	}

	hr = pVW->put_Visible(OAFALSE);
	pVW->put_Owner((OAHWND) internal_hwnd);

    // Play the movie / cutscene
    hr = play_media(movie_file);
    
	// If the user interrupted playback and there was no other error,
	bool result = true;

    if ((hr == S_OK) && g_bUserInterruptedPlayback)
	{
        result = false;
	}

    hr = pVW->put_Visible(OAFALSE);
	pVW->put_Owner(NULL);

    // Release DirectShow interfaces
    cleanup_interfaces();
    CoUninitialize();

    return result;
}

HRESULT get_interfaces(void)
{
    HRESULT hr = S_OK;

    // Instantiate filter graph interface
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **) &pGB);

	if(FAILED(hr))
	{
		cleanup_interfaces();
		return hr;
	}

    // Get interfaces to control playback & screensize
    hr = pGB->QueryInterface(IID_IMediaControl,  (void **) &pMC);

	if(FAILED(hr))
	{
		cleanup_interfaces();
		return hr;
	}

    hr = pGB->QueryInterface(IID_IVideoWindow,   (void **) &pVW);

	if(FAILED(hr))
	{
		cleanup_interfaces();
		return hr;
	}

    // Get interface to allow the app to wait for completion of playback
    hr = pGB->QueryInterface(IID_IMediaEventEx,  (void **)&pME);

	if(FAILED(hr))
	{
		cleanup_interfaces();
		return hr;
	}

    return hr;
}


void cleanup_interfaces(void)
{
    // Release the DirectShow interfaces
    RELEASE(pGB);
    RELEASE(pMC);
    RELEASE(pVW);
    RELEASE(pME);
}

HRESULT play_media(LPTSTR movie_filename)
{
    HRESULT hr = S_OK;
    WCHAR wFileName[MAX_PATH];
    BOOL bSleep=TRUE;

//#ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, movie_filename, -1, wFileName, MAX_PATH);
//#else
//    lstrcpy(wFile, szFile);
//#endif

	// Allow DirectShow to create the FilterGraph for this media file
    hr = pGB->RenderFile(wFileName, NULL);
	process_messages((HWND) os_get_window());

    if (FAILED(hr)) {
        return hr;
    }

    // Set the message drain of the video window to point to our hidden
    // application window.  This allows keyboard input to be transferred
    // to our main window for processing.
    //
    // If this is an audio-only or MIDI file, then put_MessageDrain will fail.
    //	
    hr = pVW->put_MessageDrain((OAHWND) internal_hwnd);
	process_messages((HWND) os_get_window());

    if (FAILED(hr))
    {
//        Msg(TEXT("Failed(0x%08lx) to set message drain for %s.\r\n\r\n"
 //                "This sample is designed to play videos, but the file selected "
 //                "has no video component."), hr, movie_filename);
        return hr;
    }	
	
    // Set fullscreen

    hr = set_fullscreen();

    if (FAILED(hr)) {
        return hr;
    }
	
	process_messages((HWND) os_get_window());

    // Display first frame of the movie
	hr = pVW->put_Visible(OATRUE);
    hr = pMC->Pause();

    if (FAILED(hr)) {
        return hr;
    }

    // Start playback
    hr = pMC->Run();

    if (FAILED(hr)) {
        return hr;
    }

    // Update state variables
    g_bContinue = TRUE;

    // Enter a loop of checking for events and sampling keyboard input
    while (g_bContinue)
    {
        long lEventCode, lParam1, lParam2;

        // Reset sleep flag
        bSleep = TRUE;

        // Has there been a media event?  Look for end of stream condition.
        if(E_ABORT != pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0))
        {
            // Free the media event resources.
            hr = pME->FreeEventParams(lEventCode, lParam1, lParam2);

            if (FAILED(hr))
            {
				// take no action
            }

            // Is this the end of the movie?
            if (lEventCode == EC_COMPLETE)
            {
                g_bContinue = FALSE;
                bSleep = FALSE;
            }
        }

        // Give system threads time to run (and don't sample user input madly)
        if (bSleep)
		{
            Sleep(KEYBOARD_SAMPLE_FREQ);
		}

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
					g_bUserInterruptedPlayback = TRUE;
			        dx8show_stop_cutscene();
			        return S_OK;
			    }
			}
 		}
    }

    return hr;
}


HRESULT set_fullscreen(void)
{
    HRESULT hr = S_OK;
    LONG lMode;
    static HWND hDrain=0;

    if (!pVW)
	{
        return S_FALSE;
	}

    // Read current state
    hr = pVW->get_FullScreenMode(&lMode);

	if(FAILED(hr))
	{
		return hr;
	}

    if (lMode == OAFALSE)	
    {
        // Save current message drain
        hr = pVW->get_MessageDrain((OAHWND *) &hDrain);

		if(FAILED(hr))
		{
			return hr;
		}

        // Set message drain to application main window
        hr = pVW->put_MessageDrain((OAHWND) internal_hwnd);

		if(FAILED(hr))
		{
			return hr;
		}

		process_messages((HWND) os_get_window());
		  
        // Switch to full-screen mode
        lMode = OATRUE;
        hr = pVW->put_FullScreenMode(lMode);

		if(FAILED(hr))
		{
			return hr;
		}
    }

    return hr;
}


