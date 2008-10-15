/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/ShipEditorDlg.h $
 * $Revision: 1.3 $
 * $Date: 2006-05-30 05:37:29 $
 * $Author: Goober5000 $
 *
 * Single ship editing dialog
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/04/20 06:32:01  Goober5000
 * proper capitalization according to Volition
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.7  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.6  2004/09/29 17:26:33  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.5  2003/09/05 06:53:41  Goober5000
 * added code to change the player's ship in single player
 * --Goober5000
 *
 * Revision 1.4  2003/04/29 01:04:28  Goober5000
 * custom hitpoints for FRED
 * --Goober5000
 *
 * Revision 1.3  2003/03/25 07:03:29  Goober5000
 * added beginning functionality for $Texture Replace implementation in FRED
 * --Goober5000
 *
 * Revision 1.2  2002/08/15 01:06:34  penguin
 * Include filename reorg (to coordinate w/ fs2_open)
 *
 * Revision 1.1.1.1  2002/07/15 03:11:03  inquisitor
 * Initial FRED2 Checking
 *
 * 
 * 4     5/20/99 7:00p Dave
 * Added alternate type names for ships. Changed swarm missile table
 * entries.
 * 
 * 3     2/11/99 2:15p Andsager
 * Add ship explosion modification to FRED
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
 * 65    4/07/98 9:42a Allender
 * put in persona combo box into ship editor.  Removed code to assign
 * personas based on message
 * 
 * 64    3/27/98 12:02p Sandeep
 * 
 * 63    3/25/98 4:14p Hoffoss
 * Split ship editor up into ship editor and a misc dialog, which tracks
 * flags and such.
 * 
 * 62    3/16/98 8:27p Allender
 * Fred support for two new AI flags -- kamikaze and no dynamic goals.
 * 
 * 61    3/09/98 4:30p Allender
 * multiplayer secondary weapon changes.  red-alert and cargo-known-delay
 * sexpressions.  Add time cargo revealed to ship structure
 * 
 * 60    2/17/98 11:42a Hoffoss
 * Added support for hidden from sensors condition.
 * 
 * 59    2/06/98 2:54p Hoffoss
 * Fixed some bugs in dialog init, and cleared up some of the confusion
 * about how it works by renaming some variables and adding comments.
 * 
 * 58    1/29/98 5:14p Hoffoss
 * Added support for a SF_INVULNERABLE ship flag in Fred.
 * 
 * 57    11/13/97 4:14p Allender
 * automatic assignment of hotkeys for starting wings.  Appripriate
 * warnings when they are incorrectly used.  hotkeys correctly assigned to
 * ships/wing arriving after mission start
 * 
 * 56    11/10/97 10:13p Allender
 * added departure anchor to Fred and FreeSpace in preparation for using
 * docking bays.  Functional in Fred, not in FreeSpace.
 * 
 * 55    10/21/97 4:49p Allender
 * added flags to Fred and FreeSpace to forgo warp effect (toggle in ship
 * editor in Fred)
 * 
 * 54    10/14/97 5:33p Hoffoss
 * Added Fred support (and fsm support) for the no_arrival_music flags in
 * ships and wings.
 * 
 * 53    9/17/97 5:43p Hoffoss
 * Added Fred support for new player start information.
 * 
 * 52    9/04/97 4:31p Hoffoss
 * Fixed bug: Changed ship editor to not touch wing info (arrival or
 * departure cues) to avoid conflicts with wing editor's changes.
 * 
 * 51    8/30/97 9:52p Hoffoss
 * Implemented arrival location, distance, and anchor in Fred.
 * 
 * 50    8/25/97 5:56p Hoffoss
 * Added multiple asteroid field support, loading and saving of asteroid
 * fields, and ship score field to Fred.
 * 
 * 49    8/20/97 6:53p Hoffoss
 * Implemented escort flag support in Fred.
 * 
 * 48    8/16/97 12:06p Hoffoss
 * Fixed bug where a whole wing is deleted that is being referenced.
 * 
 * 47    8/12/97 7:17p Hoffoss
 * Added previous button to ship and wing editors.
 * 
 * 46    8/12/97 6:32p Hoffoss
 * Added code to allow hiding of arrival and departure cues in editors.
 * 
 * 45    8/08/97 1:31p Hoffoss
 * Added syncronization protection to cur_object_index changes.
 *
 * $NoKeywords: $
 */

#ifndef _SHIPEDITORDLG_H
#define _SHIPEDITORDLG_H

#include "Sexp_tree.h"
#include "ShipGoalsDlg.h"
#include "Management.h"

/////////////////////////////////////////////////////////////////////////////
// CShipEditorDlg dialog

#define	WM_GOODBYE	(WM_USER+5)
#define	ID_ALWAYS_ON_TOP	0x0f00

class numeric_edit_control
{
	int value;
	int unique;
	int control_id;
	CWnd *dlg;

public:
	void setup(int id, CWnd *wnd);
	void blank() { unique = 0; }
	void init(int n);
	void set(int n);
	void display();
	void save(int *n);
	void fix(int n);
};

class CShipEditorDlg : public CDialog
{
private:
	int make_ship_list(int *arr);
	int update_ship(int ship);
	int initialized;
	int multi_edit;
	int always_on_top;
	int cue_height;
	int mission_type;  // indicates if single player(1) or multiplayer(0)
	CView*	m_pSEView;
	CCriticalSection CS_update;

// Construction
public:
	int player_ship, single_ship;
	int editing;
	int modified;
	int select_sexp_node;
	int bypass_errors;
	int bypass_all;

	int enable;  // used to enable(1)/disable(0) controls based on if any ship selected
	int p_enable;  // used to enable(1)/disable(0) controls based on if a player ship

	int tristate_set(int val, int cur_state);
	void show_hide_sexp_help();
	void calc_cue_height();
	int verify();
	void OnInitMenu(CMenu *m);
	void OnOK();
	int update_data(int redraw = 1);
	void initialize_data(int full);
	CShipEditorDlg(CWnd* pParent = NULL);   // standard constructor
	CShipEditorDlg(CView* pView);

	// alternate ship name stuff
	void ship_alt_name_init(int base_ship);
	void ship_alt_name_close(int base_ship);

	// callsign stuff
	void ship_callsign_init(int base_ship);
	void ship_callsign_close(int base_ship);

	BOOL Create();

// Dialog Data
	//{{AFX_DATA(CShipEditorDlg)
	enum { IDD = IDD_SHIP_EDITOR };
	CButton	m_no_departure_warp;
	CButton	m_no_arrival_warp;
	CButton	m_player_ship;
	CSpinButtonCtrl	m_destroy_spin;
	CSpinButtonCtrl	m_departure_delay_spin;
	CSpinButtonCtrl	m_arrival_delay_spin;
	sexp_tree	m_departure_tree;
	sexp_tree	m_arrival_tree;
	CString	m_ship_name;
	CString	m_cargo1;
	int		m_ship_class;
	int		m_team;
	int		m_arrival_location;
	int		m_departure_location;
	int		m_ai_class;
	numeric_edit_control	m_arrival_delay;
	numeric_edit_control	m_departure_delay;
	int		m_hotkey;
	BOOL	m_update_arrival;
	BOOL	m_update_departure;
	numeric_edit_control	m_destroy_value;
	numeric_edit_control	m_score;
	numeric_edit_control	m_assist_score;
	numeric_edit_control	m_arrival_dist;
	numeric_edit_control m_kdamage;
	int		m_arrival_target;
	int		m_departure_target;
	int		m_persona;	
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShipEditorDlg)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShipEditorDlg)
	afx_msg void OnClose();
	afx_msg void OnRclickArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGoals();
	afx_msg void OnSelchangeShipClass();
	afx_msg void OnInitialStatus();
	afx_msg void OnWeapons();
	afx_msg void OnShipReset();
	afx_msg void OnDeleteShip();
	afx_msg void OnShipTbl();
	afx_msg void OnNext();
	afx_msg void OnSelchangedArrivalTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedDepartureTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHideCues();
	afx_msg void OnPrev();
	afx_msg void OnSelchangeArrivalLocation();
	afx_msg void OnPlayerShip();
	afx_msg void OnNoArrivalWarp();
	afx_msg void OnNoDepartureWarp();
	afx_msg void OnSelchangeDepartureLocation();
	afx_msg void OnSelchangeHotkey();
	afx_msg void OnFlags();
	afx_msg void OnIgnoreOrders();
	afx_msg void OnSpecialExp();
	afx_msg void OnTextures();
	afx_msg void OnSpecialHitpoints();
	afx_msg void OnAltShipClass();
	afx_msg void OnSetAsPlayerShip();
	afx_msg void OnRestrictArrival();
	afx_msg void OnRestrictDeparture();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
