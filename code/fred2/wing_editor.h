/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/FRED2/wing_editor.h $
 * $Revision: 1.1 $
 * $Date: 2006-01-19 02:27:32 $
 * $Author: Goober5000 $
 *
 * Wing editor dialog box handler code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.4  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.3  2003/01/06 20:49:15  Goober5000
 * FRED2 support for wing squad logos - look in the wing editor
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:04  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 2     10/07/98 6:28p Dave
 * Initial checkin. Renamed all relevant stuff to be Fred2 instead of
 * Fred. Globalized mission and campaign file extensions. Removed Silent
 * Threat specific code.
 * 
 * 1     10/07/98 3:01p Dave
 * 
 * 1     10/07/98 3:00p Dave
 * 
 * 35    3/16/98 8:27p Allender
 * Fred support for two new AI flags -- kamikaze and no dynamic goals.
 * 
 * 34    2/23/98 9:48p Allender
 * added no arrival/departure warps to wings
 * 
 * 33    11/25/97 10:03a Allender
 * added no arrival message checkbox to wing editor
 * 
 * 32    11/25/97 9:42a Hoffoss
 * Removed starting wing checkbox from wing editor.
 * 
 * 31    11/13/97 4:14p Allender
 * automatic assignment of hotkeys for starting wings.  Appripriate
 * warnings when they are incorrectly used.  hotkeys correctly assigned to
 * ships/wing arriving after mission start
 * 
 * 30    11/11/97 2:13p Allender
 * docking bay support for Fred and Freespace.  Added hook to ai code for
 * arrival/departure from dock bays.  Fred support now sufficient.
 * 
 * 29    11/10/97 10:13p Allender
 * added departure anchor to Fred and Freespace in preparation for using
 * docking bays.  Functional in Fred, not in FreeSpace.
 * 
 * 28    10/14/97 5:33p Hoffoss
 * Added Fred support (and fsm support) for the no_arrival_music flags in
 * ships and wings.
 * 
 * 27    8/30/97 9:52p Hoffoss
 * Implemented arrival location, distance, and anchor in Fred.
 * 
 * 26    8/22/97 4:16p Hoffoss
 * added support for arrival and departure info in ship editor using
 * wing's info if editing marked ships in a wing instead of using ship's.
 * 
 * 25    8/13/97 11:22p Hoffoss
 * Implemented wave delay min and max in Fred.
 * 
 * 24    8/12/97 7:17p Hoffoss
 * Added previous button to ship and wing editors.
 * 
 * 23    8/12/97 6:32p Hoffoss
 * Added code to allow hiding of arrival and departure cues in editors.
 * 
 * 22    7/25/97 2:40p Hoffoss
 * Fixed bug in sexp tree selection updating handling.
 * 
 * 21    7/09/97 2:38p Allender
 * organized ship/wing editor dialogs.  Added protect ship and ignore
 * count checkboxes to those dialogs.  Changed flag code for
 * parse_objects.  Added unprotect sexpressions
 * 
 * 20    6/05/97 6:10p Hoffoss
 * Added features: Autosaving, object hiding.  Also fixed some minor bugs.
 * 
 * 19    5/30/97 11:33a Allender
 * more hotkey combo box stuff
 * 
 * 18    5/23/97 1:53p Hoffoss
 * Fixed problems with modeless dialog updating.  It won't get caught in
 * an infinate loop anymore, but still gives an error warning 3 times when
 * using cancel and trying to switch window focus to main window.  Don't
 * know if I can fix that, but it's not too critical right now.
 * 
 * 17    4/28/97 2:37p Hoffoss
 * Added hotkey editing to Fred for ships and wings.
 * 
 * 16    3/20/97 3:55p Hoffoss
 * Major changes to how dialog boxes initialize (load) and update (save)
 * their internal data.  This should simplify things and create less
 * problems.
 * 
 * 15    2/20/97 4:03p Hoffoss
 * Several ToDo items: new reinforcement clears arrival cue, reinforcement
 * control from ship and wing dialogs, show grid toggle.
 * 
 * 14    2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

#include "Sexp_tree.h"

/////////////////////////////////////////////////////////////////////////////
// wing_editor dialog

class wing_editor : public CDialog
{
// Construction
public:
	int cue_height;
	int bypass_errors;
	int modified;
	int select_sexp_node;

	void initialize_data_safe(int full_update);
	void update_data_safe();
	void show_hide_sexp_help();
	void calc_cue_height();
	int verify();
	wing_editor(CWnd* pParent = NULL);   // standard constructor
	BOOL Create();
	void OnOK();
	int update_data(int redraw = 1);
	void initialize_data(int full);

// Dialog Data
	//{{AFX_DATA(wing_editor)
	enum { IDD = IDD_WING_EDITOR };
	CSpinButtonCtrl	m_departure_delay_spin;
	CSpinButtonCtrl	m_arrival_delay_spin;
	sexp_tree	m_departure_tree;
	sexp_tree	m_arrival_tree;
	CSpinButtonCtrl	m_threshold_spin;
	CSpinButtonCtrl	m_waves_spin;
	CString	m_wing_name;
	int		m_special_ship;
	int		m_waves;
	int		m_threshold;
	int		m_arrival_location;
	int		m_departure_location;
	int		m_arrival_delay;
	int		m_departure_delay;
	BOOL	m_reinforcement;
	int		m_hotkey;
	BOOL	m_ignore_count;
	int		m_arrival_delay_max;
	int		m_arrival_delay_min;
	int		m_arrival_dist;
	int		m_arrival_target;
	BOOL	m_no_arrival_music;
	int		m_departure_target;
	BOOL	m_no_arrival_message;
	BOOL	m_no_arrival_warp;
	BOOL	m_no_departure_warp;
	BOOL	m_no_dynamic;
	CString	m_wing_squad_filename;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(wing_editor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(wing_editor)
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnDeltaposSpinWaves(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteWing();
	afx_msg void OnDisbandWing();
	afx_msg void OnClose();
	afx_msg void OnGoals2();
	afx_msg void OnReinforcement();
	afx_msg void OnNext();
	afx_msg void OnSelchangedArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHideCues();
	afx_msg void OnPrev();
	afx_msg void OnSelchangeArrivalLocation();
	afx_msg void OnSelchangeDepartureLocation();
	afx_msg void OnSelchangeHotkey();
	afx_msg void OnSquadLogo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
