


#ifdef SCP_UNIX

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "SDL.h"

#if defined(APPLE_APP)
#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "globalincs/pstypes.h"
#include "parse/lua.h"

bool env_enabled = false;
bool cell_enabled = false;

int Global_warning_count = 0;
int Global_error_count = 0;

#define MAX_BUF_SIZE	1024
static char buffer[MAX_BUF_SIZE], buffer_tmp[MAX_BUF_SIZE];

#ifndef NDEBUG
#ifdef __APPLE__
#include <malloc/malloc.h>
#define MALLOC_USABLE(pointer) malloc_size(pointer)
#elif defined(SCP_SOLARIS)
#define MALLOC_USABLE(pointer) (*((size_t*)(pointer)-1))
#else
#ifdef SCP_BSD
#include <stdlib.h>
#include <malloc_np.h>
#endif
#define MALLOC_USABLE(pointer) malloc_usable_size(pointer)
#endif // __APPLE__
#endif // NDEBUG

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

	// this stuff migt be really useful for solving bug reports and user errors. We should output it!
	mprintf(("ASSERTION: \"%s\" at %s:%d\n", text, strrchr(filename, '/')+1, line ));

#ifdef Allow_NoWarn
	if (Cmdline_nowarn) {
		return;
	}
#endif

	// we have to call os_deinit() before abort() so we make sure that SDL gets
	// closed out and we don't lose video/input control
	os_deinit();

	abort();
}

// fatal assertion error
void WinAssert(char * text, char *filename, int line, const char * format, ... )
{
	// Karajorma - Nicked the code from the Warning function below
	va_list args;
	int i;
	int slen = 0;

	memset( buffer, 0, sizeof(buffer) );
	memset( buffer_tmp, 0, sizeof(buffer_tmp) );

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

	fprintf(stderr, "ASSERTION FAILED: \"%s\" at %s:%d  %s\n", text, filename, line, buffer);

	// this stuff migt be really useful for solving bug reports and user errors. We should output it!
	mprintf(("ASSERTION: \"%s\" at %s:%d  %s\n", text, strrchr(filename, '/')+1, line, buffer ));

#ifdef Allow_NoWarn
	if (Cmdline_nowarn) {
		return;
	}
#endif

	// we have to call os_deinit() before abort() so we make sure that SDL gets
	// closed out and we don't lose video/input control
	os_deinit();

	abort();
}

void WarningEx( char *filename, int line, const char *format, ... )
{
#ifndef NDEBUG
	if (Cmdline_extra_warn) {
		char msg[2 * MAX_BUF_SIZE];
		va_list args;
		va_start(args, format);
		vsprintf(msg, format, args);
		va_end(args);
		Warning(filename, line, "%s", msg);
	}
#endif
}

// standard warning message
void Warning( char * filename, int line, const char * format, ... )
{
	Global_warning_count++;

#ifndef NDEBUG
	va_list args;
	int i;
	int slen = 0;

	memset( buffer, 0, sizeof(buffer) );
	memset( buffer_tmp, 0, sizeof(buffer_tmp) );

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

	mprintf(("WARNING: \"%s\" at %s:%d\n", buffer, strrchr(filename, '/')+1, line));

	// Order UP!!
	fprintf(stderr, "WARNING: \"%s\" at %s:%d\n", buffer, filename, line);
#endif
}

//Display warning even in non-devug builds
void ReleaseWarning( char * filename, int line, const char * format, ... )
{
	Global_warning_count++;

	va_list args;
	int i;
	int slen = 0;

	memset( buffer, 0, sizeof(buffer) );
	memset( buffer_tmp, 0, sizeof(buffer_tmp) );

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

	mprintf(("WARNING: \"%s\" at %s:%d\n", buffer, strrchr(filename, '/')+1, line));

	// Order UP!!
	fprintf(stderr, "WARNING: \"%s\" at %s:%d\n", buffer, filename, line);
}

// fatal error message
void Error( const char * filename, int line, const char * format, ... )
{
	Global_error_count++;

	va_list args;
#ifndef APPLE_APP
	int i;
	int slen = 0;
#endif

	memset( buffer, 0, sizeof(buffer) );
	memset( buffer_tmp, 0, sizeof(buffer_tmp) );

	va_start(args, format);
	vsnprintf(buffer_tmp, sizeof(buffer_tmp) - 1, format, args);
	va_end(args);

	mprintf(("ERROR: %s\nFile: %s\nLine: %d\n", buffer_tmp, filename, line));

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

extern lua_Debug Ade_debug_info;
void LuaError(struct lua_State *L, const char *format, ...)
{
	va_list args;
	memset( &buffer, 0, sizeof(buffer) );

	if (format == NULL) {
		// make sure to cap to a sane string size
		snprintf( buffer, sizeof(buffer) - 1, "%s", lua_tostring(L, -1) );
		lua_pop(L, -1);
	} else {
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
		va_end(args);
	}

	// Order UP!!
	fprintf(stderr, "LUA ERROR: \"%s\"\n", buffer);
	fprintf(stderr, "\n");

	fprintf(stderr, "------------------------------------------------------------------\n");
	fprintf(stderr, "ADE Debug:\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Name:  %s\n",  Ade_debug_info.name);
	fprintf(stderr, "Name of:  %s\n",  Ade_debug_info.namewhat);
	fprintf(stderr, "Function type:  %s\n",  Ade_debug_info.what);
	fprintf(stderr, "Defined on:  %d\n",  Ade_debug_info.linedefined);
	fprintf(stderr, "Upvalues:  %d\n",  Ade_debug_info.nups);
	fprintf(stderr, "\n" );
	fprintf(stderr, "Source:  %s\n",  Ade_debug_info.source);
	fprintf(stderr, "Short source:  %s\n",  Ade_debug_info.short_src);
	fprintf(stderr, "Current line:  %d\n",  Ade_debug_info.currentline);

	fprintf(stderr, "------------------------------------------------------------------\n");
	fprintf(stderr, "LUA Stack:\n");
	fprintf(stderr, "\n");
	ade_stackdump(L, buffer);
	fprintf(stderr, "%s\n", buffer);
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
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

	pcount->QuadPart = (longlong)timer_now.tv_usec;

	return 1;
}

#ifndef NDEBUG
int TotalRam = 0;
#endif

int Watch_malloc = 0;
DCF_BOOL(watch_malloc, Watch_malloc )


#ifndef NDEBUG
void windebug_memwatch_init()
{
	//_CrtSetAllocHook(MyAllocHook);
	TotalRam = 0;
}
#endif

// retrieve the current working directory
int _getcwd(char *out_buf, unsigned int len)
{
	if (getcwd(out_buf, len) == NULL) {
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

			status = mkdir(tmp_path, 0755);

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

	longlong tmp;
	tmp = ((longlong) number) * ((longlong) numerator);
	tmp /= (longlong) denominator;
	result = (int) tmp;

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


/* *************************************
 *
 * memory handling functions
 *
 * *************************************/

// RamTable stuff replaced due to slow performance when freeing large amounts of memory

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
	size_t used_size = MALLOC_USABLE(ptr);
	if ( Watch_malloc )	{
		// mprintf now uses SCP_strings = recursion! Whee!!
		fprintf( stdout, "Malloc %zu bytes [%s(%d)]\n", used_size, clean_filename(filename), line );
	}

	TotalRam += used_size;
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

#ifndef NDEBUG
	size_t old_size = MALLOC_USABLE(ptr);
#endif

	void *ret_ptr = realloc( ptr, size );

	if (!ret_ptr)	{
		if (quiet && (size > 0) && (ptr != NULL)) {
			// realloc doesn't touch the original ptr in the case of failure so we could still use it
			return NULL;
		}

		Error(LOCATION, "Out of memory.");
	}

#ifndef NDEBUG
	size_t used_size = MALLOC_USABLE(ret_ptr);
	if ( Watch_malloc )	{
		// mprintf now uses SCP_strings = recursion! Whee!!
		fprintf( stdout, "Realloc %zu bytes [%s(%d)]\n", used_size, clean_filename(filename), line );
	}

	TotalRam += (used_size - old_size);
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
	TotalRam -= MALLOC_USABLE(ptr);
#endif // !NDEBUG

	free(ptr);
}

void vm_free_all()
{
}

#endif // SCP_UNIX
