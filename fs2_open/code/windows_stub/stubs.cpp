// stub routines for porting

#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#include "globalincs/pstypes.h"

void WinAssert(char * text, char *filename, int line)
{
	fprintf(stderr, "ASSERTION FAILED: \"%s\" at %s:%d\n",
			  text, filename, line);
	abort();
}



void Warning( char * filename, int line, char * format, ... )
{
	va_list args;
	va_start(args, format);
	fprintf (stderr, "WARNING: \"");
	vfprintf(stderr, format, args);
	fprintf (stderr, "\" at %s:%d\n", filename, line );
	va_end(args);
}


void Error( char * filename, int line, char * format, ... )
{
	va_list args;
	va_start(args, format);
	fprintf (stderr, "ERROR: \"");
	vfprintf(stderr, format, args);
	fprintf (stderr, "\" at %s:%d\n", filename, line );
	va_end(args);
	abort();
}


char *clean_filename(char *name)
{
	char *p = name+strlen(name)-1;
	// Move p to point to first letter of EXE filename
	while( (p > name) && (*p!='\\') && (*p!='/') && (*p!=':') )
		p--;
	p++;	

	return p;	
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


int vm_init(int min_heap_size)
{
	#ifndef NDEBUG
	TotalRam = 0;
	#endif
	return 1;
}

int _getcwd(char *buffer, unsigned int len)
{
	if (getcwd(buffer, len) == NULL) {
		Error(__FILE__, __LINE__, 
				"buffer overflow in getcwd (buf size = %u)", len);
	}
}


int _chdir(const char *path)
{
	int status = chdir(path);

	#ifndef NDEBUG
	if (status) {
		Warning(__FILE__, __LINE__,
				  "Cannot chdir to %s: %s", 
				  path, sys_errlist[errno]);
	}
	#endif

	return status;
}


int _mkdir(const char *path)
{
	int status = mkdir(path, 0777);

	#ifndef NDEBUG
	if (status) {
		Warning(__FILE__, __LINE__,
				  "Cannot mkdir %s: %s", 
				  path, sys_errlist[errno]);
	}
	#endif

	return status;
}





int MessageBox(HWND h, const char *s1, const char *s2, int i)
{
	Error(__FILE__, __LINE__,
			"MessageBox called!\n  s1 = \"%s\"\n  s2 = \"%s\"",
			s1, s2);
	return 0;
}


int MulDiv(int number, int numerator, int denominator)
{
	int result;

#if defined(__i386__)
	__asm(
		        "movl  %1,%%eax\n"
		"        movl  %2,%%ebx\n"
		"        imul  %%ebx\n"
		"        movl  %3,%%ebx\n"
		"        idiv  %%ebx\n"
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


void strlwr(char *s)
{
	if (s == NULL)
		return;
	while (*s) {
		*s = tolower(*s);
		s++;
	}
}


char *itoa(int value, char *str, int radix)
{
	Assert(radix == 10);
	sprintf(str, "%d", value);
	return str;
}




#ifdef unix
void DeleteCriticalSection(CRITICAL_SECTION *mutex)
{
	pthread_mutex_destroy(mutex);
}


void InitializeCriticalSection(CRITICAL_SECTION *mutex)
{
	pthread_mutex_init(mutex, NULL);
}


void EnterCriticalSection(CRITICAL_SECTION *mutex)
{
	pthread_mutex_lock(mutex);
}


void LeaveCriticalSection(CRITICAL_SECTION *mutex)
{
	pthread_mutex_unlock(mutex);
}



#undef malloc
#undef free
#undef strdup

#ifndef NDEBUG
void *vm_malloc( int size, char *filename, int line )
#else
void *vm_malloc( int size )
#endif
{
	#ifndef NDEBUG
	TotalRam += size;
	return malloc(size);
	#endif
 
	void *ptr = malloc(size );
	if ( ptr == NULL )	{
		Error(LOCATION, "Out of memory.");
	}

	#ifndef NDEBUG
		if ( Watch_malloc )	{
			mprintf(( "Malloc %d bytes [%s(%d)]\n", size, clean_filename(filename), line ));
		}
		TotalRam += size;
	#endif
	return ptr;
}

#ifndef NDEBUG
char *vm_strdup( const char *ptr, char *filename, int line )
#else
char *vm_strdup( const char *ptr )
#endif
{
	char *dst;
	int len = strlen(ptr);
	#ifndef NDEBUG
		dst = (char *)vm_malloc( len+1, filename, line );
	#else
		dst = (char *)vm_malloc( len+1 );
	#endif
	strcpy( dst, ptr );
	return dst;
}

#ifndef NDEBUG
void vm_free( void *ptr, char *filename, int line )
#else
void vm_free( void *ptr )
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
		#ifdef _WIN32
		  _CrtMemBlockHeader *phd = pHdr(ptr);
		  int nSize = phd->nDataSize;
		  TotalRam -= nSize;
      #else
        // mharris TODO: figure out size of block...
      #endif // ifdef WIN32
	#endif // ifndef NDEBUG

	free(ptr);
}

void vm_free_all()
{
}



#endif
