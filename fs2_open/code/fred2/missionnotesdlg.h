/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Fred2/MissionNotesDlg.h $
 * $Revision: 1.7 $
 * $Date: 2007-07-23 15:16:48 $
 * $Author: Kazan $
 *
 * Mission notes editor dialog box handling code
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2007/01/07 00:01:28  Goober5000
 * add a feature for specifying the source of Command messages
 *
 * Revision 1.5  2006/07/30 20:01:56  Kazan
 * resolve 1018 and an interface problem in fred2's ship editor
 *
 * Revision 1.4  2006/05/30 02:13:22  Goober5000
 * add substitute music boxes to FRED, and reset music properly when mission is cleared
 * --Goober5000
 *
 * Revision 1.3  2006/05/30 01:36:24  Goober5000
 * add AI Profile box to FRED
 * --Goober5000
 *
 * Revision 1.2  2006/04/05 16:11:44  karajorma
 * Changes to support the new Enable/Disable-Builtin-Messages SEXP
 *
 * Revision 1.1  2006/01/19 02:27:31  Goober5000
 * import FRED2 back into fs2_open module
 * --Goober5000
 *
 * Revision 1.17  2005/10/30 07:28:22  Goober5000
 * added Daisy Chain mission flag stuff in FRED
 * --Goober5000
 *
 * Revision 1.16  2005/07/25 06:51:33  Goober5000
 * mission flag changes for FRED
 * --Goober5000
 *
 * Revision 1.15  2005/07/13 02:40:50  Goober5000
 * remove PreProcDefine #includes in FRED
 * --Goober5000
 *
 * Revision 1.14  2004/12/15 19:16:13  Goober5000
 * FRED code for custom wing names
 * --Goober5000
 *
 * Revision 1.13  2004/10/12 07:45:30  Goober5000
 * added contrail speed threshold
 * --Goober5000
 *
 * Revision 1.12  2004/09/29 17:26:32  Kazan
 * PreProfDefines.h includes for fred2
 *
 * Revision 1.11  2004/09/17 07:14:05  Goober5000
 * fixed FRED stuff for warp effect, etc.
 * --Goober5000
 *
 * Revision 1.10  2004/04/29 03:35:18  wmcoolmon
 * Added Hull and Subsys repair ceiling fields to Mission Specs Dialog in FRED
 * Updated mission saving code to store both variables properly
 *
 * Revision 1.9  2003/05/10 00:02:42  phreak
 * code to handle user interface when selecting custom loading screens
 *
 * Revision 1.8  2003/03/19 22:49:59  Goober5000
 * yay mission flags :)
 * --Goober5000
 *
 * Revision 1.7  2003/01/19 07:02:15  Goober5000
 * fixed a bunch of bugs - "no-subspace-drive" should now work properly for
 * all ships, and all ships who have their departure anchor set to a capital ship
 * should exit to that ship when told to depart
 * --Goober5000
 *
 * Revision 1.6  2003/01/13 02:07:26  wmcoolmon
 * Removed the override checkbox + flag for ship trails and added a checkbox + flag to disable ship trails within the nebula, NO_NEB_TRAILS, to the Background dialog.
 *
 * Revision 1.5  2003/01/11 01:01:37  wmcoolmon
 * Added code for "Ship Trails override Nebula"
 *
 * Revision 1.4  2002/12/21 02:01:21  Goober5000
 * Added beam-free-all-by-default flag to FRED2
 * --Goober5000
 *
 * Revision 1.3  2002/08/10 10:50:11  wmcoolmon
 * Added support ship hull repair code
 *
 * Revision 1.2  2002/07/16 02:48:54  wmcoolmon
 * Added code to toggle ship trails regardless of nebula
 *
 * Revision 1.1.1.1  2002/06/02 01:18:39  inquisitor
 * Initial Volition Source, mixed case, windows only
 *
 * 
 * 5     8/23/99 6:21p Jefff
 * added "no traitor" option to missions (and fred)
 * 
 * 4     8/23/99 5:04p Jefff
 * Added new mission flag to disable built-in messages from playing.
 * Added fred support as well.
 * 
 * 3     3/24/99 4:05p Dave
 * Put in support for assigning the player to a specific squadron with a
 * specific logo. Preliminary work for doing pos/orient checksumming in
 * multiplayer to reduce bandwidth.
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
 * 16    6/17/98 2:47p Hoffoss
 * Changed so missions are single, multi or training.  Not supporting
 * combos of the above anymore (in Fred).
 * 
 * 15    5/05/98 11:05p Allender
 * ability to flag mission as "no promotion" where promotions and badges
 * are *not* granted even if they should be.  Slight fix to multiplayer
 * problem where locking_subsys is wrong for players current target
 * 
 * 14    4/03/98 12:17a Allender
 * new sexpression to detect departed or destroyed.  optionally disallow
 * support ships.  Allow docking with escape pods 
 * 
 * 13    3/26/98 5:24p Allender
 * put in respawn edit box into mission notes dialog.  Made loading of
 * missions/campaign happen when first entering the game setup screen.
 * 
 * 12    3/16/98 8:27p Allender
 * Fred support for two new AI flags -- kamikaze and no dynamic goals.
 * 
 * 11    3/09/98 4:30p Allender
 * multiplayer secondary weapon changes.  red-alert and cargo-known-delay
 * sexpressions.  Add time cargo revealed to ship structure
 * 
 * 10    2/04/98 4:32p Allender
 * support for multiple briefings and debriefings.  Changes to mission
 * type (now a bitfield).  Bitfield defs for multiplayer modes
 * 
 * 9     1/02/98 4:55p Hoffoss
 * Added support for Mission_all_attack flag to Fred and loading/saving
 * code.
 * 
 * 8     9/30/97 5:56p Hoffoss
 * Added music selection combo boxes to Fred.
 * 
 * 7     8/11/97 3:19p Hoffoss
 * Implemented mission description.
 * 
 * 6     6/11/97 2:14p Hoffoss
 * Added game type (mission type) selection to Fred.
 * 
 * 5     5/20/97 2:28p Hoffoss
 * Added message box queries for close window operation on all modal
 * dialog boxes.
 * 
 * 4     4/17/97 2:01p Hoffoss
 * All dialog box window states are saved between sessions now.
 * 
 * 3     2/17/97 5:28p Hoffoss
 * Checked RCS headers, added them were missing, changing description to
 * something better, etc where needed.
 *
 * $NoKeywords: $
 */

/////////////////////////////////////////////////////////////////////////////
// CMissionNotesDlg dialog
#pragma once
class CMissionNotesDlg : public CDialog
{
// Construction
public:
	int query_modified();
	void OnCancel();
	void OnOK();
	int update_data();
	int initialize_data();
	CMissionNotesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMissionNotesDlg)
	enum { IDD = IDD_MISSION_NOTES };
	CSpinButtonCtrl	m_respawn_spin;
	CString	m_created;
	CString	m_modified;
	CString	m_mission_notes;
	CString	m_designer_name;
	CString	m_mission_title;
	CString	m_mission_desc;
	CString	m_squad_filename;
	CString	m_squad_name;
	CString m_loading_640;
	CString m_loading_1024;
	int		m_ai_profile;
	int		m_event_music;
	CString	m_substitute_event_music;
	int		m_command_persona;
	CString	m_command_sender;
	BOOL		m_full_war;
	BOOL		m_red_alert;
	BOOL		m_scramble;
	BOOL		m_daisy_chained_docking;
	UINT		m_num_respawns;
	int			m_disallow_support;
	BOOL		m_no_promotion;
	BOOL		m_no_builtin_msgs;
	BOOL		m_no_builtin_command_msgs;
	BOOL		m_no_traitor;
	BOOL		m_toggle_trails;
	BOOL		m_support_repairs_hull;
	BOOL		m_beam_free_all_by_default;
	BOOL		m_player_start_using_ai;
	BOOL		m_no_briefing;
	BOOL		m_no_debriefing;
	BOOL		m_autpilot_cinematics;
	float		m_max_hull_repair_val;
	float		m_max_subsys_repair_val;
	BOOL		m_contrail_threshold_flag;
	int			m_contrail_threshold;
	//}}AFX_DATA

	CString	m_mission_notes_orig;
	CString	m_designer_name_orig;
	CString	m_mission_title_orig;
	CString	m_mission_desc_orig;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissionNotesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_type;
	void set_types();

	// Generated message map functions
	//{{AFX_MSG(CMissionNotesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnTraining();
	afx_msg void OnMulti();
	afx_msg void OnSingle();
	afx_msg void OnSquadLogo();
	afx_msg void OnLoad640();
	afx_msg void OnLoad1024();
	afx_msg void OnToggleContrailThreshold();
	afx_msg void OnCustomWingNames();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeLoadingScreen641();
};
