#if !defined(AFX_3DFXDISP_H__4C01DCEA_3180_4C00_8B7F_058A5732FB34__INCLUDED_)
#define AFX_3DFXDISP_H__4C01DCEA_3180_4C00_8B7F_058A5732FB34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// 3DFXDisp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// C3DFXDisp dialog

class C3DFXDisp : public CDialog
{
// Construction
public:
	void LoadSettings();
	void SaveSettings();
	void UpdateResList();
	C3DFXDisp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(C3DFXDisp)
	enum { IDD = IDD_3DFX };
	CComboBox	m_res_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(C3DFXDisp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(C3DFXDisp)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_3DFXDISP_H__4C01DCEA_3180_4C00_8B7F_058A5732FB34__INCLUDED_)
