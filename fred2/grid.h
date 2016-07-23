#ifndef _GRID_H
#define _GRID_H
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */



/**
 * @class CGrid
 *
 * @brief Dialog for adjusting/modifying the grid
 */
class CGrid : public CDialog
{
public:
	/**
	 * @brief Standard constructor
	 */
	CGrid(CWnd* pParent = NULL);

	/**
	 * @brief Standard constructor
	 */
	CGrid(CView* pView);

	/**
	 * @brief Creates the dialog
	 *
	 * @TODO This might not be used. Verify and remove
	 */
	BOOL Create();

	//{{AFX_DATA(CGrid)
	enum
	{
		IDD = IDD_GRID
	};

	UINT m_GridSize;    //!< Size of the the grid.
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CGrid)
	/**
	 * @brief Creates the dialog
	 *
	 * @param[in] lpszClassName
	 * @param[in] lpszWindowName
	 * @param[in] dwStyle
	 * @param[in] rect
	 * @param[in] pParentWnd
	 * @param[in] nID
	 * @param[in] pContext
	 */
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

	/**
	 * @brief Destroys the dialog
	 */
	virtual BOOL DestroyWindow();

protected:
	/**
	 * @brief Does data exchange
	 *
	 * @param[in] pDX
	 */
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CGrid)

	/**
	 * @brief Handler for the XY button
	 */
	afx_msg void OnGridXyPlane();

	/**
	 * @brief Handler for the XZ button
	 */
	afx_msg void OnGridXzPlane();

	/**
	 * @brief Handler for the YZ button
	 */
	afx_msg void OnGridYzPlane();

	/**
	 * @brief Handler for the Close button
	 */
	afx_msg void OnClose();

	/**
	 * @brief Handler for OnDestroy events
	 */
	afx_msg void OnDestroy();

	/**
	 * @brief Handler for OnKillFocus events
	 */
	afx_msg void OnKillFocus(CWnd* pNewWnd);

	/**
	 * @brief Handler for OnInitDialog events
	 */
	virtual BOOL OnInitDialog();

	/**
	 * @brief Handler for the vertical scroll buttons
	 */
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CView*	m_pGView;   //!< Reference to parent window

};
#endif // _GRID_H
