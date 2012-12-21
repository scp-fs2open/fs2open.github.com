/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



//#define USE_DIRECTINPUT

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#else
#include "SDL.h"
#endif

#include "controlconfig/controlsconfig.h" //For textify scancode
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "io/key.h"
#include "math/fix.h"
#include "io/timer.h"
#include "localization/localize.h"
#include "parse/scripting.h"
#include "cmdline/cmdline.h"

#define THREADED	// to use the proper set of macros
#include "osapi/osapi.h"


#define KEY_BUFFER_SIZE 16

//-------- Variable accessed by outside functions ---------
ubyte				keyd_buffer_type;		// 0=No buffer, 1=buffer ASCII, 2=buffer scans
ubyte				keyd_repeat;
uint				keyd_last_pressed;
uint				keyd_last_released;
ubyte				keyd_pressed[NUM_KEYS];
int				keyd_time_when_last_pressed;

typedef struct keyboard	{
	ushort			keybuffer[KEY_BUFFER_SIZE];
	uint				time_pressed[KEY_BUFFER_SIZE];
	uint				TimeKeyWentDown[NUM_KEYS];
	uint				TimeKeyHeldDown[NUM_KEYS];
	uint				TimeKeyDownChecked[NUM_KEYS];
	uint				NumDowns[NUM_KEYS];
	uint				NumUps[NUM_KEYS];
	int				down_check[NUM_KEYS];  // nonzero if has been pressed yet this mission
	uint				keyhead, keytail;
} keyboard;

keyboard key_data;

int key_inited = 0;

CRITICAL_SECTION key_lock;

//int Backspace_debug=1;	// global flag that will enable/disable the backspace key from stopping execution
								// This flag was created since the backspace key is also used to correct mistakes
								// when typing in your pilots callsign.  This global flag is checked before execution
								// is stopped.

#ifdef SCP_UNIX
	int SDLtoFS2[SDLK_LAST];
#endif

int ascii_table[128] = 
{ 255, 255, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',255,255,
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 255, 255,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39, '`',
  255, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 255,'*',
  255, ' ', 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,255,255,
  255, 255, 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255 };

int shifted_ascii_table[128] = 
{ 255, 255, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',255,255,
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 255, 255,
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 
  255, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 255,255,
  255, ' ', 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,255,255,
  255, 255, 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255 };

// used to limit the keypresses that are accepted from the keyboard
#define MAX_FILTER_KEYS 64
int Num_filter_keys;
int Key_filter[MAX_FILTER_KEYS];

static int Key_numlock_was_on = 0;	// Flag to indicate whether NumLock is on at start

#ifdef _WIN32
    static int Key_running_NT = 0;		// NT is the OS
#endif

int Cheats_enabled = 0;
int Key_normal_game = 0;

#ifdef SCP_UNIX
/**
 * Keyboard layouts
 */
enum KeyboardLayout {
	KEYBOARD_LAYOUT_DEFAULT, //!< American
	KEYBOARD_LAYOUT_QWERTZ,  //!< German
	KEYBOARD_LAYOUT_AZERTY   //!< French
};

void FillSDLArray ()
{
	KeyboardLayout layout = KEYBOARD_LAYOUT_DEFAULT;
	
	if (Cmdline_keyboard_layout) {
		if (!strcmp(Cmdline_keyboard_layout, "qwertz")) {
			layout = KEYBOARD_LAYOUT_QWERTZ;
		}
		
		if (!strcmp(Cmdline_keyboard_layout, "azerty")) {
			layout = KEYBOARD_LAYOUT_AZERTY;
		}

	}

	if(layout == KEYBOARD_LAYOUT_AZERTY) {
		SDLtoFS2[SDLK_WORLD_64] = KEY_0;
		SDLtoFS2[SDLK_AMPERSAND] = KEY_1;
		SDLtoFS2[SDLK_WORLD_73] = KEY_2;
		SDLtoFS2[SDLK_QUOTEDBL] = KEY_3;
		SDLtoFS2[SDLK_QUOTE] = KEY_4;
		SDLtoFS2[SDLK_LEFTPAREN] = KEY_5;
		SDLtoFS2[SDLK_MINUS] = KEY_6;
		SDLtoFS2[SDLK_WORLD_72] = KEY_7;
		SDLtoFS2[SDLK_UNDERSCORE] = KEY_8;
		SDLtoFS2[SDLK_WORLD_71] = KEY_9;
	} else {
		SDLtoFS2[SDLK_0] = KEY_0;
		SDLtoFS2[SDLK_1] = KEY_1;
		SDLtoFS2[SDLK_2] = KEY_2;
		SDLtoFS2[SDLK_3] = KEY_3;
		SDLtoFS2[SDLK_4] = KEY_4;
		SDLtoFS2[SDLK_5] = KEY_5;
		SDLtoFS2[SDLK_6] = KEY_6;
		SDLtoFS2[SDLK_7] = KEY_7;
		SDLtoFS2[SDLK_8] = KEY_8;
		SDLtoFS2[SDLK_9] = KEY_9;
	}

	SDLtoFS2[SDLK_a] = KEY_A;
	SDLtoFS2[SDLK_b] = KEY_B;
	SDLtoFS2[SDLK_c] = KEY_C;
	SDLtoFS2[SDLK_d] = KEY_D;
	SDLtoFS2[SDLK_e] = KEY_E;
	SDLtoFS2[SDLK_f] = KEY_F;
	SDLtoFS2[SDLK_g] = KEY_G;
	SDLtoFS2[SDLK_h] = KEY_H;
	SDLtoFS2[SDLK_i] = KEY_I;
	SDLtoFS2[SDLK_j] = KEY_J;
	SDLtoFS2[SDLK_k] = KEY_K;
	SDLtoFS2[SDLK_l] = KEY_L;
	SDLtoFS2[SDLK_m] = KEY_M;
	SDLtoFS2[SDLK_n] = KEY_N;
	SDLtoFS2[SDLK_o] = KEY_O;
	SDLtoFS2[SDLK_p] = KEY_P;
	SDLtoFS2[SDLK_q] = KEY_Q;
	SDLtoFS2[SDLK_r] = KEY_R;
	SDLtoFS2[SDLK_s] = KEY_S;
	SDLtoFS2[SDLK_t] = KEY_T;
	SDLtoFS2[SDLK_u] = KEY_U;
	SDLtoFS2[SDLK_v] = KEY_V;
	SDLtoFS2[SDLK_w] = KEY_W;
	SDLtoFS2[SDLK_x] = KEY_X;
	SDLtoFS2[SDLK_y] = KEY_Y;
	SDLtoFS2[SDLK_z] = KEY_Z;

	if(layout == KEYBOARD_LAYOUT_DEFAULT) {
		SDLtoFS2[SDLK_MINUS] = KEY_MINUS;
		SDLtoFS2[SDLK_EQUALS] = KEY_EQUAL;
		SDLtoFS2[SDLK_SLASH] = KEY_DIVIDE; // No idea - DDOI
		SDLtoFS2[SDLK_BACKSLASH] = KEY_SLASH;
		//SDLtoFS2[SDLK_BACKSLASH] = KEY_SLASH_UK; // ?
		SDLtoFS2[SDLK_COMMA] = KEY_COMMA;
		SDLtoFS2[SDLK_PERIOD] = KEY_PERIOD;
		SDLtoFS2[SDLK_SEMICOLON] = KEY_SEMICOL;

		SDLtoFS2[SDLK_LEFTBRACKET] = KEY_LBRACKET;
		SDLtoFS2[SDLK_RIGHTBRACKET] = KEY_RBRACKET;

		SDLtoFS2[SDLK_BACKQUOTE] = KEY_LAPOSTRO;
		SDLtoFS2[SDLK_QUOTE] = KEY_RAPOSTRO;
	}

	if(layout == KEYBOARD_LAYOUT_QWERTZ) {
		SDLtoFS2[SDLK_WORLD_63] = KEY_MINUS;
		SDLtoFS2[SDLK_WORLD_20] = KEY_EQUAL;
		SDLtoFS2[SDLK_MINUS] = KEY_DIVIDE;
		SDLtoFS2[SDLK_HASH] = KEY_SLASH;
		SDLtoFS2[SDLK_COMMA] = KEY_COMMA;
		SDLtoFS2[SDLK_PERIOD] = KEY_PERIOD;
		SDLtoFS2[SDLK_WORLD_86] = KEY_SEMICOL;

		SDLtoFS2[SDLK_WORLD_92] = KEY_LBRACKET;
		SDLtoFS2[SDLK_PLUS] = KEY_RBRACKET;

		SDLtoFS2[SDLK_CARET] = KEY_LAPOSTRO;
		SDLtoFS2[SDLK_WORLD_68] = KEY_RAPOSTRO;
	}

	if(layout == KEYBOARD_LAYOUT_AZERTY) {
		SDLtoFS2[SDLK_RIGHTPAREN] = KEY_MINUS;
		SDLtoFS2[SDLK_EQUALS] = KEY_EQUAL;
		SDLtoFS2[SDLK_EXCLAIM] = KEY_DIVIDE;
		SDLtoFS2[SDLK_ASTERISK] = KEY_SLASH;
		SDLtoFS2[SDLK_COMMA] = KEY_COMMA;
		SDLtoFS2[SDLK_COLON] = KEY_PERIOD;
		SDLtoFS2[SDLK_SEMICOLON] = KEY_SEMICOL;

		SDLtoFS2[SDLK_CARET] = KEY_LBRACKET;
		SDLtoFS2[SDLK_DOLLAR] = KEY_RBRACKET;

		SDLtoFS2[SDLK_WORLD_18] = KEY_LAPOSTRO;
		SDLtoFS2[SDLK_WORLD_89] = KEY_RAPOSTRO;
	}

	SDLtoFS2[SDLK_ESCAPE] = KEY_ESC;
	SDLtoFS2[SDLK_RETURN] = KEY_ENTER;
	SDLtoFS2[SDLK_BACKSPACE] = KEY_BACKSP;
	SDLtoFS2[SDLK_TAB] = KEY_TAB;
	SDLtoFS2[SDLK_SPACE] = KEY_SPACEBAR;

	SDLtoFS2[SDLK_NUMLOCK] = KEY_NUMLOCK;
	SDLtoFS2[SDLK_SCROLLOCK] = KEY_SCROLLOCK;
	SDLtoFS2[SDLK_CAPSLOCK] = KEY_CAPSLOCK;

	SDLtoFS2[SDLK_LSHIFT] = KEY_LSHIFT;
	SDLtoFS2[SDLK_RSHIFT] = KEY_RSHIFT;

	SDLtoFS2[SDLK_LALT] = KEY_LALT;
	SDLtoFS2[SDLK_RALT] = KEY_RALT;

	SDLtoFS2[SDLK_LCTRL] = KEY_LCTRL;
	SDLtoFS2[SDLK_RCTRL] = KEY_RCTRL;

	SDLtoFS2[SDLK_F1] = KEY_F1;
	SDLtoFS2[SDLK_F2] = KEY_F2;
	SDLtoFS2[SDLK_F3] = KEY_F3;
	SDLtoFS2[SDLK_F4] = KEY_F4;
	SDLtoFS2[SDLK_F5] = KEY_F5;
	SDLtoFS2[SDLK_F6] = KEY_F6;
	SDLtoFS2[SDLK_F7] = KEY_F7;
	SDLtoFS2[SDLK_F8] = KEY_F8;
	SDLtoFS2[SDLK_F9] = KEY_F9;
	SDLtoFS2[SDLK_F10] = KEY_F10;
	SDLtoFS2[SDLK_F11] = KEY_F11;
	SDLtoFS2[SDLK_F12] = KEY_F12;

	SDLtoFS2[SDLK_KP0] = KEY_PAD0;
	SDLtoFS2[SDLK_KP1] = KEY_PAD1;
	SDLtoFS2[SDLK_KP2] = KEY_PAD2;
	SDLtoFS2[SDLK_KP3] = KEY_PAD3;
	SDLtoFS2[SDLK_KP4] = KEY_PAD4;
	SDLtoFS2[SDLK_KP5] = KEY_PAD5;
	SDLtoFS2[SDLK_KP6] = KEY_PAD6;
	SDLtoFS2[SDLK_KP7] = KEY_PAD7;
	SDLtoFS2[SDLK_KP8] = KEY_PAD8;
	SDLtoFS2[SDLK_KP9] = KEY_PAD9;
	SDLtoFS2[SDLK_KP_MINUS] = KEY_PADMINUS;
	SDLtoFS2[SDLK_KP_PLUS] = KEY_PADPLUS;
	SDLtoFS2[SDLK_KP_PERIOD] = KEY_PADPERIOD;
	SDLtoFS2[SDLK_KP_DIVIDE] = KEY_PADDIVIDE;
	SDLtoFS2[SDLK_KP_MULTIPLY] = KEY_PADMULTIPLY;
	SDLtoFS2[SDLK_KP_ENTER] = KEY_PADENTER;

	SDLtoFS2[SDLK_INSERT] = KEY_INSERT;
	SDLtoFS2[SDLK_HOME] = KEY_HOME;
	SDLtoFS2[SDLK_PAGEUP] = KEY_PAGEUP;
	SDLtoFS2[SDLK_DELETE] = KEY_DELETE;
	SDLtoFS2[SDLK_END] = KEY_END;
	SDLtoFS2[SDLK_PAGEDOWN] = KEY_PAGEDOWN;
	SDLtoFS2[SDLK_UP] = KEY_UP;
	SDLtoFS2[SDLK_DOWN] = KEY_DOWN;
	SDLtoFS2[SDLK_LEFT] = KEY_LEFT;
	SDLtoFS2[SDLK_RIGHT] = KEY_RIGHT;

	SDLtoFS2[SDLK_PRINT] = KEY_PRINT_SCRN;
	SDLtoFS2[SDLK_PAUSE] = KEY_PAUSE;
	SDLtoFS2[SDLK_BREAK] = KEY_BREAK;
}
#endif

int key_numlock_is_on()
{
#ifdef _WIN32
	unsigned char keys[256];
	GetKeyboardState(keys);
	if ( keys[VK_NUMLOCK]  ) {
		return 1;
	}
	return 0;
#else
	int keys[SDLK_LAST];
	SDL_GetKeyState(keys);
	if ( keys[SDLK_NUMLOCK] ) {
		return 1;
	}
	return 0;
#endif
}

void key_turn_off_numlock()
{
#ifdef _WIN32
	unsigned char keys[256];
	GetKeyboardState(keys);
	keys[VK_NUMLOCK] = 0;
	SetKeyboardState(keys);
#endif
}

void key_turn_on_numlock()
{
#ifdef _WIN32
	unsigned char keys[256];
	GetKeyboardState(keys);
	keys[VK_NUMLOCK] = 1;
	SetKeyboardState(keys);
#endif
}

//	Convert a BIOS scancode to ASCII.
//	If scancode >= 127, returns 255, meaning there is no corresponding ASCII code.
//	Uses ascii_table and shifted_ascii_table to translate scancode to ASCII.
int key_to_ascii(int keycode )
{
	int shifted;

	if ( !key_inited ) return 255;

	shifted = keycode & KEY_SHIFTED;
	keycode &= 0xFF;

	if ( keycode>=127 )
		return 255;

	if (shifted)
		return shifted_ascii_table[keycode];
	else
		return ascii_table[keycode];
}

//	Flush the keyboard buffer.
//	Clear the keyboard array (keyd_pressed).
void key_flush()
{
	int i;
	uint CurTime;

	if ( !key_inited ) return;

	ENTER_CRITICAL_SECTION( key_lock );	

	key_data.keyhead = key_data.keytail = 0;

	//Clear the keyboard buffer
	for (i=0; i<KEY_BUFFER_SIZE; i++ )	{
		key_data.keybuffer[i] = 0;
		key_data.time_pressed[i] = 0;
	}
	
	//Clear the keyboard array

	CurTime = timer_get_milliseconds();


	for (i=0; i<NUM_KEYS; i++ )	{
		keyd_pressed[i] = 0;
		key_data.TimeKeyDownChecked[i] = CurTime;
		key_data.TimeKeyWentDown[i] = CurTime;
		key_data.TimeKeyHeldDown[i] = 0;
		key_data.NumDowns[i]=0;
		key_data.NumUps[i]=0;
	}

	LEAVE_CRITICAL_SECTION( key_lock );	
}

//	A nifty function which performs the function:
//		n = (n+1) % KEY_BUFFER_SIZE
//	(assuming positive values of n).
int add_one( int n )
{
	n++;
	if ( n >= KEY_BUFFER_SIZE ) n=0;
	return n;
}

// Returns 1 if character waiting... 0 otherwise
int key_checkch()
{
	int is_one_waiting = 0;

	if ( !key_inited ) return 0;

	ENTER_CRITICAL_SECTION( key_lock );	

	if (key_data.keytail != key_data.keyhead){
		is_one_waiting = 1;
	}

	LEAVE_CRITICAL_SECTION( key_lock );		

	return is_one_waiting;
}

//	Return key scancode if a key has been pressed,
//	else return 0.
//	Reads keys out of the key buffer and updates keyhead.

//WMC - Added so scripting can get at keys.
int Current_key_down = 0;
int key_inkey()
{
	int key = 0;

	if ( !key_inited ) return 0;

	ENTER_CRITICAL_SECTION( key_lock );	

	if (key_data.keytail!=key_data.keyhead)	{
		key = key_data.keybuffer[key_data.keyhead];
		key_data.keyhead = add_one(key_data.keyhead);
	}

	LEAVE_CRITICAL_SECTION( key_lock );	

	Current_key_down = key;

	return key;
}

//	Unget a key.  Puts it back in the input queue.
void key_outkey(int key)
{
	int	bufp;

	if ( !key_inited ) return;

	ENTER_CRITICAL_SECTION( key_lock );		

	bufp = key_data.keytail+1;

	if (bufp >= KEY_BUFFER_SIZE){
		bufp = 0;
	}

	key_data.keybuffer[key_data.keytail] = (unsigned short)key;

	key_data.keytail = bufp;

	LEAVE_CRITICAL_SECTION( key_lock );		
}



//	Return amount of time last key was held down.
//	This is currently (July 17, 1996) bogus because our timing is
//	not accurate.
int key_inkey_time(uint * time)
{
	int key = 0;

	if ( !key_inited ) {
		*time = 0;
		return 0;
	}
	
	ENTER_CRITICAL_SECTION( key_lock );		

	if (key_data.keytail!=key_data.keyhead)	{
		key = key_data.keybuffer[key_data.keyhead];
		*time = key_data.time_pressed[key_data.keyhead];
		key_data.keyhead = add_one(key_data.keyhead);
	}

	LEAVE_CRITICAL_SECTION( key_lock );		

	return key;
}


//	Returns scancode of last key pressed, if any (returns 0 if no key pressed)
//	but does not update keyhead pointer.
int key_peekkey()
{
	int key = 0;

	if ( !key_inited ) return 0;

	ENTER_CRITICAL_SECTION( key_lock );		

	if (key_data.keytail!=key_data.keyhead)	{
		key = key_data.keybuffer[key_data.keyhead];
	}
	LEAVE_CRITICAL_SECTION( key_lock );		

	return key;
}

// If not installed, uses BIOS and returns getch();
//	Else returns pending key (or waits for one if none waiting).
int key_getch()
{
	int dummy=0;
	int in;

	if ( !key_inited ) return 0;
	
	while (!key_checkch()){
		os_poll();

		dummy++;
	}
	in = key_inkey();

	return in;
}

//	Set global shift_status with modifier results (shift, ctrl, alt).
uint key_get_shift_status()
{
	unsigned int shift_status = 0;

	if ( !key_inited ) return 0;

	ENTER_CRITICAL_SECTION( key_lock );		

	if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT] )
		shift_status |= KEY_SHIFTED;

	if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT] )
		shift_status |= KEY_ALTED;

	if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL] )
		shift_status |= KEY_CTRLED;

#ifndef NDEBUG
	if (keyd_pressed[KEY_DEBUG_KEY])
		shift_status |= KEY_DEBUGGED;
#else
	if (keyd_pressed[KEY_DEBUG_KEY]) {
		mprintf(("Cheats_enabled = %i, Key_normal_game = %i\n", Cheats_enabled, Key_normal_game));
		if ((Cheats_enabled) && Key_normal_game) {
			mprintf(("Debug key\n"));
			shift_status |= KEY_DEBUGGED1;
		}
	}
#endif
	LEAVE_CRITICAL_SECTION( key_lock );		

	return shift_status;
}

//	Returns amount of time key (specified by "code") has been down since last call.
//	Returns float, unlike key_down_time() which returns a fix.
float key_down_timef(uint scancode)	
{
	uint time_down, time;
	uint delta_time;

	if ( !key_inited ) {
		return 0.0f;
	}

	if (scancode >= NUM_KEYS) {
		return 0.0f;
	}

	ENTER_CRITICAL_SECTION( key_lock );		

	time = timer_get_milliseconds();
	delta_time = time - key_data.TimeKeyDownChecked[scancode];
	key_data.TimeKeyDownChecked[scancode] = time;

	if ( delta_time <= 1 ) {
		key_data.TimeKeyWentDown[scancode] = time;
		if (keyd_pressed[scancode])	{
			LEAVE_CRITICAL_SECTION( key_lock );		
			return 1.0f;
		} else	{
			LEAVE_CRITICAL_SECTION( key_lock );		
			return 0.0f;
		}
	}

	if ( !keyd_pressed[scancode] )	{
		time_down = key_data.TimeKeyHeldDown[scancode];
		key_data.TimeKeyHeldDown[scancode] = 0;
	} else	{
		time_down =  time - key_data.TimeKeyWentDown[scancode];
		key_data.TimeKeyWentDown[scancode] = time;
	}

	LEAVE_CRITICAL_SECTION( key_lock );		

	return i2fl(time_down) / i2fl(delta_time);
}

/*
//	Returns amount of time key (specified by "code") has been down since last call.
//	Returns float, unlike key_down_time() which returns a fix.
fix key_down_time( uint code )
{
	uint time_down, time;
	uint delta_time;

	if ( !key_inited ) return 0.0f;

	if ((scancode<0)|| (scancode>=NUM_KEYS)) return 0.0f;

	EnterCriticalSection( &key_lock );

	time = timer_get_milliseconds();
	delta_time = time - TimeKeyDownChecked[scancode];
	TimeKeyDownChecked[scancode] = time;

	if ( delta_time <= 1 ) {
		LeaveCriticalSection( &key_lock );
		if (keyd_pressed[scancode])
			return F1_0;
		else
			return 0;
	}

	if ( !keyd_pressed[scancode] )	{
		time_down = key_data.TimeKeyHeldDown[scancode];
		key_data.TimeKeyHeldDown[scancode] = 0;
	} else	{
		time_down =  time - key_data.TimeKeyWentDown[scancode];
		key_data.TimeKeyWentDown[scancode] = time;
	}

	LeaveCriticalSection( &key_lock );

	return fixmuldiv( time_down, F1_0, delta_time );
}
*/


// Returns number of times key has went from up to down since last call.
int key_down_count(int scancode)	
{
	int n;

	if ( !key_inited ) return 0;
	if ((scancode<0)|| (scancode>=NUM_KEYS)) return 0;

	ENTER_CRITICAL_SECTION( key_lock );		

	n = key_data.NumDowns[scancode];
	key_data.NumDowns[scancode] = 0;

	LEAVE_CRITICAL_SECTION( key_lock );		

	return n;
}


// Returns number of times key has went from down to up since last call.
int key_up_count(int scancode)	
{
	int n;

	if ( !key_inited ) return 0;
	if ((scancode<0)|| (scancode>=NUM_KEYS)) return 0;

	ENTER_CRITICAL_SECTION( key_lock );		

	n = key_data.NumUps[scancode];
	key_data.NumUps[scancode] = 0;

	LEAVE_CRITICAL_SECTION( key_lock );		

	return n;
}

int key_check(int key)
{
	return key_data.down_check[key];
}

//	Add a key up or down code to the key buffer.  state=1 -> down, state=0 -> up
// latency => time difference in ms between when key was actually pressed and now
//void key_mark( uint code, int state )
void key_mark( uint code, int state, uint latency )
{
	uint scancode, breakbit, temp, event_time;
	ushort keycode;	

	if ( !key_inited ) return;

	ENTER_CRITICAL_SECTION( key_lock );		

	// If running in the UK, need to translate their wacky slash scancode to ours
	if ( code == KEY_SLASH_UK ) {
		code = KEY_SLASH;
	}

#ifndef SCP_UNIX

	if ( (code == 0xc5) && !Key_running_NT ) {
		key_turn_off_numlock();
	}
#endif

	Assert( code < NUM_KEYS );	

	event_time = timer_get_milliseconds() - latency;
	// event_time = timeGetTime() - latency;

	// Read in scancode
	scancode = code & (NUM_KEYS-1);
	breakbit = !state;
	
	if (breakbit)	{
		// Key going up
		keyd_last_released = scancode;
		keyd_pressed[scancode] = 0;
		key_data.NumUps[scancode]++;

		//	What is the point of this code?  "temp" is never used!
		temp = 0;
		temp |= keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT];
		temp |= keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT];
		temp |= keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL];
//#ifndef NDEBUG
		temp |= keyd_pressed[KEY_DEBUG_KEY];
//#endif	
		if (event_time < key_data.TimeKeyWentDown[scancode])
			key_data.TimeKeyHeldDown[scancode] = 0;
		else
			key_data.TimeKeyHeldDown[scancode] += event_time - key_data.TimeKeyWentDown[scancode];

		Current_key_down = scancode;
		if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT] )
			Current_key_down |= KEY_SHIFTED;

		if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT] )
			Current_key_down |= KEY_ALTED;

		if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL] )
			Current_key_down |= KEY_CTRLED;
		Script_system.SetHookVar("Key", 's', textify_scancode(Current_key_down));
		Script_system.RunCondition(CHA_KEYRELEASED);
		Script_system.RemHookVar("Key");
	} else {
		// Key going down
		keyd_last_pressed = scancode;
		keyd_time_when_last_pressed = event_time;
		if (!keyd_pressed[scancode])	{
			// First time down
			key_data.TimeKeyWentDown[scancode] = event_time;
			keyd_pressed[scancode] = 1;
			key_data.NumDowns[scancode]++;
			key_data.down_check[scancode]++;

			//WMC - For scripting
			Current_key_down = scancode;
			if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT] )
				Current_key_down |= KEY_SHIFTED;

			if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT] )
				Current_key_down |= KEY_ALTED;

			if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL] )
				Current_key_down |= KEY_CTRLED;
			Script_system.SetHookVar("Key", 's', textify_scancode(Current_key_down));
			Script_system.RunCondition(CHA_KEYPRESSED);
			Script_system.RemHookVar("Key");
		} else if (!keyd_repeat) {
			// Don't buffer repeating key if repeat mode is off
			scancode = 0xAA;		
		} 

		if ( scancode!=0xAA ) {
			keycode = (unsigned short)scancode;

			if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT] )
				keycode |= KEY_SHIFTED;

			if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT] )
				keycode |= KEY_ALTED;

			if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL] )
				keycode |= KEY_CTRLED;

#ifndef NDEBUG
			if ( keyd_pressed[KEY_DEBUG_KEY] )
				keycode |= KEY_DEBUGGED;
#else
			if ( keyd_pressed[KEY_DEBUG_KEY] ) {
				mprintf(("Cheats_enabled = %i, Key_normal_game = %i\n", Cheats_enabled, Key_normal_game));
				if (Cheats_enabled && Key_normal_game) {
					keycode |= KEY_DEBUGGED1;
				}
			}

#endif

			if ( keycode )	{
				temp = key_data.keytail+1;
				if ( temp >= KEY_BUFFER_SIZE ) temp=0;

				if (temp!=key_data.keyhead)	{
					int i, accept_key = 1;
					// Num_filter_keys will only be non-zero when a key filter has
					// been explicity set up via key_set_filter()
					for ( i = 0; i < Num_filter_keys; i++ ) {
						accept_key = 0;
						if ( Key_filter[i] == keycode ) {
							accept_key = 1;
							break;
						}
					}

					if ( accept_key ) {
						key_data.keybuffer[key_data.keytail] = keycode;
						key_data.time_pressed[key_data.keytail] = keyd_time_when_last_pressed;
						key_data.keytail = temp;
					}
				}
			}
		}
	}

	LEAVE_CRITICAL_SECTION( key_lock );		
}

#ifdef USE_DIRECTINPUT
void di_cleanup();
int di_init();
#endif


void key_close()
{
	if ( !key_inited ) return;

	#ifdef USE_DIRECTINPUT
		di_cleanup();
	#endif

	if ( Key_numlock_was_on ) {
		key_turn_on_numlock();
		Key_numlock_was_on = 0;
	}

	key_inited = 0;

	DELETE_CRITICAL_SECTION( key_lock );
}

void key_init()
{
	// Initialize queue
	if ( key_inited ) return;
	key_inited = 1;

	INITIALIZE_CRITICAL_SECTION( key_lock );

	ENTER_CRITICAL_SECTION( key_lock );
	
#ifdef SCP_UNIX
	FillSDLArray();
#endif

	keyd_time_when_last_pressed = timer_get_milliseconds();
	keyd_buffer_type = 1;
	keyd_repeat = 1;

	// Clear the keyboard array
	key_flush();

	// Clear key filter
	key_clear_filter();

	LEAVE_CRITICAL_SECTION( key_lock );

#ifdef _WIN32
	#ifdef USE_DIRECTINPUT
		di_init();
	#endif

	OSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&ver);
	if ( ver.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
		Key_running_NT = 1;
	} else {
		Key_running_NT = 0;
		if ( key_numlock_is_on() ) {
			Key_numlock_was_on = 1;
			key_turn_off_numlock();
		}
	}
#endif

	atexit(key_close);
}

void key_level_init()
{
	int i;

	for (i=0; i<NUM_KEYS; i++)
		key_data.down_check[i] = 0;
}

void key_lost_focus()
{
	if ( !key_inited ) return;

	key_flush();	
}

void key_got_focus()
{
	if ( !key_inited ) return;
	
	key_flush();	
}

// Restricts the keys that are accepted from the keyboard
//
//	filter_array	=>		array of keys to act as a filter
//	num				=>		number of keys in filter_array
//
void key_set_filter(int *filter_array, int num)
{
	int i;

	if ( num >= MAX_FILTER_KEYS ) {
		Int3();
		num = MAX_FILTER_KEYS;
	}

	Num_filter_keys = num;

	for ( i = 0; i < num; i++ ) {
		Key_filter[i] = filter_array[i];
	}
}

// Clear the key filter, so all keypresses are accepted from keyboard 
//
void key_clear_filter()
{
	int i;

	Num_filter_keys = 0;
	for ( i = 0; i < MAX_FILTER_KEYS; i++ ) {
		Key_filter[i] = -1;
	}
}


#ifdef USE_DIRECTINPUT

// JAS - April 18, 1998
// Not using because DI has the following problems:  (Everything else works ok)
// Under NT, Pause and Numlock report as identical keys.
// Under 95, Pause is the same as pressing Ctrl then Numlock.  So the game fires each
// time you hit it.
// 

//============================================================================
// Direct Input code
// For the keyboard, this basically replaces our old functionallity of:
// WM_KEYDOWN:
//    key_mark(...);
// WM_KEYUP:
//    key_mark(...);
//============================================================================


#include "directx/vdinput.h"

#define MAX_BUFFERED_KEYBOARD_EVENTS 10

static LPDIRECTINPUT			Di_object = NULL;
static LPDIRECTINPUTDEVICE	Di_keyboard = NULL;
static HANDLE					Di_thread = NULL;
static DWORD					Di_thread_id = NULL;
static HANDLE					Di_event = NULL;

DWORD di_process(DWORD lparam)
{
	while (1) {
		if ( WaitForSingleObject( Di_event, INFINITE )==WAIT_OBJECT_0 )	{

			//mprintf(( "Got event!\n" ));

			HRESULT hr;

			DIDEVICEOBJECTDATA rgdod[10]; 
			DWORD dwItems = MAX_BUFFERED_KEYBOARD_EVENTS; 

again:;
			hr = Di_keyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), rgdod,  &dwItems, 0); 

			if (hr == DIERR_INPUTLOST) {
				/*
				*  DirectInput is telling us that the input stream has
				*  been interrupted.  We aren't tracking any state
				*  between polls, so we don't have any special reset
				*  that needs to be done.  We just re-acquire and
				*  try again.
				*/
				Sleep(1000);		// Pause a second...
				hr = Di_keyboard->Acquire();
				if (SUCCEEDED(hr)) {
					goto again;
				}
			}

			if (SUCCEEDED(hr)) { 
				 // dwItems = number of elements read (could be zero)
				 if (hr == DI_BUFFEROVERFLOW) { 
					// Buffer had overflowed. 
					mprintf(( "Buffer overflowed!\n" ));
				 } 
					int i;

					//mprintf(( "Got %d events\n", dwItems ));

					for (i=0; i<(int)dwItems; i++ )	{
						int key = rgdod[i].dwOfs;
						int state = rgdod[i].dwData;
						int stamp = rgdod[i].dwTimeStamp;

						int latency;
						latency = timeGetTime() - stamp;
						if ( latency < 0 )
							latency=0;

//						if ( key == KEY_PRINT_SCRN )	{
//							key_mark( key, 1, latency );
//						}
//						key_mark( key, (state&0x80?1:0), latency );
						mprintf(( "Key=%x, State=%x, Time=%d, Latency=%d\n", key, state, stamp, latency ));
					}

			} 
		} 

	}

	return 0;
}


int di_init()
{
    HRESULT hr;

	 return 0;


    /*
     *  Register with the DirectInput subsystem and get a pointer
     *  to a IDirectInput interface we can use.
     *
     *  Parameters:
     *
     *      g_hinst
     *
     *          Instance handle to our application or DLL.
     *
     *      DIRECTINPUT_VERSION
     *
     *          The version of DirectInput we were designed for.
     *          We take the value from the <dinput.h> header file.
     *
     *      &g_pdi
     *
     *          Receives pointer to the IDirectInput interface
     *          that was created.
     *
     *      NULL
     *
     *          We do not use OLE aggregation, so this parameter
     *          must be NULL.
     *
     */
    hr = DirectInputCreate(GetModuleHandle(NULL), 0x300, &Di_object, NULL);

    if (FAILED(hr)) {
        mprintf(( "DirectInputCreate failed!\n" ));
        return FALSE;
    }

    /*
     *  Obtain an interface to the system keyboard device.
     *
     *  Parameters:
     *
     *      GUID_SysKeyboard
     *
     *          The instance GUID for the device we wish to access.
     *          GUID_SysKeyboard is a predefined instance GUID that
     *          always refers to the system keyboard device.
     *
     *      &g_pKeyboard
     *
     *          Receives pointer to the IDirectInputDevice interface
     *          that was created.
     *
     *      NULL
     *
     *          We do not use OLE aggregation, so this parameter
     *          must be NULL.
     *
     */
    hr = Di_object->CreateDevice(GUID_SysKeyboard, &Di_keyboard, NULL);

    if (FAILED(hr)) {
        mprintf(( "CreateDevice failed!\n" ));
        return FALSE;
    }

    /*
     *  Set the data format to "keyboard format".
     *
     *  A data format specifies which controls on a device we
     *  are interested in, and how they should be reported.
     *
     *  This tells DirectInput that we will be passing an array
     *  of 256 bytes to IDirectInputDevice::GetDeviceState.
     *
     *  Parameters:
     *
     *      c_dfDIKeyboard
     *
     *          Predefined data format which describes
     *          an array of 256 bytes, one per scancode.
     */
    hr = Di_keyboard->SetDataFormat(&c_dfDIKeyboard);

    if (FAILED(hr)) {
        mprintf(( "SetDataFormat failed!\n" ));
        return FALSE;
    }


    /*
     *  Set the cooperativity level to let DirectInput know how
     *  this device should interact with the system and with other
     *  DirectInput applications.
     *
     *  Parameters:
     *
     *      DISCL_NONEXCLUSIVE
     *
     *          Retrieve keyboard data when acquired, not interfering
     *          with any other applications which are reading keyboard
     *          data.
     *
     *      DISCL_FOREGROUND
     *
     *          If the user switches away from our application,
     *          automatically release the keyboard back to the system.
     *
     */
	hr = Di_keyboard->SetCooperativeLevel((HWND)os_get_window(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	if (FAILED(hr)) {
		mprintf(( "SetCooperativeLevel failed!\n" ));
		return FALSE;
	}

	DIPROPDWORD hdr;

	// Turn on buffering
	hdr.diph.dwSize = sizeof(DIPROPDWORD); 
	hdr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	hdr.diph.dwObj = 0;		
	hdr.diph.dwHow = DIPH_DEVICE;	// Apply to entire device
	hdr.dwData = 16;	//MAX_BUFFERED_KEYBOARD_EVENTS;

	hr = Di_keyboard->SetProperty( DIPROP_BUFFERSIZE, &hdr.diph );
	if (FAILED(hr)) {
		mprintf(( "SetProperty DIPROP_BUFFERSIZE failed\n" ));
		return FALSE;
	}


	Di_event = CreateEvent( NULL, FALSE, FALSE, NULL );
	Assert(Di_event != NULL);

	Di_thread = CreateThread(NULL, 1024, (LPTHREAD_START_ROUTINE)di_process, NULL, 0, &Di_thread_id);
	Assert( Di_thread != NULL );

	SetThreadPriority(Di_thread, THREAD_PRIORITY_HIGHEST);

	hr = Di_keyboard->SetEventNotification(Di_event);
	if (FAILED(hr)) {
		mprintf(( "SetEventNotification failed\n" ));
		return FALSE;
	}

	Di_keyboard->Acquire();

	return TRUE;
}

void di_cleanup()
{
    /*
     *  Destroy any lingering IDirectInputDevice object.
     */
    if (Di_keyboard) {

        /*
         *  Cleanliness is next to godliness.  Unacquire the device
         *  one last time just in case we got really confused and tried
         *  to exit while the device is still acquired.
         */
        Di_keyboard->Unacquire();

        Di_keyboard->Release();
        Di_keyboard = NULL;
    }

    /*
     *  Destroy any lingering IDirectInput object.
     */
    if (Di_object) {
        Di_object->Release();
        Di_object = NULL;
    }

	if ( Di_event )	{
		CloseHandle(Di_event);
		Di_event = NULL;
	}

}

#endif




