#if !defined(AFX_DX5DISP_H__66BC0F0C_95B5_4D43_8EE4_20CE7C215041__INCLUDED_)
#define AFX_DX5DISP_H__66BC0F0C_95B5_4D43_8EE4_20CE7C215041__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DX5Disp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDX5Disp dialog

class CDX5Disp : public CDialog
{
// Construction
public:
	void LoadSettings();
	void UpdateResList();
	void OnApply();
	CDX5Disp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDX5Disp)
	enum { IDD = IDD_DX5 };
	CComboBox	m_res_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDX5Disp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDX5Disp)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DX5DISP_H__66BC0F0C_95B5_4D43_8EE4_20CE7C215041__INCLUDED_)
