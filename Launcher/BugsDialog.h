#if !defined(AFX_BUGSDIALOG_H__DFE5CDE0_3010_4FCD_8FFA_FEFE8E1AB855__INCLUDED_)
#define AFX_BUGSDIALOG_H__DFE5CDE0_3010_4FCD_8FFA_FEFE8E1AB855__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BugsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBugsDialog dialog

class CBugsDialog : public CDialog
{
// Construction
public:
	CBugsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBugsDialog)
	enum { IDD = IDD_BUGS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBugsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBugsDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUGSDIALOG_H__DFE5CDE0_3010_4FCD_8FFA_FEFE8E1AB855__INCLUDED_)
