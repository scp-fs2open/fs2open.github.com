/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/OsApi/OsApi.cpp $
 * $Revision: 2.2 $
 * $Date: 2002-08-01 01:41:09 $
 * $Author: penguin $
 *
 * Low level Windows code
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:10:40  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/06/03 03:42:13  mharris
 * fAppActive is now set correctly; translate some keyboard scancodes
 * that appear to be different between SDL and Win32; combine KEYDOWN
 * and KEYUP events.
 *
 * Revision 1.2  2002/05/21 15:45:58  mharris
 * init outwnd in debug mode; added SDL support
 *
 * Revision 1.1  2002/05/16 00:47:21  mharris
 * New version of OsApi files for Unix variants.
 *
 * Revision 1.2  2002/05/09 13:52:01  mharris
 * debug_int3() calls abort()
 *
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 * 
 * 
 * 7     6/30/99 5:53p Dave
 * Put in new anti-camper code.
 * 
 * 6     6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 5     6/02/99 6:18p Dave
 * Fixed TNT lockup problems! Wheeeee!
 * 
 * 4     12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/08/98 2:38p Dave
 * Cleanup up OsAPI code significantly. Removed old functions, centralized
 * registry functions.
 * 
 * 118   7/10/98 5:04p Dave
 * Fix connection speed bug on standalone server.
 * 
 * 117   5/24/98 2:28p Hoffoss
 * Because we never really care about if the left or the right shift or
 * alt key was used, but rather than either shift or alt was used, made
 * both map to the left one.  Solves some problems, causes none.
 * 
 * 116   5/18/98 9:22p John
 * Took out the annoying tab check.
 * 
 * 115   5/18/98 11:17a John
 * Fixed some bugs with software window and output window.
 * 
 * 114   5/16/98 2:20p John
 * Changed the os_suspend and resume to use a critical section to prevent
 * threads from executing rather than just suspending the thread.  Had to
 * make sure gr_close was called before os_close.
 * 
 * 113   5/15/98 4:49p John
 * 
 * 112   5/15/98 3:36p John
 * Fixed bug with new graphics window code and standalone server.  Made
 * hwndApp not be a global anymore.
 * 
 * 111   5/14/98 5:42p John
 * Revamped the whole window position/mouse code for the graphics windows.
 * 
 * 110   5/04/98 11:08p Hoffoss
 * Expanded on Force Feedback code, and moved it all into Joy_ff.cpp.
 * Updated references everywhere to it.
 *
 * $NoKeywords: $
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <pthread.h>
#ifndef _WIN32
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include "globalincs/pstypes.h"
#include "osapi/osapi.h"
#include "io/key.h"
#include "palman/palman.h"
#include "io/mouse.h"
#include "osapi/outwnd.h"
#include "graphics/2d.h"
#include "cfile/cfile.h"
#include "sound/sound.h"
#include "freespace2/freespaceresource.h"
#include "playerman/managepilot.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "osapi/osregistry.h"
#include "cmdline/cmdline.h"

// ----------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

// os-wide globals
static int			fAppActive = 1;
static char			szWinTitle[128];
static char			szWinClass[128];
static int			WinX, WinY, WinW, WinH;
static int			Os_inited = 0;

static CRITICAL_SECTION Os_lock;

static byte sdl_key_translation[256] = {
	/* 00 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 08 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 10 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 18 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 20 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 28 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 30 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 38 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 40 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 48 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 50 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 58 */  0x00, 0xC7, 0xC8, 0xC9, 0xCB, 0x00, 0xCD, 0xCF,
	/* 60 */  0xD0, 0xD1, 0xD2, 0xD3, 0x00, 0x00, 0x45, 0xB7,
	/* 68 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 70 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 78 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 80 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 88 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 90 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 98 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* A0 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* A8 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* B0 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* B8 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* C0 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* C8 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* D0 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* D8 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* E0 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* E8 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* F0 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* F8 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


int Os_debugger_running = 0;

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

#ifdef THREADED
	// thread handler for the main message thread
	DWORD unix_process(DWORD lparam);
#else
	DWORD unix_process1(DWORD lparam);
	DWORD unix_process2(DWORD lparam);
#endif

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit();

//  // go through all windows and try and find the one that matches the search string
//  BOOL os_enum_windows( HWND hwnd, char * search_string );

//  // message handler for the main thread
//  LRESULT CALLBACK unix_message_handler(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam);

// create the main window
BOOL unix_create_window();


// ----------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// condition variable: a mutex + a "raw" cv
struct os_cv_t {
	pthread_mutex_t cv_mutex;
	pthread_cond_t  cv_cond;
	bool            status;

	inline os_cv_t()
		  : status(false)
		{
			  pthread_mutex_init(&cv_mutex, NULL);
			  pthread_cond_init(&cv_cond, NULL);
		}

	inline ~os_cv_t()
		{
			pthread_mutex_destroy(&cv_mutex);
			pthread_cond_destroy(&cv_cond);
		}

	inline void lock()
		{
			pthread_mutex_lock(&cv_mutex);
		}

	int wait(const timespec * wait_time = NULL)
		{
			if (wait_time == NULL)
				return pthread_cond_wait(&cv_cond, &cv_mutex);
			else
				return pthread_cond_timedwait(&cv_cond, &cv_mutex, wait_time);
		}
};


// initialization/shutdown functions -----------------------------------------------

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(char * wclass, char * title, char *app_name, char *version_string )
{
	os_init_registry_stuff(Osreg_company_name, title, version_string);
	
	strcpy( szWinTitle, title );
	strcpy( szWinClass, wclass );	

#ifndef NDEBUG
	outwnd_init(1);
#endif	

	InitializeCriticalSection( &Os_lock );

	#ifdef THREADED
	{
		// Create an condition variable to signal that the window is
		// created, so that we don't return from this function until the
		// window is all properly created.
		os_cv_t Window_created;
		Window_created.lock();

		// Create the thread
		pthread_create(&ThreadID, NULL, unix_process, &Window_created);

		// Wait for thread to signal the cv
		timeval now;
		timespec wait_time;
		gettimeofday(&now);
		wait_time.tv_sec = now.tv_sec + 5;       // wait 5 seconds
		wait_time.tv_nsec = now.tv_usec * 1000;  // usec -> msec
		while (!Window_created.status) {
			int status = Window_created.wait(&wait_time);
			if (status == ETIMEDOUT) {
				mprintf(( "Wait timeout!\n" ));
				break;
			}
		}
	}
	#else
		unix_process1(0);
	#endif // THREADED

	// initialized
	Os_inited = 1;

	atexit(os_deinit);
}

// set the main window title
void os_set_title( char * title )
{
	strcpy( szWinTitle, title );
	// mharris TODO
// 	XStoreName(display, win, szWinTitle);
}

// call at program end
void os_cleanup()
{
	// Tell the app to quit
	// mharris TODO
//  	XDestroyWindow(display, window);
	
	#ifndef NDEBUG
		outwnd_close();
	#endif
}


// window management -----------------------------------------------------------------

// Returns 1 if app is not the foreground app.
int os_foreground()
{
	return fAppActive;
}

//  // Returns the handle to the main window
//  uint os_get_window()
//  {
//  	  // mharris TODO
//  }


// process management -----------------------------------------------------------------

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(int ms)
{
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = ((long)(ms % 1000)) * 1000000;   // * 1M to get nanosecs
	nanosleep(&ts, NULL);
}

// Used to stop message processing
void os_suspend()
{
	ENTER_CRITICAL_SECTION(&Os_lock);	
}

// resume message processing
void os_resume()
{
	LEAVE_CRITICAL_SECTION(&Os_lock);	
}


// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

#ifdef THREADED

// thread handler for the main message thread
DWORD unix_process(DWORD lparam)
{
	MSG msg;
	HANDLE Window_created = (HANDLE)lparam;

	if ( !unix_create_window() )
		return 0;

	// Let the app continue once the window is created
	SetEvent(Window_created);

	while (1)	{	
		if (WaitMessage() == TRUE)	{
			EnterCriticalSection(&Os_lock);			
			while(PeekMessage(&msg,0,0,0,PM_REMOVE))	{
				if ( msg.message == WM_DESTROY )	{
					LeaveCriticalSection(&Os_lock);

					// cleanup and exit this thread!!
					DeleteCriticalSection(&Os_lock);
					return 0;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			LeaveCriticalSection(&Os_lock);
		} 
	}
	return 0;
}

#else

DWORD unix_process1(DWORD lparam)
{
	  // mharris TODO
	if ( !unix_create_window() )
		return 0;
	return 0;
}


DWORD unix_process2(DWORD lparam)
{
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_ACTIVEEVENT:
			if (event.active.state & (SDL_APPACTIVE | SDL_APPINPUTFOCUS)) {
				mprintf(("SDL_ACTIVEEVENT (%02x, %02x) happened\n",
							event.active.state, event.active.gain));
				fAppActive = event.active.gain;
			}
			break;

		case SDL_KEYDOWN: {
      case SDL_KEYUP:
			if (event.key.keysym.scancode <= 8) {
				Assert(event.key.keysym.scancode == 0);
				break;
			}
			
			// the keycode returned by SDL seems to be 8 higher than Win32...
			byte keydata = event.key.keysym.scancode - 8;
			int latency = 0; // don't think we can get this from SDL

			int mark;
			if (event.type == SDL_KEYDOWN) {
				mprintf(("Key press detected: Scancode: %02X, keycode: %02X\n", 
							keydata, event.key.keysym.sym));
				mark = 1;
			} else {
				mprintf(("Key release detected: Scancode: %02X, keycode: %02X\n", 
							keydata, event.key.keysym.sym));
				mark = 0;
			}

			byte trans_key = sdl_key_translation[keydata];
			if (trans_key) {
				// SDL scancodes are different than Win32 in some cases...
				mprintf(("  Translating to %02x\n", trans_key));
				keydata = trans_key;
			}

			key_mark( keydata, mark, latency );
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			int state = (event.button.state == SDL_PRESSED);
			uint button;
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				mouse_mark_button(MOUSE_LEFT_BUTTON, state);
				break;
			case SDL_BUTTON_MIDDLE:
				mouse_mark_button(MOUSE_MIDDLE_BUTTON, state);
				break;
			case SDL_BUTTON_RIGHT:
				mouse_mark_button(MOUSE_RIGHT_BUTTON, state);
				break;
			}
		}
		}
	}


	  // mharris TODO
//  	  XEvent event_return;
//  	  while (true) {
//  			 XNextEvent(display, &event_return);
//  			 if (event_return.type = DestroyNotify)
//  					break;
//  	  }
	return 0;
}
#endif // THREADED


// called at shutdown. Makes sure all thread processing terminates.
void os_deinit()
{
	// mharris TODO -- maybe?
}

//  // go through all windows and try and find the one that matches the search string
//  BOOL __stdcall os_enum_windows( HWND hwnd, char * search_string )
//  {
//  	  // mharris TODO
//  }


int Got_message = 0;
// message handler for the main thread
//  void unix_message_handler()
//  {
//  	// Got_message++;

//  	switch(msg)	{

//  	case WM_QUERYNEWPALETTE:
//  		// mprintf(( "WM: QueryNewPalette\n" ));
//  		return TRUE;	// Say that I've realized my own palette
//  		break;
//  	case WM_PALETTECHANGED:
//  		// mprintf(( "WM: PaletteChanged\n" ));
//  		break;
//  	case WM_PALETTEISCHANGING:
//  		// mprintf(( "WM: PaletteIsChanging\n" ));
//  		break;

//  	case WM_DISPLAYCHANGE:
//  		// mprintf(( "WM: DisplayChange\n" ));
//  		break;

//  	case WM_LBUTTONDOWN:
//  		mouse_mark_button( MOUSE_LEFT_BUTTON, 1 );
//  		break;

//  	case WM_LBUTTONUP:
//  		mouse_mark_button( MOUSE_LEFT_BUTTON, 0 );
//  		break;

//  	case WM_RBUTTONDOWN:
//  		mouse_mark_button( MOUSE_RIGHT_BUTTON, 1 );
//  		break;

//  	case WM_RBUTTONUP:
//  		mouse_mark_button( MOUSE_RIGHT_BUTTON, 0 );
//  		break;

//  	case WM_MBUTTONDOWN:
//  		mouse_mark_button( MOUSE_MIDDLE_BUTTON, 1 );
//  		break;

//  	case WM_MBUTTONUP:
//  		mouse_mark_button( MOUSE_MIDDLE_BUTTON, 0 );
//  		break;

//  	case WM_TIMER:
//  		break;

//  	case WM_SYSKEYDOWN:
//  	case WM_KEYDOWN:	{		
//  			int nVirtKey;
//  			uint lKeyData;

//  			int latency;
//  			latency = timeGetTime() - GetMessageTime();
//  			if ( latency < 0 )
//  				latency=0;

//  			nVirtKey = (int)wParam;    // virtual-key code 
//  			lKeyData = (lParam>>16) & 255;          // key data 
//  			if ( (lParam>>16) & 256 ) lKeyData += 0x80;

//  			// Fix up print screen, whose OEM code is wrong under 95.
//  			if ( nVirtKey == VK_SNAPSHOT )	{
//  				lKeyData = KEY_PRINT_SCRN;	
//  			}

//  			if (lKeyData == KEY_RSHIFT)  // either shift is just a shift to us..
//  				lKeyData = KEY_LSHIFT;

//  			if (lKeyData == KEY_RALT)  // Same with alt keys..
//  				lKeyData = KEY_LALT;

//  //			mprintf(( "Key down = 0x%x|%x\n", lKeyData, nVirtKey ));
//  			key_mark( lKeyData, 1, latency );
//  //			mprintf(( "Key down = 0x%x\n", lKeyData ));
//  			//Warning( LOCATION, "Key = 0x%x", lKeyData );			
//  		}
//  		break;

//  	case WM_SYSKEYUP:
//  	case WM_KEYUP:	 {		
//  			int nVirtKey;
//  			uint lKeyData;

//  			int latency;
//  			latency = timeGetTime() - GetMessageTime();
//  			if ( latency < 0 )
//  				latency=0;

//  			nVirtKey = (int) wParam;    // virtual-key code 
//  			lKeyData = (lParam>>16) & 255;          // key data 
//  			if ( (lParam>>16) & 256 ) lKeyData += 0x80;

//  			// Fix up print screen, whose OEM code is wrong under 95.
//  			if ( nVirtKey == VK_SNAPSHOT )	{
//  				lKeyData = KEY_PRINT_SCRN;	
//  			}

//  			if (lKeyData == KEY_RSHIFT)  // either shift is just a shift to us..
//  				lKeyData = KEY_LSHIFT;

//  			if (lKeyData == KEY_RALT)  // Same with alt keys..
//  				lKeyData = KEY_LALT;

//  //			mprintf(( "Key up = 0x%x|%x\n", lKeyData, nVirtKey ));
//  			if ( lKeyData == 0xB7 )	{
//  				// Hack for PrintScreen which only sends one up message!
//  				key_mark( lKeyData, 1, latency );		
//  				key_mark( lKeyData, 0, latency );

//  			} else {
//  				key_mark( lKeyData, 0, latency );
//  			}			
//  		}		
//  		break;

//  	case WM_KILLFOCUS:
//  		key_lost_focus();

//  		gr_activate(0);
//  		break;

//  	case WM_SETFOCUS:
//  		key_got_focus();

//  		gr_activate(1);
//  		break;

//  	case WM_ACTIVATE:			//APP:	//
//  	case WM_ACTIVATEAPP:
//  		{
//  			BOOL OldfAppActive = fAppActive;
			
//  			// The application z-ordering has changed. If we are now the
//  			// foreground application wParm will be TRUE.
//  			if ( msg == WM_ACTIVATE )	{
//  				if ( (LOWORD(wParam) == WA_ACTIVE) || (LOWORD(wParam)==WA_CLICKACTIVE))	{
//  					fAppActive = TRUE;	//(BOOL)wParam;
//  				} else {
//  					fAppActive = FALSE;	//(BOOL)wParam;
//  				}

//  			} else {
//  				fAppActive = (BOOL)wParam;
//  			}

//  			//mprintf(( "Activate: %d\n", fAppActive ));

//  			if ( OldfAppActive != fAppActive )	{

//  				if ( fAppActive )	{
//  					// maximize it
//  					//	mprintf(( "Priority: HIGH\n" ));
//  					joy_reacquire_ff();

//  #ifdef THREADED
//  					SetThreadPriority( hThread, THREAD_PRIORITY_HIGHEST );
//  #endif

//  					gr_activate(fAppActive);
//  					//SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );
//  				} else {
//  					//mprintf(( "Priority: LOW\n" ));
//  					joy_unacquire_ff();
//  					if (Mouse_hidden)	{
//  						Mouse_hidden=0;
//  						//ShowCursor(1);
//  						//mprintf(( "Mouse:Alive\n" ));		
//  					}
//  					// minimize it
//  					// key_outkey( KEY_PAUSE );

//  #ifdef THREADED
//  					SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );
//  #endif
//  					gr_activate(fAppActive);
//  				}
//  			}
//  		}
//  		break;

//  	case WM_DESTROY:
//  		// mprintf(( "WM_DESTROY called\n" ));
//  		PostQuitMessage(0);
//  		break;

//  	case WM_CLOSE:
//  		gameseq_post_event(GS_EVENT_QUIT_GAME);
//  		break;

//  	case WM_SYSCOMMAND:
//  		// mprintf(( "Sys command called '%x'\n", wParam ));
//  		 if ( wParam != SC_SCREENSAVE ){
//  			 return DefWindowProc(hwnd, msg, wParam, lParam);
//  		 }
//  		 break;

//  /*
//  	case MM_WIM_DATA:
//  		rtvoice_stream_data((uint)hwnd, (uint)wParam, (uint)lParam);
//  		break;
//  */

//  	default:
//  		return DefWindowProc(hwnd, msg, wParam, lParam);
//  		break;
//  	}

//  	return 0;
//  }

// create the main window
BOOL unix_create_window()
{
//  	int windowed = 0;
//  	int hires = 0;
//  	char *ptr = os_config_read_string(NULL, NOX("Videocard"), NULL);	
//  	if(ptr && strstr(ptr, NOX("Direct 3D -")) && Cmdline_window){
//  		windowed = 1;
//  	}
//  	if(ptr && strstr(ptr, NOX("1024")) && Cmdline_window){
//  		hires = 1;
//  	}

//  	// make sure we adjust for the actual window border	
//  	int x_add = 0;
//  	int y_add = 0;
//  	x_add += GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
//  	y_add += GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);

//  	// Make a correctly sized window.  It never should get shown, because the graphics code
//  	// will then size it.
//  	if(windowed){
//  		  int width, height;
//  		  if (hires) {
//  				 width  = 1024 + x_add;
//  				 height =  768 + y_add;
//  		  } else {
//  				 width  =  640 + x_add;
//  				 height =  480 + y_add;
//  		  }
//  		  hwndApp = CreateWindow( szWinClass, szWinTitle,
//  										  style,   
//  										  CW_USEDEFAULT,
//  										  CW_USEDEFAULT,
//  										  width,
//  										  height,
//  										  NULL, (HMENU)NULL, hInst, (LPSTR)NULL);	
//  	} else {
//  		// Make a 32x32 window.  It never should get shown, because the graphics code
//  		// will then size it.
//  		hwndApp = CreateWindow( szWinClass, szWinTitle,
//  									style,   
//  									(GetSystemMetrics( SM_CXSCREEN )-32 )/2,	//x
//  									(GetSystemMetrics( SM_CYSCREEN )-32 )/2,	//y
//  									32,32,									// wh
//  									NULL, (HMENU)NULL, hInst, (LPSTR)NULL);	
//  	}

//  	// Hack!! Turn off Window's cursor.
//  	ShowCursor(0);
//  	ClipCursor(NULL);

//  	main_window_inited = 1;
//  	#ifndef NDEBUG
//  		outwnd_init(1);
//  	#endif	

//  	if(windowed){
//  		ShowWindow( hwndApp, SW_SHOWNORMAL );
//  		UpdateWindow( hwndApp );
//  	}

	return TRUE;
}

void os_poll()
{
#ifndef THREADED
	unix_process2(0);
#else
	MSG msg;
	EnterCriticalSection(&Os_lock);
	while(PeekMessage(&msg,0,0,0,PM_NOREMOVE))	{		
		if ( msg.message == WM_DESTROY )	{
			break;
		}
		if (PeekMessage(&msg,0,0,0,PM_REMOVE))	{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}		
		Got_message++;
	}
	LeaveCriticalSection(&Os_lock);
#endif
}

void debug_int3()
{
   abort();
}
