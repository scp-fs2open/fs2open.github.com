/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#if !defined(AFX_DUMPSTATS_H__04996431_1A80_11D3_A923_0060088FAE88__INCLUDED_)
#define AFX_DUMPSTATS_H__04996431_1A80_11D3_A923_0060088FAE88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DumpStats.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DumpStats dialog

class DumpStats : public CDialog
{
// Construction
public:
	DumpStats(CWnd* pParent = NULL);   // standard constructor
	void get_mission_stats(CString &buffer);
	void get_background_stats(CString &buffer);
	void get_object_stats(CString &buffer);
	void get_objectives_and_goals(CString &buffer);
	void get_ship_weapon_selection(CString &buffer);
	void get_messaging_info(CString &buffer);
	void get_species_ship_breakdown(CString &buffer);
	void get_default_ship_loadouts(CString &buffer);

// Dialog Data
	//{{AFX_DATA(DumpStats)
	enum { IDD = IDD_DUMP_STATS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DumpStats)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DumpStats)
	virtual BOOL OnInitDialog();
	afx_msg void OnDumpToFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DUMPSTATS_H__04996431_1A80_11D3_A923_0060088FAE88__INCLUDED_)
