#include <dshow.h>
#include <stdio.h>

#include "osapi/osapi.h"
#include "dx8show.h"
#include "debugconsole/dbugfile.h"

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

    // Initialize COM
	RUN_FUNCTION_ROF(CoInitialize(NULL));

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
    CoUninitialize();

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
