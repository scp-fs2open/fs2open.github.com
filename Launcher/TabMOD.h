#if !defined(AFX_TABMOD_H__FB07B738_1087_4CB8_81B2_69E9FCC068FE__INCLUDED_)
#define AFX_TABMOD_H__FB07B738_1087_4CB8_81B2_69E9FCC068FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabMOD.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabMOD dialog

class CTabMOD : public CDialog
{
// Construction
public:
	CTabMOD(CWnd* pParent = NULL);   // standard constructor
	void SetMOD(char *absolute_path); 
	void GetModCommandLine(char *result);
	void GetActiveModName(char *result);

	void  SetSettings(char *flags); 
	char *GetSettings(bool defaultSettings = false); 

	void OnApply(int flags);
	void LoadSettings(int flags);

// Dialog Data
	//{{AFX_DATA(CTabMOD)
	enum { IDD = IDD_MOD };
	CStatic	m_mod_image;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabMOD)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabMOD)
	afx_msg void OnModSelect();
	afx_msg void OnModNone();
	afx_msg void OnModWebsite();
	afx_msg void OnModForum();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	HBITMAP LoadBitmap(char * path);
	void SetMODImage(char *path);
	void SetModName(char *path, char *name);
	bool parse_ini_file(char * ini_name);
	void CheckModSetting(char *mod_string);

private:
	HBITMAP MOD_bitmap;

	enum
	{
		INI_MOD_NAME,
		INI_IMAGE_NAME,
		INI_MOD_TEXT,
		INI_URL_WEBSITE,
		INI_URL_FORUM,
		INI_MOD_PRI,
		INI_MOD_SEC,
		INI_MAX
	};


	bool mod_selected;
	char *ini_text[INI_MAX]; 
	char m_absolute_text[MAX_PATH];
	char m_active_mod_name[MAX_PATH];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABMOD_H__FB07B738_1087_4CB8_81B2_69E9FCC068FE__INCLUDED_)
