/////////////////////////////////////////////////////////////////////////////
/* 
DESCRIPTION:
	CFolderDialog  - Folder Selection Dialog Class
	Copyright(C) Armen Hakobyan, 2002 - 2005
	http://www.codeproject.com/dialog/cfolderdialog.asp
	
VERSION HISTORY:
	24 Mar 2002 - First release
	30 Mar 2003 - Some minor changes
				- Added missing in old Platform SDK new flag definitions  
				- Added support for both MFC 6.0 and 7.0
				- Added OnIUnknown handler for Windows XP folder filtration
				- Added SetExpanded and SetOKText and GetSelectedFolder functions
	24 May 2003 - Added OnSelChanged implementation
	14 Jul 2003 - Added custom filtration for Windows XP, thanks to Arik Poznanski
	29 Nov 2003 - Added SetRootFolder, thanks to Eckhard Schwabe ( and Jose Insa )
	02 Jan 2004 - Added GetRootFolder, uncomment if needed
	15 Feb 2005 - Small bug fix in DoModal, thanks to WindSeven
*/
/////////////////////////////////////////////////////////////////////////////

#ifndef __FOLDERDLG_H__
#define __FOLDERDLG_H__

#if defined( _MSC_VER ) && ( _MSC_VER >= 1020 )
	#pragma once
#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef __AFXDLGS_H__
	#include < AfxDlgs.h >
#endif

// Goober5000 - dunno why this wasn't included in the first place
#ifndef _INC_SHLOBJ
	#include <shlobj.h>
#endif

#ifndef __ATLCONV_H__
	#include < AtlConv.h >	// MBCS/Unicode Conversion Macros
#endif

// Uncomment if using GetRootFolder
//#ifndef _INC_SHLWAPI
//	#include < shlwapi.h >
//#endif
//#pragma comment( lib, "shlwapi.lib" )

/////////////////////////////////////////////////////////////////////////////

#ifndef		SAFE_DELETE2
	#define SAFE_DELETE2( p ) \
		if( p ){ delete[] p; p = NULL; }
#endif

#ifndef		SAFE_ZEROMEMORY
	#define SAFE_ZEROMEMORY( p, size ) \
		if( p ){ ZeroMemory( p, size ); }
#endif

#ifndef	SAFE_RELEASE
	#if defined( __cplusplus )
		#define SAFE_RELEASE( p ) \
			if( p ){ p->Release(); p = NULL; }
	#else
		#define SAFE_RELEASE( p ) \
			if( p ){ p->lpVtbl->Release( p ); p = NULL; }
	#endif
#endif

#ifndef		SAFE_COTASKMEMFREE
	#define SAFE_COTASKMEMFREE( p ) \
		if( p ){ CoTaskMemFree( (LPVOID)p ); p = NULL; }
#endif

#ifndef		_countof
	#define _countof( x ) \
		( sizeof( x ) / sizeof( x[ 0 ] ) )
#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef BFFM_SETOKTEXT					// Version 5.0 or later
	#define BFFM_SETOKTEXT				( WM_USER + 105 )	// Unicode only, req. BIF_USENEWUI
	#define BFFM_SETEXPANDED			( WM_USER + 106 )	// Unicode only, req. BIF_USENEWUI
#endif

#ifndef BIF_NEWDIALOGSTYLE				// Version 5.0 or later
	#define BIF_NEWDIALOGSTYLE			0x0040	
	#define BIF_BROWSEINCLUDEURLS		0x0080
	#define BIF_UAHINT					0x0100				// Req. BIF_NEWDIALOGSTYLE
	#define BIF_NONEWFOLDERBUTTON		0x0200
	#define BIF_NOTRANSLATETARGETS		0x0400	
	#define BIF_SHAREABLE				0x8000				// Req. BIF_USENEWUI
	#define BIF_USENEWUI				( BIF_NEWDIALOGSTYLE | BIF_EDITBOX )	
#endif

/////////////////////////////////////////////////////////////////////////////

class CFolderDialog : public CCommonDialog
{
	DECLARE_DYNAMIC( CFolderDialog )

public:
	CFolderDialog(	IN LPCTSTR	pszTitle	= NULL, 
					IN LPCTSTR	pszSelPath	= NULL,
					IN CWnd*	pWndParent	= NULL,
					IN UINT		uFlags		= BIF_RETURNONLYFSDIRS );
	virtual ~CFolderDialog( VOID );

public:
	#if ( _MFC_VER >= 0x0700 )	// VC++ 2002 (7.0)
		virtual INT_PTR		DoModal( VOID );
	#else
		virtual INT			DoModal( VOID );
	#endif

	BOOL	SetRootFolder( IN LPCTSTR pszPath );
	BOOL	GetRootFolder( IN OUT LPTSTR pszPath );
	BOOL	SetSelectedFolder( IN LPCTSTR pszPath );

public:	
	AFX_INLINE LPCTSTR	GetFolderPath( VOID )  const;
	AFX_INLINE LPCTSTR	GetFolderName( VOID )  const;
	AFX_INLINE INT		GetFolderImage( VOID ) const;		
	AFX_INLINE LPCTSTR	GetSelectedFolder( VOID ) const;

	AFX_INLINE BROWSEINFO&		 GetBI( VOID );
	AFX_INLINE const BROWSEINFO& GetBI( VOID ) const;

protected:	
	BROWSEINFO	m_bi;

	TCHAR	m_szSelPath[ MAX_PATH ];
	TCHAR	m_szFolPath[ MAX_PATH ];

protected:
	DECLARE_MESSAGE_MAP()
	
protected: // Overridables

	virtual VOID OnInitialized( VOID );
	virtual VOID OnSelChanged( IN LPITEMIDLIST  pItemIDList );
	virtual INT	 OnValidateFailed( IN LPCTSTR /*pszPath*/ );		

protected: // Windows XP or later
	virtual VOID OnIUnknown( IN IUnknown* /*pIUnknown*/ );
	
protected: // Valid to call only from the above handlers

	VOID EnableOK( IN BOOL bEnable = TRUE );
	VOID SetSelection( IN LPITEMIDLIST pItemIDList );
	VOID SetSelection( IN LPCTSTR pszPath );	
	VOID SetStatusText( IN LPCTSTR pszText );

protected: // Shell version 5.0 or later:
	VOID SetExpanded( IN LPCTSTR pszPath );
	VOID SetExpanded( IN LPITEMIDLIST pItemIDList );
	VOID SetOKText( IN LPCTSTR pszText );

private:
	HWND m_hWnd; // used only in the callback function

private:
	static INT CALLBACK BrowseCallbackProc(
		IN HWND hWnd, IN UINT uMsg, IN LPARAM lParam, IN LPARAM lpData
	);
};

/////////////////////////////////////////////////////////////////////////////

AFX_INLINE LPCTSTR CFolderDialog::GetSelectedFolder( VOID ) const
	{ return m_szSelPath; }

AFX_INLINE BROWSEINFO& CFolderDialog::GetBI( VOID )
	{ return m_bi; }

AFX_INLINE const BROWSEINFO& CFolderDialog::GetBI( VOID ) const
	{ return m_bi; }

/////////////////////////////////////////////////////////////////////////////
// Filled after a call to DoModal

AFX_INLINE LPCTSTR CFolderDialog::GetFolderPath( VOID ) const
	{ return m_szFolPath; }

AFX_INLINE LPCTSTR CFolderDialog::GetFolderName( VOID ) const
	{ return m_bi.pszDisplayName; }

AFX_INLINE INT CFolderDialog::GetFolderImage( VOID ) const
	{ return m_bi.iImage; }

/////////////////////////////////////////////////////////////////////////////
#endif // __FOLDERDLG_H__
/////////////////////////////////////////////////////////////////////////////
