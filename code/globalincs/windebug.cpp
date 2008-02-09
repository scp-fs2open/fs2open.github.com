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
 * $Revision: 2.2 $
 * $Date: 2003-03-02 05:30:26 $
 * $Author: penguin $
 *
 * Debug stuff
 *
 * $Log: not supported by cvs2svn $
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

#include "globalincs/pstypes.h"

#ifdef _MSC_VER
#include <crtdbg.h>

//Uncomment SHOW_CALL_STACK to show the call stack in Asserts, Warnings, and Errors
#define SHOW_CALL_STACK
#endif // _MSC_VER


#ifndef __ASSERT
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
	Symbols = (MemSymbol *)malloc(Max_symbols*sizeof(MemSymbol));
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
	
	free( Symbols );
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
		if ( !isFunction && (currentSym->SectionNumber>-1) )	{
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


void DumpCallsStack( DumpBuffer& dumpBuffer )
{
	const char* separator = "------------------------------------------------------------------\r\n" ;
	static PE_Debug PE_debug ;

	dumpBuffer.Printf( "\r\nCall stack:\r\n" ) ;
	dumpBuffer.Printf( separator ) ;

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


	dumpBuffer.Printf( separator ) ;
	PE_debug.ClearReport() ;  // Prepare for future calls
}


// This ought to be local to VerboseAssert, but it
// causes problems in Visual C++ (in the CRTL init phase)
static DumpBuffer dumpBuffer ;

#endif	//SHOW_CALL_STACK


char AssertText1[1024];
char AssertText2[1024];

uint flags = MB_TASKMODAL|MB_SETFOREGROUND;

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
	int val;

	gr_force_windowed();


	sprintf( AssertText1, "Assert: %s\r\nFile: %s\r\nLine: %d", text, filename, linenum );

	#ifdef SHOW_CALL_STACK
		dumpBuffer.Clear();
		dumpBuffer.Printf( AssertText1 );
		dumpBuffer.Printf( "\r\n" );
		DumpCallsStack( dumpBuffer ) ;  
		dump_text_to_clipboard(dumpBuffer.buffer);

		dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
		dumpBuffer.Printf( "\r\n\r\nUse Ok to break into Debugger, Cancel to exit.\r\n");

		val = MessageBox(NULL, dumpBuffer.buffer, "Assertion Failed!", MB_OKCANCEL|flags );
	#else
		val = MessageBox(NULL, AssertText1, "Assertion Failed!", MB_OKCANCEL|flags );
	#endif

	if (val == IDCANCEL)
		exit(1);

#ifndef INTERPLAYQA
	Int3();
#else
	AsmInt3();
#endif


} 

void _cdecl Error( char * filename, int line, char * format, ... )
{
	int val;
	va_list args;

	gr_force_windowed();

	va_start(args, format);
	vsprintf(AssertText1,format,args);
	va_end(args);
	sprintf(AssertText2,"Error: %s\r\nFile:%s\r\nLine: %d", AssertText1, filename, line );

	#ifdef SHOW_CALL_STACK
		dumpBuffer.Clear();
		dumpBuffer.Printf( AssertText2 );
		dumpBuffer.Printf( "\r\n" );
		DumpCallsStack( dumpBuffer ) ;  
		dump_text_to_clipboard(dumpBuffer.buffer);

		dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
		dumpBuffer.Printf( "\r\n\r\nUse Ok to break into Debugger, Cancel exits.\r\n");

		val = MessageBox(NULL, dumpBuffer.buffer, "Error!", flags|MB_OKCANCEL );
	#else
		strcat(AssertText2,"\r\n\r\nUse Ok to break into Debugger, Cancel exits.\r\n");
		val = MessageBox(NULL, AssertText2, "Error!", flags|MB_OKCANCEL );
	#endif

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
#ifndef NDEBUG

	int id;
	
	va_list args;

	gr_force_windowed();

	va_start(args, format);
	vsprintf(AssertText1,format,args);
	va_end(args);
	sprintf(AssertText2,"Warning: %s\r\nFile:%s\r\nLine: %d", AssertText1, filename, line );


	#ifdef SHOW_CALL_STACK
		dumpBuffer.Clear();
		dumpBuffer.Printf( AssertText2 );
		dumpBuffer.Printf( "\r\n" );
		DumpCallsStack( dumpBuffer ) ;  
		dump_text_to_clipboard(dumpBuffer.buffer);

		dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
		dumpBuffer.Printf("\r\n\r\nUse Yes to break into Debugger, No to continue.\r\nand Cancel to Quit");

		id = MessageBox(NULL, dumpBuffer.buffer, "Warning!", MB_YESNOCANCEL|flags );
	#else
		strcat(AssertText2,"\r\n\r\nUse Yes to break into Debugger, No to continue.\r\nand Cancel to Quit");
		id = MessageBox(NULL, AssertText2, "Warning!", MB_YESNOCANCEL|flags );
	#endif
	if ( id == IDCANCEL )
		exit(1);
	else if ( id == IDYES ) {
#ifndef INTERPLAYQA
		Int3();
#else
		AsmInt3();
#endif
	}
#endif // NDEBUG
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

/*   mprintf(( "Memory operation in %s, line %d: %s a %d-byte '%s' block (# %ld)\n",
            szFileName, nLine, operation[nAllocType], nSize, 
            blockType[nBlockUse], lRequest ));
   if ( pvData != NULL )
      mprintf(( " at %X", pvData ));

	mprintf(("\n" ));
*/

   return( TRUE );         // Allow the memory operation to proceed
}
#endif  // #if 0


 
void windebug_memwatch_init()
{
	//_CrtSetAllocHook(MyAllocHook);
	TotalRam = 0;
}

#endif

#define NEW_MALLOC


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

#ifndef NDEBUG
void *vm_malloc( int size, char *filename, int line )
#else
void *vm_malloc( int size )
#endif
{
	#if (!defined(NDEBUG)) && defined(_MSC_VER)
	if ( !Heap )	{
		TotalRam += size;

		return _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__ );
	}
	#endif
 
	void *ptr = HeapAlloc(Heap, HEAP_FLAG, size );

	if ( ptr == NULL )	{
		mprintf(( "HeapAlloc failed!!!!!!!!!!!!!!!!!!!\n" ));

			Error(LOCATION, "Out of memory.  Try closing down other applications, increasing your\n"
				"virtual memory size, or installing more physical RAM.\n");

	}
	#ifndef NDEBUG
		int actual_size = HeapSize(Heap, HEAP_FLAG, ptr);
		if ( Watch_malloc )	{
			mprintf(( "Malloc %d bytes [%s(%d)]\n", actual_size, clean_filename(filename), line ));
		}
		TotalRam += actual_size;
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

#if (!defined(NDEBUG)) && defined(_MSC_VER)
	if ( !Heap )	{
		_CrtMemBlockHeader *phd = pHdr(ptr);
		int nSize = phd->nDataSize;

		TotalRam -= nSize;

		_free_dbg(ptr,_NORMAL_BLOCK);
		return;
	}

	int actual_size = HeapSize(Heap, HEAP_FLAG, ptr);
	if ( Watch_malloc )	{
		mprintf(( "Free %d bytes [%s(%d)]\n", actual_size, clean_filename(filename), line ));
	}
	TotalRam -= actual_size;
#endif
	HeapFree( Heap, HEAP_FLAG, ptr );
	HeapCompact(Heap, HEAP_FLAG);
}

void vm_free_all()
{
}




