/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <winsock.h>
#include <stdarg.h>
#include <direct.h>

#include "globalincs/pstypes.h"
#include "io/key.h"
#include "palman/palman.h"
#include "io/mouse.h"
#include "osapi/outwnd.h"
#include "sound/sound.h"
#include "freespace2/freespaceresource.h"
#include "playerman/managepilot.h"
#include "io/joy.h"
#include "io/joy_ff.h"
#include "gamesequence/gamesequence.h"
#include "freespace2/freespace.h"
#include "osapi/osregistry.h"
#include "cmdline/cmdline.h"
#include "sound/voicerec.h"
#include "graphics/2d.h"

#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"


// used to be a THREADED define but only use multiple process threads if this is defined
// NOTE: may hang if set
//#define THREADED_PROCESS


// ----------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

// os-wide globals
static HINSTANCE	hInstApp;
static HWND			hwndApp = NULL;
static HDC			dcApp = NULL;
static int			fAppActive = 0;
static int			fOldAppActive = 0;
static int			main_window_inited = 0;
static char			szWinTitle[128];
static char			szWinClass[128];
static HANDLE		hThread=NULL;
static DWORD		ThreadID;
static int			WinX, WinY, WinW, WinH;
static int			Os_inited = 0;

static CRITICAL_SECTION Os_lock;

int Os_debugger_running = 0;

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

#ifdef THREADED_PROCESS
	// thread handler for the main message thread
	DWORD win32_process(DWORD lparam);
#else
	DWORD win32_process1(DWORD lparam);
	DWORD win32_process2(DWORD lparam);
#endif

// Fills in the Os_debugger_running with non-zero if debugger detected.
void os_check_debugger();

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit();

// go through all windows and try and find the one that matches the search string
BOOL __stdcall os_enum_windows( HWND hwnd, char * search_string );

// message handler for the main thread
LRESULT CALLBACK win32_message_handler(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam);


// ----------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// detect home/base directory  (placeholder for possible future Win32 userdir support, just returns current directory for now)
char Cur_path[MAX_PATH_LEN];
const char *detect_home(void)
{
	if ( strlen(Cfile_root_dir) )
		return Cfile_root_dir;

	memset( Cur_path, 0, MAX_PATH_LEN );
	GetCurrentDirectory( MAX_PATH_LEN-1, Cur_path );

	return Cur_path;
}

// initialization/shutdown functions -----------------------------------------------

void os_set_process_affinity()
{
	HANDLE pHandle = GetCurrentProcess();
	DWORD pMaskProcess = 0, pMaskSystem = 0;

	if ( GetProcessAffinityMask(pHandle, &pMaskProcess, &pMaskSystem) ) {
		// only do this if we have at least 2 procs
		if (pMaskProcess >= 3) {
			// prefer running on the second processor by default
			pMaskProcess = os_config_read_uint(NULL, "ProcessorAffinity", 2);

			if (pMaskProcess > 0) {
				SetProcessAffinityMask(pHandle, pMaskProcess);
			}
		}
	}
}

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(const char * wclass, const char * title, const char *app_name, const char *version_string )
{
	os_init_registry_stuff(Osreg_company_name, title, version_string);

	strcpy_s( szWinTitle, title );
	strcpy_s( szWinClass, wclass );	

	INITIALIZE_CRITICAL_SECTION( Os_lock );
/*
	#ifdef THREADED_PROCESS
		// Create an even to signal that the window is created, 
		// so that we don't return from this function until 
		// the window is all properly created.
		HANDLE Window_created = CreateEvent( NULL, FALSE, FALSE, NULL );
		hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)win32_process, Window_created, 0, &ThreadID );
		if ( WaitForSingleObject( Window_created, 5000 )==WAIT_TIMEOUT)	{			//INFINITE );
			mprintf(( "Wait timeout!\n" ));
		}
		CloseHandle(Window_created);
		Window_created = NULL;
	#endif // THREADED
*/
	// initialized
	Os_inited = 1;

	// check to see if we're running under msdev
	os_check_debugger();

	// deal with processor affinity
	os_set_process_affinity();

	atexit(os_deinit);
}

// set the main window title
void os_set_title( const char * title )
{
	strcpy_s( szWinTitle, title );
	SetWindowText( hwndApp, szWinTitle );
}

extern void gr_opengl_shutdown();

// call at program end
void os_cleanup()
{
	if (gr_screen.mode == GR_OPENGL)
		gr_opengl_shutdown();

	if (dcApp != NULL) {
		ReleaseDC( hwndApp, dcApp );
	}

	// destroy the window (takes care of a lot of window related cleanup and sys messages)
	DestroyWindow( hwndApp );

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

// Returns the handle to the main window
uint os_get_window()
{
	return (uint)hwndApp;
}

uint os_get_dc()
{
	if (dcApp == NULL) {
		dcApp = GetDC(hwndApp);
	}

	return (uint)dcApp;
}

// Returns the handle to the main window
void os_set_window(uint new_handle)
{
	hwndApp = (HWND)new_handle;
}


// process management -----------------------------------------------------------------

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(int ms)
{
	Sleep(ms);
}

// Used to stop message processing
void os_suspend()
{
	ENTER_CRITICAL_SECTION( Os_lock );	
}

// resume message processing
void os_resume()
{
	LEAVE_CRITICAL_SECTION( Os_lock );	
}


// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

#ifdef THREADED_PROCESS

// thread handler for the main message thread
DWORD win32_process(DWORD lparam)
{
/*	MSG msg;
	HANDLE Window_created = (HANDLE)lparam;

	if ( !win32_create_window() )
		return 0;

	// Let the app continue once the window is created
	SetEvent(Window_created);

	while (1)	{	
		if (WaitMessage() == TRUE)	{
			ENTER_CRITICAL_SECTION( Os_lock );			
			while(PeekMessage(&msg,0,0,0,PM_REMOVE))	{
				if ( msg.message == WM_DESTROY )	{
					LEAVE_CRITICAL_SECTION( Os_lock );

					// cleanup and exit this thread!!
					DELETE_CRITICAL_SECTION( Os_lock );
					return 0;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			LEAVE_CRITICAL_SECTION( Os_lock );
		} 
	}*/

	return 0;
}

#else

DWORD win32_process(DWORD lparam)
{
	MSG msg;

	while ( PeekMessage(&msg, 0, 0, 0, PM_REMOVE) ) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
#endif // THREADED_PROCESS

// Fills in the Os_debugger_running with non-zero if debugger detected.
void os_check_debugger()
{
	HMODULE hMod;
	char search_string[256];
	char myname[128];
	int namelen;
	char * p;

	Os_debugger_running = 0;		// Assume its not

	// Find my EXE file name
	hMod = GetModuleHandle(NULL);
	if ( !hMod ) return;
	namelen = GetModuleFileName( hMod, myname, 127 );	
	if ( namelen < 1 ) return;
	
	// Strip off the .EXE
	p = strstr( myname, ".exe" );
	if (!p) return;
	*p = '\0';

	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') )
		p--;
	p++;	
	if ( strlen(p) < 1 ) return;

	// Build what the debugger's window title would be if the debugger is running...
	sprintf( search_string, "[run] - %s -", p );

	// ... and then search for it.
	EnumWindows( (int (__stdcall *)(struct HWND__ *,long))os_enum_windows, (long)&search_string );
}

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit()
{
	if (hThread)	{
		CloseHandle(hThread);
		hThread = NULL;
	}
}

// go through all windows and try and find the one that matches the search string
BOOL __stdcall os_enum_windows( HWND hwnd, char * search_string )
{
	char tmp[128];
	int len;

	len = GetWindowText( hwnd, tmp, 127 );
	 
	if ( len )	{
		if ( strstr( tmp, search_string ))	{
			Os_debugger_running = 1;		// found the search string!
			return FALSE;	// stop enumerating windows
		}
	}

	return TRUE;	// continue enumeration
}

void change_window_active_state()
{
	if (fAppActive != fOldAppActive) {
		if (fAppActive) {
			// maximize it
			joy_reacquire_ff();

			if (!Cmdline_no_unfocus_pause)
			{
				game_unpause();
			}

#ifdef THREADED_PROCESS
			SetThreadPriority( hThread, THREAD_PRIORITY_HIGHEST );
#endif

			if ( !Is_standalone )
				disableWindowsKey();

            if (!Cmdline_window)
            {
                SetWindowPos(hwndApp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
		}
		else {
			joy_unacquire_ff();

			if (Mouse_hidden)
				Mouse_hidden = 0;

			if(!Cmdline_no_unfocus_pause)
			{
				// Pause sounds and put up pause screen if necessary
				game_pause();
			}

#ifdef THREADED_PROCESS
			SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );
#endif

			if ( !Is_standalone )
				enableWindowsKey();

            if (!Cmdline_window)
            {
                SetWindowPos(hwndApp, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
		}

		if (!Cmdline_no_unfocus_pause)
		{
			gr_activate(fAppActive);
		}

		fOldAppActive = fAppActive;
	}
}

int Got_message = 0;
extern bool Messagebox_active;
// message handler for the main thread
LRESULT CALLBACK win32_message_handler(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam)
{
	// Got_message++;

	switch(msg)	{

	case WM_QUERYNEWPALETTE:
		// mprintf(( "WM: QueryNewPalette\n" ));
		return TRUE;	// Say that I've realized my own palette
		break;
	case WM_PALETTECHANGED:
		// mprintf(( "WM: PaletteChanged\n" ));
		break;
	case WM_PALETTEISCHANGING:
		// mprintf(( "WM: PaletteIsChanging\n" ));
		break;

	case WM_DISPLAYCHANGE:
		// mprintf(( "WM: DisplayChange\n" ));
		break;

	case WM_LBUTTONDOWN:
		mouse_mark_button( MOUSE_LEFT_BUTTON, 1 );
		break;

	case WM_LBUTTONUP:
		mouse_mark_button( MOUSE_LEFT_BUTTON, 0 );
		break;

	case WM_RBUTTONDOWN:
		mouse_mark_button( MOUSE_RIGHT_BUTTON, 1 );
		break;

	case WM_RBUTTONUP:
		mouse_mark_button( MOUSE_RIGHT_BUTTON, 0 );
		break;

	case WM_MBUTTONDOWN:
		mouse_mark_button( MOUSE_MIDDLE_BUTTON, 1 );
		break;

	case WM_MBUTTONUP:
		mouse_mark_button( MOUSE_MIDDLE_BUTTON, 0 );
		break;

	case WM_TIMER:
		break;

    case WM_SYSCHAR:
        break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:	{		
			int nVirtKey;
			uint lKeyData;

			int latency;
			latency = timeGetTime() - GetMessageTime();
			if ( latency < 0 )
				latency=0;

			nVirtKey = (int)wParam;    // virtual-key code 
			lKeyData = (lParam>>16) & 255;          // key data 
			if ( (lParam>>16) & 256 ) lKeyData += 0x80;

			// Fix up print screen, whose OEM code is wrong under 95.
			if ( nVirtKey == VK_SNAPSHOT )	{
				lKeyData = KEY_PRINT_SCRN;	
			}

			if (lKeyData == KEY_RSHIFT)  // either shift is just a shift to us..
				lKeyData = KEY_LSHIFT;

			if (lKeyData == KEY_RALT)  // Same with alt keys..
				lKeyData = KEY_LALT;

//			mprintf(( "Key down = 0x%x|%x\n", lKeyData, nVirtKey ));
			key_mark( lKeyData, 1, latency );
//			mprintf(( "Key down = 0x%x\n", lKeyData ));
			//Warning( LOCATION, "Key = 0x%x", lKeyData );			
		}
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:	 {		
			int nVirtKey;
			uint lKeyData;

			int latency;
			latency = timeGetTime() - GetMessageTime();
			if ( latency < 0 )
				latency=0;

			nVirtKey = (int) wParam;    // virtual-key code 
			lKeyData = (lParam>>16) & 255;          // key data 
			if ( (lParam>>16) & 256 ) lKeyData += 0x80;

			// Fix up print screen, whose OEM code is wrong under 95.
			if ( nVirtKey == VK_SNAPSHOT )	{
				lKeyData = KEY_PRINT_SCRN;	
			}

			if (lKeyData == KEY_RSHIFT)  // either shift is just a shift to us..
				lKeyData = KEY_LSHIFT;

			if (lKeyData == KEY_RALT)  // Same with alt keys..
				lKeyData = KEY_LALT;

//			mprintf(( "Key up = 0x%x|%x\n", lKeyData, nVirtKey ));
			if ( lKeyData == 0xB7 )	{
				// Hack for PrintScreen which only sends one up message!
				key_mark( lKeyData, 1, latency );		
				key_mark( lKeyData, 0, latency );

			} else {
				key_mark( lKeyData, 0, latency );
			}			
		}		
		break;

	case WM_KILLFOCUS:
		{
			if (Messagebox_active)
				break;

			key_lost_focus();
            mouse_lost_focus();
			if ( !Is_standalone )
				gr_activate(0);
			break;
		}

	case WM_SETFOCUS:
		{
			if (Messagebox_active)
				break;

			key_got_focus();
            mouse_got_focus();
			if ( !Is_standalone )
				gr_activate(1);
			break;
		}


	case WM_ACTIVATE:		   
	{
		if (Messagebox_active)
			break;

		int flag = LOWORD(wParam);
		fAppActive = (( flag == WA_ACTIVE) || (flag==WA_CLICKACTIVE)) ? TRUE : FALSE;
		change_window_active_state();
		break;
	}

	case WM_ACTIVATEAPP:
		if (Messagebox_active)
			break;

		fAppActive = (BOOL)wParam;
		change_window_active_state();
		break;

	case WM_DESTROY:
		// mprintf(( "WM_DESTROY called\n" ));
		PostQuitMessage(0);
		break;

	case WM_CLOSE:
		gameseq_post_event(GS_EVENT_QUIT_GAME);
		break;

	case WM_SYSCOMMAND:
		// mprintf(( "Sys command called '%x'\n", wParam ));
		 if ( wParam != SC_SCREENSAVE ){
			 return DefWindowProc(hwnd, msg, wParam, lParam);
		 }
		 break;

/*
	case MM_WIM_DATA:
		rtvoice_stream_data((uint)hwnd, (uint)wParam, (uint)lParam);
		break;
*/
#ifdef FS2_VOICER
    case WM_RECOEVENT:
		if ( Game_mode & GM_IN_MISSION && Cmdline_voice_recognition)
		{
			VOICEREC_process_event( hwnd );
		}
        break;
#endif

	// report back that we handle this ourselves (with gr_clear()) in order to
	// prevent flickering (especially with movies)
	case WM_ERASEBKGND:
		return TRUE;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return 0;
}

// create the main window
void win32_create_window(int width, int height)
{
	WNDCLASSEX wclass;							// Huh?
	HINSTANCE hInst = GetModuleHandle(NULL);

	if (hwndApp != NULL) {
		if (dcApp != NULL) {
			ReleaseDC( hwndApp, dcApp );
			dcApp = NULL;
		}

		DestroyWindow( hwndApp );
		hwndApp = NULL;
	}

	memset( &wclass, 0, sizeof(WNDCLASSEX) );

	wclass.hInstance 		= hInst;
	wclass.lpszClassName	= szWinClass;
	wclass.lpfnWndProc		= (WNDPROC)win32_message_handler;	  

//	if (Cmdline_window) {
//		wclass.style			= CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
//	} else {
//		wclass.style			= CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW;
//	}
	wclass.style			= CS_OWNDC;	// using CS_OWNDC for better Win9x/WinME support (I think it's implied with WinNT+)

	wclass.cbSize			= sizeof(WNDCLASSEX);
	wclass.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP_ICON) );
	wclass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wclass.lpszMenuName		= NULL;	//"FreeSpaceMenu";
	wclass.cbClsExtra		= 0;
	wclass.cbWndExtra		= 0;
	// set background to erase/clear with a black brush
	// (NULL means that we had to do it ourselves, and created a white-screen problem)
	wclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);

	if ( !RegisterClassEx(&wclass) ) {
		Error( LOCATION, "FATAL ERROR:  Unable to register window class!!" );
	}

	int style = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;

	if (Cmdline_window)
		style |= (WS_CAPTION | WS_SYSMENU | WS_BORDER);

	int x_add, y_add;
	int start_x, start_y;

	if (Cmdline_window || Cmdline_fullscreen_window) {
		RECT my_rect;

		// make sure we adjust for the actual window border	
		if (Cmdline_window) {
			x_add = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
			y_add = 2 * GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
		} else {
			x_add = y_add = 0;
		}

		GetWindowRect( GetDesktopWindow(), &my_rect );

		start_x = (my_rect.right - width - x_add) / 2;
		start_y = (my_rect.bottom - height - y_add) / 2;

		if (start_x < 0)
			start_x = 0;
		if (start_y < 0)
			start_y = 0;
	} else {
		x_add = y_add = 0;
		start_x = start_y = 0;
	}

	// we don't sicky TOPMOST for windowed mode since we wouldn't be able to bring
	// the debug window (or anything else) to the true foreground otherwise
	hwndApp = CreateWindowEx( (Cmdline_window || Cmdline_fullscreen_window) ? 0 : WS_EX_TOPMOST,
								szWinClass, szWinTitle,
								style,   
								start_x,		// x
								start_y,		// y
								width + x_add,	// w
								height + y_add,	// h
								NULL, (HMENU)NULL, hInst,
								(LPSTR)NULL );

	if ( !hwndApp ) {
		Error( LOCATION, "FATAL ERROR:  Unable to create game window!!" );
	}

	main_window_inited = 1;

	win32_process(0);

#ifndef NDEBUG
	extern void outwnd_init_debug_window(int);
	outwnd_init_debug_window(1);
#endif

	ShowWindow( hwndApp, SW_SHOWNORMAL );

	SetForegroundWindow( hwndApp );
	SetActiveWindow( hwndApp );
	SetFocus( hwndApp );

	// Hack!! Turn off Window's cursor.
//	ShowCursor(false);
//	ClipCursor(NULL);

	return;// TRUE;
}

void os_poll()
{
#ifndef THREADED_PROCESS
	win32_process(0);
#else
	MSG msg;
	ENTER_CRITICAL_SECTION( Os_lock );
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
	LEAVE_CRITICAL_SECTION( Os_lock );
#endif
}

void debug_int3(char *file, int line)
{
	mprintf(("Int3(): From %s at line %d\n", file, line));

	gr_activate(0);

#ifdef _WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400
	__debugbreak( );
#elif defined(_MSC_VER)
	_asm int 3;
#elif defined __GNUC__
	asm("int $3");
#else
#error debug_int3: unknown compiler
#endif

#else
#error debug_int3: unknown OS
#endif

	gr_activate(1);

}


// Goober5000 - code provided by jr2 to disable windows key when FSO is in the foreground

#ifdef _WIN32

static HHOOK g_hKeyboardHook = NULL;

// ugh
#ifndef WH_KEYBOARD_LL
  #define WH_KEYBOARD_LL	13
#endif

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0 || nCode != HC_ACTION)  // do not process message 
		return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam); 

	// hack!
	// this is because the KBDLLHOOKSTRUCT type requires a mess of #includes,
	// but all we need from it is the first field
	DWORD vkCode = *( (DWORD *) lParam );

	// determine key event
	switch (wParam) 
	{
		case WM_KEYDOWN:  
		case WM_KEYUP:    
			if ( (vkCode == VK_LWIN) || (vkCode == VK_RWIN) )
				return 1;
	}

	return CallNextHookEx( g_hKeyboardHook, nCode, wParam, lParam );
}

void disableWindowsKey()
{
	if (g_hKeyboardHook != NULL)
		return;

	g_hKeyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL,  LowLevelKeyboardProc, GetModuleHandle(NULL), 0 );
}

void enableWindowsKey()
{
	if (g_hKeyboardHook == NULL)
		return;

	UnhookWindowsHookEx( g_hKeyboardHook );
	g_hKeyboardHook = NULL;
}

#endif // _WIN32
