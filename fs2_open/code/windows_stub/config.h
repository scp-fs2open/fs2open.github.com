// config.h

/*
 * $Logfile: $
 * $Revision: 2.12 $
 * $Date: 2005-12-28 22:34:01 $
 * $Author: taylor $
 *
 * OS-dependent definitions.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.11  2005/12/06 03:02:28  taylor
 * move the pragma warnings here from pstypes.h for Windows
 * add indexed palette support for DDS images
 *
 * Revision 2.10  2005/07/13 03:35:31  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.9  2005/05/30 05:27:30  taylor
 * me is a moron, this broke audio streaming
 *
 * Revision 2.8  2005/04/11 05:45:38  taylor
 * _endthread() doesn't take an argument so do support one (Jens Granseuer)
 * debug variable fixes in freespace.cpp (Jens Granseuer)
 * as a safety catch we should Assert() on future usage of things we don't yet support in *nix _splitpath()
 *
 * Revision 2.7  2005/03/27 08:51:25  taylor
 * this is what coding on an empty stomach will get you
 *
 * Revision 2.6  2005/03/27 06:17:54  taylor
 * fill in some stuff for timer and mmio calls, not 100% but it works ok
 *
 * Revision 2.5  2005/01/30 18:32:42  taylor
 * merge with Linux/OSX tree - p0130
 * remove extra brace in cfile.cpp
 *
 * Revision 2.4  2004/08/11 05:06:36  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.3  2003/03/02 06:25:31  penguin
 * Tweaked for gcc
 *  - penguin
 *
 * Revision 2.2  2002/07/22 01:06:04  penguin
 * More defines for winsock compatibility
 *
 * Revision 2.1  2002/07/07 19:56:00  penguin
 * Back-port to MSVC
 *
 * $NoKeywords: $
 */

#ifndef _CONFIG_H
#define _CONFIG_H


#if defined _WIN32

// 4127 is constant conditional (assert)
// 4100 is unreferenced formal parameters,
// 4514 is unreferenced inline function removed, 
// 4201 is nameless struct extension used. (used by windows header files)
// 4410 illegal size for operand... ie... 	fxch st(1)
// 4611 is _setjmp warning.  Since we use setjmp alot, and we don't really use constructors or destructors, this warning doesn't really apply to us.
// 4725 is the pentium division bug warning, and I can't seem to get rid of it, even with this pragma.
//      JS: I figured out the disabling 4725 works, but not on the first function in the module.
//      So to disable this, I add in a stub function at the top of each module that does nothing.
// 4710 is inline function not expanded (who cares?)
// 4711 tells us an inline function was expanded (who cares?)
// 4702 unreachable code.  I care, but too many to deal with
// 4201 nonstandard extension used : nameless struct/union (happens a lot in Windows include headers)
// 4390 emptry control statement (triggered by nprintf and mprintf's inside of one-line if's, etc)
// 4996 depreciated strcpy, strcat, sprintf, etc. (from MSVC 2005) - taylor
#pragma warning(disable: 4127 4100 4514 4201 4410 4611 4725 4710 4711 4702 4201 4390 4996)


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
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef int BOOL, *PBOOL, *LPBOOL;
typedef unsigned char BYTE, *PBYTE, *LPBYTE;
typedef unsigned char UCHAR, *PUCHAR;
typedef unsigned short WORD, *PWORD, *LPWORD;
typedef unsigned short USHORT, *PUSHORT;
typedef int INT, *PINT, *LPINT;
typedef unsigned int UINT, *PUINT;
typedef long *LPLONG;
#ifdef IAM_64BIT
// force 32-bit version of DWORD
typedef unsigned int DWORD;
typedef unsigned int FOURCC;
typedef unsigned long *PDWORD, *LPDWORD;
#else
typedef unsigned long FOURCC;
typedef unsigned long DWORD, *PDWORD, *LPDWORD;
#endif
typedef unsigned long ULONG, *PULONG;
typedef float FLOAT, *PFLOAT;
typedef double DOUBLE;
typedef double DATE;
typedef unsigned char byte, *pbyte, *lpbyte;
typedef unsigned short UINT16, *PUINT16;
typedef unsigned int UINT32, *PUINT32;
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
#define __int64 long long
#define __int32 int

typedef struct _LARGE_INTEGER {
	long long QuadPart;
} LARGE_INTEGER;

// DDS format stuff ...
#define DDSD_LINEARSIZE			0x00080000
#define DDSD_PITCH				0x00000008
#define DDPF_ALPHAPIXELS		0x00000001
#define DDPF_FOURCC				0x00000004
#define DDPF_PALETTEINDEXED4	0x00000008
#define DDPF_PALETTEINDEXEDTO8	0x00000010
#define DDPF_PALETTEINDEXED8	0x00000020
#define DDPF_RGB				0x00000040

typedef struct _DDCOLORKEY {
	DWORD dwColorSpaceLowValue;
	DWORD dwColorSpaceHighValue;
} DDCOLORKEY;

typedef struct _DDPIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwRGBAlphaBitMask;
} DDPIXELFORMAT;

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
#define WSAEWOULDBLOCK  EWOULDBLOCK
#define WSAEISCONN      EISCONN
#define WSAENOTSOCK     ENOTSOCK
#define WSAECONNRESET   ECONNRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAESHUTDOWN    ESHUTDOWN
#define SOCKET_ERROR	(-1)

#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET) -1)
#endif

// sound defines/structs
#define WAVE_FORMAT_PCM		1
#define WAVE_FORMAT_ADPCM	2

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

// ummph, I need to do a better job with this
#ifdef IAM_64BIT
SDL_TimerID timeSetEvent(DWORD uDelay, uint uResolution, unsigned __int64 lpTimeProc,  DWORD *dwUser, uint fuEvent);
#else
SDL_TimerID timeSetEvent(DWORD uDelay, uint uResolution, unsigned int lpTimeProc,  DWORD *dwUser, uint fuEvent);
#endif
SDL_bool timeKillEvent(SDL_TimerID uTimerID);
#define TIME_PERIODIC	0

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
char *itoa(int value, char *str, int radix);
void Sleep(int mili);

struct POINT {
	int x, y;
};

#endif  // if !defined (WINDOWS)

#endif // ifndef _CONFIG_H
