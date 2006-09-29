#if !defined(AFX_VoiceActingManager_H__920EF950_8A59_4888_B7F6_E218DC869800__INCLUDED_)
#define AFX_VoiceActingManager_H__920EF950_8A59_4888_B7F6_E218DC869800__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VoiceActingManager.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VoiceActingManager dialog

class VoiceActingManager : public CDialog
{
// Construction
public:
	VoiceActingManager(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(VoiceActingManager)
	enum { IDD = IDD_VOICE_MANAGER };
	CString	m_abbrev_briefing;
	CString	m_abbrev_campaign;
	CString	m_abbrev_command_briefing;
	CString	m_abbrev_debriefing;
	CString	m_abbrev_message;
	CString	m_abbrev_mission;
	CString	m_example;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VoiceActingManager)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CString get_suffix();
	int calc_digits(int size);
	void build_example();
	void build_example(CString section);
	CString generate_filename(CString section, int number, int digits);

	// Generated message map functions
	//{{AFX_MSG(VoiceActingManager)
	virtual BOOL OnInitDialog();
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
	afx_msg void OnChangeOtherSuffix();
	afx_msg void OnGenerateFileNames();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VoiceActingManager_H__920EF950_8A59_4888_B7F6_E218DC869800__INCLUDED_)
