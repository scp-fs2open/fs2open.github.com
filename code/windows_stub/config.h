// config.h



#ifndef _CONFIG_H
#define _CONFIG_H


#if defined _WIN32

// Goober5000 - now these warnings will only be disabled when compiling with MSVC :)
#if defined _MSC_VER

// 4002 is too many actual parameters for macro 'Assertion'
// 4100 is unreferenced formal parameters,
// 4127 is constant conditional (assert)
// 4201 nonstandard extension used: nameless struct/union (happens a lot in Windows include headers)
// 4290 C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
// 4390 empty control statement (triggered by nprintf and mprintf's inside of one-line if's, etc)
// 4410 illegal size for operand... ie... 	fxch st(1)
// 4511 copy constructor could not be generated (happens a lot in Windows include headers)
// 4512 assignment operator could not be generated (happens a lot in Windows include headers)
// 4514 unreferenced inline function removed, 
// 4611 _setjmp warning.  Since we use setjmp alot, and we don't really use constructors or destructors, this warning doesn't really apply to us.
// 4663 C++ language change (template specification)
// 4710 is inline function not expanded (who cares?)
// 4711 tells us an inline function was expanded (who cares?)
// 4725 is the pentium division bug warning, and I can't seem to get rid of it, even with this pragma.
//      JS: I figured out the disabling 4725 works, but not on the first function in the module.
//      So to disable this, I add in a stub function at the top of each module that does nothing.
// 4786 is identifier truncated to 255 characters (happens all the time in Microsoft #includes) -- Goober5000
// 4996 deprecated strcpy, strcat, sprintf, etc. (from MSVC 2005) - taylor
#pragma warning(disable: 4002 4100 4127 4201 4290 4390 4410 4511 4512 4514 4611 4663 4710 4711 4725 4786 4996)

#endif

#if !defined BYTE_ORDER
 #define LITTLE_ENDIAN 1234
 #define BIG_ENDIAN    4321

 #if defined _M_IX86 || defined _X86_
  #define BYTE_ORDER   LITTLE_ENDIAN
 #else
  #error unknown byte order
 #endif
#endif  // BYTE_ORDER

#ifndef snprintf
#define snprintf _snprintf
#endif

#else  // ! Win32


#include <unistd.h>
#include "SDL.h"
#include "SDL_thread.h"


// don't verbose stub funtions unless we're debugging
#define STUB_FUNCTION nprintf(( "Warning", "STUB: %s in "__FILE__" at line %d, thread %d\n", __FUNCTION__, __LINE__, getpid() ))
#define DEBUGME(d1) nprintf(( "Warning", "DEBUGME: %s in "__FILE__" at line %d, msg \"%s\", thread %d\n", __FUNCTION__, __LINE__, d1, getpid() ))


#define _cdecl
#define __cdecl
#define __stdcall
#define PASCAL
#define CALLBACK
#define WINAPI
#define FAR

// Standard data types
typedef int BOOL;
typedef unsigned short WORD;
typedef unsigned int UINT;
#ifdef IAM_64BIT
// force 32-bit version of DWORD
typedef unsigned int DWORD;
typedef unsigned int FOURCC;
typedef unsigned int *PDWORD, *LPDWORD;
#else
typedef unsigned long FOURCC;
typedef unsigned long DWORD, *PDWORD, *LPDWORD;
#endif
//typedef void *HMMIO;
typedef SDL_RWops *HMMIO;
typedef void *HACMSTREAM;
typedef long LONG;
typedef long HRESULT;
typedef long HTASK;
typedef unsigned long SEGPTR;
typedef long LONG_PTR, *PLONG_PTR;
typedef long LRESULT;
typedef long LPARAM;
typedef long (CALLBACK *FARPROC16)();
typedef	unsigned int MMRESULT;
typedef void *HWND;
typedef void *HINSTANCE;
typedef void *HANDLE;
typedef char *LPSTR;
typedef char *HPSTR;
typedef void *LPMMIOPROC;
#if __WORDSIZE == 64
#define __int64 long int
#else
#define __int64 long long int	// TODO: really need a compile-time assert on all of this
#endif
#define __int32 int

typedef struct _LARGE_INTEGER {
	__int64 QuadPart;
} LARGE_INTEGER;

// networking/socket stuff
#define SOCKET			int
#define SOCKADDR		struct sockaddr
#define SOCKADDR_IN		struct sockaddr_in
#define LPSOCKADDR		struct sockaddr*
#define HOSTENT			struct hostent
#define SERVENT			struct servent
#define closesocket(x)	close(x)
#define WSAEALREADY     EALREADY
#define WSAEINVAL       EINVAL
#define WSAEWOULDBLOCK  EINPROGRESS
#define WSAEISCONN      EISCONN
#define WSAENOTSOCK     ENOTSOCK
#define WSAECONNRESET   ECONNRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAESHUTDOWN    ESHUTDOWN
#define WSAEADDRINUSE	EADDRINUSE
#define SOCKET_ERROR	(-1)
#define ioctlsocket(x, y, z)	ioctl(x, y, z)

#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET) -1)
#endif

// sound defines/structs
#define WAVE_FORMAT_PCM		1
#define WAVE_FORMAT_ADPCM		2
#define WAVE_FORMAT_IEEE_FLOAT	3

#pragma pack(1) // required to get proper values in ds_parse_wave()
typedef struct {
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD nBlockAlign;
} WAVEFORMAT;

typedef struct {
	WAVEFORMAT wf;
	WORD wBitsPerSample;
} PCMWAVEFORMAT;

typedef struct {
	WORD  wFormatTag;
	WORD  nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD  nBlockAlign;
	WORD  wBitsPerSample;
	WORD  cbSize;
} WAVEFORMATEX;
#pragma pack()

// MessageBox-Codes and stuff
#define MB_ABORTRETRYIGNORE 0
#define MB_CANCELTRYCONTINUE 0
#define MB_HELP 0
#define MB_OK 0
#define MB_OKCANCEL 0
#define MB_RETRYCANCEL 0
#define MB_YESNO 0
#define MB_YESNOCANCEL 0
#define MB_ICONEXCLAMATION 0
#define MB_ICONWARNING 0
#define MB_ICONINFORMATION 0
#define MB_ICONASTERISK 0
#define MB_ICONQUESTION 0
#define MB_ICONSTOP 0
#define MB_ICONERROR 0
#define MB_ICONHAND 0
#define MB_DEFBUTTON1 0
#define MB_DEFBUTTON2 0
#define MB_DEFBUTTON3 0
#define MB_DEFBUTTON4 0
#define MB_APPLMODAL 0
#define MB_SYSTEMMODAL 0
#define MB_TASKMODAL 0
#define MB_DEFAULT_DESKTOP_ONLY 0
#define MB_RIGHT 0
#define MB_RTLREADING 0
#define MB_SETFOREGROUND 0
#define MB_TOPMOST 0
#define MB_SERVICE_NOTIFICATION 0
#define MB_SERVICE_NOTIFICATION_NT3X 0

int MessageBox(HWND h, const char *s1, const char *s2, int i);

// thread/process related stuff
#define _beginthread(x, y, z)
#define _endthread()

typedef SDL_mutex* CRITICAL_SECTION;

// timer stuff
typedef timeval TIMEVAL;
bool QueryPerformanceCounter(LARGE_INTEGER *pcount);

// file related items
#define _MAX_FNAME					255
#define _MAX_PATH					255
#define MAX_PATH					255
#define SetCurrentDirectory(s)		_chdir(s)
#define GetCurrentDirectory(i, s)	_getcwd((s), (i))
#define _unlink(s)					unlink(s)

// mmio stuff
typedef struct { 
	DWORD		dwFlags; 
	FOURCC		fccIOProc; 
	LPMMIOPROC	pIOProc; 
	UINT		wErrorRet; 
	HTASK		hTask; 
	LONG		cchBuffer; 
	HPSTR		pchBuffer; 
	HPSTR		pchNext; 
	HPSTR		pchEndRead; 
	HPSTR		pchEndWrite; 
	LONG		lBufOffset; 
	LONG		lDiskOffset; 
	DWORD		adwInfo[4]; 
	DWORD		dwReserved1; 
	DWORD		dwReserved2; 
	HMMIO		hmmio; 
} MMIOINFO;

typedef MMIOINFO *LPMMIOINFO;

#define FOURCC_MEM	0

#define MMIO_READ		(1<<0)
#define MMIO_READWRITE	(1<<1)
#define MMIO_WRITE		(1<<2)
#define MMIO_ALLOCBUF	(1<<3)

#define MMIOERR_CANNOTWRITE		1

HMMIO mmioOpen(LPSTR szFilename, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags);
long mmioSeek(HMMIO hmmio, long lOffset, int iOrigin);
long mmioRead(HMMIO hmmio, HPSTR pch, long cch);
MMRESULT mmioClose(HMMIO hmmio, uint wFlags);


int filelength(int fd);
int _chdir(const char *path);
int _getcwd(char *buffer, unsigned int len);
int _mkdir(const char *path);
void _splitpath(char *path, char *drive, char *dir, char *fname, char *ext);

// string related
#define stricmp(s1, s2)			strcasecmp((s1), (s2))
#define strnicmp(s1, s2, n)		strncasecmp((s1), (s2), (n))
#define _strnicmp(s1, s2, n)	strncasecmp((s1), (s2), (n))
#define _strlwr(s)				strlwr(s)

void strlwr(char *s);
char *strnset( char *string, int fill, size_t count);

// other stuff
#define _isnan(f)     isnan(f)
#define _hypot(x, y)  hypot(x, y)

int MulDiv(int number, int numerator, int denominator);
void Sleep(int mili);

struct POINT {
	int x, y;
};

#endif  // if !defined (WINDOWS)

#endif // ifndef _CONFIG_H
