#if !defined(AFX_TABHELP_H__48E3367A_08C5_4B51_8F02_45E3055CFC2D__INCLUDED_)
#define AFX_TABHELP_H__48E3367A_08C5_4B51_8F02_45E3055CFC2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabHelp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabHelp dialog

class CTabHelp : public CDialog
{
// Construction
public:
	CTabHelp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabHelp)
	enum { IDD = IDD_HELP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabHelp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabHelp)
	afx_msg void OnReadme();
	afx_msg void OnGotoForum();
	afx_msg void OnReportBug();
	afx_msg void OnFeaturesOn();
	afx_msg void OnFeaturesOff();
	afx_msg void OnCustom();
	virtual BOOL OnInitDialog();
	afx_msg void OnRegisterButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void OpenInternetPage(char *page);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABHELP_H__48E3367A_08C5_4B51_8F02_45E3055CFC2D__INCLUDED_)
