#if !defined(AFX_DX9DISP_H__5B9EC0B6_796C_423C_9CA2_4C76E1D126F6__INCLUDED_)
#define AFX_DX9DISP_H__5B9EC0B6_796C_423C_9CA2_4C76E1D126F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DX9Disp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDX9Disp dialog

class CDX9Disp : public CDialog
{
// Construction
public:
	void LoadSettings();
	void SaveSettings();
	void UpdateAntialiasList(int select = 0);
	void UpdateResList(
		unsigned int requested_wdith = -1, 
		unsigned int requested_height = -1, 
		int requested_cdepth = -1);
	void UpdateAdapterList(int select = 0);
	CDX9Disp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDX9Disp)
	enum { IDD = IDD_DX9 };
	CButton	m_show_all_modes_checkbox;
	CComboBox	m_adapter_list;
	CComboBox	m_res_list;
	CComboBox	m_antialias_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDX9Disp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDX9Disp)
	afx_msg void OnSelchangeResList();
	afx_msg void OnSelchangeAntialiasList();
	afx_msg void OnSelchangeAdapterList();
	virtual BOOL OnInitDialog();
	afx_msg void OnGenCaps();
	afx_msg void OnDestroy();
	afx_msg void OnAllowNonSmodes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool m_allow_only_standard_modes;
	bool m_dx_initialised_ok;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DX9DISP_H__6E23FCC0_3E03_4626_A00B_BB770A8EDA7A__INCLUDED_)
