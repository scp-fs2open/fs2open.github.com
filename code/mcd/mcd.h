/*
 * Softpixel MemCheckDeluxe v1.2.2
 *  2003 cwright <cwright@softpixel.com>
 */

/* Copyright (C) 2002, 2003 Softpixel (http://softpixel.com/)
 * This file is covered by a BSD-Style License, available in
 * the LICENSE file in the root directory of this package, as well as at
 * http://prj.softpixel.com/licenses/#bsd
 */


#ifndef MCD_H
#define MCD_H

#pragma warning(disable:4514)
#pragma warning(disable:4291)
#include <stdio.h>	//for file redirection

#define MCD_VERSION	0x010201

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MCD_GONE

#ifdef _MCD_CHECK

#include <stdarg.h>

#ifdef WIN32
	#define __FUNCTION__ __FILE__
#endif

#ifndef MCD_QUIET
	#ifndef WIN32
	#warning - Building with memory checking.  
	#warning   expect lower performance. -
	#endif
#endif

//warn about redefinitions (sometimes these can be #defined)

#ifdef malloc
#undef malloc
	#ifndef WIN32
	#warning ------ Redefining malloc() ------
	#endif
#endif

#ifdef calloc
#undef calloc
	#ifndef WIN32
	#warning ------ Redefining calloc() ------
	#endif
#endif

#ifdef realloc
#undef realloc
	#ifndef WIN32
	#warning ------ Redefining realloc() ------
	#endif
#endif

#ifdef free
#undef free
	#ifndef WIN32
	#warning ------ Redefining free() ------
	#endif
#endif

#ifdef strdup
#undef strdup
	#ifndef WIN32
	#warning ------- Redefining strdup() --------
	#endif
#endif

#ifdef strndup
#undef strndup
	#ifndef WIN32
	#warning ------- Redefining strndup() -------
	#endif
#endif

#ifdef asprintf
#undef asprintf
	#ifndef WIN32
	#warning ------ Redefining asprintf() ------
	#endif
#endif

#ifdef vasprintf
#undef vasprintf
	#ifndef WIN32
	#warning ------ Redefining vasprintf() ------
	#endif
#endif

#ifdef scanf
#undef scanf
	#ifndef WIN32
	#warning ------ Redefining scanf() ------
	#endif
#endif

#ifdef fscanf
#undef fscanf
	#ifndef WIN32
	#warning ------ Redefining fscanf() ------
	#endif
#endif

#ifdef sscanf
#undef sscanf
	#ifndef WIN32
	#warning ------ Redefining sscanf() ------
	#endif
#endif

#ifdef getcwd
#undef getcwd
	#ifndef WIN32
	#warning ------ Redefining getcwd() ------
	#endif
#endif

#define strdup(p)		MCD_strdup(p,__FUNCTION__,__FILE__,__LINE__)
#ifndef WIN32		//no strndup in win32
#define strndup(p,n)		MCD_strndup(p,n,__FUNCTION__,__FILE__,__LINE__)
#endif
#define malloc(size)		MCD_malloc(size,__FUNCTION__,__FILE__,__LINE__)
#define calloc(n,s)		MCD_calloc(s*n,__FUNCTION__,__FILE__,__LINE__)
#define	realloc(p,s)		MCD_realloc(p,s,__FUNCTION__,__FILE__,__LINE__)

#ifdef _GNU_SOURCE
#define asprintf(p,f,args...)	MCD_asprintf(p,f,__FUNCTION__,__FILE__,__LINE__, ## args)
#define vasprintf(p,f,ap)	MCD_vasprintf(p,f,ap,__FUNCTION__,__FILE__,__LINE__)
#endif

#ifndef WIN32
/* windows doesn't like variable arguments in #define's */
#ifndef MCD_FASTFREE	//fastfree isnt supported here yet
#define scanf(f,args...)       MCD_scanf(f,__FUNCTION__,__FILE__,__LINE__, ## args)
#define fscanf(s,f,args...)    MCD_fscanf(s,f,__FUNCTION__,__FILE__,__LINE__, ## args)
#define sscanf(s,f,args...)    MCD_sscanf(s,f,__FUNCTION__,__FILE__,__LINE__, ## args) 
#define getcwd(p,s)		MCD_getcwd(p,s,__FUNCTION__,__FILE__,__LINE__)
#endif	//fastfree
#endif	//win32

#define free(p)			MCD_free(p,__FUNCTION__,__FILE__,__LINE__)

#endif	// _MCD_CHECK


/* !!! These are called by the defines only.  Do NOT use directly. !!! */
void	*MCD_malloc(int size, char*, char*, int);
void	*MCD_calloc(int size, char*, char*, int);
void	*MCD_realloc(void *p, int size, char*, char*, int);
char	*MCD_getcwd (char *p, int size, char*, char*, int);
#ifdef __GNUC__
char	*MCD_strdup (const char*s, char*, char*, int);
char	*MCD_strndup(const char*s, int n, char*, char*, int);
#else
char	*MCD_strdup (char*s, char*, char*, int);
char	*MCD_strndup(char*s, int n, char*, char*, int);
#endif
int	MCD_scanf (const char *fmt, char*fun, char*file, int line,...);
int	MCD_fscanf(FILE *stream,const char *fmt,char*fun,char*file,int line,...);
int	MCD_sscanf(const char *str,const char *fmt,char*fun,char*file,int line,...); 
// Private MCD function, no need to be in public namespace
//void	scan_args(const char*fmt,va_list argptr,char*fun,char*file,int line);
#ifdef _GNU_SOURCE
int	MCD_asprintf(char **ptr, const char*fmt,char*,char*,int,...);
int	MCD_vasprintf(char **ptr, const char*fmt,va_list argptr,char*,char*,int);
#endif	// _GNU_SOURCE

void	MCD_free(void *p,char*,char*,int);

/* --- call this for memory stats --- */
void showMemStats(void);

/* --- to send realtime stats somewhere other than stderr,
	put an opened fp in here --- */
void _MCD_RealTimeLog(FILE*);

/* --- to send showMemStats() somewhere other than stdout,
	put an opened fp in here --- */
void _MCD_MemStatLog(FILE*);

#else	//MCD is gone

/* define functions so source will compile without modification */
#define showMemStats()
#define _MCD_RealTimeLog(x)
#define _MCD_MemStatLog(x)

#endif	//_MCD_GONE

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifndef _MCD_GONE	// check again, outside of extern "C" mode
#ifdef __cplusplus	/* Some C++ new/delete operator overloading */

#ifndef WIN32
	#warning C++ Extentions Enabled
#endif

#ifdef new
	#undef new
#endif

#ifdef delete
	#undef delete
#endif

extern char	*_MCD_LastSetFile,*_MCD_LastSetFun;
extern int	_MCD_LastSetLine;

inline void setFileFunLineState(char*file,char*fun,int line)
{
	_MCD_LastSetLine=line;
	_MCD_LastSetFile=file;
	_MCD_LastSetFun=fun;
}

inline void* operator new	(unsigned int size,char *file,
char*fun,int line)
{
	return MCD_malloc(size,file,fun,line);
}

inline void* operator new[]	(unsigned int size,char*file, 
char*fun,int line)
{
	return MCD_malloc(size,file,fun,line);
}

// currently, passing args to delete operator is not working at all...
#ifndef WIN32	//win32 doesn't like default params to delete
inline void  operator delete	(void * buf,char*file=__FILE__, 
#ifdef WIN32
char*fun=__FILE__,int line=__LINE__)
#else
char*fun=__FUNCTION__,int line=__LINE__)
#endif
{
	MCD_free(buf,_MCD_LastSetFile,_MCD_LastSetFun,_MCD_LastSetLine);
}
// ...so we have these here for the day they work, which isn't today.
inline void  operator delete[]	(void * buf,char*file=__FILE__, 
#ifdef WIN32
char*fun=__FILE__,int line=__LINE__)
#else
char*fun=__FUNCTION__,int line=__LINE__)
#endif
{
	MCD_free(buf,_MCD_LastSetFile,_MCD_LastSetFun,_MCD_LastSetLine);
}
#endif	// win32 default delete params

inline void  operator delete	(void * buf)
{
	MCD_free(buf,0,0,0);
}

inline void  operator delete[]	(void * buf)
{
	MCD_free(buf,0,0,0);
}

#ifdef WIN32
#define	new	new(__FILE__,__FILE__,__LINE__)
#else
#define	new	new(__FILE__,__FUNCTION__,__LINE__)
#endif
//#define	delete setFileFunLineState(__FILE__,__FUNCTION__,__LINE__);delete
//#define delete	delete(__FILE__,__FUNCTION__,__LINE__)

#endif // __cplusplus
#endif // _MCD_GONE


#endif	// MCD_H
