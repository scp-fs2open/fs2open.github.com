#if !defined(AFX_TABSPEECH_H__9BC1005D_6480_4F0F_8866_2CF6EC965FD0__INCLUDED_)
#define AFX_TABSPEECH_H__9BC1005D_6480_4F0F_8866_2CF6EC965FD0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabSpeech.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabSpeech dialog

class CTabSpeech : public CDialog
{
// Construction
public:
	void LoadSettings();
	void SaveSettings();
	void UpdateVoiceFromCombo();
	CTabSpeech(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabSpeech)
	enum { IDD = IDD_SPEECH };
	CSliderCtrl	m_volume_control;
	CButton	m_checkbox_techroom;
	CButton	m_checkbox_ingame;
	CButton	m_checkbox_briefings;
	CEdit	m_edit_box;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabSpeech)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabSpeech)
	virtual BOOL OnInitDialog();
	afx_msg void OnPlay();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSelchangeVoiceCombo();
	afx_msg void OnGetVoices();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DWORD m_speech_volume;
	bool  m_speech_supported;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABSPEECH_H__9BC1005D_6480_4F0F_8866_2CF6EC965FD0__INCLUDED_)
