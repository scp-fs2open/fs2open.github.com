
/*
 * $Logfile: $
 * $Revision: 2.26 $
 * $Date: 2006-01-13 14:19:34 $
 * $Author: taylor $
 *
 * OS-dependent functions.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.25  2005/10/27 16:23:03  taylor
 * go back to using abort() over exit() for Assert's and Int3's but close out SDL first this time, makes debugging much easier
 *
 * Revision 2.24  2005/10/17 05:48:18  taylor
 * dynamically allocate object collision pairs
 *
 * Revision 2.23  2005/10/11 08:30:38  taylor
 * fix memory freakage from dynamic spawn weapon types
 *
 * Revision 2.22  2005/09/22 11:21:22  taylor
 * delay for fox OS X, SDL messes up for some reason and though I don't like this, even Apple does it
 *
 * Revision 2.21  2005/09/14 20:38:12  taylor
 * some vm_* fixage for Windows
 *
 * Revision 2.20  2005/09/06 05:32:12  taylor
 * use exit(EXIT_FAILURE) rather than abort() so that all atexit() calls will actually get executed
 *
 * Revision 2.19  2005/09/05 09:38:19  taylor
 * merge of OSX tree
 * a lot of byte swaps were still missing, will hopefully be fully network compatible now
 *
 * Revision 2.18  2005/08/14 21:01:59  Kazan
 * I'm stupid, sorry - fixed release-build-crash
 *
 * Revision 2.17  2005/06/22 15:18:58  taylor
 * make sure that _mkdir() will return an error if the path is invalid
 *
 * Revision 2.16  2005/05/28 19:44:17  taylor
 * haven't got a clue what I was thinking with that crap
 *
 * Revision 2.15  2005/05/24 20:52:10  taylor
 * make sure that we recursively create directories for Linux/OSX
 * replace older recursive mkdir method in cfileextractor with new method
 *
 * Revision 2.14  2005/05/12 17:49:18  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.13  2005/04/21 15:52:24  taylor
 * stupid little comment fix, no big deal
 *
 * Revision 2.12  2005/04/11 05:45:38  taylor
 * _endthread() doesn't take an argument so do support one (Jens Granseuer)
 * debug variable fixes in freespace.cpp (Jens Granseuer)
 * as a safety catch we should Assert() on future usage of things we don't yet support in *nix _splitpath()
 *
 * Revision 2.11  2005/03/27 08:51:25  taylor
 * this is what coding on an empty stomach will get you
 *
 * Revision 2.10  2005/03/27 06:17:54  taylor
 * fill in some stuff for timer and mmio calls, not 100% but it works ok
 *
 * Revision 2.9  2005/03/24 23:22:59  taylor
 * make sure we will get the right error message
 *
 * Revision 2.8  2005/03/10 08:00:18  taylor
 * change min/max to MIN/MAX to fix GCC problems
 * add lab stuff to Makefile
 * build unbreakage for everything that's not MSVC++ 6
 * lots of warning fixes
 * fix OpenGL rendering problem with ship insignias
 * no Warnings() in non-debug mode for Linux (like Windows)
 * some campaign savefile fixage to stop reverting everyones data
 *
 * Revision 2.7  2005/02/04 10:12:33  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.6  2005/01/30 18:32:42  taylor
 * merge with Linux/OSX tree - p0130
 * remove extra brace in cfile.cpp
 *
 *
 * $NoKeywords: $
 */

#ifdef SCP_UNIX

#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/time.h>
#include "SDL.h"

#if defined(APPLE_APP)
#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

#include "globalincs/pstypes.h"
#include "parse/lua.h"

bool env_enabled = false;
bool cell_enabled = false;

static char buffer[200], buffer_tmp[200];


char *strnset( char* string, int fill, size_t count)
{
	char *p = string;

 	for(; *p; p++ ) {
		if( count == 0 )
			break;

 		*p = (char)fill;
		count--;
 	}

	return string;
}

// find the size of a file
int filelength(int fd)
{
	struct stat buf;

	if (fstat (fd, &buf) == -1)
		return -1;

	return buf.st_size;
}

// non-blocking process pause
void Sleep(int mili)
{
#ifdef __APPLE__
	// ewwww, I hate this!!  SDL_Delay() is causing issues for us though and this
	// basically matches Apple examples of the same thing.  Same as SDL_Delay() but
	// we aren't hitting up the system for anything during the process
	uint then = SDL_GetTicks() + mili;

	while (then > SDL_GetTicks());
#else
	SDL_Delay(mili);
#endif
}

extern void os_deinit();
// fatal assertion error
void WinAssert(char * text, char *filename, int line)
{
	fprintf(stderr, "ASSERTION FAILED: \"%s\" at %s:%d\n", text, filename, line);

	// we have to call os_deinit() before abort() so we make sure that SDL gets
	// closed out and we don't lose video/input control
	os_deinit();

	abort();
}

// standard warning message
void Warning( char * filename, int line, char * format, ... )
{
#ifndef NDEBUG
	va_list args;
	int i;
	int slen = 0;

	memset( &buffer, 0, sizeof(buffer) );
	memset( &buffer_tmp, 0, sizeof(buffer_tmp) );

	va_start(args, format);
	vsnprintf(buffer_tmp, sizeof(buffer_tmp) - 1, format, args);
	va_end(args);

	slen = strlen(buffer_tmp);

	// strip out the newline char so the output looks better
	for (i = 0; i < slen; i++){
		if (buffer_tmp[i] == (char)0x0a) {
			buffer[i] = ' ';
		} else {
			buffer[i] = buffer_tmp[i];
		}
	}

	// kill off extra white space at end
	if (buffer[slen-1] == (char)0x20) {
		buffer[slen-1] = '\0';
	} else {
		// just being careful
		buffer[slen] = '\0';
	}

	// Order UP!!
	fprintf(stderr, "WARNING: \"%s\" at %s:%d\n", buffer, filename, line);
#endif
}

// fatal error message
void Error( char * filename, int line, char * format, ... )
{
	va_list args;
	int i;
	int slen = 0;

	memset( &buffer, 0, sizeof(buffer) );
	memset( &buffer_tmp, 0, sizeof(buffer_tmp) );

	va_start(args, format);
	vsnprintf(buffer_tmp, sizeof(buffer_tmp) - 1, format, args);
	va_end(args);

#if defined(APPLE_APP)
	CFStringRef AsMessage;
	char AsText[1024];
	CFOptionFlags result;

	snprintf(AsText, 1024, "Error: %s\n\nFile: %s\nLine %d\n", buffer_tmp, filename, line);
	AsMessage = CFStringCreateWithCString(NULL, AsText, kCFStringEncodingASCII);

	CFUserNotificationDisplayAlert(0, kCFUserNotificationStopAlertLevel, NULL, NULL, NULL, CFSTR("Error!"), AsMessage, CFSTR("Exit"), NULL, NULL, &result);
#else
	slen = strlen(buffer_tmp);

	// strip out the newline char so the output looks better
	for (i = 0; i < slen; i++){
		if (buffer_tmp[i] == (char)0x0a) {
			buffer[i] = ' ';
		} else {
			buffer[i] = buffer_tmp[i];
		}
	}

	// kill off extra white space at end
	if (buffer[slen-1] == (char)0x20) {
		buffer[slen-1] = '\0';
	} else {
		// just being careful
		buffer[slen] = '\0';
	}

	// Order UP!!
	fprintf(stderr, "ERROR: \"%s\" at %s:%d\n", buffer, filename, line);
#endif

	exit(EXIT_FAILURE);
}

void LuaError(char * filename, int line, struct lua_State *L)
{
#ifdef USE_LUA
	memset( &buffer, 0, sizeof(buffer) );

	// make sure to cap to a sane string size
	snprintf( buffer, sizeof(buffer) - 1, "%s", lua_tostring(L, -1) );
	lua_pop(L, -1);

	// Order UP!!
	fprintf(stderr, "LUA ERROR: \"%s\" at %s:%d\n", buffer, filename, line);
#endif
}


HMMIO mmioOpen(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags)
{
	SDL_RWops *handle = NULL;

	char *mode = "rb";

	if (dwOpenFlags & MMIO_READ)
		mode = "rb";
	else if (dwOpenFlags & MMIO_READWRITE)
		mode = "r+b";
	else if (dwOpenFlags & MMIO_WRITE)
		mode = "wb";

	if ( szFilename != NULL ) {
		Assert( lpmmioinfo == NULL );

		handle = SDL_RWFromFile( szFilename, mode );
	} else if ( lpmmioinfo != NULL ) {
		Assert( szFilename == NULL );

		handle = SDL_RWFromMem( lpmmioinfo->pchBuffer, lpmmioinfo->cchBuffer );
	}

	return handle;
}

long mmioSeek(HMMIO hmmio, long lOffset, int iOrigin)
{
	return (long) SDL_RWseek( hmmio, lOffset, iOrigin );
}

long mmioRead(HMMIO hmmio, HPSTR pch, long cch)
{
	return (long) SDL_RWread( hmmio, pch, 1, cch );
}

MMRESULT mmioClose(HMMIO hmmio, uint wFlags)
{
	if (wFlags != 0)
		STUB_FUNCTION;

	int rc = 0;

	rc = SDL_RWclose( hmmio );

	if (rc)
		return MMIOERR_CANNOTWRITE;

	return 0;
}

// slightly different options and return than the Windows version
SDL_TimerID timeSetEvent(DWORD uDelay, uint uResolution, ptr_u lpTimeProc,  DWORD *dwUser, uint fuEvent)
{
	return SDL_AddTimer( uDelay, (SDL_NewTimerCallback)lpTimeProc, (void *) dwUser );
}

SDL_bool timeKillEvent(SDL_TimerID uTimerID)
{
	return SDL_RemoveTimer( uTimerID );
}

// get a filename minus any leading path
char *clean_filename(char *name)
{
	char *p = name + strlen(name)-1;

	// Move p to point to first letter of EXE filename
	while( (p > name) && (*p != '\\') && (*p != '/') && (*p != ':') )
		p--;

	p++;	

	return p;	
}

// high precision timer
bool QueryPerformanceCounter(LARGE_INTEGER *pcount)
{
	struct timeval timer_now;

	gettimeofday(&timer_now, NULL);

	pcount->QuadPart = (long long)timer_now.tv_usec;
	
	return 1;
}

#ifndef NDEBUG
int TotalRam = 0;
#endif

int Watch_malloc = 0;
DCF_BOOL(watch_malloc, Watch_malloc );


#ifndef NDEBUG
void windebug_memwatch_init()
{
	//_CrtSetAllocHook(MyAllocHook);
	TotalRam = 0;
}
#endif

// retrieve the current working directory
int _getcwd(char *buffer, unsigned int len)
{
	if (getcwd(buffer, len) == NULL) {
		Error(__FILE__, __LINE__, "buffer overflow in getcwd (buf size = %u)", len);
	}

	return 1;
}

// change directory to specified path
int _chdir(const char *path)
{
	int status = chdir(path);

#ifndef NDEBUG
	int m_error = errno;

	if (status) {
		Warning(__FILE__, __LINE__, "Cannot chdir to %s: %s", path, strerror(m_error));
	}
#endif

	return status;
}

// make specified directory
int _mkdir(const char *path)
{
	int status = 1;		// if we don't ever call mkdir() to update this then assume we are in error
	char *c, tmp_path[MAX_PATH];

	memset(tmp_path, 0, MAX_PATH);
	strncpy(tmp_path, path, MAX_PATH-1);

	c = &tmp_path[1];

	while (c++) {
		c = strchr(c, '/');

		if (c) {
			*c = '\0';

			status = mkdir(tmp_path, 0777);

#ifndef NDEBUG
			int m_error = errno;

			if (status && (m_error != EEXIST) ) {
				Warning(__FILE__, __LINE__, "Cannot mkdir %s: %s", tmp_path, strerror(m_error));
			}
#endif
			*c = '/';
		}
	}

	return status;
}

void _splitpath (char *path, char *drive, char *dir, char *fname, char *ext)
{
	if ( (path == NULL) || (fname == NULL) )
		return;

	// stop at these in case they ever get used, we need to support them at that point
	Assert( (dir == NULL) && (ext == NULL) );

	/* fs2 only uses fname */
	if (fname != NULL) {
		const char *ls = strrchr(path, '/');

		if (ls != NULL) {
			ls++;		// move past '/'
		} else {
			ls = path;
		}

		const char *lp = strrchr(path, '.');

		if (lp == NULL) {
			lp = ls + strlen(ls);	// move to the end
		}

		int dist = lp-ls;

		if (dist > (_MAX_FNAME-1))
			dist = _MAX_FNAME-1;

		strncpy(fname, ls, dist);
		fname[dist] = 0;	// add null, just in case
	}
}

// some type of info message
int MessageBox(HWND h, const char *s1, const char *s2, int i)
{
	if ( (h != NULL) && (i > -1) ) {
		// placeholder for some future time
	}

	fprintf(stderr, "%s: \"%s\"\n", s2, s1);

	return 0;
}


int MulDiv(int number, int numerator, int denominator)
{
	int result;

	if (denominator == 0)
		return 0;

#if defined(__i386__) && !defined(_DEBUG)
	__asm(
		"movl  %1,%%eax\n"
		"movl  %2,%%ebx\n"
		"imul  %%ebx\n"
		"movl  %3,%%ebx\n"
		"idiv  %%ebx\n"
		: "=eax" (result)
		: "m" (number), "m" (numerator), "m" (denominator)
		: "ebx","edx");
#else
	longlong tmp;
	tmp = ((longlong) number) * ((longlong) numerator);
	tmp /= (longlong) denominator;
	result = (int) tmp;
#endif

	return result;
}

// lowercase a string
void strlwr(char *s)
{
	if (s == NULL)
		return;

	while (*s) {
		*s = tolower(*s);
		s++;
	}
}

// add value to string
char *itoa(int value, char *str, int radix)
{
	Assert(radix == 10);

	sprintf(str, "%d", value);

	return str;
}



/* *************************************
 *
 * memory handling functions
 *
 * *************************************/

// RamTable stuff comes out of icculus.org
#ifndef NDEBUG
typedef struct RAM {
	ptr_u addr;
	int size;

	RAM *next;
} RAM;

static RAM *RamTable;
#endif

int vm_init(int min_heap_size)
{
#ifndef NDEBUG
	TotalRam = 0;
#endif

	return 1;
}

#ifndef NDEBUG
void *_vm_malloc( int size, char *filename, int line, int quiet )
#else
void *_vm_malloc( int size, int quiet )
#endif
{
	void *ptr = malloc( size );

	if (!ptr)	{
		if (quiet) {
			return NULL;
		}

		Error(LOCATION, "Out of memory.");
	}

#ifndef NDEBUG
	if ( Watch_malloc )	{
		mprintf(( "Malloc %d bytes [%s(%d)]\n", size, clean_filename(filename), line ));
	}

	RAM *next = (RAM *)malloc(sizeof(RAM));

	next->addr = (ptr_u)ptr;
	next->size = (size + sizeof(RAM));

	next->next = RamTable;
	RamTable = next;

	TotalRam += size;
#endif

	return ptr;
}

#ifndef NDEBUG
void *_vm_realloc( void *ptr, int size, char *filename, int line, int quiet )
#else
void *_vm_realloc( void *ptr, int size, int quiet )
#endif
{
	if (ptr == NULL)
		return vm_malloc(size);

	void *ret_ptr = realloc( ptr, size );

	if (!ret_ptr)	{
		if (quiet && (size > 0) && (ptr != NULL)) {
			// realloc doesn't touch the original ptr in the case of failure so we could still use it
			return NULL;
		}

		Error(LOCATION, "Out of memory.");
	}

#ifndef NDEBUG
	RAM *item = RamTable;

	while (item != NULL) {
		if (item->addr == (ptr_u)ret_ptr) {
			TotalRam += (size - item->size);
			item->size = size;
			break;
		}
		item = item->next;
    }
#endif

	return ret_ptr;
}

#ifndef NDEBUG
char *_vm_strdup( const char *ptr, char *filename, int line )
#else
char *_vm_strdup( const char *ptr )
#endif
{
	char *dst;
	int len = strlen(ptr);

	dst = (char *)vm_malloc( len+1 );

	if (!dst)
		return NULL;

	strcpy( dst, ptr );

	return dst;
}

#ifndef NDEBUG
char *_vm_strndup( const char *ptr, int size, char *filename, int line )
#else
char *_vm_strndup( const char *ptr, int size )
#endif
{
	char *dst;

	dst = (char *)vm_malloc( size+1 );

	if (!dst)
		return NULL;

	strncpy( dst, ptr, size );
	// make sure it has a NULL terminiator
	dst[size] = '\0';

	return dst;
}

#ifndef NDEBUG
void _vm_free( void *ptr, char *filename, int line )
#else
void _vm_free( void *ptr )
#endif
{
	if ( !ptr ) {
#ifndef NDEBUG
		mprintf(("Why are you trying to free a NULL pointer?  [%s(%d)]\n", clean_filename(filename), line));
#else
		mprintf(("Why are you trying to free a NULL pointer?\n"));
#endif
		return;
	}

#ifndef NDEBUG
	RAM *item = RamTable;
    RAM **mark = &RamTable;

	while (item != NULL) {
		if (item->addr == (ptr_u)ptr) {
			RAM *tmp = item;

			*mark = item->next;

			TotalRam -= tmp->size;

			free(tmp);

			break;
		}

		mark = &(item->next);

		item = item->next;
    }
#endif // !NDEBUG

	free(ptr);
}

void vm_free_all()
{
}

#endif // SCP_UNIX
