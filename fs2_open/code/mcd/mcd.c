/*
 * Softpixel MemCheckDeluxe v1.2.2
 *  2003 cwright <cwright@softpixel.com>
 */

/* Copyright (C) 2002, 2003 Softpixel (http://softpixel.com/)
 * This file is covered by a BSD-Style License, available in
 * the LICENSE file in the root directory of this package, as well as at
 * http://prj.softpixel.com/licenses/#bsd
 */

#ifndef _MCD_GONE	//if mcd is gone, don't do anything

#ifdef _MCD_CHECK
#undef _MCD_CHECK	//don't want trace allocations in this file =)
#endif

#include <stdio.h>
#ifndef __USE_GNU
#define __USE_GNU	// activate strndup in GNU headers
#endif

#include <string.h>
#include <malloc.h>
#include <stdarg.h>
//#include <varargs.h>	// fixes the vscanf warnings, 
			//  but kills the va_* macros...

#define MCD_C_FILE_BUILD	//flag some stuff in mcd.h
#include "mcd.h"

#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <errno.h>
#endif

#define CHUNK_MAGIC	0x1337C0DE

#ifndef MCD_FASTFREE
#define TOP_TO_BOTTOM	0x00	//search like old style
#define BOTTOM_TO_TOP	0xff	//search from LastChunk up

#define TRACK_LENGTH	5	//how many previous biases to use
#endif

/* Number of bytes to add to each allocation to check for padding */
//#define OVERFLOW_PAD	4	// unimplimented

/* all allocations are accounted for with 'chunnks' */
typedef struct ChunkS
{
#ifdef MCD_FASTFREE
	int		magic;	//help make sure the chunk is really a chunk
#endif
	int		size;	//size of allocation in bytes
	int		line;	//line it was alocated on
#ifdef WIN32
	LONGLONG	id;	// 64bit allocation ID
#else
	long long	id;	// 64bit allocation ID
#endif
	char		*function;	//creating function
	char		*file;		//file function is in
	void		*ptr;		//pointer to allocation
	struct ChunkS 	*next,		//next chunk (null if nonw)
			*prev;		//previous chunk (null if nonw)
}Chunk;
/*note: *ptr+SOC is actually the allocation.  with fastfree, ptr points to
the chunk itself, and SOC adds the offset where the actual data is */


/* MemoryChunks stores the first entry to the allocation
chunks.  Lastchunk stores the last one, to save list traversals.  */

Chunk *MemoryChunks=NULL,*LastChunk=NULL;

/* 
	smallest = smallest allocation
	largest = largest allocation
	current = amount currently allocated (for peak tracking)
*/
int smallest=0,largest=0,maximum=0,current=0;

/*
	totalAlloc = total allocations
	totalFree = total frees
	peakAlloc = maximum number of outstanging allocations
	totalOverflow = number of pointers that were overflowed
	  (unimplimented)
*/
int totalAlloc=0, totalFree=0, peakAlloc=0, totalOverflow=0;

/* Our log files (Null causes default behaviour) */
FILE *RealTimeLog=NULL;
FILE *MemStatLog=NULL;

//return realtimelog if it's set, otherwise stderr
#define RTL	(RealTimeLog?RealTimeLog:stderr)

/* SOC (SizeOf Chunk) is used in verbose mode to print correct pointers */
#ifdef MCD_FASTFREE
#define SOC	sizeof(Chunk)	//fastfree pointer offset
#else
#define SOC	0		//no fastfree implies raw pointers (no offset)
#endif

/* idbase is the id variable. each new allocation increments it.
	to keep it useful, it is defined to be as long as possible
	(64 bits) to keep it from wrapping (unless your program does
	a _lot_ of allocating :)
 */
#ifdef WIN32
LONGLONG idbase=0;
#else
long long idbase=0;
#endif

/* chunklock is our simple lock variable to keep the chunk list thread-safe */
char chunkLock=0;

/* TrackBias is used to record the weight of the previous locations 
	TrackPos keeps our current position.  each free increments 
	and wraps TrackPos.
*/
#ifndef MCD_FASTFREE
unsigned int TrackBias[TRACK_LENGTH]={0x7f};	//start in middle
int TrackPos=0;
#endif

/* Here, *p is the raw pointer.  we stick our chunk pointer there for easy lookups */
void addChunk(void*p,int size,char*fun,char*file,int line)
{
#ifdef MCD_FASTFREE
	Chunk *c=(Chunk*)p;	//extra space for chunk is accounted for
#else	//not FASTFREE
	Chunk *c=(Chunk*)malloc(sizeof(Chunk));
#endif
	while(chunkLock)
#ifdef WIN32
		Sleep(1);
#else
		usleep(1);
#endif
	++chunkLock;
	#ifdef MCD_FASTFREE
	c->magic=CHUNK_MAGIC;
	#endif
	c->size=size;
	c->line=line;
	c->function=fun;
	c->file=file;
	c->ptr=p;
	c->next=NULL;
	c->prev=NULL;
	c->id=idbase++;
	
//overflow padding checks (incomplete)
//	pad=p+size;
//	for(po=0;po<OVERFLOW_PAD;++po)
//	{
//		pad[po]=(0x7f)<<(po&1);
//		// 0x7f, 0xfe, 0x7f, 0xfe ... etc
//	}

	current+=size;
	if(current>maximum)
		maximum=current;
	if(size>largest)
		largest=size;
	if(size<smallest || smallest==0)
		smallest=size;
	++totalAlloc;
	if((totalAlloc-totalFree)>peakAlloc)
		peakAlloc=totalAlloc-totalFree;

	if(!MemoryChunks)	//first chunk
	{
		MemoryChunks=c;
		LastChunk=c;
	}
	else			//new chunk
	{
		c->prev=LastChunk;
		LastChunk->next=c;
		LastChunk=c;
	}
	--chunkLock;
	return;
}

/* here, *p is the pointer, adjusted to be the chunk*/
int delChunk(void*p/*,char*fun,char*file,int line*/)
{
	Chunk *s;
#ifndef MCD_FASTFREE
	unsigned int bias,i;
	int depth=0;
#endif
	//should be handled in MCD_free now
	//if(!p)
	//	return 0;	// null pointer.  list walker would crash

	while(chunkLock)
#ifdef WIN32
		Sleep(1);
#else
		usleep(1);
#endif
	++chunkLock;		//0wn the list

#ifdef MCD_FASTFREE

	s=(Chunk*)p;
	if(s->magic!=CHUNK_MAGIC)
	{
		/*freeing illegal pointer, or wrote out of bounds*/
		fprintf(RTL,"** Magic is [%x], which is wrong...[ptr:%x]\n",s->magic,(int)p);
		fprintf(RTL,"**  Either some buffer overflowed, or this is a bad pointer\n");
		fprintf(RTL,"**  I'm just going to free it and see what happens... might segfault.\n");
		fflush(RTL);
		/*because this is broken, we're not really sure what to do 
		with it...  so, we opt to just free it*/
		--chunkLock;
		return 1;
	}
	//pointer is to previous chunk, so use that as a basis, plus some pointer acrobatics

	if(s->prev==NULL)	//first chunk
	{
		if(MemoryChunks->next)	//next chunk gets promoted to MemoryChunks
		{
			current-=MemoryChunks->size;
			//prev=MemoryChunks;
			MemoryChunks=MemoryChunks->next;
			MemoryChunks->prev=NULL;
		}
		else	//only chunk left (empty list)
		{
			current-=MemoryChunks->size;
			MemoryChunks=NULL;
			LastChunk=NULL;
		}
	}
	else if(s->next==NULL)	//not first, is it last?
	{
		current-=s->size;
		LastChunk=s->prev;
		LastChunk->next=NULL;	//last
	}
	else	//must be in the middle somewhere...
	{

		s->prev->next=s->next;
		s->next->prev=s->prev;
		current-=s->size;
	}
	++totalFree;
#else	//not FASTFREE
	//non fastfree version.  here, we expect the chunk to be freed upon exit.

	for(bias=0,i=0;i<TRACK_LENGTH;++i)
		bias+=TrackBias[i];
	if(bias/TRACK_LENGTH<0x7f)
		bias=TOP_TO_BOTTOM;
	else
		bias=BOTTOM_TO_TOP;
	if(bias==BOTTOM_TO_TOP)
		s=LastChunk;	//bottom up!
	else
		s=MemoryChunks;	//top down!
	
	while(s)
	{
		if(depth==(totalAlloc-totalFree)/2)	//uhoh, passed mid point. adapt trend
		{
			if(bias==TOP_TO_BOTTOM)
				TrackBias[TrackPos]=BOTTOM_TO_TOP;
			else
				TrackBias[TrackPos]=TOP_TO_BOTTOM;
			++TrackPos;
			TrackPos%=TRACK_LENGTH;
		}
		++depth;
		
		if(s->ptr==p)
		{
			current-=s->size;
			
			if(s==MemoryChunks)	//first in list
			{
				MemoryChunks=MemoryChunks->next;	//set next as first
				free(s);
				if(MemoryChunks)		//if there is a next, set prev to none
					MemoryChunks->prev=NULL;
				--chunkLock;
				++totalFree;
				return 0;
			}
			else if(s==LastChunk)	//last guy...
			{
				LastChunk=LastChunk->prev;
				free(s);
				if(LastChunk)	//LastChunk could be first as well (no next), dont assume anything.
					LastChunk->next=NULL;
				--chunkLock;
				++totalFree;
				return 0;
			}
			else // somewhere in the middle...
			{
				// s should have a next and first (handled by previous cases)
				s->prev->next=s->next;
				s->next->prev=s->prev;
				free(s);
				--chunkLock;
				++totalFree;
				return 0;
			}
		}
		else
		{
			if(bias==BOTTOM_TO_TOP)
				s=s->prev;
			else
				s=s->next;
		}
	}
	//hmm, didnt find it in the list, must be foreign or error...
	#ifdef MCD_VERBOSE
	--chunkLock;
	fprintf(RTL,"** Pointer [%08X] wasn't in list...\n",(int)p);
	fprintf(RTL,"**  Either it came from an external library, or it's a bad pointer\n");
	fprintf(RTL,"**  I'm just going to free it and see.  might segfault\n");
	fflush(RTL);
	#endif
	//return 1;	//error goes unnoticed for now
#endif	//fastfree
	--chunkLock;
	return 0;
}

/* (FastFree) because we use fake pointers, we must add sizeof(Chunk) to all of 
	them for proper pointer hiding.
*/

#ifndef WIN32
inline	//inline to make it a bit snappier
#endif
void phex(FILE*f,	//print our 64bit id in a generic way
#ifdef WIN32
		LONGLONG v
#else
		long long v
#endif
)
{
	int i;
	char t[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	for(i=0;i<64;i+=4)
	{
		fprintf(f,"%c",t[(v>>(60-i))&0xf]);
	}
	fprintf(f,"]\n");
	fflush(f);
	return;
}

void *MCD_malloc(int size,char*fun,char*file,int line)
{
#ifdef MCD_FASTFREE
	void*p=malloc(size+sizeof(Chunk));
#else	//not FASTFREE
	void*p=malloc(size);
#endif
	
#ifdef MCD_VERBOSE
	fprintf(RTL,"malloc\t[%X][%i bytes]\tfrom %s:%s, line %i [id ",
			(int)p+SOC,size,file,fun,line);
	phex(RTL,idbase);
#endif
	addChunk(p,size,fun,file,line);

	return p
#ifdef MCD_FASTFREE
		+sizeof(Chunk)
#endif
		;
}


void *MCD_calloc(int size,char*fun,char*file,int line)
{
#ifdef MCD_FASTFREE
	void*p=calloc(size+sizeof(Chunk),1);
#else
	void*p=calloc(size,1);
#endif
#ifdef MCD_VERBOSE
	fprintf(RTL,"calloc\t[%X][%i bytes]\tfrom %s:%s, line %i [id ",
			(int)p+SOC,size,file,fun,line);
	phex(RTL,idbase);
#endif

	addChunk(p,size,fun,file,line);
	return p
#ifdef MCD_FASTFREE
		+sizeof(Chunk)
#endif
		;
}

/* instead of really reallocing, we just allocate a new area, and copy 
  this shouldn't matter, but there may be a better way */
void *MCD_realloc(void *p,int size,char*fun,char*file,int line)
{
#ifdef MCD_FASTFREE
	Chunk *New,*Old;

	void*n=malloc(size+sizeof(Chunk));
	New=(Chunk*)n;
	Old=(Chunk*)p-sizeof(Chunk);
	
	if(Old->size==size)	//no change??
		return p;
	
	if(size>Old->size)	//allocated more room, just copy old stuff
		memcpy(n,p-sizeof(Chunk),Old->size+sizeof(Chunk));
	else			//allocated less room (truncate)
		memcpy(n,p-sizeof(Chunk),Old->size+sizeof(Chunk));
	
	
#ifdef MCD_VERBOSE
	fprintf(RTL,"realloc\t[%X]->[%X][%i bytes]\tfrom %s:%s, line %i [id ",
			(int)p+SOC,(int)n+SOC,size,file,fun,line);
	phex(RTL,idbase);
#endif

	delChunk(p/*,fun,file,line*/);
	free(p-sizeof(Chunk));
	addChunk(n,size,fun,file,line);
	return n+sizeof(Chunk);
#else	//non-fastfree
	void*n=realloc(p,size);
#ifdef MCD_VERBOSE
	fprintf(RTL,"realloc [%X]->[%X][%ib] from %s:%s, line %i [id ",
		(int)p+SOC,(int)n+SOC,size,file,fun,line);
	phex(RTL,idbase);
#endif
	delChunk(p/*,fun,file,line*/);
	addChunk(n,size,fun,file,line);
	return n;
#endif
}

// getcwd isn't implimented in fastfree yet
#ifndef MCD_FASTFREE
#ifndef WIN32	//win32's getcwd doesn't allocate anyway
char	*MCD_getcwd(char *p,int size, char*fun,char*file,int line)
{
	int olderrno,wsize=256;
	char *tmp=NULL;

	//according to man page:
	// if p is null, size bytes are allocated
	// if p is null and size is zero, strlen(wd) bytes are allocated
	
	if(p!=NULL)	//ok, do nothing
	{
		return getcwd(p,size);
	}
	else	// ok, interesting stuff
	{
		if(!size)	//alloc as big as needed
		{
			while(tmp==NULL)
			{
				tmp=(char*)malloc(wsize);
				olderrno=errno;
				if(getcwd(tmp,0)==NULL)
				{	//too small, let's try again
					wsize*=2;
					free(tmp);
				}
				errno=olderrno;	//restore old errno, just in case
			}
			p=(char*)malloc(strlen(tmp)+1);
			memcpy(p,tmp,strlen(tmp)+1);
			free(tmp);
			addChunk(p, strlen(tmp)+1, file, fun, line);
		}
		else		//alloc size bytes
		{
			// TODO:  see if failure condition here still allocs p
			p=(char*)malloc(size);
			addChunk(p, size, file, fun, line);
			getcwd(p,size);
		}
		return p;
	}
}
#endif //win32
#endif //fastfree

/*(in fastfree) can't make this simple due to pointer stuffing :( */

#ifdef __GNUC__
char *MCD_strdup(const char*s,char*fun,char*file,int line)
#else
char *MCD_strdup(char*s,char*fun,char*file,int line)
#endif
{
	char *n;

#ifdef MCD_FASTFREE
	n=(char*)malloc(sizeof(char)*strlen(s)+1+sizeof(Chunk));
	memcpy(n+sizeof(Chunk),s,strlen(s));
#else	//non-fastfree
	n=strdup(s);	//unpointer'd version
#endif
#ifdef MCD_VERBOSE
	fprintf(RTL,"strdup\t[%X][%ib]\t\tfrom %s:%s, line %i [id ",
			(int)n+SOC,(int)strlen(s),file,fun,line);
	phex(RTL,idbase);
#endif
	addChunk(n,strlen(s)+1,fun,file,line);
	return n
#ifdef MCD_FASTFREE
		+sizeof(Chunk)
#endif
		;
}

#ifndef WIN32	//win32 lacks strndup completely
#ifdef __GNUC__
char *MCD_strndup(const char*s, int z,char*fun,char*file,int line)
#else
char *MCD_strndup(char*s, int z,char*fun,char*file,int line)
#endif
{
	char *n;
	int size;
	if((signed)strlen(s)>z)
		size=z;
	else
		size=strlen(s);
#ifdef MCD_FASTFREE
	n=(char*)malloc(sizeof(char)*size+1+sizeof(Chunk));
	memcpy(n+sizeof(Chunk),s,size);
#else	//non-fastfree
	n=(char*)strndup(s,z);	//unpointer'd version
#endif
#ifdef MCD_VERBOSE
	fprintf(RTL,"strndup\t[%X][%ib]\t\tfrom %s:%s, line %i [id ",
			(int)n+SOC,(int)size,file,fun,line);
	phex(RTL,idbase);
#endif
	addChunk(n,size+1,fun,file,line);
	return n
#ifdef MCD_FASTFREE
		+sizeof(Chunk)
#endif
		;
}

/* asprintf/vasprintf based on a patch from Stephen Lee <slee@tuxsoft.com> */
#ifdef _GNU_SOURCE

int MCD_asprintf(char **ptr,const char *fmt,char *fun, char*file, int line,...)
{
	int retval;
	va_list argptr;
#ifdef MCD_FASTFREE
	void *fc;
#endif	
	
	va_start(argptr,line);
	if((retval=vasprintf(ptr, fmt, argptr))<0)
	{
		#ifdef MCD_VERBOSE
		fprintf(RTL,"asprintf failure %s:%s, line %i [id ",
			file,fun,line);
		#endif
		return retval;
	}
	va_end(argptr);
	
#ifdef MCD_FASTFREE
	fc=malloc(sizeof(Ghunk)+retval+1);	//make string+chunk 
	memcpy(fc+sizeof(Chunk),ptr,retval);	//hack in string
	free(ptr);				//clean up string
	ptr=fc+sizeof(Chunk);			//hack pointer      
	addChunk(fc,retval+1,fun,file,line);
#else	//non-fastfree
	addChunk(*ptr, retval+1, fun, file, line);
#endif
	#ifdef MCD_VERBOSE
		fprintf(RTL,"asprintf\t[%X][%ib]\t\tfrom %s:%s, line %i [id ",
			(int)*ptr+SOC,retval+1,file,fun,line);
			phex(RTL,idbase);
	#endif
	return retval;
}

int MCD_vasprintf(char **ptr,const char *fmt,va_list argptr,char *fun, char*file, int line)
{
	int retval;
#ifdef MCD_FASTFREE
	void *fc;
#endif
	
	if((retval=vasprintf(ptr, fmt, argptr))<0)
	{
		#ifdef MCD_VERBOSE
		fprintf(RTL,"vasprintf failure %s:%s, line %i [id ",
			file,fun,line);
		#endif
		return retval;
	}
#ifdef MCD_FASTFREE
	fc=malloc(sizeof(Chunk)+retval+1);
	memcpy(fc+sizeof(Chunk),ptr,retval);
	free(ptr);
	ptr=fc+sizeof(Chunk);
	addChunk(fc,retval+1,fun,file,line);
#else	//non-fastfree
	addChunk(*ptr, retval+1, fun, file, line);
#endif
	#ifdef MCD_VERBOSE
		fprintf(RTL,"vasprintf\t[%X][%ib]\t\tfrom %s:%s, line %i [id ",
			(int)*ptr+SOC,retval+1,file,fun,line);
			phex(RTL,idbase);
	#endif
	return retval;
}

#endif	// _GNU_SOURCE
#endif	// WIN32

/* scanf family patch provided by Stephen Lee <slee@tuxsoft.com> */
void	scan_args(const char*fmt,va_list argptr,char*fun,char*file,int line);

#ifndef __cplusplus	// c++ doesn't seem to compile when vscanf etc are implicit
#ifndef WIN32		//win32 doesn't link vscanf stuff nicely
int MCD_scanf(const char *fmt,char*fun,char*file,int line,...)
{
	int retval;
	va_list argptr;

	va_start(argptr,line);
	if((retval=vscanf(fmt,argptr))<1) {
		/* no args, so let's return */
		va_end(argptr);
		return retval;
	}
	va_end(argptr);
	va_start(argptr,line);
	scan_args(fmt,argptr,fun,file,line);
	va_end(argptr);
	return retval;
}

int MCD_fscanf(FILE *stream,const char *fmt,char*fun,char*file,int line,...)
{
	int retval;
	va_list argptr;

	va_start(argptr,line);
	if((retval=vfscanf(stream,fmt,argptr))<1) {
		/* no args, so let's return */
		va_end(argptr);
		return retval;
	}
	va_end(argptr);
	va_start(argptr,line);
	scan_args(fmt,argptr,fun,file,line);
	va_end(argptr);
	return retval;
}

int MCD_sscanf(const char *str,const char *fmt,char*fun,char*file,int line,...)
{
	int retval;
	va_list argptr;

	va_start(argptr,line);
	if((retval=vsscanf(str,fmt,argptr))<1) {
		/* no args, so let's return */
		va_end(argptr);
		return retval;
	}
	va_end(argptr);
	va_start(argptr,line);
	scan_args(fmt,argptr,fun,file,line);
	va_end(argptr);
	return retval;
}
#endif //Win32 link
#endif //__cplusplus (implicit blah blah)

/* scanf etc helper function */
void scan_args(const char *fmt,va_list argptr,char*fun,char*file,int line)
{
	char **ptr;
	void *dummy;	// clear up the unused warning

	for(;*fmt;fmt++) {
		if(*fmt!='%')
			continue;
		switch(*(++fmt)) {
		case 'a': /* malloc'd string */
			ptr=(char **)va_arg(argptr,char *);
			addChunk(*ptr,strlen(*ptr)+1,fun,file,line);
			break;
		case '%':
			break;
		default: /* next arg */
			dummy=va_arg(argptr,void *);
		break;
		}
	}
	return;
}

void MCD_free(void *p,char*fun,char*file,int line)
{
	//quiet the unused argument warnings when MCD_VERBOSE isn't defined
#ifndef MCD_VERBOSE
	void *dummy=&fun;
	dummy=&file;
	dummy=(char*)&line;
#endif	//MCD_VERBOSE
	
	if(!p)
	{
#ifdef MCD_VERBOSE
	fprintf(RTL,"** Call to free with NULL argument in %s:%s, line %i\n",
			file,fun,line);
#endif	//mcd_verbose
		/* If they want native free(null), do it */
		/* in case free(null) does something bad, */
		/* rather than let them think it's ok */
		/* Must be enabled, off by default. */
		#ifdef MCD_FREE_NULL
		free(p);	//native free(null) for same behaviour
		#endif
		return;		//we do nothing
	}
#ifdef MCD_VERBOSE
	fprintf(RTL,"free\t[%X]\t\tfrom %s:%s, line %i\n",
			(int)p,file,fun,line);
#endif
	//should find id, but that's delchunk's job...

#ifdef MCD_FASTFREE
	// if delChunk returns something, it means the pointer is crazy
	if(!delChunk(p-sizeof(Chunk)/*,file,fun,line*/))
		free(p-sizeof(Chunk));
	else
	{
		fprintf(RTL,"** Bad or Foreign pointer [%08X] from %s:%s line %i\n",
			(int)p,file,fun,line);
		fflush(RTL);
		free(p);	//assume it's just a foreign pointer (could die)
	}
#else	//not fastfree

	delChunk(p);	//return value here doesn't matter, but 1 is still 'error'
			// error behaviour is simply the same (free and see)
	free(p);
#endif
	return;
}

void showMemStats()
{
	Chunk*c=MemoryChunks;
	int total=0;
	
	FILE *o;
	
	if(MemStatLog)
		o=MemStatLog;
	else
		o=stdout;
	
	fprintf(o,"Memory Stats:\n");
	while(c)
	{
		fprintf(o,"%12i bytes allocated in [%s:%s], line %7i [%08X][id ",
				c->size,c->function,c->file,c->line,(int)c->ptr+SOC);
		phex(o,c->id);
		total+=c->size;
		c=c->next;
		fflush(o);
	}	         
	fprintf(o,"\t\tTotal unfreed bytes: %12i\n",total);
	//fprintf(o,"\t\tOverflows[approx.]:  %12i\n",totalOverflow);
	fprintf(o,"\t\tPeak memory usage:   %12i\n",maximum);
	fprintf(o,"\t\tLargest allocation:  %12i\n",largest);
	fprintf(o,"\t\tSmallest allocation: %12i\n",smallest);
	fprintf(o,"\t\tTotal Allocations:   %12i\n",totalAlloc);
	fprintf(o,"\t\tTotal Frees:         %12i\n",totalFree);
	fprintf(o,"\t\tPeak Allocations:    %12i\n",peakAlloc);
	fprintf(o,"\n");
	return;
}

void _MCD_RealTimeLog(FILE*f)
{
	RealTimeLog=f;
	return;
}

void _MCD_MemStatLog(FILE*f)
{
	MemStatLog=f;
	return;
}

#ifdef __cplusplus

char	*_MCD_LastSetFile,*_MCD_LastSetFun;
int	_MCD_LastSetLine;


/*void* operator new	(unsigned int size,char *file, char*fun)
{
	return MCD_malloc(size,file,fun,5);
}

void* operator new[]	(unsigned int size,char*file, char*fun)
{
	return MCD_malloc(size,file,fun,5);
}*/


/*void* operator new (size_t size, char* file, char*fun, int line)
{
	return MCD_malloc(size, file, fun, line);
}

void* operator new[] (size_t size, char* file, char* fun, int line)
{
	return MCD_malloc(size, file, fun, line);
}

void operator delete(void* buffer, char*file, char*fun, int line)
{
	MCD_free(buffer, file, fun, line);

void operator delete[] (void* buffer, char*file, char*fun, int line)
{
	MCD_free(buffer, file, fun, line);

}*/

/*
void* operator new (size_t size)
{
	return MCD_malloc(size, __FILE__, __FUNCTION__, __LINE__);
}

void* operator new[] (size_t size)
{
	return MCD_malloc(size, __FILE__, __FUNCTION__, __LINE__);
}

void operator delete(void* buffer)
{
	MCD_free(buffer, __FILE__, __FUNCTION__, __LINE__);
}

void operator delete[](void* buffer)
{
	MCD_free(buffer, __FILE__, __FUNCTION__, __LINE__);
}*/
#endif // __cplusplus

#endif	//_MCD_GONE
