#if !defined(AFX_TABNETWORK_H__7C04D105_3D99_4353_882A_66A2BC1D1F19__INCLUDED_)
#define AFX_TABNETWORK_H__7C04D105_3D99_4353_882A_66A2BC1D1F19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabNetwork.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabNetwork dialog

class CTabNetwork : public CDialog
{
// Construction
public:
	void LoadSettings();
	void OnApply();
	CTabNetwork(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabNetwork)
	enum { IDD = IDD_NETWORK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabNetwork)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabNetwork)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABNETWORK_H__7C04D105_3D99_4353_882A_66A2BC1D1F19__INCLUDED_)
