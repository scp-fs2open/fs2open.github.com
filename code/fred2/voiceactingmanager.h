#if !defined(AFX_VOICEACTINGMANAGER_H__49686A3D_8661_45BD_BED0_32A43FC6888A__INCLUDED_)
#define AFX_VOICEACTINGMANAGER_H__49686A3D_8661_45BD_BED0_32A43FC6888A__INCLUDED_

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
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VoiceActingManager)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VoiceActingManager)
	afx_msg void OnGenFilenames();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOICEACTINGMANAGER_H__49686A3D_8661_45BD_BED0_32A43FC6888A__INCLUDED_)
