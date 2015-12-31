/*
 * Created by Hassan "Karajorma" Kazmi for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#if !defined(AFX_EditContainerAddNewDlg_H)
#define AFX_EditContainerAddNewDlg_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditContainerAddNewDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Add Container dialog

class CEditContainerAddNewDlg : public CDialog
{
// Construction
public:
	CEditContainerAddNewDlg(CWnd* pParent = NULL);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CEditContainerAddNewDlg)
	enum { IDD = IDD_ADD_NEW_CONTAINER };
	CString	m_new_container_name;
	bool cancelled;
	//}}AFX_DATA

	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditContainerAddNewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditContainerAddNewDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditContainerAddNewDlg_H)