/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _KEY_H
#define _KEY_H

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

#include "globalincs/pstypes.h"

#define NUM_KEYS 256

extern int shifted_ascii_table[];
extern int ascii_table[];

extern ubyte keyd_pressed[NUM_KEYS];

// O/S level hooks...
void key_init();
void key_level_init();
void key_lost_focus();
void key_got_focus();
void key_mark( uint code, int state, uint latency );
int key_getch();
int key_peekkey();
void key_flush();

// Routines/data you can access:
//NOT USED! extern fix key_down_time( uint code );
float key_down_timef( uint code );

int key_to_ascii(int keycode );
int key_inkey();

// global flag that will enable/disable the backspace key from stopping execution
//extern int Backspace_debug;

uint key_get_shift_status();
int key_down_count(int scancode);
int key_up_count(int scancode);
int key_checkch();
int key_check(int key);

//	Put "key" back in the input buffer.
void key_outkey(int key);

// used to restrict keys that are read into keyboard buffer
void key_set_filter(int *filter_array, int num);
void key_clear_filter();

extern int Cheats_enabled;
extern int Key_normal_game;

#define KEY_SHIFTED     0x1000
#define KEY_ALTED       0x2000
#define KEY_CTRLED      0x4000
#define KEY_DEBUGGED		0x8000
#define KEY_DEBUGGED1	0x0800		//	Cheat bit in release version of game.
#define KEY_MASK			0x00FF

#define KEY_DEBUG_KEY	0x29			//	KEY_LAPOSTRO (shifted = tilde, near upper-left of keyboard)

#define KEY_0           0x0B
#define KEY_1           0x02
#define KEY_2           0x03
#define KEY_3           0x04
#define KEY_4           0x05
#define KEY_5           0x06
#define KEY_6           0x07
#define KEY_7           0x08
#define KEY_8           0x09
#define KEY_9           0x0A

#define KEY_A           0x1E
#define KEY_B           0x30
#define KEY_C           0x2E
#define KEY_D           0x20
#define KEY_E           0x12
#define KEY_F           0x21
#define KEY_G           0x22
#define KEY_H           0x23
#define KEY_I           0x17
#define KEY_J           0x24
#define KEY_K           0x25
#define KEY_L           0x26
#define KEY_M           0x32
#define KEY_N           0x31
#define KEY_O           0x18
#define KEY_P           0x19
#define KEY_Q           0x10
#define KEY_R           0x13
#define KEY_S           0x1F
#define KEY_T           0x14
#define KEY_U           0x16
#define KEY_V           0x2F
#define KEY_W           0x11
#define KEY_X           0x2D
#define KEY_Y           0x15
#define KEY_Z           0x2C

#define KEY_MINUS       0x0C
#define KEY_EQUAL       0x0D
#define KEY_DIVIDE      0x35
#define KEY_SLASH       0x2B
#define KEY_SLASH_UK		0x56
#define KEY_COMMA       0x33
#define KEY_PERIOD      0x34
#define KEY_SEMICOL     0x27

#define KEY_LBRACKET    0x1A
#define KEY_RBRACKET    0x1B

#define KEY_RAPOSTRO    0x28
#define KEY_LAPOSTRO    0x29

#define KEY_ESC         0x01
#define KEY_ENTER       0x1C
#define KEY_BACKSP      0x0E
#define KEY_TAB         0x0F
#define KEY_SPACEBAR    0x39

#define KEY_NUMLOCK     0x45
#define KEY_SCROLLOCK   0x46
#define KEY_CAPSLOCK    0x3A

#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36

#define KEY_LALT        0x38
#define KEY_RALT        0xB8

#define KEY_LCTRL       0x1D
#define KEY_RCTRL       0x9D

#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_F11         0x57
#define KEY_F12         0x58

#define KEY_PAD0        0x52
#define KEY_PAD1        0x4F
#define KEY_PAD2        0x50
#define KEY_PAD3        0x51
#define KEY_PAD4        0x4B
#define KEY_PAD5        0x4C
#define KEY_PAD6        0x4D
#define KEY_PAD7        0x47
#define KEY_PAD8        0x48
#define KEY_PAD9        0x49
#define KEY_PADMINUS    0x4A
#define KEY_PADPLUS     0x4E
#define KEY_PADPERIOD   0x53
#define KEY_PADDIVIDE   0xB5
#define KEY_PADMULTIPLY 0x37
#define KEY_PADENTER    0x9C

#define KEY_INSERT      0xD2
#define KEY_HOME        0xC7
#define KEY_PAGEUP      0xC9
#define KEY_DELETE      0xd3
#define KEY_END         0xCF
#define KEY_PAGEDOWN    0xD1
#define KEY_UP          0xC8
#define KEY_DOWN        0xD0
#define KEY_LEFT        0xCB
#define KEY_RIGHT       0xCD

#define KEY_PRINT_SCRN	0xB7
#define KEY_PAUSE			0x45	//DOS: 0x61
#define KEY_BREAK			0xc6

/*
#ifdef __cplusplus
}
#endif
*/

#endif
