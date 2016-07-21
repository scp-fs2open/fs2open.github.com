#if !defined(_BACKGROUND_CHOOSER_H_)
#define _BACKGROUND_CHOOSER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BackgroundChooser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BackgroundChooser dialog

class BackgroundChooser : public CDialog
{
// Construction
public:
	BackgroundChooser(int numBackgrounds);   // standard constructor
	void OnOK();
	void OnCancel();

	int GetChosenBackground();

// Dialog Data
	//{{AFX_DATA(BackgroundChooser)
	enum { IDD = IDD_BACKGROUND_CHOOSER};
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BackgroundChooser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_numBackgrounds;
	int m_chosenBackground;

	// Generated message map functions
	//{{AFX_MSG(BackgroundChooser)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(_BACKGROUND_CHOOSER_H_)
