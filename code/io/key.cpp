/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "controlconfig/controlsconfig.h" //For textify scancode
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "io/key.h"
#include "math/fix.h"
#include "io/timer.h"
#include "localization/localize.h"
#include "scripting/scripting.h"
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

SDL_mutex* key_lock;

//int Backspace_debug=1;	// global flag that will enable/disable the backspace key from stopping execution
								// This flag was created since the backspace key is also used to correct mistakes
								// when typing in your pilots callsign.  This global flag is checked before execution
								// is stopped.

SCP_map<SDL_Scancode, int> SDLtoFS2;
SCP_map<int, SDL_Scancode> FS2toSDL; // Convenience map to speed up FS2->SDL lookups needed for displaying localized keycodes in config

int ascii_table[SIZE_OF_ASCII_TABLE] =
{ 255, 255, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',255,255,
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 255, 255,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39, '`',
  255, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 255,'*',
  255, ' ', 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,255,255,
  255, 255, 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255 };

int shifted_ascii_table[SIZE_OF_ASCII_TABLE] =
{ 255, 255, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',255,255,
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 255, 255,
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 
  255, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 255,255,
  255, ' ', 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,255,255,
  255, 255, 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255 };

static int Key_numlock_was_on = 0;	// Flag to indicate whether NumLock is on at start


int Cheats_enabled = 0;
int Key_normal_game = 0;

namespace
{
	bool key_down_event_handler(const SDL_Event& e)
	{
		if (!os::events::isWindowEvent(e, os::getSDLMainWindow())) {
			return false;
		}

		if (SDLtoFS2[e.key.keysym.scancode]) {
			key_mark(SDLtoFS2[e.key.keysym.scancode], 1, 0);

			return true;
		}

		return false;
	}

	bool key_up_event_handler(const SDL_Event& e)
	{
		if (!os::events::isWindowEvent(e, os::getSDLMainWindow())) {
			return false;
		}

		if (SDLtoFS2[e.key.keysym.scancode]) {
			key_mark(SDLtoFS2[e.key.keysym.scancode], 0, 0);

			return true;
		}

		return false;
	}
}

void FillSDLArray ()
{
	SDLtoFS2[SDL_SCANCODE_0] = KEY_0;
	SDLtoFS2[SDL_SCANCODE_1] = KEY_1;
	SDLtoFS2[SDL_SCANCODE_2] = KEY_2;
	SDLtoFS2[SDL_SCANCODE_3] = KEY_3;
	SDLtoFS2[SDL_SCANCODE_4] = KEY_4;
	SDLtoFS2[SDL_SCANCODE_5] = KEY_5;
	SDLtoFS2[SDL_SCANCODE_6] = KEY_6;
	SDLtoFS2[SDL_SCANCODE_7] = KEY_7;
	SDLtoFS2[SDL_SCANCODE_8] = KEY_8;
	SDLtoFS2[SDL_SCANCODE_9] = KEY_9;

	SDLtoFS2[SDL_SCANCODE_A] = KEY_A;
	SDLtoFS2[SDL_SCANCODE_B] = KEY_B;
	SDLtoFS2[SDL_SCANCODE_C] = KEY_C;
	SDLtoFS2[SDL_SCANCODE_D] = KEY_D;
	SDLtoFS2[SDL_SCANCODE_E] = KEY_E;
	SDLtoFS2[SDL_SCANCODE_F] = KEY_F;
	SDLtoFS2[SDL_SCANCODE_G] = KEY_G;
	SDLtoFS2[SDL_SCANCODE_H] = KEY_H;
	SDLtoFS2[SDL_SCANCODE_I] = KEY_I;
	SDLtoFS2[SDL_SCANCODE_J] = KEY_J;
	SDLtoFS2[SDL_SCANCODE_K] = KEY_K;
	SDLtoFS2[SDL_SCANCODE_L] = KEY_L;
	SDLtoFS2[SDL_SCANCODE_M] = KEY_M;
	SDLtoFS2[SDL_SCANCODE_N] = KEY_N;
	SDLtoFS2[SDL_SCANCODE_O] = KEY_O;
	SDLtoFS2[SDL_SCANCODE_P] = KEY_P;
	SDLtoFS2[SDL_SCANCODE_Q] = KEY_Q;
	SDLtoFS2[SDL_SCANCODE_R] = KEY_R;
	SDLtoFS2[SDL_SCANCODE_S] = KEY_S;
	SDLtoFS2[SDL_SCANCODE_T] = KEY_T;
	SDLtoFS2[SDL_SCANCODE_U] = KEY_U;
	SDLtoFS2[SDL_SCANCODE_V] = KEY_V;
	SDLtoFS2[SDL_SCANCODE_W] = KEY_W;
	SDLtoFS2[SDL_SCANCODE_X] = KEY_X;
	SDLtoFS2[SDL_SCANCODE_Y] = KEY_Y;
	SDLtoFS2[SDL_SCANCODE_Z] = KEY_Z;


	SDLtoFS2[SDL_SCANCODE_MINUS] = KEY_MINUS;
	SDLtoFS2[SDL_SCANCODE_EQUALS] = KEY_EQUAL;
	SDLtoFS2[SDL_SCANCODE_SLASH] = KEY_DIVIDE; // No idea - DDOI
	SDLtoFS2[SDL_SCANCODE_BACKSLASH] = KEY_SLASH;
	SDLtoFS2[SDL_SCANCODE_NONUSBACKSLASH] = KEY_SLASH_UK;
	SDLtoFS2[SDL_SCANCODE_COMMA] = KEY_COMMA;
	SDLtoFS2[SDL_SCANCODE_PERIOD] = KEY_PERIOD;
	SDLtoFS2[SDL_SCANCODE_SEMICOLON] = KEY_SEMICOL;

	SDLtoFS2[SDL_SCANCODE_LEFTBRACKET] = KEY_LBRACKET;
	SDLtoFS2[SDL_SCANCODE_RIGHTBRACKET] = KEY_RBRACKET;

	SDLtoFS2[SDL_SCANCODE_GRAVE] = KEY_LAPOSTRO;
	SDLtoFS2[SDL_SCANCODE_APOSTROPHE] = KEY_RAPOSTRO;


	SDLtoFS2[SDL_SCANCODE_ESCAPE] = KEY_ESC;
	SDLtoFS2[SDL_SCANCODE_RETURN] = KEY_ENTER;
	SDLtoFS2[SDL_SCANCODE_BACKSPACE] = KEY_BACKSP;
	SDLtoFS2[SDL_SCANCODE_TAB] = KEY_TAB;
	SDLtoFS2[SDL_SCANCODE_SPACE] = KEY_SPACEBAR;

	SDLtoFS2[SDL_SCANCODE_NUMLOCKCLEAR] = KEY_NUMLOCK;
	SDLtoFS2[SDL_SCANCODE_SCROLLLOCK] = KEY_SCROLLOCK;
	SDLtoFS2[SDL_SCANCODE_CAPSLOCK] = KEY_CAPSLOCK;

	SDLtoFS2[SDL_SCANCODE_LSHIFT] = KEY_LSHIFT;
	SDLtoFS2[SDL_SCANCODE_RSHIFT] = KEY_RSHIFT;

	SDLtoFS2[SDL_SCANCODE_LALT] = KEY_LALT;
	SDLtoFS2[SDL_SCANCODE_RALT] = KEY_RALT;

	SDLtoFS2[SDL_SCANCODE_LCTRL] = KEY_LCTRL;
	SDLtoFS2[SDL_SCANCODE_RCTRL] = KEY_RCTRL;

	SDLtoFS2[SDL_SCANCODE_F1] = KEY_F1;
	SDLtoFS2[SDL_SCANCODE_F2] = KEY_F2;
	SDLtoFS2[SDL_SCANCODE_F3] = KEY_F3;
	SDLtoFS2[SDL_SCANCODE_F4] = KEY_F4;
	SDLtoFS2[SDL_SCANCODE_F5] = KEY_F5;
	SDLtoFS2[SDL_SCANCODE_F6] = KEY_F6;
	SDLtoFS2[SDL_SCANCODE_F7] = KEY_F7;
	SDLtoFS2[SDL_SCANCODE_F8] = KEY_F8;
	SDLtoFS2[SDL_SCANCODE_F9] = KEY_F9;
	SDLtoFS2[SDL_SCANCODE_F10] = KEY_F10;
	SDLtoFS2[SDL_SCANCODE_F11] = KEY_F11;
	SDLtoFS2[SDL_SCANCODE_F12] = KEY_F12;

	SDLtoFS2[SDL_SCANCODE_KP_0] = KEY_PAD0;
	SDLtoFS2[SDL_SCANCODE_KP_1] = KEY_PAD1;
	SDLtoFS2[SDL_SCANCODE_KP_2] = KEY_PAD2;
	SDLtoFS2[SDL_SCANCODE_KP_3] = KEY_PAD3;
	SDLtoFS2[SDL_SCANCODE_KP_4] = KEY_PAD4;
	SDLtoFS2[SDL_SCANCODE_KP_5] = KEY_PAD5;
	SDLtoFS2[SDL_SCANCODE_KP_6] = KEY_PAD6;
	SDLtoFS2[SDL_SCANCODE_KP_7] = KEY_PAD7;
	SDLtoFS2[SDL_SCANCODE_KP_8] = KEY_PAD8;
	SDLtoFS2[SDL_SCANCODE_KP_9] = KEY_PAD9;
	SDLtoFS2[SDL_SCANCODE_KP_MINUS] = KEY_PADMINUS;
	SDLtoFS2[SDL_SCANCODE_KP_PLUS] = KEY_PADPLUS;
	SDLtoFS2[SDL_SCANCODE_KP_PERIOD] = KEY_PADPERIOD;
	SDLtoFS2[SDL_SCANCODE_KP_DIVIDE] = KEY_PADDIVIDE;
	SDLtoFS2[SDL_SCANCODE_KP_MULTIPLY] = KEY_PADMULTIPLY;
	SDLtoFS2[SDL_SCANCODE_KP_ENTER] = KEY_PADENTER;

	SDLtoFS2[SDL_SCANCODE_INSERT] = KEY_INSERT;
	SDLtoFS2[SDL_SCANCODE_HOME] = KEY_HOME;
	SDLtoFS2[SDL_SCANCODE_PAGEUP] = KEY_PAGEUP;
	SDLtoFS2[SDL_SCANCODE_DELETE] = KEY_DELETE;
	SDLtoFS2[SDL_SCANCODE_END] = KEY_END;
	SDLtoFS2[SDL_SCANCODE_PAGEDOWN] = KEY_PAGEDOWN;
	SDLtoFS2[SDL_SCANCODE_UP] = KEY_UP;
	SDLtoFS2[SDL_SCANCODE_DOWN] = KEY_DOWN;
	SDLtoFS2[SDL_SCANCODE_LEFT] = KEY_LEFT;
	SDLtoFS2[SDL_SCANCODE_RIGHT] = KEY_RIGHT;

	SDLtoFS2[SDL_SCANCODE_PRINTSCREEN] = KEY_PRINT_SCRN;
	SDLtoFS2[SDL_SCANCODE_PAUSE] = KEY_PAUSE;
	//	SDLtoFS2[SDL_SCANCODE_BREAK] = KEY_BREAK;

	for (auto& lookup : SDLtoFS2) {
		FS2toSDL[lookup.second] = lookup.first;
	}
}

int key_numlock_is_on()
{
	const Uint8 *state = SDL_GetKeyboardState(NULL);

	return state[SDL_SCANCODE_NUMLOCKCLEAR];
}

void key_turn_off_numlock()
{

}

void key_turn_on_numlock()
{

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

	SDL_LockMutex( key_lock );	

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

	SDL_UnlockMutex( key_lock );	
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

	SDL_LockMutex( key_lock );	

	if (key_data.keytail != key_data.keyhead){
		is_one_waiting = 1;
	}

	SDL_UnlockMutex( key_lock );		

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

	SDL_LockMutex( key_lock );	

	if (key_data.keytail!=key_data.keyhead)	{
		key = key_data.keybuffer[key_data.keyhead];
		key_data.keyhead = add_one(key_data.keyhead);
	}

	SDL_UnlockMutex( key_lock );	

	Current_key_down = key;

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

	SDL_LockMutex( key_lock );		

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
	SDL_UnlockMutex( key_lock );		

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

	SDL_LockMutex( key_lock );		

	time = timer_get_milliseconds();
	delta_time = time - key_data.TimeKeyDownChecked[scancode];
	key_data.TimeKeyDownChecked[scancode] = time;

	if ( delta_time <= 1 ) {
		key_data.TimeKeyWentDown[scancode] = time;
		if (keyd_pressed[scancode])	{
			SDL_UnlockMutex( key_lock );		
			return 1.0f;
		} else	{
			SDL_UnlockMutex( key_lock );		
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

	SDL_UnlockMutex( key_lock );		

	return i2fl(time_down) / i2fl(delta_time);
}

// Returns number of times key has went from up to down since last call.
int key_down_count(int scancode)	
{
	int n;

	if ( !key_inited ) return 0;
	if ((scancode<0)|| (scancode>=NUM_KEYS)) return 0;

	SDL_LockMutex( key_lock );		

	n = key_data.NumDowns[scancode];
	key_data.NumDowns[scancode] = 0;

	SDL_UnlockMutex( key_lock );		

	return n;
}

//	Add a key up or down code to the key buffer.  state=1 -> down, state=0 -> up
// latency => time difference in ms between when key was actually pressed and now
//void key_mark( uint code, int state )
void key_mark( uint code, int state, uint latency )
{
	uint scancode, breakbit, temp, event_time;
	ushort keycode;	

	if ( !key_inited ) return;

	SDL_LockMutex( key_lock );		

	// If running in the UK, need to translate their wacky slash scancode to ours
	if ( code == KEY_SLASH_UK ) {
		code = KEY_SLASH;
	}

	Assert( code < NUM_KEYS );

	event_time = timer_get_milliseconds() - latency;
	// event_time = timeGetTime() - latency;

	// Read in scancode
	scancode = code & (NUM_KEYS-1);
	breakbit = !state;
	
	if (breakbit) {
		// Key going up
		keyd_last_released = scancode;
		keyd_pressed[scancode] = 0;
		key_data.NumUps[scancode]++;

		// What is the point of this code?  "temp" is never used!
		temp = 0;
		temp |= keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT];
		temp |= keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT];
		temp |= keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL];
		temp |= keyd_pressed[KEY_DEBUG_KEY];
	
		if (event_time < key_data.TimeKeyWentDown[scancode]) {
			key_data.TimeKeyHeldDown[scancode] = 0;
		} else {
			key_data.TimeKeyHeldDown[scancode] += event_time - key_data.TimeKeyWentDown[scancode];
		}

		Current_key_down = scancode;
		if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT] ) {
			Current_key_down |= KEY_SHIFTED;
		}

		if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT] ) {
			Current_key_down |= KEY_ALTED;
		}

		if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL] ) {
			Current_key_down |= KEY_CTRLED;
		}

		Script_system.SetHookVar("Key", 's', textify_scancode_universal(Current_key_down));
		Script_system.RunCondition(CHA_KEYRELEASED);
		Script_system.RemHookVar("Key");
	} else {
		// Key going down
		keyd_last_pressed = scancode;
		keyd_time_when_last_pressed = event_time;
		if (!keyd_pressed[scancode]) {
			// First time down
			key_data.TimeKeyWentDown[scancode] = event_time;
			keyd_pressed[scancode] = 1;
			key_data.NumDowns[scancode]++;
			key_data.down_check[scancode]++;

			//WMC - For scripting
			Current_key_down = scancode;
			if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT] ) {
				Current_key_down |= KEY_SHIFTED;
			}

			if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT] ) {
				Current_key_down |= KEY_ALTED;
			}

			if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL] ) {
				Current_key_down |= KEY_CTRLED;
			}

			// We use the universal value here to keep the scripting interface consistent regardless of the current language
			Script_system.SetHookVar("Key", 's', textify_scancode_universal(Current_key_down));
			Script_system.RunCondition(CHA_KEYPRESSED);
			Script_system.RemHookVar("Key");
		} else if (!keyd_repeat) {
			// Don't buffer repeating key if repeat mode is off
			scancode = 0xAA;		
		} 

		if ( scancode!=0xAA ) {
			keycode = (unsigned short)scancode;

			if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT] ) {
				keycode |= KEY_SHIFTED;
			}

			if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT] ) {
				keycode |= KEY_ALTED;
			}

			if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL] ) {
				keycode |= KEY_CTRLED;
			}

#ifndef NDEBUG
			if ( keyd_pressed[KEY_DEBUG_KEY] ) {
				keycode |= KEY_DEBUGGED;
			}
#else
			if ( keyd_pressed[KEY_DEBUG_KEY] ) {
				mprintf(("Cheats_enabled = %i, Key_normal_game = %i\n", Cheats_enabled, Key_normal_game));
				if (Cheats_enabled && Key_normal_game) {
					keycode |= KEY_DEBUGGED1;
				}
			}

#endif

			if ( keycode ) {
				temp = key_data.keytail+1;
				if ( temp >= KEY_BUFFER_SIZE ) temp=0;

				if (temp!=key_data.keyhead) {
					key_data.keybuffer[key_data.keytail] = keycode;
					key_data.time_pressed[key_data.keytail] = keyd_time_when_last_pressed;
					key_data.keytail = temp;
				}
			}
		}
	}

	SDL_UnlockMutex( key_lock );
}

void key_close()
{
	if ( !key_inited ) return;

	if ( Key_numlock_was_on ) {
		key_turn_on_numlock();
		Key_numlock_was_on = 0;
	}

	key_inited = 0;

	SDL_DestroyMutex( key_lock );
}

void key_init()
{
	// Initialize queue
	if ( key_inited ) return;
	key_inited = 1;

	key_lock = SDL_CreateMutex();

	SDL_LockMutex(key_lock);

	FillSDLArray();

	keyd_time_when_last_pressed = timer_get_milliseconds();
	keyd_buffer_type = 1;
	keyd_repeat = 1;

	// Clear the keyboard array
	key_flush();

	SDL_UnlockMutex( key_lock );

	os::events::addEventListener(SDL_KEYDOWN, os::events::DEFAULT_LISTENER_WEIGHT, key_down_event_handler);
	os::events::addEventListener(SDL_KEYUP, os::events::DEFAULT_LISTENER_WEIGHT, key_up_event_handler);

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
