/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#include <stdarg.h>
#include "network/multi_log.h"
#include "cfile/cfile.h"



// ----------------------------------------------------------------------------------------------------
// MULTI LOGFILE DEFINES/VARS
//

// max length for a line of the logfile
#define MAX_LOGFILE_LINE_LEN					256

// name of the multiplayer logfile
#define MULTI_LOGFILE_NAME						"multi.log"

// echo all ml_printf's to the debug window
//#define MULTI_LOGFILE_ECHO_TO_DEBUG

// how often we'll write an update to the logfile (in seconds)
#define MULTI_LOGFILE_UPDATE_TIME			2520			// every 42 minutes

// outfile itself
CFILE *Multi_log_out = NULL;

// time when the logfile was opened
int Multi_log_open_systime = -1;

// time when we last updated the logfile
int Multi_log_update_systime = -1;

// ----------------------------------------------------------------------------------------------------
// MULTI LOGFILE FUNCTIONS
//

// write the standard header to the logfile
void multi_log_write_header()
{
	char str[1024];
	time_t timer;	

	// header message
	timer = time(NULL);	
	strftime(str, 1024, "FreeSpace Multi Log - Opened %a, %b %d, %Y  at %I:%M%p\n----\n----\n----\n\n", localtime(&timer));
	ml_string(str, 0);	
}

// write the standard shutdown trailer
void multi_log_write_trailer()
{
	char str[1024];
	time_t timer;		

	// header message
	timer = time(NULL);
	strftime(str, 1024, "\n\n----\n----\n----\nFreeSpace Multi Log - Closing on %a, %b %d, %Y  at %I:%M%p", localtime(&timer));
	ml_string(str, 0);	
}

// write out some info about stuff
void multi_log_write_update()
{
	int diff = (int)difftime(time(NULL), Multi_log_open_systime);
	int hours, mins, seconds;

	// figure out some time values
	hours = diff / 3600;
	mins = (diff - (hours * 3600)) / 60;
	seconds = (diff - (hours * 3600) - (mins * 60));

	// print it out
	ml_printf("Server has been active for %d hours, %d minutes, and %d seconds", hours, mins, seconds);
}

// initialize the multi logfile
void multi_log_init()
{
	// attempt to open the file
	Multi_log_out = cfopen(MULTI_LOGFILE_NAME, "wt", CFILE_NORMAL, CF_TYPE_DATA);

	// if we successfully opened the file, write the header
	if(Multi_log_out != NULL){
		multi_log_write_header();

		// initialize our timer info
		Multi_log_open_systime = (int) time(NULL);
		Multi_log_update_systime = Multi_log_open_systime;
	} else {
		nprintf(("Network","Error opening %s for writing!!\n",MULTI_LOGFILE_NAME));
	}
}

// close down the multi logfile
void multi_log_close()
{
	// if we have a valid file, write a trailer and close
	if(Multi_log_out != NULL){
		multi_log_write_trailer();

		cfclose(Multi_log_out);
		Multi_log_out = NULL;
	}
}

// give some processing time to the logfile system so it can check up on stuff
void multi_log_process()
{
	// if we don't have a valid logfile, do nothing
	if(Multi_log_out == NULL){
		return;
	}

	// check to see if we've been active a long enough time, and 
	if(time(NULL) - Multi_log_update_systime > MULTI_LOGFILE_UPDATE_TIME){
		// write the update
		multi_log_write_update();

		Multi_log_update_systime = (int) time(NULL);
	}
}

// printf function itself called by the ml_printf macro
void ml_printf(char *format, ...)
{
	char tmp[MAX_LOGFILE_LINE_LEN*4];
	va_list args;

	// if we don't have a valid logfile do nothing
	if(Multi_log_out == NULL){
		return;
	}
	
	// format the text
	va_start(args, format);
	vsprintf(tmp, format, args);
	va_end(args);
	
	// log the string
	ml_string(tmp);
}

// string print function
void ml_string(char *string, int add_time)
{
	char tmp[MAX_LOGFILE_LINE_LEN*4];
	char time_str[128];
	time_t timer;	

	// if we don't have a valid logfile do nothing
	if(Multi_log_out == NULL){
		return;
	}

	// if the passed string is NULL, do nothing
	if(string == NULL){
		return;
	}

	// maybe add the time
	if(add_time){
		timer = time(NULL);

		strftime(time_str, 128, "%m/%d %H:%M:%S~   ", localtime(&timer));
		strcpy_s(tmp, time_str);
		strcat_s(tmp, string);
	} else{
		strcpy_s(tmp, string);
	}
	strcat_s(tmp, "\n");

	// now print it to the logfile if necessary	
	cfputs(tmp, Multi_log_out);
	cflush(Multi_log_out);

	// add to standalone UI too
	extern int Is_standalone;
	extern void std_debug_multilog_add_line(const char *str);
	if (Is_standalone) {
		std_debug_multilog_add_line(tmp);
	}

#if defined(MULTI_LOGFILE_ECHO_TO_DEBUG)
	// nprintf(("Network","%s\n",tmp));
	mprintf(("ML %s", tmp));
#endif
}
