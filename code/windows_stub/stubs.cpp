


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

#ifdef __linux__
// Hopefully both these headers are available...
#include <execinfo.h>
#include <cxxabi.h>
#endif

#include "cmdline/cmdline.h"
#include "debugconsole/console.h"
#include "globalincs/pstypes.h"
#include "parse/lua.h"

bool env_enabled = false;
bool cell_enabled = false;

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


SCP_string dump_stacktrace()
{
#ifdef __linux__
	// The following is adapted from here: https://panthema.net/2008/0901-stacktrace-demangled/
	const int ADDR_SIZE = 64;
	void *addresses[ADDR_SIZE];
	
	auto numstrings = backtrace(addresses, ADDR_SIZE);
	
	if (numstrings == 0)
	{
		return "No stacktrace available (possibly corrupt)";
	}
	
	auto symbollist = backtrace_symbols(addresses, numstrings);
	
	if (symbollist == nullptr)
	{
		return "No stacktrace available (possibly corrupt)";
	}
	
	// Demangle c++ function names to a more readable format using the ABI functions
	// TODO: Maybe add configure time checks to check if the required features are available
	SCP_stringstream stackstream;
	
	size_t funcnamesize = 256;
	char* funcname = reinterpret_cast<char*>(malloc(funcnamesize));
	
	// iterate over the returned symbol lines. skip the first, it is the
	// address of this function.
	for (int i = 1; i < numstrings; i++)
	{
		char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

		// find parentheses and +address offset surrounding the mangled name:
		// ./module(function+0x15c) [0x8048a6d]
		for (char *p = symbollist[i]; *p; ++p)
		{
			if (*p == '(')
				begin_name = p;
			else if (*p == '+')
				begin_offset = p;
			else if (*p == ')' && begin_offset) {
				end_offset = p;
				break;
			}
		}

		if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
		{
			*begin_name++ = '\0';
			*begin_offset++ = '\0';
			*end_offset = '\0';

			// mangled name is now in [begin_name, begin_offset) and caller
			// offset in [begin_offset, end_offset). now apply
			// __cxa_demangle():

			int status;
			char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
			
			if (status == 0) {
				funcname = ret; // use possibly realloc()-ed string
				stackstream << "  " << symbollist[i] << " : " << funcname << "+" << begin_offset << "\n";
			}
			else {
				// demangling failed. Output function name as a C function with
				// no arguments.
				stackstream << "  " << symbollist[i] << " : " << begin_name << "()+" << begin_offset << "\n";
			}
		}
		else
		{
			// couldn't parse the line? print the whole line.
			stackstream << "  " << symbollist[i] << "\n";
		}
	}

	free(funcname);
	free(symbollist);

	return stackstream.str();
#else
	return "No stacktrace available";
#endif
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
