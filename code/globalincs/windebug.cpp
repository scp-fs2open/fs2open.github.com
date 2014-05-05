/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

// Nothing in this module should be externalized!!!
//XSTR:OFF

//#define DUMPRAM	// This dumps all symbol sizes. See John for more info

/* Windows Headers */
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#ifdef _MSC_VER
#	include <crtdbg.h>
	/* Uncomment SHOW_CALL_STACK to show the call stack in Asserts, Warnings, and Errors */
#	define SHOW_CALL_STACK
#endif // _MSC_VER

/* STL Headers */
#include <string>

/* SCP Headers */
#include "osapi/osapi.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "cmdline/cmdline.h"
#include "parse/lua.h"
#include "parse/parselo.h"
#include "debugconsole/console.h"

#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )
#	include "globalincs/mspdb_callstack.h"
#endif

extern void gr_activate(int active);

bool Messagebox_active = false;

int Global_warning_count = 0;
int Global_error_count = 0;

const int Messagebox_lines = 30;

#ifndef _ASSERT
  #ifndef _DEBUG
    #define _ASSERT(expr) ((void)0)
  #else
    #define _ASSERT(expr) (assert(expr))
//    #error _ASSERT is not defined yet for debug mode with non-MSVC compilers
  #endif
#endif

const char *clean_filename( const char *name)
{
	const char *p = name+strlen(name)-1;
	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') && (p>= name) )
		p--;
	p++;	

	return p;	
}		   

#if defined( SHOW_CALL_STACK )
static bool Dump_to_log = true; 

class DumpBuffer
{
public :
	enum { BUFFER_SIZE = 32000 } ;
	DumpBuffer() ;
	void Clear() ;
	void Printf( const char* format, ... ) ;
	void SetWindowText( HWND hWnd ) const ;
	char buffer[ BUFFER_SIZE ] ;

	void Append(const char* text);
	void Truncate(size_t size);
	void TruncateLines(int num_allowed_lines);
	size_t Size() const;
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


void DumpBuffer :: Append(const char* text)
{
	strcat_s(buffer, text);
}


void DumpBuffer :: Truncate(size_t size)
{
	if (size >= strlen(buffer))
		return;

	buffer[size] = 0;
}


// adapted from parselo
void DumpBuffer :: TruncateLines(int num_allowed_lines)
{
	Assert(num_allowed_lines > 0);
	char *find_from = buffer;
	char *lastch = find_from + strlen(buffer) - 6;

	while (find_from < lastch)
	{
		if (num_allowed_lines <= 0)
		{
			*find_from = 0;
			strcat_s(buffer, "[...]");
			break;
		}

		char *p = strchr(find_from, '\n');
		if (p == NULL)
			break;

		num_allowed_lines--;
		find_from = p + 1;
	}
}


size_t DumpBuffer :: Size() const
{
	return strlen(buffer);
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

/* Needed by LUA printf */
// This ought to be local to VerboseAssert, but it
// causes problems in Visual C++ (in the CRTL init phase)
static DumpBuffer dumpBuffer;
const char* Separator = "------------------------------------------------------------------\n" ;

#endif

/* MSVC2005+ callstack support
 */
#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )

class SCP_DebugCallStack : public SCP_IDumpHandler
{
public:
	virtual bool ResolveSymbols( )
	{
		return true;
	}

	virtual void OnBegin( )
	{
	}

	virtual void OnEnd( )
	{
	}

	virtual void OnEntry( void* address, const char* module, const char* symbol )
	{
		UNREFERENCED_PARAMETER( address );

		StackEntry entry;
		entry.module = clean_filename( module );
		entry.symbol = symbol;
		m_stackFrames.push_back( entry );
	}
	
	virtual void OnError( const char* error )
	{
		/* No error handling here! */
		UNREFERENCED_PARAMETER( error );
	}

	SCP_string DumpToString( )
	{
		SCP_string callstack;
		for ( size_t i = 0; i < m_stackFrames.size( ); i++ )
		{
			callstack += m_stackFrames[ i ].module + "! " + m_stackFrames[ i ].symbol + "\n";
		}

		return callstack; /* Inefficient, but we don't need efficient here */
	}
private:
	struct StackEntry
	{
		SCP_string module;
		SCP_string symbol;
	};

	SCP_vector< StackEntry > m_stackFrames;
};

#elif defined( SHOW_CALL_STACK )

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
  if( lineNum != none ) {
    dumpBuffer.Printf( "  line %d\r\n", lineNum ) ;
	if (Dump_to_log) {
		mprintf(( "  line %d\r\n", lineNum )) ;
	}
  }	
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
	//strcpy_s( dst, src );
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
	
	strcpy_s( sym->name, name );	
	strcat_s( sym->name, "(" );	
	strcat_s( sym->name, module );	
	strcat_s( sym->name, ")" );	

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

	insertion_sort( Symbols, Num_symbols, sizeof(MemSymbol), Sym_compare );
	
	for (i=0;i<Num_symbols; i++ )	{
		MemSymbol * sym1 = &Symbols[i];
		MemSymbol * sym2 = &Symbols[i+1];
		if ( (i<Num_symbols-1) && (sym1->section == sym2->section) )	{
			sym1->size = sym2->offset-sym1->offset;
		} else {
			sym1->size = -1;
		}
	}

	insertion_sort( Symbols, Num_symbols, sizeof(MemSymbol), Sym_compare1 );


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
					strcpy_s( tmpFile, auxSym ) ;
					strcpy_s( pretty_module, tmpFile );
					char *p = pretty_module+strlen(pretty_module)-1;
					// Move p to point to first letter of EXE filename
					while( (*p!='\\') && (*p!='/') && (*p!=':') )
						p--;
					p++;	
					if ( strlen(p) < 1 ) {
						strcpy_s( pretty_module, "<unknown>" );
					} else {
						memmove( pretty_module, p, strlen(p)+1 );
					}
				} else {
					strcpy_s( pretty_module, "" );
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
			strcpy_s( latestFile, auxSym ) ;
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
		if (Dump_to_log) {
			mprintf(("    %s()", tmp_name )) ;
		}
	} else {
		dumpBuffer.Printf( "    <unknown>" ) ;
		if (Dump_to_log) {		
			mprintf(("    <unknown>" )) ;
		}
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
		strcpy_s( latestModule, module );
		//JAS    dumpBuffer.Printf( "Module: %s\r\n", module );
		MapFileInMemory( module );
		FindDebugInfo();
	}

	char pretty_module[1024];

	strcpy_s( pretty_module, module );
	char *p = pretty_module+strlen(pretty_module)-1;
	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') )
		p--;
	p++;	
	if ( strlen(p) < 1 ) {
		strcpy_s( pretty_module, "<unknown>" );
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
				if (Dump_to_log) {
					mprintf(("    %s %08x()\r\n", pretty_module, caller )) ;
				}
				return 0;
			}
		} catch( ... )	{
			// Header wrong, do nothing
			return 0;
      }
	} else	{
		dumpBuffer.Printf( "    %s %08x()\r\n", pretty_module, caller ) ;
		if (Dump_to_log) {
			mprintf(( "    %s %08x()\r\n", pretty_module, caller )) ;
		}
		//JAS dumpBuffer.Printf( "  module not accessible\r\n" ) ;
		//JAS dumpBuffer.Printf( "    address: %8X\r\n", caller ) ;
		return 0;
	}

	Int3();

}

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

#endif	//SHOW_CALL_STACK


char AssertText1[2048];
char AssertText2[1024];

uint flags = MB_SYSTEMMODAL|MB_SETFOREGROUND;
//uint flags = MB_SYSTEMMODAL;

extern void gr_force_windowed();

void dump_text_to_clipboard( const char *text )
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

	// this stuff migt be really useful for solving bug reports and user errors. We should output it! 
	mprintf(("ASSERTION: \"%s\" at %s:%d\n", text, strrchr(filename, '\\')+1, linenum ));

#ifdef Allow_NoWarn
	if (Cmdline_nowarn) {
		return;
	}
#endif

	Messagebox_active = true;

	gr_activate(0);

	filename = strrchr(filename, '\\')+1;
	sprintf( AssertText1, "Assert: %s\r\nFile: %s\r\nLine: %d\r\n", text, filename, linenum );

#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )
	/* Dump the callstack */
	SCP_DebugCallStack callStack;
	SCP_DumpStack( dynamic_cast< SCP_IDumpHandler* >( &callStack ) );
	
	/* Format the string */
	SCP_string assertString( AssertText1 );
	assertString += "\n";
	assertString += callStack.DumpToString( );
	
	/* Copy to the clipboard */
	dump_text_to_clipboard( assertString.c_str( ) );

	// truncate text
	truncate_message_lines(assertString, Messagebox_lines);

	assertString += "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";
	assertString += "\n\nUse Ok to break into Debugger, Cancel to exit.\n";
	val = MessageBox( NULL, assertString.c_str( ), "Assertion Failed!", MB_OKCANCEL | flags );

#elif defined( SHOW_CALL_STACK )
	dumpBuffer.Clear();
	dumpBuffer.Printf( AssertText1 );
	dumpBuffer.Printf( "\r\n" );
	DumpCallsStack( dumpBuffer ) ;  
	dump_text_to_clipboard(dumpBuffer.buffer);

	// truncate text
	dumpBuffer.TruncateLines(Messagebox_lines);

	dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
	dumpBuffer.Printf( "\r\n\r\nUse Ok to break into Debugger, Cancel to exit.\r\n");

	val = MessageBox(NULL, dumpBuffer.buffer, "Assertion Failed!", MB_OKCANCEL|flags );
#else
	val = MessageBox(NULL, AssertText1, "Assertion Failed!", MB_OKCANCEL|flags );
#endif

	if (val == IDCANCEL)
		exit(1);

	Int3();

	gr_activate(1);

	Messagebox_active = false;
}

void _cdecl WinAssert(char * text, char * filename, int linenum, const char * format, ... )
{
	int val;
	
	va_list args;

	va_start(args, format);
	vsprintf(AssertText2, format, args);
	va_end(args);

	// this stuff migt be really useful for solving bug reports and user errors. We should output it! 
	mprintf(("ASSERTION: \"%s\" at %s:%d\n %s\n", text, strrchr(filename, '\\')+1, linenum, AssertText2 ));

#ifdef Allow_NoWarn
	if (Cmdline_nowarn) {
		return;
	}
#endif

	Messagebox_active = true;

	gr_activate(0);

	filename = strrchr(filename, '\\')+1;
	sprintf( AssertText1, "Assert: %s\r\nFile: %s\r\nLine: %d\r\n%s\r\n", text, filename, linenum, AssertText2 );

#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )
	/* Dump the callstack */
	SCP_DebugCallStack callStack;
	SCP_DumpStack( dynamic_cast< SCP_IDumpHandler* >( &callStack ) );
	
	/* Format the string */
	SCP_string assertString( AssertText1 );
	assertString += "\n";
	assertString += callStack.DumpToString( );
	
	/* Copy to the clipboard */
	dump_text_to_clipboard( assertString.c_str( ) );

	// truncate text
	truncate_message_lines(assertString, Messagebox_lines);

	assertString += "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";
	assertString += "\n\nUse Ok to break into Debugger, Cancel to exit.\n";
	val = MessageBox( NULL, assertString.c_str( ), "Assertion Failed!", MB_OKCANCEL | flags );

#elif defined ( SHOW_CALL_STACK	)
	dumpBuffer.Clear();
	dumpBuffer.Printf( AssertText1 );
	dumpBuffer.Printf( "\r\n" );
	DumpCallsStack( dumpBuffer ) ;  
	dump_text_to_clipboard(dumpBuffer.buffer);

	// truncate text
	dumpBuffer.TruncateLines(Messagebox_lines);

	dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
	dumpBuffer.Printf( "\r\n\r\nUse Ok to break into Debugger, Cancel to exit.\r\n");

	val = MessageBox(NULL, dumpBuffer.buffer, "Assertion Failed!", MB_OKCANCEL|flags );
#else
	val = MessageBox(NULL, AssertText1, "Assertion Failed!", MB_OKCANCEL|flags );
#endif

	if (val == IDCANCEL)
		exit(1);

	Int3();

	gr_activate(1);

	Messagebox_active = false;
}

void LuaDebugPrint(lua_Debug &ar)
{
	dumpBuffer.Printf( "Name:\t\t%s\r\n",  ar.name);
	dumpBuffer.Printf( "Name of:\t%s\r\n",  ar.namewhat);
	dumpBuffer.Printf( "Function type:\t%s\r\n",  ar.what);
	dumpBuffer.Printf( "Defined on:\t%d\r\n",  ar.linedefined);
	dumpBuffer.Printf( "Upvalues:\t%d\r\n",  ar.nups);
	dumpBuffer.Printf( "\r\n" );
	dumpBuffer.Printf( "Source:\t\t%s\r\n",  ar.source);
	dumpBuffer.Printf( "Short source:\t%s\r\n",  ar.short_src);
	dumpBuffer.Printf( "Current line:\t%d\r\n",  ar.currentline);
	dumpBuffer.Printf( "- Function line:\t%d\r\n", (ar.linedefined ? (1 + ar.currentline - ar.linedefined) : 0));
}

extern lua_Debug Ade_debug_info;
extern char debug_stack[4][32];
void LuaError(struct lua_State *L, char *format, ...)
{
	int val;

	Messagebox_active = true;

	gr_activate(0);

	/*
	va_start(args, format);
	vsprintf(AssertText1,format,args);
	va_end(args);
	*/
	
	//filename = strrchr(filename, '\\')+1;
	//sprintf(AssertText2,"LuaError: %s\r\nFile: %s\r\nLine: %d\r\n", AssertText1, filename, line );

	dumpBuffer.Clear();
	//WMC - if format is set to NULL, assume this is acting as an
	//error handler for Lua.
	if(format == NULL)
	{
		dumpBuffer.Printf("LUA ERROR: %s", lua_tostring(L, -1));
		lua_pop(L, -1);
	}
	else
	{
		va_list args;
		va_start(args, format);
		vsprintf(AssertText1,format,args);
		dumpBuffer.Printf(AssertText1);
		va_end(args);
	}

	dumpBuffer.Printf( "\r\n" );
	dumpBuffer.Printf( "\r\n" );

	//WMC - This is virtually worthless.
/*
	dumpBuffer.Printf(Separator);
	dumpBuffer.Printf( "LUA Debug:" );
	dumpBuffer.Printf( "\r\n" );
	dumpBuffer.Printf(Separator);

	lua_Debug ar;
	if(lua_getstack(L, 0, &ar))
	{
		lua_getinfo(L, "nSlu", &ar);
		LuaDebugPrint(ar);
	}
	else
	{
		dumpBuffer.Printf("(No stack debug info)\r\n");
	}
*/
//	TEST CODE

	dumpBuffer.Printf(Separator);
	dumpBuffer.Printf( "ADE Debug:" );
	dumpBuffer.Printf( "\r\n" );
	dumpBuffer.Printf(Separator);
	LuaDebugPrint(Ade_debug_info);
	dumpBuffer.Printf(Separator);

	dumpBuffer.Printf( "\r\n" );
	dumpBuffer.Printf( "\r\n" );

	AssertText2[0] = '\0';
	dumpBuffer.Printf(Separator);
	
	// Get the stack via the debug.traceback() function
	lua_getglobal(L, LUA_DBLIBNAME);

	if (!lua_isnil(L, -1))
	{
		dumpBuffer.Printf( "\r\n" );
		lua_getfield(L, -1, "traceback");
		lua_remove(L, -2);

		if (lua_pcall(L, 0, 1, 0) != 0)
			dumpBuffer.Printf("Error while retrieving stack: %s", lua_tostring(L, -1));
		else
			dumpBuffer.Printf(lua_tostring(L, -1));

		lua_pop(L, 1);
	}
	else
	{
		// If the debug library is nil then fall back to the default debug stack
		dumpBuffer.Printf("LUA Stack:\r\n");
		int i;
		for (i = 0; i < 4; i++) {
			if (debug_stack[i][0] != '\0')
				dumpBuffer.Printf("\t%s\r\n", debug_stack[i]);
		}
	}
	dumpBuffer.Printf( "\r\n" );

	dumpBuffer.Printf(Separator);
	ade_stackdump(L, AssertText2);
	dumpBuffer.Printf( AssertText2 );
	dumpBuffer.Printf( "\r\n" );
	dumpBuffer.Printf(Separator);

	dump_text_to_clipboard(dumpBuffer.buffer);

	// truncate text
	dumpBuffer.TruncateLines(Messagebox_lines);

	dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
	dumpBuffer.Printf( "\r\n\r\nUse Yes to break into Debugger, No to continue.\r\nand Cancel to Quit");

	val = MessageBox(NULL, dumpBuffer.buffer, "Error!", flags|MB_YESNOCANCEL );

	if (val == IDCANCEL ) {
		exit(1);
	} else if(val == IDYES) {
		Int3();
	}

	gr_activate(1);

	Messagebox_active = false;
}

void _cdecl Error( const char * filename, int line, const char * format, ... )
{
	Global_error_count++;

	int val;
	va_list args;

	va_start(args, format);
	vsprintf(AssertText1, format, args);
	va_end(args);

	filename = strrchr(filename, '\\')+1;
	sprintf(AssertText2, "Error: %s\r\nFile: %s\r\nLine: %d\r\n", AssertText1, filename, line);
	mprintf(("ERROR: %s\r\nFile: %s\r\nLine: %d\r\n", AssertText1, filename, line));

	Messagebox_active = true;

	gr_activate(0);

#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )
	/* Dump the callstack */
	SCP_DebugCallStack callStack;
	SCP_DumpStack( dynamic_cast< SCP_IDumpHandler* >( &callStack ) );
	
	/* Format the string */
	SCP_string assertString( AssertText1 );
	assertString += "\n";
	assertString += callStack.DumpToString( );
	
	/* Copy to the clipboard */
	dump_text_to_clipboard( assertString.c_str( ) );

	// truncate text
	truncate_message_lines(assertString, Messagebox_lines);

	assertString += "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";
	assertString += "\n\nUse Ok to break into Debugger, Cancel to exit.\n";
	val = MessageBox( NULL, assertString.c_str( ), "Error!", flags | MB_DEFBUTTON2 | MB_OKCANCEL );

#elif defined( SHOW_CALL_STACK )
	dumpBuffer.Clear();
	dumpBuffer.Printf( AssertText2 );
	dumpBuffer.Printf( "\r\n" );
	DumpCallsStack( dumpBuffer ) ;  
	dump_text_to_clipboard(dumpBuffer.buffer);

	// truncate text
	dumpBuffer.TruncateLines(Messagebox_lines);

	dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
	dumpBuffer.Printf( "\r\n\r\nUse Ok to break into Debugger, Cancel exits.\r\n");

	val = MessageBox(NULL, dumpBuffer.buffer, "Error!", flags | MB_DEFBUTTON2 | MB_OKCANCEL );
#else
	strcat_s(AssertText2,"\r\n\r\nUse Ok to break into Debugger, Cancel exits.\r\n");

	val = MessageBox(NULL, AssertText2, "Error!", flags | MB_DEFBUTTON2 | MB_OKCANCEL );
#endif

	switch (val)
	{
		case IDCANCEL:
			exit(1);

		default:
			Int3();
			break;
	}

	gr_activate(1);

	Messagebox_active = false;
}

void _cdecl WarningEx( char *filename, int line, const char *format, ... )
{
#ifndef NDEBUG
	if (Cmdline_extra_warn) {
		char msg[sizeof(AssertText1)];
		va_list args;
		va_start(args, format);
		vsprintf(msg, format, args);
		va_end(args);
		Warning(filename, line, msg);
	}
#endif
}

void _cdecl Warning( char *filename, int line, const char *format, ... )
{
	Global_warning_count++;

#ifndef NDEBUG
	va_list args;
	int result;
	int i;
	int slen = 0;

	// output to the debug log before anything else (so that we have a complete record)

	memset( AssertText1, 0, sizeof(AssertText1) );
	memset( AssertText2, 0, sizeof(AssertText2) );

	va_start(args, format);
	vsprintf(AssertText1, format, args);
	va_end(args);

	slen = strlen(AssertText1);

	// strip out the newline char so the output looks better
	for (i = 0; i < slen; i++){
		if (AssertText1[i] == (char)0x0a) {
			AssertText2[i] = ' ';
		} else {
			AssertText2[i] = AssertText1[i];
		}
	}

	// kill off extra white space at end
	if (AssertText2[slen-1] == (char)0x20) {
		AssertText2[slen-1] = '\0';
	} else {
		// just being careful
		AssertText2[slen] = '\0';
	}

	mprintf(("WARNING: \"%s\" at %s:%d\n", AssertText2, strrchr(filename, '\\')+1, line));

	// now go for the additional popup window, if we want it ...
#ifdef Allow_NoWarn
	if (Cmdline_nowarn) {
		return;
	}
#endif

	filename = strrchr(filename, '\\')+1;
	sprintf(AssertText2, "Warning: %s\r\nFile: %s\r\nLine: %d\r\n", AssertText1, filename, line );

	Messagebox_active = true;

	gr_activate(0);

#if defined( SHOW_CALL_STACK ) && defined( PDB_DEBUGGING )
	/* Dump the callstack */
	SCP_DebugCallStack callStack;
	SCP_DumpStack( dynamic_cast< SCP_IDumpHandler* >( &callStack ) );
	
	/* Format the string */
	SCP_string assertString( AssertText1 );
	assertString += "\n";
	assertString += callStack.DumpToString( );
	
	/* Copy to the clipboard */
	dump_text_to_clipboard( assertString.c_str( ) );

	// truncate text
	truncate_message_lines(assertString, Messagebox_lines);

	assertString += "\n[ This info is in the clipboard so you can paste it somewhere now ]\n";
	assertString += "\n\nUse Yes to break into Debugger, No to continue.\nand Cancel to Quit\n";
	result = MessageBox( NULL, assertString.c_str( ), "Warning!", MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING | flags );

#elif defined ( SHOW_CALL_STACK	)
	//we don't want to dump the call stack for every single warning
	Dump_to_log = false; 

	dumpBuffer.Clear();
	dumpBuffer.Printf( AssertText2 );
	dumpBuffer.Printf( "\r\n" );
	DumpCallsStack( dumpBuffer ) ;  
	dump_text_to_clipboard(dumpBuffer.buffer);

	// truncate text
	dumpBuffer.TruncateLines(Messagebox_lines);

	dumpBuffer.Printf( "\r\n[ This info is in the clipboard so you can paste it somewhere now ]\r\n" );
	dumpBuffer.Printf("\r\n\r\nUse Yes to break into Debugger, No to continue.\r\nand Cancel to Quit");

	result = MessageBox((HWND)os_get_window(), dumpBuffer.buffer, "Warning!", MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING | flags );

	Dump_to_log = true; 

#else
	strcat_s(AssertText2,"\r\n\r\nUse Yes to break into Debugger, No to continue.\r\nand Cancel to Quit");
	result = MessageBox((HWND)os_get_window(), AssertText2, "Warning!", MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING | flags );
#endif

	switch (result)
	{
		case IDYES:
			Int3();
			break;

		case IDNO:
			break;

		case IDCANCEL:
			exit(1);
	}

	gr_activate(1);

	Messagebox_active = false;
#endif // !NDEBUG
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

int Watch_malloc = 0;
DCF_BOOL(watch_malloc, Watch_malloc );

// Returns 0 if not enough RAM.
int vm_init(int min_heap_size)
{
	#ifndef NDEBUG
	TotalRam = 0;
	#endif

	return 1;
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
	insertion_sort(mem_block_list, MAX_MEM_MODULES, sizeof(MemBlockInfo), memblockinfo_sort_compare );
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
			strcpy_s(mem_block_list[i].filename, filename);
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
	strcpy_s(mem_ptr_list[count].filename, filename);

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
	int magic1, magic2, len;
	
	char *temp = strrchr(filename, '\\');
	if(temp)
		filename = temp + 1;

	len = strlen(filename);

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

	strcpy_s( dst, len + 1, ptr );
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
		return;
	}



#ifndef NDEBUG
	_CrtMemBlockHeader *phd = pHdr(ptr);
	int nSize = phd->nDataSize;

	TotalRam -= nSize;
	if(Cmdline_show_mem_usage)
		unregister_malloc(filename, nSize, ptr);
#endif

	_free_dbg(ptr,_NORMAL_BLOCK);
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
}
