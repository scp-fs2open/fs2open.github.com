#if !defined(AFX_DX8DISP_H__6E23FCC0_3E03_4626_A00B_BB770A8EDA7A__INCLUDED_)
#define AFX_DX8DISP_H__6E23FCC0_3E03_4626_A00B_BB770A8EDA7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DX8Disp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDX8Disp dialog

class CDX8Disp : public CDialog
{
// Construction
public:
	bool m_allow_only_standard_modes;
	void LoadSettings(int flags);
	void UpdateAntialiasList(int select = 0);
	void UpdateResList(
		unsigned int requested_wdith = -1, 
		unsigned int requested_height = -1, 
		int requested_cdepth = -1);
	void UpdateAdapterList(int select = 0);
	CDX8Disp(CWnd* pParent = NULL);   // standard constructor
	void OnApply(int flags);

// Dialog Data
	//{{AFX_DATA(CDX8Disp)
	enum { IDD = IDD_DX8 };
	CComboBox	m_adapter_list;
	CComboBox	m_res_list;
	CComboBox	m_cdepth_list;
	CComboBox	m_antialias_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDX8Disp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDX8Disp)
	afx_msg void OnSelchangeResList();
	afx_msg void OnChangeCDepth();
	afx_msg void OnSelchangeAntialiasList();
	afx_msg void OnSelchangeAdapterList();
	virtual BOOL OnInitDialog();
	afx_msg void OnGenCaps();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool m_dx8_initialised_ok;
	int GetCDepth(int cdepth);
	void UpdateCDepthList();
	void UpdateTexFilterList();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DX8DISP_H__6E23FCC0_3E03_4626_A00B_BB770A8EDA7A__INCLUDED_)
