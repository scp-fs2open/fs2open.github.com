#if !defined(AFX_TABCOMMLINE_H__ED21C389_0363_490B_92C8_09E5B6D46C0E__INCLUDED_)
#define AFX_TABCOMMLINE_H__ED21C389_0363_490B_92C8_09E5B6D46C0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabCommLine.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabCommLine dialog

class CTabCommLine : public CDialog
{
// Construction
public:
	void UpdateFlagList();
	CTabCommLine(CWnd* pParent = NULL);   // standard constructor
	~CTabCommLine();

	void SetModParameters(char *path);
	void LoadSettings(char *reg_path);
	void ConstructFlagList();
	void ConstructFlagListRetail();
	void GetStandardParameters(char *standard_params);
	int GetFlags();
	void SelectRegPathAndExeType();

	CString GetCommandLine();
	void UpdateCommandLine();
	void SaveSettings();

// Dialog Data
	//{{AFX_DATA(CTabCommLine)
	enum { IDD = IDD_COMM_LINE };
	CComboBox	m_flag_type_list;
	CComboBox	m_easy_flag;
	CListCtrl	m_flag_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabCommLine)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabCommLine)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeCustomParam();
	afx_msg void OnItemchangedFlagList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeFlagSetup();
	afx_msg void OnDblclkFlagList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeFlagType();
	afx_msg void OnSettingsNormal();
	afx_msg void OnSettingsMod();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool m_flag_gen_in_process;
	void ConstructFlagListInternal();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABCOMMLINE_H__ED21C389_0363_490B_92C8_09E5B6D46C0E__INCLUDED_)
