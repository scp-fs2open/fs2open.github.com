/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */
#ifndef _ADJUSTGRIDDLG_H
#define _ADJUSTGRIDDLG_H

/**
 * @class adjust_grid_dlg
 *
 * @brief Dialog for the mission grid properties.
 */
class adjust_grid_dlg : public CDialog
{
public:
	/**
	 * @brief Standard constructor, nothing special
	 *
	 * @see adjust_grid_dlg::OnInitDialog
	 */
	adjust_grid_dlg(CWnd* pParent = NULL);

	/**
	 * @brief Handler for the OK button.
	 *
	 * @details Transfers the X, Y, Z value from the spin control to the appropriate grid value and then exchanges data. The type of grid that is drawn (XZ, XY, YZ) is controlled by which button is active.
	 */
	void OnOK();

	enum
	{
		IDD = IDD_ADJUST_GRID
	};

	CSpinButtonCtrl m_spinx;    //!< Spin control X value
	CSpinButtonCtrl m_spiny;    //!< Spin control Y value
	CSpinButtonCtrl m_spinz;    //!< Spin control Z value

	int m_x;    //!< Grid's X value
	int m_y;    //!< Grid's Y value
	int m_z;    //!< Grid's Z value
	//}}AFX_DATA

	//{{AFX_VIRTUAL(adjust_grid_dlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	//{{AFX_MSG(adjust_grid_dlg)

	/**
	 * @brief Handler for dialog initialization.
	 */
	virtual BOOL OnInitDialog();

	/**
	 * @brief Handler for the XY button
	 */
	afx_msg void OnXyPlane();

	/**
	 * @brief Handler for the XZ button
	 */
	afx_msg void OnXzPlane();

	/**
	 * @brief Handler for the YZ button
	 */
	afx_msg void OnYzPlane();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
#endif	// _ADJUSTGRIDDLG_H
