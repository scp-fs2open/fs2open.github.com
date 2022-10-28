#ifndef _MAINFRM_H
#define _MAINFRM_H
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */



#define	WM_MENU_POPUP_TEST	(WM_USER+9)

/**
 * @class color_combo_box
 *
 * @brief Our flavor of the combo box, Items are highlighted with their species color.
 *
 * @detail Shivans are Red, Terrans are blue, the Vasudans are green, and uh, FRED loves you? Such a terrible ryhme.
 *  Er, anyway. The colors for each species are defined in their respective species_info, which is from species_defs.tbl
 *
 * @TODO Maybe move this to its own .h and .cpp
 */
class color_combo_box : public CComboBox
{
public:
	/**
	 * @brief Sets the given item index as the selected item
	 *
	 * @param[in] model_index The index of the item within the combobox
	 *
	 * @returns The item index if successful, or
	 * @returns CB_ERR otherwise
	 */
	int SetCurSelNEW(int model_index);

	/**
	 * @brief Gets an index of the current selection
	 *
	 * @returns The ship_info index of the ship/object, or
	 * @returns The current selection if it is a special item, or
	 * @returns CB_ERR if an error occured
	 */
	int GetCurSelNEW();

private:
	/**
	 * @brief Draws the given item
	 *
	 * @param[in] lpDrawItemStruct Item to draw
	 */
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	/**
	 * @brief Calculates the minimum height required to fit all items. This is determined by the "tallest" character
	 */
	int  CalcMinimumItemHeight();

	/**
	 * @brief Measures the given item
	 *
	 * @details Er, actually does nothing other than Assert that the style of the combo box is LBS_ONWERDRAWFIXED and CBS_HASSTRINGS
	 */
	void MeasureItem(LPMEASUREITEMSTRUCT);
};

/**
 * @class CMainFrame
 *
 * @brief The main FRED window
 */
class CMainFrame : public CFrameWnd
{
public:

	void init_tools();

	/**
	 * @breif Standard deconstructor
	 */
	virtual ~CMainFrame();

	//{{AFX_VIRTUAL(CMainFrame)
	/**
	 * @brief Handler for pre-creation
	 *
	 * @details Can alter the window styles from this function. Currently we just pass the CS on to CFrameWnd
	 */
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

#ifdef _DEBUG
	/**
	 * @brief Asserts the window is valid
	 */
	virtual void AssertValid() const;

	/**
	 * @brief Presumably called when a stack dump is required
	 */
	virtual void Dump(CDumpContext& dc) const;
#endif

	CToolBar    m_wndToolBar;   //!< Instance of the tool bar
	CStatusBar  m_wndStatusBar; //!< Instance of the status bar

protected:
	/**
	 * @brief Constructor. Create from serialization only
	 */
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

	//{{AFX_MSG(CMainFrame)
	/**
	 * @brief OnCreate Handler. Creates the frame, toolbar, status bar, etc.
	 */
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	/**
	 * @brief Opens the CMissionNotes dialog
	 */
	afx_msg void OnFileMissionnotes();

	/**
	 * @brief Handler for LButtonUp events
	 *
	 * @note Seems to be dead code
	 * @TODO Verify and remove
	 */
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	/**
	 * @brief OnDestroy handler. Invoked when the Close button is clicked or from File->Exit
	 */
	afx_msg void OnDestroy();

	/**
	 * @brief Handler for View->Status Bar. Shows/Hides the status bar
	 */
	afx_msg void OnViewStatusBar();

	/**
	 * @brief Checks/unchecks the Status Bar menu item on the View menu
	 */
	afx_msg void OnUpdateViewStatusBar(CCmdUI* pCmdUI);

	/**
	 * @brief Called when the Left Mouse Button is pressed
	 */
	afx_msg void OnUpdateLeft(CCmdUI* pCmdUI);

	/**
	 * @brief Called when the Right Mouse Button is pressed
	 */
	afx_msg void OnUpdateRight(CCmdUI* pCmdUI);

	/**
	 * @brief Handler for Misc->Adjust Grid
	 */
	afx_msg void OnMikeGridcontrol();

	/**
	 * @brief Toggles the popup menu variable
	 */
	afx_msg void OnMenuPopupToggle1();

	/**
	 * @brief
	 */
	afx_msg void OnUpdateMenuPopupToggle1(CCmdUI* pCmdUI);

	/**
	 * @brief Handler for Right button clicks. Shows a popup menu at the mouse cursor's location
	 */
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	/**
	 * @brief Calls dialog1
	 *
	 * @note Not sure if this is actually useful
	 * @TODO Verify and remove
	 */
	afx_msg void OnHelpInputInterface();

	/**
	 * @brief Handler for OnClose() events. called by ON_WM_CLOSE()
	 */
	afx_msg void OnClose();

	/**
	 * @brief Handler for menu intialiation
	 */
	afx_msg void OnInitMenu(CMenu* pMenu);

	/**
	 * @brief Handler for Help->Help Topics. Opens .html documentation in the default browser
	 */
	afx_msg void OnFredHelp();
	//}}AFX_MSG

	/**
	 * @brief Handler for when the combobox selection changes. Brings the main FRED window back infocus
	 */
	afx_msg void OnNewShipTypeChange();

	/**
	 * @brief Related to the popup menu
	 */
	LRESULT OnMenuPopupTest(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};


extern CMainFrame *Fred_main_wnd;   //!< The main FRED window

extern color_combo_box m_new_ship_type_combo_box;  //!< The combo box
extern size_t num_ships_in_combo_box;

#endif // _MAINFRM_H
