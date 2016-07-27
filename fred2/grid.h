/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



/////////////////////////////////////////////////////////////////////////////
// CGrid dialog

class CGrid : public CDialog
{
private:
	CView*	m_pGView;

// Construction
public:
	CGrid(CWnd* pParent = NULL);   // standard constructor
	CGrid(CView* pView);

	BOOL Create();

// Dialog Data
	//{{AFX_DATA(CGrid)
	enum { IDD = IDD_GRID };
	UINT	m_GridSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGrid)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGrid)
	afx_msg void OnGridXyPlane();
	afx_msg void OnGridXzPlane();
	afx_msg void OnGridYzPlane();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
