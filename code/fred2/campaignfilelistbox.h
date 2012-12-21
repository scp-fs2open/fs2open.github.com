/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

// CampaignFilelistBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// campaign_filelist_box window

class campaign_filelist_box : public CListBox
{
// Construction
public:
	void initialize();
	campaign_filelist_box();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(campaign_filelist_box)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~campaign_filelist_box();

	// Generated message map functions
protected:
	//{{AFX_MSG(campaign_filelist_box)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
