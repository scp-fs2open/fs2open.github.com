/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <string.h>

#include "globalincs/pstypes.h"
#include "graphics/font.h"
#include "io/timer.h"
#include "io/key.h"
#include "globalincs/alphacolors.h"
#include "osapi/osapi.h"


#define MAX_COMMANDS 300

static int Num_debug_commands = 0;
static debug_command *Debug_command[MAX_COMMANDS];


debug_command::debug_command(char *_name, char *_help, void (*_func)() )
{
	int i;

	if ( Num_debug_commands >= MAX_COMMANDS ) {
		Int3();			// Too many debug console commands!! Increase MAX_COMMANDS!!
		return;
	}

	for (i=0; i<Num_debug_commands; i++ ) {
		int ret  = stricmp( Debug_command[i]->name, _name );

		if ( ret == 0) {
			Int3();		// This debug console command already exists!!!! 
			return;
		} else if ( ret > 0 ) {
			break;		// Insert it here

		} else if ( ret < 0 ) {
			// do nothing
		}
	}

	if ( i < Num_debug_commands ) {
		// Insert it at element i
		int j;
		for (j=Num_debug_commands; j>i; j-- ) {
			Debug_command[j] = Debug_command[j-1];
		}
		Debug_command[i] = this;
		Num_debug_commands++;
	} else {
		Debug_command[Num_debug_commands] = this;
		Num_debug_commands++;
	}

	name = _name;
	help = _help;
	func = _func;
}

// some global variables
int Dc_command;			// If this is set, then process the command
int Dc_help;			// If this is set, then print out the help text in the form, "usage: ... \nLong description\n" );
int Dc_status;			// If this is set, then print out the current status of the command.
char *Dc_arg;			// The (lowercased) string value of the argument retrieved from dc_arg
char *Dc_arg_org;		// Dc_arg before it got converted to lowercase
uint Dc_arg_type;		// The type of dc_arg.
char *Dc_command_line;	// The rest of the command line, from the end of the last processed arg on.
int Dc_arg_int;			// If Dc_arg_type & ARG_INT is set, then this is the value
ubyte Dc_arg_ubyte;		// If Dc_arg_type & ARG_UBYTE is set, then this is the value
float Dc_arg_float;		// If Dc_arg_type & ARG_FLOAT is set, then this is the value

int scroll_times = 0;	// incremented each time display scrolls

int debug_inited = 0;

#define DROWS 25
#define DCOLS 80

int debug_x=0, debug_y=0;
char debug_text[DROWS][DCOLS];


static char command_line[1024];
static int command_line_pos = 0;
#define DEBUG_HISTORY 16
static char oldcommand_line[DEBUG_HISTORY][1024];
int last_oldcommand=-1;
int command_scroll = 0;

///=========================== SCANNER =======================
typedef enum {
	LETTER, QUOTE, SPECIAL, EOF_CODE, DIGIT
} CHAR_CODE;

typedef enum {
	NO_TOKEN, IDENTIFIER, NUMBER, STRING
} TOKEN_CODE;


#define MAX_TOKEN_STRING_LENGTH 128

char		scanner_ch;
TOKEN_CODE	scanner_token;

char scanner_token_string[MAX_TOKEN_STRING_LENGTH];
char scanner_word_string[MAX_TOKEN_STRING_LENGTH];
char * scanner_bufferp = "";
char * scanner_tokenp = scanner_token_string;

CHAR_CODE scanner_char_table[256];

#define scanner_char_code(x) scanner_char_table[x]

void scanner_get_char()
{
	if ( *scanner_bufferp == '\0' ) {
		scanner_ch = 0;
		return;
	}
	scanner_ch = *scanner_bufferp++;
}

void scanner_init()
{
	int ch;
	for (ch=0; ch<256; ++ch) scanner_char_table[ch] = SPECIAL;
	for (ch='0'; ch<='9'; ++ch) scanner_char_table[ch] = DIGIT;
	for (ch='A'; ch<='Z'; ++ch) scanner_char_table[ch] = LETTER;
	for (ch='a'; ch<='z'; ++ch) scanner_char_table[ch] = LETTER;

	scanner_char_table['.'] = DIGIT;
	scanner_char_table['-'] = DIGIT;
	scanner_char_table['+'] = DIGIT;

	scanner_char_table['_'] = LETTER;
	scanner_char_table[34] = QUOTE;
	scanner_char_table[0] = EOF_CODE;

	scanner_char_table[':'] = LETTER;
	scanner_char_table['\\'] = LETTER;

	scanner_ch = 0;
}

void scanner_skip_blanks()
{
	while( (scanner_ch ==' ') || (scanner_ch =='\t') )
		scanner_get_char();
}

void scanner_downshift_word()
{
	int offset = 'a' - 'A';
	char * tp;

	strcpy_s( scanner_word_string, scanner_token_string );
	
	tp = scanner_word_string;
	do {
		*tp = (char)((*tp>='A') && (*tp <='Z') ? *tp + offset : *tp) ;
		tp++;
	} while (*tp != '\0' );
}

void scanner_get_word()
{
	while( (scanner_char_code(scanner_ch)==LETTER) || (scanner_char_code(scanner_ch)==DIGIT) ) {
		*scanner_tokenp++ = scanner_ch;
		scanner_get_char();
	}
	*scanner_tokenp = '\0';

	scanner_token = IDENTIFIER;
}

void scanner_get_string()
{
	*scanner_tokenp++ = 34;
	scanner_get_char();

	while(scanner_ch != 34 ) {
		*scanner_tokenp++ = scanner_ch;
		scanner_get_char();
	}
	scanner_get_char();
	*scanner_tokenp++ = 34;
	*scanner_tokenp = '\0';
	scanner_token = STRING;
}

void scanner_get_token()
{
	scanner_skip_blanks();
	scanner_tokenp = scanner_token_string;
	*scanner_tokenp = 0;

	switch( scanner_char_code(scanner_ch) ) {
	case QUOTE: scanner_get_string(); break;
	case EOF_CODE: scanner_token = NO_TOKEN; break;
	case DIGIT:
	case LETTER: scanner_get_word(); break;
	default:
		*scanner_tokenp++ = scanner_ch;
		*scanner_tokenp = '\0';
		scanner_get_char();
		scanner_token = IDENTIFIER;
		break;
	}

	scanner_downshift_word();
}

void scanner_start_command( char * s )
{
	scanner_bufferp = s;
	scanner_get_char();
}

int Dc_debug_on = 0;
jmp_buf dc_bad_arg;

void dc_get_arg(uint type)
{
	scanner_get_token();

	Dc_command_line = scanner_bufferp;
	Dc_arg_org = scanner_token_string;
	Dc_arg = scanner_word_string;

	if (Dc_debug_on) {
		dc_printf( "next arg is '%s', was originally '%s'\n", Dc_arg, Dc_arg_org );
		dc_printf( "Rest of the command line is '%s'\n", Dc_command_line );
	}

	if ( scanner_token == NO_TOKEN ) {
		Dc_arg_type = ARG_NONE;
	} else if ( scanner_token == IDENTIFIER ) {
		Dc_arg_type = ARG_STRING;
	} else if ( scanner_token == STRING ) {
		Dc_arg_type = ARG_QUOTE;
	} else {
		Dc_arg_type = ARG_STRING;
	}

	if ( Dc_arg_type & ARG_STRING ) {
		int i, num_digits, len;

		len = strlen(Dc_arg);
		num_digits = 0;

		for (i=0; i<len; i++) {
			if ( scanner_char_table[Dc_arg[i]] == DIGIT ) {
				num_digits++;
			}

			if ( num_digits==len ) {
				Dc_arg_type |= ARG_FLOAT;
				Dc_arg_float = (float)atof(Dc_arg);
				if ( !strchr( Dc_arg, '.' )) {
					Dc_arg_type |= ARG_INT;
					Dc_arg_int = atoi(Dc_arg);
					Dc_arg_type |= ARG_UBYTE;
					Dc_arg_ubyte = (ubyte)atoi(Dc_arg);
				}
			} else {
				if ( (Dc_arg[0] == '0') && (Dc_arg[1] == 'x') ) {
					char *p;
					int n;
					n = strtol(Dc_arg,&p,0);
					if ( *p == 0 ) {
						Dc_arg_type |= ARG_INT|ARG_HEX;
						Dc_arg_int = n;
					}
				}
			}
		}

		if (Dc_debug_on) {
			if ( Dc_arg_type & ARG_FLOAT ) {
				dc_printf( "Found float number! %f\n", Dc_arg_float );
			}

			if ( Dc_arg_type & ARG_INT ) {
				dc_printf( "Found int number! %d\n", Dc_arg_int );
			}

			if ( Dc_arg_type & ARG_UBYTE ) {
				dc_printf( "Found ubyte number! %d\n", Dc_arg_ubyte );
			}

			if ( Dc_arg_type & ARG_HEX ) {
				dc_printf( "Found hex number! 0x%x\n", Dc_arg_int );
			}
		}

		if ( !stricmp( Dc_arg, "on" ) ) {
			Dc_arg_type |= ARG_TRUE;
		}
		if ( !stricmp( Dc_arg, "true" ) ) {
			Dc_arg_type |= ARG_TRUE;
		}
		if ( !stricmp( Dc_arg, "off" ) ) {
			Dc_arg_type |= ARG_FALSE;
		}
		if ( !stricmp( Dc_arg, "false" ) ) {
			Dc_arg_type |= ARG_FALSE;
		}

		if ( !stricmp( Dc_arg, "+" ) ) {
			Dc_arg_type |= ARG_PLUS;
		}

		if ( !stricmp( Dc_arg, "-" ) ) {
			Dc_arg_type |= ARG_MINUS;
		}

		if ( !stricmp( Dc_arg, "," ) ) {
			Dc_arg_type |= ARG_COMMA;
		}
	}

	if ( Dc_arg_type & ARG_INT) {
		if ( Dc_arg_int ) {
			Dc_arg_type |= ARG_TRUE;
		} else {
			Dc_arg_type |= ARG_FALSE;
		}
	}

	if ( Dc_arg_type & ARG_UBYTE) {
		if ( Dc_arg_ubyte ) {
			Dc_arg_type |= ARG_TRUE;
		} else {
			Dc_arg_type |= ARG_FALSE;
		}
	}

	if ( !(Dc_arg_type&type) ) {
		if ( (Dc_arg_type & ARG_NONE) && !(type & ARG_NONE) ) {
			dc_printf( "Error: Not enough parameters.\n" );
		} else {
			dc_printf( "Error: '%s' invalid type\n", Dc_arg );
			longjmp(dc_bad_arg,1);
		}
	}
}

void debug_help();

void debug_do_command(char * command)
{

	int i;
	int mode = 0;

	if ( strlen(command) < 1 ) {
		return;
	}

	Dc_debug_on = 0;
	Dc_command_line = command;
	scanner_start_command(command);

	if (setjmp(dc_bad_arg) ) {
		return;
	}

	dc_get_arg( ARG_ANY );

	if ( !strcmp( Dc_arg, "debug" ) ) {
		Dc_debug_on = 1;
		dc_printf( "Command line: '%s'\n", Dc_command_line );
		dc_get_arg( ARG_ANY );
	}

	if ( !stricmp( Dc_arg, "xyzzy" ) ) {
		dc_printf("Nothing happens.\n");
		return;
	}

	if ( !strcmp( Dc_arg, "?" ) ) {
		mode = 1;
		dc_get_arg( ARG_ANY );

		if ( Dc_arg_type&ARG_NONE ) {
			debug_help();
			return;
		}
	}

	if ( !strcmp( Dc_arg, "help" ) || !strcmp( Dc_arg, "man" ) ) {
		mode = 2;
		dc_get_arg( ARG_ANY );
		if ( Dc_arg_type&ARG_NONE ) {
			debug_help();
			return;
		}
	}

	if ( strstr( Dc_command_line, "?" ) ) {
		mode = 2;
	}

	if ( !(Dc_arg_type&ARG_STRING) ) {
		dc_printf( "Invalid keyword '%s'\n", Dc_arg );
		return;
	}


	if (Dc_debug_on) {
		dc_printf( "Searching for command '%s'\n", Dc_arg );
	}

	for (i=0; i<Num_debug_commands; i++ ) {
		if ( !stricmp( Debug_command[i]->name, Dc_arg ) ) {
		
			if (mode==0) {
				if (Dc_debug_on) {
					dc_printf( "Calling function '%s'\n", Dc_arg );
				}
				Dc_command = 1;
				Dc_help = 0;
				Dc_status = 1;
			} else if (mode==1) {
				if (Dc_debug_on) {
					dc_printf( "Checking status for '%s'\n", Dc_arg );
				}
				Dc_command = 0;
				Dc_help = 0;
				Dc_status = 1;
			} else {
				if (Dc_debug_on) {
					dc_printf( "Doing help for '%s'\n", Dc_arg );
				}
				Dc_command = 0;
				Dc_help = 1;
				Dc_status = 0;
			}

			(*Debug_command[i]->func)();

			if (mode==0) {
				dc_get_arg(ARG_ANY);
				if (!(Dc_arg_type&ARG_NONE)) {
					dc_printf( "Ignoring the unused command line tail '%s %s'\n", Dc_arg_org, Dc_command_line );
				}
			}

			return;
		}
	}

	dc_printf( "Unknown command '%s'\n", Dc_arg );
}

void debug_draw()
{
	int i;

	gr_clear();
	gr_set_font(FONT1);
	gr_set_color_fast( &Color_bright );
	gr_string( 0x8000, 3, "Debug Console" );

	gr_set_color_fast( &Color_normal );

	for (i=0; i<DROWS; i++ ) {
		gr_string( 0, i*16+16, debug_text[i] );
	}

	int t = timer_get_fixed_seconds() / (F1_0/3);
	if ( t & 1 ) {
		int w,h;
		char c;

		c = debug_text[debug_y][command_line_pos+1];
		debug_text[debug_y][command_line_pos+1] = 0;

		gr_get_string_size( &w, &h, debug_text[debug_y] );

		//gr_string( w, debug_y*16, "_" );
		gr_rect(w+1,debug_y*16+1+16,2,14);

		debug_text[debug_y][command_line_pos+1] = c;
	}

	gr_flip();
}


void debug_output( char c )
{
	if ( c == '\t' ) {
		int next_tab = ((debug_x/28)+1)*28;

		if ( next_tab >= DCOLS-1 ) {
			debug_x=0;
			debug_y++;
			scroll_times++;
			if ( debug_y >= DROWS ) {
				int i;
				for (i=1; i<DROWS; i++ ) {
					strcpy_s( debug_text[i-1], debug_text[i] );
				}
				debug_y = DROWS-1;
				debug_x = 0;
				debug_text[debug_y][debug_x] = 0;
			}
			debug_text[debug_y][debug_x] = 0;
			return;
		}
	
		for ( ; debug_x < next_tab; ) {
			debug_text[debug_y][debug_x++] = ' ';
		}
		debug_text[debug_y][debug_x] = 0;
		return;
	}

	if ( (c == '\n') || (debug_x >= DCOLS-1) ) {
		debug_x=0;
		debug_y++;
		scroll_times++;
		if ( debug_y >= DROWS ) {
			int i;
			for (i=1; i<DROWS; i++ ) {
				strcpy_s( debug_text[i-1], debug_text[i] );
			}
			debug_y = DROWS-1;
			debug_x = 0;
			debug_text[debug_y][debug_x] = 0;
		}
		debug_text[debug_y][debug_x] = 0;
		if ( c == '\n' ) {
			return;
		}
	}

	debug_text[debug_y][debug_x++] = c;
	debug_text[debug_y][debug_x] = 0;
}

void dc_printf(char *format, ...)
{
	char tmp[DCOLS*DROWS];
	va_list args;

	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);

	char *p = tmp;
	while( *p != '\0' ) {
		debug_output(*p);
		p++;
	}
}

void debug_init()
{
	int i;
	if ( debug_inited ) {
		return;
	}

	debug_inited = 1;

	debug_x=0;
	debug_y=0;

	for (i=0; i<DROWS; i++ ) {
		debug_text[i][0] = 0;
	}

	dc_printf("Debug console started.\n" );
}

void debug_console( void (*_func)() )
{
	int done = 0;

	scanner_init();

	while( key_inkey() ) {
		os_poll();
	}

	if ( !debug_inited ) {
		debug_init();
	}

	debug_draw();

	while (!done) {
		// poll the os
		os_poll();

		int k = key_inkey();
		switch( k ) {

		case KEY_SHIFTED+KEY_ENTER:
		case KEY_ESC:
			done=1; break;

		case KEY_BACKSP:
			if ( command_line_pos > 0 ) {
				command_line[--command_line_pos] = 0;
			}
			break;

		case KEY_F3:
			if ( last_oldcommand > -1 ) {
				strcpy_s( command_line, oldcommand_line[last_oldcommand] );
				command_line_pos = strlen(command_line);
				command_line[command_line_pos] = 0;
			}
			break;

		case KEY_UP:
			command_scroll--;
			if (command_scroll<0) {
				command_scroll = last_oldcommand;
			}

			if ( command_scroll > -1 ) {
				strcpy_s( command_line, oldcommand_line[command_scroll] );
				command_line_pos = strlen(command_line);
				command_line[command_line_pos] = 0;
			}
			break;

		case KEY_DOWN:
			command_scroll++;
			if (command_scroll>last_oldcommand) {
				command_scroll = 0;
			}
			if (command_scroll>last_oldcommand) {
				command_scroll = -1;
			}
			if ( command_scroll > -1 ) {
				strcpy_s( command_line, oldcommand_line[command_scroll] );
				command_line_pos = strlen(command_line);
				command_line[command_line_pos] = 0;
			}
			break;

		case KEY_ENTER: {
			debug_output( '\n' );
			debug_draw();

			debug_do_command(command_line);

			int i, found = 0;
			for (i=0; i<=last_oldcommand; i++ ) {
				if (!stricmp( oldcommand_line[i], command_line )) {
					found = 1;
				}
			}
			if ( !found ) {
				if ( last_oldcommand < DEBUG_HISTORY-1 ) {
					last_oldcommand++;
					strcpy_s( oldcommand_line[last_oldcommand], command_line);
				} else {
					int iLoop;
					for (iLoop=0; iLoop<last_oldcommand; iLoop++ ) {
						strcpy_s( oldcommand_line[iLoop], oldcommand_line[iLoop+1] );
					}
					strcpy_s( oldcommand_line[last_oldcommand], command_line);
				}
			}

			debug_output( '\n' );
			command_line_pos = 0;
			command_line[command_line_pos] = 0;

			command_scroll = 0;

			} 
			break;
		default: {
				ubyte c = (ubyte)key_to_ascii(k);
				if ( c != 255 ) {
					command_line[command_line_pos++] = c;
					command_line[command_line_pos] = 0;
				}
			}
		}

		strcpy_s( debug_text[debug_y], ">" );
		strcat_s( debug_text[debug_y], command_line );
		debug_draw();

		if ( _func ) {
			_func();
		}
	}

	while( key_inkey() ) {
		os_poll();
	}
}

void debug_help()
{
	int s, i;

	dc_printf( "Available functions:\n\n" );

	s = scroll_times;
	for (i=0; i<Num_debug_commands; i++ ) {
		dc_printf( " %s - %s\n", Debug_command[i]->name, Debug_command[i]->help );

		if ( scroll_times - s > DROWS - 3 ) {
			int k;
			dc_printf( "       Press a key...B for back\n" );
			debug_draw();
			k = key_getch();
			s = scroll_times;
			if ( k == KEY_B ) {
				i -= ((DROWS-3)*2);
				if ( i <= 0 ) {
					i = -1;
				}
			}
		}
		debug_draw();
	}
	dc_printf( "\n" );

	dc_printf( "Typing '? function_name' will give the current status.\n" );
	dc_printf( "Typing 'function_name ?' will give help on the function.\n" );
	dc_printf( "Typing ? or help will give you help.\n");
	dc_printf( "F3 selects last command line.\n" );
}
