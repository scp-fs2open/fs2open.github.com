/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Io/Timer.cpp $
 * $Revision: 2.4 $
 * $Date: 2004-07-12 16:32:51 $
 * $Author: Kazan $
 *
 * Include file for timer stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.3  2003/03/02 05:46:28  penguin
 * Added gcc-compatible asm blocks
 *  - penguin
 *
 * Revision 2.2  2002/08/01 01:41:06  penguin
 * The big include file move
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/24 16:46:47  mharris
 * Fixed stupid error I made in timer_get_fixed_seconds()
 *
 * Revision 1.3  2002/05/17 06:45:53  mharris
 * More porting tweaks.  It links!  but segfaults...
 *
 * Revision 1.2  2002/05/09 13:50:50  mharris
 * temporarily ifdef out asm
 *
 * Revision 1.1  2002/05/02 18:03:08  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     5/26/99 3:19p Dave
 * Fixed release build errors.
 * 
 * 3     5/17/99 6:03p Dave
 * Added new timing code. Put in code to allow camera speed zooming.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 14    4/13/98 10:16a John
 * Switched gettime back to timer_get_milliseconds, which is now thread
 * safe.
 * 
 * 13    4/13/98 10:11a John
 * Made timer functions thread safe.  Made timer_init be called in all
 * projects.
 * 
 * 12    4/13/98 8:09a John
 * Made timer_get functions thread safe.
 * 
 * 11    12/02/97 3:56p John
 * fixed a bug with timestamp_until, created one with it rolling over.
 * 
 * 10    9/11/97 7:12p Hoffoss
 * Added more functionality to training sexp handling code.
 * 
 * 9     8/29/97 4:49p Allender
 * added mprintf to check for wrap problems with timestamp ticker
 * 
 * 8     8/28/97 1:38p Allender
 * from John:  changes to timer func to detect early rollever
 * 
 * 7     7/29/97 5:30p Lawrance
 * move gettime() from keyboard module to timer module
 * 
 * 6     7/16/97 4:42p Mike
 * Make afterburner shake viewer, not ship.
 * Shake for limited time.
 * Add timestamp_until() function to timer library.
 * 
 * 5     2/17/97 5:18p John
 * Added a bunch of RCS headers to a bunch of old files that don't have
 * them.
 *
 * $NoKeywords: $
 */

#if defined _WIN32
	#include <windows.h>
#elif defined unix
	#include <sys/time.h>
	#include <unistd.h>
#endif

#include	"limits.h"
#include "globalincs/pstypes.h"
#include "io/timer.h"
#include "graphics/2d.h"
#include "globalincs/alphacolors.h"

// memory tracking - ALWAYS INCLUDE LAST
#include "mcd/mcd.h"

#ifndef NDEBUG
	#define USE_TIMING
#endif

#if defined _WIN32
static longlong Timer_last_value, Timer_base;
static uint Timer_freq=0;
#elif defined unix
static struct timeval Timer_last_value, Timer_base;
#endif

static int Timer_inited = 0;

static CRITICAL_SECTION Timer_lock;

void timer_close()
{
	if ( Timer_inited )	{
		Timer_inited = 0;
		DeleteCriticalSection( &Timer_lock );
	}
}

void timer_init()
{
	if ( !Timer_inited )	{
#if defined _WIN32
		LARGE_INTEGER tmp;
		QueryPerformanceFrequency(&tmp);
		Assert( tmp.HighPart == 0 );
		Timer_freq = tmp.LowPart;

		QueryPerformanceCounter((LARGE_INTEGER *)&Timer_base);
		QueryPerformanceCounter((LARGE_INTEGER *)&Timer_last_value);
#elif defined unix
		gettimeofday(&Timer_base, NULL);
		gettimeofday(&Timer_last_value, NULL);
#endif
		InitializeCriticalSection(&Timer_lock);
		
		Timer_inited = 1;

		atexit(timer_close);
	}
}

// Fills Time_now with the ticks since program start
#if defined _WIN32
static void timer_get(LARGE_INTEGER * out)
#elif defined unix
static void timer_get(timeval * out)
#endif
{
	EnterCriticalSection(&Timer_lock);

#if defined _WIN32
	longlong time_tmp;
	longlong Time_now;

	QueryPerformanceCounter((LARGE_INTEGER *)&time_tmp);
	if ( time_tmp < Timer_last_value )	{
		// The clock has rolled!
		Timer_base = time_tmp;
		mprintf(( "TIMER ROLLED!\n" ));
		// Hack: I'm not accounting for the time before roll occured,
		// since I'm not sure at what value this timer is going to roll at.
		Time_now = time_tmp;
	}
	Time_now = time_tmp - Timer_base;
	Timer_last_value = time_tmp;

	out->QuadPart = Time_now;
#elif defined unix
	timeval time_tmp;
	gettimeofday(&time_tmp, NULL);
	out->tv_usec = time_tmp.tv_usec - Timer_base.tv_usec;
	out->tv_sec  = time_tmp.tv_sec  - Timer_base.tv_sec;
	if (out->tv_usec < 0) {
		out->tv_usec += 1000000;
		out->tv_sec  -= 1;
	}		
#endif

	LeaveCriticalSection(&Timer_lock);
}

fix timer_get_fixed_seconds()
{
	if (!Timer_inited) {
		Int3();					// Make sure you call timer_init before anything that uses timer functions!
		return 0;
	}

#if defined(_WIN32)
	int tmp;
	LARGE_INTEGER temp_large;
	timer_get(&temp_large);

#if defined(_MSC_VER)
	// Timing in fixed point (16.16) seconds.
	// Can be used for up to 1000 hours
	_asm	mov edx, temp_large.HighPart
	_asm	mov eax, temp_large.LowPart

	_asm	shld    edx, eax, 16            ; Keep 32+11 bits
	_asm	shl     eax, 16			
	// edx:eax = number of 1.19Mhz pulses elapsed.
	_asm	mov     ebx, Timer_freq

	// Make sure we won't divide overflow.  Make time wrap at about 9 hours
sub_again:
	_asm	sub     edx, ebx	; subtract until negative...
	_asm	jns     sub_again	; ...to prevent divide overflow...
	_asm	add     edx, ebx	; ...then add in to get correct value.
	_asm	div     ebx
	//eax = fixed point seconds elapsed...
	_asm mov tmp, eax

#elif defined(__GNUC__)

 	// Timing in fixed point (16.16) seconds.
 	// Can be used for up to 1000 hours
	asm(
		 "mov   %1, %%edx;"		  // temp_large.HighPart
		 "mov   %2, %%eax;"		  // temp_large.LowPart

		 "shld  $16,%%edx,%%eax;"			 // Keep 32+11 bits
		 "shl   $16,%%eax;"
		 // edx:eax = number of 1.19Mhz pulses elapsed.
		 "mov   %3,%%ebx;"		  // Timer_freq

		 // Make sure we won't divide overflow.  Make time wrap at about 9 hours
		 "sub_again:"
		 "sub   %%ebx,%%edx;"	  // subtract until negative...
		 "jns   sub_again;"		  // ...to prevent divide overflow..."
		 "add   %%ebx,%%edx;"	  // ...then add in to get correct value."
		 "div   %%ebx;"
		 //eax = fixed point seconds elapsed...
		 "mov   %%eax,%0;"		  // tmp
		 : "=g" (tmp)
		 : "g" (temp_large.HighPart), "g" (temp_large.LowPart), "g" (Timer_freq)
		 : "edx", "eax", "ebx" );

#else
#error unknown Win32 compiler
#endif

	return tmp;

#elif defined unix

	timeval tmp_tv;
	timer_get(&tmp_tv);
	// TODO: we are not handling overflow properly
	return ((tmp_tv.tv_sec & 0x7fff) << 16)
		| ((tmp_tv.tv_usec << 11) / 31250);

#else
#error unknown architecture/compiler
#endif
}

fix timer_get_fixed_secondsX()
{
	return timer_get_fixed_seconds();
}

fix timer_get_approx_seconds()
{
	return timer_get_fixed_seconds();
}

int timer_get_milliseconds()
{
	if (!Timer_inited) {
		Int3();					// Make sure you call timer_init before anything that uses timer functions!
		return 0;
	}

#if defined(_WIN32)
	int tmp;
	LARGE_INTEGER temp_large;
	timer_get(&temp_large);
	temp_large.QuadPart *= (longlong)1000;

#if defined(_MSC_VER)
	// Timing in milliseconds.
	_asm	mov edx, temp_large.HighPart
	_asm	mov eax, temp_large.LowPart

	//_asm	shld    edx, eax, 16            ; Keep 32+11 bits
	//_asm	shl     eax, 16			
	// edx:eax = number of 1.19Mhz pulses elapsed.
	_asm	mov     ebx, Timer_freq
	// Make sure we won't divide overflow.  Make time wrap at about 9 hours
sub_again:
	_asm	sub     edx, ebx	; subtract until negative...
	_asm	jns     sub_again	; ...to prevent divide overflow...
	_asm	add     edx, ebx	; ...then add in to get correct value.
	_asm	div     ebx
	//eax = milliseconds elapsed...
	_asm mov tmp, eax

#elif defined(__GNUC__)
	// Timing in milliseconds.
	asm(
		 "mov   %1,%%edx;\n"			// temp_large.HighPart
		 "mov   %2,%%eax;\n"			// temp_large.LowPart  
		 // edx:eax = number of 1.19Mhz pulses elapsed.
		 "mov   %3,%%ebx;\n"			// Timer_freq
		 // Make sure we won't divide overflow.  Make time wrap at about 9 hours
		 "sub_again2:"
		 "sub   %%ebx,%%edx;\n"		// subtract until negative...
		 "jns   sub_again2;\n"		// ...to prevent divide overflow...
		 "add   %%ebx,%%edx;\n"		// ...then add in to get correct value.
		 "div   %%ebx;\n"
		 //eax = milliseconds elapsed...
		 "mov   %%eax,%0;\n"		// tmp
		 : "=g" (tmp)
		 : "g" (temp_large.HighPart), "g" (temp_large.LowPart), "g" (Timer_freq)
		 : "edx", "eax", "ebx");
#else
#error unknown Win32 compiler
#endif

	return tmp;


#elif defined unix

	timeval tmp_tv;
	timer_get(&tmp_tv);
	return (tmp_tv.tv_sec * 1000) + (tmp_tv.tv_usec / 1000);

#else
#error unknown architecture/compiler
#endif

}

int timer_get_microseconds()
{
	if (!Timer_inited) {
		Int3();					// Make sure you call timer_init before anything that uses timer functions!
		return 0;
	}

#if defined(_WIN32)
	int tmp;
	LARGE_INTEGER temp_large;
	timer_get(&temp_large);
	temp_large.QuadPart *= (longlong)1000000;

#if defined(_MSC_VER)

	// Timing in milliseconds.
	_asm	mov edx, temp_large.HighPart
	_asm	mov eax, temp_large.LowPart

	//_asm	shld    edx, eax, 16            ; Keep 32+11 bits
	//_asm	shl     eax, 16			
	// edx:eax = number of 1.19Mhz pulses elapsed.
	_asm	mov     ebx, Timer_freq

	// Make sure we won't divide overflow.  Make time wrap at about 9 hours
sub_again:
	_asm	sub     edx, ebx	; subtract until negative...
	_asm	jns     sub_again	; ...to prevent divide overflow...
	_asm	add     edx, ebx	; ...then add in to get correct value.
	_asm	div     ebx
	//eax = milliseconds elapsed...
	_asm mov tmp, eax

#elif defined(__GNUC__)
	// Timing in milliseconds.
	asm(
		 "mov   %1,%%edx;\n"			// temp_large.HighPart
		 "mov   %2,%%eax;\n"			// temp_large.LowPart  
		 // edx:eax = number of 1.19Mhz pulses elapsed.
		 "mov   %3,%%ebx;\n"			// Timer_freq
		 // Make sure we won't divide overflow.  Make time wrap at about 9 hours
		 "sub_again3:"
		 "sub   %%ebx,%%edx;\n"		// subtract until negative...
		 "jns   sub_again3;\n"		// ...to prevent divide overflow...
		 "add   %%ebx,%%edx;\n"		// ...then add in to get correct value.
		 "div   %%ebx;\n"
		 //eax = milliseconds elapsed...
		 "mov   %%eax,%0;\n"		// tmp
		 : "=g" (tmp)
		 : "g" (temp_large.HighPart), "g" (temp_large.LowPart), "g" (Timer_freq)
		 : "edx", "eax", "ebx");
#else
#error unknown Win32 compiler
#endif

	return tmp;

#elif defined unix

	timeval tmp_tv;
	timer_get(&tmp_tv);
	return (tmp_tv.tv_sec * 1000000) + tmp_tv.tv_usec;

#else
#error unknown architecture/compiler
#endif
}

// 0 means invalid,
// 1 means always return true
// 2 and above actually check the time
int timestamp_ticker = 2;

void timestamp_reset()
{
	timestamp_ticker = 2;
}

// Restrict all time values between 0 and MAX_TIME
// so we don't have to use UINTs to calculate rollover.
// For debugging & testing, you could set this to 
// something like 1 minute (6000).
#define MAX_TIME (INT_MAX/2)

void timestamp_inc(float frametime)
{
	timestamp_ticker += (int)(frametime*TIMESTAMP_FREQUENCY);

	if ( timestamp_ticker > MAX_TIME )	{
		timestamp_ticker = 2;		// Roll!
	}

	if (timestamp_ticker < 2 ) {
		mprintf(("Whoa!!!  timestamp_ticker < 2 -- resetting to 2!!!\n"));
		timestamp_ticker = 2;
	}
}

int timestamp(int delta_ms )
{
	int t2;
	if (delta_ms < 0 ) return 0;
	if (delta_ms == 0 ) return 1;
	t2 = timestamp_ticker + delta_ms;
	if ( t2 > MAX_TIME )	{
		// wrap!!!
		t2 = delta_ms - (MAX_TIME-timestamp_ticker);
	}
	if (t2 < 2 ) t2 = 2;	// hack??
	return t2;
}

//	Returns milliseconds until timestamp will elapse.
//	Negative value gives milliseconds ago that timestamp elapsed.
int timestamp_until(int stamp)
{
	// JAS: FIX
	// HACK!! This doesn't handle rollover!
	// (Will it ever happen?)
	
	return stamp - timestamp_ticker;

/*
	uint	delta;

	delta = stamp - timestamp_ticker;
	

	if (delta > UINT_MAX/2)
		delta = UINT_MAX - delta + 1;
	else if (delta < - ( (int) (UINT_MAX/2)))
		delta = UINT_MAX + delta + 1;

	return delta;
*/
}

// alternate timestamp functions.  The way these work is you call xtimestamp() to get the
// current counter value, and then call
int timestamp()
{
	return timestamp_ticker;
}

int timestamp_has_time_elapsed(int stamp, int time)
{
	int t;

	if (time <= 0)
		return 1;

	t = stamp + time;
	if (t <= timestamp_ticker)
		return 1;  // if we are unlucky enough to have it wrap on us, this will assume time has elapsed.

	return 0;
}

// timing functions -------------------------------------------------------------------------------

#define MAX_TIMING_EVENTS		15

// timing struct
#ifdef USE_TIMING

typedef struct timing {
	char name[64];
	int microseconds_total;
	int start;
	int ref_count;
} timing;

timing Timing_frame;
timing Timing_events[MAX_TIMING_EVENTS];
int Timing_event_count = 0;

#endif

// lookup a timing event
int timing_event_lookup(char *event_name)
{
#ifndef USE_TIMING
	return -1;
#else
	int idx;

	// sanity
	if(event_name == NULL){
		return -1;
	}

	// look through all events
	for(idx=0; idx<MAX_TIMING_EVENTS; idx++){
		if(!stricmp(Timing_events[idx].name, event_name)){
			return idx;
		}
	}

	return -1;
#endif
}

// start timing frame stuff
void timing_frame_start()
{
#ifndef USE_TIMING
	return;
#else
	int idx;

	// restart the frame
	Timing_event_count = 0;
	Timing_frame.start = timer_get_microseconds();
	for(idx=0; idx<MAX_TIMING_EVENTS; idx++){
		Timing_events[idx].microseconds_total = 0;
		strcpy(Timing_events[idx].name, "");
		Timing_events[idx].ref_count = 0;
	}
#endif
}

// done timing the frame
void timing_frame_stop()
{
#ifndef USE_TIMING
	return;
#else	
	int stop_time;

	// stop time
	stop_time = timer_get_microseconds();

	// calc times
	Timing_frame.microseconds_total = stop_time - Timing_frame.start;	
#endif
}

// get the total frame time in microseconds
int timing_frame_total()
{
#ifndef USE_TIMING
	return 0;
#else
	return Timing_frame.microseconds_total;
#endif
}

// time an individual event
void timing_event_start(char *event_name)
{
#ifndef USE_TIMING
	return;
#else
	int event;

	// sanity
	if(event_name == NULL){
		return;
	}

	// try and find the event
	event = timing_event_lookup(event_name);

	// if we already have one
	if(event != -1){
		Assert(Timing_events[event].ref_count == 0);
		Timing_events[event].start = timer_get_microseconds();
		Timing_events[event].ref_count++;
	}
	// if we need to add a new one
	else {
		if(Timing_event_count < MAX_TIMING_EVENTS){
			strcpy(Timing_events[Timing_event_count].name, event_name);
			Timing_events[Timing_event_count].start = timer_get_microseconds();
			Timing_events[Timing_event_count++].ref_count++;
		}
	}
#endif
}

// stop timing an event
void timing_event_stop(char *event_name)
{
#ifndef USE_TIMING
	return;
#else
	int event;

	// sanity
	if(event_name == NULL){
		return;
	}

	// try and find the event
	event = timing_event_lookup(event_name);

	// if we already have one
	if(event != -1){
		Assert(Timing_events[event].ref_count == 1);
		Timing_events[event].microseconds_total += timer_get_microseconds() - Timing_events[event].start;
		Timing_events[event].ref_count--;
	}
#endif
}

// get the total time for an event in microseconds
int timing_event_total(char *event_name)
{
#ifndef USE_TIMING
	return -1;
#else
	int event;

	// sanity
	if(event_name == NULL){
		return -1;
	}

	// try and find the event
	event = timing_event_lookup(event_name);
	if(event == -1){
		return -1;
	}
	
	return Timing_events[event].microseconds_total;
#endif
}

// get the percentage of total frametime for the event (0.0 to 1.0)
float timing_event_pct(char *event_name)
{
#ifndef USE_TIMING
	return 0.0f;
#else
	int event;

	// sanity
	if(event_name == NULL){
		return -1.0f;
	}

	// try and find the event
	event = timing_event_lookup(event_name);
	if(event == -1){
		return -1.0f;
	}

	return (float)((double)Timing_events[event].microseconds_total / (double)Timing_frame.microseconds_total);
#endif
}

// display timing 
void timing_display(int x, int y)
{
#ifndef USE_TIMING
	return;
#else
	int idx;

	gr_set_color_fast(&Color_bright_blue);

	// total time
	gr_printf(x, y, "Total time %f\n", (float)timing_frame_total() / 1000000.0f);
	y += 10;

	// each event percentage
	for(idx=0; idx<Timing_event_count; idx++){
		gr_printf(x, y, "%s : %f\n", Timing_events[idx].name, timing_event_pct(Timing_events[idx].name));
		y += 10;
	}
#endif
}
