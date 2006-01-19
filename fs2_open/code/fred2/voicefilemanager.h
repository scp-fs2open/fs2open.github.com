#if !defined(AFX_VoiceFileManager_H__920EF950_8A59_4888_B7F6_E218DC869800__INCLUDED_)
#define AFX_VoiceFileManager_H__920EF950_8A59_4888_B7F6_E218DC869800__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VoiceFileManager.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VoiceFileManager dialog

class VoiceFileManager : public CDialog
{
// Construction
public:
	VoiceFileManager(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(VoiceFileManager)
	enum { IDD = IDD_VOICE_FILES };
	CString	m_abbrev_briefing;
	CString	m_abbrev_campaign;
	CString	m_abbrev_command_briefing;
	CString	m_abbrev_debriefing;
	CString	m_abbrev_message;
	CString	m_example;
	CString	m_abbrev_mission;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VoiceFileManager)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void build_example();
	void build_example(CString section);
	CString get_separator();
	CString generate_filename(CString section, int number, int digits);

	// Generated message map functions
	//{{AFX_MSG(VoiceFileManager)
	virtual BOOL OnInitDialog();
	afx_msg void OnSepDashes();
	afx_msg void OnSepNothing();
	afx_msg void OnSepUnderscores();
	afx_msg void OnSetfocusAbbrevBriefing();
	afx_msg void OnSetfocusAbbrevCampaign();
	afx_msg void OnSetfocusAbbrevCommandBriefing();
	afx_msg void OnSetfocusAbbrevDebriefing();
	afx_msg void OnSetfocusAbbrevMessage();
	afx_msg void OnSetfocusAbbrevMission();
	afx_msg void OnChangeAbbrevBriefing();
	afx_msg void OnChangeAbbrevCampaign();
	afx_msg void OnChangeAbbrevCommandBriefing();
	afx_msg void OnChangeAbbrevDebriefing();
	afx_msg void OnChangeAbbrevMessage();
	afx_msg void OnChangeAbbrevMission();
	afx_msg void OnAutogenerate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VoiceFileManager_H__920EF950_8A59_4888_B7F6_E218DC869800__INCLUDED_)
