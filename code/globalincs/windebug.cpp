/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/GlobalIncs/WinDebug.cpp $
 * $Revision: 2.33 $
 * $Date: 2006-01-19 16:00:04 $
 * $Author: wmcoolmon $
 *
 * Debug stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.32  2006/01/12 04:18:10  wmcoolmon
 * Finished committing codebase
 *
 * Revision 2.31  2005/11/13 05:25:59  phreak
 * LuaError() body should not be compiled into the code unless USE_LUA was defined in the project file.
 *
 * Revision 2.30  2005/11/08 01:03:59  wmcoolmon
 * More warnings instead of Int3s/Asserts, better Lua scripting, weapons_expl.tbl is no longer needed nor read, added "$Disarmed ImpactSnd:", fire-beam fix
 *
 * Revision 2.29  2005/10/17 05:48:18  taylor
 * dynamically allocate object collision pairs
 *
 * Revision 2.28  2005/10/11 08:30:37  taylor
 * fix memory freakage from dynamic spawn weapon types
 *
 * Revision 2.27  2005/09/15 05:19:25  taylor
 * gah, I still messed that up.  Add a NULL check and have register_malloc() actually handle the correct pointer
 *
 * Revision 2.26  2005/09/14 20:38:12  taylor
 * some vm_* fixage for Windows
 *
 * Revision 2.25  2005/08/16 20:06:24  Kazan
 * [hopefully] Fix the bug i introduced in the show memory usage code, and the convergence bug during autopilot [also saves cpu cycles - MANY of them]
 *
 * Revision 2.24  2005/08/14 23:05:27  Kazan
 * i introduced a bug in _vm_realloc.. fixed it
 *
 * Revision 2.23  2005/08/14 21:01:59  Kazan
 * I'm stupid, sorry - fixed release-build-crash
 *
 * Revision 2.22  2005/08/14 17:20:56  Kazan
 * diabled NEW_MALLOC on windows - it was causing crashing - must have been corrupting it's own heap
 *
 * Revision 2.21  2005/06/01 15:10:26  phreak
 * clarified error messages to say that the files listed on asserts/warnings/errors
 * are located on the computers that built the exe.
 *
 * Revision 2.20  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.19  2005/03/08 04:41:39  Goober5000
 * whoops
 *
 * Revision 2.18  2005/03/08 03:50:25  Goober5000
 * edited for language ;)
 * --Goober5000
 *
 * Revision 2.17  2005/03/01 06:55:40  bobboau
 * oh, hey look I've commited something :D
 * animation system, weapon models detail box alt-tab bug, probly other stuff
 *
 * Revision 2.16  2005/02/04 10:12:29  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.15  2005/01/30 09:27:41  Goober5000
 * nitpicked some boolean tests, and fixed two small bugs
 * --Goober5000
 *
 * Revision 2.14  2004/10/31 21:34:39  taylor
 * rename __ASSERT check to _ASSERT to fix constant warning message - why did no one else fix this?
 *
 * Revision 2.13  2004/07/26 20:47:31  Kazan
 * remove MCD complete
 *
 * Revision 2.12  2004/07/25 18:46:28  Kazan
 * -fred_no_warn has become -no_warn and applies to both fred and fs2
 * added new ai directive (last commit) and disabled afterburners while performing AIM_WAYPOINTS or AIM_FLY_TO_SHIP
 * fixed player ship speed bug w/ player-use-ai, now stays in formation correctly and manages speed
 * made -radar_reduce ignore itself if no parameter is given (ignoring launcher bug)
 *
 * Revision 2.11  2004/07/25 00:31:28  Kazan
 * i have absolutely nothing to say about that subject
 *
 * Revision 2.10  2004/07/12 16:32:47  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.9  2004/05/01 21:53:39  taylor
 * add some error handling to vm_strdup()
 *
 * Revision 2.8  2004/04/03 18:11:20  Kazan
 * FRED fixes
 *
 * Revision 2.7  2004/02/25 05:53:32  Goober5000
 * added DONT_SHOW_WARNINGS compile branch
 * --Goober5000
 *
 * Revision 2.6  2004/02/16 21:22:15  randomtiger
 * Use _REPORT_MEM_LEAKS compile flag in code.lib to get a report of memory leaks from malloc calls.
 *
 * Revision 2.5  2004/02/14 00:18:31  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.4  2004/01/29 01:34:01  randomtiger
 * Added malloc montoring system, use -show_mem_usage, debug exes only to get an ingame list of heap usage.
 * Also added -d3d_notmanaged flag to activate non managed D3D path, in experimental stage.
 *
 * Revision 2.3  2004/01/24 12:47:48  randomtiger
 * Font and other small changes for Fred
 *
 * Revision 2.2  2003/03/02 05:30:26  penguin
 * Added #ifdef _MSC_VER to MSVC-specific code
 *  - penguin
 *
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     12/01/98 4:46p Dave
 * Put in targa bitmap support (16 bit).
 * 
 * 3     11/30/98 5:31p Dave
 * Fixed up Fred support for software mode.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 32    5/25/98 1:39p John
 * added code to give error and abort if malloc fails.
 * 
 * 31    5/06/98 8:03p Allender
 * AL: Do early out if trying to free NULL pointer in vm_free().  Print
 * out warning message.
 * 
 * 30    4/30/98 12:02a Lawrance
 * compile out Warnings() on NDEBUG builds
 * 
 * 29    4/22/98 5:15p Lawrance
 * fix bug in strdup
 * 
 * 28    4/21/98 1:02p John
 * fixed bug where the clipboard text dumped wasn't null terminated.
 * 
 * 27    4/20/98 3:22p John
 * fixed bug that displayed wrong amount of allocated ram.
 * 
 * 26    4/17/98 3:27p Allender
 * 
 * 25    4/17/98 1:41p Allender
 * made to compile with NDEBUG defined
 * 
 * 24    4/17/98 7:00a John
 * Added in hooks for new memory allocator.  I haven't tried compiling
 * under NDEBUG, but I tried to put in code for it.
 * 
 * 23    4/01/98 9:21p John
 * Made NDEBUG, optimized build with no warnings or errors.
 * 
 * 22    3/31/98 8:17p John
 * Added code to dump memory contents
 * 
 * 21    3/31/98 11:17a Allender
 * fix bug with INTERPLAYQA Int3's and Warning dialogs
 * 
 * 20    3/30/98 4:02p John
 * Made machines with < 32 MB of RAM use every other frame of certain
 * bitmaps.   Put in code to keep track of how much RAM we've malloc'd.
 * 
 * 19    3/14/98 3:35p John
 * cleaned up call stack code.  Exited out on not properly aligned EBP's.
 * 
 * 18    3/14/98 3:25p John
 * Added code to check the parentEBP pointer for validity before
 * derefrencing.
 * 
 * 17    3/11/98 5:34p Lawrance
 * Fix typo in error dialog box
 * 
 * 16    3/06/98 2:21p John
 * Correct some ok/cancel messages.  Made call stack info prettier for
 * modules with no debugging info
 * 
 * 15    3/05/98 3:04p John
 * Made Errors, Assert, Warning info paste to clipboard.
 * 
 * 14    3/05/98 9:17a John
 * Limited stack dump depth to 16
 * 
 * 13    3/05/98 9:14a John
 * Added in a simple function name unmangler
 * 
 * 12    3/04/98 7:08p John
 * Made Freespace generate COFF files.   Made Assert,Error, and Warning
 * display the call stack.
 * 
 * 11    2/22/98 2:48p John
 * More String Externalization Classification
 *
 * $NoKeywords: $
 */

// Nothing in this module should be externalized!!!
//XSTR:OFF

//#define DUMPRAM	// This dumps all symbol sizes. See John for more info

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#include "osapi/osapi.h"
#include "globalincs/pstypes.h"
#include "cmdline/cmdline.h"
#include "parse/lua.h"

#ifdef _MSC_VER
#include <crtdbg.h>



//Uncomment SHOW_CALL_STACK to show the call stack in Asserts, Warnings, and Errors
#define SHOW_CALL_STACK
#endif // _MSC_VER


#ifndef _ASSERT
  #ifndef _DEBUG
    #define _ASSERT(expr) ((void)0)
  #else
    #define _ASSERT(expr) (assert(expr))
//    #error _ASSERT is not defined yet for debug mode with non-MSVC compilers
  #endif
#endif
				   

#ifdef SHOW_CALL_STACK

class DumpBuffer
  {
  public :
    enum { BUFFER_SIZE = 32000 } ;
    DumpBuffer() ;
    void Clear() ;
    void Printf( const char* format, ... ) ;
    void SetWindowText( HWND hWnd ) const ;
    char buffer[ BUFFER_SIZE ] ;
  private :
    char* current ;
  } ;



DumpBuffer :: DumpBuffer()
  {
  Clear() ;
  }


void DumpBuffer :: Clear()
  {
  current = buffer ;
  }


void DumpBuffer :: Printf( const char* format, ... )
  {
  // protect against obvious buffer overflow
  if( current - buffer < BUFFER_SIZE )
    {
    va_list argPtr ;
    va_start( argPtr, format ) ;
    int count = vsprintf( current, format, argPtr ) ;
    va_end( argPtr ) ;
    current += count ;
    }
  }


void DumpBuffer :: SetWindowText( HWND hWnd ) const
  {
  SendMessage( hWnd, WM_SETTEXT, 0, (LPARAM)buffer ) ;
  }





class PE_Debug
  {
  public :
    PE_Debug() ;
    ~PE_Debug() ;
    void ClearReport() ;
    int DumpDebugInfo( DumpBuffer& dumpBuffer, const BYTE* caller, HINSTANCE hInstance ) ;
    void Display() ;
  private :
    // Report data
    enum { MAX_MODULENAME_LEN = 512, VA_MAX_FILENAME_LEN = 256 } ;
    char latestModule[ MAX_MODULENAME_LEN ] ;
    char latestFile[ VA_MAX_FILENAME_LEN ] ;
    // File mapping data
    HANDLE hFile ;
    HANDLE hFileMapping ;
    PIMAGE_DOS_HEADER fileBase ;
    // Pointers to debug information
    PIMAGE_NT_HEADERS NT_Header ;
    PIMAGE_COFF_SYMBOLS_HEADER COFFDebugInfo ;
    PIMAGE_SYMBOL COFFSymbolTable ;
    int COFFSymbolCount ;
    const char* stringTable ;

    void ClearFileCache() ;
    void ClearDebugPtrs() ;
    void MapFileInMemory( const char* module ) ;
    void FindDebugInfo() ;
    void DumpSymbolInfo( DumpBuffer& dumpBuffer, DWORD relativeAddress ) ;
    void DumpLineNumber( DumpBuffer& dumpBuffer, DWORD relativeAddress ) ;
    PIMAGE_COFF_SYMBOLS_HEADER GetDebugHeader() ;
    PIMAGE_SECTION_HEADER SectionHeaderFromName( const char* name ) ;
    const char* GetSymbolName( PIMAGE_SYMBOL sym ) ;
  } ;


// Add an offset to a pointer and cast to a given type; may be
// implemented as a template function but Visual C++ has some problems.
#define BasedPtr( type, ptr, ofs ) (type)( (DWORD)(ptr) + (DWORD)(ofs) )


PE_Debug :: PE_Debug()
  {
  // Init file mapping cache
  hFileMapping = 0 ;
  hFile = INVALID_HANDLE_VALUE ;
  fileBase = 0 ;
  ClearDebugPtrs() ;
  ClearReport() ;
  }


PE_Debug :: ~PE_Debug()
  {
  ClearFileCache() ;
  }


void PE_Debug :: ClearReport()
  {
  latestModule[ 0 ] = 0 ;
  latestFile[ 0 ] = 0 ;
  }


void PE_Debug :: ClearDebugPtrs()
  {
  NT_Header = NULL ;
  COFFDebugInfo = NULL ;
  COFFSymbolTable = NULL ;
  COFFSymbolCount = 0 ;
  stringTable = NULL ;
  }


void PE_Debug :: ClearFileCache()
  {
  if( fileBase )
    {
    UnmapViewOfFile( fileBase ) ;
    fileBase = 0 ;
    }
  if( hFileMapping != 0 )
    {
    CloseHandle( hFileMapping ) ;
    hFileMapping = 0 ;
    }
  if( hFile != INVALID_HANDLE_VALUE )
    {
    CloseHandle( hFile ) ;
    hFile = INVALID_HANDLE_VALUE ;
    }
  }


void PE_Debug :: DumpLineNumber( DumpBuffer& dumpBuffer, DWORD relativeAddress )
  {
  PIMAGE_LINENUMBER line = BasedPtr( PIMAGE_LINENUMBER, COFFDebugInfo,
                                     COFFDebugInfo->LvaToFirstLinenumber ) ;
  DWORD lineCount = COFFDebugInfo->NumberOfLinenumbers ;
  const DWORD none = (DWORD)-1 ;
  DWORD maxAddr = 0 ;
  DWORD lineNum = none ;
  for( DWORD i=0; i < lineCount; i++ )
    {
    if( line->Linenumber != 0 )  // A regular line number
      {
      // look for line with bigger address <= relativeAddress
      if( line->Type.VirtualAddress <= relativeAddress &&
          line->Type.VirtualAddress > maxAddr )
        {
        maxAddr = line->Type.VirtualAddress ;
        lineNum = line->Linenumber ;
        }
      }
    line++ ;
    }
  if( lineNum != none )
    dumpBuffer.Printf( "  line %d\r\n", lineNum ) ;
//  else
//  dumpBuffer.Printf( "  line <unknown>\r\n" ) ;
  }


const char* PE_Debug :: GetSymbolName( PIMAGE_SYMBOL sym )
  {
  const int NAME_MAX_LEN = 64 ;
  static char buf[ NAME_MAX_LEN ] ;
  if( sym->N.Name.Short != 0 )
    {
    strncpy( buf, (const char*)sym->N.ShortName, 8 ) ;
    buf[ 8 ] = 0 ;
    }
  else
    {
    strncpy( buf, stringTable + sym->N.Name.Long, NAME_MAX_LEN ) ;
    buf[ NAME_MAX_LEN - 1 ] = 0 ;
    }
  return( buf ) ;
  }

void unmangle(char *dst, const char *src)
{
	//strcpy( dst, src );
	//return;

	src++;
	while( (*src) && (*src!=' ') && (*src!='@') )	{
		*dst++ = *src++;
	}
	*dst++ = 0;
}

#ifdef DUMPRAM

typedef struct MemSymbol {
	int	section;
	int	offset;
	int	size;
	char	name[132];
} MemSymbol;

int Num_symbols = 0;
int Max_symbols = 0;
MemSymbol *Symbols;

void InitSymbols()
{
	Num_symbols = 0;
	Max_symbols = 5000;
	Symbols = (MemSymbol *)vm_malloc(Max_symbols*sizeof(MemSymbol));
	if ( !Symbols )	{
		Max_symbols = 0;
	}
}

void Add_Symbol( int section, int offset, const char *name, char *module )
{
	if ( Num_symbols >= Max_symbols ) {
		return;
	}

	MemSymbol * sym = &Symbols[Num_symbols++];
	
	sym->section = section;
	sym->offset = offset;
	sym->size = -1;
	
	strcpy( sym->name, name );	
	strcat( sym->name, "(" );	
	strcat( sym->name, module );	
	strcat( sym->name, ")" );	

}

int Sym_compare( const void *arg1, const void *arg2 )
{
	MemSymbol * sym1 = (MemSymbol *)arg1;
	MemSymbol * sym2 = (MemSymbol *)arg2;

	if ( sym1->section < sym2->section )	{
		return -1;
	} else if ( sym1->section > sym2->section ) {
		return 1;
	} else {
		if ( sym1->offset > sym2->offset )	{
			return 1;
		} else {
			return -1;
		}
	}
}

int Sym_compare1( const void *arg1, const void *arg2 )
{
	MemSymbol * sym1 = (MemSymbol *)arg1;
	MemSymbol * sym2 = (MemSymbol *)arg2;

	if ( sym1->size < sym2->size )	{
		return 1;
	} else if ( sym1->size > sym2->size ) {
		return -1;
	} else {
		return 0;
	}
}

void DumpSymbols()
{
	int i;

	qsort( Symbols, Num_symbols, sizeof(MemSymbol), Sym_compare );
	
	for (i=0;i<Num_symbols; i++ )	{
		MemSymbol * sym1 = &Symbols[i];
		MemSymbol * sym2 = &Symbols[i+1];
		if ( (i<Num_symbols-1) && (sym1->section == sym2->section) )	{
			sym1->size = sym2->offset-sym1->offset;
		} else {
			sym1->size = -1;
		}
	}

	qsort( Symbols, Num_symbols, sizeof(MemSymbol), Sym_compare1 );


	FILE *fp = fopen( "dump", "wt" );

	fprintf( fp, "%-100s %10s %10s\n", "Name", "Size", "Total" );

	int total_size = 0;
	for (i=0;i<Num_symbols; i++ )	{
		MemSymbol * sym = &Symbols[i];
		if ( sym->size > 0 )
			total_size += sym->size;
		fprintf( fp, "%-100s %10d %10d\n", sym->name, sym->size, total_size );
	}

	fclose(fp);
	
	vm_free( Symbols );
	Symbols = NULL;
	_asm int 3
}
#endif

void PE_Debug::DumpSymbolInfo( DumpBuffer& dumpBuffer, DWORD relativeAddress )
{
	// Variables to keep track of function symbols
	PIMAGE_SYMBOL currentSym = COFFSymbolTable ;
	PIMAGE_SYMBOL fnSymbol = NULL ;
	DWORD maxFnAddress = 0 ;

	#ifdef DUMPRAM
	InitSymbols();
	#endif

	// Variables to keep track of file symbols
	PIMAGE_SYMBOL fileSymbol = NULL ;
	PIMAGE_SYMBOL latestFileSymbol = NULL ;
	for ( int i = 0; i < COFFSymbolCount; i++ )	{

		// Look for .text section where relativeAddress belongs to.
		// Keep track of the filename the .text section belongs to.
		if ( currentSym->StorageClass == IMAGE_SYM_CLASS_FILE )	{
			latestFileSymbol = currentSym;
		}

		// Borland uses "CODE" instead of the standard ".text" entry
		// Microsoft uses sections that only _begin_ with .text
		const char* symName = GetSymbolName( currentSym ) ;

		if ( strnicmp( symName, ".text", 5 ) == 0 || strcmpi( symName, "CODE" ) == 0 )	{
			if ( currentSym->Value <= relativeAddress )	{
				PIMAGE_AUX_SYMBOL auxSym = (PIMAGE_AUX_SYMBOL)(currentSym + 1) ;
				if ( currentSym->Value + auxSym->Section.Length >= relativeAddress )	{
					fileSymbol = latestFileSymbol ;
				}
			}
		}


		// Look for the function with biggest address <= relativeAddress
		BOOL isFunction = ISFCN( currentSym->Type ); // Type == 0x20, See WINNT.H
		if ( isFunction && ( currentSym->StorageClass == IMAGE_SYM_CLASS_EXTERNAL || currentSym->StorageClass == IMAGE_SYM_CLASS_STATIC ) )	{

			if ( currentSym->Value <= relativeAddress && currentSym->Value > maxFnAddress )	{
				maxFnAddress = currentSym->Value ;
				fnSymbol = currentSym ;
			}
		}

	#ifdef DUMPRAM
		if ( !isFunction && (currentSym->SectionNumber >= 0) )	{
			if ( (symName[0]=='_' && symName[1]!='$') || (symName[0]=='?') ) {

				char pretty_module[1024];

				if ( fileSymbol )	{
					const char* auxSym = (const char*)(latestFileSymbol + 1) ;
					char tmpFile[ VA_MAX_FILENAME_LEN ] ;
					strcpy( tmpFile, auxSym ) ;
					strcpy( pretty_module, tmpFile );
					char *p = pretty_module+strlen(pretty_module)-1;
					// Move p to point to first letter of EXE filename
					while( (*p!='\\') && (*p!='/') && (*p!=':') )
						p--;
					p++;	
					if ( strlen(p) < 1 ) {
						strcpy( pretty_module, "<unknown>" );
					} else {
						memmove( pretty_module, p, strlen(p)+1 );
					}
				} else {
					strcpy( pretty_module, "" );
				}

				Add_Symbol( currentSym->SectionNumber, currentSym->Value, symName, pretty_module );
			}
		}
	#endif

		// Advance counters, skip aux symbols
		i += currentSym->NumberOfAuxSymbols ;
		currentSym += currentSym->NumberOfAuxSymbols ;
		currentSym++ ;
	}

	#ifdef DUMPRAM
	DumpSymbols();
	#endif
	
	// dump symbolic info if found
	if ( fileSymbol )	{
		const char* auxSym = (const char*)(fileSymbol + 1) ;

		if( strcmpi( latestFile, auxSym ) )	{
			strcpy( latestFile, auxSym ) ;
			//JAS      dumpBuffer.Printf( "  file: %s\r\n", auxSym ) ;    
		}
	} else {
		latestFile[ 0 ] = 0 ;
		//JAS    dumpBuffer.Printf( "  file: unknown\r\n" ) ;    
	}
	
	if ( fnSymbol )	{
		char tmp_name[1024];
		unmangle(tmp_name, GetSymbolName( fnSymbol ) );
		dumpBuffer.Printf( "    %s()", tmp_name ) ;
	} else {
		dumpBuffer.Printf( "    <unknown>" ) ;
	}
}


PIMAGE_SECTION_HEADER PE_Debug :: SectionHeaderFromName( const char* name )
  {
  PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION( NT_Header ) ;
  for( unsigned i = 0; i < NT_Header->FileHeader.NumberOfSections; i++ )
    {
    if( strnicmp( (const char*)section->Name, name, IMAGE_SIZEOF_SHORT_NAME ) == 0 )
      return( section ) ;
    else
      section++ ;
    }
  return 0;
  }


PIMAGE_COFF_SYMBOLS_HEADER PE_Debug :: GetDebugHeader()
  {
  // Some files have a wrong entry in the COFF header, so
  // first check if the debug info exists at all
  if( NT_Header->FileHeader.PointerToSymbolTable == 0 )
    return( 0 ) ;
  DWORD debugDirRVA = NT_Header->OptionalHeader.
                      DataDirectory[ IMAGE_DIRECTORY_ENTRY_DEBUG ].
                      VirtualAddress;
  if( debugDirRVA == 0 )
    return( 0 ) ;

  // The following values must be calculated differently for MS/Borland files
  PIMAGE_DEBUG_DIRECTORY debugDir ;
  DWORD size ;

  // Borland files have the debug directory at the beginning of a .debug section
  PIMAGE_SECTION_HEADER debugHeader = SectionHeaderFromName( ".debug" ) ;
  if( debugHeader && debugHeader->VirtualAddress == debugDirRVA )
    {
    debugDir = (PIMAGE_DEBUG_DIRECTORY)(debugHeader->PointerToRawData + (DWORD)fileBase) ;
    size = NT_Header->OptionalHeader.
           DataDirectory[ IMAGE_DIRECTORY_ENTRY_DEBUG ].Size *
           sizeof( IMAGE_DEBUG_DIRECTORY ) ;
    }
  else
  // Microsoft debug directory is in the .rdata section
    {
    debugHeader = SectionHeaderFromName( ".rdata" ) ;
    if( debugHeader == 0 )
      return( 0 ) ;
    size = NT_Header->OptionalHeader.
           DataDirectory[ IMAGE_DIRECTORY_ENTRY_DEBUG ].Size ;
    DWORD offsetInto_rdata = debugDirRVA - debugHeader->VirtualAddress ;
    debugDir = BasedPtr( PIMAGE_DEBUG_DIRECTORY, fileBase,
                         debugHeader->PointerToRawData + offsetInto_rdata ) ;
    }

  // look for COFF debug info
  DWORD debugFormats = size / sizeof( IMAGE_DEBUG_DIRECTORY ) ;
  for( DWORD i = 0; i < debugFormats; i++ )
    {
    if( debugDir->Type == IMAGE_DEBUG_TYPE_COFF )
      return( (PIMAGE_COFF_SYMBOLS_HEADER)((DWORD)fileBase + debugDir->PointerToRawData) ) ;
    else
      debugDir++ ;
    }
  return( NULL ) ;
  }


void PE_Debug :: FindDebugInfo()
  {
  ClearDebugPtrs() ;
  // Put everything into a try/catch in case the file has wrong fields
  try
    {
    // Verify that fileBase is a valid pointer to a DOS header
    if( fileBase->e_magic == IMAGE_DOS_SIGNATURE )
      {
      // Get a pointer to the PE header
      NT_Header = BasedPtr( PIMAGE_NT_HEADERS, fileBase, fileBase->e_lfanew ) ;
      // Verify that NT_Header is a valid pointer to a NT header
      if( NT_Header->Signature == IMAGE_NT_SIGNATURE )
        {
        // Get a pointer to the debug header if any
        COFFDebugInfo = GetDebugHeader() ;
        // Get a pointer to the symbol table and retrieve the number of symbols
        if( NT_Header->FileHeader.PointerToSymbolTable )
          COFFSymbolTable = 
            BasedPtr( PIMAGE_SYMBOL, fileBase, NT_Header->FileHeader.PointerToSymbolTable ) ;
        COFFSymbolCount = NT_Header->FileHeader.NumberOfSymbols ;
        // The string table starts right after the symbol table
        stringTable = (const char*)(COFFSymbolTable + COFFSymbolCount) ; 
        }
      }
    }
  catch( ... )
    {
    // Header wrong, do nothing
    }
  }


void PE_Debug :: MapFileInMemory( const char* module )
  {
  ClearFileCache() ;
  hFile = CreateFile( module, GENERIC_READ, FILE_SHARE_READ, NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 ) ;
  if( hFile != INVALID_HANDLE_VALUE )
    {
    hFileMapping = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL ) ;
    if( hFileMapping != 0 )
      fileBase = (PIMAGE_DOS_HEADER)MapViewOfFile( hFileMapping, FILE_MAP_READ, 0, 0, 0 ) ;
    }
  // NB: open files/mapping are closed later in ClearFileCache
  }


int PE_Debug::DumpDebugInfo( DumpBuffer& dumpBuffer, const BYTE* caller, HINSTANCE hInstance )
{
	// Avoid to open, map and looking for debug header/symbol table
	// by caching the latest and comparing the actual module with
	// the latest one.
	static char module[ MAX_MODULENAME_LEN ] ;
	GetModuleFileName( hInstance, module, MAX_MODULENAME_LEN ) ;

	// New module
	if( strcmpi( latestModule, module ) )	{
		strcpy( latestModule, module );
		//JAS    dumpBuffer.Printf( "Module: %s\r\n", module );
		MapFileInMemory( module );
		FindDebugInfo();
	}

	char pretty_module[1024];

	strcpy( pretty_module, module );
	char *p = pretty_module+strlen(pretty_module)-1;
	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') )
		p--;
	p++;	
	if ( strlen(p) < 1 ) {
		strcpy( pretty_module, "<unknown>" );
	} else {
		memmove( pretty_module, p, strlen(p)+1 );
	}

	if ( fileBase )	{
		// Put everything into a try/catch in case the file has wrong fields
		try	{
			DWORD relativeAddress = caller - (BYTE*)hInstance ;
			// Dump symbolic information and line number if available
			if( COFFSymbolCount != 0 && COFFSymbolTable != NULL )	{
				DumpSymbolInfo( dumpBuffer, relativeAddress ) ;
				if( COFFDebugInfo )
					DumpLineNumber( dumpBuffer, relativeAddress ) ;
				return 1;
			} else {
				//dumpBuffer.Printf( "Call stack is unavailable, because there is\r\nno COFF debugging info in this module.\r\n" ) ;
				//JAS dumpBuffer.Printf( "  no debug information\r\n" ) ;
				dumpBuffer.Printf( "    %s %08x()\r\n", pretty_module, caller ) ;
				return 0;
			}
		} catch( ... )	{
			// Header wrong, do nothing
			return 0;
      }
	} else	{
		dumpBuffer.Printf( "    %s %08x()\r\n", pretty_module, caller ) ;
		//JAS dumpBuffer.Printf( "  module not accessible\r\n" ) ;
		//JAS dumpBuffer.Printf( "    address: %8X\r\n", caller ) ;
		return 0;
	}

	Int3();

}

const char* Separator = "------------------------------------------------------------------\r\n" ;


void DumpCallsStack( DumpBuffer& dumpBuffer )
{
	static PE_Debug PE_debug ;

	dumpBuffer.Printf( "\r\nCall stack:\r\n" ) ;
	dumpBuffer.Printf( Separator ) ;

	// The structure of the stack frames is the following:
	// EBP -> parent stack frame EBP
	//        return address for this call ( = caller )
	// The chain can be navigated iteratively, after the
	// initial value of EBP is loaded from the register
	DWORD parentEBP, retval;
	MEMORY_BASIC_INFORMATION mbi ;
	HINSTANCE hInstance;

	int depth = 0;

	__asm MOV parentEBP, EBP

	do	{
		depth++;
		if ( depth > 16 ) 
			break;

		if ( (parentEBP & 3) || IsBadReadPtr((DWORD*)parentEBP, sizeof(DWORD)) )	{
			break;
		}
		parentEBP = *(DWORD*)parentEBP ;

		BYTE **NextCaller = ((BYTE**)parentEBP + 1);

		if (IsBadReadPtr(NextCaller, sizeof(BYTE *)))	{
			break;
		}

		BYTE* caller = *NextCaller;		// Error sometimes!!!

		// Skip the first EBP as it points to AssertionFailed, which is
		// uninteresting for the user

		if ( depth > 1 )	{

			// Get the instance handle of the module where caller belongs to
			retval = VirtualQuery( caller, &mbi, sizeof( mbi ) ) ;

			// The instance handle is equal to the allocation base in Win32
			hInstance = (HINSTANCE)mbi.AllocationBase ;

			if( ( retval == sizeof( mbi ) ) && hInstance )	{
				if ( !PE_debug.DumpDebugInfo( dumpBuffer, caller, hInstance ) )	{
					//break;
				}
			} else {
				break ; // End of the call chain
			}
		}
	}  while( TRUE ) ;


	dumpBuffer.Printf( Separator ) ;
	PE_debug.ClearReport() ;  // Prepare for future calls
}


// This ought to be local to VerboseAssert, but it
// causes problems in Visual C++ (in the CRTL init phase)
static DumpBuffer dumpBuffer ;

#endif	//SHOW_CALL_STACK


char AssertText1[1024];
char AssertText2[1024];

uint flags = MB_SYSTEMMODAL|MB_SETFOREGROUND;
//uint flags = MB_SYSTEMMODAL;

extern void gr_force_windowed();

void dump_text_to_clipboard(char *text)
{
	int len = strlen(text)+1024;

	HGLOBAL h_text = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, len );
	if ( !h_text ) return;
	char *ptr = (char *)GlobalLock(h_text);
	if ( !ptr ) return;

	// copy then, if you find any \n's without \r's, then add in the \r.
	char last_char = 0;
	while( *text )	{
		if ( (*text == '\n') && (last_char != '\r') )	{
			*ptr++ = '\r';
		}
		last_char = *text;
		*ptr++ = last_char;
		text++;
	}
	*ptr++ = 0;
	GlobalUnlock(h_text);
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, h_text);
	CloseClipboard();
}


void _cdecl WinAssert(char * text, char * filename, int linenum )
{
	if (CmdLine_NoWarn)
		return;

	int val;

	gr_force_windowed();


	sprintf( AssertText1, "Assert: %s\r\nFile: %s\r\nLine: %d\r\n[This filename points to the location of a file on the computer that built this executable]", text, filename, linenum );

	#ifdef SHOW_CALL_STACK
		dumpBuffer.Clear();
		dumpBuffer.Printf( AssertText1 );
		dumpBuffer.Printf( "\r\n" );
		DumpCallsStack( dumpBuffer ) ;  
		dump_text_to_clipboard(dumpBuffer.buffer);

		dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
		dumpBuffer.Printf( "\r\n\r\nUse Ok to break into Debugger, Cancel to exit.\r\n");

	stay_minimized = true;
		val = MessageBox(NULL, dumpBuffer.buffer, "Assertion Failed!", MB_OKCANCEL|flags );
	#else
		val = MessageBox(NULL, AssertText1, "Assertion Failed!", MB_OKCANCEL|flags );
	#endif
	stay_minimized = false;

	ShowCursor(false);

	if (val == IDCANCEL)
		exit(1);
#ifndef FRED
#ifndef INTERPLAYQA
	Int3();
#else
	AsmInt3();
#endif
#endif

} 

int LuaError(struct lua_State *L)
{
#ifdef USE_LUA
	int val;

	gr_force_windowed();
	

	/*
	va_start(args, format);
	vsprintf(AssertText1,format,args);
	va_end(args);
	*/
	
	//sprintf(AssertText2,"LuaError: %s\r\nFile:%s\r\nLine: %d\r\n[This filename points to the location of a file on the computer that built this executable]", AssertText1, filename, line );

	dumpBuffer.Clear();
	dumpBuffer.Printf("LUA ERROR: %s", lua_tostring(L, -1));
	lua_pop(L, -1);

	dumpBuffer.Printf( "\r\n" );
	dumpBuffer.Printf(Separator);

	lua_Debug ar;
	if(lua_getstack(L, 0, &ar))
	{
		lua_getinfo(L, "nSlu", &ar);
		dumpBuffer.Printf( "Name:\t\t%s\r\n",  ar.name);
		dumpBuffer.Printf( "Name of:\t%s\r\n",  ar.namewhat);
		dumpBuffer.Printf( "Function type:\t%s\r\n",  ar.what);
		dumpBuffer.Printf( "Defined on:\t%d\r\n",  ar.linedefined);
		dumpBuffer.Printf( "Upvalues:\t%d\r\n",  ar.nups);
		dumpBuffer.Printf( "\r\n" );
		dumpBuffer.Printf( "Source:\t\t%s\r\n",  ar.source);
		dumpBuffer.Printf( "Short source:\t%s\r\n",  ar.short_src);
		dumpBuffer.Printf( "Current line:\t%d\r\n",  ar.currentline);
	}
	else
	{
		dumpBuffer.Printf("(No stack debug info)\r\n");
	}

	dumpBuffer.Printf(Separator);

	AssertText2[0] = '\0';
	dumpBuffer.Printf("\r\nLUA Stack:");
	dumpBuffer.Printf( "\r\n" );
	dumpBuffer.Printf(Separator);
	lua_stackdump(L, AssertText2);
	dumpBuffer.Printf( AssertText2 );
	dumpBuffer.Printf(Separator);

	dump_text_to_clipboard(dumpBuffer.buffer);

	dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
	dumpBuffer.Printf( "\r\n\r\nUse Yes to break into Debugger, No to continue.\r\nand Cancel to Quit");

	stay_minimized = true;
	val = MessageBox(NULL, dumpBuffer.buffer, "Error!", flags|MB_YESNOCANCEL );
	stay_minimized = false;

	ShowCursor(false);

	if (val == IDCANCEL ) {
		exit(1);
	} else if(val == IDYES) {
		Int3();
	}

#endif
	//WMC - According to documentation, this will always be the error
	//if error handler is called
	return LUA_ERRRUN;
}

void _cdecl Error( char * filename, int line, char * format, ... )
{
	int val;
	va_list args;

//	gr_activate(0);
	gr_force_windowed();

	va_start(args, format);
	vsprintf(AssertText1,format,args);
	va_end(args);
	sprintf(AssertText2,"Error: %s\r\nFile:%s\r\nLine: %d\r\n[This filename points to the location of a file on the computer that built this executable]", AssertText1, filename, line );

	#ifdef SHOW_CALL_STACK
		dumpBuffer.Clear();
		dumpBuffer.Printf( AssertText2 );
		dumpBuffer.Printf( "\r\n" );
		DumpCallsStack( dumpBuffer ) ;  
		dump_text_to_clipboard(dumpBuffer.buffer);

		dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
		dumpBuffer.Printf( "\r\n\r\nUse Ok to break into Debugger, Cancel exits.\r\n");

	stay_minimized = true;
		val = MessageBox(NULL, dumpBuffer.buffer, "Error!", flags|MB_OKCANCEL );
	#else
		strcat(AssertText2,"\r\n\r\nUse Ok to break into Debugger, Cancel exits.\r\n");
		val = MessageBox(NULL, AssertText2, "Error!", flags|MB_OKCANCEL );
	#endif
	stay_minimized = false;

	ShowCursor(false);

	if (val == IDCANCEL ) {
		exit(1);
	} else {
#ifndef INTERPLAYQA
		Int3();
#else
		AsmInt3();
#endif
	}
}

void _cdecl Warning( char * filename, int line, char * format, ... )
{
#ifdef FRED

	if (!CmdLine_NoWarn)
	{
		va_list args;
		static bool show_warnings = true;

		if(show_warnings == false) return;

		char *explanation = 
			"This warning system is new to release Fred2_open. The following issue will not prevent "
			"your mission from loading in FS2 but it could cause instablility (i.e. crashes), please fix it.";

		char *end = "Continue with warnings?";//\n\nYes: continue with warnings\nNo: continue without\nCancel: shutdown Fred";

		va_start(args, format);
		vsprintf(AssertText1,format,args);
		va_end(args);
		sprintf(AssertText2,"%s\n\nWarning: %s\r\nFile:%s\r\nLine: %d\n\n%s", 
			explanation, AssertText1, filename, line, end);

	stay_minimized = true;
		int result = MessageBox((HWND) os_get_window(), AssertText2, "Fred Warning", MB_ICONWARNING | MB_YESNO);//CANCEL);
	stay_minimized = false;

	ShowCursor(false);
		switch(result)
		{
			case IDNO: show_warnings = false; break;
			case IDCANCEL: exit(1);
		}
	}
#else

#ifndef NDEBUG
#ifndef DONT_SHOW_WARNINGS	// Goober5000

	if (CmdLine_NoWarn)
		return;

	va_list args;
	int id;

	gr_force_windowed();

	gr_activate(0);

	va_start(args, format);
	vsprintf(AssertText1,format,args);
	va_end(args);
	sprintf(AssertText2,"Warning: %s\r\nFile:%s\r\nLine: %d\r\n[This filename points to the location of a file on the computer that built this executable]", AssertText1, filename, line );

	#ifdef SHOW_CALL_STACK
		dumpBuffer.Clear();
		dumpBuffer.Printf( AssertText2 );
		dumpBuffer.Printf( "\r\n" );
		DumpCallsStack( dumpBuffer ) ;  
		dump_text_to_clipboard(dumpBuffer.buffer);

		dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
		dumpBuffer.Printf("\r\n\r\nUse Yes to break into Debugger, No to continue.\r\nand Cancel to Quit");

	stay_minimized = true;
		id = MessageBox(NULL, dumpBuffer.buffer, "Warning!", MB_YESNOCANCEL|flags );
	#else
		strcat(AssertText2,"\r\n\r\nUse Yes to break into Debugger, No to continue.\r\nand Cancel to Quit");
		id = MessageBox(NULL, AssertText2, "Warning!", MB_YESNOCANCEL|flags );
	#endif
	stay_minimized = false;
	ShowCursor(false);
	if ( id == IDCANCEL )
		exit(1);
	else if ( id == IDYES ) {
#ifndef INTERPLAYQA
		Int3();
#else
		AsmInt3();
#endif
	}
	gr_activate(1);

#endif // DONT_SHOW_WARNINGS - Goober5000
#endif // NDEBUG

#endif // FRED OGL else
}



//================= memory stuff
/*
char *format_mem( DWORD num )
{
	if ( num < 1024 )	{
		sprintf( tmp_mem, "%d bytes", num );
	} else if ( num < 1024*1024 )	{
		sprintf( tmp_mem, "%.3f KB", (float)num/1024.0f );
	} else 	{
		sprintf( tmp_mem, "%.3f MB", (float)(num/1024)/(1024.0f)  );
	}
	return tmp_mem;	
}
*/

/*
void getmem()
{
	DWORD retval;
	MEMORY_BASIC_INFORMATION mbi ;
	HINSTANCE hInstance;

	MEMORYSTATUS ms;

	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	
	printf( "Percent of memory in use: %d%%\n", ms.dwMemoryLoad );
	printf( "Bytes of physical memory:	%s\n", format_mem(ms.dwTotalPhys) );     
	printf( "Free physical memory bytes:	%s\n", format_mem(ms.dwAvailPhys) );     //  
	printf( "Bytes of paging file:	%s\n", format_mem(ms.dwTotalPageFile) ); // bytes of paging file 
	printf( "Free bytes of paging file:	%s\n", format_mem(ms.dwAvailPageFile) ); // free bytes of paging file 
	printf( "User bytes of address space:	%s\n", format_mem(ms.dwTotalVirtual) );  //  
	printf( "Free user bytes:	%s\n", format_mem(ms.dwAvailVirtual) );  // free user bytes 


	// Get the instance handle of the module where caller belongs to
	retval = VirtualQuery( getmem, &mbi, sizeof( mbi ) ) ;

	// The instance handle is equal to the allocation base in Win32
	hInstance = (HINSTANCE)mbi.AllocationBase ;

	if( ( retval == sizeof( mbi ) ) && hInstance )	{
		printf( "Virtual Query succeeded...\n" );
	} else {
		printf( "Virtual Query failed...\n" );
	}

}
*/



#ifndef NDEBUG

int TotalRam = 0;
#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
        struct _CrtMemBlockHeader * pBlockHeaderNext;
        struct _CrtMemBlockHeader * pBlockHeaderPrev;
        char *                      szFileName;
        int                         nLine;
        size_t                      nDataSize;
        int                         nBlockUse;
        long                        lRequest;
        unsigned char               gap[nNoMansLandSize];
        /* followed by:
         *  unsigned char           data[nDataSize];
         *  unsigned char           anotherGap[nNoMansLandSize];
         */
} _CrtMemBlockHeader;

#define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)


// this block of code is never referenced...
#if 0
int __cdecl MyAllocHook(
   int      nAllocType,
   void   * pvData,
   size_t   nSize,
   int      nBlockUse,
   long     lRequest,
   const char * szFileName,
   int      nLine
   )
{
   // char *operation[] = { "", "allocating", "re-allocating", "freeing" };
   // char *blockType[] = { "Free", "Normal", "CRT", "Ignore", "Client" };

   if ( nBlockUse == _CRT_BLOCK )   // Ignore internal C runtime library allocations
      return( TRUE );

   _ASSERT( ( nAllocType > 0 ) && ( nAllocType < 4 ) );
   _ASSERT( ( nBlockUse >= 0 ) && ( nBlockUse < 5 ) );

	if ( nAllocType == 3 )	{
		_CrtMemBlockHeader *phd = pHdr(pvData);
		nSize = phd->nDataSize;
	} 

//	mprintf(( "Total RAM = %d\n", TotalRam ));

   mprintf(( "Memory operation in %s, line %d: %s a %d-byte '%s' block (# %ld)\n",
            szFileName, nLine, operation[nAllocType], nSize, 
            blockType[nBlockUse], lRequest ));
   if ( pvData != NULL )
      mprintf(( " at %X", pvData ));

	mprintf(("\n" ));


   return( TRUE );         // Allow the memory operation to proceed
}
#endif  // #if 0


 
void windebug_memwatch_init()
{
	//_CrtSetAllocHook(MyAllocHook);
	TotalRam = 0;
}

#endif


//**************************************
// WARNING - ENABLE THIS FEATURE AT YOUR 
// OWN RISK - IT CAUSES GAURANTEED CRASHING
// Warned by: Kazan
// Featured Implemented by: Unknown
//#define NEW_MALLOC
//**************************************

int Watch_malloc = 0;
DCF_BOOL(watch_malloc, Watch_malloc );

HANDLE Heap = 0;

#define HEAP_FLAG HEAP_NO_SERIALIZE
// #define HEAP_FLAG	HEAP_GENERATE_EXCEPTIONS

// Returns 0 if not enough RAM.
int vm_init(int min_heap_size)
{
	#ifndef NDEBUG
	TotalRam = 0;
	#endif

	#ifdef NEW_MALLOC
		Heap = HeapCreate( HEAP_FLAG, min_heap_size, 0 );
		if ( Heap == NULL )	{
			return 0;
		}
	#endif
	return 1;
}

char *clean_filename(char *name)
{
	char *p = name+strlen(name)-1;
	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') )
		p--;
	p++;	

	return p;	
}

#ifdef _DEBUG

#ifdef _REPORT_MEM_LEAKS
const int MAX_MEM_POINTERS = 50000;

typedef struct 
{
	char  filename[33];
	int   size;
	int   line;
	void *ptr;
} MemPtrInfo;

MemPtrInfo mem_ptr_list[MAX_MEM_POINTERS];

#endif

const int MAX_MEM_MODULES  = 600;

typedef struct
{
	char filename[33];
	int  size;
	int  magic_num1;
	int  magic_num2;
	bool in_use;

} MemBlockInfo;

MemBlockInfo mem_block_list[MAX_MEM_MODULES];

int memblockinfo_sort_compare( const void *arg1, const void *arg2 )
{
	MemBlockInfo *mbi1 = (MemBlockInfo *) arg1;
	MemBlockInfo *mbi2 = (MemBlockInfo *) arg2;

	if (mbi1->size > mbi2->size)
		return -1;
		
	if (mbi1->size < mbi2->size)
		return 1;
		
	return 0; 
}

void memblockinfo_sort()
{
	qsort(mem_block_list, MAX_MEM_MODULES, sizeof(MemBlockInfo), memblockinfo_sort_compare );
}

void memblockinfo_sort_get_entry(int index, char *filename, int *size)
{
	Assert(index < MAX_MEM_MODULES);

	strcpy(filename, mem_block_list[index].filename);
	*size = mem_block_list[index].size;
}

static bool first_time = true;

void register_malloc( int size, char *filename, int line, void *ptr)
{
	if(first_time == true)
	{
		ZeroMemory(mem_block_list, MAX_MEM_MODULES * sizeof(MemBlockInfo) );
		first_time = false;

		// Get current flag
		int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
		
		// Turn on leak-checking bit
		tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
		
		// Set flag to the new value
		_CrtSetDbgFlag( tmpFlag );

#ifdef _REPORT_MEM_LEAKS
		ZeroMemory(mem_ptr_list, MAX_MEM_POINTERS * sizeof(MemPtrInfo)); 
#endif
	}

	char *temp = strrchr(filename, '\\');

	if(temp)
		filename = temp + 1;

	// calculate magic numbers
	int magic1, magic2, len = strlen(filename);

	magic1 = magic2 = 0;

	for(int c = 0; c < len; c++)
	{
		magic1 += filename[c];

		if(c % 2)
			magic2 += filename[c];
		else
			magic2 -= filename[c];
	}

	for(int i = 0; i < MAX_MEM_MODULES; i++)
	{
		// Found the first empty entry, fill it
		if(mem_block_list[i].in_use == false)
		{
			strcpy(mem_block_list[i].filename, filename);
			mem_block_list[i].size = size;
			mem_block_list[i].magic_num1 = magic1;
			mem_block_list[i].magic_num2 = magic2;
			mem_block_list[i].in_use     = true;
			break;
		}

		// Found a matching entry, update it
		if(	mem_block_list[i].magic_num1 == magic1 &&
			mem_block_list[i].magic_num2 == magic2 &&
			stricmp(mem_block_list[i].filename, filename) == 0)
		{
			mem_block_list[i].size += size;
			break;
		}
	}

	// Now if system is compiled register it with the fuller system
	#ifdef _REPORT_MEM_LEAKS

	// Find empty slot
	int count = 0;
	while(mem_ptr_list[count].ptr != NULL)
	{
		count++;
		// If you hit this just increase MAX_MEM_POINTERS
		Assert(count < MAX_MEM_POINTERS);
	}
	mem_ptr_list[count].ptr  = ptr;
	mem_ptr_list[count].line = line;
	mem_ptr_list[count].size = size;
	strcpy(mem_ptr_list[count].filename, filename);

	#endif
}

void memblockinfo_output_memleak()
{
	if(!Cmdline_show_mem_usage)	return;

	if(TotalRam == 0) 
		return;

	if(TotalRam < 0) {
		 _RPT1(_CRT_WARN, "TotalRam bad value!",TotalRam);
		return;
	}

	_RPT1(_CRT_WARN, "malloc memory leak of %d\n",TotalRam);

// Now if system is compiled register it with the fuller system
#ifdef _REPORT_MEM_LEAKS

	int total = 0;
	// Find empty slot
	for(int f = 0; f < MAX_MEM_POINTERS; f++)
	{
		if(mem_ptr_list[f].ptr)
		{
		 	_RPT3(_CRT_WARN, "Memory leaks: (%s line %d) of %d bytes\n", mem_ptr_list[f].filename, mem_ptr_list[f].line, mem_ptr_list[f].size);
			total += mem_ptr_list[f].size;
		}
	}

	Assert(TotalRam == total);

#else

	for(int i = 0; i < MAX_MEM_MODULES; i++)
	{
		// Found the first empty entry, fill it
	  	if(mem_block_list[i].size > 0)
		{
			// Oh... bad code... making assumsions...
		 	_RPT2(_CRT_WARN, "Possible memory leaks: %s %d\n", mem_block_list[i].filename, mem_block_list[i].size);
		}
	}
#endif
}

void unregister_malloc(char *filename, int size, void *ptr)
{
	// calculate magic numbers
	int magic1, magic2, len = strlen(filename);

	magic1 = magic2 = 0;

	for(int c = 0; c < len; c++)
	{
		magic1 += filename[c];

		if(c % 2)
			magic2 += filename[c];
		else
			magic2 -= filename[c];
	}

// Now if system is compiled register it with the fuller system
#ifdef _REPORT_MEM_LEAKS

	// Find empty slot
	for(int f = 0; f < MAX_MEM_POINTERS; f++)
	{
		if(mem_ptr_list[f].ptr == ptr) {
			mem_ptr_list[f].ptr = NULL;
			break;
		}
	}

	// This should never happen
	Assert(f < MAX_MEM_POINTERS);

#endif

	for(int i = 0; i < MAX_MEM_MODULES; i++)
	{
		// Found a matching entry, update it
		if(	mem_block_list[i].magic_num1 == magic1 &&
			mem_block_list[i].magic_num2 == magic2 &&
			stricmp(mem_block_list[i].filename, filename) == 0)
		{
			mem_block_list[i].size -= size;
			return;
		}
	}
}

#endif

#ifndef NDEBUG
void *_vm_malloc( int size, char *filename, int line, int quiet )
#else
void *_vm_malloc( int size, int quiet )
#endif
{
	void *ptr = NULL;

#ifndef NEW_MALLOC
	ptr = _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__ );

	if (ptr == NULL)
	{
		mprintf(( "Malloc failed!!!!!!!!!!!!!!!!!!!\n" ));

		if (quiet) {
			return NULL;
		}

		Error(LOCATION, "Malloc Failed!\n");
	}
#ifndef NDEBUG
	TotalRam += size;

	if(Cmdline_show_mem_usage)
		register_malloc(size, filename, line, ptr);
#endif
	return ptr;
#else
 

	ptr = HeapAlloc(Heap, HEAP_FLAG, size );


	if ( ptr == NULL )	{
		mprintf(( "HeapAlloc failed!!!!!!!!!!!!!!!!!!!\n" ));

		if (quiet) {
			return NULL;
		}

		Error(LOCATION, "Out of memory.  Try closing down other applications, increasing your\n"
				"virtual memory size, or installing more physical RAM.\n");

	}
	#ifndef NDEBUG
		int actual_size = HeapSize(Heap, HEAP_FLAG, ptr);
		if ( Watch_malloc )	{
			mprintf(( "Malloc %d bytes [%s(%d)]\n", actual_size, clean_filename(filename), line ));
		}
		TotalRam += actual_size;

	if(Cmdline_show_mem_usage)
		register_malloc(actual_size, filename, line, ptr);

	#endif
#endif

	return ptr;
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
	_CrtMemBlockHeader *phd = pHdr(ptr);
	int nSize = phd->nDataSize;

	TotalRam -= nSize;
	if(Cmdline_show_mem_usage)
		unregister_malloc(filename, nSize, ptr);
#endif

#ifndef NEW_MALLOC
	_free_dbg(ptr,_NORMAL_BLOCK);

#else
	int actual_size = HeapSize(Heap, HEAP_FLAG, ptr);
	if ( Watch_malloc )	{
		mprintf(( "Free %d bytes [%s(%d)]\n", actual_size, clean_filename(filename), line ));
	}
	TotalRam -= actual_size;

	HeapFree( Heap, HEAP_FLAG, ptr );
	HeapCompact(Heap, HEAP_FLAG);
#endif
}

void vm_free_all()
{
}

#ifndef NDEBUG
void *_vm_realloc( void *ptr, int size, char *filename, int line, int quiet )
#else
void *_vm_realloc( void *ptr, int size, int quiet )
#endif
{
	// if this is the first time it's used then we need to malloc it first
	if ( ptr == NULL )
		return vm_malloc(size);

	void *ret_ptr = NULL;

#ifndef NEW_MALLOC

	
#ifndef NDEBUG
	// Unregistered the previous allocation
	_CrtMemBlockHeader *phd = pHdr(ptr);
	int nSize = phd->nDataSize;

	TotalRam -= nSize;
	if(Cmdline_show_mem_usage)
		unregister_malloc(filename, nSize, ptr);
#endif

	ret_ptr = _realloc_dbg(ptr, size,  _NORMAL_BLOCK, __FILE__, __LINE__ );

	if (ret_ptr == NULL) {
		mprintf(( "realloc failed!!!!!!!!!!!!!!!!!!!\n" ));

		if (quiet && (size > 0) && (ptr != NULL)) {
			// realloc doesn't touch the original ptr in the case of failure so we could still use it
			return NULL;
		}

		Error(LOCATION, "Out of memory.  Try closing down other applications, increasing your\n"
			"virtual memory size, or installing more physical RAM.\n");
	}
#ifndef	NDEBUG 
	TotalRam += size;

	// register this allocation
	if(Cmdline_show_mem_usage)
		register_malloc(size, filename, line, ret_ptr);
#endif
	return ret_ptr;
	

#else

	ret_ptr = HeapReAlloc(Heap, HEAP_FLAG, ptr, size);

	if (ret_ptr == NULL) {
		mprintf(( "HeapReAlloc failed!!!!!!!!!!!!!!!!!!!\n" ));

		if (quiet && (size > 0) && (ptr != NULL)) {
			// realloc doesn't touch the original ptr in the case of failure so we could still use it
			return NULL;
		}

		Error(LOCATION, "Out of memory.  Try closing down other applications, increasing your\n"
			"virtual memory size, or installing more physical RAM.\n");
	}

	// do a size check now since we need to know if we got what was asked for
	int actual_size = HeapSize(Heap, HEAP_FLAG, ret_ptr);

	if (actual_size < size) {
		mprintf(( "HeapReAlloc failed!!!!!!!!!!!!!!!!!!!\n" ));

		Error(LOCATION, "The required ammount of memory cannot be allocated.\n"
			"Try closing down other applications, increasing your\n"
			"virtual memory size, or installing more physical RAM.\n");

		vm_free(ret_ptr);

		return NULL;
	}

	#ifndef NDEBUG
		if ( Watch_malloc )	{
			mprintf(( "ReAlloc %d bytes [%s(%d)]\n", actual_size, clean_filename(filename), line ));
		}
		TotalRam += actual_size;

	if(Cmdline_show_mem_usage)
		register_malloc(actual_size, filename, line, ret_ptr);

	#endif
	return ret_ptr;
#endif
}


