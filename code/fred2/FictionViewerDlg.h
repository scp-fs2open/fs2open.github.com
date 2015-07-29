#if !defined(AFX_FICTIONVIEWERDLG_H__46BAD1B6_F9DC_4414_9C33_18EC17FD491F__INCLUDED_)
#define AFX_FICTIONVIEWERDLG_H__46BAD1B6_F9DC_4414_9C33_18EC17FD491F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FictionViewerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// FictionViewerDlg dialog

class FictionViewerDlg : public CDialog
{
// Construction
public:
	FictionViewerDlg(CWnd* pParent = NULL);   // standard constructor
	int query_modified();
	void OnOK();
	void OnCancel();

// Dialog Data
	//{{AFX_DATA(FictionViewerDlg)
	enum { IDD = IDD_FICTION_VIEWER };
	CString	m_story_file;
	CString	m_font_file;
	CString m_voice_file;
	int		m_fiction_music;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FictionViewerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FictionViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FICTIONVIEWERDLG_H__46BAD1B6_F9DC_4414_9C33_18EC17FD491F__INCLUDED_)
