#if !defined(AFX_SOUNDENVIRONMENTDLG_H__C27DA258_9C66_4987_8D20_AEEB260644A8__INCLUDED_)
#define AFX_SOUNDENVIRONMENTDLG_H__C27DA258_9C66_4987_8D20_AEEB260644A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// soundenvironmentdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SoundEnvironment dialog

class SoundEnvironment : public CDialog
{
// Construction
public:
	SoundEnvironment(CWnd* pParent = NULL);   // standard constructor
	int query_modified();
	void OnOK();
	void OnCancel();

// Dialog Data
	//{{AFX_DATA(SoundEnvironment)
	enum { IDD = IDD_SOUND_ENVIRONMENT };
	int		m_environment;
	float	m_damping;
	float	m_decay_time;
	float	m_volume;
	CString	m_wave_filename;
	//}}AFX_DATA

	CBitmap m_play_bm;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SoundEnvironment)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SoundEnvironment)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSelChangeSoundEnvironment();
	afx_msg void OnBrowseWave();
	afx_msg void OnPlay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int m_wave_id;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDENVIRONMENT_H__C27DA258_9C66_4987_8D20_AEEB260644A8__INCLUDED_)
