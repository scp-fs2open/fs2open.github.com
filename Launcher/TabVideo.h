#if !defined(AFX_TABVIDEO_H__D2FDD185_AC10_4C07_900D_4E75E7E5F42C__INCLUDED_)
#define AFX_TABVIDEO_H__D2FDD185_AC10_4C07_900D_4E75E7E5F42C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabVideo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabVideo dialog

class CTabVideo : public CDialog
{
// Construction
public:
	int SelectAPI(int api);
	void LoadSettings(int flags);
	void Update(int type, int flgas);
	void SelectTab(int select);
	BOOL InitTabControl(int type);
	CTabVideo(CWnd* pParent = NULL);   // standard constructor

	void OnApply(int flags);

// Dialog Data
	//{{AFX_DATA(CTabVideo)
	enum { IDD = IDD_VIDEO };
	CButton	m_gf4_fix_button;
	CStatic	m_api_list;
	CButton	m_largetxt_checkbox;
	CButton	m_hi_sparky_checkbox;
	CComboBox	m_gfx_level_droplist;
	CComboBox	m_api_droplist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabVideo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_last_tab;

	// Generated message map functions
	//{{AFX_MSG(CTabVideo)
	afx_msg void OnSelchangeGfxapiList();
	virtual BOOL OnInitDialog();
	afx_msg void OnGf4Ffix();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABVIDEO_H__D2FDD185_AC10_4C07_900D_4E75E7E5F42C__INCLUDED_)
