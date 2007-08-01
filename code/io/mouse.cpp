/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Io/Mouse.cpp $
 * $Revision: 2.16 $
 * $Date: 2007-08-01 12:25:51 $
 * $Author: turey $
 *
 * Routines to read the mouse.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.15  2006/03/15 17:35:24  taylor
 * make sure our event state doesn't get filled with things we should poll for (was waiting for the new input code, but really needed to get at least this in)
 *
 * Revision 2.14  2006/01/12 04:18:10  wmcoolmon
 * Finished committing codebase
 *
 * Revision 2.13  2005/07/18 03:45:07  taylor
 * more non-standard res fixing
 *  - I think everything should default to resize now (much easier than having to figure that crap out)
 *  - new mouse_get_pos_unscaled() function to return 1024x768/640x480 relative values so we don't have to do it later
 *  - lots of little cleanups which fix several strange offset/size problems
 *  - fix gr_resize/unsize_screen_pos() so that it won't wrap on int (took too long to track this down)
 *
 * Revision 2.12  2005/06/19 02:48:13  taylor
 * remove xmouse.cpp since it's pretty much useless
 *
 * Revision 2.11  2005/03/14 03:38:54  taylor
 * fix for mouse cursor problem when running in a window
 *
 * Revision 2.10  2005/03/03 16:18:19  taylor
 * lockup fixes, and it's Linux friendly too :)
 *
 * Revision 2.9  2005/03/03 06:05:28  wmcoolmon
 * Merge of WMC's codebase. "Features and bugs, making Goober say "Grr!", as release would be stalled now for two months for sure"
 *
 * Revision 2.8  2005/02/04 10:12:30  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.7  2004/07/26 20:47:33  Kazan
 * remove MCD complete
 *
 * Revision 2.6  2004/07/12 16:32:51  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.5  2004/02/14 00:18:33  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.4  2004/02/04 09:02:44  Goober5000
 * got rid of unnecessary double semicolons
 * --Goober5000
 *
 * Revision 2.3  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/29 20:12:31  penguin
 * added #ifdef _WIN32 around windows-specific system headers
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.3  2002/05/21 15:43:05  mharris
 * Cosmetics - removed commented-out code
 *
 * Revision 1.2  2002/05/17 03:04:09  mharris
 * Make mouse routines more portable
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     7/15/99 9:20a Andsager
 * FS2_DEMO initial checkin
 * 
 * 3     6/02/99 6:18p Dave
 * Fixed TNT lockup problems! Wheeeee!
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 29    6/10/98 2:52p Hoffoss
 * Made mouse code use DI by default, but fall back on normal code if that
 * fails.
 * 
 * 28    5/24/98 1:35p Hoffoss
 * Fixed bug where  mouse cursor is always recentering with a
 * mouse_flush() call in debug version.
 * 
 * 27    5/22/98 4:50p Hoffoss
 * Trying to fix mouse acceleration problem..
 * 
 * 26    5/21/98 12:26p Lawrance
 * Fixed mouse jerk at mission start while in debug build only.
 * 
 * 25    5/15/98 2:41p Hoffoss
 * Made mouse default to off (for flying ship) and fixed some new pilot
 * init bugs.
 * 
 * 24    5/08/98 4:13p Hoffoss
 * Fixed problem with mouse pointer centering causing lost keypresses.
 * 
 * 23    5/07/98 6:58p Hoffoss
 * Made changes to mouse code to fix a number of problems.
 * 
 * 22    5/05/98 8:38p Hoffoss
 * Added sensitivity adjustment to options menu and made it save to pilot
 * file.
 * 
 * 21    5/05/98 1:03p Hoffoss
 * Fixed initialization bug.
 * 
 * 20    5/01/98 5:45p Hoffoss
 * Made further improvements to the mouse code.
 * 
 * 19    5/01/98 3:35p Hoffoss
 * Made changes to release version of mouse code.
 * 
 * 18    5/01/98 1:14p Hoffoss
 * Changed mouse usage so directInput is only used for release version.
 * 
 * 17    4/30/98 5:40p Hoffoss
 * Added mouse as a supported control to fly the ship.
 * 
 * 16    4/29/98 12:13a Lawrance
 * Add function to check down count of mouse button without reseting the
 * internal count.  Added hook to reset demo trailer timer when a button
 * is pressed.
 * 
 * 15    4/02/98 5:26p John
 * 
 * 14    1/19/98 6:15p John
 * Fixed all my Optimized Build compiler warnings
 * 
 * 13    12/04/97 3:47p John
 * Made joystick move mouse cursor
 * 
 * 12    11/20/97 5:36p Dave
 * Hooked in a bunch of main hall changes (including sound). Made it
 * possible to reposition (rewind/ffwd) 
 * sound buffer pointers. Fixed animation direction change framerate
 * problem.
 * 
 * 11    5/12/97 11:41a John
 * Added range checking to mouse position
 * 
 * 10    4/22/97 5:55p Lawrance
 * let mouse.cpp decide if mouse has moved
 * 
 * 9     4/22/97 12:29p John
 * Changed mouse code so that you have to call mouse_init for the mouse
 * stuff to work.
 * 
 * 8     4/22/97 10:56a John
 * fixed some resource leaks.
 * 
 * 7     3/26/97 10:52a Lawrance
 * mouse always on in menus, disappears in gameplay after 1 second
 * 
 * 6     3/11/97 1:37p Lawrance
 * added mouse_up_count(), changed mouse_mark() to mouse_mark_button() &
 * mouse_mark_move()
 * 
 * 5     12/09/96 1:29p Lawrance
 * adding 3 button support
 * 
 * 4     12/03/96 4:19p John
 * Added some code so that holding down the mouse buttons between menus
 * doesn't select the next menu.
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include "io/mouse.h"
#include "graphics/2d.h"

#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"

#ifdef WIN32
#define USE_DIRECTINPUT
#endif

#define MOUSE_MODE_DI	0
#define MOUSE_MODE_WIN	1

#ifdef USE_DIRECTINPUT
#ifdef NDEBUG
LOCAL int Mouse_mode = MOUSE_MODE_DI;
#else
LOCAL int Mouse_mode = MOUSE_MODE_WIN;
#endif
#else // !USE_DIRECTINPUT
LOCAL int Mouse_mode = MOUSE_MODE_WIN;
#endif // ifdef USE_DIRECTINPUT

int mouse_inited = 0;
#ifdef USE_DIRECTINPUT
LOCAL int Di_mouse_inited = 0;
#endif
LOCAL int Mouse_x;
LOCAL int Mouse_y;

CRITICAL_SECTION mouse_lock;

int mouse_flags;
int mouse_left_pressed = 0;
int mouse_right_pressed = 0;
int mouse_middle_pressed = 0;
int mouse_left_up = 0;
int mouse_right_up = 0;
int mouse_middle_up = 0;
int Mouse_dx = 0;
int Mouse_dy = 0;
int Mouse_dz = 0;

int Mouse_sensitivity = 4;
int Use_mouse_to_fly = 0;
int Mouse_hidden = 0;
int Keep_mouse_centered = 0;

#ifdef USE_DIRECTINPUT
int di_init();
void di_cleanup();
void mouse_eval_deltas_di();
#endif

extern int Cmdline_window;

void mouse_force_pos(int x, int y);

int mouse_is_visible()
{
	return !Mouse_hidden;
}

void mouse_close()
{
	if (!mouse_inited)
		return;

#ifdef USE_DIRECTINPUT
	di_cleanup();
#endif
	mouse_inited = 0;

	DELETE_CRITICAL_SECTION( mouse_lock );
}

void mouse_init()
{
	// Initialize queue
	if ( mouse_inited ) return;
	mouse_inited = 1;

	INITIALIZE_CRITICAL_SECTION( mouse_lock );

	ENTER_CRITICAL_SECTION( mouse_lock );

	mouse_flags = 0;
	Mouse_x = gr_screen.max_w / 2;
	Mouse_y = gr_screen.max_h / 2;

#ifdef USE_DIRECTINPUT
	if (!di_init() || Cmdline_window)
		Mouse_mode = MOUSE_MODE_WIN;
#else
	Mouse_mode = MOUSE_MODE_WIN;
#endif

#ifdef SCP_UNIX
	// we poll for mouse motion events so be sure to skip those in normal event polling
	SDL_EventState( SDL_MOUSEMOTION, SDL_IGNORE );

	// we do want to make sure that button presses go through event polling though
	// (should be on by default already, just here as a reminder)
	SDL_EventState( SDL_MOUSEBUTTONDOWN, SDL_ENABLE );
	SDL_EventState( SDL_MOUSEBUTTONUP, SDL_ENABLE );
#endif

	LEAVE_CRITICAL_SECTION( mouse_lock );	

	atexit( mouse_close );
}


// ----------------------------------------------------------------------------
// mouse_mark_button() is called asynchronously by the OS when a mouse button
// goes up or down.  The mouse button that is affected is passed via the 
// flags parameter.  
//
// parameters:   flags ==> mouse button pressed/released
//               set   ==> 1 - button is pressed
//                         0 - button is released

void mouse_mark_button( uint flags, int set)
{
	if ( !mouse_inited ) return;

	ENTER_CRITICAL_SECTION( mouse_lock );

	if ( !(mouse_flags & MOUSE_LEFT_BUTTON) )	{

		if ( (flags & MOUSE_LEFT_BUTTON) && (set == 1) ) {
			mouse_left_pressed++;

////////////////////////////
/// SOMETHING TERRIBLE IS ABOUT TO HAPPEN.  I FEEL THIS IS NECESSARY FOR THE DEMO, SINCE
/// I DON'T WANT TO CALL CRITICAL SECTION CODE EACH FRAME TO CHECK THE LEFT MOUSE BUTTON.
/// PLEASE SEE ALAN FOR MORE INFORMATION.
////////////////////////////
#ifdef FS2_DEMO
					{
					extern void demo_reset_trailer_timer();
					demo_reset_trailer_timer();
					}
#endif
////////////////////////////
/// IT'S OVER.  SEE, IT WASN'T SO BAD RIGHT?  IT'S IS VERY UGLY LOOKING, I KNOW.
////////////////////////////

		}
	}
	else {
		if ( (flags & MOUSE_LEFT_BUTTON) && (set == 0) ){
			mouse_left_up++;
		}
	}

	if ( !(mouse_flags & MOUSE_RIGHT_BUTTON) )	{

		if ( (flags & MOUSE_RIGHT_BUTTON) && (set == 1) ){
			mouse_right_pressed++;
		}
	}
	else {
		if ( (flags & MOUSE_RIGHT_BUTTON) && (set == 0) ){
			mouse_right_up++;
		}
	}

	if ( !(mouse_flags & MOUSE_MIDDLE_BUTTON) )	{

		if ( (flags & MOUSE_MIDDLE_BUTTON) && (set == 1) ){
			mouse_middle_pressed++;
		}
	}
	else {
		if ( (flags & MOUSE_MIDDLE_BUTTON) && (set == 0) ){
			mouse_middle_up++;
		}
	}

	if ( set ){
		mouse_flags |= flags;
	} else {
		mouse_flags &= ~flags;
	}

	LEAVE_CRITICAL_SECTION( mouse_lock );	
}

void mouse_flush()
{
	if (!mouse_inited)
		return;

	mouse_eval_deltas();
	Mouse_dx = Mouse_dy = Mouse_dz = 0;
	ENTER_CRITICAL_SECTION( mouse_lock );
	mouse_left_pressed = 0;
	mouse_right_pressed = 0;
	mouse_middle_pressed = 0;
	mouse_flags = 0;
	LEAVE_CRITICAL_SECTION( mouse_lock );	
}

int mouse_down_count(int n, int reset_count)
{
	int tmp = 0;
	if ( !mouse_inited ) return 0;

	if ( (n < LOWEST_MOUSE_BUTTON) || (n > HIGHEST_MOUSE_BUTTON)) return 0;

	ENTER_CRITICAL_SECTION( mouse_lock );

	switch (n) {
		case MOUSE_LEFT_BUTTON:
			tmp = mouse_left_pressed;
			if ( reset_count ) {
				mouse_left_pressed = 0;
			}
			break;

		case MOUSE_RIGHT_BUTTON:
			tmp = mouse_right_pressed;
			if ( reset_count ) {
				mouse_right_pressed = 0;
			}
			break;

		case MOUSE_MIDDLE_BUTTON:
			tmp = mouse_middle_pressed;
			if ( reset_count ) {
				mouse_middle_pressed = 0;
			}
			break;
	} // end switch

	LEAVE_CRITICAL_SECTION( mouse_lock );	

	return tmp;
}

// mouse_up_count() returns the number of times button n has gone from down to up
// since the last call
//
// parameters:  n - button of mouse (see #define's in mouse.h)
//
int mouse_up_count(int n)
{
	int tmp = 0;
	if ( !mouse_inited ) return 0;

	if ( (n < LOWEST_MOUSE_BUTTON) || (n > HIGHEST_MOUSE_BUTTON)) return 0;

	ENTER_CRITICAL_SECTION( mouse_lock );

	switch (n) {
		case MOUSE_LEFT_BUTTON:
			tmp = mouse_left_up;
			mouse_left_up = 0;
			break;

		case MOUSE_RIGHT_BUTTON:
			tmp = mouse_right_up;
			mouse_right_up = 0;
			break;

		case MOUSE_MIDDLE_BUTTON:
			tmp = mouse_middle_up;
			mouse_middle_up = 0;
			break;

		default:
			Assert(0);	// can't happen
			break;
	} // end switch

	LEAVE_CRITICAL_SECTION( mouse_lock );	

	return tmp;
}

// returns 1 if mouse button btn is down, 0 otherwise

int mouse_down(int btn)
{
	int tmp;
	if ( !mouse_inited ) return 0;

	if ( (btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_BUTTON)) return 0;


	ENTER_CRITICAL_SECTION( mouse_lock );


	if ( mouse_flags & btn )
		tmp = 1;
	else
		tmp = 0;

	LEAVE_CRITICAL_SECTION( mouse_lock );	

	return tmp;
}

// returns the fraction of time btn has been down since last call 
// (currently returns 1 if buttons is down, 0 otherwise)
//
float mouse_down_time(int btn)
{
	float tmp;
	if ( !mouse_inited ) return 0.0f;

	if ( (btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_BUTTON)) return 0.0f;

	ENTER_CRITICAL_SECTION( mouse_lock );

	if ( mouse_flags & btn )
		tmp = 1.0f;
	else
		tmp = 0.0f;

	LEAVE_CRITICAL_SECTION( mouse_lock );

	return tmp;
}

void mouse_get_delta(int *dx, int *dy, int *dz)
{
	if (dx)
		*dx = Mouse_dx;
	if (dy)
		*dy = Mouse_dy;
	if (dz)
		*dz = Mouse_dz;
}

// Forces the actual windows cursor to be at (x,y).  This may be independent of our tracked (x,y) mouse pos.
void mouse_force_pos(int x, int y)
{
	if (os_foreground()) {  // only mess with windows's mouse if we are in control of it
		POINT pnt;

		pnt.x = x;
		pnt.y = y;
		setWindowMousePos(&pnt);
	}
}

#include "gamesequence/gamesequence.h"

// change in mouse position since last call
void mouse_eval_deltas()
{
	static int old_x = 0;
	static int old_y = 0;
	int tmp_x, tmp_y, cx, cy;

	Mouse_dx = Mouse_dy = Mouse_dz = 0;
	if (!mouse_inited)
		return;

#ifdef USE_DIRECTINPUT
	if (Mouse_mode == MOUSE_MODE_DI) {
		mouse_eval_deltas_di();
		return;
	}
#endif

	cx = gr_screen.max_w / 2;
	cy = gr_screen.max_h / 2;

	ENTER_CRITICAL_SECTION( mouse_lock );

	POINT pnt;
	getWindowMousePos(&pnt);
	tmp_x = pnt.x;
	tmp_y = pnt.y;

	Mouse_dx = tmp_x - old_x;
	Mouse_dy = tmp_y - old_y;
	Mouse_dz = 0;

	// Speeds up the menu mouse on higher resolutions. The check for a
	// visible mouse should eliminate any possible gameplay changes.
	if ( mouse_is_visible() ) {
		gr_resize_screen_pos( &Mouse_dx, &Mouse_dy );
	}

	if (Keep_mouse_centered && Mouse_hidden) {
		if (Mouse_dx || Mouse_dy)
			mouse_force_pos(cx, cy);

		old_x = cx;
		old_y = cy;

	} else {
		old_x = tmp_x;
		old_y = tmp_y;
	}

	LEAVE_CRITICAL_SECTION( mouse_lock );
}

#ifdef USE_DIRECTINPUT
#include "directx/vdinput.h"

static LPDIRECTINPUT			Di_mouse_obj = NULL;
static LPDIRECTINPUTDEVICE	Di_mouse = NULL;

void mouse_eval_deltas_di()
{
	int repeat = 1;
	HRESULT hr = 0;
	DIMOUSESTATE mouse_state;

	Mouse_dx = Mouse_dy = Mouse_dz = 0;
	if (!Di_mouse_inited)
		return;

	repeat = 1;
	memset(&mouse_state, 0, sizeof(mouse_state));
	while (repeat) {
		repeat = 0;

		hr = Di_mouse->GetDeviceState(sizeof(mouse_state), &mouse_state);
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
			// DirectInput is telling us that the input stream has
			// been interrupted.  We aren't tracking any state
			// between polls, so we don't have any special reset
			// that needs to be done.  We just re-acquire and
			// try again.
			Sleep(500);		// Pause a half second...
			hr = Di_mouse->Acquire();
			if (SUCCEEDED(hr))
				repeat = 1;
		}
	}

	if (SUCCEEDED(hr)) {
		Mouse_dx = (int) mouse_state.lX;
		Mouse_dy = (int) mouse_state.lY;
		Mouse_dz = (int) mouse_state.lZ;

	} else {
		Mouse_dx = Mouse_dy = Mouse_dz = 0;
	}

	// Speeds up the menu mouse on higher resolutions. The check for a
	// visible mouse should eliminate any possible gameplay changes.
	if ( mouse_is_visible() ) {
		gr_resize_screen_pos( &Mouse_dx, &Mouse_dy );
	}

	Mouse_x += Mouse_dx;
	Mouse_y += Mouse_dy;

	if (Mouse_x < 0)
		Mouse_x = 0;

	if (Mouse_y < 0)
		Mouse_y = 0;

	if (Mouse_x >= gr_screen.max_w)
   		Mouse_x = gr_screen.max_w - 1;

	if (Mouse_y >= gr_screen.max_h)
  		Mouse_y = gr_screen.max_h - 1;

	// keep the mouse inside our window so we don't switch applications or anything (debug bug people reported?)
	// JH: Dang!  This makes the mouse readings in DirectInput act screwy!
//	mouse_force_pos(gr_screen.max_w / 2, gr_screen.max_h / 2);
}
#endif  // ifdef USE_DIRECTINPUT


int mouse_get_pos(int *xpos, int *ypos)
{
	int flags;

	if (Mouse_mode == MOUSE_MODE_DI) {
		if (xpos)
			*xpos = Mouse_x;

		if (ypos)
			*ypos = Mouse_y;

		return mouse_flags;
	}

	if (!mouse_inited) {
		*xpos = *ypos = 0;
		return 0;
	}

	POINT pnt;
	getWindowMousePos(&pnt);

//	EnterCriticalSection(&mouse_lock);

	flags = mouse_flags;
	Mouse_x = pnt.x;
	Mouse_y = pnt.y;

//	LeaveCriticalSection(&mouse_lock);

	if (Mouse_x < 0){
		Mouse_x = 0;
	}

	if (Mouse_y < 0){
		Mouse_y = 0;
	}

	if (Mouse_x >= gr_screen.max_w){
		Mouse_x = gr_screen.max_w - 1;
	}

	if (Mouse_y >= gr_screen.max_h){
 		Mouse_y = gr_screen.max_h - 1;
	}
	
	if (xpos){
		*xpos = Mouse_x;
	}

	if (ypos){
		*ypos = Mouse_y;
	}

	return flags;
}

int mouse_get_pos_unscaled( int *xpos, int *ypos )
{
	int flags = mouse_get_pos( xpos, ypos );

	gr_unsize_screen_pos( (xpos) ? xpos : NULL, (ypos) ? ypos : NULL );

	return flags;
}

void mouse_get_real_pos(int *mx, int *my)
{
	if (Mouse_mode == MOUSE_MODE_DI) {
		*mx = Mouse_x;
		*my = Mouse_y;
		return;
	}

	POINT pnt;
	getWindowMousePos(&pnt);
	
	*mx = pnt.x;
	*my = pnt.y;
}

void mouse_set_pos(int xpos, int ypos)
{
	if (Mouse_mode == MOUSE_MODE_DI) {
		Mouse_x = xpos;
		Mouse_y = ypos;
		return;
	}

	if ((xpos != Mouse_x) || (ypos != Mouse_y)){
		mouse_force_pos(xpos, ypos);
	}
}

#ifdef USE_DIRECTINPUT
int di_init()
{
	HRESULT hr;

	if (Mouse_mode == MOUSE_MODE_WIN){
		return 0;
	}

	Di_mouse_inited = 0;
	hr = DirectInputCreate(GetModuleHandle(NULL), DIRECTINPUT_VERSION, &Di_mouse_obj, NULL);
	if (FAILED(hr)) {
		hr = DirectInputCreate(GetModuleHandle(NULL), 0x300, &Di_mouse_obj, NULL);
		if (FAILED(hr)) {
			mprintf(( "DirectInputCreate() failed!\n" ));
			return FALSE;
		}
	}

	hr = Di_mouse_obj->CreateDevice(GUID_SysMouse, &Di_mouse, NULL);
	if (FAILED(hr)) {
		mprintf(( "CreateDevice() failed!\n" ));
		return FALSE;
	}

	hr = Di_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(hr)) {
		mprintf(( "SetDataFormat() failed!\n" ));
		return FALSE;
	}

	hr = Di_mouse->SetCooperativeLevel((HWND)os_get_window(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr)) {
		mprintf(( "SetCooperativeLevel() failed!\n" ));
		return FALSE;
	}
/*
	DIPROPDWORD hdr;

	// Turn on buffering
	hdr.diph.dwSize = sizeof(DIPROPDWORD); 
	hdr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	hdr.diph.dwObj = 0;		
	hdr.diph.dwHow = DIPH_DEVICE;	// Apply to entire device
	hdr.dwData = 16;	//MAX_BUFFERED_KEYBOARD_EVENTS;

	hr = Di_mouse->SetProperty( DIPROP_BUFFERSIZE, &hdr.diph );
	if (FAILED(hr)) {
		mprintf(( "SetProperty DIPROP_BUFFERSIZE failed\n" ));
		return FALSE;
	}

	Di_event = CreateEvent( NULL, FALSE, FALSE, NULL );
	Assert(Di_event != NULL);

	hr = Di_mouse->SetEventNotification(Di_event);
	if (FAILED(hr)) {
		mprintf(( "SetEventNotification failed\n" ));
		return FALSE;
	}
*/
	Di_mouse->Acquire();

	Di_mouse_inited = 1;
	return TRUE;
}


void di_cleanup()
{
	// Destroy any lingering IDirectInputDevice object.
	if (Di_mouse) {
		// Unacquire the device one last time just in case we got really confused
		// and tried to exit while the device is still acquired.
		Di_mouse->Unacquire();

		Di_mouse->Release();
		Di_mouse = NULL;
	}

	// Destroy any lingering IDirectInput object.
	if (Di_mouse_obj) {
		Di_mouse_obj->Release();
		Di_mouse_obj = NULL;
	}

	Di_mouse_inited = 0;
}
#endif  // ifdef USE_DIRECTINPUT


// portable routine to get the mouse position, relative
// to current window
void getWindowMousePos(POINT * pt)
{
	Assert(pt != NULL);

#ifdef _WIN32
	GetCursorPos(pt);
	ScreenToClient((HWND)os_get_window(), pt);
#else
	SDL_GetMouseState(&pt->x, &pt->y);
#endif
}


// portable routine to get the mouse position, relative
// to current window
void setWindowMousePos(POINT * pt)
{
	Assert(pt != NULL);

#ifdef _WIN32
	ClientToScreen((HWND) os_get_window(), pt);
	SetCursorPos(pt->x, pt->y);
#else
	SDL_WarpMouse(pt->x, pt->y);
#endif
}
