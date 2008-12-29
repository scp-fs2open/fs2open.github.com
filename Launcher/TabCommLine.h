#if !defined(AFX_TABCOMMLINE_H__ED21C389_0363_490B_92C8_09E5B6D46C0E__INCLUDED_)
#define AFX_TABCOMMLINE_H__ED21C389_0363_490B_92C8_09E5B6D46C0E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabCommLine.h : header file
//

enum
{
	// First 6 flags must remain set they are
//	FLAG_D3D9		= 1 << 0,
	FLAG_OGL		= 1 << 0,
	FLAG_D3D8		= 1 << 1,
	FLAG_D3D5		= 1 << 2,
	FLAG_3DFX		= 1 << 3,
	FLAG_DD5		= 1 << 4,
	FLAG_SFT		= 1 << 5,
	FLAG_FS1		= 1 << 6,
	FLAG_FS2		= 1 << 7,
	FLAG_MULTI		= 1 << 8,
	FLAG_MOD		= 1 << 9,
	FLAG_FS2OPEN	= 1 << 10,
	FLAG_SCP		= /*FLAG_D3D9 |*/ FLAG_D3D8 | FLAG_OGL | FLAG_FS2 | FLAG_MOD | FLAG_FS2OPEN,
	FLAG_NEW		= 1 << 31,
};
	
enum
	{
		EXE_TYPE_FS2,
		EXE_TYPE_FS2DEMO,
		EXE_TYPE_FS1,
		MAX_EXE_TYPES,
		EXE_TYPE_CUSTOM,
		EXE_TYPE_NONE = MAX_EXE_TYPES,
	};

/////////////////////////////////////////////////////////////////////////////
// CTabCommLine dialog

class CTabCommLine : public CDialog
{
// Construction
public:
	void UpdateFlagList();
	CTabCommLine(CWnd* pParent = NULL);   // standard constructor
	~CTabCommLine();

	void SetModParam(char *path);
	void LoadSettings(char *reg_path);
	void ConstructFlagList();
	void ConstructFlagListRetail();
	void UpdateStandardParam(char *standard_param);
	int GetFlags();
	void SelectRegPathAndExeType();

	CString GetCommLine();
	void UpdateFields();
	bool SaveSettings();

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
